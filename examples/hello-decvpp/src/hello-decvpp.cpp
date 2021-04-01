//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) decode and VPP application,
/// using 2.x API with internal memory management
///
/// @file

#include "util/util.h"

#define DEC_OUTPUT_FILE       "dec_out.raw"
#define VPP1_OUTPUT_FILE      "vpp_320x240_out.raw"
#define VPP2_OUTPUT_FILE      "vpp_128x96_out.raw"
#define BITSTREAM_BUFFER_SIZE 2000000

void Usage(void) {
    printf("\n");
    printf("   Usage  :  hello-decvpp \n");
    printf("     -sw/-hw        use software or hardware implementation\n");
    printf("     -i             input file name (HEVC elementary stream)\n\n");
    printf("   Example:  hello-decvpp -i in.h265\n");
    printf(
        "   To view:  ffplay -f rawvideo -pixel_format yuv420p -video_size [width]x[height] %s\n\n",
        DEC_OUTPUT_FILE);
    printf("             ffplay -f rawvideo -pixel_format yuv420p -video_size 320x240 %s\n\n",
           VPP1_OUTPUT_FILE);
    printf("             ffplay -f rawvideo -pixel_format bgra -video_size 128x96 %s\n\n",
           VPP2_OUTPUT_FILE);
    printf(
        " * Decode HEVC/H265 elementary stream to raw frames in %s and vpp in %s (native fourcc) and %s (bgra)\n\n",
        DEC_OUTPUT_FILE,
        VPP1_OUTPUT_FILE,
        VPP2_OUTPUT_FILE);
    printf("   CPU native color format is I420/yuv420p.  GPU native color format is NV12\n");
    return;
}

