/*############################################################################
  # Copyright (C) 2021 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "vpl_implementation_loader.h"
#include "sample_defs.h"
#include "sample_utils.h"
#include "vpl/mfxdispatcher.h"

#if defined(LINUX32) || defined(LINUX64)
    #include <link.h>
    #include <string.h>
#else
    #include <atlbase.h>
    #include <dxgi.h>
#endif
#if defined(_WIN32)
    #include <dxgi.h>
#endif

#include <map>
#include <regex>

static mfxI32 GetAdapterNumber(const mfxChar* cDeviceID) {
    std::string strDevID(cDeviceID);
    size_t idx        = strDevID.rfind('/');
    mfxI32 adapterIdx = -1;

    if (idx != std::string::npos && (idx + 1) < strDevID.size())
        adapterIdx = std::stoi(strDevID.substr(idx + 1));

    return adapterIdx;
}

#if defined(_WIN32)
static mfxU16 FoundSuitableAdapter() {
    HRESULT hRes = E_FAIL;

    CComPtr<IDXGIFactory> pFactory;
    hRes = CreateDXGIFactory(__uuidof(pFactory), reinterpret_cast<void**>(&pFactory));
    if (FAILED(hRes)) {
        return -1;
    }

    CComPtr<IDXGIAdapter> pAdapter;
    DXGI_ADAPTER_DESC desc = { 0 };
    for (UINT i = 0; SUCCEEDED(hRes = pFactory->EnumAdapters(i, &pAdapter)); ++i) {
        hRes = pAdapter->GetDesc(&desc);
        if (FAILED(hRes)) {
            return -1;
        }

        if (desc.VendorId == 0x8086)
            return i;

        pAdapter = nullptr;
    }
    return -1;
}
#endif

const std::map<mfxAccelerationMode, const std::string> mfxAccelerationModeNames = {
    { MFX_ACCEL_MODE_NA, "MFX_ACCEL_MODE_NA" },
    { MFX_ACCEL_MODE_VIA_D3D9, "MFX_ACCEL_MODE_VIA_D3D9" },
    { MFX_ACCEL_MODE_VIA_D3D11, "MFX_ACCEL_MODE_VIA_D3D11" },
    { MFX_ACCEL_MODE_VIA_VAAPI, "MFX_ACCEL_MODE_VIA_VAAPI" },
    { MFX_ACCEL_MODE_VIA_VAAPI_DRM_RENDER_NODE, "MFX_ACCEL_MODE_VIA_VAAPI_DRM_RENDER_NODE" },
    { MFX_ACCEL_MODE_VIA_VAAPI_DRM_MODESET, "MFX_ACCEL_MODE_VIA_VAAPI_DRM_MODESET" },
    { MFX_ACCEL_MODE_VIA_VAAPI_GLX, "MFX_ACCEL_MODE_VIA_VAAPI_GLX" },
    { MFX_ACCEL_MODE_VIA_VAAPI_X11, "MFX_ACCEL_MODE_VIA_VAAPI_X11" },
    { MFX_ACCEL_MODE_VIA_VAAPI_WAYLAND, "MFX_ACCEL_MODE_VIA_VAAPI_WAYLAND" },
    { MFX_ACCEL_MODE_VIA_HDDLUNITE, "MFX_ACCEL_MODE_VIA_HDDLUNITE" }
};

VPLImplementationLoader::VPLImplementationLoader()
        : m_loader(),
          m_idesc(),
          m_Loader(),
          m_ImplIndex(0),
          m_Impl(MFX_IMPL_TYPE_HARDWARE),
          m_adapterType(mfxMediaAdapterType::MFX_MEDIA_UNKNOWN),
          m_dGfxIdx(-1),
          m_adapterNum(-1),
          m_MinVersion(mfxVersion{ { 0 /*minor*/, 1 /*major*/ } }),
          m_PCIDomain(0),
          m_PCIBus(0),
          m_PCIDevice(0),
          m_PCIFunction(0),
          m_PCIDeviceSetup(false),
#if defined(_WIN32)
          m_LUID(0)
#else
          m_DRMRenderNodeNum(0),
          m_DRMRenderNodeNumUsed(0)
