/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __PLUGIN_UTILS_H__
#define __PLUGIN_UTILS_H__

#include <string>
#include "sample_defs.h"

#if defined(_WIN32) || defined(_WIN64)
    #define MSDK_CPU_ROTATE_PLUGIN "sample_rotate_plugin.dll"
    #define MSDK_OCL_ROTATE_PLUGIN "sample_plugin_opencl.dll"
#else
    #define MSDK_CPU_ROTATE_PLUGIN "libsample_rotate_plugin.so"
    #define MSDK_OCL_ROTATE_PLUGIN "libsample_plugin_opencl.so"
#endif

typedef mfxI32 msdkComponentType;
enum {
    MSDK_VDECODE = 0x0001,
    MSDK_VENCODE = 0x0002,
    MSDK_VPP     = 0x0004,
    MSDK_VENC    = 0x0008,
    MSDK_FEI     = 0x1000,
};

typedef enum { MFX_PLUGINLOAD_TYPE_GUID = 1, MFX_PLUGINLOAD_TYPE_FILE = 2 } MfxPluginLoadType;

struct sPluginParams {
    mfxPluginUID pluginGuid;
    std::string strPluginPath;
    MfxPluginLoadType type;
    sPluginParams() : pluginGuid{ 0 }, strPluginPath(), type(MfxPluginLoadType(0)) {}
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

const mfxPluginUID& msdkGetPluginUID(mfxIMPL impl, msdkComponentType type, mfxU32 uCodecid);

sPluginParams ParsePluginGuid(const std::string&);
sPluginParams ParsePluginPath(const std::string&);
mfxStatus ConvertStringToGuid(const std::string& strGuid, mfxPluginUID& mfxGuid);

#endif //__PLUGIN_UTILS_H__
