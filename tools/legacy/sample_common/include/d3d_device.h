/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#if defined(_WIN32) || defined(_WIN64)

    #include "hw_device.h"

    #pragma warning(disable : 4201)
    #include <d3d9.h>
    #include <dxva.h>
    #include <dxva2api.h>
    #include <windows.h>

    #define VIDEO_MAIN_FORMAT D3DFMT_YUY2

/** Direct3D 9 device implementation.
@note Device always set D3DPRESENT_PARAMETERS::Windowed to TRUE.
*/
class CD3D9Device : public CHWDevice {
public:
    CD3D9Device();
    virtual ~CD3D9Device();

    virtual mfxStatus Init(mfxHDL hWindow, mfxU16 nViews, mfxU32 nAdapterNum);
    virtual mfxStatus Reset();
    virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL* pHdl);
    virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL hdl);
    virtual mfxStatus RenderFrame(mfxFrameSurface1* pSurface, mfxFrameAllocator* pmfxAlloc);
    virtual void UpdateTitle(double /*fps*/) {}
    virtual void Close();
    void DefineFormat(bool isA2rgb10) {
        m_bIsA2rgb10 = (isA2rgb10) ? TRUE : FALSE;
    }
    virtual void SetMondelloInput(bool /*isMondelloInputEnabled*/) {}
    virtual void SetDxgiFullScreen() {}

protected:
    mfxStatus CreateVideoProcessors();
    bool CheckOverlaySupport();
    virtual mfxStatus FillD3DPP(mfxHDL hWindow, mfxU16 nViews, D3DPRESENT_PARAMETERS& D3DPP);

private:
    IDirect3D9Ex* m_pD3D9;
    IDirect3DDevice9Ex* m_pD3DD9;
    IDirect3DDeviceManager9* m_pDeviceManager9;
    D3DPRESENT_PARAMETERS m_D3DPP;
    UINT m_resetToken;

    mfxU16 m_nViews;

    D3DSURFACE_DESC m_backBufferDesc;

    // service required to create video processors
    IDirectXVideoProcessorService* m_pDXVAVPS;
    //left channel processor
    IDirectXVideoProcessor* m_pDXVAVP_Left;
    // right channel processor
    IDirectXVideoProcessor* m_pDXVAVP_Right;

    // target rectangle
    RECT m_targetRect;

    // various structures for DXVA2 calls
    DXVA2_VideoDesc m_VideoDesc;
    DXVA2_VideoProcessBltParams m_BltParams;
    DXVA2_VideoSample m_Sample;

    BOOL m_bIsA2rgb10;
};

#endif // #if defined( _WIN32 ) || defined ( _WIN64 )
