/*############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################*/

//NOLINT(build/header_guard)

#include "src/caps.h"

const mfxU32 encColorFmt_c00_p00_m00[] = {
    MFX_FOURCC_I420,
    MFX_FOURCC_I010,
};

const EncMemDesc encMemDesc_c00_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        nullptr,
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

const mfxU32 encColorFmt_c01_p00_m00[] = {
    MFX_FOURCC_I420,
};

const EncMemDesc encMemDesc_c01_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        nullptr,
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

const EncMemDesc encMemDesc_c01_p01[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        nullptr,
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

const mfxU32 encColorFmt_c02_p00_m00[] = {
    MFX_FOURCC_I420,
};

const EncMemDesc encMemDesc_c02_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        nullptr,
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

const EncMemDesc encMemDesc_c02_p01[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        nullptr,
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

const EncCodec encCodec[] = {
    {
        MFX_CODEC_AV1,
        MFX_LEVEL_AV1_53,
        1,
#ifdef ONEVPL_EXPERIMENTAL
        nullptr,
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
        nullptr,
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
        nullptr,
        {},
#else
        {},
#endif
        2,
        (EncProfile *)encProfile_c02,
    },
};

const mfxEncoderDescription encoderDesc = {
    { { 0, 1 } },
    {},
    3,
    (EncCodec *)encCodec,
};
