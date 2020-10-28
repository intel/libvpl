//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) pipeline (VPP+Enc) application,
/// using oneVPL external memory management for VPP in and Vpp out/Enc in
///
/// @file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo.h"

#define MAX_PATH              260
#define MAX_WIDTH             3840
#define MAX_HEIGHT            2160
#define OUTPUT_WIDTH          640
#define OUTPUT_HEIGHT         480
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

mfxStatus LoadRawFrame(mfxFrameSurface1 *surface, FILE *f);
void WriteEncodedStream(mfxBitstream &bs, FILE *f);
char *ValidateFileName(char *in);
mfxU16 ValidateSize(char *in, mfxU16 max);
mfxU32 GetSurfaceSize(mfxU32 fourcc, mfxU16 width, mfxU16 height);

void Usage(void) {
    printf("\n");
    printf("   Usage  :  vpl-vppenc InputI420File width height\n\n");
    printf("             InputI420File    ... input file name (i420 raw frames)\n");
    printf("             width            ... input width\n");
    printf("             height           ... input height\n\n");
    printf("   Example:  vpl-vppenc in.i420 128 96\n");
    printf("   To view:  ffplay %s\n\n", OUTPUT_FILE);
    printf(
        " * Resize I420 raw frames to %dx%d size and encode it to HEVC/H265 elementary stream in %s\n\n",
        OUTPUT_WIDTH,
        OUTPUT_HEIGHT,
        OUTPUT_FILE);
    return;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        Usage();
        return 1;
    }

    char *in_filename                   = NULL;
    FILE *source                        = NULL;
    FILE *sink                          = NULL;
    mfxStatus sts                       = MFX_ERR_NONE;
    mfxLoader loader                    = NULL;
    mfxConfig cfg                       = NULL;
    mfxVariant impl_value               = { 0 };
    mfxSession session                  = NULL;
    mfxU16 input_width                  = 0;
    mfxU16 input_height                 = 0;
    mfxU16 input_width_aligned          = 0;
    mfxU16 input_height_aligned         = 0;
    mfxU16 out_width_aligned            = 0;
    mfxU16 out_height_aligned           = 0;
    mfxVideoParam vpp_params            = { 0 };
    mfxFrameAllocRequest vpp_request[2] = { 0 };
    mfxU16 num_surfaces_vppin           = 0;
    mfxU16 num_surfaces_vppout_enc      = 0;
    mfxU32 surface_size                 = 0;
    std::vector<mfxU8> data_vppin;
    std::vector<mfxU8> data_vppout_enc;
    mfxU8 *psurf_data = NULL;
    std::vector<mfxFrameSurface1> surfaces_vppin;
    std::vector<mfxFrameSurface1> surfaces_vppout_enc;
    mfxBitstream bitstream           = { 0 };
    mfxVideoParam encode_params      = { 0 };
    mfxFrameAllocRequest enc_request = { 0 };
    int available_surface_index_in   = 0;
    int available_surface_index_out  = 0;
    mfxSyncPoint syncp               = { 0 };
    mfxU32 framenum                  = 0;
    bool b_run_encoder               = false;
    bool is_draining_vpp             = false;
    bool is_draining_enc             = false;
    bool is_stillgoing               = true;
    mfxU16 i;

    // Setup input and output files
    in_filename = ValidateFileName(argv[1]);
    VERIFY(in_filename, "Input filename is not valid");

    source = fopen(in_filename, "rb");
    VERIFY(source, "Could not open input file");

    sink = fopen(OUTPUT_FILE, "wb");
    VERIFY(sink, "Could not create output file");

    input_width = ValidateSize(argv[2], MAX_WIDTH);
    VERIFY(input_width, "Input width is not valid");

    input_height = ValidateSize(argv[3], MAX_HEIGHT);
    VERIFY(input_height, "Input height is not valid");

    // Initialize VPL session for video processing + encode
    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed");

    // Configuration for video processing
    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "MFXCreateConfig failed")

    impl_value.Type     = MFX_VARIANT_TYPE_U32;
    impl_value.Data.U32 = MFX_EXTBUFF_VPP_SCALING;
    sts                 = MFXSetConfigFilterProperty(
        cfg,
        reinterpret_cast<const mfxU8 *>("mfxImplDescription.mfxVPPDescription.filter.FilterFourCC"),
        impl_value);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    // Configuration for encode
    impl_value.Type     = MFX_VARIANT_TYPE_U32;
    impl_value.Data.U32 = MFX_CODEC_HEVC;
    sts                 = MFXSetConfigFilterProperty(
        cfg,
        reinterpret_cast<const mfxU8 *>("mfxImplDescription.mfxEncoderDescription.encoder.CodecID"),
        impl_value);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    // Create session
    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session supporting VPP");

    // -------------------------
    // Initialize VPP parameters
    // Input data
    vpp_params.vpp.In.FourCC        = MFX_FOURCC_I420;
    vpp_params.vpp.In.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    vpp_params.vpp.In.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    vpp_params.vpp.In.FrameRateExtN = FRAMERATE;
    vpp_params.vpp.In.FrameRateExtD = 1;
    vpp_params.vpp.In.CropW         = input_width;
    vpp_params.vpp.In.CropH         = input_height;
    vpp_params.vpp.In.Width         = ALIGN16(input_width);
    vpp_params.vpp.In.Height        = ALIGN16(input_height);
    // Output data - change output size to OUTPUT_WIDTH, OUTPUT_HEIGHT
    vpp_params.vpp.Out.FourCC        = MFX_FOURCC_I420;
    vpp_params.vpp.Out.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    vpp_params.vpp.Out.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    vpp_params.vpp.Out.FrameRateExtN = FRAMERATE;
    vpp_params.vpp.Out.FrameRateExtD = 1;
    vpp_params.vpp.Out.CropW         = OUTPUT_WIDTH;
    vpp_params.vpp.Out.CropH         = OUTPUT_HEIGHT;
    vpp_params.vpp.Out.Width         = ALIGN16(OUTPUT_WIDTH);
    vpp_params.vpp.Out.Height        = ALIGN16(OUTPUT_HEIGHT);

    vpp_params.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    // ----------------------------
    // Initialize encode parameters
    encode_params.mfx.CodecId                 = MFX_CODEC_HEVC;
    encode_params.mfx.TargetUsage             = MFX_TARGETUSAGE_BALANCED;
    encode_params.mfx.TargetKbps              = TARGETKBPS;
    encode_params.mfx.RateControlMethod       = MFX_RATECONTROL_VBR;
    encode_params.mfx.FrameInfo.FrameRateExtN = FRAMERATE;
    encode_params.mfx.FrameInfo.FrameRateExtD = 1;
    encode_params.mfx.FrameInfo.FourCC        = MFX_FOURCC_I420;
    encode_params.mfx.FrameInfo.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    encode_params.mfx.FrameInfo.CropW         = OUTPUT_WIDTH;
    encode_params.mfx.FrameInfo.CropH         = OUTPUT_HEIGHT;
    encode_params.mfx.FrameInfo.Width         = ALIGN16(OUTPUT_WIDTH);
    encode_params.mfx.FrameInfo.Height        = ALIGN16(OUTPUT_HEIGHT);

    encode_params.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    // Query number of required surfaces for VPP
    sts = MFXVideoVPP_QueryIOSurf(session, &vpp_params, vpp_request);
    VERIFY(MFX_ERR_NONE == sts, "QueryIOSurf error");

    // Query number of required surfaces for encode
    sts = MFXVideoENCODE_QueryIOSurf(session, &encode_params, &enc_request);
    VERIFY(MFX_ERR_NONE == sts, "QueryIOSurf error");

    num_surfaces_vppin      = vpp_request[0].NumFrameSuggested;
    num_surfaces_vppout_enc = vpp_request[1].NumFrameSuggested + enc_request.NumFrameSuggested;

    // Allocate surfaces for VPP in
    // Frame surface array keeps pointers to all surface planes and general
    // frame info
    input_width_aligned  = vpp_params.vpp.In.Width;
    input_height_aligned = vpp_params.vpp.In.Height;

    surface_size = GetSurfaceSize(MFX_FOURCC_I420, input_width_aligned, input_height_aligned);
    VERIFY(surface_size, "VPP input surface size is wrong");

    data_vppin.resize((size_t)surface_size * num_surfaces_vppin);
    psurf_data = data_vppin.data();
    surfaces_vppin.resize(num_surfaces_vppin);

    for (i = 0; i < num_surfaces_vppin; i++) {
        memset(&surfaces_vppin[i], 0, sizeof(mfxFrameSurface1));
        surfaces_vppin[i].Info   = vpp_params.vpp.In;
        surfaces_vppin[i].Data.Y = &psurf_data[surface_size * i];
        surfaces_vppin[i].Data.U =
            surfaces_vppin[i].Data.Y + (size_t)input_width_aligned * input_height_aligned;
        surfaces_vppin[i].Data.V = surfaces_vppin[i].Data.U +
                                   (((size_t)input_width_aligned / 2) * (input_height_aligned / 2));
        surfaces_vppin[i].Data.Pitch = input_width_aligned;
    }

    // Allocate surfaces for VPP out
    // Frame surface array keeps pointers to all surface planes and general
    // frame info
    out_width_aligned  = vpp_params.vpp.Out.Width;
    out_height_aligned = vpp_params.vpp.Out.Height;

    surface_size = GetSurfaceSize(MFX_FOURCC_I420, out_width_aligned, out_height_aligned);
    VERIFY(surface_size, "VPP out surface size is wrong");

    data_vppout_enc.resize((size_t)surface_size * num_surfaces_vppout_enc);
    psurf_data = data_vppout_enc.data();
    surfaces_vppout_enc.resize(num_surfaces_vppout_enc);

    for (i = 0; i < num_surfaces_vppout_enc; i++) {
        memset(&surfaces_vppout_enc[i], 0, sizeof(mfxFrameSurface1));
        surfaces_vppout_enc[i].Info   = vpp_params.vpp.Out;
        surfaces_vppout_enc[i].Data.Y = &psurf_data[surface_size * i];
        surfaces_vppout_enc[i].Data.U =
            surfaces_vppout_enc[i].Data.Y + (size_t)out_width_aligned * out_height_aligned;
        surfaces_vppout_enc[i].Data.V =
            surfaces_vppout_enc[i].Data.U +
            (((size_t)out_width_aligned / 2) * (out_height_aligned / 2));
        surfaces_vppout_enc[i].Data.Pitch = out_width_aligned;
    }

    // Initialize VPP and start processing
    sts = MFXVideoVPP_Init(session, &vpp_params);
    VERIFY(MFX_ERR_NONE == sts, "Could not initialize VPP");

    // Initialize the encoder
    sts = MFXVideoENCODE_Init(session, &encode_params);
    VERIFY(MFX_ERR_NONE == sts, "Encode init failed");

    // Prepare output bitstream and start encoding
    bitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
    bitstream.Data      = reinterpret_cast<mfxU8 *>(malloc(bitstream.MaxLength * sizeof(mfxU8)));

    printf("Processing %s -> %s\n", in_filename, OUTPUT_FILE);

    while (is_stillgoing == true) {
        // Load a new frame if not vpp draining mode
        if (is_draining_vpp == false) {
            available_surface_index_in = -1;
            for (mfxU16 i = 0; i < surfaces_vppin.size(); i++) {
                if (!surfaces_vppin[i].Data.Locked) {
                    available_surface_index_in = i;
                }
            }
            VERIFY(available_surface_index_in >= 0, "No available surface for VPP in");

            sts = LoadRawFrame(&surfaces_vppin[available_surface_index_in], source);
            // no more data to read, time to drain vpp buffers
            if (sts != MFX_ERR_NONE)
                is_draining_vpp = true;
        }

        // VPP - scaling
        // If encode is not in draining mode, we need vpp out from RunFrameVPPAsync()
        // otherwise, we skip this and run EncodeFrameAsync() with null input.
        if (is_draining_enc == false) {
            // Find free frame surface for VPP out and encode

            available_surface_index_out = -1;
            for (mfxU16 i = 0; i < surfaces_vppout_enc.size(); i++) {
                if (!surfaces_vppout_enc[i].Data.Locked) {
                    available_surface_index_out = i;
                }
            }
            VERIFY(available_surface_index_out >= 0, "No available surface for VPP out and encode");

            sts = MFXVideoVPP_RunFrameVPPAsync(
                session,
                (is_draining_vpp == true) ? NULL : &surfaces_vppin[available_surface_index_in],
                &surfaces_vppout_enc[available_surface_index_out],
                NULL,
                &syncp);

            switch (sts) {
                case MFX_ERR_NONE:
                    b_run_encoder = true;
                    break;
                case MFX_ERR_MORE_DATA:
                    // Drained all frames from VPP buffer and it's time to drain encode buffer
                    if (is_draining_vpp == true)
                        is_draining_enc = true;
                    break;
                default:
                    printf("unknown status %d\n", sts);
                    b_run_encoder = false;
                    is_stillgoing = false;
                    break;
            }
        }

        // Encode - HEVC
        if (b_run_encoder == true) {
            sts = MFXVideoENCODE_EncodeFrameAsync(
                session,
                NULL,
                (is_draining_enc == true) ? NULL
                                          : &surfaces_vppout_enc[available_surface_index_out],
                &bitstream,
                &syncp);

            switch (sts) {
                case MFX_ERR_NONE:
                    // MFX_ERR_NONE and syncp_enc indicate output is available
                    if (syncp) {
                        // Encode output is not available on CPU until sync operation completes
                        sts = MFXVideoCORE_SyncOperation(session, syncp, WAIT_100_MILLSECONDS);
                        VERIFY(MFX_ERR_NONE == sts, "MFXVideoCORE_SyncOperation error");
                        WriteEncodedStream(bitstream, sink);
                        framenum++;
                    }
                    break;
                case MFX_ERR_MORE_DATA:
                    // The function requires more data to generate any output
                    if (is_draining_enc == true)
                        is_stillgoing = false;
                    break;
                default:
                    printf("unknown status %d\n", sts);
                    is_stillgoing = false;
                    break;
            }
        }
    }

