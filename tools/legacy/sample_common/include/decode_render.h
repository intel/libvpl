/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __DECODE_D3D_RENDER_H__
#define __DECODE_D3D_RENDER_H__

#if defined(_WIN32) || defined(_WIN64)

    #pragma warning(disable : 4201)
    #include <d3d9.h>
    #include <dxva.h>
    #include <dxva2api.h>
    #include <windows.h>

#endif

#include "vpl/mfxstructures.h"
#include "vpl/mfxvideo.h"

#include "hw_device.h"

typedef void* WindowHandle;
typedef void* Handle;

#if defined(_WIN32) || defined(_WIN64)

struct sWindowParams {
    LPCTSTR lpClassName;
    LPCTSTR lpWindowName;
    DWORD dwStyle;
    int nx;
    int ny;
    int ncell;
    int nAdapter;
    int nWidth;
    int nHeight;
    HWND hWndParent;
    HMENU hMenu;
    HINSTANCE hInstance;
    LPVOID lpParam;
    bool bFullScreen; // Stretch window to full screen
};

class CDecodeD3DRender {
public:
    CDecodeD3DRender();
    virtual ~CDecodeD3DRender();

    virtual mfxStatus Init(sWindowParams pWParams);
    virtual mfxStatus RenderFrame(mfxFrameSurface1* pSurface, mfxFrameAllocator* pmfxAlloc);
    virtual VOID UpdateTitle(double fps);
    virtual VOID SetDxgiFullScreen() {
        m_bDxgiFs = true;
    }
    void Close();

    HWND GetWindowHandle();

    VOID OnDestroy(HWND hwnd);
    VOID OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
    VOID ChangeWindowSize(bool bFullScreen);

    void SetHWDevice(CHWDevice* dev) {
        m_hwdev = dev;
    }

protected:
    void AdjustWindowRect(RECT* rect);

    mfxStatus AllocateShiftedSurfaceIfNeeded(const mfxFrameSurface1* refSurface,
                                             mfxFrameAllocator* allocator);
    mfxFrameAllocResponse shiftSurfaceResponse;
    mfxFrameSurface1 shiftedSurface;
    mfxFrameAllocator* pAllocator;

    CHWDevice* m_hwdev;

    sWindowParams m_sWindowParams;
    HWND m_Hwnd;
    RECT m_rect;
    DWORD m_style;

    bool EnableDwmQueuing();
    static BOOL CALLBACK MonitorEnumProc(HMONITOR, HDC, LPRECT lprcMonitor, LPARAM dwData);
    static bool m_bIsMonitorFound;

    bool m_bDwmEnabled;
    int m_nMonitorCurrent;
    ::RECT m_RectWindow;

    bool m_bDxgiFs;
};
#endif // #if defined(_WIN32) || defined(_WIN64)

#endif // __DECODE_D3D_RENDER_H__
