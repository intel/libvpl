/*###########################################################################
  # Copyright (C) 2005-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#ifndef __PLUGIN_UTILS_H__
#define __PLUGIN_UTILS_H__

#include "sample_defs.h"
#include "sample_types.h"

#if defined(_WIN32) || defined(_WIN64)
    #define MSDK_CPU_ROTATE_PLUGIN MSDK_STRING("sample_rotate_plugin.dll")
    #define MSDK_OCL_ROTATE_PLUGIN MSDK_STRING("sample_plugin_opencl.dll")
#else
    #define MSDK_CPU_ROTATE_PLUGIN MSDK_STRING("libsample_rotate_plugin.so")
    #define MSDK_OCL_ROTATE_PLUGIN MSDK_STRING("libsample_plugin_opencl.so")
#endif

typedef mfxI32 msdkComponentType;
enum {
    MSDK_VDECODE = 0x0001,
    MSDK_VENCODE = 0x0002,
    MSDK_VPP     = 0x0004,
    MSDK_VENC    = 0x0008,
#if (MFX_VERSION >= 1027)
    MSDK_FEI = 0x1000,
#endif
};

typedef enum {
    MFX_PLUGINLOAD_TYPE_GUID = 1,
    MFX_PLUGINLOAD_TYPE_FILE = 2
} MfxPluginLoadType;

struct sPluginParams {
    mfxPluginUID pluginGuid;
    mfxChar strPluginPath[MSDK_MAX_FILENAME_LEN];
    MfxPluginLoadType type;
    sPluginParams() {
        MSDK_ZERO_MEMORY(*this);
    }
};

static const mfxPluginUID MSDK_PLUGINGUID_NULL = { { 0x00,
                                                     0x00,
                                                     0x00,
                                                     0x00,
                                                     0x00,
                                                     0x00,
                                                     0x00,
                                                     0x00,
                                                     0x00,
                                                     0x00,
                                                     0x00,
                                                     0x00,
                                                     0x00,
                                                     0x00,
                                                     0x00,
                                                     0x00 } };

bool AreGuidsEqual(const mfxPluginUID& guid1, const mfxPluginUID& guid2);

const mfxPluginUID& msdkGetPluginUID(mfxIMPL impl,
                                     msdkComponentType type,
                                     mfxU32 uCodecid);

sPluginParams ParsePluginGuid(msdk_char*);
sPluginParams ParsePluginPath(msdk_char*);
mfxStatus ConvertStringToGuid(const msdk_string& strGuid,
                              mfxPluginUID& mfxGuid);

#endif //__PLUGIN_UTILS_H__
