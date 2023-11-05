/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "sample_vpp_utils.h"
#include "sample_utils.h"
#include "vm/time_defs.h"
#include "vpl/mfxvideo++.h"
#include "vpl_implementation_loader.h"

#include "sample_vpp_pts.h"

#include "sysmem_allocator.h"

#ifdef D3D_SURFACES_SUPPORT
    #include "d3d_allocator.h"
    #include "d3d_device.h"

#endif
#ifdef MFX_D3D11_SUPPORT
    #include "d3d11_allocator.h"
    #include "d3d11_device.h"

#endif
#ifdef LIBVA_SUPPORT
    #include "vaapi_allocator.h"
    #include "vaapi_device.h"

#endif

#include <algorithm>
#include "general_allocator.h"
#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo++.h"

#if defined(_WIN64) || defined(_WIN32)
    #include "vpl/mfxadapter.h"
#endif

#define MFX_CHECK_STS(sts)       \
    {                            \
        if (MFX_ERR_NONE != sts) \
            return sts;          \
    }

#undef min

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

/* ******************************************************************* */

static void WipeFrameProcessor(sFrameProcessor* pProcessor);

static void WipeMemoryAllocator(sMemoryAllocator* pAllocator);

void ownToMfxFrameInfo(sOwnFrameInfo* in, mfxFrameInfo* out, bool copyCropParams = false);

/* ******************************************************************* */

static const char* FourCC2Str(mfxU32 FourCC) {
    switch (FourCC) {
        case MFX_FOURCC_NV12:
            return "NV12";
        case MFX_FOURCC_YV12:
            return "YV12";
        case MFX_FOURCC_YUY2:
            return "YUY2";
        case MFX_FOURCC_RGB565:
            return "RGB565";
        case MFX_FOURCC_RGB3:
            return "RGB3";
        case MFX_FOURCC_RGB4:
            return "RGB4";
#if !(defined(_WIN32) || defined(_WIN64))
        case MFX_FOURCC_RGBP:
            return "RGBP";
#endif
        case MFX_FOURCC_YUV400:
            return "YUV400";
        case MFX_FOURCC_YUV411:
            return "YUV411";
        case MFX_FOURCC_YUV422H:
            return "YUV422H";
        case MFX_FOURCC_YUV422V:
            return "YUV422V";
        case MFX_FOURCC_YUV444:
            return "YUV444";
        case MFX_FOURCC_P010:
            return "P010";
        case MFX_FOURCC_P210:
            return "P210";
        case MFX_FOURCC_NV16:
            return "NV16";
        case MFX_FOURCC_A2RGB10:
            return "A2RGB10";
        case MFX_FOURCC_UYVY:
            return "UYVY";
        case MFX_FOURCC_AYUV:
            return "AYUV";
        case MFX_FOURCC_I420:
            return "I420";
        case MFX_FOURCC_Y210:
            return "Y210";
        case MFX_FOURCC_Y410:
            return "Y410";
        case MFX_FOURCC_P016:
            return "P016";
        case MFX_FOURCC_Y216:
            return "Y216";
        case MFX_FOURCC_Y416:
            return "Y416";
        case MFX_FOURCC_I010:
            return "I010";
        default:
            return "Unknown";
    }
}

const char* IOpattern2Str(mfxU32 IOpattern) {
    switch (IOpattern) {
        case MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY:
            return "sys_to_sys";
        case MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY:
            return "sys_to_d3d";
        case MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY:
            return "d3d_to_sys";
        case MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY:
            return "d3d_to_d3d";
        default:
            return "Not defined";
    }
}

/* ******************************************************************* */

//static
const char* PicStruct2Str(mfxU16 PicStruct) {
    switch (PicStruct) {
        case MFX_PICSTRUCT_PROGRESSIVE:
            return "progressive";
        case MFX_PICSTRUCT_FIELD_TFF:
            return "interlace (TFF)";
        case MFX_PICSTRUCT_FIELD_BFF:
            return "interlace (BFF)";
        case MFX_PICSTRUCT_UNKNOWN:
            return "unknown";
        default:
            return "interlace (no detail)";
    }
}

void PrintLibInfo(sFrameProcessor* pProcessor) {
    mfxStatus sts = pProcessor->mfxSession.PrintLibInfo(pProcessor->pLoader.get());
    if (sts != MFX_ERR_NONE)
        printf("mfxSession.PrintLibInfo() failed\n");
    return;
}

/* ******************************************************************* */

void PrintStreamInfo(sInputParams* pParams,
                     mfxVideoParam* pMfxParams,
                     MFXVideoSession* pMfxSession) {
    mfxFrameInfo Info;

    MSDK_CHECK_POINTER_NO_RET(pParams);
    MSDK_CHECK_POINTER_NO_RET(pMfxParams);

    Info = pMfxParams->vpp.In;
    printf("Input format\t%s\n", FourCC2Str(Info.FourCC));
    printf("Resolution\t%dx%d\n", Info.Width, Info.Height);
    printf("Crop X,Y,W,H\t%d,%d,%d,%d\n", Info.CropX, Info.CropY, Info.CropW, Info.CropH);
    printf("Frame rate\t%.2f\n", (double)((mfxF64)Info.FrameRateExtN / Info.FrameRateExtD));
    printf("PicStruct\t%s\n", PicStruct2Str(Info.PicStruct));

    Info = pMfxParams->vpp.Out;
    printf("Output format\t%s\n", FourCC2Str(Info.FourCC));
    printf("Resolution\t%dx%d\n", Info.Width, Info.Height);
    printf("Crop X,Y,W,H\t%d,%d,%d,%d\n", Info.CropX, Info.CropY, Info.CropW, Info.CropH);
    printf("Frame rate\t%.2f\n", (double)((mfxF64)Info.FrameRateExtN / Info.FrameRateExtD));
    printf("PicStruct\t%s\n", PicStruct2Str(Info.PicStruct));

    printf("\n");
    printf("Video Enhancement Algorithms\n");
    printf(
        "Deinterlace\t%s\n",
        (pParams->frameInfoIn[0].PicStruct != pParams->frameInfoOut[0].PicStruct) ? "ON" : "OFF");
    printf("Signal info\t%s\n",
           (VPP_FILTER_DISABLED != pParams->videoSignalInfoParam[0].mode) ? "ON" : "OFF");
    printf("Scaling\t\t%s\n", (VPP_FILTER_DISABLED != pParams->bScaling) ? "ON" : "OFF");
    printf("CromaSiting\t\t%s\n", (VPP_FILTER_DISABLED != pParams->bChromaSiting) ? "ON" : "OFF");
    printf("Denoise\t\t%s\n",
           (VPP_FILTER_DISABLED != pParams->denoiseParam[0].mode) ? "ON" : "OFF");
#ifdef ENABLE_MCTF
    printf("MCTF\t\t%s\n", (VPP_FILTER_DISABLED != pParams->mctfParam[0].mode) ? "ON" : "OFF");
#endif

    printf("ProcAmp\t\t%s\n",
           (VPP_FILTER_DISABLED != pParams->procampParam[0].mode) ? "ON" : "OFF");
    printf("DetailEnh\t%s\n", (VPP_FILTER_DISABLED != pParams->detailParam[0].mode) ? "ON" : "OFF");
    if (VPP_FILTER_DISABLED != pParams->frcParam[0].mode) {
        if (MFX_FRCALGM_FRAME_INTERPOLATION == pParams->frcParam[0].algorithm) {
            printf("FRC:Interp\tON\n");
        }
        else if (MFX_FRCALGM_DISTRIBUTED_TIMESTAMP == pParams->frcParam[0].algorithm) {
            printf("FRC:AdvancedPTS\tON\n");
        }
        else {
            printf("FRC:\t\tON\n");
        }
    }
    //printf("FRC:Advanced\t%s\n",   (VPP_FILTER_DISABLED != pParams->frcParam.mode)  ? "ON": "OFF");
    // MSDK 3.0
    printf("GamutMapping \t%s\n",
           (VPP_FILTER_DISABLED != pParams->gamutParam[0].mode) ? "ON" : "OFF");
    printf("ColorSaturation\t%s\n",
           (VPP_FILTER_DISABLED != pParams->tccParam[0].mode) ? "ON" : "OFF");
    printf("ContrastEnh  \t%s\n",
           (VPP_FILTER_DISABLED != pParams->aceParam[0].mode) ? "ON" : "OFF");
    printf("SkinToneEnh  \t%s\n",
           (VPP_FILTER_DISABLED != pParams->steParam[0].mode) ? "ON" : "OFF");
    printf("MVC mode    \t%s\n",
           (VPP_FILTER_DISABLED != pParams->multiViewParam[0].mode) ? "ON" : "OFF");
    // MSDK 6.0
    printf("ImgStab    \t%s\n",
           (VPP_FILTER_DISABLED != pParams->istabParam[0].mode) ? "ON" : "OFF");
    printf("\n");

    printf("IOpattern type               \t%s\n", IOpattern2Str(pParams->IOPattern));
    printf("Number of asynchronious tasks\t%hu\n", (unsigned short)pParams->asyncNum);
    printf("Time stamps checking         \t%s\n", pParams->ptsCheck ? "ON" : "OFF");

    // info about ROI testing
    if (ROI_FIX_TO_FIX == pParams->roiCheckParam.mode) {
        printf("ROI checking                 \tOFF\n");
    }
    else {
        printf("ROI checking                 \tON (seed1 = %i, seed2 = %i)\n",
               pParams->roiCheckParam.srcSeed,
               pParams->roiCheckParam.dstSeed);
    }

    printf("\n");

    //-------------------------------------------------------
    mfxIMPL impl;
    pMfxSession->QueryIMPL(&impl);
    bool isHWlib = MFX_IMPL_SOFTWARE != impl;

    const char* sImpl = (isHWlib) ? "hw" : "sw";
    printf("MediaSDK impl\t%s", sImpl);

#ifndef LIBVA_SUPPORT
    if (isHWlib || (pParams->vaType & (ALLOC_IMPL_VIA_D3D9 | ALLOC_IMPL_VIA_D3D11))) {
        bool isD3D11       = ((ALLOC_IMPL_VIA_D3D11 == pParams->vaType) ||
                        (pParams->ImpLib == (MFX_IMPL_HARDWARE | MFX_IMPL_VIA_D3D11)))
                                 ? true
                                 : false;
        const char* sIface = (isD3D11) ? "VIA_D3D11" : "VIA_D3D9";
        printf(" | %s", sIface);
    }
#endif
    printf("\n");
    //-------------------------------------------------------

    if (isHWlib && !pParams->bPartialAccel)
        printf("HW accelaration is enabled\n");
    else
        printf("HW accelaration is disabled\n");

#if (defined(_WIN64) || defined(_WIN32)) && (MFX_VERSION >= 1031)
    if (pParams->bPreferdGfx)
        printf("dGfx adapter is preferred\n");

    if (pParams->bPreferiGfx)
        printf("iGfx adapter is preferred\n");
#endif

    mfxVersion ver;
    pMfxSession->QueryVersion(&ver);
    printf("MediaSDK ver\t%d.%d\n", ver.Major, ver.Minor);

    return;
}

