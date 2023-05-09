/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#ifndef __PLUGIN_LOADER_H__
    #define __PLUGIN_LOADER_H__

    #include "plugin_utils.h"
    #include "sample_utils.h"
    #include "vm/so_defs.h"

    #include <iomanip> // for std::setfill, std::setw
    #include <iostream>
    #include <memory> // for std::unique_ptr

class MsdkSoModule {
protected:
    msdk_so_handle m_module;

public:
    MsdkSoModule() : m_module(NULL) {}
    MsdkSoModule(const std::string& pluginName) : m_module(NULL) {
        m_module = msdk_so_load(pluginName.c_str());
        if (NULL == m_module) {
            MSDK_TRACE_ERROR(std::string("Failed to load shared module: ") + pluginName);
        }
    }
    template <class T>
    T GetAddr(const std::string& fncName) {
        T pCreateFunc = reinterpret_cast<T>(msdk_so_get_addr(m_module, fncName.c_str()));
        if (NULL == pCreateFunc) {
            MSDK_TRACE_ERROR(std::string("Failed to get function addres: ") + fncName.c_str());
        }
        return pCreateFunc;
    }

    virtual ~MsdkSoModule() {
        if (m_module) {
            msdk_so_free(m_module);
            m_module = NULL;
        }
    }
};

/*
* Rationale: class to load+register any mediasdk plugin decoder/encoder/generic by given name
*/
class PluginLoader : public MFXPlugin {
protected:
    mfxPluginType ePluginType;

    mfxSession m_session;
    mfxPluginUID m_uid;

private:
    const char* msdkGetPluginName(const mfxPluginUID& guid) {
        if (AreGuidsEqual(guid, MFX_PLUGINID_HEVCD_SW))
            return "Intel (R) Media SDK plugin for HEVC DECODE";
        else if (AreGuidsEqual(guid, MFX_PLUGINID_HEVCD_HW))
            return "Intel (R) Media SDK HW plugin for HEVC DECODE";
        else if (AreGuidsEqual(guid, MFX_PLUGINID_HEVCE_SW))
            return "Intel (R) Media SDK plugin for HEVC ENCODE";
        else if (AreGuidsEqual(guid, MFX_PLUGINID_HEVCE_HW))
            return "Intel (R) Media SDK HW plugin for HEVC ENCODE";
        else if (AreGuidsEqual(guid, MFX_PLUGINID_H264LA_HW))
            return "Intel (R) Media SDK plugin for LA ENC";
        else if (AreGuidsEqual(guid, MFX_PLUGINID_ITELECINE_HW))
            return "Intel (R) Media SDK PTIR plugin (HW)";
        else if (AreGuidsEqual(guid, MFX_PLUGINID_HEVCE_GACC))
            return "Intel (R) Media SDK GPU-Accelerated plugin for HEVC ENCODE";
        else if (AreGuidsEqual(guid, MFX_PLUGINID_VP9D_HW))
            return "Intel (R) Media SDK HW plugin for VP9 DECODE";
        else if (AreGuidsEqual(guid, MFX_PLUGINID_VP9E_HW))
            return "Intel (R) Media SDK HW plugin for VP9 ENCODE";
        else
    #if !defined(_WIN32) && !defined(_WIN64)
            if (AreGuidsEqual(guid, MFX_PLUGINID_HEVC_FEI_ENCODE))
            return "Intel (R) Media SDK HW plugin for HEVC FEI ENCODE";
        else
    #endif
            return "Unknown plugin";
    }

public:
    PluginLoader(mfxPluginType type,
                 mfxSession session,
                 const mfxPluginUID& uid,
                 mfxU32 version,
                 const mfxChar* pluginName,
                 mfxU32 len)
            : ePluginType(type),
              m_session(),
              m_uid() {
        mfxStatus sts = MFX_ERR_NONE;
        std::stringstream strStream;

        MSDK_MEMCPY(&m_uid, sizeof(m_uid), &uid, sizeof(uid));
        for (size_t i = 0; i != sizeof(mfxPluginUID); i++) {
            strStream << "0x" << std::setfill('0') << std::setw(2) << std::hex
                      << (int)m_uid.Data[i];
            if (i != (sizeof(mfxPluginUID) - 1))
                strStream << ", ";
        }

        if ((ePluginType == MFX_PLUGINTYPE_AUDIO_DECODE) ||
            (ePluginType == MFX_PLUGINTYPE_AUDIO_ENCODE)) {
            // Audio plugins are not loaded by path
            sts = MFX_ERR_UNSUPPORTED;
        }
        else {
            sts = MFXVideoUSER_LoadByPath(session, &m_uid, version, pluginName, len);
        }

        if (MFX_ERR_NONE != sts) {
            MSDK_TRACE_ERROR("Failed to load plugin from GUID, sts="
                             << sts << ": { " << strStream.str() << " } ("
                             << msdkGetPluginName(m_uid) << ")");
        }
        else {
            MSDK_TRACE_INFO("Plugin was loaded from GUID");
            m_session = session;
        }
    }

