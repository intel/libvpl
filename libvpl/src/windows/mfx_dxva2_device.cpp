/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#define INITGUID
#include <d3d9.h>
#include <dxgi.h>

#include "src/windows/mfx_dxva2_device.h"
#include "src/windows/mfx_load_dll.h"

using namespace MFX;

// convert LUID to mfxU64
mfxU64 LUIDtomfxU64(LUID luid) {
    return (((mfxU64)luid.HighPart << 32) | ((mfxU64)luid.LowPart));
}

DXDevice::DXDevice(void) {
    m_hModule = (HMODULE)0;

    m_numAdapters = 0;

    m_vendorID      = 0;
    m_deviceID      = 0;
    m_driverVersion = 0;
    m_luid          = {};

} // DXDevice::DXDevice(void)

DXDevice::~DXDevice(void) {
    Close();

    // free DX library only when device is destroyed
    UnloadDLLModule();

} // DXDevice::~DXDevice(void)

mfxU32 DXDevice::GetVendorID(void) const {
    return m_vendorID;

} // mfxU32 DXDevice::GetVendorID(void) const

mfxU32 DXDevice::GetDeviceID(void) const {
    return m_deviceID;

} // mfxU32 DXDevice::GetDeviceID(void) const

mfxU64 DXDevice::GetDriverVersion(void) const {
    return m_driverVersion;

} // mfxU64 DXDevice::GetDriverVersion(void) const

mfxU64 DXDevice::GetLUID(void) const {
    return m_luid;

} // mfxU64 DXDevice::GetLUID(void) const

mfxU32 DXDevice::GetAdapterCount(void) const {
    return m_numAdapters;

} // mfxU32 DXDevice::GetAdapterCount(void) const

void DXDevice::Close(void) {
    m_numAdapters = 0;

    m_vendorID = 0;
    m_deviceID = 0;
    m_luid     = {};

} // void DXDevice::Close(void)

void DXDevice::LoadDLLModule(const wchar_t *pModuleName) {
    // unload the module if it is required
    UnloadDLLModule();

#if !defined(MEDIASDK_UWP_DISPATCHER)
    DWORD prevErrorMode = 0;
    // set the silent error mode
    #if (_WIN32_WINNT >= 0x0600)
    SetThreadErrorMode(SEM_FAILCRITICALERRORS, &prevErrorMode);
    #else
    prevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    #endif
#endif // !defined(MEDIASDK_UWP_DISPATCHER)

    // load specified library
    m_hModule = LoadLibraryExW(pModuleName, NULL, 0);

#if !defined(MEDIASDK_UWP_DISPATCHER)
    // set the previous error mode
    #if (_WIN32_WINNT >= 0x0600)
    SetThreadErrorMode(prevErrorMode, NULL);
    #else
    SetErrorMode(prevErrorMode);
    #endif
#endif // !defined(MEDIASDK_UWP_DISPATCHER)

} // void LoadDLLModule(const wchar_t *pModuleName)

void DXDevice::UnloadDLLModule(void) {
    if (m_hModule) {
        FreeLibrary(m_hModule);
        m_hModule = (HMODULE)0;
    }

} // void DXDevice::UnloaDLLdModule(void)

#ifdef MFX_D3D9_ENABLED
D3D9Device::D3D9Device(void) {
    m_pD3D9   = (void *)0;
    m_pD3D9Ex = (void *)0;

} // D3D9Device::D3D9Device(void)

D3D9Device::~D3D9Device(void) {
    Close();

} // D3D9Device::~D3D9Device(void)

void D3D9Device::Close(void) {
    // release the interfaces
    if (m_pD3D9Ex) {
        ((IDirect3D9Ex *)m_pD3D9Ex)->Release();
    }

    // release the interfaces
    if (m_pD3D9) {
        ((IDirect3D9 *)m_pD3D9)->Release();
    }

    m_pD3D9   = (void *)0;
    m_pD3D9Ex = (void *)0;

} // void D3D9Device::Close(void)

typedef IDirect3D9 *(WINAPI *D3DCreateFunctionPtr_t)(UINT);

typedef HRESULT(WINAPI *D3DExCreateFunctionPtr_t)(UINT, IDirect3D9Ex **);

