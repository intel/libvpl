//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./hw-device.h"

mfxStatus DevCtxD3D11::InitDevice(mfxU32 nAdapterNum, mfxHandleType *pHandleType, mfxHDL *pHandle) {
    mfxStatus sts = MFX_ERR_NONE;
    HRESULT hres  = S_OK;

    static D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL pFeatureLevelsOut;

    CComPtr<IDXGIFactory2> pDXGIFactory_l;
    hres = CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void **)(&pDXGIFactory_l));
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    pDXGIFactory = pDXGIFactory_l;

    CComQIPtr<IDXGIAdapter> pAdapter_l;
    hres = pDXGIFactory->EnumAdapters(nAdapterNum, &pAdapter_l);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    pAdapter = pAdapter_l;

    CComPtr<ID3D11Device> pD3D11Device_l;
    CComPtr<ID3D11DeviceContext> pD3D11Ctx_l;
    hres = D3D11CreateDevice(pAdapter,
                             D3D_DRIVER_TYPE_UNKNOWN,
                             NULL,
#ifdef _DEBUG
                             D3D11_CREATE_DEVICE_DEBUG,
#else
                             0,
#endif
                             FeatureLevels,
                             sizeof(FeatureLevels) / sizeof(FeatureLevels[0]),
                             D3D11_SDK_VERSION,
                             &pD3D11Device_l,
                             &pFeatureLevelsOut,
                             &pD3D11Ctx_l);

    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    pD3D11Device = pD3D11Device_l;
    pD3D11Ctx    = pD3D11Ctx_l;

    // turn on multithreading for the Context
    CComQIPtr<ID3D10Multithread> mt(pD3D11Ctx);

    if (mt)
        mt->SetMultithreadProtected(true);
    else
        return MFX_ERR_DEVICE_FAILED;

    *pHandleType = MFX_HANDLE_D3D11_DEVICE;
    *pHandle     = (mfxHDL)pD3D11Device;

    return sts;
}

ID3D11Device *DevCtxD3D11::GetVideoDeviceFromFrameSurface(mfxFrameSurface1 *pSrf) {
    // get handle of internal device created by runtime, use same device for rendering
    mfxHDL device_handle      = nullptr;
    mfxHandleType device_type = {};

    mfxStatus sts = pSrf->FrameInterface->GetDeviceHandle(pSrf, &device_handle, &device_type);
    if (sts != MFX_ERR_NONE || !device_handle || device_type != MFX_HANDLE_D3D11_DEVICE)
        return nullptr;

    return (ID3D11Device *)device_handle;
}

ID3D11Texture2D *DevCtxD3D11::GetVideoTextureFromFrameSurface(mfxFrameSurface1 *pSrf) {
    // get native handle of texture created by runtime
    mfxHDL frame_handle      = nullptr;
    mfxResourceType res_type = {};

    mfxStatus sts = pSrf->FrameInterface->GetNativeHandle(pSrf, &frame_handle, &res_type);
    if (sts != MFX_ERR_NONE || !frame_handle || res_type != MFX_RESOURCE_DX11_TEXTURE)
        return nullptr;

    return (ID3D11Texture2D *)frame_handle;
}