/* ******************************************************************* */

mfxStatus InitParamsVPP(MfxVideoParamsWrapper* pParams, sInputParams* pInParams, mfxU32 paramID) {
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(pInParams, MFX_ERR_NULL_PTR);

    if (pInParams->compositionParam.mode != VPP_FILTER_ENABLED_CONFIGURED &&
        (pInParams->frameInfoIn[paramID].nWidth == 0 ||
         pInParams->frameInfoIn[paramID].nHeight == 0)) {
        vppPrintHelp("sample_vpp", "ERROR: Source width is not defined.\n");
        return MFX_ERR_UNSUPPORTED;
    }
    if (pInParams->frameInfoOut[paramID].nWidth == 0 ||
        pInParams->frameInfoOut[paramID].nHeight == 0) {
        vppPrintHelp("sample_vpp", "ERROR: Source height is not defined.\n");
        return MFX_ERR_UNSUPPORTED;
    }
    *pParams = MfxVideoParamsWrapper();
    /* input data */
    pParams->vpp.In.Shift          = pInParams->frameInfoIn[paramID].Shift;
    pParams->vpp.In.BitDepthLuma   = pInParams->frameInfoIn[paramID].BitDepthLuma;
    pParams->vpp.In.BitDepthChroma = pInParams->frameInfoIn[paramID].BitDepthChroma;
    pParams->vpp.In.FourCC         = pInParams->frameInfoIn[paramID].FourCC;
    pParams->vpp.In.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;

    pParams->vpp.In.CropX = pInParams->frameInfoIn[paramID].CropX;
    pParams->vpp.In.CropY = pInParams->frameInfoIn[paramID].CropY;
    pParams->vpp.In.CropW = pInParams->frameInfoIn[paramID].CropW;
    pParams->vpp.In.CropH = pInParams->frameInfoIn[paramID].CropH;

    if (pInParams->ImpLib == MFX_IMPL_SOFTWARE) {
        pParams->vpp.In.Width  = pInParams->frameInfoIn[paramID].nWidth;
        pParams->vpp.In.Height = pInParams->frameInfoIn[paramID].nHeight;
    }
    else {
        pParams->vpp.In.Width = MSDK_ALIGN16(pInParams->frameInfoIn[paramID].nWidth);
        pParams->vpp.In.Height =
            (MFX_PICSTRUCT_PROGRESSIVE == pInParams->frameInfoIn[paramID].PicStruct)
                ? MSDK_ALIGN16(pInParams->frameInfoIn[paramID].nHeight)
                : MSDK_ALIGN32(pInParams->frameInfoIn[paramID].nHeight);
    }

    // width must be a multiple of 16
    // height must be a multiple of 16 in case of frame picture and
    // a multiple of 32 in case of field picture
    mfxU16 maxWidth = 0, maxHeight = 0;
    if (pInParams->compositionParam.mode == VPP_FILTER_ENABLED_CONFIGURED) {
        for (mfxU16 i = 0; i < pInParams->numStreams; i++) {
            pInParams->inFrameInfo[i].nWidth = MSDK_ALIGN16(pInParams->inFrameInfo[i].nWidth);
            pInParams->inFrameInfo[i].nHeight =
                (MFX_PICSTRUCT_PROGRESSIVE == pInParams->inFrameInfo[i].PicStruct)
                    ? MSDK_ALIGN16(pInParams->inFrameInfo[i].nHeight)
                    : MSDK_ALIGN32(pInParams->inFrameInfo[i].nHeight);
            if (pInParams->inFrameInfo[i].nWidth > maxWidth)
                maxWidth = pInParams->inFrameInfo[i].nWidth;
            if (pInParams->inFrameInfo[i].nHeight > maxHeight)
                maxHeight = pInParams->inFrameInfo[i].nHeight;
        }

        pParams->vpp.In.Width  = maxWidth;
        pParams->vpp.In.Height = maxHeight;
        pParams->vpp.In.CropX  = 0;
        pParams->vpp.In.CropY  = 0;
        pParams->vpp.In.CropW  = maxWidth;
        pParams->vpp.In.CropH  = maxHeight;
    }
    pParams->vpp.In.PicStruct = pInParams->frameInfoIn[paramID].PicStruct;

    ConvertFrameRate(pInParams->frameInfoIn[paramID].dFrameRate,
                     &pParams->vpp.In.FrameRateExtN,
                     &pParams->vpp.In.FrameRateExtD);

    /* output data */
    pParams->vpp.Out.Shift          = pInParams->frameInfoOut[paramID].Shift;
    pParams->vpp.Out.BitDepthLuma   = pInParams->frameInfoOut[paramID].BitDepthLuma;
    pParams->vpp.Out.BitDepthChroma = pInParams->frameInfoOut[paramID].BitDepthChroma;
    pParams->vpp.Out.FourCC         = pInParams->frameInfoOut[paramID].FourCC;
    pParams->vpp.Out.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;

    pParams->vpp.Out.CropX = pInParams->frameInfoOut[paramID].CropX;
    pParams->vpp.Out.CropY = pInParams->frameInfoOut[paramID].CropY;
    pParams->vpp.Out.CropW = pInParams->frameInfoOut[paramID].CropW;
    pParams->vpp.Out.CropH = pInParams->frameInfoOut[paramID].CropH;

    if (pInParams->ImpLib == MFX_IMPL_SOFTWARE) {
        pParams->vpp.Out.Width  = pInParams->frameInfoOut[paramID].nWidth;
        pParams->vpp.Out.Height = pInParams->frameInfoOut[paramID].nHeight;
    }
    else {
        // width must be a multiple of 16
        // height must be a multiple of 16 in case of frame picture and
        // a multiple of 32 in case of field picture
        pParams->vpp.Out.Width = MSDK_ALIGN16(pInParams->frameInfoOut[paramID].nWidth);
        pParams->vpp.Out.Height =
            (MFX_PICSTRUCT_PROGRESSIVE == pInParams->frameInfoOut[paramID].PicStruct)
                ? MSDK_ALIGN16(pInParams->frameInfoOut[paramID].nHeight)
                : MSDK_ALIGN32(pInParams->frameInfoOut[paramID].nHeight);
    }

    pParams->vpp.Out.PicStruct = pInParams->frameInfoOut[paramID].PicStruct;

    ConvertFrameRate(pInParams->frameInfoOut[paramID].dFrameRate,
                     &pParams->vpp.Out.FrameRateExtN,
                     &pParams->vpp.Out.FrameRateExtD);

    pParams->IOPattern = pInParams->IOPattern;

    // async depth
    pParams->AsyncDepth = pInParams->asyncNum;

    return MFX_ERR_NONE;
}

#if (defined(_WIN64) || defined(_WIN32)) && (MFX_VERSION >= 1031)
mfxU32 GetPreferredAdapterNum(const mfxAdaptersInfo& adapters, const sInputParams& params) {
    if (adapters.NumActual == 0 || !adapters.Adapters)
        return 0;

    if (params.bPreferdGfx) {
        // Find dGfx adapter in list and return it's index

        auto idx = std::find_if(adapters.Adapters,
                                adapters.Adapters + adapters.NumActual,
                                [](const mfxAdapterInfo info) {
                                    return info.Platform.MediaAdapterType ==
                                           mfxMediaAdapterType::MFX_MEDIA_DISCRETE;
                                });

        // No dGfx in list
        if (idx == adapters.Adapters + adapters.NumActual) {
            printf("Warning: No dGfx detected on machine. Will pick another adapter\n");
            return 0;
        }

        return static_cast<mfxU32>(std::distance(adapters.Adapters, idx));
    }

    if (params.bPreferiGfx) {
        // Find iGfx adapter in list and return it's index

        auto idx = std::find_if(adapters.Adapters,
                                adapters.Adapters + adapters.NumActual,
                                [](const mfxAdapterInfo info) {
                                    return info.Platform.MediaAdapterType ==
                                           mfxMediaAdapterType::MFX_MEDIA_INTEGRATED;
                                });

        // No iGfx in list
        if (idx == adapters.Adapters + adapters.NumActual) {
            printf("Warning: No iGfx detected on machine. Will pick another adapter\n");
            return 0;
        }

        return static_cast<mfxU32>(std::distance(adapters.Adapters, idx));
    }

    // Other ways return 0, i.e. best suitable detected by dispatcher
    return 0;
}

