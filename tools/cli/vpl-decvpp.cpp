//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) decode+vpp application,
/// using oneVPL internal memory management
///
/// @file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo.h"

#define MAX_PATH      260
#define MAX_WIDTH     3840
#define MAX_HEIGHT    2160
#define OUTPUT_WIDTH  640
#define OUTPUT_HEIGHT 480
#define FRAMERATE     30
#define OUTPUT_FILE   "out.i420"

#define ALIGN16(value) (((value + 15) >> 4) << 4)

#define WAIT_100_MILLSECONDS  100
#define BITSTREAM_BUFFER_SIZE 20000000

#define VERIFY(x, y)       \
    if (!(x)) {            \
        printf("%s\n", y); \
        goto end;          \
    }

mfxStatus ReadEncodedStream(mfxBitstream &bs, FILE *f);
void WriteRawFrame(mfxFrameSurface1 *surface, FILE *f);
mfxU32 GetSurfaceSize(mfxU32 fourcc, mfxU16 width, mfxU16 height);
char *ValidateFileName(char *in);

void Usage(void) {
    printf("\n");
    printf("   Usage  :  vpl-decvpp InputH265File\n\n");
    printf("             InputH265File    ... input file name (HEVC/H265 elementary stream)\n");
    printf("   Example:  vpl-decvpp in.h265\n");
    printf("   To view:  ffplay -f rawvideo -pixel_format yuv420p -video_size %dx%d %s\n\n",
           OUTPUT_WIDTH,
           OUTPUT_HEIGHT,
           OUTPUT_FILE);
    printf(" * Decode+Resize H265 elementary stream to I420 raw frames with %dx%d size in %s\n\n",
           OUTPUT_WIDTH,
           OUTPUT_HEIGHT,
           OUTPUT_FILE);
    return;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
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
    mfxBitstream bitstream              = { 0 };
    mfxFrameSurface1 *dec_surface_out   = NULL;
    mfxFrameSurface1 *vpp_surfaces_out  = NULL;
    mfxSyncPoint syncp                  = { 0 };
    mfxVideoParam vpp_params            = { 0 };
    mfxFrameAllocRequest vpp_request[2] = { 0 };
    mfxU16 num_surfaces_in              = 0;
    mfxU16 num_surfaces_out             = 0;
    mfxU8 *vpp_data_out                 = NULL;
    mfxU16 out_width                    = 0;
    mfxU32 out_height                   = 0;
    mfxU16 input_width                  = 0;
    mfxU16 input_height                 = 0;
    mfxU32 surface_size                 = 0;
    bool is_draining_dec                = false;
    bool is_draining_vpp                = false;
    bool is_stillgoing                  = true;
    mfxU32 framenum                     = 0;
    mfxU16 i;
    int available_surface_index = 0;

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

    impl_value.Type     = MFX_VARIANT_TYPE_U32;
    impl_value.Data.U32 = MFX_EXTBUFF_VPP_SCALING;
    sts                 = MFXSetConfigFilterProperty(
        cfg,
        reinterpret_cast<const mfxU8 *>("mfxImplDescription.mfxVPPDescription.filter.FilterFourCC"),
        impl_value);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session supporting decode+VPP");

    // Prepare input bitstream and start decoding
    bitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
    bitstream.Data      = reinterpret_cast<mfxU8 *>(malloc(bitstream.MaxLength * sizeof(mfxU8)));
    VERIFY(bitstream.Data, "Not able to allocate input buffer");
    memset(bitstream.Data, 0, bitstream.MaxLength * sizeof(mfxU8));
    bitstream.CodecId = MFX_CODEC_HEVC;
    ReadEncodedStream(bitstream, source);

    mfxVideoParam mfxDecParams;
    memset(&mfxDecParams, 0, sizeof(mfxDecParams));

    mfxDecParams.mfx.CodecId = MFX_CODEC_HEVC;
    sts                      = MFXVideoDECODE_DecodeHeader(session, &bitstream, &mfxDecParams);
    VERIFY(MFX_ERR_NONE == sts, "MFXDecodeHeader failed");

    input_width  = mfxDecParams.mfx.FrameInfo.Width;
    input_height = mfxDecParams.mfx.FrameInfo.Height;

    vpp_params.vpp.In.FourCC        = MFX_FOURCC_I420;
    vpp_params.vpp.In.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    vpp_params.vpp.In.Width         = ALIGN16(input_width);
    vpp_params.vpp.In.Height        = ALIGN16(input_height);
    vpp_params.vpp.In.CropW         = input_width;
    vpp_params.vpp.In.CropH         = input_height;
    vpp_params.vpp.In.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    vpp_params.vpp.In.FrameRateExtN = FRAMERATE;
    vpp_params.vpp.In.FrameRateExtD = 1;

    // Output data
    vpp_params.vpp.Out.FourCC        = MFX_FOURCC_I420;
    vpp_params.vpp.Out.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    vpp_params.vpp.Out.Width         = ALIGN16(OUTPUT_WIDTH);
    vpp_params.vpp.Out.Height        = ALIGN16(OUTPUT_HEIGHT);
    vpp_params.vpp.Out.CropW         = OUTPUT_WIDTH;
    vpp_params.vpp.Out.CropH         = OUTPUT_HEIGHT;
    vpp_params.vpp.Out.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    vpp_params.vpp.Out.FrameRateExtN = FRAMERATE;
    vpp_params.vpp.Out.FrameRateExtD = 1;

    vpp_params.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    sts = MFXVideoVPP_QueryIOSurf(session, &vpp_params, vpp_request);
    VERIFY(MFX_ERR_NONE == sts, "QueryIOSurf error");

    num_surfaces_in  = vpp_request[0].NumFrameSuggested;
    num_surfaces_out = vpp_request[1].NumFrameSuggested;
    VERIFY(num_surfaces_out, "Sugested output surface count is zero");

    // Allocate surfaces for VPP out
    // Frame surface array keeps pointers to all surface planes and general
    // frame info
    out_width  = vpp_params.vpp.Out.Width;
    out_height = vpp_params.vpp.Out.Height;

    surface_size = GetSurfaceSize(MFX_FOURCC_I420, out_width, out_height);
    VERIFY(surface_size, "VPP-out surface size is wrong");

    vpp_data_out = reinterpret_cast<mfxU8 *>(calloc(num_surfaces_out, surface_size));
    VERIFY(vpp_data_out, "Could not allocate buffer for VPP output frames");

    vpp_surfaces_out =
        reinterpret_cast<mfxFrameSurface1 *>(calloc(num_surfaces_out, sizeof(mfxFrameSurface1)));
    VERIFY(vpp_surfaces_out, "Could not allocate VPP output surfaces");

    for (i = 0; i < num_surfaces_out; i++) {
        vpp_surfaces_out[i].Info   = vpp_params.vpp.Out;
        vpp_surfaces_out[i].Data.Y = &vpp_data_out[surface_size * i];
        vpp_surfaces_out[i].Data.U = vpp_surfaces_out[i].Data.Y + out_width * out_height;
        vpp_surfaces_out[i].Data.V =
            vpp_surfaces_out[i].Data.U + ((out_width / 2) * (out_height / 2));
        vpp_surfaces_out[i].Data.Pitch = out_width;
    }

    // Initialize VPP and start processing
    sts = MFXVideoVPP_Init(session, &vpp_params);
    VERIFY(MFX_ERR_NONE == sts, "Could not initialize VPP");

    printf("Decoding %s -> %s\n", in_filename, OUTPUT_FILE);
    while (is_stillgoing) {
        if (is_draining_dec == false) {
            sts = ReadEncodedStream(bitstream, source);
            if (sts != MFX_ERR_NONE)
                is_draining_dec = true;
        }

        if (!is_draining_vpp) {
            sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                                  (is_draining_dec) ? NULL : &bitstream,
                                                  NULL,
                                                  &dec_surface_out,
                                                  &syncp);
        }
        else {
            sts = MFX_ERR_NONE;
        }

        switch (sts) {
            case MFX_ERR_NONE:
                // Find free frame surface for VPP out
                available_surface_index = -1;
                for (int i = 0; i < num_surfaces_out; i++) {
                    if (!vpp_surfaces_out[i].Data.Locked) {
                        available_surface_index = i;
                        break;
                    }
                }
                VERIFY(available_surface_index >= 0, "Could not find available output surface");

                sts = MFXVideoVPP_RunFrameVPPAsync(session,
                                                   (is_draining_vpp) ? NULL : dec_surface_out,
                                                   &vpp_surfaces_out[available_surface_index],
                                                   NULL,
                                                   &syncp);

                if (sts == MFX_ERR_NONE) {
                    sts = MFXVideoCORE_SyncOperation(session, syncp, WAIT_100_MILLSECONDS);
                    VERIFY(MFX_ERR_NONE == sts, "MFXVideoCORE_SyncOperation error");
                    WriteRawFrame(&vpp_surfaces_out[available_surface_index], sink);
                    framenum++;
                }
                else if (sts == MFX_ERR_MORE_DATA) {
                    if (is_draining_vpp == true)
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
                    is_draining_vpp = true;
                break;
            default:
                is_stillgoing = false;
                break;
        }
    }
end:
    printf("Decoded+processed %d frames\n", framenum);

    if (loader)
        MFXUnload(loader);

    if (bitstream.Data)
        free(bitstream.Data);

    if (vpp_surfaces_out) {
        free(vpp_surfaces_out);
    }

    if (vpp_data_out)
        free(vpp_data_out);

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

// Write raw I420 frame to file
void WriteRawFrame(mfxFrameSurface1 *surface, FILE *f) {
    mfxU16 w, h, i, pitch;
    mfxFrameInfo *info = &surface->Info;
    mfxFrameData *data = &surface->Data;

    w = info->Width;
    h = info->Height;

    switch (info->FourCC) {
        case MFX_FOURCC_I420:
            // write luminance plane (Y)
            pitch = data->Pitch;
            for (i = 0; i < h; i++) {
                fwrite(data->Y + i * pitch, 1, w, f);
            }

            // write chrominance (U, V)
            pitch /= 2;
            h /= 2;
            w /= 2;
            for (i = 0; i < h; i++) {
                fwrite(data->U + i * pitch, 1, w, f);
            }

            for (i = 0; i < h; i++) {
                fwrite(data->V + i * pitch, 1, w, f);
            }
            break;
        default:
            printf("Unsupported FourCC code, skip WriteRawFrame\n");
            break;
    }

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
