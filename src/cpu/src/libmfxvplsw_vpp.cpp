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
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXVideoVPP_QueryIOSurf(mfxSession session,
                                  mfxVideoParam *par,
                                  mfxFrameAllocRequest request[2]) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXVideoVPP_Init(mfxSession session, mfxVideoParam *par) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXVideoVPP_Reset(mfxSession session, mfxVideoParam *par) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXVideoVPP_Close(mfxSession session) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXVideoVPP_GetVideoParam(mfxSession session, mfxVideoParam *par) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXVideoVPP_GetVPPStat(mfxSession session, mfxVPPStat *stat) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXVideoVPP_RunFrameVPPAsync(mfxSession session,
                                       mfxFrameSurface1 *in,
                                       mfxFrameSurface1 *out,
                                       mfxExtVppAuxData *aux,
                                       mfxSyncPoint *syncp) {
    return MFX_ERR_UNSUPPORTED;
}
