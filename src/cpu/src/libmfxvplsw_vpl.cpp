/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
/// oneAPI Video Processing Library (oneVPL) dispatcher query implementation
#include "vpl/mfxdispatcher.h"

#include "./libmfxvplsw_caps.h"

#ifdef __cplusplus
extern "C" {
#endif

// query and release are independent of session - called during
//   caps query and config stage using oneVPL extensions
mfxHDL MFXQueryImplDescription(mfxImplCapsDeliveryFormat format) {
    // only structure format is currently supported
    if (format != MFX_IMPLCAPS_IMPLDESCSTRUCTURE)
        return nullptr;

    mfxImplDescription *implDesc = new mfxImplDescription;

    // clear everything, only allocate new structures as needed
    memset(implDesc, 0, sizeof(mfxImplDescription));

    implDesc->Version.Major = DEF_STRUCT_VERSION_MAJOR;
    implDesc->Version.Minor = DEF_STRUCT_VERSION_MINOR;

    implDesc->Impl             = MFX_IMPL_SOFTWARE;
    implDesc->accelerationMode = 0;

    implDesc->ApiVersion.Major = MFX_VERSION_MAJOR;
    implDesc->ApiVersion.Minor = MFX_VERSION_MINOR;

    strncpy_s((char *)implDesc->ImplName,
              sizeof(implDesc->ImplName),
              "",
              sizeof(implDesc->ImplName) - 1);
    strncpy_s((char *)implDesc->License,
              sizeof(implDesc->License),
              "",
              sizeof(implDesc->ImplName) - 1);
    strncpy_s((char *)implDesc->Keywords,
              sizeof(implDesc->Keywords),
              "",
              sizeof(implDesc->ImplName) - 1);

    implDesc->VendorID     = 0x8086;
    implDesc->VendorImplID = 0;
    implDesc->NumExtParam  = 0;

    InitDecoderCaps(&(implDesc->Dec));

    InitEncoderCaps(&(implDesc->Enc));

    InitVPPCaps(&(implDesc->VPP));

    return (mfxHDL)implDesc;
}

// walk through implDesc and delete dynamically-allocated structs
mfxStatus MFXReleaseImplDescription(mfxHDL hdl) {
    mfxImplDescription *implDesc = (mfxImplDescription *)hdl;

    if (!implDesc) {
        return MFX_ERR_NULL_PTR;
    }

    FreeDecoderCaps(&(implDesc->Dec));
    FreeEncoderCaps(&(implDesc->Enc));
    FreeVPPCaps(&(implDesc->VPP));

    memset(implDesc, 0, sizeof(mfxImplDescription));
    delete implDesc;

    return MFX_ERR_NONE;
}

// memory functions are associated with initialized session
mfxStatus MFXMemory_GetSurfaceForVPP(mfxSession session,
                                     mfxFrameSurface1 **surface) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXMemory_GetSurfaceForEncode(mfxSession session,
                                        mfxFrameSurface1 **surface) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXMemory_GetSurfaceForDecode(mfxSession session,
                                        mfxFrameSurface1 **surface) {
    return MFX_ERR_UNSUPPORTED;
}

#ifdef __cplusplus
}
#endif
