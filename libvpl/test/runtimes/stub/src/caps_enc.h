/*############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################*/

//NOLINT(build/header_guard)

#include "src/caps.h"

// leave table formatting alone
// clang-format off

const mfxU32 encColorFmt_c00_p00_m00[] = {
    MFX_FOURCC_I420,
    MFX_FOURCC_I010,
};

#ifdef ONEVPL_EXPERIMENTAL
const mfxU16 encTargetChromaSubsampling_c00_p00_m00[] = {
    MFX_CHROMAFORMAT_YUV420,
};

const mfxEncMemExtDescription encMemExtDesc_c00_p00_m00 = {
    { 0, 1 },
    {},
    10,
    1,
    (mfxU16 *)encTargetChromaSubsampling_c00_p00_m00,
};
#endif

const EncMemDesc encMemDesc_c00_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        (mfxEncMemExtDescription *)&encMemExtDesc_c00_p00_m00,
        0,
#else
        {},
#endif
        2,
        (mfxU32 *)encColorFmt_c00_p00_m00,
    },
};

const EncProfile encProfile_c00[] = {
    {
        MFX_PROFILE_AV1_HIGH,
        {},
        1,
        (EncMemDesc *)encMemDesc_c00_p00,
    },
};

#ifdef ONEVPL_EXPERIMENTAL
const mfxU16 encRateControlMethods_c00[] = {
    MFX_RATECONTROL_CBR,
    MFX_RATECONTROL_VBR,
    MFX_RATECONTROL_CQP,
};

const mfxU32 encExtBufferIDs_c00[] = {
    MFX_EXTBUFF_CODING_OPTION,
};

const mfxEncExtDescription encExtDesc_c00 = {
    { 0, 1 },
    {},
    3,
    (mfxU16 *)encRateControlMethods_c00,
    {},
    1,
    (mfxU32 *)encExtBufferIDs_c00,
};
#endif

const mfxU32 encColorFmt_c01_p00_m00[] = {
    MFX_FOURCC_I420,
};

#ifdef ONEVPL_EXPERIMENTAL
const mfxU16 encTargetChromaSubsampling_c01_p00_m00[] = {
    MFX_CHROMAFORMAT_YUV420,
};

const mfxEncMemExtDescription encMemExtDesc_c01_p00_m00 = {
    { 0, 1 },
    {},
    8,
    1,
    (mfxU16 *)encTargetChromaSubsampling_c01_p00_m00,
};
#endif

const EncMemDesc encMemDesc_c01_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        (mfxEncMemExtDescription *)&encMemExtDesc_c01_p00_m00,
        0,
#else
        {},
#endif
        1,
        (mfxU32 *)encColorFmt_c01_p00_m00,
    },
};

const mfxU32 encColorFmt_c01_p01_m00[] = {
    MFX_FOURCC_I010,
};

#ifdef ONEVPL_EXPERIMENTAL
const mfxU16 encTargetChromaSubsampling_c01_p01_m00[] = {
    MFX_CHROMAFORMAT_YUV420,
};

const mfxEncMemExtDescription encMemExtDesc_c01_p01_m00 = {
    { 0, 1 },
    {},
    10,
    1,
    (mfxU16 *)encTargetChromaSubsampling_c01_p01_m00,
};
#endif

const EncMemDesc encMemDesc_c01_p01[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        (mfxEncMemExtDescription *)&encMemExtDesc_c01_p01_m00,
        0,
#else
        {},
#endif
        1,
        (mfxU32 *)encColorFmt_c01_p01_m00,
    },
};

const EncProfile encProfile_c01[] = {
    {
        MFX_PROFILE_AVC_BASELINE,
        {},
        1,
        (EncMemDesc *)encMemDesc_c01_p00,
    },
    {
        MFX_PROFILE_AVC_MAIN,
        {},
        1,
        (EncMemDesc *)encMemDesc_c01_p01,
    },
};

#ifdef ONEVPL_EXPERIMENTAL
const mfxU16 encRateControlMethods_c01[] = {
    MFX_RATECONTROL_CBR,
    MFX_RATECONTROL_VBR,
    MFX_RATECONTROL_CQP,
    MFX_RATECONTROL_AVBR,
};