bool D3D9Device::Init(const mfxU32 adapterNum) {
    // load the library
    if (NULL == m_hModule) {
        LoadDLLModule(L"d3d9.dll");
    }

    if (m_hModule) {
        if (NULL == m_pD3D9) {
            D3DCreateFunctionPtr_t pFunc;

            // load address of procedure to create D3D device
            pFunc = (D3DCreateFunctionPtr_t)GetProcAddress(m_hModule, "Direct3DCreate9");
            if (pFunc) {
                // create D3D object
                m_pD3D9 = pFunc(D3D_SDK_VERSION);

                if (NULL == m_pD3D9) {
                    DXVA2DEVICE_TRACE(("FAIL: Direct3DCreate9(%d) : GetLastError()=0x%x",
                                       D3D_SDK_VERSION,
                                       GetLastError()));
                    return false;
                }
            }
            else {
                DXVA2DEVICE_TRACE_OPERATION({
                    wchar_t path[1024];
                    DWORD lastErr = GetLastError();
                    GetModuleFileNameW(m_hModule, path, sizeof(path) / sizeof(path[0]));
                    DXVA2DEVICE_TRACE((
                        "FAIL: invoking GetProcAddress(Direct3DCreate9) in %S : GetLastError()==0x%x\n",
                        path,
                        lastErr));
                });
                return false;
            }
        }

        D3DADAPTER_IDENTIFIER9 adapterIdent;
        IDirect3D9 *pD3D9;
        HRESULT hRes;

        // cast the interface
        pD3D9 = (IDirect3D9 *)m_pD3D9;

        m_numAdapters = pD3D9->GetAdapterCount();
        if (adapterNum >= m_numAdapters) {
            return false;
        }

        // get the card's parameters
        hRes = pD3D9->GetAdapterIdentifier(adapterNum, 0, &adapterIdent);
        if (D3D_OK != hRes) {
            DXVA2DEVICE_TRACE(("FAIL: GetAdapterIdentifier(%d) = 0x%x \n", adapterNum, hRes));
            return false;
        }

        m_vendorID      = adapterIdent.VendorId;
        m_deviceID      = adapterIdent.DeviceId;
        m_driverVersion = (mfxU64)adapterIdent.DriverVersion.QuadPart;

        // load LUID
        IDirect3D9Ex *pD3D9Ex;
        LUID d3d9LUID;

        if (NULL == m_pD3D9Ex) {
            D3DExCreateFunctionPtr_t pFuncEx;

            // find the appropriate function
            pFuncEx = (D3DExCreateFunctionPtr_t)GetProcAddress(m_hModule, "Direct3DCreate9Ex");
            if (NULL == pFuncEx) {
                // the extended interface is not supported
                return true;
            }

            // create extended interface
            hRes = pFuncEx(D3D_SDK_VERSION, (IDirect3D9Ex **)&m_pD3D9Ex);
            if (FAILED(hRes)) {
                // can't create extended interface
                return true;
            }
        }

        pD3D9Ex = (IDirect3D9Ex *)m_pD3D9Ex;

        // obtain D3D9 device LUID
        hRes = pD3D9Ex->GetAdapterLUID(adapterNum, &d3d9LUID);
        if (FAILED(hRes)) {
            // can't get LUID
            return true;
        }
        // copy the LUID
        m_luid = LUIDtomfxU64(d3d9LUID);
    }
    else {
        DXVA2DEVICE_TRACE(
            ("FAIL: invoking LoadLibrary(\"d3d9.dll\") : GetLastError()==0x%x\n", GetLastError()));
        return false;
    }

    return true;

} // bool D3D9Device::Init(const mfxU32 adapterNum)
#endif //MFX_D3D9_ENABLED

typedef HRESULT(WINAPI *DXGICreateFactoryFunc)(REFIID riid, void **ppFactory);

DXGI1Device::DXGI1Device(void) {
    m_pDXGIFactory1 = (void *)0;
    m_pDXGIAdapter1 = (void *)0;

} // DXGI1Device::DXGI1Device(void)

DXGI1Device::~DXGI1Device(void) {
    Close();

} // DXGI1Device::~DXGI1Device(void)

void DXGI1Device::Close(void) {
    // release the interfaces
    if (m_pDXGIAdapter1) {
        ((IDXGIAdapter1 *)m_pDXGIAdapter1)->Release();
    }

    if (m_pDXGIFactory1) {
        ((IDXGIFactory1 *)m_pDXGIFactory1)->Release();
    }

    m_pDXGIFactory1 = (void *)0;
    m_pDXGIAdapter1 = (void *)0;

} // void DXGI1Device::Close(void)

