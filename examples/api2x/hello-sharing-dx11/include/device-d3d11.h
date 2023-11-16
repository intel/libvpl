//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef DISPATCHER_TEST_HELLO_SHARING_IMPORT_SRC_HW_DEVICE_H_
#define DISPATCHER_TEST_HELLO_SHARING_IMPORT_SRC_HW_DEVICE_H_

#if defined(_WIN32) || defined(_WIN64)

    #include <atlbase.h>
    #include <d3d11.h>
    #include <dxgi1_2.h>
    #include "vpl/mfx.h"

struct DevCtxD3D11 {
public:
    DevCtxD3D11() : pDXGIFactory(), pAdapter(), pD3D11Device(), pD3D11Ctx() {}
    ~DevCtxD3D11() {}

    mfxStatus InitDevice(mfxU32 nAdapterNum, mfxHandleType *pHandleType, mfxHDL *pHandle);
    CComPtr<ID3D11Texture2D> GetVideoTexture(mfxU16 width, mfxU16 height, mfxU8 *data);

    __inline ID3D11Device *GetDeviceHandle(void) {
        return pD3D11Device;
    }

private:
    CComPtr<IDXGIFactory2> pDXGIFactory;
    CComQIPtr<IDXGIAdapter> pAdapter;
    CComPtr<ID3D11Device> pD3D11Device;
    CComPtr<ID3D11DeviceContext> pD3D11Ctx;
};

struct DevCtx : DevCtxD3D11

{
public:
    DevCtx() {}
    ~DevCtx() {}
};
#endif // Windows

#endif // DISPATCHER_TEST_HELLO_SHARING_IMPORT_SRC_HW_DEVICE_H_