mfxStatus GetImpl(const mfxVideoParam& params, mfxIMPL& impl, const sInputParams& cmd_params) {
    if (!(impl & MFX_IMPL_HARDWARE))
        return MFX_ERR_NONE;

    mfxU32 num_adapters_available;

    mfxStatus sts = MFXQueryAdaptersNumber(&num_adapters_available);
    MSDK_CHECK_STATUS(sts, "MFXQueryAdaptersNumber failed");

    mfxComponentInfo interface_request;
    memset(&interface_request, 0, sizeof(interface_request));
    interface_request.Type             = mfxComponentType::MFX_COMPONENT_VPP;
    interface_request.Requirements.vpp = params.vpp;

    std::vector<mfxAdapterInfo> displays_data(num_adapters_available);
    mfxAdaptersInfo adapters = { displays_data.data(), mfxU32(displays_data.size()), 0u };

    sts = MFXQueryAdapters(&interface_request, &adapters);
    if (sts == MFX_ERR_NOT_FOUND) {
        printf("ERROR: No suitable adapters found for this workload\n");
    }
    MSDK_CHECK_STATUS(sts, "MFXQueryAdapters failed");

    impl &= ~MFX_IMPL_HARDWARE;

    mfxU32 idx = GetPreferredAdapterNum(adapters, cmd_params);
    switch (adapters.Adapters[idx].Number) {
        case 0:
            impl |= MFX_IMPL_HARDWARE;
            break;
        case 1:
            impl |= MFX_IMPL_HARDWARE2;
            break;
        case 2:
            impl |= MFX_IMPL_HARDWARE3;
            break;
        case 3:
            impl |= MFX_IMPL_HARDWARE4;
            break;

        default:
            // Try searching on all display adapters
            impl |= MFX_IMPL_HARDWARE_ANY;
            break;
    }

    return MFX_ERR_NONE;
}
#endif // (defined(_WIN64) || defined(_WIN32)) && (MFX_VERSION >= 1031)

/* ******************************************************************* */

mfxStatus CreateFrameProcessor(sFrameProcessor* pProcessor,
                               mfxVideoParam* pParams,
                               sInputParams* pInParams) {
    mfxStatus sts = MFX_ERR_NONE;
    mfxIMPL impl  = pInParams->ImpLib;

    MSDK_CHECK_POINTER(pProcessor, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);

    WipeFrameProcessor(pProcessor);

    //MFX session
    if (pInParams->verSessionInit == API_1X) {
#if (defined(_WIN64) || defined(_WIN32)) && (MFX_VERSION >= 1031)
        sts = GetImpl(*pParams, impl, *pInParams);
        MSDK_CHECK_STATUS(sts, "GetImpl failed");
#endif
        mfxVersion version = { { 10, 1 } };
        mfxInitParamlWrap initParams;
        initParams.ExternalThreads = 0;
        initParams.GPUCopy         = pInParams->GPUCopyValue;
        initParams.Implementation  = impl;
        initParams.Version         = version;
        initParams.NumExtParam     = 0;
        sts                        = pProcessor->mfxSession.InitEx(initParams);

        MSDK_CHECK_STATUS_SAFE(sts, "pProcessor->mfxSession.Init failed", {
            WipeFrameProcessor(pProcessor);
        });
    }
    else {
        pProcessor->pLoader.reset(new VPLImplementationLoader);

        if (pInParams->dGfxIdx >= 0)
            pProcessor->pLoader->SetDiscreteAdapterIndex(pInParams->dGfxIdx);
        else
            pProcessor->pLoader->SetAdapterType(pInParams->adapterType);

        if (pInParams->adapterNum >= 0)
            pProcessor->pLoader->SetAdapterNum(pInParams->adapterNum);

        if (pInParams->PCIDeviceSetup)
            pProcessor->pLoader->SetPCIDevice(pInParams->PCIDomain,
                                              pInParams->PCIBus,
                                              pInParams->PCIDevice,
                                              pInParams->PCIFunction);

#if (defined(_WIN64) || defined(_WIN32))
        if (pInParams->luid.HighPart > 0 || pInParams->luid.LowPart > 0)
            pProcessor->pLoader->SetupLUID(pInParams->luid);
#else
        pProcessor->pLoader->SetupDRMRenderNodeNum(pInParams->DRMRenderNodeNum);
#endif

        bool bLowLatencyMode = !pInParams->dispFullSearch;

        sts = pProcessor->pLoader->ConfigureAndEnumImplementations(impl,
                                                                   pInParams->accelerationMode,
                                                                   bLowLatencyMode);
        MSDK_CHECK_STATUS(sts, "mfxSession.EnumImplementations failed");
        sts = pProcessor->mfxSession.CreateSession(pProcessor->pLoader.get());
        MSDK_CHECK_STATUS(sts, "m_mfxSession.CreateSession failed");
    }
    // VPP
    pProcessor->pmfxVPP = new MFXVideoVPP(pProcessor->mfxSession);

    return MFX_ERR_NONE;
}

/* ******************************************************************* */

