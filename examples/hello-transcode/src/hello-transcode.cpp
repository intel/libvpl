//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) transcode application
///
/// @file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfxjpeg.h"
#include "vpl/mfxvideo.h"

#define MAX_PATH              260
#define TARGETKBPS            4000
#define FRAMERATE             30
#define OUTPUT_FILE           "out.h265"
#define WAIT_100_MILLSECONDS  100
#define BITSTREAM_BUFFER_SIZE 2000000

#define VERIFY(x, y)       \
    if (!(x)) {            \
        printf("%s\n", y); \
        goto end;          \
    }

#define ALIGN16(value) (((value + 15) >> 4) << 4)

mfxStatus CheckPipelineCapability(mfxLoader loader);
mfxStatus PrepareDecodeVariables(mfxSession session,
                                 FILE *f,
                                 mfxBitstream *bs,
                                 mfxVideoParam *param);
mfxStatus PrepareEncodeVariables(mfxU16 width,
                                 mfxU16 height,
                                 mfxBitstream *bs,
                                 mfxVideoParam *param);
mfxStatus ReadEncodedStream(mfxBitstream &bs, FILE *f);
void WriteEncodedStream(mfxBitstream &bs, FILE *f);
char *ValidateFileName(char *in);

void Usage(void) {
    printf("\n");
    printf("   Usage  :  hello-transcode InputBitstream\n\n");
    printf("             InputBitstream   ... input file name (JPEG elementary stream)\n\n");
    printf("   Example:  hello-transcode in.mjpeg\n");
    printf("   To view:  ffplay %s\n\n", OUTPUT_FILE);
    printf(" * Transcode JPEG elementary stream to HEVC/H265 elementary stream in %s\n\n",
           OUTPUT_FILE);
    return;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        Usage();
        return 1;
    }

    char *in_filename  = NULL;
    FILE *source       = NULL;
    FILE *sink         = NULL;
    mfxStatus sts      = MFX_ERR_NONE;
    mfxStatus sts_r    = MFX_ERR_NONE;
    mfxLoader loader   = NULL;
    mfxSession session = NULL;
    // Decode
    mfxBitstream bs_dec_in            = { 0 };
    mfxFrameSurface1 *dec_surface_out = NULL;
    mfxVideoParam stream_info         = { 0 };
    // Encode
    mfxBitstream bs_enc_out  = { 0 };
    mfxVideoParam enc_params = { 0 };

    mfxSyncPoint syncp   = { 0 };
    mfxU32 framenum      = 0;
    bool is_draining_dec = false;
    bool is_draining_enc = false;
    bool is_stillgoing   = true;

    // Setup input and output files
    in_filename = ValidateFileName(argv[1]);
    VERIFY(in_filename, "Input filename is not valid");

    source = fopen(in_filename, "rb");
    VERIFY(source, "Could not open input file");

    sink = fopen(OUTPUT_FILE, "wb");
    VERIFY(sink, "Could not create output file");

    // Creates the loader
    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed");

    // Check JPEG decode, and H.265 encode capability for transcode processing
    sts = CheckPipelineCapability(loader);
    VERIFY(MFX_ERR_NONE == sts, "VPL plugin does not have transcode capability");

    // Loads and initializes the implementation
    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session supporting transcode");

    // Initiate bs_dec_in for decode input stream and parse basic stream information (input width and height)
    //   in : session, source
    //   out: bs_dec_in, stream_info
    sts = PrepareDecodeVariables(session, source, &bs_dec_in, &stream_info);
    VERIFY(MFX_ERR_NONE == sts, "Not able to prepare decode variables");

    // Prepare bitstream for encode output and encode params
    //   in : stream_info.mfx.FrameInfo.Width, stream_info.mfx.FrameInfo.Height
    //   out: bs_enc_out, enc_params
    sts = PrepareEncodeVariables(stream_info.mfx.FrameInfo.Width,
                                 stream_info.mfx.FrameInfo.Height,
                                 &bs_enc_out,
                                 &enc_params);
    VERIFY(MFX_ERR_NONE == sts, "Not able to prepare encode variables");

    // Initialize the encoder
    sts = MFXVideoENCODE_Init(session, &enc_params);
    VERIFY(MFX_ERR_NONE == sts, "Could not initialize Encode");

    printf("Transcoding %s -> %s\n", in_filename, OUTPUT_FILE);

    while (is_stillgoing == true) {
        sts = MFX_ERR_NONE;

        // Read input stream for decode
        if (is_draining_dec == false) {
            sts = ReadEncodedStream(bs_dec_in, source);
            if (sts != MFX_ERR_NONE) // No more data to read, start decode draining mode
                is_draining_dec = true;
        }

        // Decode MJPEG stream
        if (is_draining_enc == false) {
            sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                                  (is_draining_dec == true) ? NULL : &bs_dec_in,
                                                  NULL,
                                                  &dec_surface_out,
                                                  &syncp);
        }

        switch (sts) {
            case MFX_ERR_NONE: // Got 1 decoded frame
                break;
            case MFX_ERR_MORE_DATA: // The function requires more bitstream at input before decoding can proceed
                if (is_draining_dec == true)
                    is_draining_enc =
                        true; // No more data to drain from decoder, start encode draining mode
                else
                    continue; // read more data
                break;
            default:
                printf("unknown status %d\n", sts);
                is_stillgoing = false;
                continue;
        }

        // Encode to H265 stream
        sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                              NULL,
                                              (is_draining_enc == true) ? NULL : dec_surface_out,
                                              &bs_enc_out,
                                              &syncp);

        if (is_draining_enc == false && dec_surface_out) {
            sts_r = dec_surface_out->FrameInterface->Release(dec_surface_out);
            VERIFY(MFX_ERR_NONE == sts_r, "mfxFrameSurfaceInterface->Release failed");
        }

        switch (sts) {
            case MFX_ERR_NONE:
                // MFX_ERR_NONE and syncp indicate output is available
                if (syncp) {
                    // Encode output is not available on CPU until sync operation completes
                    sts = MFXVideoCORE_SyncOperation(session, syncp, WAIT_100_MILLSECONDS);
                    VERIFY(MFX_ERR_NONE == sts, "MFXVideoCORE_SyncOperation error");
                    WriteEncodedStream(bs_enc_out, sink);
                    framenum++;
                }
                break;
            case MFX_ERR_MORE_DATA: // The function requires more data to generate any output
                if (is_draining_enc == true)
                    is_stillgoing = false; // No more data to drain from encoder, exit loop
                else
                    continue;
                break;
            default:
                printf("unknown status %d\n", sts);
                is_stillgoing = false;
                break;
        }
    }

