/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "sample_defs.h"

#pragma once
enum EPresetModes { PRESET_DEFAULT, PRESET_DSS, PRESET_CONF, PRESET_GAMING, PRESET_MAX_MODES };

enum EPresetCodecs { PRESET_AVC, PRESET_HEVC, PRESET_MAX_CODECS };

struct CPresetParameters {
    mfxU16 GopRefDist;

    mfxU16 TargetUsage;

    mfxU16 RateControlMethod;
    mfxU16 ExtBRCUsage;
    mfxU16 AsyncDepth;
    mfxU16 BRefType;
    mfxU16 EncTools;
    mfxU16 AdaptiveMaxFrameSize;
    mfxU16 LowDelayBRC;

    mfxU16 IntRefType;
    mfxU16 IntRefCycleSize;
    mfxU16 IntRefQPDelta;
    mfxU16 IntRefCycleDist;

    mfxU16 WeightedPred;
    mfxU16 WeightedBiPred;

    bool EnableBPyramid;
    bool EnablePPyramid;
    //    bool EnableLTR;
};

struct CDependentPresetParameters {
    mfxU16 TargetKbps;
    mfxU16 MaxKbps;
    mfxU16 GopPicSize;
    mfxU16 BufferSizeInKB;
    mfxU16 LookAheadDepth;
    mfxU32 MaxFrameSize;
};

struct COutputPresetParameters : public CPresetParameters, CDependentPresetParameters {
    std::string PresetName;

    void Clear() {
        memset(dynamic_cast<CPresetParameters*>(this), 0, sizeof(CPresetParameters));
        memset(dynamic_cast<CDependentPresetParameters*>(this),
               0,
               sizeof(CDependentPresetParameters));
    }

    COutputPresetParameters() {
        Clear();
    }

    COutputPresetParameters(CPresetParameters src) {
        Clear();
        *(CPresetParameters*)this = src;
    }
};

class CPresetManager {
public:
    ~CPresetManager();
    COutputPresetParameters GetPreset(EPresetModes mode,
                                      mfxU32 codecFourCC,
                                      mfxF64 fps,
                                      mfxU32 width,
                                      mfxU32 height,
                                      bool isHWLib);
    COutputPresetParameters GetBasicPreset(EPresetModes mode, mfxU32 codecFourCC);
    CDependentPresetParameters GetDependentPresetParameters(EPresetModes mode,
                                                            mfxU32 codecFourCC,
                                                            mfxF64 fps,
                                                            mfxU32 width,
                                                            mfxU32 height,
                                                            mfxU16 targetUsage);

    static CPresetManager Inst;
    static EPresetModes PresetNameToMode(const char* name);
    static EPresetModes PresetNameToMode(const std::string& name);

protected:
    CPresetManager();
    static CPresetParameters presets[PRESET_MAX_MODES][PRESET_MAX_CODECS];
    static const char* modesName[PRESET_MAX_MODES];
};

#define MODIFY_AND_PRINT_PARAM(paramName, presetName, shouldPrintPresetInfo) \
    if (!paramName) {                                                        \
        paramName = presetParams.presetName;                                 \
        if (shouldPrintPresetInfo) {                                         \
            printf(#presetName ": %d\n", (int)paramName);                    \
        }                                                                    \
    }                                                                        \
    else {                                                                   \
        if (shouldPrintPresetInfo) {                                         \
            printf(#presetName ": %d (original preset value: %d)\n",         \
                   (int)paramName,                                           \
                   (int)presetParams.presetName);                            \
        }                                                                    \
    }

#define MODIFY_AND_PRINT_PARAM_EXT(paramName, presetName, value, shouldPrintPresetInfo) \
    if (!paramName) {                                                                   \
        paramName = (value);                                                            \
        if (shouldPrintPresetInfo) {                                                    \
            printf(#presetName ": %d\n", (int)paramName);                               \
        }                                                                               \
    }                                                                                   \
    else {                                                                              \
        if (shouldPrintPresetInfo) {                                                    \
            printf(#presetName ": %d (original preset value: %d)\n",                    \
                   (int)paramName,                                                      \
                   (int)(value));                                                       \
        }                                                                               \
    }
