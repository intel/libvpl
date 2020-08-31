/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"
#include "vpl/mfxvideo.h"

mfxStatus MFXVideoENCODE_Query(mfxSession session,
                               mfxVideoParam *in,
                               mfxVideoParam *out) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(out, MFX_ERR_NULL_PTR);

    return CpuEncode::EncodeQuery(in, out);
}

mfxStatus MFXVideoENCODE_QueryIOSurf(mfxSession session,
                                     mfxVideoParam *par,
                                     mfxFrameAllocRequest *request) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(par && request, MFX_ERR_NULL_PTR);

    return CpuEncode::EncodeQueryIOSurf(par, request);
}

mfxStatus MFXVideoENCODE_Init(mfxSession session, mfxVideoParam *par) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(par, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    RET_IF_FALSE(ws->GetEncoder() == nullptr, MFX_ERR_UNDEFINED_BEHAVIOR);

    std::unique_ptr<CpuEncode> encoder(new CpuEncode(ws));
    RET_IF_FALSE(encoder, MFX_ERR_MEMORY_ALLOC);
    RET_ERROR(encoder->InitEncode(par));

    ws->SetEncoder(encoder.release());

    return MFX_ERR_NONE;
}

mfxStatus MFXVideoENCODE_Close(mfxSession session) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);

    CpuWorkstream *ws  = reinterpret_cast<CpuWorkstream *>(session);
    CpuEncode *encoder = ws->GetEncoder();
    RET_IF_FALSE(encoder, MFX_ERR_NOT_INITIALIZED);

    ws->SetEncoder(nullptr);

    return MFX_ERR_NONE;
}

mfxStatus MFXVideoENCODE_EncodeFrameAsync(mfxSession session,
                                          mfxEncodeCtrl *ctrl,
                                          mfxFrameSurface1 *surface,
                                          mfxBitstream *bs,
                                          mfxSyncPoint *syncp) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(bs && syncp, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws  = reinterpret_cast<CpuWorkstream *>(session);
    CpuEncode *encoder = ws->GetEncoder();
    RET_IF_FALSE(encoder, MFX_ERR_NOT_INITIALIZED);

    *syncp = (mfxSyncPoint)(0x12345678);

    mfxStatus sts = encoder->EncodeFrame(surface, bs);
    RET_ERROR(sts);
    return sts;
}

// stubs
mfxStatus MFXVideoENCODE_Reset(mfxSession session, mfxVideoParam *par) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(par, MFX_ERR_NULL_PTR);
    MFXVideoENCODE_Close(session);
    return MFXVideoENCODE_Init(session, par);
}

mfxStatus MFXVideoENCODE_GetVideoParam(mfxSession session, mfxVideoParam *par) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(par, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws  = reinterpret_cast<CpuWorkstream *>(session);
    CpuEncode *encoder = ws->GetEncoder();
    RET_IF_FALSE(encoder, MFX_ERR_NOT_INITIALIZED);

    return encoder->GetVideoParam(par);
}

mfxStatus MFXVideoENCODE_GetEncodeStat(mfxSession session,
                                       mfxEncodeStat *stat) {
    VPL_TRACE_FUNC;
    return MFX_ERR_NOT_IMPLEMENTED;
}
