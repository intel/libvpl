//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./render-frames.h"

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
#ifdef _WIN64
    RenderCtx *rc = (RenderCtx *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
    RenderCtx *rc = (RenderCtx *)LongToPtr(GetWindowLongPtr(hWnd, GWL_USERDATA));
#endif
    if (rc) {
        switch (message) {
            // see windowsx.h for function prototype to implement for each message type
            HANDLE_MSG(hWnd, WM_DESTROY, rc->OnDestroy);
            HANDLE_MSG(hWnd, WM_KEYUP, rc->OnKey);
        }
    }

    // any unhandled messages go to default processor
    return DefWindowProc(hWnd, message, wParam, lParam);
}

mfxStatus RenderCtxD3D11::RenderInit(DevCtx *devCtx, mfxU32 width, mfxU32 height, std::string infileName) {
    mfxStatus sts = MFX_ERR_NONE;

    m_devCtx       = devCtx;
    m_pD3D11Device = m_devCtx->GetDeviceHandle();
    m_dispWidth    = width;
    m_dispHeight   = height;

    // register new window class
    WNDCLASS window      = {};
    window.lpfnWndProc   = (WNDPROC)WindowProc;
    window.hInstance     = GetModuleHandle(NULL);
    window.hCursor       = LoadCursor(NULL, IDC_ARROW);
    window.lpszClassName = "hello-export Window Class";
    if (!RegisterClass(&window))
        return MFX_ERR_UNKNOWN;

    // create rendering window
    HMODULE hmod = GetModuleHandle(NULL);
    m_hwnd       = CreateWindow(window.lpszClassName, infileName.c_str(), WS_OVERLAPPEDWINDOW, 0, 0, width, height, NULL, NULL, hmod, NULL);
    if (!m_hwnd)
        return MFX_ERR_UNKNOWN;

    ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hwnd);

    // set user data pointer for the window to this RenderCtx instance
    SetLastError(0);
    LONG_PTR winPtr =
#ifdef _WIN64
        SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);
#else
        SetWindowLong(m_hwnd, GWL_USERDATA, PtrToLong(this));
#endif

    if (!winPtr) {
        DWORD err = GetLastError();
        if (err)
            printf("err = %d\n", err);
    }

    HRESULT hres = S_OK;

    D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc = {};
    ContentDesc.InputFrameFormat                   = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
    ContentDesc.InputFrameRate.Numerator           = 30000;
    ContentDesc.InputFrameRate.Denominator         = 1000;
    ContentDesc.InputWidth                         = m_dispWidth;
    ContentDesc.InputHeight                        = m_dispHeight;
    ContentDesc.OutputWidth                        = m_dispWidth;
    ContentDesc.OutputHeight                       = m_dispHeight;
    ContentDesc.OutputFrameRate.Numerator          = 30000;
    ContentDesc.OutputFrameRate.Denominator        = 1000;
    ContentDesc.Usage                              = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

    CComPtr<ID3D11VideoDevice> pDX11VideoDevice;
    hres = m_pD3D11Device->QueryInterface(__uuidof(ID3D11VideoDevice), (void **)(&pDX11VideoDevice));
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    m_pDX11VideoDevice = pDX11VideoDevice;

    CComPtr<ID3D11VideoProcessorEnumerator> pVideoProcessorEnum;
    hres = m_pDX11VideoDevice->CreateVideoProcessorEnumerator(&ContentDesc, &pVideoProcessorEnum);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    m_pVideoProcessorEnum = pVideoProcessorEnum;

    CComPtr<ID3D11VideoProcessor> pVideoProcessor;
    hres = m_pDX11VideoDevice->CreateVideoProcessor(m_pVideoProcessorEnum, 0, &pVideoProcessor);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    m_pVideoProcessor = pVideoProcessor;

    // create swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
    swapChainDesc.Width                 = 0; // Use automatic sizing.
    swapChainDesc.Height                = 0;
    swapChainDesc.Format                = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.Stereo                = FALSE;
    swapChainDesc.SampleDesc.Count      = 1; // Don't use multi-sampling.
    swapChainDesc.SampleDesc.Quality    = 0;
    swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount           = 2; // Use double buffering to minimize latency.
    swapChainDesc.Scaling               = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags                 = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    CComPtr<IDXGIDevice2> pDXGIDevice;
    hres = m_pDX11VideoDevice->QueryInterface(__uuidof(IDXGIDevice2), (void **)&pDXGIDevice);

    CComPtr<IDXGIAdapter> pDXGIAdapter;
    hres = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&pDXGIAdapter);

    CComPtr<IDXGIFactory2> pIDXGIFactory;
    pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void **)&pIDXGIFactory);

    CComPtr<ID3D11DeviceContext> pDeviceContext;
    m_pD3D11Device->GetImmediateContext(&pDeviceContext);

    m_pDeviceContext = pDeviceContext;

    CComPtr<ID3D11VideoContext> pVideoContext;
    hres = m_pDeviceContext->QueryInterface(__uuidof(ID3D11VideoContext), (void **)&pVideoContext);

    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    m_pVideoContext = pVideoContext;

    CComPtr<IDXGISwapChain1> pSwapChain;
    hres = pIDXGIFactory->CreateSwapChainForHwnd(m_pDX11VideoDevice, (HWND)m_hwnd, &swapChainDesc, NULL, NULL, reinterpret_cast<IDXGISwapChain1 **>(&pSwapChain));

    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    m_pSwapChain = pSwapChain;

    return sts;
}

