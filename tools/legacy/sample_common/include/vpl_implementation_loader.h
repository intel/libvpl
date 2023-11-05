/*############################################################################
  # Copyright (C) 2021 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __VPL_IMPLEMENTATION_LOADER_H__
#define __VPL_IMPLEMENTATION_LOADER_H__

#include <memory>
#include <string>
#include <vector>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo++.h"

#if defined(_WIN32)
    #include <dxgi.h>
#endif

// default behavior for dispatcher low-latency mode
// override via cmd-line switches -dispatcher:fullSearch or -dispatcher:lowLatency
#if defined(_WIN32)
    // Windows: enable dispatcher low-latency path
    #define DEF_DISP_FULLSEARCH false
#else
    // Linux: enable dispatcher full search
    #define DEF_DISP_FULLSEARCH true
#endif

class VPLImplementationLoader {
    std::shared_ptr<_mfxLoader> m_loader;
    std::shared_ptr<mfxImplDescription> m_idesc;

    mfxLoader m_Loader;
    mfxU32 m_ImplIndex;
    mfxImplType m_Impl;
    mfxU16 m_adapterType;
    mfxI32 m_dGfxIdx;
    mfxI32 m_adapterNum;
    mfxVersion m_MinVersion;
    // Extended device ID info, available in 2.6 and newer APIs
    mfxU32 m_PCIDomain;
    mfxU32 m_PCIBus;
    mfxU32 m_PCIDevice;
    mfxU32 m_PCIFunction;
    bool m_PCIDeviceSetup;
#if defined(_WIN32)
    mfxU64 m_LUID;
#else
    mfxU32 m_DRMRenderNodeNum;
    mfxU32 m_DRMRenderNodeNumUsed;
#endif

public:
    VPLImplementationLoader();
    ~VPLImplementationLoader();

    mfxStatus CreateConfig(char const* data, const char* propertyName);
    mfxStatus CreateConfig(mfxU16 data, const char* propertyName);
    mfxStatus CreateConfig(mfxU32 data, const char* propertyName);
    mfxStatus ConfigureImplementation(mfxIMPL impl);
    mfxStatus ConfigureAccelerationMode(mfxAccelerationMode accelerationMode, mfxIMPL impl);
    mfxStatus ConfigureVersion(mfxVersion const version);
    void SetAdapterType(mfxU16 adapterType);
    void SetDiscreteAdapterIndex(mfxI32 dGfxIdx);
    void SetAdapterNum(mfxI32 adapterNum);
    mfxStatus EnumImplementations();
    mfxStatus ConfigureAndEnumImplementations(mfxIMPL impl,
                                              mfxAccelerationMode accelerationMode,
                                              bool lowLatencyMode = true);
    mfxLoader GetLoader() const;
    mfxU32 GetImplIndex() const;
    std::string GetImplName() const;
    mfxU16 GetImplType() const;
    mfxVersion GetVersion() const;
    std::pair<mfxI16, mfxI32> GetDeviceIDAndAdapter() const;
    mfxU16 GetAdapterType() const;
    void SetMinVersion(mfxVersion const& version);
    mfxStatus SetPCIDevice(mfxI32 domain, mfxI32 bus, mfxI32 device, mfxI32 function);
#if defined(_WIN32)
    mfxStatus SetupLUID(LUID luid);
#else
    mfxStatus SetupDRMRenderNodeNum(mfxU32 DRMRenderNodeNum);
    mfxU32 GetDRMRenderNodeNumUsed();
#endif
};

class MainVideoSession : public MFXVideoSession {
public:
    mfxStatus CreateSession(VPLImplementationLoader* Loader);
    mfxStatus PrintLibInfo(VPLImplementationLoader* Loader);
};

#endif //__VPL_IMPLEMENTATION_LOADER_H__
