/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/windows/mfx_library_iterator.h"

#include "src/windows/mfx_dispatcher.h"
#include "src/windows/mfx_dispatcher_log.h"

#include "src/windows/mfx_dxva2_device.h"
#include "src/windows/mfx_load_dll.h"

#include <tchar.h>
#include <windows.h>

#include <vector>

namespace MFX {

enum { MFX_MAX_MERIT = 0x7fffffff };

//
// declare registry keys
//

const wchar_t rootDispPath[]    = L"Software\\Intel\\MediaSDK\\Dispatch";
const wchar_t vendorIDKeyName[] = L"VendorID";
const wchar_t deviceIDKeyName[] = L"DeviceID";
const wchar_t meritKeyName[]    = L"Merit";
const wchar_t pathKeyName[]     = L"Path";
const wchar_t apiVersionName[]  = L"APIVersion";

mfxStatus SelectImplementationType(DXVA2Device &dxvaDevice,
                                   const mfxU32 adapterNum,
                                   mfxIMPL *pImplInterface,
                                   mfxU32 *pVendorID,
                                   mfxU32 *pDeviceID,
                                   mfxU64 *pLUID) {
    if (NULL == pImplInterface) {
        return MFX_ERR_NULL_PTR;
    }
    mfxIMPL impl_via = *pImplInterface;

    if (MFX_IMPL_VIA_D3D9 == impl_via) {
        // try to create the Direct3D 9 device and find right adapter
        if (!dxvaDevice.InitD3D9(adapterNum)) {
            DISPATCHER_LOG_INFO((("dxvaDevice.InitD3D9(%d) Failed "), adapterNum));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (MFX_IMPL_VIA_D3D11 == impl_via) {
        // try to open DXGI 1.1 device to get hardware ID
        if (!dxvaDevice.InitDXGI1(adapterNum)) {
            DISPATCHER_LOG_INFO((("dxvaDevice.InitDXGI1(%d) Failed "), adapterNum));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (MFX_IMPL_VIA_ANY == impl_via) {
        // try the Direct3D 9 device
        if (dxvaDevice.InitD3D9(adapterNum)) {
            *pImplInterface = MFX_IMPL_VIA_D3D9; // store value for GetImplementationType() call
        }
        // else try to open DXGI 1.1 device to get hardware ID
        else if (dxvaDevice.InitDXGI1(adapterNum)) {
            *pImplInterface = MFX_IMPL_VIA_D3D11; // store value for GetImplementationType() call
        }
        else {
            DISPATCHER_LOG_INFO((("Unsupported adapter %d "), adapterNum));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else {
        DISPATCHER_LOG_ERROR((("Unknown implementation type %d "), *pImplInterface));
        return MFX_ERR_UNSUPPORTED;
    }

    // obtain card's parameters
    if (pVendorID && pDeviceID) {
        *pVendorID = dxvaDevice.GetVendorID();
        *pDeviceID = dxvaDevice.GetDeviceID();
    }

    if (pLUID) {
        *pLUID = dxvaDevice.GetLUID();
    }

    return MFX_ERR_NONE;
}

mfxStatus SelectImplementationType(DXVA2Device &dxvaDevice,
                                   const mfxU32 adapterNum,
                                   mfxIMPL *pImplInterface,
                                   mfxU32 *pVendorID,
                                   mfxU32 *pDeviceID) {
    // do not return LUID
    return SelectImplementationType(dxvaDevice,
                                    adapterNum,
                                    pImplInterface,
                                    pVendorID,
                                    pDeviceID,
                                    nullptr);
}

mfxStatus SelectImplementationType2(const mfxU32 adapterNum,
                                    mfxIMPL *pImplInterface,
                                    mfxU32 *pVendorID,
                                    mfxU32 *pDeviceID,
                                    mfxU64 *pLUID) {
    DXVA2Device dxvaDevice;
    // do not return LUID
    return SelectImplementationType(dxvaDevice,
                                    adapterNum,
                                    pImplInterface,
                                    pVendorID,
                                    pDeviceID,
                                    pLUID);
}

MFXLibraryIterator::MFXLibraryIterator(DXVA2Device &dxvaDevice)
        : m_dxvaDevice(dxvaDevice)
#if !defined(MEDIASDK_UWP_DISPATCHER)
          ,
          m_baseRegKey()
#endif
{
    m_implType      = MFX_LIB_PSEUDO;
    m_implInterface = MFX_IMPL_UNSUPPORTED;

    m_vendorID = 0;
    m_deviceID = 0;

    m_lastLibIndex = 0;
    m_lastLibMerit = MFX_MAX_MERIT;

    m_bIsSubKeyValid = 0;
    m_StorageID      = 0;

    m_SubKeyName[0]     = 0;
    m_driverStoreDir[0] = 0;
} // MFXLibraryIterator::MFXLibraryIterator(void)

MFXLibraryIterator::~MFXLibraryIterator(void) {
    Release();

} // MFXLibraryIterator::~MFXLibraryIterator(void)

void MFXLibraryIterator::Release(void) {
    m_implType      = MFX_LIB_PSEUDO;
    m_implInterface = MFX_IMPL_UNSUPPORTED;

    m_vendorID = 0;
    m_deviceID = 0;

    m_lastLibIndex  = 0;
    m_lastLibMerit  = MFX_MAX_MERIT;
    m_SubKeyName[0] = 0;

} // void MFXLibraryIterator::Release(void)

DECLSPEC_NOINLINE HMODULE GetThisDllModuleHandle() {
    HMODULE hDll = NULL;

    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&GetThisDllModuleHandle),
        &hDll);
    return hDll;
}

// wchar_t* sImplPath must be allocated with size not less then msdk_disp_path_len
bool GetImplPath(int storageID, wchar_t *sImplPath) {
    HMODULE hModule = NULL;

    sImplPath[0] = L'\0';

    switch (storageID) {
        case MFX_APP_FOLDER:
            hModule = 0;
            break;
        case MFX_PATH_MSDK_FOLDER:
            hModule           = GetThisDllModuleHandle();
            HMODULE exeModule = GetModuleHandleW(NULL);
            //It should works only if Dispatcher is linked with Dynamic Linked Library
            if (!hModule || !exeModule || hModule == exeModule)
                return false;
            break;
    }

    DWORD nSize     = 0;
    DWORD allocSize = msdk_disp_path_len;

    nSize = GetModuleFileNameW(hModule, &sImplPath[0], allocSize);

    if (nSize == 0 || nSize == allocSize) {
        // nSize == 0 meanse that system can't get this info for hModule
        // nSize == allocSize buffer is too small
        return false;
    }

    // for any case because WinXP implementation of GetModuleFileName does not add \0 to the end of string
    sImplPath[nSize] = L'\0';

    wchar_t *dirSeparator = wcsrchr(sImplPath, L'\\');
    if (dirSeparator != NULL && dirSeparator < (sImplPath + msdk_disp_path_len)) {
        *++dirSeparator = 0;
    }
    return true;
}

mfxStatus MFXLibraryIterator::Init(eMfxImplType implType,
                                   mfxIMPL implInterface,
                                   const mfxU32 adapterNum,
                                   int storageID) {
    // check error(s)
    if ((MFX_LIB_SOFTWARE != implType) && (MFX_LIB_HARDWARE != implType)) {
        return MFX_ERR_UNSUPPORTED;
    }

    // release the object before initialization
    Release();
    m_StorageID     = storageID;
    m_lastLibIndex  = 0;
    m_implType      = implType;
    m_implInterface = implInterface != 0 ? implInterface : MFX_IMPL_VIA_ANY;

    // for HW impl check impl interface, check adapter, obtain deviceID and vendorID
    if (m_implType != MFX_LIB_SOFTWARE) {
        mfxStatus mfxRes = MFX::SelectImplementationType(m_dxvaDevice,
                                                         adapterNum,
                                                         &m_implInterface,
                                                         &m_vendorID,
                                                         &m_deviceID);
        if (MFX_ERR_NONE != mfxRes) {
            return mfxRes;
        }
    }

#if !defined(MEDIASDK_UWP_DISPATCHER)
    if (storageID == MFX_CURRENT_USER_KEY || storageID == MFX_LOCAL_MACHINE_KEY ||
        storageID == MFX_CURRENT_USER_KEY_ONEVPL || storageID == MFX_LOCAL_MACHINE_KEY_ONEVPL) {
        return InitRegistry(storageID);
    }

    #if defined(MFX_TRACER_WA_FOR_DS)
    if (storageID == MFX_TRACER) {
        return InitRegistryTracer();
    }
    #endif

#endif
    if (storageID == MFX_DRIVER_STORE) {
        m_driverStoreDir[0] = 0;
        if (!m_driverStoreLoader.GetDriverStorePath(m_driverStoreDir,
                                                    sizeof(m_driverStoreDir),
                                                    m_deviceID,
                                                    L"DriverStorePathForMediaSDK")) {
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (storageID == MFX_DRIVER_STORE_ONEVPL || storageID == MFX_DRIVER_STORE_ONEVPL_MFXINIT) {
        // get path to runtime directory only (without library name)
        m_driverStoreDir[0] = 0;
        if (!m_driverStoreLoader.GetDriverStorePath(m_driverStoreDir,
                                                    sizeof(m_driverStoreDir),
                                                    m_deviceID,
                                                    L"DriverStorePathForVPL")) {
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (!GetImplPath(storageID, m_driverStoreDir)) {
        return MFX_ERR_UNSUPPORTED;
    }

    // only need the path for Intel® Video Processing Library (Intel® VPL) loader
    if (storageID == MFX_DRIVER_STORE_ONEVPL || storageID == MFX_CURRENT_USER_KEY_ONEVPL ||
        storageID == MFX_LOCAL_MACHINE_KEY_ONEVPL) {
        return MFX_ERR_NONE;
    }

    return InitFolder(implType, m_driverStoreDir, storageID);

} // mfxStatus MFXLibraryIterator::Init(eMfxImplType implType, const mfxU32 adapterNum, int storageID)

mfxStatus MFXLibraryIterator::InitRegistry(int storageID) {
#if !defined(MEDIASDK_UWP_DISPATCHER)
    HKEY rootHKey;
    bool bRes;

    // open required registry key
    switch (storageID) {
        case MFX_LOCAL_MACHINE_KEY:
        case MFX_LOCAL_MACHINE_KEY_ONEVPL:
            rootHKey = HKEY_LOCAL_MACHINE;
            break;
        default:
            rootHKey = HKEY_CURRENT_USER;
            break;
    }

    bRes = m_baseRegKey.Open(rootHKey, rootDispPath, KEY_READ);
    if (false == bRes) {
        DISPATCHER_LOG_WRN(
            (("Can't open %s\\%S : RegOpenKeyExA()==0x%x\n"),
             (MFX_LOCAL_MACHINE_KEY == storageID) ? ("HKEY_LOCAL_MACHINE") : ("HKEY_CURRENT_USER"),
             rootDispPath,
             GetLastError()))
        return MFX_ERR_UNKNOWN;
    }

    DISPATCHER_LOG_INFO(
        (("Inspecting %s\\%S\n"),
         (MFX_LOCAL_MACHINE_KEY == storageID) ? ("HKEY_LOCAL_MACHINE") : ("HKEY_CURRENT_USER"),
         rootDispPath))

    return MFX_ERR_NONE;
#else
    (void)storageID;
    return MFX_ERR_UNSUPPORTED;
#endif // #if !defined(MEDIASDK_UWP_DISPATCHER)

} // mfxStatus MFXLibraryIterator::InitRegistry(int storageID)

#if defined(MFX_TRACER_WA_FOR_DS)
mfxStatus MFXLibraryIterator::InitRegistryTracer() {
    #if !defined(MEDIASDK_UWP_DISPATCHER)

    const wchar_t tracerRegKeyPath[] = L"Software\\Intel\\MediaSDK\\Dispatch\\tracer";

    if (!m_baseRegKey.Open(HKEY_LOCAL_MACHINE, tracerRegKeyPath, KEY_READ) &&
        !m_baseRegKey.Open(HKEY_CURRENT_USER, tracerRegKeyPath, KEY_READ)) {
        DISPATCHER_LOG_WRN(("can't find tracer registry key\n"))
        return MFX_ERR_UNKNOWN;
    }

    DISPATCHER_LOG_INFO(("found tracer registry key\n"))
    return MFX_ERR_NONE;

    #else
    return MFX_ERR_UNSUPPORTED;
    #endif // #if !defined(MEDIASDK_UWP_DISPATCHER)

} // mfxStatus MFXLibraryIterator::InitRegistryTracer()
#endif

mfxStatus MFXLibraryIterator::InitFolder(eMfxImplType implType,
                                         const wchar_t *path,
                                         const int storageID) {
    const int maxPathLen = sizeof(m_path) / sizeof(m_path[0]);
    m_path[0]            = 0;
    wcscpy_s(m_path, maxPathLen, path);
    size_t pathLen = wcslen(m_path);

    if (storageID == MFX_APP_FOLDER) {
        // we looking for runtime in application folder, it should be named libmfxsw64 or libmfxsw32
        mfx_get_default_dll_name(m_path + pathLen, msdk_disp_path_len - pathLen, MFX_LIB_SOFTWARE);
    }
    else if (storageID == MFX_DRIVER_STORE_ONEVPL_MFXINIT) {
        mfx_get_default_vpl_dll_name(m_path + pathLen, msdk_disp_path_len - pathLen);
    }
    else {
        mfx_get_default_dll_name(m_path + pathLen, msdk_disp_path_len - pathLen, implType);
    }

    return MFX_ERR_NONE;
} // mfxStatus MFXLibraryIterator::InitFolder(eMfxImplType implType, const wchar_t * path, const int storageID)

mfxStatus MFXLibraryIterator::SelectDLLVersion(wchar_t *pPath,
                                               size_t pathSize,
                                               eMfxImplType *pImplType,
                                               mfxVersion minVersion) {
    UNREFERENCED_PARAMETER(minVersion);

    if (m_StorageID == MFX_APP_FOLDER) {
        if (m_lastLibIndex != 0)
            return MFX_ERR_NOT_FOUND;
        if (m_vendorID != INTEL_VENDOR_ID)
            return MFX_ERR_UNKNOWN;

        m_lastLibIndex = 1;
        wcscpy_s(pPath, pathSize, m_path);
        *pImplType = MFX_LIB_SOFTWARE;
        return MFX_ERR_NONE;
    }

    if (m_StorageID == MFX_PATH_MSDK_FOLDER || m_StorageID == MFX_DRIVER_STORE ||
        m_StorageID == MFX_DRIVER_STORE_ONEVPL_MFXINIT) {
        if (m_lastLibIndex != 0)
            return MFX_ERR_NOT_FOUND;
        if (m_vendorID != INTEL_VENDOR_ID)
            return MFX_ERR_UNKNOWN;

        m_lastLibIndex = 1;
        wcscpy_s(pPath, pathSize, m_path);
        // do not change impl type
        return MFX_ERR_NONE;
    }

#if !defined(MEDIASDK_UWP_DISPATCHER)

    #if defined(MFX_TRACER_WA_FOR_DS)
    if (m_StorageID == MFX_TRACER) {
        if (m_lastLibIndex != 0)
            return MFX_ERR_NOT_FOUND;
        if (m_vendorID != INTEL_VENDOR_ID)
            return MFX_ERR_UNKNOWN;

        m_lastLibIndex = 1;

        if (m_baseRegKey.Query(pathKeyName, REG_SZ, (LPBYTE)pPath, (DWORD *)&pathSize)) {
            DISPATCHER_LOG_INFO((("loaded %S : %S\n"), pathKeyName, pPath));
        }
        else {
            DISPATCHER_LOG_WRN(
                (("error querying %S : RegQueryValueExA()==0x%x\n"), pathKeyName, GetLastError()));
        }
        return MFX_ERR_NONE;
    }
    #endif

    wchar_t libPath[MFX_MAX_DLL_PATH] = L"";
    DWORD libIndex                    = 0;
    DWORD libMerit                    = 0;
    DWORD index;
    bool enumRes;

    // main query cycle
    index            = 0;
    m_bIsSubKeyValid = false;
    do {
        WinRegKey subKey;
        wchar_t subKeyName[MFX_MAX_REGISTRY_KEY_NAME] = { 0 };
        DWORD subKeyNameSize                          = sizeof(subKeyName) / sizeof(subKeyName[0]);

        // query next value name
        enumRes = m_baseRegKey.EnumKey(index, subKeyName, &subKeyNameSize);
        if (!enumRes) {
            DISPATCHER_LOG_WRN((("no more subkeys : RegEnumKeyExA()==0x%x\n"), GetLastError()))
        }
        else {
            DISPATCHER_LOG_INFO((("found subkey: %S\n"), subKeyName))

            bool bRes;

            // open the sub key
            bRes = subKey.Open(m_baseRegKey, subKeyName, KEY_READ);
            if (!bRes) {
                DISPATCHER_LOG_WRN((("error opening key %S :RegOpenKeyExA()==0x%x\n"),
                                    subKeyName,
                                    GetLastError()));
            }
            else {
                DISPATCHER_LOG_INFO((("opened key: %S\n"), subKeyName));

                mfxU32 vendorID = 0, deviceID = 0, merit = 0;
                DWORD size;

                // query vendor and device IDs
                size = sizeof(vendorID);
                bRes = subKey.Query(vendorIDKeyName, REG_DWORD, (LPBYTE)&vendorID, &size);
                DISPATCHER_LOG_OPERATION({
                    if (bRes) {
                        DISPATCHER_LOG_INFO((("loaded %S : 0x%x\n"), vendorIDKeyName, vendorID));
                    }
                    else {
                        DISPATCHER_LOG_WRN((("querying %S : RegQueryValueExA()==0x%x\n"),
                                            vendorIDKeyName,
                                            GetLastError()));
                    }
                })

                if (bRes) {
                    size = sizeof(deviceID);
                    bRes = subKey.Query(deviceIDKeyName, REG_DWORD, (LPBYTE)&deviceID, &size);
                    DISPATCHER_LOG_OPERATION({
                        if (bRes) {
                            DISPATCHER_LOG_INFO(
                                (("loaded %S : 0x%x\n"), deviceIDKeyName, deviceID));
                        }
                        else {
                            DISPATCHER_LOG_WRN((("querying %S : RegQueryValueExA()==0x%x\n"),
                                                deviceIDKeyName,
                                                GetLastError()));
                        }
                    })
                }
                // query merit value
                if (bRes) {
                    size = sizeof(merit);
                    bRes = subKey.Query(meritKeyName, REG_DWORD, (LPBYTE)&merit, &size);
                    DISPATCHER_LOG_OPERATION({
                        if (bRes) {
                            DISPATCHER_LOG_INFO((("loaded %S : %d\n"), meritKeyName, merit));
                        }
                        else {
                            DISPATCHER_LOG_WRN((("querying %S : RegQueryValueExA()==0x%x\n"),
                                                meritKeyName,
                                                GetLastError()));
                        }
                    })
                }

                // if the library fits required parameters,
                // query the library's path
                if (bRes) {
                    // compare device's and library's IDs
                    if (MFX_LIB_HARDWARE == m_implType) {
                        if (m_vendorID != vendorID) {
                            bRes = false;
                            DISPATCHER_LOG_WRN((("%S conflict, actual = 0x%x : required = 0x%x\n"),
                                                vendorIDKeyName,
                                                m_vendorID,
                                                vendorID));
                        }
                        if (bRes && m_deviceID != deviceID) {
                            bRes = false;
                            DISPATCHER_LOG_WRN((("%S conflict, actual = 0x%x : required = 0x%x\n"),
                                                deviceIDKeyName,
                                                m_deviceID,
                                                deviceID));
                        }
                    }

                    DISPATCHER_LOG_OPERATION({
                        if (bRes) {
                            if (!(((m_lastLibMerit > merit) ||
                                   ((m_lastLibMerit == merit) && (m_lastLibIndex < index))) &&
                                  (libMerit < merit))) {
                                DISPATCHER_LOG_WRN((
                                    ("merit conflict: lastMerit = 0x%x, requiredMerit = 0x%x, libraryMerit = 0x%x, lastindex = %d, index = %d\n"),
                                    m_lastLibMerit,
                                    merit,
                                    libMerit,
                                    m_lastLibIndex,
                                    index));
                            }
                        }
                    })

                    if ((bRes) &&
                        ((m_lastLibMerit > merit) ||
                         ((m_lastLibMerit == merit) && (m_lastLibIndex < index))) &&
                        (libMerit < merit)) {
                        wchar_t tmpPath[MFX_MAX_DLL_PATH];
                        DWORD tmpPathSize = sizeof(tmpPath);

                        bRes = subKey.Query(pathKeyName, REG_SZ, (LPBYTE)tmpPath, &tmpPathSize);
                        if (!bRes) {
                            DISPATCHER_LOG_WRN((("error querying %S : RegQueryValueExA()==0x%x\n"),
                                                pathKeyName,
                                                GetLastError()));
                        }
                        else {
                            DISPATCHER_LOG_INFO((("loaded %S : %S\n"), pathKeyName, tmpPath));

                            wcscpy_s(libPath, sizeof(libPath) / sizeof(libPath[0]), tmpPath);
                            wcscpy_s(m_SubKeyName,
                                     sizeof(m_SubKeyName) / sizeof(m_SubKeyName[0]),
                                     subKeyName);

                            libMerit = merit;
                            libIndex = index;

                            // set the library's type
                            if ((0 == vendorID) || (0 == deviceID)) {
                                *pImplType = MFX_LIB_SOFTWARE;
                                DISPATCHER_LOG_INFO((("Library type is MFX_LIB_SOFTWARE\n")));
                            }
                            else {
                                *pImplType = MFX_LIB_HARDWARE;
                                DISPATCHER_LOG_INFO((("Library type is MFX_LIB_HARDWARE\n")));
                            }
                        }
                    }
                }
            }
        }

        // advance key index
        index += 1;

    } while (enumRes);

    // if the library's path was successfully read,
    // the merit variable holds valid value
    if (0 == libMerit) {
        return MFX_ERR_NOT_FOUND;
    }

    wcscpy_s(pPath, pathSize, libPath);

    m_lastLibIndex   = libIndex;
    m_lastLibMerit   = libMerit;
    m_bIsSubKeyValid = true;

#endif

    return MFX_ERR_NONE;

} // mfxStatus MFXLibraryIterator::SelectDLLVersion(wchar_t *pPath, size_t pathSize, eMfxImplType *pImplType, mfxVersion minVersion)

mfxIMPL MFXLibraryIterator::GetImplementationType() {
    return m_implInterface;
} // mfxIMPL MFXLibraryIterator::GetImplementationType()

bool MFXLibraryIterator::GetSubKeyName(wchar_t *subKeyName, size_t length) const {
    wcscpy_s(subKeyName, length, m_SubKeyName);
    return m_bIsSubKeyValid;
}

// lightweight implementation that takes deviceID as an argument, avoiding need to init the adapter
mfxStatus MFXLibraryIterator::GetDriverStoreDir(std::wstring &driverStoreDir,
                                                size_t length,
                                                mfxU32 deviceID,
                                                int storageID) {
    wchar_t wcDir[MFX_MAX_DLL_PATH];
    wcDir[0] = 0;

    DriverStoreLoader dsLoader;

    if (storageID == MFX_DRIVER_STORE_ONEVPL) {
        // pass size of wcDir in bytes (see implementation of GetDriverStorePath)
        if (!dsLoader.GetDriverStorePath(wcDir, sizeof(wcDir), deviceID, L"DriverStorePathForVPL"))
            return MFX_ERR_UNSUPPORTED;
    }
    else if (storageID == MFX_DRIVER_STORE) {
        if (!dsLoader.GetDriverStorePath(wcDir,
                                         sizeof(wcDir),
                                         deviceID,
                                         L"DriverStorePathForMediaSDK"))
            return MFX_ERR_UNSUPPORTED;
    }

    if (wcslen(wcDir) == 0)
        return MFX_ERR_UNSUPPORTED;

    // return path to driverStorDir
    driverStoreDir = wcDir;

    return MFX_ERR_NONE;
}

mfxStatus MFXLibraryIterator::GetRegkeyDir(std::wstring &regDir, size_t length, int storageID) {
    mfxStatus sts = MFX_ERR_UNSUPPORTED;
    DXVA2Device dxvaDevice;
    MFX::MFXLibraryIterator libIterator(dxvaDevice);
    wchar_t wRegDir[MFX_MAX_DLL_PATH];

    regDir.clear();
    sts = libIterator.Init(MFX_LIB_HARDWARE, MFX_IMPL_VIA_D3D11, 0, storageID);
    if (sts)
        return MFX_ERR_UNSUPPORTED;

    eMfxImplType implType = MFX_LIB_HARDWARE;
    mfxVersion ver        = { 0, 1 };
    sts =
        libIterator.SelectDLLVersion(wRegDir, sizeof(wRegDir) / sizeof(wRegDir[0]), &implType, ver);
    if (sts)
        return MFX_ERR_UNSUPPORTED;

    // remove DLL name - only need the path
    std::wstring s = wRegDir;
    size_t f       = s.find_last_of('\\');
    if (f == std::string::npos)
        return MFX_ERR_UNSUPPORTED;

    regDir = s.substr(0, f);

    return MFX_ERR_NONE;
}

} // namespace MFX
