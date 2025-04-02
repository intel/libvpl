/*############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################*/

//NOLINT(build/header_guard)

#include "src/caps.h"

const mfxU32 decColorFmt_c00_p00_m00[] = {
    MFX_FOURCC_I420,
    MFX_FOURCC_I010,
};

const DecMemDesc decMemDesc_c00_p00[] = {
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
        (mfxU32 *)decColorFmt_c00_p00_m00,
    },
};

const DecProfile decProfile_c00[] = {
    {
        MFX_PROFILE_AV1_HIGH,
        {},
        1,
        (DecMemDesc *)decMemDesc_c00_p00,
    },
};

const mfxU32 decColorFmt_c01_p00_m00[] = {
    MFX_FOURCC_I420,
};

const DecMemDesc decMemDesc_c01_p00[] = {
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
        (mfxU32 *)decColorFmt_c01_p00_m00,
    },
};

const mfxU32 decColorFmt_c01_p01_m00[] = {
    MFX_FOURCC_I010,
};

const DecMemDesc decMemDesc_c01_p01[] = {
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
        (mfxU32 *)decColorFmt_c01_p01_m00,
    },
};

const DecProfile decProfile_c01[] = {
    {
        MFX_PROFILE_MPEG2_MAIN,
        {},
        1,
        (DecMemDesc *)decMemDesc_c01_p00,
    },
    {
        MFX_PROFILE_MPEG2_HIGH,
        {},
        1,
        (DecMemDesc *)decMemDesc_c01_p01,
    },
};

const mfxU32 decColorFmt_c02_p00_m00[] = {
    MFX_FOURCC_I420,
};

const DecMemDesc decMemDesc_c02_p00[] = {
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
        (mfxU32 *)decColorFmt_c02_p00_m00,
    },
};

const mfxU32 decColorFmt_c02_p01_m00[] = {
    MFX_FOURCC_I010,
};

const DecMemDesc decMemDesc_c02_p01[] = {
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
        (mfxU32 *)decColorFmt_c02_p01_m00,
    },
};

const DecProfile decProfile_c02[] = {
    {
        MFX_PROFILE_HEVC_MAINSP,
        {},
        1,
        (DecMemDesc *)decMemDesc_c02_p00,
    },
    {
        MFX_PROFILE_HEVC_SCC,
        {},
        1,
        (DecMemDesc *)decMemDesc_c02_p01,
    },
};

const DecCodec decCodec[] = {
    {
        MFX_CODEC_AV1,
#ifdef ONEVPL_EXPERIMENTAL
        {},
        nullptr,
        {},
#else
        {},
#endif
        MFX_LEVEL_AV1_53,
        1,
        (DecProfile *)decProfile_c00,
    },
    {
        MFX_CODEC_MPEG2,
#ifdef ONEVPL_EXPERIMENTAL
        {},
        nullptr,
        {},
#else
        {},
#endif
        MFX_LEVEL_MPEG2_HIGH,
        2,
        (DecProfile *)decProfile_c01,
    },
    {
        MFX_CODEC_HEVC,
#ifdef ONEVPL_EXPERIMENTAL
        {},
        nullptr,
        {},
#else
        {},
#endif
        MFX_LEVEL_HEVC_51,
        2,
        (DecProfile *)decProfile_c02,
    },
};

const mfxDecoderDescription decoderDesc = {
    { 0, 1 },
    {},
    3,
    (DecCodec *)decCodec,
};
