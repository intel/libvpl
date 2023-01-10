/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __MFXDEPRECATED_H__
#define __MFXDEPRECATED_H__

#include "vpl/mfxcommon.h"
#include "vpl/mfxsession.h"
#include "vpl/mfxstructures.h"

#if defined(MFX_ONEVPL)

    #ifdef __cplusplus
extern "C" {
    #endif /* __cplusplus */

enum {
    MFX_ERR_INCOMPATIBLE_AUDIO_PARAM = -19, /* incompatible audio parameters */
    MFX_ERR_INVALID_AUDIO_PARAM      = -20, /* invalid audio parameters */

    MFX_WRN_INCOMPATIBLE_AUDIO_PARAM = 11, /* incompatible audio parameters */
};

mfxStatus MFX_CDECL MFXVideoVPP_RunFrameVPPAsyncEx(mfxSession session,
                                                   mfxFrameSurface1* in,
                                                   mfxFrameSurface1* surface_work,
                                                   mfxFrameSurface1** surface_out,
                                                   mfxSyncPoint* syncp);

MFX_PACK_BEGIN_STRUCT_W_PTR()
typedef struct {
    mfxU32 reserved[4];
    mfxHDL pthis;
    mfxStatus(MFX_CDECL* Alloc)(mfxHDL pthis, mfxU32 nbytes, mfxU16 type, mfxMemId* mid);
    mfxStatus(MFX_CDECL* Lock)(mfxHDL pthis, mfxMemId mid, mfxU8** ptr);
    mfxStatus(MFX_CDECL* Unlock)(mfxHDL pthis, mfxMemId mid);
    mfxStatus(MFX_CDECL* Free)(mfxHDL pthis, mfxMemId mid);
} mfxBufferAllocator;
MFX_PACK_END()

mfxStatus MFX_CDECL MFXVideoCORE_SetBufferAllocator(mfxSession session,
                                                    mfxBufferAllocator* allocator);

    #if defined(MFX_ENABLE_OPAQUE_MEMORY)
        //Sanity check
        #error MFX_ENABLE_OPAQUE_MEMORY should not be defined when 'mfxdeprecated.h' is included
    #endif //MFX_ENABLE_OPAQUE_MEMORY

/* IOPattern */
enum { MFX_IOPATTERN_IN_OPAQUE_MEMORY = 0x04, MFX_IOPATTERN_OUT_OPAQUE_MEMORY = 0x40 };

enum { MFX_MEMTYPE_OPAQUE_FRAME = 0x0004 };

/* Extended Buffer Ids */
enum { MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION = MFX_MAKEFOURCC('O', 'P', 'Q', 'S') };

MFX_PACK_BEGIN_STRUCT_W_PTR()
typedef struct {
    mfxExtBuffer Header;
    mfxU32 reserved1[2];
    struct {
        mfxFrameSurface1** Surfaces;
        mfxU32 reserved2[5];
        mfxU16 Type;
        mfxU16 NumSurface;
    } In, Out;
} mfxExtOpaqueSurfaceAlloc;
MFX_PACK_END()

mfxStatus MFX_CDECL MFXDoWork(mfxSession session);

mfxStatus MFX_CDECL MFXVideoENC_Query(mfxSession session, mfxVideoParam* in, mfxVideoParam* out);
mfxStatus MFX_CDECL MFXVideoENC_QueryIOSurf(mfxSession session,
                                            mfxVideoParam* par,
                                            mfxFrameAllocRequest* request);
mfxStatus MFX_CDECL MFXVideoENC_Init(mfxSession session, mfxVideoParam* par);
mfxStatus MFX_CDECL MFXVideoENC_Reset(mfxSession session, mfxVideoParam* par);
mfxStatus MFX_CDECL MFXVideoENC_Close(mfxSession session);

typedef struct _mfxENCInput {
    char dummy;
} mfxENCInput;
typedef struct _mfxENCOutput {
    char dummy;
} mfxENCOutput;
mfxStatus MFX_CDECL MFXVideoENC_ProcessFrameAsync(mfxSession session,
                                                  mfxENCInput* in,
                                                  mfxENCOutput* out,
                                                  mfxSyncPoint* syncp);

mfxStatus MFX_CDECL MFXVideoENC_GetVideoParam(mfxSession session, mfxVideoParam* par);

MFX_PACK_BEGIN_STRUCT_W_PTR()
typedef struct {
    char dummy;
} mfxPAKInput;
MFX_PACK_END()

MFX_PACK_BEGIN_STRUCT_W_PTR()
typedef struct {
    char dummy;
} mfxPAKOutput;
MFX_PACK_END()

mfxStatus MFX_CDECL MFXVideoPAK_Query(mfxSession session, mfxVideoParam* in, mfxVideoParam* out);
mfxStatus MFX_CDECL MFXVideoPAK_QueryIOSurf(mfxSession session,
                                            mfxVideoParam* par,
                                            mfxFrameAllocRequest request[2]);
mfxStatus MFX_CDECL MFXVideoPAK_Init(mfxSession session, mfxVideoParam* par);
mfxStatus MFX_CDECL MFXVideoPAK_Reset(mfxSession session, mfxVideoParam* par);
mfxStatus MFX_CDECL MFXVideoPAK_Close(mfxSession session);

mfxStatus MFX_CDECL MFXVideoPAK_ProcessFrameAsync(mfxSession session,
                                                  mfxPAKInput* in,
                                                  mfxPAKOutput* out,
                                                  mfxSyncPoint* syncp);

mfxStatus MFX_CDECL MFXVideoPAK_GetVideoParam(mfxSession session, mfxVideoParam* par);

enum {
    /*!
       This extended buffer allow to specify multi-frame submission parameters.
    */
    MFX_EXTBUFF_MULTI_FRAME_PARAM = MFX_MAKEFOURCC('M', 'F', 'R', 'P'),
    /*!
       This extended buffer allow to manage multi-frame submission in runtime.
    */
    MFX_EXTBUFF_MULTI_FRAME_CONTROL = MFX_MAKEFOURCC('M', 'F', 'R', 'C'),
};

/* Multi-Frame Mode */
enum { MFX_MF_DEFAULT = 0, MFX_MF_DISABLED = 1, MFX_MF_AUTO = 2, MFX_MF_MANUAL = 3 };

/* Multi-Frame Initialization parameters */
MFX_PACK_BEGIN_USUAL_STRUCT()
typedef struct {
    mfxExtBuffer Header;

    mfxU16 MFMode;
    mfxU16 MaxNumFrames;

    mfxU16 reserved[58];
} mfxExtMultiFrameParam;
MFX_PACK_END()

/* Multi-Frame Run-time controls */
MFX_PACK_BEGIN_USUAL_STRUCT()
typedef struct {
    mfxExtBuffer Header;

    mfxU32 Timeout; /* timeout in millisecond */
    mfxU16 Flush; /* Flush internal frame buffer, e.g. submit all collected frames. */

    mfxU16 reserved[57];
} mfxExtMultiFrameControl;
MFX_PACK_END()

/* RateControlMethod */
enum {
    MFX_RATECONTROL_LA_EXT = 12,
    MFX_RATECONTROL_VME    = 15,
};
    #ifdef __cplusplus
}
    #endif /* __cplusplus */

#endif //MFX_ONEVPL
#endif //__MFXDEPRECATED_H__
