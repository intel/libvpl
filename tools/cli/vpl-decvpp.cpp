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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vpl/mfxjpeg.h>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo.h"

#define MAX_PATH   260
#define MAX_WIDTH  3840
#define MAX_HEIGHT 2160
#define FRAMERATE  30

#define WAIT_100_MILLSECONDS  100
#define BITSTREAM_BUFFER_SIZE 20000000

#define VERIFY(x, y)       \
    if (!(x)) {            \
        printf("%s\n", y); \
        return_code = -1;  \
        goto end;          \
    }

typedef struct {
    mfxU32 width;
    mfxU32 height;
    mfxU32 framerate_numerator;
    mfxU32 framerate_denominator;

} AV1EncConfig;

AV1EncConfig *g_conf = NULL;
mfxU32 repeatCount   = 0;
bool g_read_streamheader;

mfxStatus ReadStreamInfo(mfxSession session, FILE *f, mfxBitstream *bs, mfxVideoParam *param);
mfxStatus ReadEncodedStream(mfxBitstream &bs, mfxU32 codecid, FILE *f, mfxU32 repeat);
void WriteRawFrame(mfxFrameSurface1 *surface, FILE *f);
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height);
int strignorecasecmp(const char *str1, const char *str2);
mfxU32 GetCodecId(char *codec);
char *ValidateFileName(char *in);
char *ValidateCodec(char *in);
char *ValidateFourCC(char *in);
char *ValidateDigits(char *in);
bool ValidateSize(char *in, mfxU16 *vsize, mfxU32 vmax);

