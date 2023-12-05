//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal Intel® Video Processing Library (Intel® VPL) VPP application
/// using the core API subset.  For more information see:
/// https://intel.github.io/libvpl
/// @file

#include "util.hpp"

#define OUTPUT_WIDTH  640
#define OUTPUT_HEIGHT 480
#define OUTPUT_FILE   "out.raw"

void Usage(void) {
    printf("\n");
    printf("   Usage  :  legacy-vpp\n");
    printf("     -i input file name (NV12 raw frames)\n");
    printf("     -w input width\n");
    printf("     -h input height\n\n");
    printf("   Example:  legacy-vpp -i in.NV12 -w 320 -h 240\n");
    printf("   To view:  ffplay -f rawvideo -pixel_format bgra -video_size %dx%d  "
           "%s\n\n",
           OUTPUT_WIDTH,
           OUTPUT_HEIGHT,
           OUTPUT_FILE);
    printf(" * Resize raw frames to %dx%d and convert to BGRA in %s\n\n",
           OUTPUT_WIDTH,
           OUTPUT_HEIGHT,
           OUTPUT_FILE);
    return;
}

int main(int argc, char *argv[]) {
    bool isDraining      = false;
    bool isStillGoing    = true;
    bool isFailed        = false;
    FILE *sink           = NULL;
    FILE *source         = NULL;
    int accel_fd         = 0;
    int nIndexVPPInSurf  = 0;
    int nIndexVPPOutSurf = 0;
    int result           = 0;
    mfxConfig cfg[1];
    mfxVariant cfgVal[1];
    mfxFrameAllocRequest VPPRequest[2]  = {};
    mfxFrameSurface1 *vppInSurfacePool  = NULL;
    mfxFrameSurface1 *vppOutSurfacePool = NULL;
    mfxLoader loader                    = NULL;
    mfxSession session                  = NULL;
    mfxStatus sts                       = MFX_ERR_NONE;
    mfxSyncPoint syncp;
    mfxU16 nSurfNumVPPIn    = 0;
    mfxU16 nSurfNumVPPOut   = 0;
    mfxU32 framenum         = 0;
    mfxU8 *vppInBuf         = NULL;
    mfxU8 *vppOutBuf        = NULL;
    mfxVideoParam VPPParams = {};
    Params cliParams        = {};
    void *accelHandle       = NULL;

    // Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_VPP) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    source = fopen(cliParams.infileName, "rb");
    VERIFY(source, "Could not open input file");

    sink = fopen(OUTPUT_FILE, "wb");
    VERIFY(sink, "Could not create output file");

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

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts,
           "Cannot create session -- no implementations meet selection criteria");

    // Print info about implementation loaded
    ShowImplementationInfo(loader, 0);

    // Convenience function to initialize available accelerator(s)
    accelHandle = InitAcceleratorHandle(session, &accel_fd);

    // Initialize VPP parameters
    PrepareFrameInfo(&VPPParams.vpp.In, (MFX_FOURCC_NV12), cliParams.srcWidth, cliParams.srcHeight);
    PrepareFrameInfo(&VPPParams.vpp.Out, MFX_FOURCC_BGRA, OUTPUT_WIDTH, OUTPUT_HEIGHT);

    VPPParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    // Initialize VPP
    sts = MFXVideoVPP_Init(session, &VPPParams);
    VERIFY(MFX_ERR_NONE == sts, "Could not initialize VPP");

    // Query number of required surfaces for VPP
    sts = MFXVideoVPP_QueryIOSurf(session, &VPPParams, VPPRequest);
    VERIFY(MFX_ERR_NONE == sts, "Error in QueryIOSurf");

    nSurfNumVPPIn  = VPPRequest[0].NumFrameSuggested; // vpp in
    nSurfNumVPPOut = VPPRequest[1].NumFrameSuggested; // vpp out

    // Allocate surfaces for VPP in and VPP out
    // - Width and height of buffer must be aligned, a multiple of 32
    // - Frame surface array keeps pointers all surface planes and general frame
    // info
    vppInSurfacePool = (mfxFrameSurface1 *)calloc(sizeof(mfxFrameSurface1), nSurfNumVPPIn);

    sts = AllocateExternalSystemMemorySurfacePool(&vppInBuf,
                                                  vppInSurfacePool,
                                                  VPPParams.vpp.In,
                                                  nSurfNumVPPIn);
    VERIFY(MFX_ERR_NONE == sts, "Error in external surface allocation for VPP in\n");

    vppOutSurfacePool = (mfxFrameSurface1 *)calloc(sizeof(mfxFrameSurface1), nSurfNumVPPOut);
    sts               = AllocateExternalSystemMemorySurfacePool(&vppOutBuf,
                                                  vppOutSurfacePool,
                                                  VPPParams.vpp.Out,
                                                  nSurfNumVPPOut);
    VERIFY(MFX_ERR_NONE == sts, "Error in external surface allocation for VPP out\n");

    // ===================================
    // Start processing the frames
    //

    printf("Processing %s -> %s\n", cliParams.infileName, OUTPUT_FILE);

    while (isStillGoing == true) {
        // Load a new frame if not draining
        if (isDraining == false) {
            nIndexVPPInSurf = GetFreeSurfaceIndex(vppInSurfacePool,
                                                  nSurfNumVPPIn); // Find free input frame surface

            sts = ReadRawFrame(&vppInSurfacePool[nIndexVPPInSurf],
                               source); // Load frame from file into surface
            if (sts != MFX_ERR_NONE)
                isDraining = true;
        }

        nIndexVPPOutSurf = GetFreeSurfaceIndex(vppOutSurfacePool,
                                               nSurfNumVPPOut); // Find free output frame surface

        sts = MFXVideoVPP_RunFrameVPPAsync(
            session,
            (isDraining == true) ? NULL : &vppInSurfacePool[nIndexVPPInSurf],
            &vppOutSurfacePool[nIndexVPPOutSurf],
            NULL,
            &syncp);

        switch (sts) {
            case MFX_ERR_NONE: {
                sts = MFXVideoCORE_SyncOperation(session, syncp, WAIT_100_MILLISECONDS * 1000);
                VERIFY(MFX_ERR_NONE == sts, "Error in SyncOperation");

                mfxFrameSurface1 *pmfxOutSurface;
                pmfxOutSurface = &vppOutSurfacePool[nIndexVPPOutSurf];

                // output surface
                sts = WriteRawFrame(pmfxOutSurface, sink);
                if (sts != MFX_ERR_NONE) {
                    printf("Error in WriteRawFrame\n");
                    result   = sts;
                    isFailed = true;
                    goto end;
                }

                printf("Frame number: %d\r", ++framenum);
                fflush(stdout);

            } break;
            case MFX_ERR_MORE_DATA:
                // Need more input frames before VPP can produce an output
                if (isDraining)
                    isStillGoing = false;
                break;
            case MFX_ERR_MORE_SURFACE:
                // The output frame is ready after synchronization.
                // Need more surfaces at output for additional output frames available.
                // This applies to external memory allocations and should not be
                // expected for a simple internal allocation case like this
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

    if (session) {
        MFXVideoVPP_Close(session);
        MFXClose(session);
    }

    if (vppInBuf || vppInSurfacePool) {
        FreeExternalSystemMemorySurfacePool(vppInBuf, vppInSurfacePool);
    }

    if (vppOutBuf || vppOutSurfacePool) {
        FreeExternalSystemMemorySurfacePool(vppOutBuf, vppOutSurfacePool);
    }

    if (source)
        fclose(source);

    if (sink)
        fclose(sink);

    FreeAcceleratorHandle(accelHandle, accel_fd);
    accelHandle = NULL;
    accel_fd    = 0;

    if (loader)
        MFXUnload(loader);

    if (isFailed || result != 0) {
        return -1;
    }
    else {
        return 0;
    }
}
