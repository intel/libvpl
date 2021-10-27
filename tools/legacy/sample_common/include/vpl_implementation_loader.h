/*############################################################################
  # Copyright (C) 2021 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __VPL_IMPLEMENTATION_LOADER_H__
#define __VPL_IMPLEMENTATION_LOADER_H__

#include <string>
#include <vector>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo++.h"

class VPLImplementationLoader {
    mfxLoader m_Loader;
    std::vector<mfxConfig> m_Configs;
    mfxImplDescription* m_idesc;
    mfxU32 m_ImplIndex;
    mfxU16 m_adapterType;
    mfxI32 m_dGfxIdx;
    mfxI32 m_adapterNum;
    mfxU32 m_MinVersion;

    VPLImplementationLoader(const VPLImplementationLoader&) {}
    VPLImplementationLoader& operator=(const VPLImplementationLoader&) {
        return *this;
    }

public:
    VPLImplementationLoader();
    ~VPLImplementationLoader();

    mfxStatus CreateConfig(mfxU16 data, const char* propertyName);
    mfxStatus CreateConfig(mfxU32 data, const char* propertyName);
    mfxStatus ConfigureImplementation(mfxIMPL impl);
    mfxStatus ConfigureAccelerationMode(mfxAccelerationMode accelerationMode, mfxIMPL impl);
    mfxStatus ConfigureVersion(mfxVersion const version);
    void SetAdapterType(mfxU16 adapterType);
    void SetDiscreteAdapterIndex(mfxI32 dGfxIdx);
    void SetAdapterNum(mfxI32 adapterNum);
    mfxStatus EnumImplementations();
    mfxStatus ConfigureAndEnumImplementations(mfxIMPL impl, mfxAccelerationMode accelerationMode);
    mfxLoader GetLoader() const;
    mfxU32 GetImplIndex() const;
    std::string GetImplName() const;
    mfxVersion GetVersion() const;
    std::pair<mfxI16, mfxI32> GetDeviceIDAndAdapter() const;
    mfxU16 GetAdapterType() const;
    void SetMinVersion(mfxVersion const version);
};

class MainVideoSession : public MFXVideoSession {
public:
    mfxStatus CreateSession(VPLImplementationLoader* Loader);
};

#endif //__VPL_IMPLEMENTATION_LOADER_H__