mfxStatus InitFrameProcessor(sFrameProcessor* pProcessor, mfxVideoParam* pParams) {
    mfxStatus sts = MFX_ERR_NONE;

    MSDK_CHECK_POINTER(pProcessor, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(pProcessor->pmfxVPP, MFX_ERR_NULL_PTR);

    // close VPP in case it was initialized
    sts = pProcessor->pmfxVPP->Close();
    MSDK_IGNORE_MFX_STS(sts, MFX_ERR_NOT_INITIALIZED);
    MSDK_CHECK_STATUS(sts, "pProcessor->pmfxVPP->Close failed");

    // init VPP
    sts = pProcessor->pmfxVPP->Init(pParams);
    return sts;
}

/* ******************************************************************* */

mfxStatus InitSurfaces(sMemoryAllocator* pAllocator,
                       mfxFrameAllocRequest* pRequest,
                       bool isInput,
                       int streamIndex) {
    mfxStatus sts = MFX_ERR_NONE;
    mfxU16 nFrames, i;

    mfxFrameAllocResponse& response =
        isInput ? pAllocator->responseIn[streamIndex] : pAllocator->responseOut;
    mfxFrameSurfaceWrap*& pSurfaces =
        isInput ? pAllocator->pSurfacesIn[streamIndex] : pAllocator->pSurfacesOut;

    sts = pAllocator->pMfxAllocator->Alloc(pAllocator->pMfxAllocator->pthis, pRequest, &response);
    MSDK_CHECK_STATUS_SAFE(sts, "pAllocator->pMfxAllocator->Alloc failed", {
        WipeMemoryAllocator(pAllocator);
    });

    nFrames   = response.NumFrameActual;
    pSurfaces = new mfxFrameSurfaceWrap[nFrames];

    for (i = 0; i < nFrames; i++) {
        pSurfaces[i].Info       = pRequest->Info;
        pSurfaces[i].Data.MemId = response.mids[i];
    }

    return sts;
}

/* ******************************************************************* */

mfxStatus InitMemoryAllocator(sFrameProcessor* pProcessor,
                              sMemoryAllocator* pAllocator,
                              mfxVideoParam* pParams,
                              sInputParams* pInParams) {
    mfxStatus sts = MFX_ERR_NONE;
    mfxFrameAllocRequest request[2]; // [0] - in, [1] - out
    //mfxFrameInfo requestFrameInfoRGB;

    MSDK_CHECK_POINTER(pProcessor, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(pAllocator, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(pProcessor->pmfxVPP, MFX_ERR_NULL_PTR);

    MSDK_ZERO_MEMORY(request);

    pAllocator->pMfxAllocator = new GeneralAllocator;

    bool isHWLib = (MFX_IMPL_HARDWARE & pInParams->ImpLib) ? true : false;

    if (isHWLib) {
        if ((pInParams->ImpLib & IMPL_VIA_MASK) == MFX_IMPL_VIA_D3D9) {
#ifdef D3D_SURFACES_SUPPORT
            // prepare device manager
            pAllocator->pDevice = new CD3D9Device();

            mfxU32 adapterNum = (pInParams->verSessionInit == API_1X)
                                    ? MSDKAdapter::GetNumber(pProcessor->mfxSession)
                                    : MSDKAdapter::GetNumber(pProcessor->pLoader.get());

            sts = pAllocator->pDevice->Init(0, 1, adapterNum);
            MSDK_CHECK_STATUS_SAFE(sts,
                                   "pAllocator->pDevice->Init failed",
                                   WipeMemoryAllocator(pAllocator));

            mfxHDL hdl = 0;
            sts        = pAllocator->pDevice->GetHandle(MFX_HANDLE_D3D9_DEVICE_MANAGER, &hdl);
            MSDK_CHECK_STATUS_SAFE(sts,
                                   "pAllocator->pDevice->GetHandle failed",
                                   WipeMemoryAllocator(pAllocator));
            sts = pProcessor->mfxSession.SetHandle(MFX_HANDLE_D3D9_DEVICE_MANAGER, hdl);
            MSDK_CHECK_STATUS_SAFE(sts,
                                   "pAllocator->pDevice->SetHandle failed",
                                   WipeMemoryAllocator(pAllocator));

            // prepare allocator
            D3DAllocatorParams* pd3dAllocParams = new D3DAllocatorParams;

            pd3dAllocParams->pManager    = (IDirect3DDeviceManager9*)hdl;
            pAllocator->pAllocatorParams = pd3dAllocParams;
#endif
        }
        else if ((pInParams->ImpLib & IMPL_VIA_MASK) == MFX_IMPL_VIA_D3D11) {
#if MFX_D3D11_SUPPORT
            pAllocator->pDevice = new CD3D11Device();

            mfxU32 adapterNum = (pInParams->verSessionInit == API_1X)
                                    ? MSDKAdapter::GetNumber(pProcessor->mfxSession)
                                    : MSDKAdapter::GetNumber(pProcessor->pLoader.get());

            sts = pAllocator->pDevice->Init(0, 1, adapterNum);
            MSDK_CHECK_STATUS_SAFE(sts,
                                   "pAllocator->pDevice->Init failed",
                                   WipeMemoryAllocator(pAllocator));

            mfxHDL hdl = 0;
            sts        = pAllocator->pDevice->GetHandle(MFX_HANDLE_D3D11_DEVICE, &hdl);
            MSDK_CHECK_STATUS_SAFE(sts,
                                   "pAllocator->pDevice->GetHandle failed",
                                   WipeMemoryAllocator(pAllocator));
            sts = pProcessor->mfxSession.SetHandle(MFX_HANDLE_D3D11_DEVICE, hdl);
            MSDK_CHECK_STATUS_SAFE(sts,
                                   "pAllocator->pDevice->SetHandle failed",
                                   WipeMemoryAllocator(pAllocator));

            // prepare allocator
            D3D11AllocatorParams* pd3d11AllocParams = new D3D11AllocatorParams;

            pd3d11AllocParams->pDevice   = (ID3D11Device*)hdl;
            pAllocator->pAllocatorParams = pd3d11AllocParams;
#endif
        }
        else if ((pInParams->ImpLib & IMPL_VIA_MASK) == MFX_IMPL_VIA_VAAPI) {
#ifdef LIBVA_SUPPORT
            if (pInParams->strDevicePath.empty() && pInParams->verSessionInit == API_2X) {
                pInParams->strDevicePath =
                    "/dev/dri/renderD" +
                    std::to_string(pProcessor->pLoader->GetDRMRenderNodeNumUsed());
            }

            pAllocator->pDevice = CreateVAAPIDevice(pInParams->strDevicePath);
            MSDK_CHECK_POINTER(pAllocator->pDevice, MFX_ERR_NULL_PTR);

            mfxU32 adapterNum = (pInParams->verSessionInit == API_1X)
                                    ? MSDKAdapter::GetNumber(pProcessor->mfxSession)
                                    : MSDKAdapter::GetNumber(pProcessor->pLoader.get());

            sts = pAllocator->pDevice->Init(0, 1, adapterNum);
            MSDK_CHECK_STATUS_SAFE(sts,
                                   "pAllocator->pDevice->Init failed",
                                   WipeMemoryAllocator(pAllocator));

            mfxHDL hdl = 0;
            sts        = pAllocator->pDevice->GetHandle(MFX_HANDLE_VA_DISPLAY, &hdl);
            MSDK_CHECK_STATUS_SAFE(sts,
                                   "pAllocator->pDevice->GetHandle failed",
                                   WipeMemoryAllocator(pAllocator));
            sts = pProcessor->mfxSession.SetHandle(MFX_HANDLE_VA_DISPLAY, hdl);
            MSDK_CHECK_STATUS_SAFE(sts,
                                   "pAllocator->pDevice->SetHandle failed",
                                   WipeMemoryAllocator(pAllocator));

            // prepare allocator
            vaapiAllocatorParams* pVaapiAllocParams = new vaapiAllocatorParams;

            pVaapiAllocParams->m_dpy     = (VADisplay)hdl;
            pAllocator->pAllocatorParams = pVaapiAllocParams;

#endif
        }
    }
    else {
#ifdef LIBVA_SUPPORT
        //in case of system memory allocator we also have to pass MFX_HANDLE_VA_DISPLAY to HW library
        mfxIMPL impl;
        pProcessor->mfxSession.QueryIMPL(&impl);

        if (MFX_IMPL_HARDWARE == MFX_IMPL_BASETYPE(impl)) {
            if (pInParams->strDevicePath.empty() && pInParams->verSessionInit == API_2X) {
                pInParams->strDevicePath =
                    "/dev/dri/renderD" +
                    std::to_string(pProcessor->pLoader->GetDRMRenderNodeNumUsed());
            }

            pAllocator->pDevice = CreateVAAPIDevice(pInParams->strDevicePath);
            if (!pAllocator->pDevice)
                sts = MFX_ERR_MEMORY_ALLOC;
            MSDK_CHECK_STATUS_SAFE(sts,
                                   "pAllocator->pDevice creation failed",
                                   WipeMemoryAllocator(pAllocator));

            mfxHDL hdl = 0;
            sts        = pAllocator->pDevice->GetHandle(MFX_HANDLE_VA_DISPLAY, &hdl);
            MSDK_CHECK_STATUS_SAFE(sts,
                                   "pAllocator->pDevice->GetHandle failed",
                                   WipeMemoryAllocator(pAllocator));

            sts = pProcessor->mfxSession.SetHandle(MFX_HANDLE_VA_DISPLAY, hdl);
            MSDK_CHECK_STATUS_SAFE(sts,
                                   "pAllocator->pDevice->SetHandle failed",
                                   WipeMemoryAllocator(pAllocator));
        }
#endif
    }
    /* This sample uses external memory allocator model for both system and HW memory */
    sts = pProcessor->mfxSession.SetFrameAllocator(pAllocator->pMfxAllocator);
    MSDK_CHECK_STATUS_SAFE(sts,
                           "pProcessor->mfxSession.SetFrameAllocator failed",
                           WipeMemoryAllocator(pAllocator));
    pAllocator->bUsedAsExternalAllocator = true;

    sts = pAllocator->pMfxAllocator->Init(pAllocator->pAllocatorParams);
    MSDK_CHECK_STATUS_SAFE(sts,
                           "pAllocator->pMfxAllocator->Init failed",
                           WipeMemoryAllocator(pAllocator));

    mfxVideoParam tmpParam = { 0 };
    tmpParam.ExtParam      = pParams->ExtParam;
    tmpParam.NumExtParam   = pParams->NumExtParam;
    sts                    = pProcessor->pmfxVPP->Query(pParams, &tmpParam);
    *pParams               = tmpParam;
    MSDK_CHECK_STATUS_SAFE(sts,
                           "pProcessor->pmfxVPP->Query failed",
                           WipeMemoryAllocator(pAllocator));

    sts = pProcessor->pmfxVPP->QueryIOSurf(pParams, request);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    MSDK_CHECK_STATUS_SAFE(sts,
                           "pProcessor->pmfxVPP->QueryIOSurf failed",
                           WipeMemoryAllocator(pAllocator));

    // alloc frames for vpp
    // [IN]
    // If we have only one input stream - allocate as many surfaces as were requested. Otherwise (in case of composition) - allocate 1 surface per input
    // Modify frame info as well
    if (pInParams->compositionParam.mode != VPP_FILTER_ENABLED_CONFIGURED) {
        sts = InitSurfaces(pAllocator, &(request[VPP_IN]), true, 0);
        MSDK_CHECK_STATUS_SAFE(sts, "InitSurfaces failed", WipeMemoryAllocator(pAllocator));
    }
    else {
        for (int i = 0; i < pInParams->numStreams; i++) {
            ownToMfxFrameInfo(&pInParams->inFrameInfo[i], &request[VPP_IN].Info, true);
            request[VPP_IN].NumFrameSuggested = 1;
            request[VPP_IN].NumFrameMin       = request[VPP_IN].NumFrameSuggested;
            sts = InitSurfaces(pAllocator, &(request[VPP_IN]), true, i);
            MSDK_CHECK_STATUS_SAFE(sts, "InitSurfaces failed", WipeMemoryAllocator(pAllocator));
        }
    }

    // [OUT]
    sts = InitSurfaces(pAllocator, &(request[VPP_OUT]), false, 0);
    MSDK_CHECK_STATUS_SAFE(sts, "InitSurfaces failed", WipeMemoryAllocator(pAllocator));

    return MFX_ERR_NONE;

} // mfxStatus InitMemoryAllocator(...)}

/* ******************************************************************* */

mfxStatus InitResources(sAppResources* pResources,
                        mfxVideoParam* pParams,
                        sInputParams* pInParams) {
    mfxStatus sts = MFX_ERR_NONE;

    MSDK_CHECK_POINTER(pResources, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);
    sts = CreateFrameProcessor(pResources->pProcessor, pParams, pInParams);
    MSDK_CHECK_STATUS_SAFE(sts, "CreateFrameProcessor failed", {
        WipeResources(pResources);
        WipeParams(pInParams);
    });

    sts = InitMemoryAllocator(pResources->pProcessor, pResources->pAllocator, pParams, pInParams);
    MSDK_CHECK_STATUS_SAFE(sts, "InitMemoryAllocator failed", {
        WipeResources(pResources);
        WipeParams(pInParams);
    });

    sts = Config3dlut(pInParams, pResources);
    MSDK_CHECK_STATUS_SAFE(sts, "InitMemoryAllocator failed", {
        WipeResources(pResources);
        WipeParams(pInParams);
    });

    sts = InitFrameProcessor(pResources->pProcessor, pParams);

    if (MFX_WRN_PARTIAL_ACCELERATION == sts || MFX_WRN_FILTER_SKIPPED == sts)
        return sts;
    else {
        MSDK_CHECK_STATUS_SAFE(sts, "InitFrameProcessor failed", {
            WipeResources(pResources);
            WipeParams(pInParams);
        });
    }

    return sts;
}

/* ******************************************************************* */

void WipeFrameProcessor(sFrameProcessor* pProcessor) {
    MSDK_CHECK_POINTER_NO_RET(pProcessor);

    MSDK_SAFE_DELETE(pProcessor->pmfxVPP);

    pProcessor->mfxSession.Close();
}

void WipeMemoryAllocator(sMemoryAllocator* pAllocator) {
    MSDK_CHECK_POINTER_NO_RET(pAllocator);

    for (int i = 0; i < MAX_INPUT_STREAMS; i++) {
        MSDK_SAFE_DELETE_ARRAY(pAllocator->pSurfacesIn[i]);
    }
    //    MSDK_SAFE_DELETE_ARRAY(pAllocator->pSurfaces[VPP_IN_RGB]);
    MSDK_SAFE_DELETE_ARRAY(pAllocator->pSurfacesOut);

    mfxU32 did;
    for (did = 0; did < 8; did++) {
        MSDK_SAFE_DELETE_ARRAY(pAllocator->pSvcSurfaces[did]);
    }

    // delete frames
    if (pAllocator->pMfxAllocator) {
        for (int i = 0; i < MAX_INPUT_STREAMS; i++) {
            if (pAllocator->responseIn[i].NumFrameActual) {
                pAllocator->pMfxAllocator->Free(pAllocator->pMfxAllocator->pthis,
                                                &pAllocator->responseIn[i]);
            }
        }
        pAllocator->pMfxAllocator->Free(pAllocator->pMfxAllocator->pthis, &pAllocator->responseOut);

        for (did = 0; did < 8; did++) {
            pAllocator->pMfxAllocator->Free(pAllocator->pMfxAllocator->pthis,
                                            &pAllocator->svcResponse[did]);
        }
    }

    // delete allocator
    MSDK_SAFE_DELETE(pAllocator->pMfxAllocator);
    MSDK_SAFE_DELETE(pAllocator->pDevice);

    // delete allocator parameters
    MSDK_SAFE_DELETE(pAllocator->pAllocatorParams);

} // void WipeMemoryAllocator(sMemoryAllocator* pAllocator)

void WipeConfigParam(sAppResources* pResources) {
    auto multiViewConfig = pResources->pVppParams->GetExtBuffer<mfxExtMVCSeqDesc>();
    if (multiViewConfig) {
        delete[] multiViewConfig->View;
    }
} // void WipeConfigParam( sAppResources* pResources )

void WipeResources(sAppResources* pResources) {
    MSDK_CHECK_POINTER_NO_RET(pResources);

    if (pResources->pAllocator && pResources->pAllocator->pMfxAllocator &&
        pResources->p3dlutResponse) {
        pResources->pAllocator->pMfxAllocator->FreeFrames(pResources->p3dlutResponse);
        MSDK_SAFE_DELETE(pResources->p3dlutResponse);
    }

    WipeFrameProcessor(pResources->pProcessor);

    WipeMemoryAllocator(pResources->pAllocator);

    for (int i = 0; i < pResources->numSrcFiles; i++) {
        if (pResources->pSrcFileReaders[i]) {
            pResources->pSrcFileReaders[i]->Close();
        }
    }
    pResources->numSrcFiles = 0;

    if (pResources->pDstFileWriters) {
        for (mfxU32 i = 0; i < pResources->dstFileWritersN; i++) {
            pResources->pDstFileWriters[i].Close();
        }
        delete[] pResources->pDstFileWriters;
        pResources->dstFileWritersN = 0;
        pResources->pDstFileWriters = NULL;
    }

    auto compositeConfig = pResources->pVppParams->GetExtBuffer<mfxExtVPPComposite>();
    if (compositeConfig) {
        delete[] compositeConfig->InputStream;
        compositeConfig->InputStream = nullptr;
    }

    WipeConfigParam(pResources);

} // void WipeResources(sAppResources* pResources)

/* ******************************************************************* */

void WipeParams(sInputParams* pParams) {
    pParams->strDstFiles.clear();

} // void WipeParams(sInputParams* pParams)

/* ******************************************************************* */

CRawVideoReader::CRawVideoReader()
        : m_fSrc(NULL),
          m_it(),
          m_SurfacesList(),
          m_isPerfMode(false),
          m_Repeat(0),
          m_pPTSMaker(NULL),
          m_initFcc(0) {}

mfxStatus CRawVideoReader::Init(const char* strFileName, PTSMaker* pPTSMaker, mfxU32 fcc) {
    Close();

    MSDK_CHECK_POINTER(strFileName, MFX_ERR_NULL_PTR);

    MSDK_FOPEN(m_fSrc, strFileName, "rb");
    MSDK_CHECK_POINTER(m_fSrc, MFX_ERR_ABORTED);

    m_pPTSMaker = pPTSMaker;
    m_initFcc   = fcc;
    return MFX_ERR_NONE;
}

CRawVideoReader::~CRawVideoReader() {
    Close();
}

void CRawVideoReader::Close() {
    if (m_fSrc != 0) {
        fclose(m_fSrc);
        m_fSrc = 0;
    }
    m_SurfacesList.clear();
}

mfxStatus CRawVideoReader::LoadNextFrame(mfxFrameData* pData, mfxFrameInfo* pInfo) {
    MSDK_CHECK_POINTER(pData, MFX_ERR_NOT_INITIALIZED);
    MSDK_CHECK_POINTER(pInfo, MFX_ERR_NOT_INITIALIZED);

    // Only (I420|YV12) -> NV12 in-place conversion supported
    if (pInfo->FourCC != m_initFcc &&
        (pInfo->FourCC != MFX_FOURCC_NV12 ||
         (m_initFcc != MFX_FOURCC_I420 && m_initFcc != MFX_FOURCC_YV12))) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    mfxU32 w, h, i, pitch;
    mfxU32 nBytesRead;
    mfxU8* ptr;

    if (pInfo->CropH > 0 && pInfo->CropW > 0) {
        w = pInfo->CropW;
        h = pInfo->CropH;
    }
    else {
        w = pInfo->Width;
        h = pInfo->Height;
    }

    pitch = ((mfxU32)pData->PitchHigh << 16) + pData->PitchLow;

    if (pInfo->FourCC == MFX_FOURCC_YV12 || pInfo->FourCC == MFX_FOURCC_I420) {
        ptr = pData->Y + pInfo->CropX + pInfo->CropY * pitch;

        // read luminance plane
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        w >>= 1;
        h >>= 1;
        pitch >>= 1;
        // load U/V
        ptr = (pInfo->FourCC == MFX_FOURCC_I420 ? pData->U : pData->V) + (pInfo->CropX >> 1) +
              (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
        // load V/U
        ptr = (pInfo->FourCC == MFX_FOURCC_I420 ? pData->V : pData->U) + (pInfo->CropX >> 1) +
              (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_YUV400) {
        ptr = pData->Y + pInfo->CropX + pInfo->CropY * pitch;

        // read luminance plane
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_YUV411) {
        ptr = pData->Y + pInfo->CropX + pInfo->CropY * pitch;

        // read luminance plane
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        w /= 4;

        // load U
        ptr = pData->U + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
        // load V
        ptr = pData->V + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_YUV422H) {
        ptr = pData->Y + pInfo->CropX + pInfo->CropY * pitch;

        // read luminance plane
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        w >>= 1;

        // load U
        ptr = pData->U + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
        // load V
        ptr = pData->V + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_YUV422V) {
        ptr = pData->Y + pInfo->CropX + pInfo->CropY * pitch;

        // read luminance plane
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        h >>= 1;

        // load U
        ptr = pData->U + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
        // load V
        ptr = pData->V + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_YUV444) {
        ptr = pData->Y + pInfo->CropX + pInfo->CropY * pitch;

        // read luminance plane
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        // load U
        ptr = pData->U + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
        // load V
        ptr = pData->V + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_NV12) {
        ptr = pData->Y + pInfo->CropX + pInfo->CropY * pitch;

        // read luminance plane
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        switch (m_initFcc) {
            case MFX_FOURCC_NV12: {
                // load UV
                h >>= 1;
                ptr = pData->UV + pInfo->CropX + (pInfo->CropY >> 1) * pitch;
                for (i = 0; i < h; i++) {
                    nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
                    IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
                }
                break;
            }
            case MFX_FOURCC_I420:
            case MFX_FOURCC_YV12: {
                mfxU8 buf[2048]; // maximum supported chroma width for nv12
                mfxU32 j, dstOffset[2];
                w /= 2;
                h /= 2;
                ptr = pData->UV + pInfo->CropX + (pInfo->CropY / 2) * pitch;
                if (w > 2048) {
                    return MFX_ERR_UNSUPPORTED;
                }

                if (m_initFcc == MFX_FOURCC_I420) {
                    dstOffset[0] = 0;
                    dstOffset[1] = 1;
                }
                else {
                    dstOffset[0] = 1;
                    dstOffset[1] = 0;
                }

                // load first chroma plane: U (input == I420) or V (input == YV12)
                for (i = 0; i < h; i++) {
                    nBytesRead = (mfxU32)fread(buf, 1, w, m_fSrc);
                    if (w != nBytesRead) {
                        return MFX_ERR_MORE_DATA;
                    }
                    for (j = 0; j < w; j++) {
                        ptr[i * pitch + j * 2 + dstOffset[0]] = buf[j];
                    }
                }

                // load second chroma plane: V (input == I420) or U (input == YV12)
                for (i = 0; i < h; i++) {
                    nBytesRead = (mfxU32)fread(buf, 1, w, m_fSrc);

                    if (w != nBytesRead) {
                        return MFX_ERR_MORE_DATA;
                    }
                    for (j = 0; j < w; j++) {
                        ptr[i * pitch + j * 2 + dstOffset[1]] = buf[j];
                    }
                }
                break;
            }
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_NV16) {
        ptr = pData->Y + pInfo->CropX + pInfo->CropY * pitch;

        // read luminance plane
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        // load UV
        ptr = pData->UV + pInfo->CropX + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_I010) {
        ptr = pData->Y;

        // read luminance plane
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w * 2, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w * 2, MFX_ERR_MORE_DATA);
        }

        pitch >>= 1;
        h >>= 1;

        // load U
        ptr = pData->U;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        // load V
        ptr = pData->V;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_P010 || pInfo->FourCC == MFX_FOURCC_P016) {
        ptr = pData->Y + pInfo->CropX * 2 + pInfo->CropY * pitch;

        // read luminance plane
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w * 2, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w * 2, MFX_ERR_MORE_DATA);
        }

        // load UV
        h >>= 1;
        ptr = pData->UV + pInfo->CropX + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w * 2, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w * 2, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_P210) {
        ptr = pData->Y + pInfo->CropX * 2 + pInfo->CropY * pitch;

        // read luminance plane
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w * 2, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w * 2, MFX_ERR_MORE_DATA);
        }

        // load UV
        ptr = pData->UV + pInfo->CropX + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w * 2, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w * 2, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_RGB565) {
        MSDK_CHECK_POINTER(pData->R, MFX_ERR_NOT_INITIALIZED);
        MSDK_CHECK_POINTER(pData->G, MFX_ERR_NOT_INITIALIZED);
        MSDK_CHECK_POINTER(pData->B, MFX_ERR_NOT_INITIALIZED);

        ptr = pData->B;
        ptr = ptr + pInfo->CropX + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, 2 * w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, 2 * w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_RGB3) {
        MSDK_CHECK_POINTER(pData->R, MFX_ERR_NOT_INITIALIZED);
        MSDK_CHECK_POINTER(pData->G, MFX_ERR_NOT_INITIALIZED);
        MSDK_CHECK_POINTER(pData->B, MFX_ERR_NOT_INITIALIZED);

        ptr = std::min(std::min(pData->R, pData->G), pData->B);
        ptr = ptr + pInfo->CropX + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, 3 * w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, 3 * w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_RGB4 || pInfo->FourCC == MFX_FOURCC_A2RGB10) {
        MSDK_CHECK_POINTER(pData->R, MFX_ERR_NOT_INITIALIZED);
        MSDK_CHECK_POINTER(pData->G, MFX_ERR_NOT_INITIALIZED);
        MSDK_CHECK_POINTER(pData->B, MFX_ERR_NOT_INITIALIZED);
        // there is issue with A channel in case of d3d, so A-ch is ignored
        //MSDK_CHECK_POINTER(pData->A, MFX_ERR_NOT_INITIALIZED);

        ptr = std::min(std::min(pData->R, pData->G), pData->B);
        ptr = ptr + pInfo->CropX + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, 4 * w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, 4 * w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_YUY2) {
        ptr = pData->Y + pInfo->CropX + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, 2 * w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, 2 * w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_UYVY) {
        ptr = pData->U + pInfo->CropX + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, 2 * w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, 2 * w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_IMC3) {
        ptr = pData->Y + pInfo->CropX + pInfo->CropY * pitch;

        // read luminance plane
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        h >>= 1;

        // load U
        ptr = pData->V + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
        // load V
        ptr = pData->U + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_AYUV) {
        ptr = std::min(std::min(pData->Y, pData->U), std::min(pData->V, pData->A));
        ptr = ptr + pInfo->CropX + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, 4 * w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, 4 * w, MFX_ERR_MORE_DATA);
        }
    }

    else if (pInfo->FourCC == MFX_FOURCC_Y210 || pInfo->FourCC == MFX_FOURCC_Y216) {
        ptr = (mfxU8*)(pData->Y16 + pInfo->CropX * 2) + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, 4 * w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, 4 * w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_Y410) {
        ptr = (mfxU8*)(pData->Y410 + pInfo->CropX) + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, 4 * w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, 4 * w, MFX_ERR_MORE_DATA);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_Y416) {
        ptr = (mfxU8*)(pData->U16 + pInfo->CropX * 4) + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, 8 * w, m_fSrc);
            IOSTREAM_MSDK_CHECK_NOT_EQUAL(nBytesRead, 8 * w, MFX_ERR_MORE_DATA);
        }
    }
    else {
        return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_NONE;
}

