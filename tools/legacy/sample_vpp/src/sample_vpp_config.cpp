/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "sample_vpp_utils.h"

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

mfxStatus ConfigVideoEnhancementFilters(sInputParams* pParams,
                                        sAppResources* pResources,
                                        mfxU32 paramID) {
    MfxVideoParamsWrapper* pVppParam = pResources->pVppParams;
    mfxU32 enhFilterCount            = 0;

    // [1] video enhancement algorithms can be enabled with default parameters
    if (VPP_FILTER_DISABLED != pParams->denoiseParam[paramID].mode) {
        pResources->tabDoUseAlg[enhFilterCount++] = MFX_EXTBUFF_VPP_DENOISE;
    }
#ifdef ENABLE_MCTF
    if (VPP_FILTER_ENABLED_DEFAULT == pParams->mctfParam[paramID].mode) {
        pResources->tabDoUseAlg[enhFilterCount++] = MFX_EXTBUFF_VPP_MCTF;
    }
#endif
    if (VPP_FILTER_DISABLED != pParams->procampParam[paramID].mode) {
        pResources->tabDoUseAlg[enhFilterCount++] = MFX_EXTBUFF_VPP_PROCAMP;
    }
    if (VPP_FILTER_DISABLED != pParams->detailParam[paramID].mode) {
        pResources->tabDoUseAlg[enhFilterCount++] = MFX_EXTBUFF_VPP_DETAIL;
    }
    // MSDK API 2013
    if (VPP_FILTER_ENABLED_DEFAULT == pParams->istabParam[paramID].mode) {
        pResources->tabDoUseAlg[enhFilterCount++] = MFX_EXTBUFF_VPP_IMAGE_STABILIZATION;
    }
    /*if( VPP_FILTER_DISABLED != pParams->aceParam.mode )
    {
        pResources->tabDoUseAlg[enhFilterCount++] = MFX_EXTBUFF_VPP_AUTO_CONTRAST;
    }
    if( VPP_FILTER_DISABLED != pParams->steParam.mode )
    {
        pResources->tabDoUseAlg[enhFilterCount++] = MFX_EXTBUFF_VPP_SKIN_TONE;
    }
    if( VPP_FILTER_DISABLED != pParams->tccParam.mode )
    {
        pResources->tabDoUseAlg[enhFilterCount++] = MFX_EXTBUFF_VPP_COLOR_SATURATION_LEVEL;
    }*/

    if (VPP_FILTER_DISABLED != pParams->videoSignalInfoIn[paramID].mode) {
        pResources->tabDoUseAlg[enhFilterCount++] = MFX_EXTBUFF_VIDEO_SIGNAL_INFO_IN;
    }

    if (VPP_FILTER_DISABLED != pParams->videoSignalInfoOut[paramID].mode) {
        pResources->tabDoUseAlg[enhFilterCount++] = MFX_EXTBUFF_VIDEO_SIGNAL_INFO_OUT;
    }

    if (enhFilterCount > 0) {
        auto doUse     = pVppParam->AddExtBuffer<mfxExtVPPDoUse>();
        doUse->NumAlg  = enhFilterCount;
        doUse->AlgList = pResources->tabDoUseAlg;
    }

    // [2] video enhancement algorithms can be configured
    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->denoiseParam[paramID].mode) {
        // recommend to use mfxExtVPPDenoise2
        auto denoiseConfig      = pVppParam->AddExtBuffer<mfxExtVPPDenoise2>();
        denoiseConfig->Strength = pParams->denoiseParam[paramID].factor;
        denoiseConfig->Mode     = (mfxDenoiseMode)pParams->denoiseParam[paramID].config;
    }
#ifdef ENABLE_MCTF
    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->mctfParam[paramID].mode) {
        auto mctfConfig            = pVppParam->AddExtBuffer<mfxExtVppMctf>();
        mctfConfig->FilterStrength = pParams->mctfParam[paramID].params.FilterStrength;
    }
