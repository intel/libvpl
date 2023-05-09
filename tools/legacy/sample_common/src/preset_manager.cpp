/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "preset_manager.h"
#include "brc_routines.h"
#include "vm/strings_defs.h"
#include "vpl/mfxvideo.h"

CPresetManager CPresetManager::Inst;

const char* CPresetManager::modesName[PRESET_MAX_MODES] = {
    "Default",
    "DSS",
    "Conference",
    "Gaming",
};

//GopRefDist, TargetUsage, RateControlMethod, ExtBRCType, AsyncDepth, BRefType, EncTools
// AdaptiveMaxFrameSize, LowDelayBRC, IntRefType, IntRefCycleSize, IntRefQPDelta, IntRefCycleDist, WeightedPred, WeightedBiPred, EnableBPyramid, EnablePPyramid

CPresetParameters CPresetManager::presets[PRESET_MAX_MODES][PRESET_MAX_CODECS] = {
    // Default
    { { 4,
        MFX_TARGETUSAGE_BALANCED,
        MFX_RATECONTROL_VBR,
        EXTBRC_DEFAULT,
        4,
        MFX_B_REF_PYRAMID,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        MFX_WEIGHTED_PRED_UNKNOWN,
        MFX_WEIGHTED_PRED_UNKNOWN,
        0,
        0 },
      { 0,
        MFX_TARGETUSAGE_BALANCED,
        MFX_RATECONTROL_VBR,
        EXTBRC_DEFAULT,
        4,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        MFX_WEIGHTED_PRED_UNKNOWN,
        MFX_WEIGHTED_PRED_UNKNOWN,
        0,
        0 } },
    // DSS
    {
        { 1,
          MFX_TARGETUSAGE_BALANCED,
          MFX_RATECONTROL_QVBR,
          EXTBRC_DEFAULT,
          1,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          MFX_WEIGHTED_PRED_UNKNOWN,
          MFX_WEIGHTED_PRED_UNKNOWN,
          0,
          0 },
        { 1,
          MFX_TARGETUSAGE_BALANCED,
          MFX_RATECONTROL_QVBR,
          EXTBRC_DEFAULT,
          1,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          MFX_WEIGHTED_PRED_UNKNOWN,
          MFX_WEIGHTED_PRED_UNKNOWN,
          1,
          1 },
    },
    // Conference
    {
        { 1,
          MFX_TARGETUSAGE_BALANCED,
          MFX_RATECONTROL_VCM,
          EXTBRC_DEFAULT,
          1,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          MFX_WEIGHTED_PRED_UNKNOWN,
          MFX_WEIGHTED_PRED_UNKNOWN,
          0,
          0 },
        { 1,
          MFX_TARGETUSAGE_BALANCED,
          MFX_RATECONTROL_VBR,
          EXTBRC_ON,
          1,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          MFX_WEIGHTED_PRED_UNKNOWN,
          MFX_WEIGHTED_PRED_UNKNOWN,
          0,
          0 },
    },
    // Gaming
    {
        { 1,
          MFX_TARGETUSAGE_BALANCED,
          MFX_RATECONTROL_QVBR,
          EXTBRC_DEFAULT,
          1,
          0,
          0,
          MFX_CODINGOPTION_ON,
          MFX_CODINGOPTION_ON,
          MFX_REFRESH_HORIZONTAL,
          8,
          0,
          4,
          MFX_WEIGHTED_PRED_UNKNOWN,
          MFX_WEIGHTED_PRED_UNKNOWN,
          0,
          0 },
        { 1,
          MFX_TARGETUSAGE_BALANCED,
          MFX_RATECONTROL_VBR,
          EXTBRC_ON,
          1,
          0,
          0,
          MFX_CODINGOPTION_ON,
          MFX_CODINGOPTION_ON,
          MFX_REFRESH_HORIZONTAL,
          8,
          0,
          4,
          MFX_WEIGHTED_PRED_UNKNOWN,
          MFX_WEIGHTED_PRED_UNKNOWN,
          0,
          0 },
    }
};