#endif
{
    m_loader.reset(MFXLoad(), MFXUnload);
    m_Loader = m_loader.get();
}

VPLImplementationLoader::~VPLImplementationLoader() {}

mfxStatus VPLImplementationLoader::CreateConfig(char const* data, const char* propertyName) {
    mfxConfig cfg = MFXCreateConfig(m_Loader);
    mfxVariant variant;
    variant.Type     = MFX_VARIANT_TYPE_PTR;
    variant.Data.Ptr = mfxHDL(data);
    mfxStatus sts    = MFXSetConfigFilterProperty(cfg, (mfxU8*)propertyName, variant);
    MSDK_CHECK_STATUS(sts, "MFXSetConfigFilterProperty failed");

    return sts;
}

mfxStatus VPLImplementationLoader::CreateConfig(mfxU16 data, const char* propertyName) {
    mfxConfig cfg = MFXCreateConfig(m_Loader);
    mfxVariant variant;
    variant.Type     = MFX_VARIANT_TYPE_U16;
    variant.Data.U32 = data;
    mfxStatus sts    = MFXSetConfigFilterProperty(cfg, (mfxU8*)propertyName, variant);
    MSDK_CHECK_STATUS(sts, "MFXSetConfigFilterProperty failed");

    return sts;
}

mfxStatus VPLImplementationLoader::CreateConfig(mfxU32 data, const char* propertyName) {
    mfxConfig cfg = MFXCreateConfig(m_Loader);
    mfxVariant variant;
    variant.Type     = MFX_VARIANT_TYPE_U32;
    variant.Data.U32 = data;
    mfxStatus sts    = MFXSetConfigFilterProperty(cfg, (mfxU8*)propertyName, variant);
    MSDK_CHECK_STATUS(sts, "MFXSetConfigFilterProperty failed");

    return sts;
}

mfxStatus VPLImplementationLoader::ConfigureImplementation(mfxIMPL impl) {
    mfxConfig cfgImpl = MFXCreateConfig(m_Loader);

    std::vector<mfxU32> hwImpls = { MFX_IMPL_HARDWARE,  MFX_IMPL_HARDWARE_ANY, MFX_IMPL_HARDWARE2,
                                    MFX_IMPL_HARDWARE3, MFX_IMPL_HARDWARE4,    MFX_IMPL_VIA_D3D9,
                                    MFX_IMPL_VIA_D3D11 };

    std::vector<mfxU32>::iterator hwImplsIt =
        std::find_if(hwImpls.begin(), hwImpls.end(), [impl](const mfxU32& val) {
            return (val == MFX_IMPL_VIA_MASK(impl) || val == MFX_IMPL_BASETYPE(impl));
        });

    if (MFX_IMPL_BASETYPE(impl) == MFX_IMPL_SOFTWARE) {
        m_Impl = MFX_IMPL_TYPE_SOFTWARE;
    }
    else if (hwImplsIt != hwImpls.end()) {
        m_Impl = MFX_IMPL_TYPE_HARDWARE;
    }
    else {
        return MFX_ERR_UNSUPPORTED;
    }

    mfxVariant ImplVariant;
    ImplVariant.Type     = MFX_VARIANT_TYPE_U32;
    ImplVariant.Data.U32 = m_Impl;
    mfxStatus sts =
        MFXSetConfigFilterProperty(cfgImpl, (mfxU8*)"mfxImplDescription.Impl", ImplVariant);
    MSDK_CHECK_STATUS(sts, "MFXSetConfigFilterProperty failed");
    printf("CONFIGURE LOADER: required implementation: %s \n",
           ImplVariant.Data.U32 == MFX_IMPL_TYPE_HARDWARE ? "hw" : "sw");
    return sts;
}

