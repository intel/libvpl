/*############################################################################
  # Copyright (C) 2013 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __MFXAUDIO_H__
#define __MFXAUDIO_H__
#include "mfxastructures.h"
#include "vpl/mfxsession.h"

#define MFX_AUDIO_VERSION_MAJOR 1
#define MFX_AUDIO_VERSION_MINOR 15

#ifdef __cplusplus
extern "C" {
#endif

/* AudioCORE */
mfxStatus MFX_CDECL MFXAudioCORE_SyncOperation(mfxSession session, mfxSyncPoint syncp, mfxU32 wait);

/* AudioENCODE */
mfxStatus MFX_CDECL MFXAudioENCODE_Query(mfxSession session, mfxAudioParam* in, mfxAudioParam* out);
mfxStatus MFX_CDECL MFXAudioENCODE_QueryIOSize(mfxSession session,
                                               mfxAudioParam* par,
                                               mfxAudioAllocRequest* request);
mfxStatus MFX_CDECL MFXAudioENCODE_Init(mfxSession session, mfxAudioParam* par);
mfxStatus MFX_CDECL MFXAudioENCODE_Reset(mfxSession session, mfxAudioParam* par);
mfxStatus MFX_CDECL MFXAudioENCODE_Close(mfxSession session);
mfxStatus MFX_CDECL MFXAudioENCODE_GetAudioParam(mfxSession session, mfxAudioParam* par);
mfxStatus MFX_CDECL MFXAudioENCODE_EncodeFrameAsync(mfxSession session,
                                                    mfxAudioFrame* frame,
                                                    mfxBitstream* bs,
                                                    mfxSyncPoint* syncp);

/* AudioDECODE */
mfxStatus MFX_CDECL MFXAudioDECODE_Query(mfxSession session, mfxAudioParam* in, mfxAudioParam* out);
mfxStatus MFX_CDECL MFXAudioDECODE_DecodeHeader(mfxSession session,
                                                mfxBitstream* bs,
                                                mfxAudioParam* par);
mfxStatus MFX_CDECL MFXAudioDECODE_Init(mfxSession session, mfxAudioParam* par);
mfxStatus MFX_CDECL MFXAudioDECODE_Reset(mfxSession session, mfxAudioParam* par);
mfxStatus MFX_CDECL MFXAudioDECODE_Close(mfxSession session);
mfxStatus MFX_CDECL MFXAudioDECODE_QueryIOSize(mfxSession session,
                                               mfxAudioParam* par,
                                               mfxAudioAllocRequest* request);
mfxStatus MFX_CDECL MFXAudioDECODE_GetAudioParam(mfxSession session, mfxAudioParam* par);
mfxStatus MFX_CDECL MFXAudioDECODE_DecodeFrameAsync(mfxSession session,
                                                    mfxBitstream* bs,
                                                    mfxAudioFrame* frame,
                                                    mfxSyncPoint* syncp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