#endif
    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->frcParam[paramID].mode) {
        auto frcConfig = pVppParam->AddExtBuffer<mfxExtVPPFrameRateConversion>();
        frcConfig->Algorithm =
            (mfxU16)pParams->frcParam[paramID].algorithm; //MFX_FRCALGM_DISTRIBUTED_TIMESTAMP;
    }

    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->videoSignalInfoParam[paramID].mode) {
        auto videoSignalInfoConfig          = pVppParam->AddExtBuffer<mfxExtVPPVideoSignalInfo>();
        videoSignalInfoConfig->In           = pParams->videoSignalInfoParam[paramID].In;
        videoSignalInfoConfig->Out          = pParams->videoSignalInfoParam[paramID].Out;
        videoSignalInfoConfig->NominalRange = pParams->videoSignalInfoParam[paramID].NominalRange;
        videoSignalInfoConfig->TransferMatrix =
            pParams->videoSignalInfoParam[paramID].TransferMatrix;
    }

    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->mirroringParam[paramID].mode) {
        auto mirroringConfig  = pVppParam->AddExtBuffer<mfxExtVPPMirroring>();
        mirroringConfig->Type = pParams->mirroringParam[paramID].Type;
    }

    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->colorfillParam[paramID].mode) {
        auto colorfillConfig = pVppParam->AddExtBuffer<mfxExtVPPColorFill>();
        colorfillConfig      = &pParams->colorfillParam[paramID];
        std::ignore          = colorfillConfig;
    }

    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->videoSignalInfoIn[paramID].mode) {
        auto videoSignalInfoInConfig             = pVppParam->AddExtBuffer<mfxExtVideoSignalInfo>();
        videoSignalInfoInConfig->Header.BufferId = MFX_EXTBUFF_VIDEO_SIGNAL_INFO_IN;
        videoSignalInfoInConfig->Header.BufferSz = sizeof(mfxExtVideoSignalInfo);
        videoSignalInfoInConfig->VideoFullRange =
            pParams->videoSignalInfoIn[paramID].VideoFullRange;
        videoSignalInfoInConfig->ColourPrimaries =
            pParams->videoSignalInfoIn[paramID].ColourPrimaries;
        videoSignalInfoInConfig->TransferCharacteristics =
            pParams->videoSignalInfoIn[paramID].TransferCharacteristics;
    }

    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->videoSignalInfoOut[paramID].mode) {
        auto videoSignalInfoOutConfig = pVppParam->AddExtBuffer<mfxExtVideoSignalInfo>();
        videoSignalInfoOutConfig->Header.BufferId = MFX_EXTBUFF_VIDEO_SIGNAL_INFO_OUT;
        videoSignalInfoOutConfig->Header.BufferSz = sizeof(mfxExtVideoSignalInfo);
        videoSignalInfoOutConfig->VideoFullRange =
            pParams->videoSignalInfoOut[paramID].VideoFullRange;
        videoSignalInfoOutConfig->ColourPrimaries =
            pParams->videoSignalInfoOut[paramID].ColourPrimaries;
        videoSignalInfoOutConfig->TransferCharacteristics =
            pParams->videoSignalInfoOut[paramID].TransferCharacteristics;
    }

    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->procampParam[paramID].mode) {
        auto procampConfig        = pVppParam->AddExtBuffer<mfxExtVPPProcAmp>();
        procampConfig->Hue        = pParams->procampParam[paramID].hue;
        procampConfig->Saturation = pParams->procampParam[paramID].saturation;
        procampConfig->Contrast   = pParams->procampParam[paramID].contrast;
        procampConfig->Brightness = pParams->procampParam[paramID].brightness;
    }
    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->detailParam[paramID].mode) {
        auto detailConfig          = pVppParam->AddExtBuffer<mfxExtVPPDetail>();
        detailConfig->DetailFactor = pParams->detailParam[paramID].factor;
    }
    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->deinterlaceParam[paramID].mode) {
        auto deinterlaceConfig              = pVppParam->AddExtBuffer<mfxExtVPPDeinterlacing>();
        deinterlaceConfig->Mode             = pParams->deinterlaceParam[paramID].algorithm;
        deinterlaceConfig->TelecinePattern  = pParams->deinterlaceParam[paramID].tc_pattern;
        deinterlaceConfig->TelecineLocation = pParams->deinterlaceParam[paramID].tc_pos;
    }
    if (0 != pParams->rotate[paramID]) {
        auto rotationConfig   = pVppParam->AddExtBuffer<mfxExtVPPRotation>();
        rotationConfig->Angle = pParams->rotate[paramID];
    }
    if (pParams->bScaling) {
        auto scalingConfig                 = pVppParam->AddExtBuffer<mfxExtVPPScaling>();
        scalingConfig->ScalingMode         = pParams->scalingMode;
        scalingConfig->InterpolationMethod = pParams->interpolationMethod;
    }
    if (pParams->bChromaSiting) {
        auto chromaSitingConfig          = pVppParam->AddExtBuffer<mfxExtColorConversion>();
        chromaSitingConfig->ChromaSiting = pParams->uChromaSiting;
    }
    //if( VPP_FILTER_ENABLED_CONFIGURED == pParams->gamutParam.mode )
    //{
    //    pResources->gamutConfig.Header.BufferId = MFX_EXTBUFF_VPP_GAMUT_MAPPING;
    //    pResources->gamutConfig.Header.BufferSz = sizeof(mfxExtVPPGamutMapping);

    //    //pResources->detailConfig.DetailFactor   = pParams->detailParam.factor;
    //    if( pParams->gamutParam.bBT709 )
    //    {
    //        pResources->gamutConfig.InTransferMatrix  = MFX_TRASNFERMATRIX_XVYCC_BT709;
    //        pResources->gamutConfig.OutTransferMatrix = MFX_TRANSFERMATRIX_BT709;
    //    }
    //    else
    //    {
    //        pResources->gamutConfig.InTransferMatrix  = MFX_TRANSFERMATRIX_XVYCC_BT601;
    //        pResources->gamutConfig.OutTransferMatrix = MFX_TRANSFERMATRIX_BT601;
    //    }

    //    pVppParam->ExtParam[pVppParam->NumExtParam++] = (mfxExtBuffer*)&(pResources->gamutConfig);
    //}
    // MSDK 1.5 -------------------------------------------
    //if( VPP_FILTER_ENABLED_CONFIGURED == pParams->tccParam.mode )
    //{
    //    pResources->tccConfig.Header.BufferId = MFX_EXTBUFF_VPP_COLOR_SATURATION_LEVEL;
    //    pResources->tccConfig.Header.BufferSz = sizeof(mfxExtVPPColorSaturationLevel);

    //    pResources->tccConfig.Red     = pParams->tccParam.Red;
    //    pResources->tccConfig.Green   = pParams->tccParam.Green;
    //    pResources->tccConfig.Blue    = pParams->tccParam.Blue;
    //    pResources->tccConfig.Magenta = pParams->tccParam.Magenta;
    //    pResources->tccConfig.Yellow  = pParams->tccParam.Yellow;
    //    pResources->tccConfig.Cyan    = pParams->tccParam.Cyan;

    //    pVppParam->ExtParam[pVppParam->NumExtParam++] = (mfxExtBuffer*)&(pResources->tccConfig);
    //}
    //if( VPP_FILTER_ENABLED_CONFIGURED == pParams->aceParam.mode )
    //{
    //    // to do
    //}
    //if( VPP_FILTER_ENABLED_CONFIGURED == pParams->steParam.mode )
    //{
    //    pResources->steConfig.Header.BufferId = MFX_EXTBUFF_VPP_SKIN_TONE;
    //    pResources->steConfig.Header.BufferSz = sizeof(mfxExtVPPSkinTone);
    //    pResources->steConfig.SkinToneFactor  = pParams->steParam.SkinToneFactor;

    //    pVppParam->ExtParam[pVppParam->NumExtParam++] = (mfxExtBuffer*)&(pResources->steConfig);
    //}
    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->istabParam[paramID].mode) {
        auto istabConfig  = pVppParam->AddExtBuffer<mfxExtVPPImageStab>();
        istabConfig->Mode = pParams->istabParam[paramID].istabMode;
    }

    // ----------------------------------------------------
    // MVC
    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->multiViewParam[paramID].mode) {
        auto multiViewConfig     = pVppParam->AddExtBuffer<mfxExtMVCSeqDesc>();
        multiViewConfig->NumView = pParams->multiViewParam[paramID].viewCount;
        multiViewConfig->View =
            new mfxMVCViewDependency[pParams->multiViewParam[paramID].viewCount];

        ViewGenerator viewGenerator(pParams->multiViewParam[paramID].viewCount);

        for (mfxU16 viewIndx = 0; viewIndx < pParams->multiViewParam[paramID].viewCount;
             viewIndx++) {
            multiViewConfig->View[viewIndx].ViewId = viewGenerator.GetNextViewID();
        }
    }

    // Composition
    if (VPP_FILTER_ENABLED_CONFIGURED == pParams->compositionParam.mode) {
        auto compositeConfig            = pVppParam->AddExtBuffer<mfxExtVPPComposite>();
        compositeConfig->NumInputStream = pParams->numStreams;
        compositeConfig->InputStream = new mfxVPPCompInputStream[compositeConfig->NumInputStream]();

        for (int i = 0; i < compositeConfig->NumInputStream; i++) {
            compositeConfig->InputStream[i].DstX =
                pParams->compositionParam.streamInfo[i].compStream.DstX;
            compositeConfig->InputStream[i].DstY =
                pParams->compositionParam.streamInfo[i].compStream.DstY;
            compositeConfig->InputStream[i].DstW =
                pParams->compositionParam.streamInfo[i].compStream.DstW;
            compositeConfig->InputStream[i].DstH =
                pParams->compositionParam.streamInfo[i].compStream.DstH;
            if (pParams->compositionParam.streamInfo[i].compStream.GlobalAlphaEnable != 0) {
                compositeConfig->InputStream[i].GlobalAlphaEnable =
                    pParams->compositionParam.streamInfo[i].compStream.GlobalAlphaEnable;
                compositeConfig->InputStream[i].GlobalAlpha =
                    pParams->compositionParam.streamInfo[i].compStream.GlobalAlpha;
            }
            if (pParams->compositionParam.streamInfo[i].compStream.LumaKeyEnable != 0) {
                compositeConfig->InputStream[i].LumaKeyEnable =
                    pParams->compositionParam.streamInfo[i].compStream.LumaKeyEnable;
                compositeConfig->InputStream[i].LumaKeyMin =
                    pParams->compositionParam.streamInfo[i].compStream.LumaKeyMin;
                compositeConfig->InputStream[i].LumaKeyMax =
                    pParams->compositionParam.streamInfo[i].compStream.LumaKeyMax;
            }
            if (pParams->compositionParam.streamInfo[i].compStream.PixelAlphaEnable != 0) {
                compositeConfig->InputStream[i].PixelAlphaEnable =
                    pParams->compositionParam.streamInfo[i].compStream.PixelAlphaEnable;
            }
        } // for (int i = 0; i < compositeConfig->NumInputStream; i++)
    }

    return MFX_ERR_NONE;

} // mfxStatus ConfigVideoEnhancementFilters( sAppResources* pResources, mfxVideoParam* pParams )

