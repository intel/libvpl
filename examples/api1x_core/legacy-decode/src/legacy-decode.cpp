//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal Intel® Video Processing Library (Intel® VPL) decode application
/// using the core API subset.  For more information see:
/// https://software.intel.com/content/www/us/en/develop/articles/upgrading-from-msdk-to-onevpl.html
/// https://intel.github.io/libvpl
/// @file

#include "util.hpp"

#define OUTPUT_FILE           "out.raw"
#define BITSTREAM_BUFFER_SIZE 2000000

void Usage(void) {
    printf("\n");
    printf("   Usage  :  legacy-decode \n\n");
    printf("     -i             input file name (HEVC elementary stream)\n\n");
    printf("   Example:  legacy-decode -i in.h265\n");
    printf(
        "   To view:  ffplay -f rawvideo -pixel_format yuv420p -video_size [width]x[height] %s\n\n",
        OUTPUT_FILE);
    printf(" * Decode HEVC/H265 elementary stream to raw frames in %s\n\n", OUTPUT_FILE);
    return;
}

int main(int argc, char *argv[]) {
    //Variables used for legacy and 2.x
    bool isDraining                 = false;
    bool isStillGoing               = true;
    bool isFailed                   = false;
    FILE *sink                      = NULL;
    FILE *source                    = NULL;
    int accel_fd                    = 0;
    mfxBitstream bitstream          = {};
    mfxFrameSurface1 *decSurfaceOut = NULL;
    mfxSession session              = NULL;
    mfxStatus sts                   = MFX_ERR_NONE;
    mfxSyncPoint syncp              = {};
    mfxU32 framenum                 = 0;
    Params cliParams                = {};
    void *accelHandle               = NULL;
    mfxVideoParam mfxDecParams      = {};

    //variables used only in legacy version
    int nIndex                      = -1;
    mfxFrameAllocRequest decRequest = {};
    mfxFrameSurface1 *decSurfPool   = NULL;
    mfxU8 *decOutBuf                = NULL;

    // variables used only in 2.x version
    mfxConfig cfg;
    mfxVariant cfgVal[1];
    mfxLoader loader = NULL;

    //Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_DECODE) == false) {
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
    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "MFXCreateConfig failed")
    cfgVal[0].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[0].Data.U32 = MFX_IMPL_TYPE_HARDWARE;

    sts = MFXSetConfigFilterProperty(cfg, (mfxU8 *)"mfxImplDescription.Impl", cfgVal[0]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for Impl");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts,
           "Cannot create session -- no implementations meet selection criteria");

    // Print info about implementation loaded
    ShowImplementationInfo(loader, 0);

    // Convenience function to initialize available accelerator(s)
    accelHandle = InitAcceleratorHandle(session, &accel_fd);

    // Prepare input bitstream and start decoding
    bitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
    bitstream.Data      = (mfxU8 *)calloc(bitstream.MaxLength, sizeof(mfxU8));
    VERIFY(bitstream.Data, "Not able to allocate input buffer");

    //Pre-parse input stream
    sts = ReadEncodedStream(bitstream, source);
    VERIFY(MFX_ERR_NONE == sts, "Error reading bitstream\n");

    mfxDecParams.mfx.CodecId = MFX_CODEC_HEVC;
    mfxDecParams.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    sts                      = MFXVideoDECODE_DecodeHeader(session, &bitstream, &mfxDecParams);
    VERIFY(MFX_ERR_NONE == sts, "Error decoding header\n");

    // input parameters finished, now initialize decode
    sts = MFXVideoDECODE_Init(session, &mfxDecParams);
    VERIFY(MFX_ERR_NONE == sts, "Error initializing decode\n");

    // Query number required surfaces for decoder
    MFXVideoDECODE_QueryIOSurf(session, &mfxDecParams, &decRequest);

    // External (application) allocation of decode surfaces
    decSurfPool =
        (mfxFrameSurface1 *)calloc(sizeof(mfxFrameSurface1), decRequest.NumFrameSuggested);
    sts = AllocateExternalSystemMemorySurfacePool(&decOutBuf,
                                                  decSurfPool,
                                                  mfxDecParams.mfx.FrameInfo,
                                                  decRequest.NumFrameSuggested);
    VERIFY(MFX_ERR_NONE == sts, "Error in external surface allocation\n");

    printf("Decoding %s -> %s\n", cliParams.infileName, OUTPUT_FILE);

    nIndex = GetFreeSurfaceIndex(decSurfPool, decRequest.NumFrameSuggested);
    while (isStillGoing == true) {
        // Load encoded stream if not draining
        if (isDraining == false) {
            sts = ReadEncodedStream(bitstream, source);
            if (sts != MFX_ERR_NONE)
                isDraining = true;
        }

        sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                              (isDraining) ? NULL : &bitstream,
                                              &decSurfPool[nIndex],
                                              &decSurfaceOut,
                                              &syncp);

        switch (sts) {
            case MFX_ERR_NONE:
                do {
                    sts = MFXVideoCORE_SyncOperation(session, syncp, WAIT_100_MILLISECONDS);
                    if (MFX_ERR_NONE == sts) {
                        WriteRawFrame(decSurfaceOut, sink);
                        framenum++;
                    }
                } while (sts == MFX_WRN_IN_EXECUTION);
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
                nIndex = GetFreeSurfaceIndex(decSurfPool, decRequest.NumFrameSuggested);
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
    printf("Decoded %d frames\n", framenum);

    // Clean up resources - It is recommended to close components first, before
    // releasing allocated surfaces, since some surfaces may still be locked by
    // internal resources.

    if (session) {
        MFXVideoDECODE_Close(session);
        MFXClose(session);
    }

    if (bitstream.Data)
        free(bitstream.Data);

    if (decSurfPool || decOutBuf) {
        FreeExternalSystemMemorySurfacePool(decOutBuf, decSurfPool);
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

    if (isFailed) {
        return -1;
    }
    else {
        return 0;
    }
}
