/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "mfx_samples_config.h"

#if defined(_WIN32) || defined(_WIN64)

    #include "sample_defs.h"

    #if MFX_D3D11_SUPPORT

        #include "d3d11_device.h"

CD3D11Device::CD3D11Device()
        : m_pD3D11Device(),
          m_pD3D11Ctx(),
          m_pDX11VideoDevice(),
          m_pVideoContext(),
          m_VideoProcessorEnum(),
          m_pDXGIDev(),
          m_pAdapter(),
          m_pAdapter1(),
          m_pDXGIFactory(),
          m_pSwapChain(),
          m_pSwapChain3(),
          m_pSwapChain4(),
          m_pVideoProcessor(),
          m_pInputViewLeft(),
          m_pInputViewRight(),
          m_pOutputView(),
          m_pDXGIBackBuffer(),
          m_pTempTexture(),
          m_pDisplayControl(),
          m_pDXGIOutput(),
          m_nViews(0),
          m_nPrimaryWidth(0),
          m_nPrimaryHeight(0),
          m_bDefaultStereoEnabled(FALSE),
          m_bIsA2rgb10(FALSE),
          m_bHdrSupport(FALSE),
          m_HandleWindow(NULL),
          m_bDxgiFs(FALSE),
          m_pColorSpaceDataTemp(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709),
          m_pHDRMetaDataTemp({}) {}

CD3D11Device::~CD3D11Device() {
    Close();
}

mfxStatus CD3D11Device::FillSCD(mfxHDL hWindow, DXGI_SWAP_CHAIN_DESC& scd) {
    scd.Windowed         = TRUE;
    scd.OutputWindow     = (HWND)hWindow;
    scd.SampleDesc.Count = 1;
    scd.BufferDesc.Format =
        (m_bIsA2rgb10) ? DXGI_FORMAT_R10G10B10A2_UNORM : DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 1;

    return MFX_ERR_NONE;
}

mfxStatus CD3D11Device::FillSCD1(DXGI_SWAP_CHAIN_DESC1& scd1) {
    scd1.Width  = 0; // Use automatic sizing.
    scd1.Height = 0;
    scd1.Format = (m_bIsA2rgb10) ? DXGI_FORMAT_R10G10B10A2_UNORM : DXGI_FORMAT_B8G8R8A8_UNORM;
    scd1.Stereo = m_nViews == 2 ? TRUE : FALSE;
    scd1.SampleDesc.Count   = 1; // Don't use multi-sampling.
    scd1.SampleDesc.Quality = 0;
    scd1.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd1.BufferCount        = 2; // Use double buffering to minimize latency.
    scd1.Scaling            = DXGI_SCALING_STRETCH;
    scd1.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd1.Flags              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    return MFX_ERR_NONE;
}

