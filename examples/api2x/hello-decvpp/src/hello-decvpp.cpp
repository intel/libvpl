//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal Intel® Video Processing Library (Intel® VPL) decode and VPP application
/// using Intel® VPL 2.2 API features including internal memory. For more information see:
/// https://intel.github.io/libvpl
///
/// @file

#include "util.hpp"

#define DEC_OUTPUT_FILE            "dec_out.raw"
#define VPP1_OUTPUT_FILE           "vpp_640x480_out.raw"
#define VPP2_OUTPUT_FILE           "vpp_128x96_out.raw"
#define BITSTREAM_BUFFER_SIZE      2000000
#define SYNC_TIMEOUT               100
#define MAJOR_API_VERSION_REQUIRED 2
#define MINOR_API_VERSION_REQUIRED 2

void Usage(void) {
    printf("\n");
    printf("   Usage  :  hello-decvpp \n");
    printf("     -i             input file name (HEVC elementary stream)\n\n");
    printf("   Example:  hello-decvpp -i in.h265\n");
    printf(
        "   To view:  ffplay -f rawvideo -pixel_format yuv420p -video_size [width]x[height] %s\n\n",
        DEC_OUTPUT_FILE);
    printf("             ffplay -f rawvideo -pixel_format yuv420p -video_size 640x480 %s\n\n",
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
    bool isDraining                       = false;
    bool isStillGoing                     = true;
    bool isFailed                         = false;
    FILE *sinkDec                         = NULL; // for decoded frames
    FILE *sinkVPP1                        = NULL; // for decoded output -> 640x480 i420 vpp frames
    FILE *sinkVPP2                        = NULL; // for decoded output -> 128x96 bgra vpp frames
    FILE *source                          = NULL;
    mfxBitstream bitstream                = {};
    mfxSession session                    = NULL;
    mfxStatus sts                         = MFX_ERR_NONE;
    mfxSurfaceArray *outSurfaces          = nullptr;
    mfxFrameSurface1 *aSurf               = nullptr;
    mfxU32 framenum                       = 0;
    mfxU32 numVPPCh                       = 0;
    mfxVideoChannelParam **mfxVPPChParams = nullptr;
    Params cliParams                      = {};
    mfxVideoParam mfxDecParams            = {};

    //variables used only in 2.x version
    mfxConfig cfg[3];
    mfxVariant cfgVal[3];
    mfxLoader loader = NULL;

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

    // Initialize session
    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed -- is implementation in path?");

    // Implementation used must be the type requested from command line
    cfg[0] = MFXCreateConfig(loader);
    VERIFY(NULL != cfg[0], "MFXCreateConfig failed")
    cfgVal[0].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[0].Data.U32 = MFX_IMPL_TYPE_HARDWARE;
    sts = MFXSetConfigFilterProperty(cfg[0], (mfxU8 *)"mfxImplDescription.Impl", cfgVal[0]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for Impl");

    // Implementation must provide an HEVC decoder
    cfg[1] = MFXCreateConfig(loader);
    VERIFY(NULL != cfg[1], "MFXCreateConfig failed")
    cfgVal[1].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[1].Data.U32 = MFX_CODEC_HEVC;
    sts                = MFXSetConfigFilterProperty(
        cfg[1],
        (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        cfgVal[1]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for decoder CodecID");

    // Implementation used must provide API version 2.2 or newer
    cfg[2] = MFXCreateConfig(loader);
    VERIFY(NULL != cfg[2], "MFXCreateConfig failed")
    cfgVal[2].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[2].Data.U32 = VPLVERSION(MAJOR_API_VERSION_REQUIRED, MINOR_API_VERSION_REQUIRED);
    sts                = MFXSetConfigFilterProperty(cfg[2],
                                     (mfxU8 *)"mfxImplDescription.ApiVersion.Version",
                                     cfgVal[2]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for API version");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts,
           "Cannot create session -- no implementations meet selection criteria");

    // Print info about implementation loaded
    ShowImplementationInfo(loader, 0);

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
            isFailed = true;
            goto end;
            break;
    }

    if (0 == mfxDecParams.mfx.FrameInfo.FrameRateExtN &&
        0 == mfxDecParams.mfx.FrameInfo.FrameRateExtD) {
        mfxDecParams.mfx.FrameInfo.FrameRateExtN = 30;
        mfxDecParams.mfx.FrameInfo.FrameRateExtD = 1;
    }

    numVPPCh       = 2;
    mfxVPPChParams = new mfxVideoChannelParam *[numVPPCh];
    for (mfxU32 i = 0; i < numVPPCh; i++) {
        mfxVPPChParams[i] = new mfxVideoChannelParam;
        memset(mfxVPPChParams[i], 0, sizeof(mfxVideoChannelParam));
    }

    // scaled output to 640x480
    mfxVPPChParams[0]->VPP.FourCC        = mfxDecParams.mfx.FrameInfo.FourCC;
    mfxVPPChParams[0]->VPP.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxVPPChParams[0]->VPP.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    mfxVPPChParams[0]->VPP.FrameRateExtN = mfxDecParams.mfx.FrameInfo.FrameRateExtN;
    mfxVPPChParams[0]->VPP.FrameRateExtD = mfxDecParams.mfx.FrameInfo.FrameRateExtD;
    mfxVPPChParams[0]->VPP.CropW         = 640;
    mfxVPPChParams[0]->VPP.CropH         = 480;
    mfxVPPChParams[0]->VPP.Width         = ALIGN16(mfxVPPChParams[0]->VPP.CropW);
    mfxVPPChParams[0]->VPP.Height        = ALIGN16(mfxVPPChParams[0]->VPP.CropH);
    mfxVPPChParams[0]->VPP.ChannelId     = 1;
    mfxVPPChParams[0]->Protected         = 0;
    mfxVPPChParams[0]->IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    mfxVPPChParams[0]->ExtParam  = NULL;
    mfxVPPChParams[0]->NumExtParam = 0;

    // scaled output to 128x96 and color conversion to bgra
    mfxVPPChParams[1]->VPP.FourCC        = MFX_FOURCC_BGRA;
    mfxVPPChParams[1]->VPP.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxVPPChParams[1]->VPP.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    mfxVPPChParams[1]->VPP.FrameRateExtN = mfxDecParams.mfx.FrameInfo.FrameRateExtN;
    mfxVPPChParams[1]->VPP.FrameRateExtD = mfxDecParams.mfx.FrameInfo.FrameRateExtD;
    mfxVPPChParams[1]->VPP.CropW         = 128;
    mfxVPPChParams[1]->VPP.CropH         = 96;
    mfxVPPChParams[1]->VPP.Width         = ALIGN16(mfxVPPChParams[1]->VPP.CropW);
    mfxVPPChParams[1]->VPP.Height        = ALIGN16(mfxVPPChParams[1]->VPP.CropH);
    mfxVPPChParams[1]->VPP.ChannelId     = 2;
    mfxVPPChParams[1]->Protected         = 0;
    mfxVPPChParams[1]->IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    mfxVPPChParams[1]->ExtParam  = NULL;
    mfxVPPChParams[1]->NumExtParam = 0;

    // mfxVPPChParams[0] for vpp output (640x480), mfxVPPChParams[1] for vpp output (128x96)
    sts = MFXVideoDECODE_VPP_Init(session, &mfxDecParams, mfxVPPChParams, numVPPCh);
    VERIFY(MFX_ERR_NONE == sts, "Error initializing decodevpp\n");

    printf("Decoding and VPP %s -> %s and %s, %s\n",
           cliParams.infileName,
           DEC_OUTPUT_FILE,
           VPP1_OUTPUT_FILE,
           VPP2_OUTPUT_FILE);

    // output frames will be delivered in outSurfaces->Surfaces[]
    // outSurfaces->Surfaces[0]    : decode output
    // outSurfaces->Surfaces[1],[2]: vpp output
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
                if (outSurfaces == nullptr) {
                    printf("ERROR - empty array of surfaces.\n");
                    isStillGoing = false;
                    continue;
                }

                for (mfxU32 i = 0; i < outSurfaces->NumSurfaces; i++) {
                    aSurf = outSurfaces->Surfaces[i];
                    do {
                        sts = aSurf->FrameInterface->Synchronize(aSurf, SYNC_TIMEOUT);
                        VERIFY(MFX_ERR_NONE == sts || MFX_WRN_IN_EXECUTION == sts,
                               "ERROR - FrameInterface->Synchronizee failed");
                        if (sts == MFX_ERR_NONE) {
                            if (aSurf->Info.ChannelId == 0) { // decoder output
                                sts = WriteRawFrame_InternalMem(aSurf, sinkDec);
                                VERIFY(MFX_ERR_NONE == sts,
                                       "ERROR - Could not write decode output");
                            }
                            else { // VPP filter output
                                sts = WriteRawFrame_InternalMem(aSurf,
                                                                (i == 1) ? sinkVPP1 : sinkVPP2);
                                VERIFY(MFX_ERR_NONE == sts, "ERROR - Could not write vpp output");
                            }
                        }
                        if (sts != MFX_WRN_IN_EXECUTION) {
                            sts = aSurf->FrameInterface->Release(aSurf);
                            VERIFY(MFX_ERR_NONE == sts, "Could not release output surface");
                        }

                    } while (sts == MFX_WRN_IN_EXECUTION);
                }
                framenum++;

                sts = outSurfaces->Release(outSurfaces);
                VERIFY(MFX_ERR_NONE == sts, "ERROR - mfxSurfaceArray->Release failed");
                outSurfaces = nullptr;

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

    MFXVideoDECODE_VPP_Close(session);
    MFXClose(session);

    // Clean up resources - It is recommended to close components first, before
    // releasing allocated surfaces, since some surfaces may still be locked by
    // internal resources.
    if (mfxVPPChParams) {
        for (mfxU32 i = 0; i < numVPPCh; i++) {
            if (mfxVPPChParams[i])
                delete mfxVPPChParams[i];
        }
        delete[] mfxVPPChParams;
    }

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

    if (loader)
        MFXUnload(loader);

    if (isFailed) {
        return -1;
    }
    else {
        return 0;
    }
}