mfxStatus VPLImplementationLoader::ConfigureAccelerationMode(mfxAccelerationMode accelerationMode,
                                                             mfxIMPL impl) {
    mfxStatus sts = MFX_ERR_NONE;
    bool isHW     = MFX_IMPL_BASETYPE(impl) != MFX_IMPL_SOFTWARE;

    // configure accelerationMode, except when required implementation is MFX_IMPL_TYPE_HARDWARE, but m_accelerationMode not set
    if (accelerationMode != MFX_ACCEL_MODE_NA || !isHW) {
        sts = CreateConfig((mfxU32)accelerationMode, "mfxImplDescription.AccelerationMode");
        printf("CONFIGURE LOADER: required implementation mfxAccelerationMode: %s \n",
               mfxAccelerationModeNames.at(accelerationMode).c_str());
    }

    return sts;
}

mfxStatus VPLImplementationLoader::ConfigureVersion(mfxVersion const version) {
    mfxStatus sts = MFX_ERR_NONE;

    sts = CreateConfig(version.Version, "mfxImplDescription.ApiVersion.Version");
    sts = CreateConfig(version.Major, "mfxImplDescription.ApiVersion.Major");
    sts = CreateConfig(version.Minor, "mfxImplDescription.ApiVersion.Minor");
    printf("CONFIGURE LOADER: required version: %d.%d.%d\n",
           version.Major,
           version.Minor,
           version.Version);

    return sts;
}

#if defined(_WIN32)
mfxStatus VPLImplementationLoader::SetupLUID(LUID luid) {
    *((LUID*)&m_LUID) = luid;

    printf("CONFIGURE LOADER: required LUID %llx\n", *((mfxU64*)&luid));

    return MFX_ERR_NONE;
}
#else

mfxStatus VPLImplementationLoader::SetupDRMRenderNodeNum(mfxU32 DRMRenderNodeNum) {
    m_DRMRenderNodeNum = DRMRenderNodeNum;
    return MFX_ERR_NONE;
}

mfxU32 VPLImplementationLoader::GetDRMRenderNodeNumUsed() {
    return m_DRMRenderNodeNumUsed;
}

#endif

mfxStatus VPLImplementationLoader::SetPCIDevice(mfxI32 domain,
                                                mfxI32 bus,
                                                mfxI32 device,
                                                mfxI32 function) {
    m_PCIDomain      = domain;
    m_PCIBus         = bus;
    m_PCIDevice      = device;
    m_PCIFunction    = function;
    m_PCIDeviceSetup = true;
    printf("CONFIGURE LOADER: required domain:bus:device.function: %d:%d:%d.%d\n",
           domain,
           bus,
           device,
           function);

    return MFX_ERR_NONE;
}

void VPLImplementationLoader::SetAdapterType(mfxU16 adapterType) {
    if (m_adapterNum != -1 && adapterType != mfxMediaAdapterType::MFX_MEDIA_UNKNOWN) {
        printf(
            "CONFIGURE LOADER: required adapter type may conflict with adapter number, adapter type will be ignored \n");
    }
    else if (adapterType != mfxMediaAdapterType::MFX_MEDIA_UNKNOWN) {
        m_adapterType = adapterType;

        std::stringstream ss;
        std::cout << "CONFIGURE LOADER: required adapter type: "
                  << (m_adapterType == mfxMediaAdapterType::MFX_MEDIA_INTEGRATED ? "integrated"
                                                                                 : "discrete")
                  << std::endl;
    }
}

void VPLImplementationLoader::SetDiscreteAdapterIndex(mfxI32 dGfxIdx) {
    if (m_adapterNum != -1) {
        printf(
            "CONFIGURE LOADER: required adapter type may conflict with adapter number, adapter type will be ignored \n");
    }
    else if (dGfxIdx >= 0) {
        m_adapterType = mfxMediaAdapterType::MFX_MEDIA_DISCRETE;
        m_dGfxIdx     = dGfxIdx;
        printf("CONFIGURE LOADER: required discrete adapter index %d \n", m_dGfxIdx);
    }
}

void VPLImplementationLoader::SetAdapterNum(mfxI32 adapterNum) {
    if (adapterNum >= 0) {
        if (m_adapterType != mfxMediaAdapterType::MFX_MEDIA_UNKNOWN) {
            m_adapterType = mfxMediaAdapterType::MFX_MEDIA_UNKNOWN;
            m_dGfxIdx     = -1;
            printf(
                "CONFIGURE LOADER: required adapter type may conflict with adapter number, adapter type will be ignored \n");
        }
        m_adapterNum = adapterNum;
        printf("CONFIGURE LOADER: required adapter number: %d \n", m_adapterNum);
    }
}