const mfxU32 encExtBufferIDs_c01[] = {
    MFX_EXTBUFF_CODING_OPTION,
};

const mfxEncExtDescription encExtDesc_c01 = {
    { 0, 1 },
    {},
    4,
    (mfxU16 *)encRateControlMethods_c01,
    {},
    1,
    (mfxU32 *)encExtBufferIDs_c01,
};
#endif

const mfxU32 encColorFmt_c02_p00_m00[] = {
    MFX_FOURCC_I420,
};

#ifdef ONEVPL_EXPERIMENTAL
const mfxU16 encTargetChromaSubsampling_c02_p00_m00[] = {
    MFX_CHROMAFORMAT_YUV420,
};

const mfxEncMemExtDescription encMemExtDesc_c02_p00_m00 = {
    { 0, 1 },
    {},
    8,
    1,
    (mfxU16 *)encTargetChromaSubsampling_c02_p00_m00,
};
#endif

const EncMemDesc encMemDesc_c02_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        (mfxEncMemExtDescription *)&encMemExtDesc_c02_p00_m00,
        0,
#else
        {},
#endif
        1,
        (mfxU32 *)encColorFmt_c02_p00_m00,
    },
};

const mfxU32 encColorFmt_c02_p01_m00[] = {
    MFX_FOURCC_I010,
};

#ifdef ONEVPL_EXPERIMENTAL
const mfxU16 encTargetChromaSubsampling_c02_p01_m00[] = {
    MFX_CHROMAFORMAT_YUV420,
};

const mfxEncMemExtDescription encMemExtDesc_c02_p01_m00 = {
    { 0, 1 },
    {},
    10,
    1,
    (mfxU16 *)encTargetChromaSubsampling_c02_p01_m00,
};
#endif

const EncMemDesc encMemDesc_c02_p01[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        (mfxEncMemExtDescription *)&encMemExtDesc_c02_p01_m00,
        0,
#else
        {},
#endif
        1,
        (mfxU32 *)encColorFmt_c02_p01_m00,
    },
};

const EncProfile encProfile_c02[] = {
    {
        MFX_PROFILE_HEVC_MAINSP,
        {},
        1,
        (EncMemDesc *)encMemDesc_c02_p00,
    },
    {
        MFX_PROFILE_HEVC_SCC,
        {},
        1,
        (EncMemDesc *)encMemDesc_c02_p01,
    },
};

#ifdef ONEVPL_EXPERIMENTAL
const mfxU16 encRateControlMethods_c02[] = {
    MFX_RATECONTROL_CBR,
    MFX_RATECONTROL_VBR,
    MFX_RATECONTROL_CQP,
};

const mfxU32 encExtBufferIDs_c02[] = {
    MFX_EXTBUFF_CODING_OPTION,
};

const mfxEncExtDescription encExtDesc_c02 = {
    { 0, 1 },
    {},
    3,
    (mfxU16 *)encRateControlMethods_c02,
    {},
    1,
    (mfxU32 *)encExtBufferIDs_c02,
};
#endif

const EncCodec encCodec[] = {
    {
        MFX_CODEC_AV1,
        MFX_LEVEL_AV1_53,
        1,
#ifdef ONEVPL_EXPERIMENTAL
        (mfxEncExtDescription *)&encExtDesc_c00,
        {},
#else
        {},
#endif
        1,
        (EncProfile *)encProfile_c00,
    },
    {
        MFX_CODEC_AVC,
        MFX_LEVEL_AVC_52,
        1,
#ifdef ONEVPL_EXPERIMENTAL
        (mfxEncExtDescription *)&encExtDesc_c01,
        {},
#else
        {},
#endif
        2,
        (EncProfile *)encProfile_c01,
    },
    {
        MFX_CODEC_HEVC,
        MFX_LEVEL_HEVC_51,
        1,
#ifdef ONEVPL_EXPERIMENTAL
        (mfxEncExtDescription *)&encExtDesc_c02,
        {},
#else
        {},
#endif
        2,
        (EncProfile *)encProfile_c02,
    },
};

const mfxEncoderDescription encoderDesc = {
    { { 1, 1 } },
    {},
    3,
    (EncCodec *)encCodec,
};

// end table formatting
// clang-format on
