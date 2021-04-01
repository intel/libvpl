//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) VPP application,
/// using 1.34 API subset without removed features
///
/// @file

#include "util/util.h"

#define OUTPUT_WIDTH  640
#define OUTPUT_HEIGHT 480
#define OUTPUT_FILE   "out.raw"

void Usage(void) {
    printf("\n");
    printf("   Usage  :  legacy-vpp\n");
    printf("     -hw        use hardware implementation\n");
    printf("     -sw        use software implementation\n");
    printf("     -i input file name (sw=I420 raw frames, hw=NV12)\n");
    printf("     -w input width\n");
    printf("     -h input height\n\n");
    printf("   Example:  legacy-vpp -i in.i420 -w 128 -h 96\n");
    printf(
        "   To view:  ffplay -f rawvideo -pixel_format yuv420p -video_size %dx%d -pixel_format yuv420p %s\n\n",
        OUTPUT_WIDTH,
        OUTPUT_HEIGHT,
        OUTPUT_FILE);
    printf(" * Resize raw frames to %dx%d size in %s\n\n",
           OUTPUT_WIDTH,
           OUTPUT_HEIGHT,
           OUTPUT_FILE);
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
    mfxSyncPoint syncp              = {};
    mfxU32 framenum                 = 0;
    mfxVideoParam mfxVPPParams      = {};
    void *accelHandle               = NULL;
    Params cliParams                = {};
    mfxVersion version              = { 0, 1 };

    //variables used only in legacy version
    int nIndexVPPInSurf                 = 0;
    int nIndexVPPOutSurf                = 0;
    mfxFrameAllocRequest VPPRequest[2]  = {};
    mfxFrameSurface1 *vppInSurfacePool  = NULL;
    mfxFrameSurface1 *vppOutSurfacePool = NULL;
    mfxU16 nSurfNumVPPIn                = 0;
    mfxU16 nSurfNumVPPOut               = 0;
    mfxU8 *vppInBuf                     = NULL;
    mfxU8 *vppOutBuf                    = NULL;

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
    sts = MFXInit(cliParams.impl, &version, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session");

    // Print info about implementation loaded
    ShowImplInfo(session);

    // Convenience function to initialize available accelerator(s)
    accelHandle = InitAcceleratorHandle(session);

    // Initialize VPP parameters
    if (MFX_IMPL_SOFTWARE == cliParams.impl) {
        PrepareFrameInfo(&mfxVPPParams.vpp.In,
                         MFX_FOURCC_I420,
                         cliParams.srcWidth,
                         cliParams.srcHeight);
        PrepareFrameInfo(&mfxVPPParams.vpp.Out, MFX_FOURCC_I420, OUTPUT_WIDTH, OUTPUT_HEIGHT);
    }
    else {
        PrepareFrameInfo(&mfxVPPParams.vpp.In,
                         MFX_FOURCC_NV12,
                         cliParams.srcWidth,
                         cliParams.srcHeight);
        PrepareFrameInfo(&mfxVPPParams.vpp.Out, MFX_FOURCC_NV12, OUTPUT_WIDTH, OUTPUT_HEIGHT);
    }

    mfxVPPParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    // Query number required surfaces for VPPs
    MFXVideoVPP_QueryIOSurf(session, &mfxVPPParams, VPPRequest);

    // Determine the required number of surfaces for VPP input and output
    nSurfNumVPPIn  = VPPRequest[0].NumFrameSuggested; // vpp in
    nSurfNumVPPOut = VPPRequest[1].NumFrameSuggested; // vpp out

    // Allocate surfaces for VPP in and VPP out
    // - Width and height of buffer must be aligned, a multiple of 32
    // - Frame surface array keeps pointers all surface planes and general frame info
    vppInSurfacePool = (mfxFrameSurface1 *)calloc(sizeof(mfxFrameSurface1), nSurfNumVPPIn);

    sts = AllocateExternalSystemMemorySurfacePool(&vppInBuf,
                                                  vppInSurfacePool,
                                                  mfxVPPParams.vpp.In,
                                                  nSurfNumVPPIn);
    VERIFY(MFX_ERR_NONE == sts, "Error in external surface allocation for VPP in\n");

    vppOutSurfacePool = (mfxFrameSurface1 *)calloc(sizeof(mfxFrameSurface1), nSurfNumVPPOut);

    sts = AllocateExternalSystemMemorySurfacePool(&vppOutBuf,
                                                  vppOutSurfacePool,
                                                  mfxVPPParams.vpp.Out,
                                                  nSurfNumVPPOut);
    VERIFY(MFX_ERR_NONE == sts, "Error in external surface allocation for VPP out\n");

    // Initialize VPP
    sts = MFXVideoVPP_Init(session, &mfxVPPParams);
    VERIFY(MFX_ERR_NONE == sts, "Could not initialize VPP");

    printf("Processing %s -> %s\n", cliParams.infileName, OUTPUT_FILE);

    nIndexVPPInSurf = GetFreeSurfaceIndex(vppInSurfacePool, nSurfNumVPPIn);
    VERIFY(nIndexVPPInSurf != MFX_ERR_NOT_FOUND, "Could not find available surface for VPP in");

    nIndexVPPOutSurf = GetFreeSurfaceIndex(vppOutSurfacePool, nSurfNumVPPOut);
    VERIFY(nIndexVPPOutSurf != MFX_ERR_NOT_FOUND, "Could not find available surface for VPP out");

    while (isStillGoing == true) {
        // Load a new frame if not draining
        if (isDraining == false) {
            vppInSurface  = &vppInSurfacePool[nIndexVPPInSurf];
            vppOutSurface = &vppOutSurfacePool[nIndexVPPOutSurf];

            sts = ReadRawFrame(vppInSurface, source);
            if (sts == MFX_ERR_MORE_DATA)
                isDraining = true;
            else
                VERIFY(MFX_ERR_NONE == sts, "Unknown error reading input");
        }

        sts = MFXVideoVPP_RunFrameVPPAsync(session,
                                           (isDraining == true) ? NULL : vppInSurface,
                                           vppOutSurface,
                                           NULL,
                                           &syncp);

        switch (sts) {
            case MFX_ERR_NONE:
                sts = WriteRawFrame(vppOutSurface, sink);
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

    FreeExternalSystemMemorySurfacePool(vppInBuf, vppInSurfacePool);
    FreeExternalSystemMemorySurfacePool(vppOutBuf, vppOutSurfacePool);

    if (accelHandle)
        FreeAcceleratorHandle(accelHandle);

    return 0;
}
