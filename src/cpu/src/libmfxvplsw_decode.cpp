/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"
#include "vpl/mfxvideo.h"

// NOTES - for now, just do init, decode the first frame, get params, close codec
//
// Differences vs. MSDK 1.0 spec
// - codec init does not happen here, just header parsing
// - should not decode a frame
// - may be called at any time before or after initialization
// - should search for sequence header and move mfxBitstream to first byte
// - optionally returns header in mfxExtCodingOptionSPSPPS struct
mfxStatus MFXVideoDECODE_DecodeHeader(mfxSession session,
                                      mfxBitstream *bs,
                                      mfxVideoParam *par) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(bs, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(par, MFX_ERR_NULL_PTR);
    RET_IF_FALSE(bs->DataLength > 0, MFX_ERR_MORE_DATA);

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);

    std::unique_ptr<CpuDecode> decoder(new CpuDecode(ws));
    RET_IF_FALSE(decoder, MFX_ERR_MEMORY_ALLOC);
    RET_ERROR(decoder->InitDecode(par, bs));

    return MFX_ERR_NONE;
}

// NOTES - only support the minimum parameters for basic decode
//
// Differences vs. MSDK 1.0 spec
// - not all parameters filled out
// - in == null mode not implemented (mark supported params only)
// - extended buffers not copied/queried
mfxStatus MFXVideoDECODE_Query(mfxSession session,
                               mfxVideoParam *in,
                               mfxVideoParam *out) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(out, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    (void)ws;
    return CpuDecode::DecodeQuery(in, out);
}

// NOTES - fixed at 1 frame for now
//
// Differences vs. MSDK 1.0 spec
// - only supports system memory, SW impl
mfxStatus MFXVideoDECODE_QueryIOSurf(mfxSession session,
                                     mfxVideoParam *par,
                                     mfxFrameAllocRequest *request) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(par && request, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    (void)ws;
    return CpuDecode::DecodeQueryIOSurf(par, request);
}

// NOTES -
//
// Differences vs. MSDK 1.0 spec
// -
mfxStatus MFXVideoDECODE_Init(mfxSession session, mfxVideoParam *par) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(par, MFX_ERR_NULL_PTR);
    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    if (ws->GetDecoder())
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    std::unique_ptr<CpuDecode> decoder(new CpuDecode(ws));
    RET_IF_FALSE(decoder, MFX_ERR_MEMORY_ALLOC);
    RET_ERROR(decoder->InitDecode(par, nullptr));

    ws->SetDecoder(decoder.release());

    return MFX_ERR_NONE;
}

// NOTES -
//
// Differences vs. MSDK 1.0 spec
// -
mfxStatus MFXVideoDECODE_Close(mfxSession session) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);

    if (ws->GetDecoder() != nullptr)
        ws->SetDecoder(nullptr);

    return MFX_ERR_NONE;
}

// NOTES -
//
// Differences vs. MSDK 1.0 spec
// -
mfxStatus MFXVideoDECODE_DecodeFrameAsync(mfxSession session,
                                          mfxBitstream *bs,
                                          mfxFrameSurface1 *surface_work,
                                          mfxFrameSurface1 **surface_out,
                                          mfxSyncPoint *syncp) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(surface_out && syncp, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws  = reinterpret_cast<CpuWorkstream *>(session);
    CpuDecode *decoder = ws->GetDecoder();
    if (!decoder) {
        // Only 2.0 API permits lazy init - requires internal memory management
        RET_IF_FALSE(surface_work == 0, MFX_ERR_NOT_INITIALIZED);

        mfxVideoParam param = { 0 };
        param.mfx.CodecId   = bs->CodecId;
        RET_ERROR(MFXVideoDECODE_DecodeHeader(session, bs, &param));
        RET_ERROR(MFXVideoDECODE_Init(session, &param));
        decoder = ws->GetDecoder();
    }

    bool bInternalMem = false;
    if (surface_work == 0) {
        // get a ref-counted surface for decoding into
        // behavior is equivalent to the application calling this and then
        //   passing the surface into DecodeFrameAsync()
        RET_ERROR(MFXMemory_GetSurfaceForDecode(session, &surface_work));
        bInternalMem = true;
    }

    mfxStatus sts = decoder->DecodeFrame(bs, surface_work, surface_out);

    // application will not know to release surface (e.g. if we
    //   need more data) so need to release it here
    if (bInternalMem && sts != MFX_ERR_NONE) {
        surface_work->FrameInterface->Release(surface_work);
    }

    *syncp = (mfxSyncPoint)(0x12345678);

    return sts;
}

mfxStatus MFXVideoDECODE_GetVideoParam(mfxSession session, mfxVideoParam *par) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(par, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws  = reinterpret_cast<CpuWorkstream *>(session);
    CpuDecode *decoder = ws->GetDecoder();
    RET_IF_FALSE(decoder, MFX_ERR_NOT_INITIALIZED);

    return decoder->GetVideoParam(par);
}

mfxStatus MFXVideoDECODE_Reset(mfxSession session, mfxVideoParam *par) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(par, MFX_ERR_NULL_PTR);

    CpuWorkstream *ws  = reinterpret_cast<CpuWorkstream *>(session);
    CpuDecode *decoder = ws->GetDecoder();
    RET_IF_FALSE(decoder, MFX_ERR_NOT_INITIALIZED);

    RET_ERROR(decoder->CheckVideoParamDecoders(par));
    mfxVideoParam oldParam = { 0 };
    decoder->GetVideoParam(&oldParam);
    RET_ERROR(decoder->IsSameVideoParam(par, &oldParam));

    RET_ERROR(MFXVideoDECODE_Close(session));
    return MFXVideoDECODE_Init(session, par);
}

// stubs
mfxStatus MFXVideoDECODE_GetDecodeStat(mfxSession session,
                                       mfxDecodeStat *stat) {
    VPL_TRACE_FUNC;
    return MFX_ERR_NOT_IMPLEMENTED;
}
mfxStatus MFXVideoDECODE_SetSkipMode(mfxSession session, mfxSkipMode mode) {
    VPL_TRACE_FUNC;
    return MFX_ERR_NOT_IMPLEMENTED;
}
mfxStatus MFXVideoDECODE_GetPayload(mfxSession session,
                                    mfxU64 *ts,
                                    mfxPayload *payload) {
    VPL_TRACE_FUNC;
    return MFX_ERR_NOT_IMPLEMENTED;
}
