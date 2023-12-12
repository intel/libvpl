//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./capture-frames.h"

mfxStatus CaptureCtxD3D11::CaptureInit(DevCtx *devCtx) {
    m_devCtx       = devCtx;
    m_pD3D11Device = m_devCtx->GetDeviceHandle();

    CComPtr<IDXGIDevice> pDXGIDevice;
    CComPtr<IDXGIAdapter> pDXGIAdapter;
    CComPtr<IDXGIOutput> pDXGIOutput;
    CComPtr<IDXGIOutput1> pDXGIOutput1;

    HRESULT hres = m_pD3D11Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    hres = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&pDXGIAdapter);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    DXGI_ADAPTER_DESC d = {};
    hres                = pDXGIAdapter->GetDesc(&d);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    hres = pDXGIAdapter->EnumOutputs(0, &pDXGIOutput);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    hres = pDXGIOutput->QueryInterface(__uuidof(pDXGIOutput), (void **)&pDXGIOutput1);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    CComPtr<IDXGIOutputDuplication> pDXGIOutputDupl;
    hres = pDXGIOutput1->DuplicateOutput(m_pD3D11Device, &pDXGIOutputDupl);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    m_pDXGIOutputDupl = pDXGIOutputDupl;

    m_pDXGIOutputDupl->GetDesc(&m_DXGIOutduplDesc);

    return MFX_ERR_NONE;
}

mfxStatus CaptureCtxD3D11::CaptureFrame(CComPtr<ID3D11Texture2D> &pTex2D) {
    CComPtr<IDXGIResource> pDXGIResource;
    DXGI_OUTDUPL_FRAME_INFO DXGIOutduplFrameInfo;

    // capture next desktop frame
    HRESULT hres = m_pDXGIOutputDupl->AcquireNextFrame(INFINITE, &DXGIOutduplFrameInfo, &pDXGIResource);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    // map resource to D3D11 texture
    hres = pDXGIResource->QueryInterface(__uuidof(ID3D11Texture2D), (void **)&pTex2D);
    if (FAILED(hres))
        return MFX_ERR_DEVICE_FAILED;

    // DBG
    // D3D11_TEXTURE2D_DESC desc = {};
    // pTex2D->GetDesc(&desc);

    return MFX_ERR_NONE;
}

mfxStatus CaptureCtxD3D11::ReleaseFrame(void) {
    m_pDXGIOutputDupl->ReleaseFrame();

    return MFX_ERR_NONE;
}
