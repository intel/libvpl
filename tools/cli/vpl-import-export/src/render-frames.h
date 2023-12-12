//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_RENDER_FRAMES_H_
#define TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_RENDER_FRAMES_H_

#include "./hw-device.h"

#if defined(_WIN32) || defined(_WIN64)

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <windowsx.h>

#else

#include "va/va.h"
#include "va/va_drm.h"

#include "va/va_drmcommon.h"

#ifdef TOOLS_ENABLE_X11
#include "X11/Xutil.h"
#include "va/va_x11.h"
#endif

#endif

#include <string>

#if defined(_WIN32) || defined(_WIN64)
struct RenderCtxD3D11 {
public:
    RenderCtxD3D11()
            : bQuit(false),
              bRotateOpenCL(true),
              bToggleSurfaceFlags(false),
              m_devCtx(),
              m_pD3D11Device(),
              m_hwnd(),
              m_pDX11VideoDevice(),
              m_pVideoProcessorEnum(),
              m_pVideoProcessor(),
              m_pDeviceContext(),
              m_pVideoContext(),
              m_pSwapChain(),
              m_pDXGIBackBuffer(),
              m_pOutputView(),
              m_pInputView(),
              m_pRenderSurface(),
              m_dispWidth(0),
              m_dispHeight(0) {}
    ~RenderCtxD3D11() {
        // raw pointer, lazy-inited only for OCL render, so call release when finished
        if (m_pRenderSurface)
            m_pRenderSurface->Release();
    }

    mfxStatus RenderInit(DevCtx *devCtx, mfxU32 width, mfxU32 height, std::string infileName);
    mfxStatus RenderFrame(mfxHDL extSurface);

    // retrieve an offline render surface, e.g. to use as output of OCL processing
    ID3D11Texture2D *RenderCtxD3D11::GetRenderSurface(mfxU32 width = 0, mfxU32 height = 0);

    void ProcessMessages() {
        MSG msg = {};
        while (msg.message != WM_QUIT && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void OnDestroy(HWND hwnd) {
        bQuit = true;
        PostQuitMessage(0);
    }

    void OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags) {
        if (TRUE == fDown)
            return;

        if (VK_ESCAPE == vk || 0x51 == vk) {
            // esc or 'Q'
            bQuit = true;
        }

        if (0x52 == vk) {
            // 'R' to toggle rotation
            bRotateOpenCL = !bRotateOpenCL;
        }

        if (0x46 == vk) {
            // 'F' to toggle surfaceFlags (SHARED <--> COPY)
            // caller should set this back to false after processing it (i.e in main processing loop)
            bToggleSurfaceFlags = true;
        }
    }

    // common flags
    bool bQuit;
    bool bRotateOpenCL;
    bool bToggleSurfaceFlags;

private:
    DevCtx *m_devCtx; // HW device context, must have been initialized with InitDevice()

    ID3D11Device *m_pD3D11Device; // plain pointer, copy of device handle

    HWND m_hwnd;

    CComPtr<ID3D11VideoDevice> m_pDX11VideoDevice;
    CComPtr<ID3D11VideoProcessorEnumerator> m_pVideoProcessorEnum;
    CComPtr<ID3D11VideoProcessor> m_pVideoProcessor;
    CComPtr<ID3D11DeviceContext> m_pDeviceContext;
    CComPtr<ID3D11VideoContext> m_pVideoContext;
    CComPtr<IDXGISwapChain1> m_pSwapChain;

    CComPtr<ID3D11Texture2D> m_pDXGIBackBuffer;
    CComPtr<ID3D11VideoProcessorOutputView> m_pOutputView;
    CComPtr<ID3D11VideoProcessorInputView> m_pInputView;

    // allocate raw pointer since it will be shared with application and passed around as mfxHDL
    // needs to be released explicitly in dtor
    ID3D11Texture2D *m_pRenderSurface;

    mfxU32 m_dispWidth;
    mfxU32 m_dispHeight;
};
#else
struct RenderCtxVAAPI {
    RenderCtxVAAPI()
            : bQuit(false),
              bToggleSurfaceFlags(false),
              m_devCtx()
#ifdef TOOLS_ENABLE_X11
              ,
              m_pX11Display(NULL),
              m_x11Window(),
              m_dispWidth(0),
              m_dispHeight(0)
#endif
    {
    }

    ~RenderCtxVAAPI() {
#ifdef TOOLS_ENABLE_X11
        if (m_x11Window != None) {
            // X11Display will be closed by DeviceContext
            XDestroyWindow(m_pX11Display, m_x11Window);
        }
#endif
    }

    mfxStatus RenderInit(DevCtx *devCtx, mfxU32 width, mfxU32 height, std::string infileName);
    mfxStatus RenderFrame(mfxHDL extSurface);

    void ProcessMessages() {}

    bool bQuit;
    bool bToggleSurfaceFlags;

private:
    DevCtx *m_devCtx; // HW device context, must have been initialized with InitDevice()

#ifdef TOOLS_ENABLE_X11
    Display *m_pX11Display;
    Window m_x11Window;
    mfxU32 m_dispWidth;
    mfxU32 m_dispHeight;
#endif
};
#endif

struct RenderCtx
#if defined(_WIN32) || defined(_WIN64)
        : RenderCtxD3D11
#else
        : RenderCtxVAAPI
#endif
{
    RenderCtx() {}
    ~RenderCtx() {}
};

#endif // TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_RENDER_FRAMES_H_
