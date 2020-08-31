/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"
#include "vpl/mfxvideo.h"

// stubs
mfxStatus MFXVideoVPP_Query(mfxSession session,
                            mfxVideoParam *in,
                            mfxVideoParam *out) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(out, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(out->Protected == 0, MFX_ERR_UNSUPPORTED);
    RET_IF_FALSE(in == nullptr || in->Protected == 0, MFX_ERR_UNSUPPORTED);

    return CpuVPP::VPPQuery(in, out);
}

mfxStatus MFXVideoVPP_QueryIOSurf(mfxSession session,
                                  mfxVideoParam *par,
                                  mfxFrameAllocRequest request[2]) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(par, MFX_ERR_NULL_PTR);

    return CpuVPP::VPPQueryIOSurf(par, request);
}

mfxStatus MFXVideoVPP_Init(mfxSession session, mfxVideoParam *par) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(par, MFX_ERR_NULL_PTR);
    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);

    std::unique_ptr<CpuVPP> vpp(new CpuVPP(ws));
    RET_IF_FALSE(vpp, MFX_ERR_MEMORY_ALLOC);
    RET_ERROR(vpp->InitVPP(par));

    ws->SetVPP(vpp.release());

    return MFX_ERR_NONE;
}

mfxStatus MFXVideoVPP_Close(mfxSession session) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    ws->SetVPP(nullptr);

    return MFX_ERR_NONE;
}

mfxStatus MFXVideoVPP_GetVideoParam(mfxSession session, mfxVideoParam *par) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(par, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    CpuVPP *vpp       = ws->GetVPP();
    RET_IF_FALSE(vpp, MFX_ERR_NOT_INITIALIZED);

    return vpp->GetVideoParam(par);
}

mfxStatus MFXVideoVPP_RunFrameVPPAsync(mfxSession session,
                                       mfxFrameSurface1 *in,
                                       mfxFrameSurface1 *out,
                                       mfxExtVppAuxData *aux,
                                       mfxSyncPoint *syncp) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(out && syncp, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    CpuVPP *vpp       = ws->GetVPP();
    RET_IF_FALSE(vpp, MFX_ERR_NOT_INITIALIZED);

    *syncp = (mfxSyncPoint)(0x12345678);

    return vpp->ProcessFrame(in, out, aux);
}

mfxStatus MFXVideoVPP_Reset(mfxSession session, mfxVideoParam *par) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(par, MFX_ERR_NULL_PTR);
    MFXVideoVPP_Close(session);
    return MFXVideoVPP_Init(session, par);
}

mfxStatus MFXVideoVPP_GetVPPStat(mfxSession session, mfxVPPStat *stat) {
    VPL_TRACE_FUNC;
    return MFX_ERR_NOT_IMPLEMENTED;
}