mfxStatus CD3D11Device::Init(mfxHDL hWindow, mfxU16 nViews, mfxU32 nAdapterNum) {
    m_HandleWindow = (HWND)hWindow;
    mfxStatus sts  = MFX_ERR_NONE;
    HRESULT hres   = S_OK;
    m_nViews       = nViews;
    if (2 < nViews)
        return MFX_ERR_UNSUPPORTED;
    m_bDefaultStereoEnabled = FALSE;

    static D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_1,
                                                 D3D_FEATURE_LEVEL_11_0,
                                                 D3D_FEATURE_LEVEL_10_1,
                                                 D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL pFeatureLevelsOut;

    hres = CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)(&m_pDXGIFactory));
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    if (m_nViews == 2) {
        hres = m_pDXGIFactory->QueryInterface(__uuidof(IDXGIDisplayControl),
                                              (void**)&m_pDisplayControl);
        if (FAILED(hres))
            return MFX_ERR_DEVICE_FAILED;

        m_bDefaultStereoEnabled = m_pDisplayControl->IsStereoEnabled();
        if (!m_bDefaultStereoEnabled)
            m_pDisplayControl->SetStereoEnabled(TRUE);
    }

    hres = m_pDXGIFactory->EnumAdapters(nAdapterNum, &m_pAdapter);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    hres = m_pDXGIFactory->EnumAdapters1(0, &m_pAdapter1);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    // Obtain primary display
    hres = m_pAdapter1->EnumOutputs(0, &m_pDXGIOutput);
    if (!FAILED(hres)) {
        hres = m_pDXGIOutput->QueryInterface(__uuidof(IDXGIOutput6), (void**)&m_pDXGIOutput6);
        if (FAILED(hres))
            return MFX_ERR_DEVICE_FAILED;

        DXGI_OUTPUT_DESC1 outDesc1 = {};
        hres                       = m_pDXGIOutput6->GetDesc1(&outDesc1);
        if (!FAILED(hres)) {
            // Obtain display resolution for -dxgiFs
            m_nPrimaryWidth  = outDesc1.DesktopCoordinates.right - outDesc1.DesktopCoordinates.left;
            m_nPrimaryHeight = outDesc1.DesktopCoordinates.bottom - outDesc1.DesktopCoordinates.top;

            if (outDesc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) {
                m_bIsA2rgb10  = TRUE;
                m_bHdrSupport = TRUE;
            }
        }
    }

    hres = D3D11CreateDevice(m_pAdapter,
                             D3D_DRIVER_TYPE_UNKNOWN,
                             NULL,
                             0,
                             FeatureLevels,
                             MSDK_ARRAY_LEN(FeatureLevels),
                             D3D11_SDK_VERSION,
                             &m_pD3D11Device,
                             &pFeatureLevelsOut,
                             &m_pD3D11Ctx);

    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    m_pDXGIDev         = m_pD3D11Device;
    m_pDX11VideoDevice = m_pD3D11Device;
    m_pVideoContext    = m_pD3D11Ctx;

    MSDK_CHECK_POINTER(m_pDXGIDev.p, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(m_pDX11VideoDevice.p, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(m_pVideoContext.p, MFX_ERR_NULL_PTR);

    // turn on multithreading for the Context
    CComQIPtr<ID3D10Multithread> p_mt(m_pVideoContext);

    if (p_mt)
        p_mt->SetMultithreadProtected(true);
    else
        return MFX_ERR_DEVICE_FAILED;

    // create swap chain only for rendering use case (hWindow != 0)
    if (hWindow) {
        MSDK_CHECK_POINTER(m_pDXGIFactory.p, MFX_ERR_NULL_PTR);
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

        sts = FillSCD1(swapChainDesc);
        MSDK_CHECK_STATUS(sts, "FillSCD1 failed");

        hres = m_pDXGIFactory->CreateSwapChainForHwnd(
            m_pD3D11Device,
            (HWND)hWindow,
            &swapChainDesc,
            NULL,
            NULL,
            reinterpret_cast<IDXGISwapChain1**>(&m_pSwapChain));
        if (FAILED(hres))
            return MFX_ERR_DEVICE_FAILED;
    }

    return sts;
}

mfxStatus CD3D11Device::CreateVideoProcessor(mfxFrameSurface1* pSrf) {
    HRESULT hres = S_OK;

    if (m_VideoProcessorEnum.p || NULL == pSrf)
        return MFX_ERR_NONE;

    //create video processor
    D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc = {};
    ContentDesc.InputFrameFormat                   = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
    ContentDesc.InputFrameRate.Numerator           = 30000;
    ContentDesc.InputFrameRate.Denominator         = 1000;
    ContentDesc.InputWidth                         = pSrf->Info.CropW;
    ContentDesc.InputHeight                        = pSrf->Info.CropH;
    ContentDesc.OutputWidth                        = pSrf->Info.CropW;
    ContentDesc.OutputHeight                       = pSrf->Info.CropH;
    ContentDesc.OutputFrameRate.Numerator          = 30000;
    ContentDesc.OutputFrameRate.Denominator        = 1000;

    ContentDesc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

    hres = m_pDX11VideoDevice->CreateVideoProcessorEnumerator(&ContentDesc, &m_VideoProcessorEnum);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    hres = m_pDX11VideoDevice->CreateVideoProcessor(m_VideoProcessorEnum, 0, &m_pVideoProcessor);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    return MFX_ERR_NONE;
}

mfxStatus CD3D11Device::Reset() {
    // Changing video mode back to the original state
    if (2 == m_nViews && !m_bDefaultStereoEnabled)
        m_pDisplayControl->SetStereoEnabled(FALSE);

    MSDK_CHECK_POINTER(m_pDXGIFactory.p, MFX_ERR_NULL_PTR);
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

    mfxStatus sts = FillSCD1(swapChainDesc);
    MSDK_CHECK_STATUS(sts, "FillSCD1 failed");

    HRESULT hres = S_OK;
    hres =
        m_pDXGIFactory->CreateSwapChainForHwnd(m_pD3D11Device,
                                               (HWND)m_HandleWindow,
                                               &swapChainDesc,
                                               NULL,
                                               NULL,
                                               reinterpret_cast<IDXGISwapChain1**>(&m_pSwapChain));

    if (FAILED(hres)) {
        if (swapChainDesc.Stereo) {
            MSDK_PRINT_RET_MSG(
                MFX_ERR_DEVICE_FAILED,
                "Cannot create swap chain required for rendering. Possibly stereo mode is not supported.");
        }
        else {
            MSDK_PRINT_RET_MSG(MFX_ERR_DEVICE_FAILED,
                               "Cannot create swap chain required for rendering.");
        }

        return MFX_ERR_DEVICE_FAILED;
    }

    m_pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain4));
    m_pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain3));

    if (m_bDxgiFs) {
        DXGI_SWAP_CHAIN_DESC desc = {};
        hres                      = m_pSwapChain->GetDesc(&desc);
        if (FAILED(hres))
            return MFX_ERR_DEVICE_FAILED;

        hres = m_pSwapChain->SetFullscreenState(TRUE, nullptr);
        if (FAILED(hres))
            return MFX_ERR_DEVICE_FAILED;

        hres = m_pSwapChain->ResizeBuffers(2,
                                           m_nPrimaryWidth,
                                           m_nPrimaryHeight,
                                           desc.BufferDesc.Format,
                                           desc.Flags);
        if (FAILED(hres))
            return MFX_ERR_DEVICE_FAILED;
    }

    return MFX_ERR_NONE;
}