mfxStatus CRawVideoReader::LoadNextFrame(mfxFrameSurface1* pSurface,
                                         int bytes_to_read,
                                         mfxU8* buf_read) {
    // check if reader is initialized
    MSDK_CHECK_POINTER(pSurface, MFX_ERR_NULL_PTR);

    int nBytesRead = static_cast<int>(fread(buf_read, 1, bytes_to_read, m_fSrc));

    if (bytes_to_read != nBytesRead) {
        return MFX_ERR_MORE_DATA;
    }

    mfxU16 w, h;
    mfxFrameInfo* pInfo = &pSurface->Info;
    mfxFrameData* pData = &pSurface->Data;

    w = pInfo->Width;
    h = pInfo->Height;

    switch (pInfo->FourCC) {
        case MFX_FOURCC_NV12:
            pData->Y  = buf_read;
            pData->UV = pData->Y + w * h;
            break;
        case MFX_FOURCC_I420:
            pData->Y = buf_read;
            pData->U = pData->Y + w * h;
            pData->V = pData->U + ((w / 2) * (h / 2));
            break;

        case MFX_FOURCC_P010:
            pData->Y  = buf_read;
            pData->UV = pData->Y + w * 2 * h;
            break;
        case MFX_FOURCC_I010:
            pData->Y = buf_read;
            pData->U = pData->Y + w * 2 * h;
            pData->V = pData->U + (w * (h / 2));
            break;

        case MFX_FOURCC_RGB4:
            // read luminance plane (Y)
            //pitch    = pData->Pitch;
            pData->B = buf_read;
            break;
        default:
            break;
    }

    return MFX_ERR_NONE;
}