    PluginLoader(mfxPluginType type, mfxSession session, const mfxPluginUID& uid, mfxU32 version)
            : ePluginType(type),
              m_session(),
              m_uid() {
        mfxStatus sts = MFX_ERR_NONE;
        std::stringstream strStream;

        MSDK_MEMCPY(&m_uid, sizeof(m_uid), &uid, sizeof(uid));
        for (size_t i = 0; i != sizeof(mfxPluginUID); i++) {
            strStream << "0x" << std::setfill('0') << std::setw(2) << std::hex
                      << (int)m_uid.Data[i];
            if (i != (sizeof(mfxPluginUID) - 1))
                strStream << ", ";
        }

        if ((ePluginType == MFX_PLUGINTYPE_AUDIO_DECODE) ||
            (ePluginType == MFX_PLUGINTYPE_AUDIO_ENCODE)) {
            sts = MFXAudioUSER_Load(session, &m_uid, version);
        }
        else {
            sts = MFXVideoUSER_Load(session, &m_uid, version);
        }

        if (MFX_ERR_NONE != sts) {
            MSDK_TRACE_ERROR("Failed to load plugin from GUID, sts="
                             << sts << ": { " << strStream.str() << " } ("
                             << msdkGetPluginName(m_uid) << ")");
        }
        else {
            MSDK_TRACE_INFO("Plugin was loaded from GUID"
                            << ": { " << strStream.str() << " } (" << msdkGetPluginName(m_uid)
                            << ")");
            m_session = session;
        }
    }

    virtual ~PluginLoader() {
        mfxStatus sts = MFX_ERR_NONE;
        if (m_session) {
            if ((ePluginType == MFX_PLUGINTYPE_AUDIO_DECODE) ||
                (ePluginType == MFX_PLUGINTYPE_AUDIO_ENCODE)) {
                sts = MFXAudioUSER_UnLoad(m_session, &m_uid);
            }
            else {
                sts = MFXVideoUSER_UnLoad(m_session, &m_uid);
            }

            if (sts != MFX_ERR_NONE) {
                MSDK_TRACE_ERROR("Failed to unload plugin from GUID, sts=" << sts);
            }
            else {
                MSDK_TRACE_INFO("MFXBaseUSER_UnLoad(session=0x" << m_session << "), sts=" << sts);
            }
        }
    }

    bool IsOk() {
        return m_session != 0;
    }
    virtual mfxStatus PluginInit(mfxCoreInterface* /*core*/) {
        return MFX_ERR_NULL_PTR;
    }
    virtual mfxStatus PluginClose() {
        return MFX_ERR_NULL_PTR;
    }
    virtual mfxStatus GetPluginParam(mfxPluginParam* /*par*/) {
        return MFX_ERR_NULL_PTR;
    }
    virtual mfxStatus Execute(mfxThreadTask /*task*/, mfxU32 /*uid_p*/, mfxU32 /*uid_a*/) {
        return MFX_ERR_NULL_PTR;
    }
    virtual mfxStatus FreeResources(mfxThreadTask /*task*/, mfxStatus /*sts*/) {
        return MFX_ERR_NULL_PTR;
    }
    virtual void Release() {}
    virtual mfxStatus Close() {
        return MFX_ERR_NULL_PTR;
    }
    virtual mfxStatus SetAuxParams(void* /*auxParam*/, int /*auxParamSize*/) {
        return MFX_ERR_NULL_PTR;
    }
};

#endif // PLUGIN_LOADER