void Usage(char *arguments[]) {
    //std::string someString(charString);
    printf("\n");
    printf(
        "   Usage  :  vpl-decvpp.exe InputFile OutputFile InputFormat OutputFormat OutputWidth OutputHeight \n\n");
    printf("   Example:  vpl-decvpp.exe cars_128x96.h265 out_300x300.i420 h265 i420 300 300\n");
    printf(
        "   To view:  ffplay -video_size [OutputWidth]x[OutputHeight] -pixel_format [pixel format] -f rawvideo [OutputFile]\n\n");
    return;
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        Usage(argv);
        return 1;
    }

    int return_code                     = 0;
    char *in_filename                   = NULL;
    char *out_filename                  = NULL;
    char *in_codec                      = NULL;
    char *out_fourcc                    = NULL;
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
    mfxU16 out_height                   = 0;
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

    out_filename = ValidateFileName(argv[2]);
    VERIFY(out_filename, "Output filename is not valid");

    in_codec = ValidateCodec(argv[3]);
    VERIFY(in_codec, "Input codec is not valid");

    out_fourcc = ValidateFourCC(argv[4]);
    VERIFY(out_fourcc, "Output FourCC is not valid");

    if (!ValidateSize(argv[5], &out_width, MAX_WIDTH))
        VERIFY(out_width, "out_width is not valid");

    if (!ValidateSize(argv[6], &out_height, MAX_HEIGHT))
        VERIFY(out_height, "out_height is not valid");

    source = fopen(in_filename, "rb");
    VERIFY(source, "Could not open input file");

    sink = fopen(out_filename, "wb");
    VERIFY(sink, "Could not create output file");

    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed");

    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "MFXCreateConfig failed")
    impl_value.Type     = MFX_VARIANT_TYPE_U32;
    impl_value.Data.U32 = GetCodecId(in_codec);
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
    bitstream.CodecId = GetCodecId(in_codec);

    mfxVideoParam mfxDecParams;
    memset(&mfxDecParams, 0, sizeof(mfxDecParams));

    mfxDecParams.mfx.CodecId = GetCodecId(in_codec);
    mfxDecParams.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    sts = ReadStreamInfo(session, source, &bitstream, &mfxDecParams);
    VERIFY(MFX_ERR_NONE == sts, "MFXDecodeHeader failed");

    input_width              = mfxDecParams.mfx.FrameInfo.Width;
    input_height             = mfxDecParams.mfx.FrameInfo.Height;
    vpp_params.vpp.In.FourCC = mfxDecParams.mfx.FrameInfo.FourCC;

    vpp_params.vpp.In.ChromaFormat  = mfxDecParams.mfx.FrameInfo.ChromaFormat;
    vpp_params.vpp.In.Width         = input_width;
    vpp_params.vpp.In.Height        = input_height;
    vpp_params.vpp.In.CropW         = input_width;
    vpp_params.vpp.In.CropH         = input_height;
    vpp_params.vpp.In.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    vpp_params.vpp.In.FrameRateExtN = FRAMERATE;
    vpp_params.vpp.In.FrameRateExtD = 1;

    // Output data
    if (strignorecasecmp(out_fourcc, "i420") == 0) {
        vpp_params.vpp.Out.FourCC       = MFX_FOURCC_I420;
        vpp_params.vpp.Out.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    }

    if (strignorecasecmp(out_fourcc, "i010") == 0) {
        vpp_params.vpp.Out.FourCC       = MFX_FOURCC_I010;
        vpp_params.vpp.Out.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    }

    if (strignorecasecmp(out_fourcc, "bgra") == 0) {
        vpp_params.vpp.Out.FourCC       = MFX_FOURCC_RGB4;
        vpp_params.vpp.Out.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    }

    vpp_params.vpp.Out.Width         = out_width;
    vpp_params.vpp.Out.Height        = out_height;
    vpp_params.vpp.Out.CropW         = out_width;
    vpp_params.vpp.Out.CropH         = out_height;
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
    //out_width  = vpp_params.vpp.Out.Width;
    //out_height = vpp_params.vpp.Out.Height;

    surface_size = GetSurfaceSize(vpp_params.vpp.Out.FourCC, out_width, out_height);
    VERIFY(surface_size, "VPP-out surface size is wrong");

    vpp_data_out = reinterpret_cast<mfxU8 *>(calloc(num_surfaces_out, surface_size));
    VERIFY(vpp_data_out, "Could not allocate buffer for VPP output frames");

    vpp_surfaces_out =
        reinterpret_cast<mfxFrameSurface1 *>(calloc(num_surfaces_out, sizeof(mfxFrameSurface1)));
    VERIFY(vpp_surfaces_out, "Could not allocate VPP output surfaces");

    if (vpp_params.vpp.Out.FourCC == MFX_FOURCC_I420 ||
        vpp_params.vpp.Out.FourCC == MFX_FOURCC_I010) {
        mfxU16 surfW = (vpp_params.vpp.Out.FourCC == MFX_FOURCC_I010) ? out_width * 2 : out_width;
        mfxU16 surfH = out_height;

        for (i = 0; i < num_surfaces_out; i++) {
            vpp_surfaces_out[i].Info   = vpp_params.vpp.Out;
            vpp_surfaces_out[i].Data.Y = &vpp_data_out[surface_size * i];
            vpp_surfaces_out[i].Data.U = vpp_surfaces_out[i].Data.Y + surfW * surfH;
            vpp_surfaces_out[i].Data.V = vpp_surfaces_out[i].Data.U + ((surfW / 2) * (surfH / 2));
            vpp_surfaces_out[i].Data.Pitch = surfW;
        }
    }
    else // bgra
    {
        mfxU16 surfW = out_width * 4;

        for (i = 0; i < num_surfaces_out; i++) {
            vpp_surfaces_out[i].Info       = vpp_params.vpp.Out;
            vpp_surfaces_out[i].Data.B     = &vpp_data_out[surface_size * i];
            vpp_surfaces_out[i].Data.G     = vpp_surfaces_out[i].Data.B + 1;
            vpp_surfaces_out[i].Data.R     = vpp_surfaces_out[i].Data.G + 1;
            vpp_surfaces_out[i].Data.A     = vpp_surfaces_out[i].Data.R + 1;
            vpp_surfaces_out[i].Data.Pitch = surfW;
        }
    }

    // Initialize VPP and start processing
    sts = MFXVideoVPP_Init(session, &vpp_params);
    VERIFY(MFX_ERR_NONE == sts, "Could not initialize VPP");

    printf("Decoding %s -> %s\n", in_filename, out_filename);
    while (is_stillgoing) {
        if (is_draining_dec == false) {
            sts = ReadEncodedStream(bitstream, bitstream.CodecId, source, 0);
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

    return return_code;
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

void WriteRawFrame(mfxFrameSurface1 *pSurface, FILE *f) {
    mfxU16 w, h, i, pitch;
    mfxFrameInfo *pInfo = &pSurface->Info;
    mfxFrameData *pData = &pSurface->Data;

    w = pInfo->Width;
    h = pInfo->Height;

    // write the output to disk
    switch (pInfo->FourCC) {
        case MFX_FOURCC_I420:
            //Y
            pitch = pData->Pitch;
            for (i = 0; i < h; i++) {
                fwrite(pData->Y + i * pitch, 1, w, f);
            }

            //U
            pitch /= 2;
            h /= 2;
            w /= 2;
            for (i = 0; i < h; i++) {
                fwrite(pData->U + i * pitch, 1, w, f);
            }
            //V
            for (i = 0; i < h; i++) {
                fwrite(pData->V + i * pitch, 1, w, f);
            }
            break;

        case MFX_FOURCC_I010:
            //Y
            pitch = pData->Pitch;
            w *= 2;
            for (i = 0; i < h; i++) {
                fwrite(pSurface->Data.Y + i * pitch, 1, w, f);
            }

            //U
            pitch /= 2;
            w /= 2;
            h /= 2;
            for (i = 0; i < h; i++) {
                fwrite(pSurface->Data.U + i * pitch, 1, w, f);
            }

            //V
            for (i = 0; i < h; i++) {
                fwrite(pSurface->Data.V + i * pitch, 1, w, f);
            }

            break;

        case MFX_FOURCC_RGB4:
            pitch = pData->Pitch;
            w *= 4;
            for (i = 0; i < h; i++) {
                fwrite(pSurface->Data.B + i * pitch, 1, w, f);
            }
            break;
        default:
            printf("Unsupported FourCC code, skip WriteRawFrame\n");
            break;
    }

    return;
}

mfxU32 GetCodecId(char *codec) {
    mfxU32 fourCC = 0;

    if (strignorecasecmp(codec, "h265") == 0) {
        fourCC = MFX_CODEC_HEVC;
    }
    else if (strignorecasecmp(codec, "av1") == 0) {
        fourCC = MFX_CODEC_AV1;
    }
    else if (strignorecasecmp(codec, "h264") == 0) {
        fourCC = MFX_CODEC_AVC;
    }
    else if (strignorecasecmp(codec, "jpeg") == 0) {
        fourCC = MFX_CODEC_JPEG;
    }

    return fourCC;
}

// Return the surface size in bytes given format and dimensions
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height) {
    mfxU32 nbytes = 0;

    switch (FourCC) {
        case MFX_FOURCC_I420:
            nbytes = width * height + (width >> 1) * (height >> 1) + (width >> 1) * (height >> 1);
            break;

        case MFX_FOURCC_I010:
            nbytes = width * height + (width >> 1) * (height >> 1) + (width >> 1) * (height >> 1);
            nbytes *= 2;
            break;

        case MFX_FOURCC_RGB4:
            nbytes = width * height * 4;
        default:
            break;
    }

    return nbytes;
}

int strignorecasecmp(const char *str1, const char *str2) {
    const unsigned char *u_str1 = (const unsigned char *)str1;
    const unsigned char *u_str2 = (const unsigned char *)str2;

    while (tolower(*u_str1) == tolower(*u_str2++)) {
        if (*u_str1++ == '\0') {
            return 0;
        }
    }

    return 1;
}

char *ValidateFileName(char *in) {
    if (in) {
        if (strlen(in) > MAX_PATH)
            return NULL;
    }

    return in;
}

char *ValidateCodec(char *in) {
    if (in) {
        if (strlen(in) > 7)
            return NULL;

        if (strignorecasecmp(in, "h265") == 0 || strignorecasecmp(in, "av1") == 0 ||
            strignorecasecmp(in, "h264") == 0 || strignorecasecmp(in, "jpeg") == 0) {
            return in;
        }
    }

    return NULL;
}

char *ValidateFourCC(char *in) {
    if (in) {
        if (strlen(in) != 4)
            return NULL;

        if (strignorecasecmp(in, "i420") == 0 || strignorecasecmp(in, "i010") == 0 ||
            strignorecasecmp(in, "bgra") == 0) {
            return in;
        }
    }

    return NULL;
}

char *ValidateDigits(char *in) {
    if (in) {
        for (int i = 0; i < strlen(in); i++) {
            if (!isdigit(in[i]))
                return NULL;
        }
    }

    return in;
}

bool ValidateSize(char *in, mfxU16 *vsize, mfxU32 vmax) {
    if (in) {
        *vsize = static_cast<mfxU16>(strtol(in, NULL, 10));
        if (*vsize <= vmax)
            return true;
    }

    *vsize = 0;
    return false;
}

mfxStatus ReadEncodedStream(mfxBitstream &bs, mfxU32 codecid, FILE *f, mfxU32 repeat) {
    memmove(bs.Data, bs.Data + bs.DataOffset, bs.DataLength);
    bs.DataOffset = 0;

    if (codecid == MFX_CODEC_AV1) {
        // spec for IVF headers
        // https://wiki.multimedia.cx/index.php/IVF

        // extract AV1 elementary stream from IVF file.
        // should remove stream header and frame header.
        // and use the frame size information to read AV1 frame.
        mfxU32 nBytesInFrame = 0;
        mfxU64 nTimeStamp    = 0;
        mfxU32 nBytesRead    = 0;

        // read stream header, only once at the beginning, 32 bytes
        if (g_read_streamheader == false) {
            mfxU8 header[32] = { 0 };
            nBytesRead       = (mfxU32)fread(header, 1, 32, f);
            if (nBytesRead == 0)
                return MFX_ERR_MORE_DATA;
            g_read_streamheader = true;
        }

        bs.DataLength = 0;

        // read frame header and parse frame data
        while (!feof(f)) {
            nBytesRead = (mfxU32)fread(&nBytesInFrame, 1, 4, f);
            if (nBytesInFrame == 0 || nBytesInFrame > bs.MaxLength)
                return MFX_ERR_ABORTED;
            if (nBytesRead == 0)
                break;

            // check whether buffer is over if we read this frame or not
            if (bs.DataLength + nBytesInFrame > bs.MaxLength) {
                // set file pointer location back to -4
                // so, the access pointer for next frame will be the "bytesinframe" location and break
                fseek(f, -4, SEEK_CUR);
                break;
            }

            nBytesRead = (mfxU32)fread(&nTimeStamp, 1, 8, f);
            if (nBytesRead == 0)
                return MFX_ERR_MORE_DATA;

            nBytesRead = (mfxU32)fread(bs.Data + bs.DataLength, 1, nBytesInFrame, f);
            if (nBytesRead == 0)
                return MFX_ERR_MORE_DATA;

            bs.DataLength += nBytesRead;
        }
    }
    else {
        bs.DataLength +=
            static_cast<mfxU32>(fread(bs.Data + bs.DataLength, 1, bs.MaxLength - bs.DataLength, f));
        if (bs.DataLength == 0)
            printf("%d\n", bs.DataLength);

        while (feof(f) && repeat > 0 && repeatCount <= repeat) {
            if (repeatCount == repeat)
                return MFX_ERR_NONE;

            // The end-of-file and error internal indicators associated to the stream are cleared after a successful call to this function,
            // and all effects from previous calls to ungetc on this stream are dropped.
            rewind(f);

            repeatCount++;
        }
    }

    if (bs.DataLength == 0)
        return MFX_ERR_MORE_DATA;

    return MFX_ERR_NONE;
}

mfxStatus ReadStreamInfo(mfxSession session, FILE *f, mfxBitstream *bs, mfxVideoParam *param) {
    mfxStatus sts = MFX_ERR_NONE;
    // Decode few frames to get the basic stream information
    // Width and height of input stream will be set to vpp in
    ReadEncodedStream(*bs, param->mfx.CodecId, f, 0);
    sts = MFXVideoDECODE_DecodeHeader(session, bs, param);
    if (sts != MFX_ERR_NONE) {
        printf("MFXDecodeHeader failed\n");
        return sts;
    }
    else {
        bs->DataLength = 0;
        rewind(f);
        if (param->mfx.CodecId == MFX_CODEC_AV1)
            g_read_streamheader = false;
    }

    //only MFX_CHROMAFORMAT_YUV420 in I420 and I010 colorspaces allowed
    switch (param->mfx.FrameInfo.FourCC) {
        case MFX_FOURCC_I420:
            if (param->mfx.FrameInfo.BitDepthLuma && (param->mfx.FrameInfo.BitDepthLuma != 8)) {
                printf("Unsupported Luma Bit Depth for I420\n");
                return MFX_ERR_INVALID_VIDEO_PARAM;
            }

            if (param->mfx.FrameInfo.ChromaFormat &&
                (param->mfx.FrameInfo.ChromaFormat != MFX_CHROMAFORMAT_YUV420)) {
                printf("Unsupported chroma format for I420\n");
                return MFX_ERR_INVALID_VIDEO_PARAM;
            }

            break;
        case MFX_FOURCC_I010:
            if (param->mfx.FrameInfo.BitDepthLuma && (param->mfx.FrameInfo.BitDepthLuma != 10)) {
                printf("Unsupported Luma Bit Depth for I010\n");
                return MFX_ERR_INVALID_VIDEO_PARAM;
            }

            if (param->mfx.FrameInfo.ChromaFormat &&
                (param->mfx.FrameInfo.ChromaFormat != MFX_CHROMAFORMAT_YUV420)) {
                printf("Unsupported chroma format for I010\n");
                return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            break;
        default:
            printf("Unsupported FourCC\n");
            return MFX_ERR_UNSUPPORTED;
    }

    return sts;
}