mfxStatus VPLImplementationLoader::EnumImplementations() {
    mfxImplDescription* idesc        = nullptr;
    mfxExtendedDeviceId* idescDevice = nullptr;
    mfxStatus sts                    = MFX_ERR_NONE;

    std::vector<std::pair<mfxU32, mfxImplDescription*>> unique_devices;

    m_ImplIndex = (mfxU32)-1;
    for (int impl = 0; sts == MFX_ERR_NONE; impl++) {
        sts =
            MFXEnumImplementations(m_Loader, impl, MFX_IMPLCAPS_IMPLDESCSTRUCTURE, (mfxHDL*)&idesc);
        if (!idesc) {
            sts = MFX_ERR_NONE;
            if (impl == 0)
                printf("No implementation was found \n");
            break;
        }
        else if (idesc->ApiVersion < m_MinVersion) {
            continue;
        }

        sts = MFXEnumImplementations(m_Loader,
                                     impl,
                                     MFX_IMPLCAPS_DEVICE_ID_EXTENDED,
                                     (mfxHDL*)&idescDevice);
        if (idescDevice) {
            if (m_PCIDeviceSetup &&
                (idescDevice->PCIDomain != m_PCIDomain || idescDevice->PCIBus != m_PCIBus ||
                 idescDevice->PCIDevice != m_PCIDevice ||
                 idescDevice->PCIFunction != m_PCIFunction)) {
                continue;
            }

#if defined(_WIN32)
            if (m_LUID > 0) {
                if (!idescDevice->LUIDValid)
                    continue;

                mfxU64 tempLuid = m_LUID;

                bool luidEq = true;
                for (int i = 0; i < 8; i++, tempLuid >>= 8) {
                    if (mfxU8((tempLuid)&0xFF) != idescDevice->DeviceLUID[i]) {
                        luidEq = false;
                    }
                }
                if (!luidEq)
                    continue;
            }
#else
            if (m_DRMRenderNodeNum > 0) {
                if (idescDevice->DRMRenderNodeNum != m_DRMRenderNodeNum)
                    continue;
            }
#endif
        }
        else {
            sts = MFX_ERR_NONE;
            if (m_PCIDeviceSetup
#if defined(_WIN32)
                || m_LUID > 0
#endif
            ) {
                continue;
            }
        }

        // collect uniq devices, try to find if adapter already collected
        auto it = std::find_if(unique_devices.begin(),
                               unique_devices.end(),
                               [idesc](const std::pair<mfxU32, mfxImplDescription*> val) {
                                   return (GetAdapterNumber(idesc->Dev.DeviceID) ==
                                           GetAdapterNumber(val.second->Dev.DeviceID));
                               });
        // if adapter type is not specified, we give preference MFX_MEDIA_INTEGRATED
        if (it == unique_devices.end()) {
            unique_devices.push_back(std::make_pair(impl, idesc));
            if (m_adapterNum == -1 && m_adapterType == mfxMediaAdapterType::MFX_MEDIA_UNKNOWN &&
                idesc->Dev.MediaAdapterType == mfxMediaAdapterType::MFX_MEDIA_INTEGRATED) {
                m_adapterType = mfxMediaAdapterType::MFX_MEDIA_INTEGRATED;
                break;
            }
        }
    }

    if (unique_devices.empty()) {
        printf("Library was not found with required version \n");
        return MFX_ERR_NOT_FOUND;
    }

    std::sort(unique_devices.begin(),
              unique_devices.end(),
              [](const std::pair<mfxU32, mfxImplDescription*>& left,
                 const std::pair<mfxU32, mfxImplDescription*>& right) {
                  return GetAdapterNumber(left.second->Dev.DeviceID) <
                         GetAdapterNumber(right.second->Dev.DeviceID);
              });

    mfxI32 dGfxIdx = -1;
    for (const auto& it : unique_devices) {
        if (it.second->Dev.MediaAdapterType == mfxMediaAdapterType::MFX_MEDIA_DISCRETE) {
            dGfxIdx++;
        }

        if ((m_adapterType == mfxMediaAdapterType::MFX_MEDIA_UNKNOWN ||
             m_adapterType == it.second->Dev.MediaAdapterType) &&
            (m_adapterNum == -1 || m_adapterNum == GetAdapterNumber(it.second->Dev.DeviceID)) &&
            (m_dGfxIdx == -1 || m_dGfxIdx == dGfxIdx)) {
            m_idesc.reset(it.second, [this](mfxImplDescription* d) {
                MFXDispReleaseImplDescription(m_Loader, d);
            });

            m_ImplIndex = it.first;
            break;
        }
    }

    if (m_ImplIndex == (mfxU32)-1) {
        printf("Library was not found with required adapter type/num \n");
        sts = MFX_ERR_NOT_FOUND;
    }

    mfxStatus stsExt = MFXEnumImplementations(m_Loader,
                                              m_ImplIndex,
                                              MFX_IMPLCAPS_DEVICE_ID_EXTENDED,
                                              (mfxHDL*)&idescDevice);
#if !defined(_WIN32)
    if (stsExt == MFX_ERR_NONE && idescDevice) {
        m_DRMRenderNodeNumUsed = idescDevice->DRMRenderNodeNum;
        MFXDispReleaseImplDescription(m_Loader, idescDevice);
    }
#endif

    return sts;
}