mfxStatus CRawVideoReader::GetNextInputFrame(sMemoryAllocator* pAllocator,
                                             mfxFrameInfo* pInfo,
                                             mfxFrameSurfaceWrap** pSurface,
                                             mfxU16 streamIndex) {
    mfxStatus sts;
    if (!m_isPerfMode) {
        sts = GetFreeSurface(pAllocator->pSurfacesIn[streamIndex],
                             pAllocator->responseIn[streamIndex].NumFrameActual,
                             pSurface);
        MSDK_CHECK_STATUS(sts, "GetFreeSurface failed");

        mfxFrameSurfaceWrap* pCurSurf = *pSurface;
        if (pCurSurf->Data.MemId || pAllocator->bUsedAsExternalAllocator) {
            // get YUV pointers
            sts = pAllocator->pMfxAllocator->Lock(pAllocator->pMfxAllocator->pthis,
                                                  pCurSurf->Data.MemId,
                                                  &pCurSurf->Data);
            MFX_CHECK_STS(sts);
            sts = LoadNextFrame(&pCurSurf->Data, pInfo);
            MFX_CHECK_STS(sts);
            sts = pAllocator->pMfxAllocator->Unlock(pAllocator->pMfxAllocator->pthis,
                                                    pCurSurf->Data.MemId,
                                                    &pCurSurf->Data);
            MFX_CHECK_STS(sts);
        }
        else {
            sts = LoadNextFrame(&pCurSurf->Data, pInfo);
            MFX_CHECK_STS(sts);
        }
    }
    else {
        sts = GetPreAllocFrame(pSurface);
        MFX_CHECK_STS(sts);
    }

    if (m_pPTSMaker) {
        if (!m_pPTSMaker->SetPTS(*pSurface))
            return MFX_ERR_UNKNOWN;
    }

    return MFX_ERR_NONE;
}

mfxStatus CRawVideoReader::GetNextInputFrame(sFrameProcessor* pProcessor,
                                             mfxFrameInfo* pInfo,
                                             mfxFrameSurfaceWrap** pSurface,
                                             int bytes_to_read,
                                             mfxU8* buf_read) {
    mfxStatus sts;
    sts = pProcessor->mfxSession.GetSurfaceForVPP((mfxFrameSurface1**)pSurface);
    MSDK_CHECK_STATUS(sts, "GetSurfaceForVPPIn failed");

    // Map makes surface writable by CPU for all implementations
    sts = (*pSurface)->FrameInterface->Map(*pSurface, MFX_MAP_WRITE);
    MSDK_CHECK_STATUS(sts, "mfxFrameSurfaceInterface->Map failed");

    sts = LoadNextFrame(*pSurface, bytes_to_read, buf_read);

    // Unmap/release returns local device access for all implementations
    mfxStatus lsts = (*pSurface)->FrameInterface->Unmap(*pSurface);
    MSDK_CHECK_STATUS(lsts, "mfxFrameSurfaceInterface->Unmap failed");

    lsts = (*pSurface)->FrameInterface->Release(*pSurface);
    MSDK_CHECK_STATUS(lsts, "mfxFrameSurfaceInterface->Release failed");

    return sts;
}

mfxStatus CRawVideoReader::GetPreAllocFrame(mfxFrameSurfaceWrap** pSurface) {
    if (m_it == m_SurfacesList.end()) {
        m_Repeat--;
        m_it = m_SurfacesList.begin();
    }

    if (m_it->Data.Locked)
        return MFX_ERR_ABORTED;

    *pSurface = &(*m_it);
    m_it++;
    if (0 == m_Repeat)
        return MFX_ERR_MORE_DATA;

    return MFX_ERR_NONE;
}

mfxStatus CRawVideoReader::PreAllocateFrameChunk(mfxVideoParam* pVideoParam,
                                                 sInputParams* pParams,
                                                 MFXFrameAllocator* pAllocator) {
    mfxStatus sts;
    mfxFrameAllocRequest request;
    mfxFrameAllocResponse response;
    mfxFrameSurfaceWrap surface;
    m_isPerfMode = true;
    m_Repeat     = pParams->numRepeat;
    request.Info = pVideoParam->vpp.In;
    request.Type =
        (pParams->IOPattern & MFX_IOPATTERN_IN_VIDEO_MEMORY)
            ? (MFX_MEMTYPE_FROM_VPPIN | MFX_MEMTYPE_INTERNAL_FRAME |
               MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET)
            : (MFX_MEMTYPE_FROM_VPPIN | MFX_MEMTYPE_INTERNAL_FRAME | MFX_MEMTYPE_SYSTEM_MEMORY);
    request.NumFrameSuggested = request.NumFrameMin = (mfxU16)pParams->numFrames;
    sts = pAllocator->Alloc(pAllocator, &request, &response);
    MFX_CHECK_STS(sts);
    for (; m_SurfacesList.size() < pParams->numFrames;) {
        surface.Data.Locked = 0;
        surface.Data.MemId  = response.mids[m_SurfacesList.size()];
        surface.Info        = pVideoParam->vpp.In;
        memset(surface.reserved, 0, sizeof(surface.reserved));
        sts = pAllocator->Lock(pAllocator->pthis, surface.Data.MemId, &surface.Data);
        MFX_CHECK_STS(sts);
        sts = LoadNextFrame(&surface.Data, &pVideoParam->vpp.In);
        MFX_CHECK_STS(sts);
        sts = pAllocator->Unlock(pAllocator->pthis, surface.Data.MemId, &surface.Data);
        MFX_CHECK_STS(sts);
        m_SurfacesList.push_back(surface);
    }
    m_it = m_SurfacesList.begin();
    return MFX_ERR_NONE;
}
/* ******************************************************************* */