end:
    printf("Transcoded %d frames\n", framenum);

    // Clean up resources - It is recommended to close components first, before
    // releasing allocated surfaces, since some surfaces may still be locked by
    // internal resources.
    if (loader)
        MFXUnload(loader);

    if (bs_enc_out.Data)
        free(bs_enc_out.Data);

    if (bs_dec_in.Data)
        free(bs_dec_in.Data);

    if (source)
        fclose(source);

    if (sink)
        fclose(sink);

    return 0;
}

// Read encoded stream from file
mfxStatus ReadEncodedStream(mfxBitstream &bs, FILE *f) {
    memmove(bs.Data, bs.Data + bs.DataOffset, bs.DataLength);
    bs.DataOffset = 0;
    bs.DataLength += (mfxU32)fread(bs.Data + bs.DataLength, 1, bs.MaxLength - bs.DataLength, f);
    if (bs.DataLength == 0)
        return MFX_ERR_MORE_DATA;

    return MFX_ERR_NONE;
}

// Write encoded stream to file
void WriteEncodedStream(mfxBitstream &bs, FILE *f) {
    fwrite(bs.Data + bs.DataOffset, 1, bs.DataLength, f);
    bs.DataLength = 0;
    return;
}

char *ValidateFileName(char *in) {
    if (in) {
        if (strlen(in) > MAX_PATH)
            return NULL;
    }

    return in;
}