CPresetManager::CPresetManager() {}

CPresetManager::~CPresetManager() {}

COutputPresetParameters CPresetManager::GetPreset(EPresetModes mode,
                                                  mfxU32 codecFourCC,
                                                  mfxF64 fps,
                                                  mfxU32 width,
                                                  mfxU32 height,
                                                  bool isHWLib) {
    COutputPresetParameters retVal = GetBasicPreset(mode, codecFourCC);
    *(dynamic_cast<CDependentPresetParameters*>(&retVal)) =
        GetDependentPresetParameters(mode, codecFourCC, fps, width, height, retVal.TargetUsage);

    if (!isHWLib) {
        // These features are unsupported in SW library
        retVal.WeightedBiPred = 0;
        retVal.WeightedPred   = 0;
    }

    return retVal;
}

COutputPresetParameters CPresetManager::GetBasicPreset(EPresetModes mode, mfxU32 codecFourCC) {
    COutputPresetParameters retVal;

    if (mode < 0 || mode >= PRESET_MAX_MODES) {
        mode = PRESET_DEFAULT;
    }

    // Reading basic preset values
    switch (codecFourCC) {
        case MFX_CODEC_AVC:
            retVal = presets[mode][PRESET_AVC];
            break;
        case MFX_CODEC_HEVC:
            retVal = presets[mode][PRESET_HEVC];
            break;
        default:
            if (mode != PRESET_DEFAULT) {
                printf(
                    "WARNING: Presets are available for h.264 or h.265 codecs only. Request for particular preset is ignored.\n");
            }

            if (codecFourCC != MFX_CODEC_JPEG) {
                retVal.TargetUsage       = MFX_TARGETUSAGE_BALANCED;
                retVal.RateControlMethod = MFX_RATECONTROL_CBR;
            }
            retVal.AsyncDepth = 4;
            return retVal;
    }

    retVal.PresetName = modesName[mode];
    return retVal;
}

CDependentPresetParameters CPresetManager::GetDependentPresetParameters(EPresetModes mode,
                                                                        mfxU32 codecFourCC,
                                                                        mfxF64 fps,
                                                                        mfxU32 width,
                                                                        mfxU32 height,
                                                                        mfxU16 targetUsage) {
    CDependentPresetParameters retVal = {};
    retVal.TargetKbps                 = codecFourCC != MFX_CODEC_JPEG
                                            ? CalculateDefaultBitrate(codecFourCC, targetUsage, width, height, fps)
                                            : 0;

    if (codecFourCC == MFX_CODEC_AVC || codecFourCC == MFX_CODEC_HEVC) {
        // Calculating dependent preset values
        retVal.MaxKbps = (mode == PRESET_GAMING ? (mfxU16)(1.2 * retVal.TargetKbps) : 0);
        retVal.GopPicSize =
            (mode == PRESET_GAMING || mode == PRESET_DEFAULT ? 0 : (mfxU16)(2 * fps));
        retVal.BufferSizeInKB =
            (mode == PRESET_DEFAULT ? 0 : retVal.TargetKbps); // 1 second buffers
        retVal.LookAheadDepth = 0; // Enable this setting if LA BRC will be enabled
        retVal.MaxFrameSize   = (mode == PRESET_GAMING ? (mfxU32)(retVal.TargetKbps * 0.166) : 0);
    }
    return retVal;
}

EPresetModes CPresetManager::PresetNameToMode(const char* name) {
    for (int i = 0; i < PRESET_MAX_MODES; i++) {
        if (msdk_match_i(modesName[i], name)) {
            return (EPresetModes)i;
        }
    }
    return PRESET_MAX_MODES;
}

EPresetModes CPresetManager::PresetNameToMode(const std::string& name) {
    for (int i = 0; i < PRESET_MAX_MODES; i++) {
        if (msdk_match_i(modesName[i], name)) {
            return (EPresetModes)i;
        }
    }
    return PRESET_MAX_MODES;
}