CRawVideoWriter::CRawVideoWriter() {
    m_fDst               = 0;
    m_pPTSMaker          = 0;
    m_forcedOutputFourcc = 0;
    return;
}

mfxStatus CRawVideoWriter::Init(const char* strFileName,
                                PTSMaker* pPTSMaker,
                                mfxU32 forcedOutputFourcc) {
    Close();

    m_pPTSMaker = pPTSMaker;
    // no need to generate output
    if (0 == strFileName)
        return MFX_ERR_NONE;

    //CHECK_POINTER(strFileName, MFX_ERR_NULL_PTR);

    MSDK_FOPEN(m_fDst, strFileName, "wb");
    MSDK_CHECK_POINTER(m_fDst, MFX_ERR_ABORTED);
    m_forcedOutputFourcc = forcedOutputFourcc;

    return MFX_ERR_NONE;
}

CRawVideoWriter::~CRawVideoWriter() {
    Close();

    return;
}

void CRawVideoWriter::Close() {
    if (m_fDst != 0) {
        fclose(m_fDst);
        m_fDst = 0;
    }

    return;
}

mfxStatus CRawVideoWriter::PutNextFrame(sMemoryAllocator* pAllocator,
                                        mfxFrameInfo* pInfo,
                                        mfxFrameSurfaceWrap* pSurface) {
    mfxStatus sts;
    if (m_fDst) {
        if (pSurface->Data.MemId) {
            // get YUV pointers
            sts = pAllocator->pMfxAllocator->Lock(pAllocator->pMfxAllocator->pthis,
                                                  pSurface->Data.MemId,
                                                  &(pSurface->Data));
            MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, MFX_ERR_ABORTED);

            sts = WriteFrame(&(pSurface->Data), pInfo);
            MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, MFX_ERR_ABORTED);

            sts = pAllocator->pMfxAllocator->Unlock(pAllocator->pMfxAllocator->pthis,
                                                    pSurface->Data.MemId,
                                                    &(pSurface->Data));
            MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, MFX_ERR_ABORTED);
        }
        else {
            sts = WriteFrame(&(pSurface->Data), pInfo);
            MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, MFX_ERR_ABORTED);
        }
    }
    else // performance mode
    {
        if (pSurface->Data.MemId) {
            sts = pAllocator->pMfxAllocator->Lock(pAllocator->pMfxAllocator->pthis,
                                                  pSurface->Data.MemId,
                                                  &(pSurface->Data));
            MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, MFX_ERR_ABORTED);
            sts = pAllocator->pMfxAllocator->Unlock(pAllocator->pMfxAllocator->pthis,
                                                    pSurface->Data.MemId,
                                                    &(pSurface->Data));
            MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, MFX_ERR_ABORTED);
        }
    }
    if (m_pPTSMaker)
        return m_pPTSMaker->CheckPTS(pSurface) ? MFX_ERR_NONE : MFX_ERR_ABORTED;

    return MFX_ERR_NONE;
}

mfxStatus CRawVideoWriter::PutNextFrame(mfxFrameInfo* pInfo, mfxFrameSurfaceWrap* pSurface) {
    mfxStatus sts;
    if (m_fDst) {
        sts = pSurface->FrameInterface->Map(pSurface, MFX_MAP_READ);
        MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, MFX_ERR_ABORTED);

        sts = WriteFrame(&(pSurface->Data), pInfo);
        MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, MFX_ERR_ABORTED);

        sts = pSurface->FrameInterface->Unmap(pSurface);
        MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, MFX_ERR_ABORTED);
    }

    sts = pSurface->FrameInterface->Release(pSurface);
    MSDK_CHECK_NOT_EQUAL(sts, MFX_ERR_NONE, MFX_ERR_ABORTED);

    return sts;
}

mfxStatus CRawVideoWriter::WriteFrame(mfxFrameData* pData, mfxFrameInfo* pInfo) {
    mfxI32 nBytesRead = 0;

    mfxI32 i, pitch;
    mfxU16 h, w;
    mfxU8* ptr;

    MSDK_CHECK_POINTER(pData, MFX_ERR_NOT_INITIALIZED);
    MSDK_CHECK_POINTER(pInfo, MFX_ERR_NOT_INITIALIZED);
    //-------------------------------------------------------
    mfxFrameData outData = *pData;

    if (pInfo->CropH > 0 && pInfo->CropW > 0) {
        w = pInfo->CropW;
        h = pInfo->CropH;
    }
    else {
        w = pInfo->Width;
        h = pInfo->Height;
    }

    pitch = outData.Pitch;

    if (pInfo->FourCC == MFX_FOURCC_YV12 || pInfo->FourCC == MFX_FOURCC_I420) {
        ptr = outData.Y + (pInfo->CropX) + (pInfo->CropY) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }

        w >>= 1;
        h >>= 1;
        pitch >>= 1;

        ptr = (pInfo->FourCC == MFX_FOURCC_I420 ? outData.U : outData.V) + (pInfo->CropX >> 1) +
              (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fwrite(ptr + i * pitch, 1, w, m_fDst);
            MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        ptr = (pInfo->FourCC == MFX_FOURCC_I420 ? outData.V : outData.U) + (pInfo->CropX >> 1) +
              (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_YUV400) {
        ptr = pData->Y + (pInfo->CropX) + (pInfo->CropY) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }

        w >>= 1;
        h >>= 1;
        pitch >>= 1;

        ptr = pData->U + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fwrite(ptr + i * pitch, 1, w, m_fDst);
            MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        ptr = pData->V + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_YUV411) {
        ptr = pData->Y + (pInfo->CropX) + (pInfo->CropY) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }

        w /= 4;
        //pitch /= 4;

        ptr = pData->U + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fwrite(ptr + i * pitch, 1, w, m_fDst);
            MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        ptr = pData->V + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_YUV422H) {
        ptr = pData->Y + (pInfo->CropX) + (pInfo->CropY) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }

        w >>= 1;
        //pitch >>= 1;

        ptr = pData->U + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fwrite(ptr + i * pitch, 1, w, m_fDst);
            MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        ptr = pData->V + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_YUV422V) {
        ptr = pData->Y + (pInfo->CropX) + (pInfo->CropY) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }

        h >>= 1;

        ptr = pData->U + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fwrite(ptr + i * pitch, 1, w, m_fDst);
            MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        ptr = pData->V + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_YUV444) {
        ptr = pData->Y + (pInfo->CropX) + (pInfo->CropY) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }

        ptr = pData->U + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fwrite(ptr + i * pitch, 1, w, m_fDst);
            MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        ptr = pData->V + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_NV12) {
        ptr = pData->Y + (pInfo->CropX) + (pInfo->CropY) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }

        switch (m_forcedOutputFourcc) {
            case MFX_FOURCC_I420: {
                int j = 0;

                // write U plane first, then V plane
                h >>= 1;
                w >>= 1;
                ptr = pData->UV + (pInfo->CropX) + (pInfo->CropY >> 1) * pitch;

                for (i = 0; i < h; i++) {
                    for (j = 0; j < w; j++) {
                        fputc(ptr[i * pitch + j * 2], m_fDst);
                    }
                }
                for (i = 0; i < h; i++) {
                    for (j = 0; j < w; j++) {
                        fputc(ptr[i * pitch + j * 2 + 1], m_fDst);
                    }
                }
            } break;

            case MFX_FOURCC_YV12: {
                int j = 0;

                // write V plane first, then U plane
                h >>= 1;
                w >>= 1;
                ptr = pData->UV + (pInfo->CropX) + (pInfo->CropY >> 1) * pitch;

                for (i = 0; i < h; i++) {
                    for (j = 0; j < w; j++) {
                        fputc(ptr[i * pitch + j * 2 + 1], m_fDst);
                    }
                }
                for (i = 0; i < h; i++) {
                    for (j = 0; j < w; j++) {
                        fputc(ptr[i * pitch + j * 2], m_fDst);
                    }
                }
            } break;

            default: {
                // write UV data
                h >>= 1;
                ptr = pData->UV + (pInfo->CropX) + (pInfo->CropY >> 1) * pitch;

                for (i = 0; i < h; i++) {
                    MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                         w,
                                         MFX_ERR_UNDEFINED_BEHAVIOR);
                }
            } break;
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_NV16) {
        ptr = pData->Y + (pInfo->CropX) + (pInfo->CropY) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }

        // write UV data
        ptr = pData->UV + (pInfo->CropX) + (pInfo->CropY >> 1) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_I010) {
        ptr = pData->Y;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w * 2, m_fDst),
                                 w * 2u,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }

        pitch >>= 1;
        h >>= 1;

        // write U data
        ptr = pData->U;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }

        // write V data
        ptr = pData->V;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_P010 || pInfo->FourCC == MFX_FOURCC_P016) {
        ptr = pData->Y + (pInfo->CropX) + (pInfo->CropY) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w * 2, m_fDst),
                                 w * 2u,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }

        // write UV data
        h >>= 1;
        ptr = pData->UV + (pInfo->CropX) + (pInfo->CropY >> 1) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w * 2, m_fDst),
                                 w * 2u,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_P210) {
        ptr = pData->Y + (pInfo->CropX) + (pInfo->CropY) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w * 2, m_fDst),
                                 w * 2u,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }

        // write UV data
        ptr = pData->UV + (pInfo->CropX) + (pInfo->CropY >> 1) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w * 2, m_fDst),
                                 w * 2u,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_YUY2) {
        ptr = pData->Y + pInfo->CropX + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, 2 * w, m_fDst),
                                 2u * w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_UYVY) {
        ptr = pData->U + pInfo->CropX + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, 2 * w, m_fDst),
                                 2u * w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_IMC3) {
        ptr = pData->Y + (pInfo->CropX) + (pInfo->CropY) * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }

        w >>= 1;
        h >>= 1;

        ptr = pData->U + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            nBytesRead = (mfxU32)fwrite(ptr + i * pitch, 1, w, m_fDst);
            MSDK_CHECK_NOT_EQUAL(nBytesRead, w, MFX_ERR_MORE_DATA);
        }

        ptr = pData->V + (pInfo->CropX >> 1) + (pInfo->CropY >> 1) * pitch;
        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_RGB4 || pInfo->FourCC == MFX_FOURCC_A2RGB10) {
        MSDK_CHECK_POINTER(pData->R, MFX_ERR_NOT_INITIALIZED);
        MSDK_CHECK_POINTER(pData->G, MFX_ERR_NOT_INITIALIZED);
        MSDK_CHECK_POINTER(pData->B, MFX_ERR_NOT_INITIALIZED);
        // there is issue with A channel in case of d3d, so A-ch is ignored
        //MSDK_CHECK_POINTER(pData->A, MFX_ERR_NOT_INITIALIZED);

        ptr = std::min(std::min(pData->R, pData->G), pData->B);
        ptr = ptr + pInfo->CropX + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, 4 * w, m_fDst),
                                 4u * w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
