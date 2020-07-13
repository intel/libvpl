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
    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == bs || 0 == par) {
        return MFX_ERR_NULL_PTR;
    }

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);

    return ws->DecodeHeader(bs, par);
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
    mfxStatus sts = MFX_ERR_NONE;

    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == out) {
        return MFX_ERR_NULL_PTR;
    }

    //CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    if (in) {
        // save a local copy of in, since user may set out == in
        mfxVideoParam inCopy = *in;
        in                   = &inCopy;

        // start with out = copy of in (does not deep copy extBufs)
        *out = *in;

        // validate fields in the input param struct

        if (in->mfx.FrameInfo.Width == 0 || in->mfx.FrameInfo.Height == 0)
            sts = MFX_ERR_UNSUPPORTED;

        if (in->mfx.CodecId != MFX_CODEC_AVC &&
            in->mfx.CodecId != MFX_CODEC_HEVC &&
            in->mfx.CodecId != MFX_CODEC_AV1 &&
            in->mfx.CodecId != MFX_CODEC_JPEG &&
            in->mfx.CodecId != MFX_CODEC_MPEG2)
            sts = MFX_ERR_UNSUPPORTED;
    }
    else {
        memset(out, 0, sizeof(mfxVideoParam));

        // set output struct to zero for unsupported params, non-zero for supported params
    }

    return sts;
}

// NOTES - fixed at 1 frame for now
//
// Differences vs. MSDK 1.0 spec
// - only supports system memory, SW impl
// - hard-coded to a single frame
mfxStatus MFXVideoDECODE_QueryIOSurf(mfxSession session,
                                     mfxVideoParam *par,
                                     mfxFrameAllocRequest *request) {
    mfxStatus sts = MFX_ERR_NONE;

    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == par || 0 == request) {
        return MFX_ERR_NULL_PTR;
    }

    request->Info              = par->mfx.FrameInfo;
    request->NumFrameMin       = 1;
    request->NumFrameSuggested = 1;
    request->Type = MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_FROM_DECODE;

    return sts;
}

// NOTES -
//
// Differences vs. MSDK 1.0 spec
// -
mfxStatus MFXVideoDECODE_Init(mfxSession session, mfxVideoParam *par) {
    mfxStatus sts = MFX_ERR_NONE;

    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == par) {
        return MFX_ERR_NULL_PTR;
    }

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);

    if (!ws->getDecInit()) {
        sts = ws->InitDecode(par->mfx.CodecId);
    }

    return sts;
}

// NOTES -
//
// Differences vs. MSDK 1.0 spec
// -
mfxStatus MFXVideoDECODE_Close(mfxSession session) {
    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);

    if (ws->getDecInit() == false)
        return MFX_ERR_NOT_INITIALIZED;

    ws->FreeDecode();

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
    mfxStatus sts = MFX_ERR_NONE;

    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == surface_work || 0 == surface_out || 0 == syncp) {
        return MFX_ERR_NULL_PTR;
    }

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);

    if (ws->getDecInit() == false)
        return MFX_ERR_NOT_INITIALIZED;

    sts = ws->DecodeFrame(bs, surface_work, surface_out);

    // consumes whole frame every time
    if (bs) {
        bs->DataOffset = 0;
        bs->DataLength = 0;
    }

    *syncp = (mfxSyncPoint)(0x12345678);

    return sts;
}

// NOTES -
//
// Differences vs. MSDK 1.0 spec
// -
mfxStatus MFXVideoCORE_SyncOperation(mfxSession session,
                                     mfxSyncPoint syncp,
                                     mfxU32 wait) {
    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == syncp) {
        return MFX_ERR_NULL_PTR;
    }

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    return ws->Sync(wait);
}

mfxStatus MFXVideoDECODE_GetVideoParam(mfxSession session, mfxVideoParam *par) {
    mfxStatus sts = MFX_ERR_NONE;

    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == par) {
        return MFX_ERR_NULL_PTR;
    }

    return sts;
}

mfxStatus MFXVideoDECODE_Reset(mfxSession session, mfxVideoParam *par) {
    mfxStatus sts = MFX_ERR_NONE;

    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == par) {
        return MFX_ERR_NULL_PTR;
    }

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);

    if (ws->getDecInit() == false)
        return MFX_ERR_NOT_INITIALIZED;

    ws->FreeDecode();

    sts = ws->InitDecode(par->mfx.CodecId);

    return sts;
}

// stubs
mfxStatus MFXVideoDECODE_GetDecodeStat(mfxSession session,
                                       mfxDecodeStat *stat) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXVideoDECODE_SetSkipMode(mfxSession session, mfxSkipMode mode) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXVideoDECODE_GetPayload(mfxSession session,
                                    mfxU64 *ts,
                                    mfxPayload *payload) {
    return MFX_ERR_UNSUPPORTED;
}
