/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
/// oneAPI Video Processing Library (oneVPL) dispatcher query implementation
#include "vpl/mfxdispatcher.h"
#include "vpl/mfximplcaps.h"

#include "./cpu_workstream.h"
#include "./libmfxvplsw_caps.h"

// query and release are independent of session - called during
//   caps query and config stage using oneVPL extensions
mfxHDL *MFXQueryImplsDescription(mfxImplCapsDeliveryFormat format,
                                 mfxU32 *num_impls) {
    VPL_TRACE_FUNC;
    // only structure format is currently supported
    if (format != MFX_IMPLCAPS_IMPLDESCSTRUCTURE)
        return nullptr;

    // allocate array of mfxHDL for each implementation
    //   (currently there is just one)
    *num_impls     = 1;
    mfxHDL *hImpls = new mfxHDL[*num_impls];
    if (!hImpls)
        return nullptr;
    memset(hImpls, 0, sizeof(mfxHDL) * (*num_impls));

    // allocate ImplDescriptionArray for each implementation
    // the first element must be a struct of type mfxImplDescription
    //   so the dispatcher can cast mfxHDL to mfxImplDescription, and
    //   will just be unaware of any other fields that follow
    ImplDescriptionArray *implDescArray = new ImplDescriptionArray;
    if (!implDescArray) {
        if (hImpls)
            delete[] hImpls;
        return nullptr;
    }

    // in _each_ implDescArray we allocate, save the pointer to the array of handles
    //   and the number of elements
    // MFXReleaseImplDescription can then be called on the individual
    //   handles in any order, and when the last one is freed it
    //   will delete the array of handles
    implDescArray->basePtr  = hImpls;
    implDescArray->currImpl = 0;
    implDescArray->numImpl  = *num_impls;

    // clear everything, only allocate new structures as needed
    mfxImplDescription *implDesc = &(implDescArray->implDesc);
    memset(implDesc, 0, sizeof(mfxImplDescription));
    hImpls[0] = &(implDescArray[0]);

    implDesc->Version.Version = MFX_IMPLDESCRIPTION_VERSION;

    implDesc->Impl             = MFX_IMPL_TYPE_SOFTWARE;
    implDesc->AccelerationMode = MFX_ACCEL_MODE_NA;

    implDesc->ApiVersion.Major = MFX_VERSION_MAJOR;
    implDesc->ApiVersion.Minor = MFX_VERSION_MINOR;

    strncpy_s((char *)implDesc->ImplName,
              sizeof(implDesc->ImplName),
              "oneAPI VPL CPU Reference Impl",
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

    InitDeviceDescription(&(implDesc->Dev));

    InitDecoderCaps(&(implDesc->Dec));

    InitEncoderCaps(&(implDesc->Enc));

    InitVPPCaps(&(implDesc->VPP));

    return hImpls;
}

// walk through implDesc and delete dynamically-allocated structs
mfxStatus MFXReleaseImplDescription(mfxHDL hdl) {
    VPL_TRACE_FUNC;
    ImplDescriptionArray *implDescArray = (ImplDescriptionArray *)hdl;
    if (!implDescArray) {
        return MFX_ERR_NULL_PTR;
    }

    mfxImplDescription *implDesc = &(implDescArray->implDesc);
    if (!implDesc) {
        return MFX_ERR_NULL_PTR;
    }

    FreeDeviceDescription(&(implDesc->Dev));
    FreeDecoderCaps(&(implDesc->Dec));
    FreeEncoderCaps(&(implDesc->Enc));
    FreeVPPCaps(&(implDesc->VPP));

    memset(implDesc, 0, sizeof(mfxImplDescription));

    // remove description from the array of handles (set to null)
    // check if this was the last description to be freed - if so,
    //   delete the array of handles
    mfxHDL *hImpls  = implDescArray->basePtr;
    mfxU32 currImpl = implDescArray->currImpl;
    mfxU32 numImpl  = implDescArray->numImpl;

    hImpls[currImpl] = nullptr;
    delete implDescArray;

    mfxU32 idx;
    for (idx = 0; idx < numImpl; idx++) {
        if (hImpls[idx])
            break;
    }

    if (idx == numImpl)
        delete[] hImpls;

    return MFX_ERR_NONE;
}

// memory functions are associated with initialized session
mfxStatus MFXMemory_GetSurfaceForVPP(mfxSession session,
                                     mfxFrameSurface1 **surface) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session && surface, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    CpuVPP *vpp       = ws->GetVPP();
    RET_IF_FALSE(vpp, MFX_ERR_NOT_INITIALIZED);

    return vpp->GetVPPSurface(surface);
}

mfxStatus MFXMemory_GetSurfaceForEncode(mfxSession session,
                                        mfxFrameSurface1 **surface) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session && surface, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws  = reinterpret_cast<CpuWorkstream *>(session);
    CpuEncode *encoder = ws->GetEncoder();
    RET_IF_FALSE(encoder, MFX_ERR_NOT_INITIALIZED);

    return encoder->GetEncodeSurface(surface);
}

mfxStatus MFXMemory_GetSurfaceForDecode(mfxSession session,
                                        mfxFrameSurface1 **surface) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session && surface, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws  = reinterpret_cast<CpuWorkstream *>(session);
    CpuDecode *decoder = ws->GetDecoder();
    RET_IF_FALSE(decoder, MFX_ERR_NOT_INITIALIZED);

    return decoder->GetDecodeSurface(surface);
}
