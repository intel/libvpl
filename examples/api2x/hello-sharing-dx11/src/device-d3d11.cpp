//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "device-d3d11.h"

mfxStatus DevCtxD3D11::InitDevice(mfxU32 nAdapterNum, mfxHandleType *pHandleType, mfxHDL *pHandle) {
    mfxStatus sts = MFX_ERR_NONE;
    HRESULT hres  = S_OK;

    static D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_1,
                                                 D3D_FEATURE_LEVEL_11_0,
                                                 D3D_FEATURE_LEVEL_10_1,
                                                 D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL pFeatureLevelsOut;

    hres = CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void **)(&pDXGIFactory));
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

CComPtr<ID3D11Texture2D> DevCtxD3D11::GetVideoTexture(mfxU16 width, mfxU16 height, mfxU8 *data) {
    CComPtr<ID3D11Texture2D> texture_holder;
    D3D11_TEXTURE2D_DESC descr_to_create_texture = D3D11_TEXTURE2D_DESC{
        UINT(width),
        UINT(height),
        1,
        1,
        DXGI_FORMAT_NV12,
        { 1, 0 },
        D3D11_USAGE_DEFAULT,
        (D3D11_BIND_DECODER | D3D11_BIND_VIDEO_ENCODER | D3D11_BIND_SHADER_RESOURCE),
        0,
        D3D11_RESOURCE_MISC_SHARED
    };

    D3D11_SUBRESOURCE_DATA initial_data = {};
    initial_data.pSysMem                = data;
    initial_data.SysMemPitch            = width;

    HRESULT hr =
        pD3D11Device->CreateTexture2D(&descr_to_create_texture, &initial_data, &texture_holder);

    if (FAILED(hr)) {
        printf("ERROR: CreateTexture2D failed!\n");
    }

    return texture_holder;
}