// return a raw pointer to texture
// lazy-inited only as needed, so must call release when finished
ID3D11Texture2D *RenderCtxD3D11::GetRenderSurface(mfxU32 width, mfxU32 height) {
    mfxStatus sts = MFX_ERR_NONE;

    if (!m_pRenderSurface) {
        ID3D11Texture2D *pTex2D;

        if (width == 0)
            width = m_dispWidth;

        if (height == 0)
            height = m_dispHeight;

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width                = width;
        desc.Height               = height;
        desc.MipLevels            = 1;
        desc.ArraySize            = 1;
        desc.Format               = DXGI_FORMAT_NV12;
        desc.SampleDesc           = { 1, 0 };
        desc.Usage                = D3D11_USAGE_DEFAULT;
        desc.BindFlags            = (D3D11_BIND_DECODER | D3D11_BIND_VIDEO_ENCODER | D3D11_BIND_SHADER_RESOURCE);
        desc.CPUAccessFlags       = 0;
        desc.MiscFlags            = D3D11_RESOURCE_MISC_SHARED;

        HRESULT hres = m_pD3D11Device->CreateTexture2D(&desc, nullptr, &pTex2D);

        if (FAILED(hres) || (pTex2D == nullptr))
            return nullptr;

        m_pRenderSurface = pTex2D;
    }

    return m_pRenderSurface;
}

mfxStatus RenderCtxD3D11::RenderFrame(mfxHDL extSurface) {
    mfxStatus sts = MFX_ERR_NONE;
    HRESULT hres  = S_OK;

    if (!extSurface)
        return MFX_ERR_NULL_PTR;

    if (!m_pD3D11Device)
        return MFX_ERR_NOT_INITIALIZED;

    mfxSurfaceHeader *surface_header = reinterpret_cast<mfxSurfaceHeader *>(extSurface);

    ID3D11Texture2D *pTex2D = nullptr;
    if (surface_header->SurfaceType == MFX_SURFACE_TYPE_D3D11_TEX2D) {
        mfxSurfaceD3D11Tex2D *es = reinterpret_cast<mfxSurfaceD3D11Tex2D *>(extSurface);
        pTex2D                   = reinterpret_cast<ID3D11Texture2D *>(es->texture2D);
    }
    else {
        return MFX_ERR_UNSUPPORTED;
    }

    if (!pTex2D)
        return MFX_ERR_NULL_PTR;

    // render frame
    CComPtr<ID3D11Texture2D> pDXGIBackBuffer;
    hres = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&pDXGIBackBuffer);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    m_pDXGIBackBuffer = pDXGIBackBuffer;

    D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC OutputViewDesc;
    OutputViewDesc.ViewDimension      = D3D11_VPOV_DIMENSION_TEXTURE2D;
    OutputViewDesc.Texture2D.MipSlice = 0;

    CComPtr<ID3D11VideoProcessorOutputView> pOutputView;
    hres = m_pDX11VideoDevice->CreateVideoProcessorOutputView(m_pDXGIBackBuffer, m_pVideoProcessorEnum, &OutputViewDesc, &pOutputView);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    m_pOutputView = pOutputView;

    D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC InputViewDesc;
    InputViewDesc.FourCC               = 0;
    InputViewDesc.ViewDimension        = D3D11_VPIV_DIMENSION_TEXTURE2D;
    InputViewDesc.Texture2D.MipSlice   = 0;
    InputViewDesc.Texture2D.ArraySlice = 0;

    CComPtr<ID3D11VideoProcessorInputView> pInputView;
    hres = m_pDX11VideoDevice->CreateVideoProcessorInputView(pTex2D, m_pVideoProcessorEnum, &InputViewDesc, &pInputView);

    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    m_pInputView = pInputView;

    //  NV12 surface to RGB backbuffer
    RECT rect   = { 0 };
    rect.right  = m_dispWidth;
    rect.bottom = m_dispHeight;

    D3D11_VIDEO_PROCESSOR_STREAM StreamData;
    StreamData.Enable                = TRUE;
    StreamData.OutputIndex           = 0;
    StreamData.InputFrameOrField     = 0;
    StreamData.PastFrames            = 0;
    StreamData.FutureFrames          = 0;
    StreamData.ppPastSurfaces        = NULL;
    StreamData.ppFutureSurfaces      = NULL;
    StreamData.pInputSurface         = m_pInputView;
    StreamData.ppPastSurfacesRight   = NULL;
    StreamData.ppFutureSurfacesRight = NULL;
    StreamData.pInputSurfaceRight    = NULL;

    m_pVideoContext->VideoProcessorSetStreamSourceRect(m_pVideoProcessor, 0, true, &rect);
    m_pVideoContext->VideoProcessorSetStreamFrameFormat(m_pVideoProcessor, 0, D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE);
    hres = m_pVideoContext->VideoProcessorBlt(m_pVideoProcessor, m_pOutputView, 0, 1, &StreamData);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    DXGI_PRESENT_PARAMETERS parameters = { 0 };
    hres                               = m_pSwapChain->Present1(0, 0, &parameters);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    return MFX_ERR_NONE;
}
