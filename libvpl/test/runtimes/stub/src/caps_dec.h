/*############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################*/

//NOLINT(build/header_guard)

#include "src/caps.h"

// leave table formatting alone
// clang-format off

const mfxU32 decColorFmt_c00_p00_m00[] = {
    MFX_FOURCC_I420,
    MFX_FOURCC_I010,
};

#ifdef ONEVPL_EXPERIMENTAL
const mfxU16 decChromaSubsampling_c00_p00_m00[] = {
    MFX_CHROMAFORMAT_YUV420,
};

const mfxDecMemExtDescription decMemExtDesc_c00_p00_m00 = {
    { 0, 1 },
    {},
    10,
    1,
    (mfxU16 *)decChromaSubsampling_c00_p00_m00,
};
#endif

const DecMemDesc decMemDesc_c00_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        (mfxDecMemExtDescription *)&decMemExtDesc_c00_p00_m00,
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

#ifdef ONEVPL_EXPERIMENTAL
const mfxU32 decExtBufferIDs_c00[] = {
    MFX_EXTBUFF_VIDEO_SIGNAL_INFO,
    MFX_EXTBUFF_DECODED_FRAME_INFO,
};

const mfxDecExtDescription decExtDesc_c00 = {
    { 0, 1 },
    {},
    2,
    (mfxU32 *)decExtBufferIDs_c00,
};
#endif

const mfxU32 decColorFmt_c01_p00_m00[] = {
    MFX_FOURCC_I420,
};

#ifdef ONEVPL_EXPERIMENTAL
const mfxU16 decChromaSubsampling_c01_p00_m00[] = {
    MFX_CHROMAFORMAT_YUV420,
};

const mfxDecMemExtDescription decMemExtDesc_c01_p00_m00 = {
    { 0, 1 },
    {},
    8,
    1,
    (mfxU16 *)decChromaSubsampling_c01_p00_m00,
};
#endif

const DecMemDesc decMemDesc_c01_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        (mfxDecMemExtDescription *)&decMemExtDesc_c01_p00_m00,
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

#ifdef ONEVPL_EXPERIMENTAL
const mfxU16 decChromaSubsampling_c01_p01_m00[] = {
    MFX_CHROMAFORMAT_YUV420,
};

const mfxDecMemExtDescription decMemExtDesc_c01_p01_m00 = {
    { 0, 1 },
    {},
    10,
    1,
    (mfxU16 *)decChromaSubsampling_c01_p01_m00,
};
#endif

const DecMemDesc decMemDesc_c01_p01[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        (mfxDecMemExtDescription *)&decMemExtDesc_c01_p01_m00,
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

#ifdef ONEVPL_EXPERIMENTAL
const mfxU32 decExtBufferIDs_c01[] = {
    MFX_EXTBUFF_VIDEO_SIGNAL_INFO,
    MFX_EXTBUFF_DECODED_FRAME_INFO,
};

const mfxDecExtDescription decExtDesc_c01 = {
    { 0, 1 },
    {},
    2,
    (mfxU32 *)decExtBufferIDs_c01,
};
#endif

const mfxU32 decColorFmt_c02_p00_m00[] = {
    MFX_FOURCC_I420,
};

#ifdef ONEVPL_EXPERIMENTAL
const mfxU16 decChromaSubsampling_c02_p00_m00[] = {
    MFX_CHROMAFORMAT_YUV420,
};

const mfxDecMemExtDescription decMemExtDesc_c02_p00_m00 = {
    { 0, 1 },
    {},
    8,
    1,
    (mfxU16 *)decChromaSubsampling_c02_p00_m00,
};
#endif

const DecMemDesc decMemDesc_c02_p00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        (mfxDecMemExtDescription *)&decMemExtDesc_c02_p00_m00,
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

#ifdef ONEVPL_EXPERIMENTAL
const mfxU16 decChromaSubsampling_c02_p01_m00[] = {
    MFX_CHROMAFORMAT_YUV420,
};

const mfxDecMemExtDescription decMemExtDesc_c02_p01_m00 = {
    { 0, 1 },
    {},
    10,
    1,
    (mfxU16 *)decChromaSubsampling_c02_p01_m00,
};
#endif

const DecMemDesc decMemDesc_c02_p01[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
#ifdef ONEVPL_EXPERIMENTAL
        {},
        (mfxDecMemExtDescription *)&decMemExtDesc_c02_p01_m00,
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

#ifdef ONEVPL_EXPERIMENTAL
const mfxU32 decExtBufferIDs_c02[] = {
    MFX_EXTBUFF_VIDEO_SIGNAL_INFO,
    MFX_EXTBUFF_DECODED_FRAME_INFO,
    MFX_EXTBUFF_HEVC_PARAM,
};

const mfxDecExtDescription decExtDesc_c02 = {
    { 0, 1 },
    {},
    3,
    (mfxU32 *)decExtBufferIDs_c02,
};
#endif

const DecCodec decCodec[] = {
    {
        MFX_CODEC_AV1,
#ifdef ONEVPL_EXPERIMENTAL
        {},
        (mfxDecExtDescription *)&decExtDesc_c00,
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
        (mfxDecExtDescription *)&decExtDesc_c01,
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
        (mfxDecExtDescription *)&decExtDesc_c02,
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
    { 1, 1 },
    {},
    3,
    (DecCodec *)decCodec,
};

// end table formatting
// clang-format on
