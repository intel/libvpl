//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal Intel® Video Processing Library (Intel® VPL) encode application,
/// using >=2.10 API with internal memory management
///
/// @file

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <algorithm>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>
#include "device-d3d11.h"
#include "util.hpp"

#define TARGETKBPS                 2000
#define FRAMERATE                  30
#define OUTPUT_FILE                "out.h265"
#define BITSTREAM_BUFFER_SIZE      2000000
#define MAJOR_API_VERSION_REQUIRED 2
#define MINOR_API_VERSION_REQUIRED 9

void Usage(void) {
    printf("\n");
    printf("   Usage  :  hello-sharing-dx11-import\n");
    printf("     -i input file name (NV12 raw frames)\n");
    printf("     -w input width\n");
    printf("     -h input height\n\n");
    printf("   Example:  hello-sharing-dx11-import -i in.NV12 -w 320 -h 240\n");
    printf("   To view:  ffplay %s\n\n", OUTPUT_FILE);
    printf(" * Encode raw frames to HEVC/H265 elementary stream in %s\n\n", OUTPUT_FILE);
    printf("   GPU native color format is "
           "NV12\n");
    return;
}

int main(int argc, char *argv[]) {
    // Variables used for legacy and 2.x
    bool isDraining                  = false;
    bool isStillGoing                = true;
    bool isFailed                    = false;
    FILE *sink                       = NULL;
    FILE *source                     = NULL;
    mfxBitstream bitstream           = {};
    mfxFrameSurface1 *pmfxEncSurface = NULL;
    mfxSession session               = NULL;
    mfxU32 framenum                  = 0;
    mfxStatus sts                    = MFX_ERR_NONE;
    mfxStatus sts_r                  = MFX_ERR_NONE;
    Params cliParams                 = {};
    mfxVideoParam encodeParams       = {};

    // variables used only in 2.x version
    mfxConfig cfg[5];
    mfxVariant cfgVal[5];
    mfxLoader loader = NULL;

    // Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_ENCODE) == false) {
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

    // Implementation used must be the hardware implementation
    cfg[0]             = MFXCreateConfig(loader);
    cfgVal[0].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[0].Data.U32 = MFX_IMPL_TYPE_HARDWARE;

    sts = MFXSetConfigFilterProperty(cfg[0], (mfxU8 *)"mfxImplDescription.Impl", cfgVal[0]);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for Impl");

    // Implementation must provide an HEVC encoder
    cfg[1] = MFXCreateConfig(loader);
    VERIFY(NULL != cfg[1], "MFXCreateConfig failed")
    cfgVal[1].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[1].Data.U32 = MFX_CODEC_HEVC;
    sts                = MFXSetConfigFilterProperty(
        cfg[1],
        (mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID",
        cfgVal[1]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for encoder CodecID");

    // Implementation used must provide API version 2.9 or newer
    cfg[2] = MFXCreateConfig(loader);
    VERIFY(NULL != cfg[2], "MFXCreateConfig failed")
    cfgVal[2].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[2].Data.U32 = VPLVERSION(MAJOR_API_VERSION_REQUIRED, MINOR_API_VERSION_REQUIRED);
    sts                = MFXSetConfigFilterProperty(cfg[2],
                                     (mfxU8 *)"mfxImplDescription.ApiVersion.Version",
                                     cfgVal[2]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for API version");

    // Implementation used must be D3D11 acceleration mode
    cfg[3]             = MFXCreateConfig(loader);
    cfgVal[3].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[3].Data.U32 = MFX_ACCEL_MODE_VIA_D3D11;
    sts                = MFXSetConfigFilterProperty(cfg[3],
                                     (mfxU8 *)"mfxImplDescription.AccelerationMode",
                                     cfgVal[3]);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for acceleration mode");

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

    cfgVal[4].Data.U32 = MFX_SURFACE_COMPONENT_ENCODE;

    sts = MFXSetConfigFilterProperty(
        cfg[4],
        (mfxU8 *)"mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceComponent",
        cfgVal[4]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for surface sharing component");

    cfgVal[4].Data.U32 = MFX_SURFACE_FLAG_IMPORT_SHARED;
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

    // Initialize encode parameters
    encodeParams.IOPattern                   = MFX_IOPATTERN_IN_VIDEO_MEMORY;
    encodeParams.mfx.LowPower                = MFX_CODINGOPTION_ON;
    encodeParams.AsyncDepth                  = 1;
    encodeParams.mfx.CodecId                 = MFX_CODEC_HEVC;
    encodeParams.mfx.TargetUsage             = MFX_TARGETUSAGE_BALANCED;
    encodeParams.mfx.TargetKbps              = TARGETKBPS;
    encodeParams.mfx.RateControlMethod       = MFX_RATECONTROL_VBR;
    encodeParams.mfx.FrameInfo.FrameRateExtN = FRAMERATE;
    encodeParams.mfx.FrameInfo.FrameRateExtD = 1;
    encodeParams.mfx.FrameInfo.FourCC        = MFX_FOURCC_NV12;
    encodeParams.mfx.FrameInfo.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    encodeParams.mfx.FrameInfo.CropW         = cliParams.srcWidth;
    encodeParams.mfx.FrameInfo.CropH         = cliParams.srcHeight;
    encodeParams.mfx.FrameInfo.Width         = ALIGN16(cliParams.srcWidth);
    encodeParams.mfx.FrameInfo.Height        = ALIGN16(cliParams.srcHeight);

    // create HW device - automatically released when devCtx goes out of scope
    DevCtx devCtx            = {};
    mfxHandleType handleType = {};
    mfxHDL handle            = nullptr;

    sts = devCtx.InitDevice(0, &handleType, &handle);
    VERIFY((MFX_ERR_NONE == sts) && (handle != nullptr), "ERROR: InitDevice");

    // pass device handle to runtime
    sts = MFXVideoCORE_SetHandle(session, handleType, handle);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXVideoCORE_SetHandle failed");

    sts = MFXVideoENCODE_Init(session, &encodeParams);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXVideoENCODE_Init failed");

    // Prepare output bitstream
    bitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
    bitstream.Data      = (mfxU8 *)calloc(bitstream.MaxLength, sizeof(mfxU8));

    printf("Encoding %s -> %s\n", cliParams.infileName, OUTPUT_FILE);

    printf("Input colorspace: ");
    switch (encodeParams.mfx.FrameInfo.FourCC) {
        case MFX_FOURCC_NV12: // GPU input
            printf("NV12\n");
            break;
        default:
            printf("Unsupported color format\n");
            isFailed = true;
            exit(1);
    }

    // Prepare NV12 frame data
    std::vector<mfxU8> cpuBufData;
    CPUFrameInfo cpuFrameInfo = {};
    mfxU16 height             = cliParams.srcHeight;
    mfxU16 width              = cliParams.srcWidth;

    if (cpuBufData.size() != (width * height))
        cpuBufData.resize(width * height * 3 / 2);

    cpuFrameInfo.height = height;
    cpuFrameInfo.width  = width;
    cpuFrameInfo.pitch  = width;
    cpuFrameInfo.data   = cpuBufData.data();

    // Get interface for ImportFrameSurface
    mfxMemoryInterface *memoryInterface;
    sts = MFXGetMemoryInterface(session, &memoryInterface);
    VERIFY((MFX_ERR_NONE == sts) && (memoryInterface != nullptr), "ERROR: MFXGetMemoryInterface");

    while (isStillGoing == true) {
        // Load a new frame if not draining
        mfxSurfaceD3D11Tex2D extSurfD3D11 = {};
        CComPtr<ID3D11Texture2D> pTex2D;

        // Set header.surfaceType, device, and texture2D, all other fields should be empty
        extSurfD3D11.SurfaceInterface.Header.SurfaceType  = MFX_SURFACE_TYPE_D3D11_TEX2D;
        extSurfD3D11.SurfaceInterface.Header.SurfaceFlags = MFX_SURFACE_FLAG_IMPORT_SHARED;
        extSurfD3D11.SurfaceInterface.Header.StructSize   = sizeof(mfxSurfaceD3D11Tex2D);

        if (isDraining == false) {
            sts = ReadRawFrameNV12(cpuFrameInfo, source);
            if (sts != MFX_ERR_NONE) {
                isDraining = true;
            }
            else {
                pTex2D                 = devCtx.GetVideoTexture(cliParams.srcWidth,
                                                cliParams.srcHeight,
                                                cpuFrameInfo.data);
                extSurfD3D11.texture2D = pTex2D.p;

                // Import Surface
                sts = memoryInterface->ImportFrameSurface(memoryInterface,
                                                          MFX_SURFACE_COMPONENT_ENCODE,
                                                          &extSurfD3D11.SurfaceInterface.Header,
                                                          &pmfxEncSurface);
                VERIFY(sts == MFX_ERR_NONE, "ERROR: ImportFrameSurface failed");
            }
        }

        // encode next frame
        mfxSyncPoint syncp = {};

        sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                              NULL,
                                              (isDraining == true) ? NULL : pmfxEncSurface,
                                              &bitstream,
                                              &syncp);
        if (!isDraining) {
            sts_r = pmfxEncSurface->FrameInterface->Release(pmfxEncSurface);
            VERIFY(MFX_ERR_NONE == sts_r, "mfxFrameSurfaceInterface->Release failed");
        }
        switch (sts) {
            case MFX_ERR_NONE:

                // MFX_ERR_NONE and syncp indicate output is available
                if (syncp) {
                    // Encode output is not available on CPU until sync operation
                    // completes
                    do {
                        sts = MFXVideoCORE_SyncOperation(session, syncp, WAIT_100_MILLISECONDS);
                        if (MFX_ERR_NONE == sts) {
                            WriteEncodedStream(bitstream, sink);
                            framenum++;
                        }
                    } while (sts == MFX_WRN_IN_EXECUTION);
                }
                break;
            case MFX_ERR_NOT_ENOUGH_BUFFER:
                printf("ERROR: not enough buffer\n");
                // This example deliberately uses a large output buffer with immediate
                // write to disk for simplicity. Handle when frame size exceeds
                // available buffer here
                break;
            case MFX_ERR_MORE_DATA:
                // The function requires more data to generate any output
                if (isDraining == true)
                    isStillGoing = false;
                break;
            case MFX_ERR_DEVICE_LOST:
                printf("ERROR: device lost\n");
                // For non-CPU implementations,
                // Cleanup if device is lost
                break;
            case MFX_WRN_DEVICE_BUSY:
                printf("ERROR: device busy\n");
                // For non-CPU implementations,
                // Wait a few milliseconds then try again
                break;
            default:
                printf("ERROR: unknown status %d\n", sts);
                isStillGoing = false;
                break;
        }
    }

    printf("Encoded %d frames\n", framenum);

    // Clean up resources - It is recommended to close components first, before
    // releasing allocated surfaces, since some surfaces may still be locked by
    // internal resources.
    if (source)
        fclose(source);

    if (sink)
        fclose(sink);

    MFXVideoENCODE_Close(session);
    MFXClose(session);

    if (bitstream.Data)
        free(bitstream.Data);

    if (loader)
        MFXUnload(loader);

    if (isFailed)
        return -1;

    return 0;
}