bool DXGI1Device::Init(const mfxU32 adapterNum) {
    IDXGIFactory1 *pFactory = NULL;
    IDXGIAdapter1 *pAdapter = NULL;
    DXGI_ADAPTER_DESC1 desc = { 0 };
    mfxU32 curAdapter       = 0;
    mfxU32 maxAdapters      = 0;
    HRESULT hRes            = E_FAIL;

    // load up the library if it is not loaded
    if (NULL == m_hModule) {
        LoadDLLModule(L"dxgi.dll");
    }

    if (NULL == m_pDXGIFactory1) {
        DXGICreateFactoryFunc pFunc = NULL;

        if (m_hModule) {
            // load address of procedure to create DXGI 1.1 factory
            pFunc = (DXGICreateFactoryFunc)GetProcAddress(m_hModule, "CreateDXGIFactory1");
        }

        if (NULL == pFunc) {
            return false;
        }

        // create the factory
#if _MSC_VER >= 1400
        hRes = pFunc(__uuidof(IDXGIFactory1), (void **)(&m_pDXGIFactory1));
#else
        hRes = pFunc(IID_IDXGIFactory1, (void **)(&m_pDXGIFactory1));
#endif

        if (FAILED(hRes)) {
            return false;
        }
    }

    pFactory = (IDXGIFactory1 *)m_pDXGIFactory1;

    // get the number of adapters
    curAdapter  = 0;
    maxAdapters = 0;

    if (m_pDXGIAdapter1) {
        ((IDXGIAdapter1 *)m_pDXGIAdapter1)->Release();
        m_pDXGIAdapter1 = (void *)0;
    }

    do {
        // get the required adapted
        hRes = pFactory->EnumAdapters1(curAdapter, &pAdapter);
        if (FAILED(hRes)) {
            break;
        }

        // if it is the required adapter, save the interface
        if (curAdapter == adapterNum) {
            m_pDXGIAdapter1 = pAdapter;
        }
        else {
            pAdapter->Release();
        }

        // get the next adapter
        curAdapter += 1;

    } while (SUCCEEDED(hRes));
    maxAdapters = curAdapter;

    m_numAdapters = maxAdapters;

    // there is no required adapter
    if (adapterNum >= maxAdapters) {
        return false;
    }
    pAdapter = (IDXGIAdapter1 *)m_pDXGIAdapter1;

    // get the adapter's parameters
    hRes = pAdapter->GetDesc1(&desc);
    if (FAILED(hRes)) {
        return false;
    }

    // save the parameters
    m_vendorID = desc.VendorId;
    m_deviceID = desc.DeviceId;
    m_luid     = LUIDtomfxU64(desc.AdapterLuid);

    return true;

} // bool DXGI1Device::Init(const mfxU32 adapterNum)

bool DXGI1Device::GetAdapterList(std::vector<DXGI1DeviceInfo> &adapterInfo) {
    mfxModuleHandle hModule = MFX::mfx_dll_load(L"dxgi.dll");
    if (!hModule)
        return false;

    DXGICreateFactoryFunc pFactoryFunc = NULL;
    pFactoryFunc = (DXGICreateFactoryFunc)MFX::mfx_dll_get_addr(hModule, "CreateDXGIFactory1");
    if (pFactoryFunc == NULL)
        return false;

    IDXGIFactory1 *pFactory = NULL;
    IDXGIAdapter1 *pAdapter = NULL;

    HRESULT hRes = pFactoryFunc(__uuidof(IDXGIFactory1), (void **)(&pFactory));
    if (FAILED(hRes))
        return false;

    bool bEnumSuccess = false;
    mfxU32 curAdapter = 0;
    while (pFactory->EnumAdapters1(curAdapter, &pAdapter) == S_OK) {
        curAdapter++;

        DXGI_ADAPTER_DESC1 desc = {};
        DXGI1DeviceInfo devInfo = {};
        if (pAdapter->GetDesc1(&desc) == S_OK) {
            // save minimal descriptive info
            devInfo.vendorID = desc.VendorId;
            devInfo.deviceID = desc.DeviceId;
            devInfo.luid     = LUIDtomfxU64(desc.AdapterLuid);

            // add to list
            adapterInfo.emplace_back(devInfo);

            // at least one valid adapter found
            bEnumSuccess = true;
        }

        pAdapter->Release();
    }
    pFactory->Release();

    mfx_dll_free(hModule);

    return bEnumSuccess;
}

DXVA2Device::DXVA2Device(void) {
    m_numAdapters = 0;

    m_vendorID = 0;
    m_deviceID = 0;

    m_driverVersion = 0;

    m_luid = {};
} // DXVA2Device::DXVA2Device(void)