mfxStatus VPLImplementationLoader::ConfigureAndEnumImplementations(
    mfxIMPL impl,
    mfxAccelerationMode accelerationMode,
    bool lowLatencyMode) {
    mfxStatus sts = ConfigureImplementation(impl);
    MSDK_CHECK_STATUS(sts, "ConfigureImplementation failed");
    sts = ConfigureAccelerationMode(accelerationMode, impl);
    MSDK_CHECK_STATUS(sts, "ConfigureAccelerationMode failed");

    if (m_Impl != MFX_IMPL_TYPE_HARDWARE ||
        m_adapterType != mfxMediaAdapterType::MFX_MEDIA_UNKNOWN || !lowLatencyMode
#if defined(_WIN32)
        || m_LUID > 0
#endif
        || (m_PCIDeviceSetup)) {
        sts = EnumImplementations();
        MSDK_CHECK_STATUS(sts, "EnumImplementations failed");
    }
    else {
        //Low-latency mode
        printf("CONFIGURE LOADER: Use dispatcher's low-latency mode\n");

        sts = CreateConfig("mfx-gen", "mfxImplDescription.ImplName");
        MSDK_CHECK_STATUS(sts, "Failed to configure mfxImplDescription.ImplName");

        sts = CreateConfig(mfxU32(0x8086), "mfxImplDescription.VendorID");
        MSDK_CHECK_STATUS(sts, "Failed to configure mfxImplDescription.VendorID");

#if !defined(_WIN32)
        // assume adapterNum 0 on Linux...
        if (m_adapterNum == -1)
            m_adapterNum = 0;
#else
        if (m_adapterNum == -1) {
            m_adapterNum = FoundSuitableAdapter();
            MSDK_CHECK_ERROR(m_adapterNum, -1, MFX_ERR_DEVICE_FAILED);
        }

        sts = CreateConfig(mfxU32(m_adapterNum), "DXGIAdapterIndex");
        MSDK_CHECK_STATUS(sts, "Failed to configure DXGIAdapterIndex");
#endif

        //only one impl. is reported
        m_ImplIndex = 0;
        //no 'mfxImplDescription' is available
        m_idesc.reset(new mfxImplDescription());
        m_idesc->ApiVersion       = m_MinVersion;
        m_idesc->Impl             = m_Impl;
        m_idesc->AccelerationMode = accelerationMode;
        snprintf(m_idesc->ImplName, sizeof(m_idesc->ImplName), "mfx-gen");
        m_idesc->VendorID = 0x8086;
        snprintf(m_idesc->Dev.DeviceID,
                 sizeof(m_idesc->Dev.DeviceID),
                 "%x/%d",
                 0 /* no ID */,
                 m_adapterNum);
    }

    return sts;
}

mfxLoader VPLImplementationLoader::GetLoader() const {
    return m_Loader;
}

