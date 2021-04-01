//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) decode application,
/// using 2.x API with internal memory management
///
/// @file

#include "util/util.h"

#define OUTPUT_FILE           "out.raw"
#define BITSTREAM_BUFFER_SIZE 2000000

void Usage(void) {
    printf("\n");
    printf("   Usage  :  hello-decode \n\n");
    printf("     -sw/-hw        use software or hardware implementation\n");
    printf("     -i             input file name (HEVC elementary stream)\n\n");
    printf("   Example:  hello-decode -sw  -i in.h265\n");
    printf(
        "   To view:  ffplay -f rawvideo -pixel_format yuv420p -video_size [width]x[height] %s\n\n",
        OUTPUT_FILE);
    printf(" * Decode HEVC/H265 elementary stream to raw frames in %s\n\n", OUTPUT_FILE);
    printf("   CPU native color format is I420/yuv420p.  GPU native color format is NV12\n");
    return;
}

int main(int argc, char *argv[]) {
    //Variables used for legacy and 2.x
    bool isDraining                 = false;
    bool isStillGoing               = true;
    FILE *sink                      = NULL;
    FILE *source                    = NULL;
    mfxBitstream bitstream          = {};
    mfxFrameSurface1 *decSurfaceOut = NULL;
    mfxSession session              = NULL;
    mfxStatus sts                   = MFX_ERR_NONE;
    mfxSyncPoint syncp              = {};
    mfxU32 framenum                 = 0;
    Params cliParams                = {};
    void *accelHandle               = NULL;
    mfxVideoParam mfxDecParams      = {};
    mfxVersion version              = { 0, 1 };

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

    sink = fopen(OUTPUT_FILE, "wb");
    VERIFY(sink, "Could not create output file");

    // Initialize VPL session
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

    // input parameters finished, now initialize decode
    sts = MFXVideoDECODE_Init(session, &mfxDecParams);
    VERIFY(MFX_ERR_NONE == sts, "Error initializing decode\n");

    printf("Decoding %s -> %s\n", cliParams.infileName, OUTPUT_FILE);

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

    while (isStillGoing == true) {
        // Load encoded stream if not draining
        if (isDraining == false) {
            sts = ReadEncodedStream(bitstream, source);
            if (sts != MFX_ERR_NONE)
                isDraining = true;
        }

        sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                              (isDraining) ? NULL : &bitstream,
                                              NULL,
                                              &decSurfaceOut,
                                              &syncp);

        switch (sts) {
            case MFX_ERR_NONE:
                do {
                    sts = decSurfaceOut->FrameInterface->Synchronize(decSurfaceOut,
                                                                     WAIT_100_MILLISECONDS);
                    if (MFX_ERR_NONE == sts) {
                        sts = WriteRawFrame_InternalMem(decSurfaceOut, sink);
                        VERIFY(MFX_ERR_NONE == sts, "Could not write decode output");

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
    if (source)
        fclose(source);

    if (sink)
        fclose(sink);

    MFXVideoDECODE_Close(session);
    MFXClose(session);

    if (bitstream.Data)
        free(bitstream.Data);

    if (accelHandle)
        FreeAcceleratorHandle(accelHandle);

    if (loader)
        MFXUnload(loader);
    return 0;
}