mfxStatus Config3dlut(sInputParams* pParams, sAppResources* pResources) {
    mfxStatus sts = MFX_ERR_NONE;
#if defined(_WIN32) || defined(_WIN64)
    MfxVideoParamsWrapper* pVppParam = pResources->pVppParams;
    mfxU32 seg_size                  = pParams->lutSize;
    mfxU32 mul_size                  = seg_size * 2 - 2;
    mfxU32 n3DLutVWidth              = seg_size * 8;
    mfxU32 n3DLutVHeight             = seg_size * mul_size;
    // 17x17x32 33x33x64 65x65x128
    mfxU32 dim[3] = { seg_size, seg_size, mul_size };

    if (pParams->b3dLut) {
        FILE* file;
        MSDK_FOPEN(file, pParams->lutTableFile, "rb");
        if (!file)
            return MFX_ERR_NULL_PTR;
        fseek(file, 0, SEEK_END);
        mfxU32 lutTblSize = ftell(file);
        rewind(file);

        pParams->lutTbl.resize(lutTblSize);
        if (!pParams->lutTbl.data())
            return MFX_ERR_NULL_PTR;
        fread(pParams->lutTbl.data(), 1, lutTblSize, file);
        fclose(file);
        file = NULL;

        if (lutTblSize != n3DLutVHeight * n3DLutVWidth)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        auto lutConfig            = pVppParam->AddExtBuffer<mfxExtVPP3DLut>();
        lutConfig->ChannelMapping = MFX_3DLUT_CHANNEL_MAPPING_RGB_RGB;

        if (pParams->bIs3dLutVideoMem) {
    #if defined(_WIN32) || defined(_WIN64)
            auto pAllocator = pResources->pAllocator->pMfxAllocator;

            mfxFrameAllocRequest request3dlut = {};

            request3dlut.Info.FourCC       = MFX_FOURCC_P8;
            request3dlut.Info.Width        = n3DLutVWidth;
            request3dlut.Info.Height       = n3DLutVHeight;
            request3dlut.NumFrameSuggested = 1;
            request3dlut.NumFrameMin       = 1;
            request3dlut.Type = MFX_MEMTYPE_FROM_VPPIN | MFX_MEMTYPE_VIDEO_MEMORY_PROCESSOR_TARGET;

            pResources->p3dlutResponse = new mfxFrameAllocResponse;
            auto pResponseOut          = pResources->p3dlutResponse;

            sts = pAllocator->Alloc(pAllocator->pthis, &request3dlut, pResponseOut);
            MSDK_CHECK_STATUS(sts, "Alloc failed!");
            mfxU16 nFrames = pResponseOut->NumFrameActual;
            if (nFrames != 1)
                return MFX_ERR_MEMORY_ALLOC;

            mfxFrameSurface1 pSurfacesOut = {};
            sts = pAllocator->Lock(pAllocator->pthis, pResponseOut->mids[0], &(pSurfacesOut.Data));
            MSDK_CHECK_STATUS(sts, "Lock failed!");

            memcpy(pSurfacesOut.Data.Y, pParams->lutTbl.data(), lutTblSize);

            sts = pAllocator->Unlock(pAllocator->pthis, pResponseOut->mids[0], NULL);
            MSDK_CHECK_STATUS(sts, "Unlock failed!");

            mfxHDLPair pair;
            mfxHDL* hdl = &(pair.first);
            sts         = pAllocator->GetHDL(pAllocator->pthis, pResponseOut->mids[0], hdl);
            MSDK_CHECK_STATUS(sts, "GetHDL failed!");
            ID3D11Texture2D* texture = (ID3D11Texture2D*)((mfxHDLPair*)(hdl))->first;

            lutConfig->BufferType            = MFX_RESOURCE_DX11_TEXTURE;
            lutConfig->VideoBuffer.DataType  = MFX_DATA_TYPE_U16;
            lutConfig->VideoBuffer.MemLayout = MFX_3DLUT_MEMORY_LAYOUT_INTEL_65LUT;
            lutConfig->VideoBuffer.MemId     = (ID3D11Texture2D*)texture;
    #else
    #endif
        }
        else {
            mfxU32 size  = dim[0] * dim[1] * dim[2];
            mfxU16* data = (mfxU16*)pParams->lutTbl.data();
            if (!data)
                return MFX_ERR_NULL_PTR;
            mfxU32 index = 0;
            pParams->RGB[0].reset(new mfxU16[size]);
            pParams->RGB[1].reset(new mfxU16[size]);
            pParams->RGB[2].reset(new mfxU16[size]);
            for (mfxU32 i = 0; i < dim[0]; i++) {
                for (mfxU32 j = 0; j < dim[1]; j++) {
                    for (mfxU32 k = 0; k < dim[2]; k++) {
                        mfxU32 pos             = (i * dim[1] * dim[2] + j * dim[2] + k) * 4;
                        pParams->RGB[0][index] = data[pos++];
                        pParams->RGB[1][index] = data[pos++];
                        pParams->RGB[2][index] = data[pos++];
                        index++;
                    }
                }
            }

            lutConfig->BufferType                       = MFX_RESOURCE_SYSTEM_SURFACE;
            lutConfig->SystemBuffer.Channel[0].Data16   = pParams->RGB[0].get();
            lutConfig->SystemBuffer.Channel[0].Size     = dim[0];
            lutConfig->SystemBuffer.Channel[0].DataType = MFX_DATA_TYPE_U16;
            lutConfig->SystemBuffer.Channel[1].Data16   = pParams->RGB[1].get();
            lutConfig->SystemBuffer.Channel[1].Size     = dim[1];
            lutConfig->SystemBuffer.Channel[0].DataType = MFX_DATA_TYPE_U16;
            lutConfig->SystemBuffer.Channel[2].Data16   = pParams->RGB[2].get();
            lutConfig->SystemBuffer.Channel[2].Size     = dim[2];
            lutConfig->SystemBuffer.Channel[0].DataType = MFX_DATA_TYPE_U16;
        }
    }
#endif
    return sts;
}

/* EOF */