mfxStatus CD3D11Device::GetHandle(mfxHandleType type, mfxHDL* pHdl) {
    if (MFX_HANDLE_D3D11_DEVICE == type) {
        *pHdl = m_pD3D11Device.p;
        return MFX_ERR_NONE;
    }
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus CD3D11Device::SetHandle(mfxHandleType type, mfxHDL hdl) {
    if (MFX_HANDLE_DEVICEWINDOW == type && hdl != NULL) //for render window handle
    {
        m_HandleWindow = (HWND)hdl;
        return MFX_ERR_NONE;
    }
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus CD3D11Device::RenderFrame(mfxFrameSurface1* pSrf, mfxFrameAllocator* pAlloc) {
    HRESULT hres = S_OK;
    mfxStatus sts;

    sts = CreateVideoProcessor(pSrf);
    MSDK_CHECK_STATUS(sts, "CreateVideoProcessor failed");

    hres = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_pDXGIBackBuffer.p);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC OutputViewDesc;
    if (2 == m_nViews) {
        m_pVideoContext->VideoProcessorSetStreamStereoFormat(
            m_pVideoProcessor,
            0,
            TRUE,
            D3D11_VIDEO_PROCESSOR_STEREO_FORMAT_SEPARATE,
            TRUE,
            TRUE,
            D3D11_VIDEO_PROCESSOR_STEREO_FLIP_NONE,
            NULL);
        m_pVideoContext->VideoProcessorSetOutputStereoMode(m_pVideoProcessor, TRUE);

        OutputViewDesc.ViewDimension                  = D3D11_VPOV_DIMENSION_TEXTURE2DARRAY;
        OutputViewDesc.Texture2DArray.ArraySize       = 2;
        OutputViewDesc.Texture2DArray.MipSlice        = 0;
        OutputViewDesc.Texture2DArray.FirstArraySlice = 0;
    }
    else {
        OutputViewDesc.ViewDimension      = D3D11_VPOV_DIMENSION_TEXTURE2D;
        OutputViewDesc.Texture2D.MipSlice = 0;
    }

    if (1 == m_nViews || 0 == pSrf->Info.FrameId.ViewId) {
        hres = m_pDX11VideoDevice->CreateVideoProcessorOutputView(m_pDXGIBackBuffer,
                                                                  m_VideoProcessorEnum,
                                                                  &OutputViewDesc,
                                                                  &m_pOutputView.p);
        if (FAILED(hres))
            return MFX_ERR_DEVICE_FAILED;
    }

    if (m_pSwapChain3) {
        if (pSrf->Info.FourCC == MFX_FOURCC_P010) {
            DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
        #if (MFX_VERSION >= 2006)
            mfxExtMasteringDisplayColourVolume* displayColor =
                (mfxExtMasteringDisplayColourVolume*)GetExtBuffer(
                    pSrf->Data.ExtParam,
                    pSrf->Data.NumExtParam,
                    MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME);
            mfxExtContentLightLevelInfo* contentLight =
                (mfxExtContentLightLevelInfo*)GetExtBuffer(pSrf->Data.ExtParam,
                                                           pSrf->Data.NumExtParam,
                                                           MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO);

            if (m_bHdrSupport && (displayColor && contentLight)) {
                // panel support HDR. check if bitstream support HDR.
                if (displayColor->InsertPayloadToggle == MFX_PAYLOAD_IDR ||
                    contentLight->InsertPayloadToggle == MFX_PAYLOAD_IDR) {
                    colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
                }
            }
        #endif
            if (memcmp(&m_pColorSpaceDataTemp, &colorSpace, sizeof(DXGI_COLOR_SPACE_TYPE))) {
                memcpy(&m_pColorSpaceDataTemp, &colorSpace, sizeof(DXGI_COLOR_SPACE_TYPE));
                hres = m_pSwapChain3->SetColorSpace1(colorSpace);
                if (FAILED(hres))
                    return MFX_ERR_DEVICE_FAILED;
            }
        }
    }

    if (m_pSwapChain4) {
        #if (MFX_VERSION >= 2006)
        mfxExtMasteringDisplayColourVolume* displayColor =
            (mfxExtMasteringDisplayColourVolume*)GetExtBuffer(
                pSrf->Data.ExtParam,
                pSrf->Data.NumExtParam,
                MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME);
        mfxExtContentLightLevelInfo* contentLight =
            (mfxExtContentLightLevelInfo*)GetExtBuffer(pSrf->Data.ExtParam,
                                                       pSrf->Data.NumExtParam,
                                                       MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO);
        if (displayColor && contentLight) {
            DXGI_HDR_METADATA_HDR10 pHDRMetaData = {};
            if (displayColor->InsertPayloadToggle == MFX_PAYLOAD_IDR) {
                pHDRMetaData.MaxMasteringLuminance = displayColor->MaxDisplayMasteringLuminance;
                pHDRMetaData.MinMasteringLuminance = displayColor->MinDisplayMasteringLuminance;
                pHDRMetaData.RedPrimary[0]         = displayColor->DisplayPrimariesX[2];
                pHDRMetaData.RedPrimary[1]         = displayColor->DisplayPrimariesY[2];
                pHDRMetaData.GreenPrimary[0]       = displayColor->DisplayPrimariesX[0];
                pHDRMetaData.GreenPrimary[1]       = displayColor->DisplayPrimariesY[0];
                pHDRMetaData.BluePrimary[0]        = displayColor->DisplayPrimariesX[1];
                pHDRMetaData.BluePrimary[1]        = displayColor->DisplayPrimariesY[1];
                pHDRMetaData.WhitePoint[0]         = displayColor->WhitePointX;
                pHDRMetaData.WhitePoint[1]         = displayColor->WhitePointY;
            }
            if (contentLight->InsertPayloadToggle == MFX_PAYLOAD_IDR) {
                pHDRMetaData.MaxContentLightLevel      = contentLight->MaxContentLightLevel;
                pHDRMetaData.MaxFrameAverageLightLevel = contentLight->MaxPicAverageLightLevel;
            }
            if (memcmp(&m_pHDRMetaDataTemp, &pHDRMetaData, sizeof(pHDRMetaData))) {
                memcpy(&m_pHDRMetaDataTemp, &pHDRMetaData, sizeof(pHDRMetaData));

                hres = m_pSwapChain4->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10,
                                                     sizeof(DXGI_HDR_METADATA_HDR10),
                                                     &m_pHDRMetaDataTemp);
                if (FAILED(hres))
                    return MFX_ERR_DEVICE_FAILED;
            }
        }
        #endif
    }

    D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC InputViewDesc;
    InputViewDesc.FourCC               = 0;
    InputViewDesc.ViewDimension        = D3D11_VPIV_DIMENSION_TEXTURE2D;
    InputViewDesc.Texture2D.MipSlice   = 0;
    InputViewDesc.Texture2D.ArraySlice = 0;

    mfxHDLPair pair = { NULL };
    sts             = pAlloc->GetHDL(pAlloc->pthis, pSrf->Data.MemId, (mfxHDL*)&pair);
    MSDK_CHECK_STATUS(sts, "pAlloc->GetHDL failed");

    ID3D11Texture2D* pRTTexture2D = reinterpret_cast<ID3D11Texture2D*>(pair.first);
    D3D11_TEXTURE2D_DESC RTTexture2DDesc;

    if (!m_pTempTexture && m_nViews == 2) {
        pRTTexture2D->GetDesc(&RTTexture2DDesc);
        hres = m_pD3D11Device->CreateTexture2D(&RTTexture2DDesc, NULL, &m_pTempTexture.p);
        if (FAILED(hres))
            return MFX_ERR_DEVICE_FAILED;
    }

    // Creating input views for left and righ eyes
    if (1 == m_nViews) {
        hres = m_pDX11VideoDevice->CreateVideoProcessorInputView(pRTTexture2D,
                                                                 m_VideoProcessorEnum,
                                                                 &InputViewDesc,
                                                                 &m_pInputViewLeft.p);
    }
    else if (2 == m_nViews && 0 == pSrf->Info.FrameId.ViewId) {
        m_pD3D11Ctx->CopyResource(m_pTempTexture, pRTTexture2D);
        hres = m_pDX11VideoDevice->CreateVideoProcessorInputView(m_pTempTexture,
                                                                 m_VideoProcessorEnum,
                                                                 &InputViewDesc,
                                                                 &m_pInputViewLeft.p);
    }
    else {
        hres = m_pDX11VideoDevice->CreateVideoProcessorInputView(pRTTexture2D,
                                                                 m_VideoProcessorEnum,
                                                                 &InputViewDesc,
                                                                 &m_pInputViewRight.p);
    }
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    //  NV12 surface to RGB backbuffer
    RECT rect   = { 0 };
    rect.right  = pSrf->Info.CropW;
    rect.bottom = pSrf->Info.CropH;

    D3D11_VIDEO_PROCESSOR_STREAM StreamData;

    if (1 == m_nViews || pSrf->Info.FrameId.ViewId == 1) {
        StreamData.Enable                = TRUE;
        StreamData.OutputIndex           = 0;
        StreamData.InputFrameOrField     = 0;
        StreamData.PastFrames            = 0;
        StreamData.FutureFrames          = 0;
        StreamData.ppPastSurfaces        = NULL;
        StreamData.ppFutureSurfaces      = NULL;
        StreamData.pInputSurface         = m_pInputViewLeft;
        StreamData.ppPastSurfacesRight   = NULL;
        StreamData.ppFutureSurfacesRight = NULL;
        StreamData.pInputSurfaceRight    = m_nViews == 2 ? m_pInputViewRight : NULL;

        m_pVideoContext->VideoProcessorSetStreamSourceRect(m_pVideoProcessor, 0, true, &rect);
        m_pVideoContext->VideoProcessorSetStreamFrameFormat(m_pVideoProcessor,
                                                            0,
                                                            D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE);
        hres =
            m_pVideoContext->VideoProcessorBlt(m_pVideoProcessor, m_pOutputView, 0, 1, &StreamData);
        if (FAILED(hres))
            return MFX_ERR_DEVICE_FAILED;
    }

    if (1 == m_nViews || 1 == pSrf->Info.FrameId.ViewId) {
        DXGI_PRESENT_PARAMETERS parameters = { 0 };
        hres                               = m_pSwapChain->Present1(0, 0, &parameters);
        if (FAILED(hres))
            return MFX_ERR_DEVICE_FAILED;
    }

    return MFX_ERR_NONE;
}

void CD3D11Device::Close() {
    // Changing video mode back to the original state
    if (2 == m_nViews && !m_bDefaultStereoEnabled)
        m_pDisplayControl->SetStereoEnabled(FALSE);

    m_HandleWindow = NULL;
}

    #endif // #if MFX_D3D11_SUPPORT
#endif // #if defined(_WIN32) || defined(_WIN64)
