/*############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################*/

//NOLINT(build/header_guard)

#include "src/caps.h"

const mfxU32 vppFormatOut_f00_m00_i00[] = {
    MFX_FOURCC_I420,
    MFX_FOURCC_RGB4,
};

const mfxU32 vppFormatOut_f00_m00_i01[] = {
    MFX_FOURCC_I010,
    MFX_FOURCC_RGB4,
};

const VPPFormat vppFormatIn_f00_m00[] = {
    {
        MFX_FOURCC_I010,
        {},
        2,
        (mfxU32 *)vppFormatOut_f00_m00_i00,
    },
    {
        MFX_FOURCC_I420,
        {},
        2,
        (mfxU32 *)vppFormatOut_f00_m00_i01,
    },
};

const VPPMemDesc vppMemDesc_f00[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        2,
        (VPPFormat *)vppFormatIn_f00_m00,
    },
};

const mfxU32 vppFormatOut_f01_m00_i00[] = {
    MFX_FOURCC_I010,
};

const mfxU32 vppFormatOut_f01_m00_i01[] = {
    MFX_FOURCC_I420,
};

const mfxU32 vppFormatOut_f01_m00_i02[] = {
    MFX_FOURCC_RGB4,
};

const VPPFormat vppFormatIn_f01_m00[] = {
    {
        MFX_FOURCC_I010,
        {},
        1,
        (mfxU32 *)vppFormatOut_f01_m00_i00,
    },
    {
        MFX_FOURCC_I420,
        {},
        1,
        (mfxU32 *)vppFormatOut_f01_m00_i01,
    },
    {
        MFX_FOURCC_RGB4,
        {},
        1,
        (mfxU32 *)vppFormatOut_f01_m00_i02,
    },
};

const VPPMemDesc vppMemDesc_f01[] = {
    {
        MFX_RESOURCE_SYSTEM_SURFACE,
        { 64, 4096, 8 },
        { 64, 4096, 8 },
        {},
        3,
        (VPPFormat *)vppFormatIn_f01_m00,
    },
};

const VPPFilter vppFilter[] = {
    {
        MFX_EXTBUFF_VPP_COLOR_CONVERSION,
        1,
        {},
        1,
        (VPPMemDesc *)vppMemDesc_f00,
    },
    {
        MFX_EXTBUFF_VPP_DEINTERLACING,
        1,
        {},
        1,
        (VPPMemDesc *)vppMemDesc_f01,
    },
};

const mfxVPPDescription vppDesc = {
    { 0, 1 },
    {},
    2,
    (VPPFilter *)vppFilter,
};
