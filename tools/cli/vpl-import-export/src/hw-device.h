//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_HW_DEVICE_H_
#define TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_HW_DEVICE_H_

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

#ifdef TOOLS_ENABLE_X11
#include "X11/Xutil.h"
#include "va/va_x11.h"
#endif

#endif

#if defined(_WIN32) || defined(_WIN64)
struct DevCtxD3D11 {
public:
    DevCtxD3D11() : pDXGIFactory(), pAdapter(), pD3D11Device(), pD3D11Ctx() {}
    ~DevCtxD3D11() {}

    mfxStatus InitDevice(mfxU32 nAdapterNum, mfxHandleType *pHandleType, mfxHDL *pHandle);
    ID3D11Device *GetVideoDeviceFromFrameSurface(mfxFrameSurface1 *pSrf);
    ID3D11Texture2D *GetVideoTextureFromFrameSurface(mfxFrameSurface1 *pSrf);

    __inline ID3D11Device *GetDeviceHandle(void) {
        return pD3D11Device;
    }

    __inline ID3D11DeviceContext *GetDeviceContext(void) {
        return pD3D11Ctx;
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
    DevCtxVAAPI()
            : m_vaDRMfd(-1),
              m_vaDisplay()
#ifdef TOOLS_ENABLE_X11
              ,
              m_pX11Display()
#endif
    {
    }

    ~DevCtxVAAPI() {
        if (m_vaDisplay) {
            vaTerminate(m_vaDisplay);
            m_vaDisplay = nullptr;
        }

        if (m_vaDRMfd >= 0) {
            close(m_vaDRMfd);
            m_vaDRMfd = -1;
        }

#ifdef TOOLS_ENABLE_X11
        if (m_pX11Display) {
            XCloseDisplay(m_pX11Display);
            m_pX11Display = nullptr;
        }
#endif
    }

    mfxStatus InitDevice(mfxU32 nAdapterNum, mfxHandleType *pHandleType, mfxHDL *pHandle);

    __inline VADisplay GetVADisplay(void) {
        return m_vaDisplay;
    }

#ifdef TOOLS_ENABLE_X11
    __inline Display *GetX11Display(void) {
        return m_pX11Display;
    }
#endif

private:
    int m_vaDRMfd;
    VADisplay m_vaDisplay;
#ifdef TOOLS_ENABLE_X11
    Display *m_pX11Display;
#endif
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

#endif // TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_HW_DEVICE_H_
