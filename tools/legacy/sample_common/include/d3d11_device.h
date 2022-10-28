/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#if defined(_WIN32) || defined(_WIN64)

    #include "sample_defs.h" // defines MFX_D3D11_SUPPORT

    #if MFX_D3D11_SUPPORT
        #include <atlbase.h>
        #include <d3d11.h>
        #include <windows.h>
        #include "hw_device.h"

        #include <dxgi1_2.h>
        #include <dxgi1_4.h>
        #include <dxgi1_5.h>
        #include <dxgi1_6.h>

class CD3D11Device : public CHWDevice {
public:
    CD3D11Device();
    virtual ~CD3D11Device();
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
    virtual void SetDxgiFullScreen() {
        m_bDxgiFs = TRUE;
    }

protected:
    virtual mfxStatus FillSCD(mfxHDL hWindow, DXGI_SWAP_CHAIN_DESC& scd);
    virtual mfxStatus FillSCD1(DXGI_SWAP_CHAIN_DESC1& scd);
    mfxStatus CreateVideoProcessor(mfxFrameSurface1* pSrf);

    CComPtr<ID3D11Device> m_pD3D11Device;
    CComPtr<ID3D11DeviceContext> m_pD3D11Ctx;
    CComQIPtr<ID3D11VideoDevice> m_pDX11VideoDevice;
    CComQIPtr<ID3D11VideoContext> m_pVideoContext;
    CComPtr<ID3D11VideoProcessorEnumerator> m_VideoProcessorEnum;

    CComQIPtr<IDXGIDevice1> m_pDXGIDev;
    CComQIPtr<IDXGIAdapter> m_pAdapter;
    CComQIPtr<IDXGIAdapter1> m_pAdapter1;

    CComPtr<IDXGIFactory2> m_pDXGIFactory;

    CComPtr<IDXGISwapChain1> m_pSwapChain;
    CComPtr<IDXGISwapChain4> m_pSwapChain4;
    CComPtr<IDXGISwapChain3> m_pSwapChain3;
    CComPtr<ID3D11VideoProcessor> m_pVideoProcessor;

private:
    CComPtr<ID3D11VideoProcessorInputView> m_pInputViewLeft;
    CComPtr<ID3D11VideoProcessorInputView> m_pInputViewRight;
    CComPtr<ID3D11VideoProcessorOutputView> m_pOutputView;

    CComPtr<ID3D11Texture2D> m_pDXGIBackBuffer;
    CComPtr<ID3D11Texture2D> m_pTempTexture;
    CComPtr<IDXGIDisplayControl> m_pDisplayControl;
    CComPtr<IDXGIOutput> m_pDXGIOutput;
    CComPtr<IDXGIOutput6> m_pDXGIOutput6;
    mfxU16 m_nViews;
    mfxU32 m_nPrimaryWidth;
    mfxU32 m_nPrimaryHeight;
    BOOL m_bDefaultStereoEnabled;
    BOOL m_bHdrSupport;
    BOOL m_bIsA2rgb10;
    HWND m_HandleWindow;
    BOOL m_bDxgiFs;

    DXGI_COLOR_SPACE_TYPE m_pColorSpaceDataTemp;
    DXGI_HDR_METADATA_HDR10 m_pHDRMetaDataTemp;
};

    #endif //#if defined( _WIN32 ) || defined ( _WIN64 )
#endif //#if MFX_D3D11_SUPPORT
