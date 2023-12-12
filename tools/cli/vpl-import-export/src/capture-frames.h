//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_CAPTURE_FRAMES_H_
#define TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_CAPTURE_FRAMES_H_

#include "./hw-device.h"

#if defined(_WIN32) || defined(_WIN64)

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>

#else

#endif

#if defined(_WIN32) || defined(_WIN64)
struct CaptureCtxD3D11 {
    CaptureCtxD3D11() : m_devCtx(), m_pD3D11Device(), m_DXGIOutduplDesc(), m_pDXGIOutputDupl() {}

    ~CaptureCtxD3D11() {}

    __inline mfxHDL GetDeviceHandle(void) {
        return reinterpret_cast<mfxHDL>(m_pD3D11Device);
    }

    __inline void GetCaptureResolution(mfxU16 &w, mfxU16 &h) {
        w = m_DXGIOutduplDesc.ModeDesc.Width;
        h = m_DXGIOutduplDesc.ModeDesc.Height;
    }

    mfxStatus CaptureInit(DevCtx *devCtx);
    mfxStatus CaptureFrame(CComPtr<ID3D11Texture2D> &pTex2D);
    mfxStatus ReleaseFrame(void);

private:
    DevCtx *m_devCtx; // HW device context, must have been initialized with InitDevice()
    ID3D11Device *m_pD3D11Device; // plain pointer, copy of device handle
    DXGI_OUTDUPL_DESC m_DXGIOutduplDesc;
    CComPtr<IDXGIOutputDuplication> m_pDXGIOutputDupl;
};
#else
struct CaptureCtxVAAPI {
    CaptureCtxVAAPI()
            : m_devCtx()
#ifdef TOOLS_ENABLE_X11
              ,
              m_pX11Display(NULL),
              m_x11Window(),
              m_dispWidth(0),
              m_dispHeight(0),
              m_lastCapturedSurfaceID(-1)
#endif
    {
    }

    ~CaptureCtxVAAPI() {}

    __inline void GetCaptureResolution(mfxU16 &w, mfxU16 &h) {
        w = 0;
        h = 0;
#ifdef TOOLS_ENABLE_X11
        w = m_dispWidth;
        h = m_dispHeight;
#endif
    }

    mfxStatus CaptureInit(DevCtx *devCtx);
    mfxStatus CaptureFrame(VASurfaceID *pvaSurfaceID);
    mfxStatus ReleaseFrame();

private:
    DevCtx *m_devCtx; // HW device context, must have been initialized with InitDevice()
#ifdef TOOLS_ENABLE_X11
    Display *m_pX11Display;
    Window m_x11Window;
    mfxU32 m_dispWidth;
    mfxU32 m_dispHeight;
    VASurfaceID m_lastCapturedSurfaceID;
#endif
};
#endif

struct CaptureCtx
#if defined(_WIN32) || defined(_WIN64)
        : CaptureCtxD3D11
#else
        : CaptureCtxVAAPI
#endif
{
    CaptureCtx() {}
    ~CaptureCtx() {}
};

#endif // TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_CAPTURE_FRAMES_H_
