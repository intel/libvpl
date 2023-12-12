//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./hw-device.h"

mfxStatus DevCtxD3D11::InitDevice(mfxU32 nAdapterNum, mfxHandleType *pHandleType, mfxHDL *pHandle) {
    mfxStatus sts = MFX_ERR_NONE;
    HRESULT hres  = S_OK;

    static D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_1,
                                                 D3D_FEATURE_LEVEL_11_0,
                                                 D3D_FEATURE_LEVEL_10_1,
                                                 D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL pFeatureLevelsOut;

    hres = CreateDXGIFactory1(__uuidof(IDXGIFactory2), reinterpret_cast<void **>(&pDXGIFactory));
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    hres = pDXGIFactory->EnumAdapters(nAdapterNum, &pAdapter);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

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
                             &pD3D11Device,
                             &pFeatureLevelsOut,
                             &pD3D11Ctx);

    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

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

    return reinterpret_cast<ID3D11Device *>(device_handle);
}

ID3D11Texture2D *DevCtxD3D11::GetVideoTextureFromFrameSurface(mfxFrameSurface1 *pSrf) {
    // get native handle of texture created by runtime
    mfxHDL frame_handle      = nullptr;
    mfxResourceType res_type = {};

    mfxStatus sts = pSrf->FrameInterface->GetNativeHandle(pSrf, &frame_handle, &res_type);
    if (sts != MFX_ERR_NONE || !frame_handle || res_type != MFX_RESOURCE_DX11_TEXTURE)
        return nullptr;

    return reinterpret_cast<ID3D11Texture2D *>(frame_handle);
}

ID3D11Texture2D *DevCtxD3D11::CreateSurfaceToShare(FrameInfo *pframeInfo,
                                                   mfxU8 *data,
                                                   bool brender) {
    ID3D11Texture2D *pTex2D;

    UINT bindFlags;
    D3D11_TEXTURE2D_DESC desc = {};

    desc.Format =
        (pframeInfo->fourcc == MFX_FOURCC_NV12) ? DXGI_FORMAT_NV12 : DXGI_FORMAT_B8G8R8A8_UNORM;

    if (brender == false) {
        bindFlags =
            (pframeInfo->fourcc == MFX_FOURCC_NV12)
                ? (D3D11_BIND_DECODER | D3D11_BIND_VIDEO_ENCODER | D3D11_BIND_SHADER_RESOURCE)
                : (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    }
    else
        bindFlags = (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

    desc.Width              = pframeInfo->width;
    desc.Height             = pframeInfo->height;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.BindFlags          = bindFlags;
    desc.CPUAccessFlags     = 0;
    desc.MiscFlags          = D3D11_RESOURCE_MISC_SHARED;

    D3D11_SUBRESOURCE_DATA initialData = {};
    initialData.pSysMem                = data;
    initialData.SysMemPitch            = pframeInfo->pitch;

    if (pframeInfo->fourcc == MFX_FOURCC_RGB4) {
        initialData.SysMemSlicePitch = pframeInfo->width * pframeInfo->height * 4;
    }

    HRESULT hr = pD3D11Device->CreateTexture2D(&desc, &initialData, &pTex2D);
    if (FAILED(hr))
        return nullptr;

    return pTex2D;
}
