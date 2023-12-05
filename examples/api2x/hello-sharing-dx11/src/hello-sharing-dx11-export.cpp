//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal Intel® Video Processing Library (Intel® VPL) decode application,
/// using 2.9 or newer API with internal memory management.
/// For more information see
/// https://intel.github.io/libvpl
/// @file

#include <vector>
#include "device-d3d11.h"
#include "util.hpp"

#define OUTPUT_FILE                "out.raw"
#define BITSTREAM_BUFFER_SIZE      2000000
#define MAJOR_API_VERSION_REQUIRED 2
#define MINOR_API_VERSION_REQUIRED 9

void Usage(void) {
    printf("\n");
    printf("   Usage  :  hello-sharing-dx11-export \n\n");
    printf("     -i             input file name (HEVC elementary stream)\n\n");
    printf("   Example:  hello-sharing-dx11-export -i in.h265\n");
    printf("   To view:  ffplay -f rawvideo -pixel_format nv12 -video_size "
           "[width]x[height] %s\n\n",
           OUTPUT_FILE);
    printf(" * Decode HEVC/H265 elementary stream to raw frames in %s\n\n", OUTPUT_FILE);
    printf("   GPU native color format is NV12\n");
    return;
}

int main(int argc, char *argv[]) {
    // Variables used for legacy and 2.x
    bool isDraining                 = false;
    bool isStillGoing               = true;
    bool isFailed                   = false;
    FILE *sink                      = NULL;
    FILE *source                    = NULL;
    mfxBitstream bitstream          = {};
    mfxFrameSurface1 *decSurfaceOut = NULL;
    mfxSession session              = NULL;
    mfxSyncPoint syncp              = {};
    mfxU32 framenum                 = 0;
    mfxStatus sts                   = MFX_ERR_NONE;
    Params cliParams                = {};
    mfxVideoParam decodeParams      = {};

    // variables used only in 2.x version
    mfxConfig cfg[5];
    mfxVariant cfgVal[5];
    mfxLoader loader = NULL;

    // Parse command line args to cliParams
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

    // Implementation used must be D3D11 acceleration mode
    cfg[3] = MFXCreateConfig(loader);
    VERIFY(NULL != cfg[3], "MFXCreateConfig failed")
    cfgVal[3].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[3].Data.U32 = MFX_ACCEL_MODE_VIA_D3D11;

    sts = MFXSetConfigFilterProperty(cfg[3],
                                     (mfxU8 *)"mfxImplDescription.AccelerationMode",
                                     cfgVal[3]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for acceleration mode");

    // Implementation used must be D3D11 surface sharing mode
    // Applying the 3 associated parameters (logical AND operation) using a single mfxConfig
    cfg[4] = MFXCreateConfig(loader);
    VERIFY(NULL != cfg[4], "MFXCreateConfig failed");
    cfgVal[4].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[4].Data.U32 = MFX_SURFACE_TYPE_D3D11_TEX2D;
    sts                = MFXSetConfigFilterProperty(cfg[4],
                                     (mfxU8 *)"mfxSurfaceTypesSupported.surftype.SurfaceType",
                                     cfgVal[4]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for surface sharing type");

    cfgVal[4].Data.U32 = MFX_SURFACE_COMPONENT_DECODE;
    sts                = MFXSetConfigFilterProperty(
        cfg[4],
        (mfxU8 *)"mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceComponent",
        cfgVal[4]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for surface sharing component");

    cfgVal[4].Data.U32 = MFX_SURFACE_FLAG_EXPORT_SHARED;
    sts                = MFXSetConfigFilterProperty(
        cfg[4],
        (mfxU8 *)"mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceFlags",
        cfgVal[4]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for surface sharing flags");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts,
           "Cannot create session -- no implementations meet selection criteria");

    // Print info about implementation loaded
    ShowImplementationInfo(loader, 0);

    // Prepare input bitstream and start decoding
    bitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
    std::vector<mfxU8> bitstream_data(bitstream.MaxLength, 0);
    bitstream.Data = bitstream_data.data();
    VERIFY(bitstream.Data, "Not able to allocate input buffer");
    bitstream.CodecId = MFX_CODEC_HEVC;

    // Pre-parse input stream
    sts = ReadEncodedStream(bitstream, source);
    VERIFY(MFX_ERR_NONE == sts, "Error reading bitstream\n");

    decodeParams.mfx.CodecId = MFX_CODEC_HEVC;
    decodeParams.IOPattern   = MFX_IOPATTERN_OUT_VIDEO_MEMORY;
    sts                      = MFXVideoDECODE_DecodeHeader(session, &bitstream, &decodeParams);
    VERIFY(MFX_ERR_NONE == sts, "Error decoding header\n");

    // create HW device - automatically released when devCtx goes out of scope
    DevCtxD3D11 devCtx       = {};
    mfxHandleType handleType = {};
    mfxHDL handle            = nullptr;

    sts               = devCtx.InitDevice(0, &handleType, &handle);
    auto pD3D11Device = reinterpret_cast<ID3D11Device *>(handle);
    VERIFY((MFX_ERR_NONE == sts) && (handle != nullptr), "ERROR: InitDevice");

    // pass device handle to runtime
    sts = MFXVideoCORE_SetHandle(session, handleType, handle);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXVideoCORE_SetHandle failed");
    // input parameters finished, now initialize decode
    sts = MFXVideoDECODE_Init(session, &decodeParams);
    VERIFY(MFX_ERR_NONE == sts, "Error initializing decode\n");

    printf("Decoding %s -> %s\n", cliParams.infileName, OUTPUT_FILE);
    printf("Output colorspace: ");
    switch (decodeParams.mfx.FrameInfo.FourCC) {
        case MFX_FOURCC_NV12: // GPU output
            printf("NV12\n");
            break;
        default:
            printf("Unsupported color format\n");
            isFailed = true;
            exit(1);
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
                        mfxSurfaceHeader exportHeader            = {};
                        exportHeader.SurfaceType                 = MFX_SURFACE_TYPE_D3D11_TEX2D;
                        exportHeader.SurfaceFlags                = MFX_SURFACE_FLAG_EXPORT_SHARED;
                        mfxSurfaceHeader *exportedSurfaceGeneral = nullptr;

                        mfxStatus sts_surf =
                            decSurfaceOut->FrameInterface->Export(decSurfaceOut,
                                                                  exportHeader,
                                                                  &exportedSurfaceGeneral);
                        VERIFY(MFX_ERR_NONE == sts_surf, "Could not export to surface");

                        auto exportedSurface =
                            reinterpret_cast<mfxSurfaceD3D11Tex2D *>(exportedSurfaceGeneral);
                        sts_surf = WriteRawFrame(pD3D11Device,
                                                 (ID3D11Texture2D *)(exportedSurface->texture2D),
                                                 sink);
                        VERIFY(MFX_ERR_NONE == sts_surf, "Could not write decode output");

                        sts_surf = exportedSurface->SurfaceInterface.Release(
                            &(exportedSurface->SurfaceInterface));
                        VERIFY(MFX_ERR_NONE == sts_surf, "Could not Release exported surface");

                        framenum++;
                    }

                    if (sts != MFX_WRN_IN_EXECUTION) {
                        sts = decSurfaceOut->FrameInterface->Release(decSurfaceOut);
                        VERIFY(MFX_ERR_NONE == sts, "Could not release decode output surface");
                    }

                } while (sts == MFX_WRN_IN_EXECUTION);
                break;
            case MFX_ERR_MORE_DATA:
                // The function requires more bitstream at input before decoding can
                // proceed
                if (isDraining)
                    isStillGoing = false;
                break;
            case MFX_ERR_MORE_SURFACE:
                // The function requires more frame surface at output before decoding
                // can proceed. This applies to external memory allocations and should
                // not be expected for a simple internal allocation case like this
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
                // In external memory allocation case, might need to reallocate the
                // output surface
                break;
            case MFX_ERR_INCOMPATIBLE_VIDEO_PARAM:
                // The function detected that video parameters provided by the
                // application are incompatible with initialization parameters. The
                // application should close the component and then reinitialize it
                break;
            case MFX_ERR_REALLOC_SURFACE:
                // Bigger surface_work required. May be returned only if
                // mfxInfoMFX::EnableReallocRequest was set to ON during initialization.
                // This applies to external memory allocations and should not be
                // expected for a simple internal allocation case like this
                break;
            default:
                printf("unknown status %d\n", sts);
                isStillGoing = false;
                break;
        }
    }

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

    if (loader)
        MFXUnload(loader);

    if (isFailed)
        return -1;

    return 0;
}