end:
    printf("Processed %d frames\n", framenum);

    // Clean up resources - It is recommended to close components first, before
    // releasing allocated surfaces, since some surfaces may still be locked by
    // internal resources.
    if (loader)
        MFXUnload(loader);

    if (bitstream.Data)
        free(bitstream.Data);

    if (source)
        fclose(source);

    if (sink)
        fclose(sink);

    return 0;
}

// Load raw I420 frames to mfxFrameSurface
mfxStatus LoadRawFrame(mfxFrameSurface1 *surface, FILE *f) {
    mfxU16 w, h, i, pitch;
    mfxU32 bytes;
    mfxU8 *ptr;
    mfxFrameInfo *info = &surface->Info;
    mfxFrameData *data = &surface->Data;

    w = info->Width;
    h = info->Height;

    switch (info->FourCC) {
        case MFX_FOURCC_I420:
            // read luminance plane (Y)
            pitch = data->Pitch;
            ptr   = data->Y;
            for (i = 0; i < h; i++) {
                bytes = (mfxU32)fread(ptr + i * pitch, 1, w, f);
                if (w != bytes)
                    return MFX_ERR_MORE_DATA;
            }

            // read chrominance (U, V)
            pitch /= 2;
            h /= 2;
            w /= 2;
            ptr = data->U;
            for (i = 0; i < h; i++) {
                bytes = (mfxU32)fread(ptr + i * pitch, 1, w, f);
                if (w != bytes)
                    return MFX_ERR_MORE_DATA;
            }

            ptr = data->V;
            for (i = 0; i < h; i++) {
                bytes = (mfxU32)fread(ptr + i * pitch, 1, w, f);
                if (w != bytes)
                    return MFX_ERR_MORE_DATA;
            }
            break;
        default:
            printf("Unsupported FourCC code, skip LoadRawFrame\n");
            break;
    }

    return MFX_ERR_NONE;
}

// Write encoded stream to file
void WriteEncodedStream(mfxBitstream &bs, FILE *f) {
    fwrite(bs.Data + bs.DataOffset, 1, bs.DataLength, f);
    bs.DataLength = 0;
    return;
}

// Return the surface size in bytes given format and dimensions
mfxU32 GetSurfaceSize(mfxU32 fourcc, mfxU16 width, mfxU16 height) {
    mfxU32 bytes = 0;

    switch (fourcc) {
        case MFX_FOURCC_I420:
            bytes = width * height + (width >> 1) * (height >> 1) + (width >> 1) * (height >> 1);
            break;
        default:
            break;
    }

    return bytes;
}

char *ValidateFileName(char *in) {
    if (in) {
        if (strlen(in) > MAX_PATH)
            return NULL;
    }

    return in;
}

mfxU16 ValidateSize(char *in, mfxU16 max) {
    mfxU16 isize = (mfxU16)strtol(in, NULL, 10);
    if (isize <= 0 || isize > max) {
        return 0;
    }

    return isize;
}
