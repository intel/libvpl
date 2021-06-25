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

#include "util.hpp"

#define BITSTREAM_BUFFER_SIZE 2000000
#define SYNC_TIMEOUT          60000

void Usage(void) {
    printf("\n");
    printf("   Usage  :  decvpp_tool\n\n");
    printf("     h265/h264      set codec type to decode\n\n");
    printf("     -sw/-hw        use software or hardware implementation\n\n");
    printf("     -i             input file name (video elementary stream)\n\n");
    printf("     -o             decode out file name\n\n");
    printf("     -vpp_num       number of vpp channels\n\n");
    printf("     -vpp_params    vpp params for each vpp channel\n");
    printf("                    support scale/csc\n");
    printf("                    ',' separator for each vpp channel\n\n");
    printf("     -vpp_out       file name for each vpp out\n");
    printf("                    ',' separator for each vpp channel\n\n");
    printf("     -vmem          use video memory\n\n");
    printf("   Example: \n");
    printf(
        "     decvpp_tool h265 -i cars_128x96.h265 -o dec.yuv -sw -vpp_num 2 -vpp_params 320x240_i420,640x480_bgra -vpp_out o1.yuv,o2.yuv -o o.yuv\n\n");
    printf("     this will generate 1 decode output file and 2 vpp output files\n");
    printf("     dec.yuv  : decode output  : 128 x 96,  i420\n");
    printf("     vpp1.yuv : 1st vpp output : 320 x 240, i420\n");
    printf("     vpp2.yuv : 2st vpp output : 640 x 480, bgra\n");

    return;
}