mfxStatus CheckPipelineCapability(mfxLoader loader) {
    mfxStatus sts         = MFX_ERR_NONE;
    mfxConfig cfg         = NULL;
    mfxVariant impl_value = {};

    cfg = MFXCreateConfig(loader);
    if (cfg == NULL) {
        printf("MFXCreateConfig failed");
        return MFX_ERR_ABORTED;
    }

    // Check JPEG DECODE capability
    impl_value.Type     = MFX_VARIANT_TYPE_U32;
    impl_value.Data.U32 = MFX_CODEC_JPEG;
    sts                 = MFXSetConfigFilterProperty(
        cfg,
        (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        impl_value);
    if (sts != MFX_ERR_NONE) {
        printf("[DECODE] MFXSetConfigFilterProperty failed");
        return sts;
    }

    // Check H265 ENCODE capability
    impl_value.Type     = MFX_VARIANT_TYPE_U32;
    impl_value.Data.U32 = MFX_CODEC_HEVC;
    sts                 = MFXSetConfigFilterProperty(
        cfg,
        (mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID",
        impl_value);
    if (sts != MFX_ERR_NONE) {
        printf("[ENCODE] MFXSetConfigFilterProperty failed");
        return sts;
    }

    return sts;
}

mfxStatus PrepareDecodeVariables(mfxSession session,
                                 FILE *f,
                                 mfxBitstream *bs,
                                 mfxVideoParam *param) {
    mfxStatus sts = MFX_ERR_NONE;

    // Prepare decode input bitstream
    bs->MaxLength = BITSTREAM_BUFFER_SIZE;
    bs->Data      = (mfxU8 *)malloc(bs->MaxLength * sizeof(mfxU8));
    if (bs->Data == NULL) {
        printf("Not able to allocate decode input stream buffer\n");
        return MFX_ERR_MEMORY_ALLOC;
    }

    memset(bs->Data, 0, bs->MaxLength * sizeof(mfxU8));
    bs->CodecId = MFX_CODEC_JPEG;

    // Decode few frames to get the basic stream information
    // Width and height of input stream will be used for encode parameter setting
    ReadEncodedStream(*bs, f);

    param->mfx.CodecId = MFX_CODEC_JPEG;
    sts                = MFXVideoDECODE_DecodeHeader(session, bs, param);
    if (sts != MFX_ERR_NONE) {
        printf("MFXDecodeHeader failed\n");
        return sts;
    }

    return sts;
}

mfxStatus PrepareEncodeVariables(mfxU16 width,
                                 mfxU16 height,
                                 mfxBitstream *bs,
                                 mfxVideoParam *param) {
    // Prepare encode output bitstream
    bs->MaxLength = BITSTREAM_BUFFER_SIZE;
    bs->Data      = (mfxU8 *)malloc(bs->MaxLength * sizeof(mfxU8));
    if (bs->Data == NULL) {
        printf("Not able to allocate encode output stream buffer\n");
        return MFX_ERR_MEMORY_ALLOC;
    }

    memset(bs->Data, 0, bs->MaxLength * sizeof(mfxU8));

    param->mfx.CodecId                 = MFX_CODEC_HEVC;
    param->mfx.TargetUsage             = MFX_TARGETUSAGE_BALANCED;
    param->mfx.TargetKbps              = TARGETKBPS;
    param->mfx.RateControlMethod       = MFX_RATECONTROL_VBR;
    param->mfx.FrameInfo.FrameRateExtN = FRAMERATE;
    param->mfx.FrameInfo.FrameRateExtD = 1;
    param->mfx.FrameInfo.FourCC        = MFX_FOURCC_I420;
    param->mfx.FrameInfo.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    param->mfx.FrameInfo.CropW         = width;
    param->mfx.FrameInfo.CropH         = height;
    param->mfx.FrameInfo.Width         = ALIGN16(width);
    param->mfx.FrameInfo.Height        = ALIGN16(height);

    param->IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    return MFX_ERR_NONE;
}