DXVA2Device::~DXVA2Device(void) {
    Close();

} // DXVA2Device::~DXVA2Device(void)

void DXVA2Device::Close(void) {
    m_numAdapters = 0;

    m_vendorID = 0;
    m_deviceID = 0;

    m_driverVersion = 0;
} // void DXVA2Device::Close(void)

#ifdef MFX_D3D9_ENABLED
bool DXVA2Device::InitD3D9(const mfxU32 adapterNum) {
    D3D9Device &d3d9Device = m_d3d9Device;
    bool bRes;

    // release the object before initialization
    Close();

    // create 'old fashion' device
    bRes = d3d9Device.Init(adapterNum);
    if (false == bRes) {
        return false;
    }

    m_numAdapters = d3d9Device.GetAdapterCount();

    // check if the application is under Remote Desktop
    if ((0 == d3d9Device.GetVendorID()) || (0 == d3d9Device.GetDeviceID())) {
        // get the required parameters alternative way and ...
        UseAlternativeWay(d3d9Device);
    }
    else {
        // save the parameters and ...
        m_vendorID      = d3d9Device.GetVendorID();
        m_deviceID      = d3d9Device.GetDeviceID();
        m_driverVersion = d3d9Device.GetDriverVersion();
        m_luid          = d3d9Device.GetLUID();
    }

    // ... say goodbye
    return true;
} // bool InitD3D9(const mfxU32 adapterNum)
#else // MFX_D3D9_ENABLED
bool DXVA2Device::InitD3D9(const mfxU32 adapterNum) {
    (void)adapterNum;
    return false;
}
#endif // MFX_D3D9_ENABLED

bool DXVA2Device::InitDXGI1(const mfxU32 adapterNum) {
    DXGI1Device &dxgi1Device = m_dxgi1Device;
    bool bRes;

    // release the object before initialization
    Close();

    // create modern DXGI device
    bRes = dxgi1Device.Init(adapterNum);
    if (false == bRes) {
        return false;
    }

    // save the parameters and ...
    m_vendorID    = dxgi1Device.GetVendorID();
    m_deviceID    = dxgi1Device.GetDeviceID();
    m_numAdapters = dxgi1Device.GetAdapterCount();
    m_luid        = dxgi1Device.GetLUID();

    // ... say goodbye
    return true;

} // bool DXVA2Device::InitDXGI1(const mfxU32 adapterNum)

#ifdef MFX_D3D9_ENABLED
void DXVA2Device::UseAlternativeWay(const D3D9Device &pD3D9Device) {
    mfxU64 d3d9LUID     = pD3D9Device.GetLUID();
    mfxU64 kInvalidLUID = {};
    // work only with valid LUIDs
    if (kInvalidLUID == d3d9LUID) {
        return;
    }

    DXGI1Device &dxgi1Device = m_dxgi1Device;
    mfxU32 curDevice         = 0;
    bool bRes                = false;

    do {
        // initialize the next DXGI1 or DXGI device
        bRes = dxgi1Device.Init(curDevice);
        if (false == bRes) {
            // there is no more devices
            break;
        }

        // is it required device ?
        if (d3d9LUID == dxgi1Device.GetLUID()) {
            m_vendorID      = dxgi1Device.GetVendorID();
            m_deviceID      = dxgi1Device.GetDeviceID();
            m_driverVersion = dxgi1Device.GetDriverVersion();
            m_luid          = dxgi1Device.GetLUID();
            return;
        }

        // get the next device
        curDevice += 1;

    } while (bRes);

    dxgi1Device.Close();
    // we need to match a DXGI(1) device to the D3D9 device

} // void DXVA2Device::UseAlternativeWay(const D3D9Device *pD3D9Device)
#endif // MFX_D3D9_ENABLED

mfxU32 DXVA2Device::GetVendorID(void) const {
    return m_vendorID;

} // mfxU32 DXVA2Device::GetVendorID(void) const

mfxU32 DXVA2Device::GetDeviceID(void) const {
    return m_deviceID;

} // mfxU32 DXVA2Device::GetDeviceID(void) const

mfxU64 DXVA2Device::GetDriverVersion(void) const {
    return m_driverVersion;
} // mfxU64 DXVA2Device::GetDriverVersion(void) const

mfxU32 DXVA2Device::GetAdapterCount(void) const {
    return m_numAdapters;

} // mfxU32 DXVA2Device::GetAdapterCount(void) const

mfxU64 DXVA2Device::GetLUID(void) const {
    return m_luid;
} // mfxU64 DXVA2Device::GetLUID(void) const