#if !(defined(_WIN32) || defined(_WIN64))
    else if (pInfo->FourCC == MFX_FOURCC_RGBP) {
        MSDK_CHECK_POINTER(pData->R, MFX_ERR_NOT_INITIALIZED);
        MSDK_CHECK_POINTER(pData->G, MFX_ERR_NOT_INITIALIZED);
        MSDK_CHECK_POINTER(pData->B, MFX_ERR_NOT_INITIALIZED);

        ptr = pData->R + pInfo->CropX + pInfo->CropY * pitch;
        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
        ptr = pData->G + pInfo->CropX + pInfo->CropY * pitch;
        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
        ptr = pData->B + pInfo->CropX + pInfo->CropY * pitch;
        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, w, m_fDst),
                                 w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
#endif
    else if (pInfo->FourCC == MFX_FOURCC_AYUV) {
        ptr = std::min(std::min(pData->Y, pData->U), std::min(pData->V, pData->A));
        ptr = ptr + pInfo->CropX + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, 4 * w, m_fDst),
                                 4u * w,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_Y210 || pInfo->FourCC == MFX_FOURCC_Y216) {
        ptr = pData->Y + pInfo->CropX + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, 4 * w, m_fDst),
                                 w * 4u,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_Y410) {
        ptr = (mfxU8*)pData->Y410 + pInfo->CropX + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, 4 * w, m_fDst),
                                 w * 4u,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else if (pInfo->FourCC == MFX_FOURCC_Y416) {
        ptr = (mfxU8*)(pData->U16 + pInfo->CropX * 4) + pInfo->CropY * pitch;

        for (i = 0; i < h; i++) {
            MSDK_CHECK_NOT_EQUAL(fwrite(ptr + i * pitch, 1, 8 * w, m_fDst),
                                 w * 8u,
                                 MFX_ERR_UNDEFINED_BEHAVIOR);
        }
    }
    else {
        return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_NONE;
}

/* ******************************************************************* */

GeneralWriter::GeneralWriter() : m_svcMode(false){};

GeneralWriter::~GeneralWriter() {
    Close();
};

void GeneralWriter::Close() {
    for (mfxU32 did = 0; did < 8; did++) {
        m_ofile[did].reset();
    }
};

mfxStatus GeneralWriter::Init(const char* strFileName,
                              PTSMaker* pPTSMaker,
                              sSVCLayerDescr* pDesc,
                              mfxU32 forcedOutputFourcc) {
    mfxStatus sts = MFX_ERR_UNKNOWN;

    mfxU32 didCount = (pDesc) ? 8 : 1;
    m_svcMode       = (pDesc) ? true : false;

    for (mfxU32 did = 0; did < didCount; did++) {
        if ((1 == didCount) || (pDesc[did].active)) {
            m_ofile[did].reset(new CRawVideoWriter());
            if (0 == m_ofile[did].get()) {
                return MFX_ERR_UNKNOWN;
            }
            char out_buf[MSDK_MAX_FILENAME_LEN * 4 + 20];
            size_t out_buf_size = MSDK_MAX_FILENAME_LEN * 4 + 20;
            char fname[MSDK_MAX_FILENAME_LEN];

#if defined(_WIN32) || defined(_WIN64)
            {
                char drive[MSDK_MAX_FILENAME_LEN];
                char dir[MSDK_MAX_FILENAME_LEN];
                char ext[MSDK_MAX_FILENAME_LEN];

                _splitpath_s(strFileName, drive, dir, fname, ext);

                snprintf(out_buf, out_buf_size, "%s%s%s_layer%i.yuv", drive, dir, fname, (int)did);
            }
#else
            {
                msdk_strncopy_s(fname,
                                MSDK_MAX_FILENAME_LEN,
                                strFileName,
                                MSDK_MAX_FILENAME_LEN - 1);
                fname[MSDK_MAX_FILENAME_LEN - 1] = 0;
                char* pFound                     = strrchr(fname, '.');
                if (pFound) {
                    *pFound = 0;
                }
                snprintf(out_buf, out_buf_size, "%s_layer%i.yuv", fname, (int)did);
            }
#endif

            sts = m_ofile[did]->Init((1 == didCount) ? strFileName : out_buf,
                                     pPTSMaker,
                                     forcedOutputFourcc);

            if (sts != MFX_ERR_NONE)
                break;
        }
    }

    return sts;
};

mfxStatus GeneralWriter::PutNextFrame(sMemoryAllocator* pAllocator,
                                      mfxFrameInfo* pInfo,
                                      mfxFrameSurfaceWrap* pSurface) {
    mfxU32 did = (m_svcMode) ? pSurface->Info.FrameId.DependencyId
                             : 0; //aya: for MVC we have 1 out file only

    mfxStatus sts = m_ofile[did]->PutNextFrame(pAllocator, pInfo, pSurface);

    return sts;
};

mfxStatus GeneralWriter::PutNextFrame(mfxFrameInfo* pInfo, mfxFrameSurfaceWrap* pSurface) {
    mfxU32 did = (m_svcMode) ? pSurface->Info.FrameId.DependencyId
                             : 0; //aya: for MVC we have 1 out file only

    mfxStatus sts = m_ofile[did]->PutNextFrame(pInfo, pSurface);

    return sts;
};

/* ******************************************************************* */

mfxStatus UpdateSurfacePool(mfxFrameInfo SurfacesInfo,
                            mfxU16 nPoolSize,
                            mfxFrameSurfaceWrap* pSurface) {
    MSDK_CHECK_POINTER(pSurface, MFX_ERR_NULL_PTR);
    if (pSurface) {
        for (mfxU16 i = 0; i < nPoolSize; i++) {
            pSurface[i].Info = SurfacesInfo;
        }
    }
    return MFX_ERR_NONE;
}

mfxStatus GetFreeSurface(mfxFrameSurfaceWrap* pSurfacesPool,
                         mfxU16 nPoolSize,
                         mfxFrameSurfaceWrap** ppSurface) {
    MSDK_CHECK_POINTER(pSurfacesPool, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(ppSurface, MFX_ERR_NULL_PTR);

    mfxU32 timeToSleep = 10; // milliseconds
    mfxU32 numSleeps   = MSDK_SURFACE_WAIT_INTERVAL / timeToSleep + 1; // at least 1

    mfxU32 i = 0;

    //wait if there's no free surface
    while ((MSDK_INVALID_SURF_IDX == GetFreeSurfaceIndex(pSurfacesPool, nPoolSize)) &&
           (i < numSleeps)) {
        MSDK_SLEEP(timeToSleep);
        i++;
    }

    mfxU16 index = GetFreeSurfaceIndex(pSurfacesPool, nPoolSize);

    if (index < nPoolSize) {
        *ppSurface = &(pSurfacesPool[index]);
        return MFX_ERR_NONE;
    }

    return MFX_ERR_NOT_ENOUGH_BUFFER;
}

//---------------------------------------------------------

void PrintDllInfo() {
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hCurrent = GetCurrentProcess();
    HMODULE* pModules;
    DWORD cbNeeded;
    int nModules;
    if (NULL == EnumProcessModules(hCurrent, NULL, 0, &cbNeeded))
        return;

    nModules = cbNeeded / sizeof(HMODULE);

    pModules = new HMODULE[nModules];
    if (NULL == pModules) {
        return;
    }
    if (NULL == EnumProcessModules(hCurrent, pModules, cbNeeded, &cbNeeded)) {
        delete[] pModules;
        return;
    }

    for (int i = 0; i < nModules; i++) {
        char buf[2048];
        GetModuleFileName(pModules[i], buf, ARRAYSIZE(buf));
        if (_tcsstr(buf, "libmfx")) {
            printf("MFX dll         %s\n", buf);
        }
    }
    delete[] pModules;
#endif
} // void PrintDllInfo()

/* ******************************************************************* */

/* EOF */
