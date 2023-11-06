/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "vpl/mfx.h"

mfxStatus MFXInit(mfxIMPL implParam, mfxVersion *ver, mfxSession *session) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXJoinSession(mfxSession session, mfxSession child) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXSetPriority(mfxSession session, mfxPriority priority) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXGetPriority(mfxSession session, mfxPriority *priority) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoCORE_SetFrameAllocator(mfxSession session, mfxFrameAllocator *allocator) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoCORE_SetHandle(mfxSession session, mfxHandleType type, mfxHDL hdl) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoCORE_GetHandle(mfxSession session, mfxHandleType type, mfxHDL *hdl) {
    // per spec, GetHandle returns UNDEFINED_BEHAVIOR for unknown handle type, which for stub RT is currently all types
    return MFX_ERR_UNDEFINED_BEHAVIOR;
}

mfxStatus MFXVideoCORE_QueryPlatform(mfxSession session, mfxPlatform *platform) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoCORE_SyncOperation(mfxSession session, mfxSyncPoint syncp, mfxU32 wait) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_DecodeHeader(mfxSession session, mfxBitstream *bs, mfxVideoParam *par) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_QueryIOSurf(mfxSession session,
                                     mfxVideoParam *par,
                                     mfxFrameAllocRequest *request) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_Init(mfxSession session, mfxVideoParam *par) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_Close(mfxSession session) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_DecodeFrameAsync(mfxSession session,
                                          mfxBitstream *bs,
                                          mfxFrameSurface1 *surface_work,
                                          mfxFrameSurface1 **surface_out,
                                          mfxSyncPoint *syncp) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_GetVideoParam(mfxSession session, mfxVideoParam *par) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_Reset(mfxSession session, mfxVideoParam *par) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_GetDecodeStat(mfxSession session, mfxDecodeStat *stat) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_SetSkipMode(mfxSession session, mfxSkipMode mode) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_GetPayload(mfxSession session, mfxU64 *ts, mfxPayload *payload) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_VPP_Init(mfxSession session,
                                  mfxVideoParam *decode_par,
                                  mfxVideoChannelParam **vpp_par_array,
                                  mfxU32 num_vpp_par) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_VPP_DecodeFrameAsync(mfxSession session,
                                              mfxBitstream *bs,
                                              mfxU32 *skip_channels,
                                              mfxU32 num_skip_channels,
                                              mfxSurfaceArray **surf_array_out) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_VPP_Reset(mfxSession session,
                                   mfxVideoParam *decode_par,
                                   mfxVideoChannelParam **vpp_par_array,
                                   mfxU32 num_vpp_par) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_VPP_GetChannelParam(mfxSession session,
                                             mfxVideoChannelParam *par,
                                             mfxU32 channel_id) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoDECODE_VPP_Close(mfxSession session) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoENCODE_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoENCODE_QueryIOSurf(mfxSession session,
                                     mfxVideoParam *par,
                                     mfxFrameAllocRequest *request) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoENCODE_Init(mfxSession session, mfxVideoParam *par) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoENCODE_Close(mfxSession session) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoENCODE_EncodeFrameAsync(mfxSession session,
                                          mfxEncodeCtrl *ctrl,
                                          mfxFrameSurface1 *surface,
                                          mfxBitstream *bs,
                                          mfxSyncPoint *syncp) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoENCODE_Reset(mfxSession session, mfxVideoParam *par) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoENCODE_GetVideoParam(mfxSession session, mfxVideoParam *par) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoENCODE_GetEncodeStat(mfxSession session, mfxEncodeStat *stat) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoVPP_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoVPP_QueryIOSurf(mfxSession session,
                                  mfxVideoParam *par,
                                  mfxFrameAllocRequest request[2]) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoVPP_Init(mfxSession session, mfxVideoParam *par) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoVPP_Close(mfxSession session) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoVPP_GetVideoParam(mfxSession session, mfxVideoParam *par) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoVPP_RunFrameVPPAsync(mfxSession session,
                                       mfxFrameSurface1 *in,
                                       mfxFrameSurface1 *out,
                                       mfxExtVppAuxData *aux,
                                       mfxSyncPoint *syncp) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoVPP_Reset(mfxSession session, mfxVideoParam *par) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoVPP_GetVPPStat(mfxSession session, mfxVPPStat *stat) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXVideoVPP_ProcessFrameAsync(mfxSession session,
                                        mfxFrameSurface1 *in,
                                        mfxFrameSurface1 **out) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

// memory functions are associated with initialized session
mfxStatus MFXMemory_GetSurfaceForVPP(mfxSession session, mfxFrameSurface1 **surface) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXMemory_GetSurfaceForEncode(mfxSession session, mfxFrameSurface1 **surface) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXMemory_GetSurfaceForDecode(mfxSession session, mfxFrameSurface1 **surface) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

mfxStatus MFXMemory_GetSurfaceForVPPOut(mfxSession session, mfxFrameSurface1 **surface) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

// DLL entry point

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID lpReserved) {
    return TRUE;
} // BOOL APIENTRY DllMain(HMODULE hModule,
#else // #if defined(_WIN32) || defined(_WIN64)
void __attribute__((constructor)) dll_init(void) {}
#endif // #if defined(_WIN32) || defined(_WIN64)
