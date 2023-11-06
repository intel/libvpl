/*############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################*/

//NOLINT(build/header_guard)

#include "src/caps.h"

#ifdef ONEVPL_EXPERIMENTAL

// use same config for DX11 and VAAPI to simplify utest implementation
static const mfxSurfaceTypesSupported::surftype::surfcomp minSurfComp_DX11_VAAPI[4] = {
    {
        MFX_SURFACE_COMPONENT_ENCODE,
        (MFX_SURFACE_FLAG_IMPORT_SHARED | MFX_SURFACE_FLAG_IMPORT_COPY),
        {},
    },
    {
        MFX_SURFACE_COMPONENT_VPP_INPUT,
        (MFX_SURFACE_FLAG_IMPORT_COPY),
        {},
    },
    {
        MFX_SURFACE_COMPONENT_VPP_OUTPUT,
        (MFX_SURFACE_FLAG_EXPORT_SHARED | MFX_SURFACE_FLAG_EXPORT_COPY),
        {},
    },
    {
        MFX_SURFACE_COMPONENT_DECODE,
        (MFX_SURFACE_FLAG_EXPORT_COPY),
        {},
    },
};

    #ifdef _WIN32
static const mfxSurfaceTypesSupported::surftype minSurfType_DX11 = {
    MFX_SURFACE_TYPE_D3D11_TEX2D,
    {},
    4,
    (mfxSurfaceTypesSupported::surftype::surfcomp *)minSurfComp_DX11_VAAPI,
};
    #else

static const mfxSurfaceTypesSupported::surftype minSurfType_VAAPI = {
    MFX_SURFACE_TYPE_VAAPI,
    {},
    4,
    (mfxSurfaceTypesSupported::surftype::surfcomp *)minSurfComp_DX11_VAAPI,
};
    #endif

static const mfxSurfaceTypesSupported::surftype::surfcomp minSurfComp_OpenCL[4] = {
    {
        MFX_SURFACE_COMPONENT_ENCODE,
        (MFX_SURFACE_FLAG_IMPORT_COPY),
        {},
    },
    {
        MFX_SURFACE_COMPONENT_VPP_INPUT,
        (MFX_SURFACE_FLAG_IMPORT_COPY),
        {},
    },
    {
        MFX_SURFACE_COMPONENT_DECODE,
        (MFX_SURFACE_FLAG_EXPORT_SHARED | MFX_SURFACE_FLAG_EXPORT_COPY),
        {},
    },
};

static const mfxSurfaceTypesSupported::surftype minSurfType_OpenCL = {
    MFX_SURFACE_TYPE_OPENCL_IMG2D,
    {},
    3,
    (mfxSurfaceTypesSupported::surftype::surfcomp *)minSurfComp_OpenCL,
};

static const mfxSurfaceTypesSupported::surftype minSurfTypes[] = {
    #ifdef _WIN32
    minSurfType_DX11,
    #else
    minSurfType_VAAPI,
    #endif
    minSurfType_OpenCL
};

const mfxSurfaceTypesSupported minSurfTypesSupported = {
    { { 0, 1 } }, // Version

    2,
    (mfxSurfaceTypesSupported::surftype *)minSurfTypes,

    {}, // Reserved
};

#endif // ONEVPL_EXPERIMENTAL