int main(int argc, char *argv[]) {
    //Variables used for legacy and 2.x
    bool isDraining                       = false;
    bool isStillGoing                     = true;
    FILE *sinkDec                         = NULL; // for decoded frames
    FILE **sinkVPP                        = NULL; // for vpp output
    FILE *source                          = NULL;
    mfxBitstream bitstream                = {};
    mfxSession session                    = NULL;
    mfxStatus sts                         = MFX_ERR_NONE;
    mfxSurfaceArray *outSurfaces          = nullptr;
    mfxU32 framenum                       = 0;
    mfxVideoChannelParam **mfxVPPChParams = nullptr;
    Params cliParams                      = {};
    void *accelHandle                     = NULL;
    mfxVideoParam mfxDecParams            = {};
    mfxVersion version                    = { 0, 1 };
    mfxFrameSurface1 *aSurf               = nullptr;

    //variables used only in 2.x version
    mfxConfig cfg      = NULL;
    mfxLoader loader   = NULL;
    mfxVariant inCodec = {};

    //Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_DECVPP) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    source = fopen(cliParams.inFileName, "rb");
    VERIFY(source, "ERROR - Could not open input file");

    if (cliParams.decOutFileName) {
        sinkDec = fopen(cliParams.decOutFileName, "wb");
        VERIFY(sinkDec, "ERROR - Could not create decode output file");
    }
    else {
        printf(
            "WARNING - No decode output filename assigned, will skip writing decode output file\n");
    }

    if (cliParams.bIsAvailableVPPOutFileName) {
        sinkVPP = new FILE *[cliParams.vppNum];
        VERIFY(sinkVPP, "ERROR - Could not create vpp list");

        for (mfxU16 i = 0; i < cliParams.vppNum; i++) {
            sinkVPP[i] = fopen(cliParams.vppOutConfigs[i].fileName, "wb");
            VERIFY(sinkVPP[i], "ERROR - Could not create vpp output file");
        }
    }
    else {
        printf("WARNING - No VPP output filename assigned, will skip writing VPP output file\n");
    }

    // Initialize VPL session for any implementation of decode
    loader = MFXLoad();
    VERIFY(NULL != loader, "ERROR - MFXLoad failed -- is implementation in path?");

    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "ERROR - MFXCreateConfig failed")

    // Implementation used must be the type requested from command line
    sts = MFXSetConfigFilterProperty(cfg,
                                     reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                     cliParams.implValue);
    VERIFY(MFX_ERR_NONE == sts, "ERROR - MFXSetConfigFilterProperty failed for Impl");

    // Implementation must provide a decoder
    inCodec.Type     = MFX_VARIANT_TYPE_U32;
    inCodec.Data.U32 = cliParams.inCodec;
    sts              = MFXSetConfigFilterProperty(
        cfg,
        reinterpret_cast<const mfxU8 *>("mfxImplDescription.mfxDecoderDescription.decoder.CodecID"),
        inCodec);
    VERIFY(MFX_ERR_NONE == sts, "ERROR - MFXSetConfigFilterProperty failed for decoder CodecID");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts, "ERROR - Not able to create VPL session");

    // Print info about implementation loaded
    version = ShowImplInfo(session);
    VERIFY(version.Major > 1, "ERROR - Sample requires 2.x API implementation, exiting");

    // Convenience function to initialize available accelerator(s)
    sts = InitAcceleratorHandle(session);
    VERIFY(MFX_ERR_NONE == sts, "ERROR - Not able to create hw device");

    // Prepare input bitstream and start decoding
    bitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
    bitstream.Data      = reinterpret_cast<mfxU8 *>(calloc(bitstream.MaxLength, sizeof(mfxU8)));
    VERIFY(bitstream.Data, "ERROR - Not able to allocate input buffer");
    bitstream.CodecId = cliParams.inCodec;

    //Pre-parse input stream
    sts = ReadEncodedStream(bitstream, source);
    VERIFY(MFX_ERR_NONE == sts, "ERROR - Reading bitstream\n");

    mfxDecParams.mfx.CodecId = cliParams.inCodec;
    mfxDecParams.IOPattern   = (cliParams.bUseVideoMemory) ? MFX_IOPATTERN_OUT_VIDEO_MEMORY
                                                         : MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    sts = MFXVideoDECODE_DecodeHeader(session, &bitstream, &mfxDecParams);
    VERIFY(MFX_ERR_NONE == sts, "ERROR - Decoding header\n");

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

    cliParams.srcFourCC = mfxDecParams.mfx.FrameInfo.FourCC;
    cliParams.srcWidth  = mfxDecParams.mfx.FrameInfo.CropW;
    cliParams.srcHeight = mfxDecParams.mfx.FrameInfo.CropH;

    mfxVPPChParams = new mfxVideoChannelParam *[cliParams.vppNum];
    for (mfxU16 i = 0; i < cliParams.vppNum; i++) {
        mfxVPPChParams[i] = new mfxVideoChannelParam;
        memset(mfxVPPChParams[i], 0, sizeof(mfxVideoChannelParam));

        // scaled output to 320x240
        mfxVPPChParams[i]->VPP.FourCC        = cliParams.vppOutConfigs[i].fourcc;
        mfxVPPChParams[i]->VPP.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
        mfxVPPChParams[i]->VPP.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
        mfxVPPChParams[i]->VPP.FrameRateExtN = mfxDecParams.mfx.FrameInfo.FrameRateExtN;
        mfxVPPChParams[i]->VPP.FrameRateExtD = mfxDecParams.mfx.FrameInfo.FrameRateExtD;
        mfxVPPChParams[i]->VPP.CropW         = cliParams.vppOutConfigs[i].w;
        mfxVPPChParams[i]->VPP.CropH         = cliParams.vppOutConfigs[i].h;
        mfxVPPChParams[i]->VPP.Width         = ALIGN16(mfxVPPChParams[i]->VPP.CropW);
        mfxVPPChParams[i]->VPP.Height        = ALIGN16(mfxVPPChParams[i]->VPP.CropH);
        mfxVPPChParams[i]->VPP.ChannelId     = i + 1;
        mfxVPPChParams[i]->Protected         = 0;
        if (cliParams.bUseVideoMemory) {
            mfxVPPChParams[i]->IOPattern =
                MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;
        }
        else {
            mfxVPPChParams[i]->IOPattern =
                MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
        }
        mfxVPPChParams[i]->ExtParam    = NULL;
        mfxVPPChParams[i]->NumExtParam = 0;
    }

    sts = MFXVideoDECODE_VPP_Init(session, &mfxDecParams, mfxVPPChParams, cliParams.vppNum);
    VERIFY(MFX_ERR_NONE == sts, "ERROR - Initializing decodevpp\n");

    printf("Start decoding and VPP ..\n");

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
                aSurf = outSurfaces->Surfaces[0];
                sts   = aSurf->FrameInterface->Synchronize(aSurf, SYNC_TIMEOUT);
                if (sts == MFX_ERR_NONE) {
                    if (cliParams.decOutFileName) {
                        sts = WriteRawFrame_InternalMem(aSurf, sinkDec);
                        VERIFY(MFX_ERR_NONE == sts, "ERROR - Could not write decode output");
                    }
                    else {
                        sts = aSurf->FrameInterface->Release(aSurf);
                        VERIFY(MFX_ERR_NONE == sts,
                               "ERROR - mfxFrameSurfaceInterface->Release failed");
                    }

                    // vpp output
                    for (mfxU16 i = 0; i < cliParams.vppNum; i++) {
                        aSurf = outSurfaces->Surfaces[i + 1];
                        if (cliParams.bIsAvailableVPPOutFileName) {
                            sts = WriteRawFrame_InternalMem(aSurf, sinkVPP[i]);
                            VERIFY(MFX_ERR_NONE == sts, "ERROR - Could not write vpp output");
                        }
                        else {
                            sts = aSurf->FrameInterface->Release(aSurf);
                            VERIFY(MFX_ERR_NONE == sts,
                                   "ERROR - mfxFrameSurfaceInterface->Release failed");
                        }
                    }

                    framenum++;
                }
                break;
            case MFX_ERR_MORE_DATA:
                // The function requires more bitstream at input before decoding can proceed
                if (isDraining) {
                    isStillGoing = false;
                    sts          = MFX_ERR_NONE;
                }
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
                printf("ERROR - Unknown status %d\n", sts);
                isStillGoing = false;
                break;
        }
    }

    if (sts == MFX_ERR_NONE) {
        printf("Decode and VPP processed %d frames\n", framenum);
        DisplayDecVPPSummary(&cliParams);
    }

end:

    // Clean up resources - It is recommended to close components first, before
    // releasing allocated surfaces, since some surfaces may still be locked by
    // internal resources.
    if (mfxVPPChParams) {
        for (mfxU16 i = 0; i < cliParams.vppNum; i++) {
            if (mfxVPPChParams[i])
                delete mfxVPPChParams[i];
        }
        delete[] mfxVPPChParams;
    }

    if (source)
        fclose(source);

    if (sinkDec)
        fclose(sinkDec);

    if (cliParams.bIsAvailableVPPOutFileName) {
        if (sinkVPP) {
            for (mfxU16 i = 0; i < cliParams.vppNum; i++) {
                if (sinkVPP[i])
                    fclose(sinkVPP[i]);
            }

            delete[] sinkVPP;
        }
    }

    if (bitstream.Data)
        free(bitstream.Data);

    if (accelHandle)
        FreeAcceleratorHandle(accelHandle);

    if (loader)
        MFXUnload(loader);
    return 0;
}
