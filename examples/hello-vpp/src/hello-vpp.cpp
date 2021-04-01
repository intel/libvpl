//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) VPP application,
/// using 2.x API with internal memory management
///
/// @file

#include "util/util.h"

#define OUTPUT_WIDTH  640
#define OUTPUT_HEIGHT 480
#define OUTPUT_FILE   "out.raw"

void Usage(void) {
    printf("\n");
    printf("   Usage  :  hello-vpp\n");
    printf("     -hw        use hardware implementation\n");
    printf("     -sw        use software implementation\n");
    printf("     -i input file name (sw=I420 raw frames, hw=NV12)\n");
    printf("     -w input width\n");
    printf("     -h input height\n\n");
    printf("   Example:  hello-vpp -i in.i420 -w 128 -h 96 -sw\n");
    printf(
        "   To view:  ffplay -f rawvideo -pixel_format yuv420p -video_size %dx%d -pixel_format bgra %s\n\n",
        OUTPUT_WIDTH,
        OUTPUT_HEIGHT,
        OUTPUT_FILE);
    printf(" * Resize raw frames to %dx%d size in %s\n\n",
           OUTPUT_WIDTH,
           OUTPUT_HEIGHT,
           OUTPUT_FILE);
    printf("   CPU native color format is I420/yuv420p.  GPU native color format is NV12\n");
    return;
}

int main(int argc, char *argv[]) {
    //Variables used for legacy and 2.x
    bool isDraining                 = false;
    bool isStillGoing               = true;
    FILE *sink                      = NULL;
    FILE *source                    = NULL;
    mfxFrameSurface1 *vppInSurface  = NULL;
    mfxFrameSurface1 *vppOutSurface = NULL;
    mfxSession session              = NULL;
    mfxStatus sts                   = MFX_ERR_NONE;
    mfxStatus sts_r                 = MFX_ERR_NONE;
    mfxSyncPoint syncp              = {};
    mfxU32 framenum                 = 0;
    mfxVideoParam mfxVPPParams      = {};
    void *accelHandle               = NULL;
    Params cliParams                = {};
    mfxVersion version              = { 0, 1 };
    mfxVariant impl_value           = {};

    //variables used only in 2.x version
    mfxConfig cfg    = NULL;
    mfxLoader loader = NULL;

    //Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_VPP) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    source = fopen(cliParams.infileName, "rb");
    VERIFY(source, "Could not open input file");

    sink = fopen(OUTPUT_FILE, "wb");
    VERIFY(sink, "Could not create output file");

    // Initialize VPL session
    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed -- is implementation in path?");

    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "MFXCreateConfig failed")

    // Implementation used must be the type requested from command line
    sts = MFXSetConfigFilterProperty(cfg, (mfxU8 *)"mfxImplDescription.Impl", cliParams.implValue);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    // Implementation must provide VPP scaling
    impl_value.Type     = MFX_VARIANT_TYPE_U32;
    impl_value.Data.U32 = MFX_EXTBUFF_VPP_SCALING;
    sts                 = MFXSetConfigFilterProperty(
        cfg,
        (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC",
        impl_value);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session");

    // Print info about implementation loaded
    version = ShowImplInfo(session);
    VERIFY(version.Major > 1, "Sample requires 2.x API implementation, exiting");

    // Convenience function to initialize available accelerator(s)
    accelHandle = InitAcceleratorHandle(session);

    // Initialize VPP parameters
    if (MFX_IMPL_SOFTWARE == cliParams.impl) {
        PrepareFrameInfo(&mfxVPPParams.vpp.In,
                         MFX_FOURCC_I420,
                         cliParams.srcWidth,
                         cliParams.srcHeight);
        PrepareFrameInfo(&mfxVPPParams.vpp.Out, MFX_FOURCC_BGRA, OUTPUT_WIDTH, OUTPUT_HEIGHT);
    }
    else {
        PrepareFrameInfo(&mfxVPPParams.vpp.In,
                         MFX_FOURCC_NV12,
                         cliParams.srcWidth,
                         cliParams.srcHeight);
        PrepareFrameInfo(&mfxVPPParams.vpp.Out, MFX_FOURCC_BGRA, OUTPUT_WIDTH, OUTPUT_HEIGHT);
    }

    mfxVPPParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    // Initialize VPP
    sts = MFXVideoVPP_Init(session, &mfxVPPParams);
    VERIFY(MFX_ERR_NONE == sts, "Could not initialize VPP");

    printf("Processing %s -> %s\n", cliParams.infileName, OUTPUT_FILE);

    while (isStillGoing == true) {
        // Load a new frame if not draining
        if (isDraining == false) {
            sts = MFXMemory_GetSurfaceForVPPIn(session, &vppInSurface);
            VERIFY(MFX_ERR_NONE == sts, "Unknown error in MFXMemory_GetSurfaceForVPPIn");

            sts = ReadRawFrame_InternalMem(vppInSurface, source);
            if (sts == MFX_ERR_MORE_DATA)
                isDraining = true;
            else
                VERIFY(MFX_ERR_NONE == sts, "Unknown error reading input");

            sts = MFXMemory_GetSurfaceForVPPOut(session, &vppOutSurface);
            VERIFY(MFX_ERR_NONE == sts, "Unknown error in MFXMemory_GetSurfaceForVPPIn");
        }

        sts = MFXVideoVPP_RunFrameVPPAsync(session,
                                           (isDraining == true) ? NULL : vppInSurface,
                                           vppOutSurface,
                                           NULL,
                                           &syncp);

        if (!isDraining) {
            sts_r = vppInSurface->FrameInterface->Release(vppInSurface);
            VERIFY(MFX_ERR_NONE == sts_r, "mfxFrameSurfaceInterface->Release failed");
        }

        switch (sts) {
            case MFX_ERR_NONE:
                sts = WriteRawFrame_InternalMem(vppOutSurface, sink);
                VERIFY(MFX_ERR_NONE == sts, "Could not write vpp output");

                framenum++;
                break;
            case MFX_ERR_MORE_DATA:
                // Need more input frames before VPP can produce an output
                if (isDraining)
                    isStillGoing = false;
                break;
            case MFX_ERR_MORE_SURFACE:
                // The output frame is ready after synchronization.
                // Need more surfaces at output for additional output frames available.
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
            default:
                printf("unknown status %d\n", sts);
                isStillGoing = false;
                break;
        }
    }

end:
    printf("Processed %d frames\n", framenum);

    // Clean up resources - It is recommended to close components first, before
    // releasing allocated surfaces, since some surfaces may still be locked by
    // internal resources.
    if (source)
        fclose(source);

    if (sink)
        fclose(sink);

    MFXVideoVPP_Close(session);
    MFXClose(session);

    if (accelHandle)
        FreeAcceleratorHandle(accelHandle);

    if (loader)
        MFXUnload(loader);

    return 0;
}
