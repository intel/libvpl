//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) decode+encode application,
/// using oneVPL internal memory management
///
/// @file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo.h"

#define MAX_PATH              260
#define MAX_WIDTH             3840
#define MAX_HEIGHT            2160
#define TARGETKBPS            4000
#define FRAMERATE             30
#define OUTPUT_FILE           "out.h265"
#define WAIT_100_MILLSECONDS  100
#define BITSTREAM_BUFFER_SIZE 20000000

#define VERIFY(x, y)       \
    if (!(x)) {            \
        printf("%s\n", y); \
        goto end;          \
    }

#define ALIGN16(value) (((value + 15) >> 4) << 4)

mfxStatus ReadEncodedStream(mfxBitstream &bs, FILE *f);
void WriteEncodedStream(mfxBitstream &bs, FILE *f);
char *ValidateFileName(char *in);

void Usage(void) {
    printf("\n");
    printf("   Usage  :  vpl-decenc InputH265File\n\n");
    printf("             InputH265File    ... input file name (HEVC/H265 elementary stream)\n");
    printf("   Example:  vpl-decenc in.h265\n");
    printf("   To view:  ffplay %s\n\n", OUTPUT_FILE);
    printf(" * Decode+Encode H265 elementary stream to %s\n\n", OUTPUT_FILE);
    return;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        Usage();
        return 1;
    }

    char *in_filename                 = NULL;
    FILE *source                      = NULL;
    FILE *sink                        = NULL;
    mfxStatus sts                     = MFX_ERR_NONE;
    mfxLoader loader                  = NULL;
    mfxConfig cfg                     = NULL;
    mfxVariant impl_value             = { 0 };
    mfxSession session                = NULL;
    mfxBitstream inbitstream          = { 0 };
    mfxBitstream outbitstream         = { 0 };
    mfxFrameSurface1 *dec_surface_out = NULL;
    mfxVideoParam encode_params       = { 0 };
    mfxSyncPoint syncp                = { 0 };
    mfxU16 input_width                = 0;
    mfxU16 input_height               = 0;
    bool is_draining_dec              = false;
    bool is_draining_enc              = false;
    bool is_stillgoing                = true;
    mfxU32 framenum                   = 0;

    in_filename = ValidateFileName(argv[1]);
    VERIFY(in_filename, "Input filename is not valid");

    source = fopen(in_filename, "rb");
    VERIFY(source, "Could not open input file");

    sink = fopen(OUTPUT_FILE, "wb");
    VERIFY(sink, "Could not create output file");

    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed");

    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "MFXCreateConfig failed")
    impl_value.Type     = MFX_VARIANT_TYPE_U32;
    impl_value.Data.U32 = MFX_CODEC_HEVC;
    sts                 = MFXSetConfigFilterProperty(
        cfg,
        reinterpret_cast<const mfxU8 *>("mfxImplDescription.mfxDecoderDescription.decoder.CodecID"),
        impl_value);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    sts = MFXSetConfigFilterProperty(
        cfg,
        reinterpret_cast<const mfxU8 *>("mfxImplDescription.mfxEncoderDescription.encoder.CodecID"),
        impl_value);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session supporting decode+enocde");

    // Prepare input bitstream and start decoding
    inbitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
    inbitstream.Data = reinterpret_cast<mfxU8 *>(malloc(inbitstream.MaxLength * sizeof(mfxU8)));
    VERIFY(inbitstream.Data, "Not able to allocate input buffer");
    memset(inbitstream.Data, 0, inbitstream.MaxLength * sizeof(mfxU8));
    inbitstream.CodecId = MFX_CODEC_HEVC;
    ReadEncodedStream(inbitstream, source);

    mfxVideoParam mfxDecParams;
    memset(&mfxDecParams, 0, sizeof(mfxDecParams));

    mfxDecParams.mfx.CodecId = MFX_CODEC_HEVC;
    sts                      = MFXVideoDECODE_DecodeHeader(session, &inbitstream, &mfxDecParams);
    VERIFY(MFX_ERR_NONE == sts, "MFXDecodeHeader failed");

    input_width  = mfxDecParams.mfx.FrameInfo.Width;
    input_height = mfxDecParams.mfx.FrameInfo.Height;

    // Initialize encode parameters
    encode_params.mfx.CodecId                 = MFX_CODEC_HEVC;
    encode_params.mfx.TargetUsage             = MFX_TARGETUSAGE_BALANCED;
    encode_params.mfx.TargetKbps              = TARGETKBPS;
    encode_params.mfx.RateControlMethod       = MFX_RATECONTROL_VBR;
    encode_params.mfx.FrameInfo.FrameRateExtN = FRAMERATE;
    encode_params.mfx.FrameInfo.FrameRateExtD = 1;
    encode_params.mfx.FrameInfo.FourCC        = MFX_FOURCC_I420;
    encode_params.mfx.FrameInfo.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    encode_params.mfx.FrameInfo.CropW         = input_width;
    encode_params.mfx.FrameInfo.CropH         = input_height;
    encode_params.mfx.FrameInfo.Width         = ALIGN16(input_width);
    encode_params.mfx.FrameInfo.Height        = ALIGN16(input_height);

    encode_params.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    // Initialize the encoder
    sts = MFXVideoENCODE_Init(session, &encode_params);
    VERIFY(MFX_ERR_NONE == sts, "Encode init failed");

    // Prepare output bitstream and start encoding
    outbitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
    outbitstream.Data = reinterpret_cast<mfxU8 *>(malloc(inbitstream.MaxLength * sizeof(mfxU8)));

    printf("Decoding+Encoding %s -> %s\n", in_filename, OUTPUT_FILE);
    while (is_stillgoing) {
        if (is_draining_dec == false) {
            sts = ReadEncodedStream(inbitstream, source);
            if (sts != MFX_ERR_NONE)
                is_draining_dec = true;
        }

        if (!is_draining_enc) {
            sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                                  (is_draining_dec) ? NULL : &inbitstream,
                                                  NULL,
                                                  &dec_surface_out,
                                                  &syncp);
        }
        else {
            sts = MFX_ERR_NONE;
        }

        switch (sts) {
            case MFX_ERR_NONE:
                sts = MFXVideoENCODE_EncodeFrameAsync(
                    session,
                    NULL,
                    (is_draining_enc == true) ? NULL : dec_surface_out,
                    &outbitstream,
                    &syncp);
                if (sts == MFX_ERR_NONE) {
                    sts = MFXVideoCORE_SyncOperation(session, syncp, WAIT_100_MILLSECONDS);
                    VERIFY(MFX_ERR_NONE == sts, "MFXVideoCORE_SyncOperation error");
                    WriteEncodedStream(outbitstream, sink);
                    framenum++;
                }
                else if (sts == MFX_ERR_MORE_DATA) {
                    if (is_draining_enc == true)
                        is_stillgoing = false;
                }
                else {
                    if (sts < 0)
                        is_stillgoing = false;
                }
                break;
            case MFX_ERR_MORE_DATA:
                // The function requires more bitstream at input before decoding can proceed
                if (is_draining_dec)
                    is_draining_enc = true;
                break;
            default:
                is_stillgoing = false;
                break;
        }
    }
end:
    printf("Decoded+Encoded %d frames\n", framenum);

    if (loader)
        MFXUnload(loader);

    if (inbitstream.Data)
        free(inbitstream.Data);

    if (outbitstream.Data)
        free(outbitstream.Data);

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