int main(int argc, char *argv[]) {
    //Variables used for legacy and 2.x
    bool isDraining                      = false;
    bool isStillGoing                    = true;
    FILE *sinkDec                        = NULL; // for decoded frames
    FILE *sinkVPP1                       = NULL; // for decoded output -> 320x240 i420 vpp frames
    FILE *sinkVPP2                       = NULL; // for decoded output -> 128x96 bgra vpp frames
    FILE *source                         = NULL;
    mfxBitstream bitstream               = {};
    mfxSession session                   = NULL;
    mfxStatus sts                        = MFX_ERR_NONE;
    mfxSurfaceArray *outSurfaces         = nullptr;
    mfxU32 framenum                      = 0;
    mfxU32 numVPPCh                      = 0;
    mfxVideoChannelParam *mfxVPPChParams = nullptr;
    Params cliParams                     = {};
    void *accelHandle                    = NULL;
    mfxVideoParam mfxDecParams           = {};
    mfxVersion version                   = { 0, 1 };

    //variables used only in 2.x version
    mfxConfig cfg      = NULL;
    mfxLoader loader   = NULL;
    mfxVariant inCodec = {};

    //Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_DECODE) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    source = fopen(cliParams.infileName, "rb");
    VERIFY(source, "Could not open input file");

    sinkDec = fopen(DEC_OUTPUT_FILE, "wb");
    VERIFY(sinkDec, "Could not create decode output file");

    sinkVPP1 = fopen(VPP1_OUTPUT_FILE, "wb");
    VERIFY(sinkVPP1, "Could not create vpp1 output file");

    sinkVPP2 = fopen(VPP2_OUTPUT_FILE, "wb");
    VERIFY(sinkVPP2, "Could not create vpp2 output file");

    // Initialize VPL session for any implementation of HEVC/H265 decode
    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed -- is implementation in path?");

    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "MFXCreateConfig failed")

    // Implementation used must be the type requested from command line
    sts = MFXSetConfigFilterProperty(cfg, (mfxU8 *)"mfxImplDescription.Impl", cliParams.implValue);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for Impl");

    // Implementation must provide an HEVC decoder
    inCodec.Type     = MFX_VARIANT_TYPE_U32;
    inCodec.Data.U32 = MFX_CODEC_HEVC;
    sts              = MFXSetConfigFilterProperty(
        cfg,
        (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        inCodec);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for decoder CodecID");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session");

    // Print info about implementation loaded
    version = ShowImplInfo(session);
    VERIFY(version.Major > 1, "Sample requires 2.x API implementation, exiting");

    // Convenience function to initialize available accelerator(s)
    accelHandle = InitAcceleratorHandle(session);

    // Prepare input bitstream and start decoding
    bitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
    bitstream.Data      = (mfxU8 *)calloc(bitstream.MaxLength, sizeof(mfxU8));
    VERIFY(bitstream.Data, "Not able to allocate input buffer");
    bitstream.CodecId = MFX_CODEC_HEVC;

    //Pre-parse input stream
    sts = ReadEncodedStream(bitstream, source);
    VERIFY(MFX_ERR_NONE == sts, "Error reading bitstream\n");

    mfxDecParams.mfx.CodecId = MFX_CODEC_HEVC;
    mfxDecParams.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    sts                      = MFXVideoDECODE_DecodeHeader(session, &bitstream, &mfxDecParams);
    VERIFY(MFX_ERR_NONE == sts, "Error decoding header\n");

    printf("Output colorspace: ");
    switch (mfxDecParams.mfx.FrameInfo.FourCC) {
        case MFX_FOURCC_I420: // CPU output
            printf("I420 (aka yuv420p)\n");
            break;
        case MFX_FOURCC_NV12: // GPU output
            printf("NV12\n");
            break;
        default:
            printf("Unsupported color format\n");
            goto end;
            break;
    }

    numVPPCh       = 2;
    mfxVPPChParams = new mfxVideoChannelParam[numVPPCh];
    for (mfxU32 i = 0; i < numVPPCh; i++) {
        mfxVPPChParams[i] = {};
    }

    // scaled output to 320x240
    mfxVPPChParams[0].VPP.FourCC        = mfxDecParams.mfx.FrameInfo.FourCC;
    mfxVPPChParams[0].VPP.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxVPPChParams[0].VPP.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    mfxVPPChParams[0].VPP.FrameRateExtN = 30;
    mfxVPPChParams[0].VPP.FrameRateExtD = 1;
    mfxVPPChParams[0].VPP.CropW         = 320;
    mfxVPPChParams[0].VPP.CropH         = 240;
    mfxVPPChParams[0].VPP.Width         = ALIGN16(mfxVPPChParams[0].VPP.CropW);
    mfxVPPChParams[0].VPP.Height        = ALIGN16(mfxVPPChParams[0].VPP.CropH);
    mfxVPPChParams[0].VPP.ChannelId     = 1;
    mfxVPPChParams[0].Protected         = 0;
    mfxVPPChParams[0].IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    mfxVPPChParams[0].ExtParam  = NULL;
    mfxVPPChParams[0].NumExtParam = 0;

    // scaled output to 128x96 and color conversion to bgra
    mfxVPPChParams[1].VPP.FourCC        = MFX_FOURCC_BGRA;
    mfxVPPChParams[1].VPP.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxVPPChParams[1].VPP.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    mfxVPPChParams[1].VPP.FrameRateExtN = 30;
    mfxVPPChParams[1].VPP.FrameRateExtD = 1;
    mfxVPPChParams[1].VPP.CropW         = 128;
    mfxVPPChParams[1].VPP.CropH         = 96;
    mfxVPPChParams[1].VPP.Width         = ALIGN16(mfxVPPChParams[1].VPP.CropW);
    mfxVPPChParams[1].VPP.Height        = ALIGN16(mfxVPPChParams[1].VPP.CropH);
    mfxVPPChParams[1].VPP.ChannelId     = 2;
    mfxVPPChParams[1].Protected         = 0;
    mfxVPPChParams[1].IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    mfxVPPChParams[1].ExtParam  = NULL;
    mfxVPPChParams[1].NumExtParam = 0;

    // mfxVPPChParams[0] for vpp output (320x240), mfxVPPChParams[1] for vpp output (128x96)
    sts = MFXVideoDECODE_VPP_Init(session, &mfxDecParams, &mfxVPPChParams, numVPPCh);

    printf("Decoding and VPP %s -> %s and %s, %s\n",
           cliParams.infileName,
           DEC_OUTPUT_FILE,
           VPP1_OUTPUT_FILE,
           VPP2_OUTPUT_FILE);

    // output frames will be delivered in outSurfaces->Surfaces[]
    // outSurfaces->Surfaces[0]    : decode output
    // outSurfaces->Surfaces[1],[2]: vpp output
    outSurfaces = new mfxSurfaceArray;

    while (isStillGoing == true) {
        // Load encoded stream if not draining
        if (isDraining == false) {
            sts = ReadEncodedStream(bitstream, source);
            if (sts != MFX_ERR_NONE)
                isDraining = true;
        }

        sts = MFXVideoDECODE_VPP_DecodeFrameAsync(session,
                                                  (isDraining) ? NULL : &bitstream,
                                                  NULL,
                                                  0,
                                                  &outSurfaces);

        switch (sts) {
            case MFX_ERR_NONE:
                // decode output
                sts = WriteRawFrame_InternalMem(outSurfaces->Surfaces[0], sinkDec);
                VERIFY(MFX_ERR_NONE == sts, "Could not write decode output");

                // vpp1 output
                sts = WriteRawFrame_InternalMem(outSurfaces->Surfaces[1], sinkVPP1);
                VERIFY(MFX_ERR_NONE == sts, "Could not write 1st vpp output");

                // vpp2 output
                sts = WriteRawFrame_InternalMem(outSurfaces->Surfaces[2], sinkVPP2);
                VERIFY(MFX_ERR_NONE == sts, "Could not write 2nd vpp output");

                framenum++;
                break;
            case MFX_ERR_MORE_DATA:
                // The function requires more bitstream at input before decoding can proceed
                if (isDraining)
                    isStillGoing = false;
                break;
            case MFX_ERR_MORE_SURFACE:
                // The function requires more frame surface at output before decoding can proceed.
                // This applies to external memory allocations and should not be expected for
                // a simple internal allocation case like this
                break;
            case MFX_ERR_DEVICE_LOST:
                // For non-CPU implementations,
                // Cleanup if device is lost
                break;
            case MFX_WRN_DEVICE_BUSY:
                // For non-CPU implementations,
                // Wait a few milliseconds then try again
                break;
            case MFX_WRN_VIDEO_PARAM_CHANGED:
                // The decoder detected a new sequence header in the bitstream.
                // Video parameters may have changed.
                // In external memory allocation case, might need to reallocate the output surface
                break;
            case MFX_ERR_INCOMPATIBLE_VIDEO_PARAM:
                // The function detected that video parameters provided by the application
                // are incompatible with initialization parameters.
                // The application should close the component and then reinitialize it
                break;
            case MFX_ERR_REALLOC_SURFACE:
                // Bigger surface_work required. May be returned only if
                // mfxInfoMFX::EnableReallocRequest was set to ON during initialization.
                // This applies to external memory allocations and should not be expected for
                // a simple internal allocation case like this
                break;
            default:
                printf("unknown status %d\n", sts);
                isStillGoing = false;
                break;
        }
    }

end:
    printf("Decode and VPP processed %d frames\n", framenum);

    // Clean up resources - It is recommended to close components first, before
    // releasing allocated surfaces, since some surfaces may still be locked by
    // internal resources.
    if (mfxVPPChParams)
        delete[] mfxVPPChParams;

    if (outSurfaces)
        delete outSurfaces;

    if (source)
        fclose(source);

    if (sinkDec)
        fclose(sinkDec);

    if (sinkVPP1)
        fclose(sinkVPP1);

    if (sinkVPP2)
        fclose(sinkVPP2);

    if (bitstream.Data)
        free(bitstream.Data);

    if (accelHandle)
        FreeAcceleratorHandle(accelHandle);

    if (loader)
        MFXUnload(loader);
    return 0;
}
