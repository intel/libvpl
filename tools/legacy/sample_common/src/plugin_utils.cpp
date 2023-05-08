/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "mfx_samples_config.h"

#include <map>
#include <sstream>
#include "plugin_utils.h"
#include "vpl/mfxvp8.h"

bool AreGuidsEqual(const mfxPluginUID& guid1, const mfxPluginUID& guid2) {
    for (size_t i = 0; i != sizeof(mfxPluginUID); i++) {
        if (guid1.Data[i] != guid2.Data[i])
            return false;
    }
    return true;
}

mfxStatus ConvertStringToGuid(const std::string& strGuid, mfxPluginUID& mfxGuid) {
    mfxStatus sts = MFX_ERR_NONE;

    // Check if symbolic GUID value
    std::map<std::string, mfxPluginUID> uid{ { "hevcd_sw", MFX_PLUGINID_HEVCD_SW },
                                             { "hevcd_hw", MFX_PLUGINID_HEVCD_HW },
                                             { "hevce_sw", MFX_PLUGINID_HEVCE_SW },
                                             { "hevce_gacc", MFX_PLUGINID_HEVCE_GACC },
                                             { "hevce_hw", MFX_PLUGINID_HEVCE_HW },
                                             { "vp8d_hw", MFX_PLUGINID_VP8D_HW },
                                             { "vp8e_hw", MFX_PLUGINID_VP8E_HW },
                                             { "vp9d_hw", MFX_PLUGINID_VP9D_HW },
                                             { "vp9e_hw", MFX_PLUGINID_VP9E_HW },
                                             { "camera_hw", MFX_PLUGINID_CAMERA_HW },
                                             { "capture_hw", MFX_PLUGINID_CAPTURE_HW },
                                             { "ptir_hw", MFX_PLUGINID_ITELECINE_HW },
                                             { "h264_la_hw", MFX_PLUGINID_H264LA_HW },
                                             { "aacd", MFX_PLUGINID_AACD },
                                             { "aace", MFX_PLUGINID_AACE },
                                             { "hevce_fei_hw", MFX_PLUGINID_HEVCE_FEI_HW } };
    auto item = uid.find(strGuid);
    if (item == uid.end()) {
        mfxGuid = MSDK_PLUGINGUID_NULL;
        sts     = MFX_ERR_UNKNOWN;
    }
    else {
        mfxGuid = uid[strGuid];
        sts     = MFX_ERR_NONE;
    }

    // Check if plain GUID value
    if (sts) {
        if (strGuid.size() != 32) {
            sts = MFX_ERR_UNKNOWN;
        }
        else {
            for (size_t i = 0; i < 16; i++) {
                unsigned int xx = 0;
                std::stringstream ss;
                ss << std::hex << strGuid.substr(i * 2, 2);
                ss >> xx;
                mfxGuid.Data[i] = (mfxU8)xx;
            }
            sts = MFX_ERR_NONE;
        }
    }
    return sts;
}

const mfxPluginUID& msdkGetPluginUID(mfxIMPL impl, msdkComponentType type, mfxU32 uCodecid) {
    if (impl == MFX_IMPL_SOFTWARE) {
        switch (type) {
            case MSDK_VDECODE:
                switch (uCodecid) {
                    case MFX_CODEC_HEVC:
                        return MFX_PLUGINID_HEVCD_SW;
                }
                break;
            case MSDK_VENCODE:
                switch (uCodecid) {
                    case MFX_CODEC_HEVC:
                        return MFX_PLUGINID_HEVCE_SW;
                }
                break;
        }
    }
    else {
        switch (type) {
// On Android implementaion of all decoders is placed in libmfx
// That's why we don't need default plugins for these codecs
#if !defined(ANDROID)
            case MSDK_VDECODE:
                switch (uCodecid) {
                    case MFX_CODEC_HEVC:
                        return MFX_PLUGINID_HEVCD_HW;
                    case MFX_CODEC_VP8:
                        return MFX_PLUGINID_VP8D_HW;
                    case MFX_CODEC_VP9:
                        return MFX_PLUGINID_VP9D_HW;
                }
                break;
#endif
            case MSDK_VENCODE:
                switch (uCodecid) {
                    case MFX_CODEC_HEVC:
                        return MFX_PLUGINID_HEVCE_HW;
                    case MFX_CODEC_VP8:
                        return MFX_PLUGINID_VP8E_HW;
                }
                break;
#if !defined(_WIN32) && !defined(_WIN64)
            case (MSDK_VENCODE | MSDK_FEI):
                switch (uCodecid) {
                    case MFX_CODEC_HEVC:
                        return MFX_PLUGINID_HEVC_FEI_ENCODE;
                }
                break;
#endif
            case MSDK_VENC:
                switch (uCodecid) {
                    case MFX_CODEC_HEVC:
                        return MFX_PLUGINID_HEVCE_FEI_HW; // HEVC FEI uses ENC interface
                }
                break;
        }
    }

    return MSDK_PLUGINGUID_NULL;
}

sPluginParams ParsePluginGuid(const std::string& strPluginGuid) {
    sPluginParams pluginParams;
    mfxPluginUID uid;
    mfxStatus sts = ConvertStringToGuid(strPluginGuid, uid);

    if (sts == MFX_ERR_NONE) {
        pluginParams.type       = MFX_PLUGINLOAD_TYPE_GUID;
        pluginParams.pluginGuid = uid;
    }

    return pluginParams;
}

sPluginParams ParsePluginPath(const std::string& strPluginGuid) {
    sPluginParams pluginParams;
    pluginParams.strPluginPath = std::string(strPluginGuid.begin(), strPluginGuid.end());
    pluginParams.type          = MFX_PLUGINLOAD_TYPE_FILE;
    return pluginParams;
}
