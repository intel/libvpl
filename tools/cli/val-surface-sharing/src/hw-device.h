//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef TOOLS_CLI_VAL_SURFACE_SHARING_SRC_HW_DEVICE_H_
#define TOOLS_CLI_VAL_SURFACE_SHARING_SRC_HW_DEVICE_H_

#include <memory>
#include "./defs.h"

#if defined(_WIN32) || defined(_WIN64)

    #include <atlbase.h>
    #include <d3d11.h>
    #include <dxgi1_2.h>

#else

    #include <fcntl.h>
    #include <unistd.h>

    #include "va/va.h"
    #include "va/va_drm.h"

    #include "va/va_drmcommon.h"

#endif

#if defined(_WIN32) || defined(_WIN64)
struct DevCtxD3D11 {
public:
    DevCtxD3D11() : pDXGIFactory(), pAdapter(), pD3D11Device(), pD3D11Ctx() {}
    ~DevCtxD3D11() {}

    mfxStatus InitDevice(mfxU32 nAdapterNum, mfxHandleType *pHandleType, mfxHDL *pHandle);
    ID3D11Device *GetVideoDeviceFromFrameSurface(mfxFrameSurface1 *pSrf);
    ID3D11Texture2D *GetVideoTextureFromFrameSurface(mfxFrameSurface1 *pSrf);
    ID3D11Texture2D *CreateSurfaceToShare(FrameInfo *pframeInfo, mfxU8 *data, bool brender = false);

    __inline ID3D11Device *GetDeviceHandle(void) {
        return pD3D11Device;
    }

private:
    CComPtr<IDXGIFactory2> pDXGIFactory;
    CComQIPtr<IDXGIAdapter> pAdapter;
    CComPtr<ID3D11Device> pD3D11Device;
    CComPtr<ID3D11DeviceContext> pD3D11Ctx;
};
#else

struct DevCtxVAAPI {
public:
    DevCtxVAAPI() : m_vaDRMfd(-1), m_vaDisplay() {}

    ~DevCtxVAAPI() {
        if (m_vaDisplay) {
            vaTerminate(m_vaDisplay);
            m_vaDisplay = nullptr;
        }

        if (m_vaDRMfd >= 0) {
            close(m_vaDRMfd);
            m_vaDRMfd = -1;
        }
    }

    mfxStatus InitDevice(mfxU32 nAdapterNum, mfxHandleType *pHandleType, mfxHDL *pHandle);

    __inline VADisplay GetVADisplay(void) {
        return m_vaDisplay;
    }

    VASurfaceID CreateSurfaceToShare(FrameInfo *pframeInfo, mfxU8 *data, bool brender = false);

private:
    int m_vaDRMfd;
    VADisplay m_vaDisplay;
};
#endif

struct DevCtx
#if defined(_WIN32) || defined(_WIN64)
        : DevCtxD3D11
#else
        : DevCtxVAAPI
#endif
{
public:
    DevCtx() {}
    ~DevCtx() {}
};

#endif // TOOLS_CLI_VAL_SURFACE_SHARING_SRC_HW_DEVICE_H_