mfxU32 VPLImplementationLoader::GetImplIndex() const {
    return m_ImplIndex;
}

mfxVersion VPLImplementationLoader::GetVersion() const {
    return m_idesc ? m_idesc->ApiVersion : mfxVersion({ { 0, 0 } });
}

std::string VPLImplementationLoader::GetImplName() const {
    if (m_idesc) {
        return std::string(m_idesc->ImplName);
    }
    else {
        return "";
    }
}

mfxU16 VPLImplementationLoader::GetImplType() const {
    return m_Impl;
}

std::pair<mfxI16, mfxI32> VPLImplementationLoader::GetDeviceIDAndAdapter() const {
    auto result = std::make_pair(-1, -1);
    if (!m_idesc)
        return result;

    std::string deviceAdapterInfo(m_idesc->Dev.DeviceID);
    std::regex pattern("([0-9a-fA-F]+)(?:/([0-9]|[1-9][0-9]+))?");
    std::smatch match;
    if (!std::regex_match(deviceAdapterInfo, match, pattern))
        return result;

    try {
        result.first = std::stoi(match[1].str(), 0, 16);
        if (match[2].matched)
            result.second = std::stoi(match[2].str());
    }
    catch (std::exception const&) {
        return result;
    }

    return result;
}

mfxU16 VPLImplementationLoader::GetAdapterType() const {
    return m_idesc ? m_idesc->Dev.MediaAdapterType : mfxMediaAdapterType::MFX_MEDIA_UNKNOWN;
}

void VPLImplementationLoader::SetMinVersion(mfxVersion const& version) {
    m_MinVersion = version;
}

mfxStatus MainVideoSession::CreateSession(VPLImplementationLoader* Loader) {
    return MFXCreateSession(Loader->GetLoader(), Loader->GetImplIndex(), &m_session);
}

mfxStatus MainVideoSession::PrintLibInfo(VPLImplementationLoader* Loader) {
    printf("Loaded Library configuration: \n");
    // Version
    mfxVersion version = {};
    mfxStatus sts      = MFXQueryVersion(m_session, &version);
    MSDK_CHECK_STATUS(sts, "Failed to query version");
    printf("    Version: %d.%d \n", version.Major, version.Minor);

    // Impl Name
    std::string implName = Loader->GetImplName();
    std::string strImplName;
    std::copy(std::begin(implName), std::end(implName), back_inserter(strImplName));
    printf("    ImplName: %s \n", strImplName.c_str());

    // Adapter Number
    printf("    Adapter number : %d \n", Loader->GetDeviceIDAndAdapter().second);

    // Media Adapter Type
    if (Loader->GetImplType() == MFX_IMPL_TYPE_SOFTWARE) {
        std::stringstream ss;
        ss << "    Adapter type: cpu" << std::endl;
        printf("%s", ss.str().c_str());
    }
    else {
        mfxPlatform platform = {};
        sts                  = MFXVideoCORE_QueryPlatform(m_session, &platform);
        MSDK_CHECK_STATUS(sts, "Failed to query platform");
        if (platform.MediaAdapterType != mfxMediaAdapterType::MFX_MEDIA_UNKNOWN) {
            std::stringstream ss;
            ss << "    Adapter type: "
               << (platform.MediaAdapterType == mfxMediaAdapterType::MFX_MEDIA_INTEGRATED
                       ? "integrated"
                       : "discrete");
            ss << std::endl;
            printf("%s", ss.str().c_str());
        }
    }

    // Module Name
#if (defined(LINUX32) || defined(LINUX64))
    printf("    DRMRenderNodeNum: %d \n", Loader->GetDRMRenderNodeNumUsed());
    printf("Used implementation number: %d \n", Loader->GetImplIndex());
    printf("Loaded modules:\n");
    int numLoad = 0;
    dl_iterate_phdr(PrintLibMFXPath, &numLoad);
#else
    #if !defined(MFX_DISPATCHER_LOG)
    PrintLoadedModules();
    #endif // !defined(MFX_DISPATCHER_LOG)
#endif //(defined(LINUX32) || defined(LINUX64))
    printf("\n");

    return sts;
}
