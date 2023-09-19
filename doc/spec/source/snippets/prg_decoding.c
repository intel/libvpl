/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <stdlib.h>
#include <stdio.h>

#include "mfxdefs.h"
#include "mfxvideo.h"
#include "mfxmvc.h"

/* These macro required for code compilation. */
#define INFINITE 0x7FFFFFFF
#define UNUSED_PARAM(x) (void)(x)

mfxSession session;
mfxBitstream *bitstream, *bits;
mfxVideoParam init_param, param;
mfxFrameAllocRequest request;
mfxStatus sts;
mfxFrameSurface1* work, *disp;
mfxSyncPoint syncp;


static void allocate_pool_of_frame_surfaces(int nFrames)
{
    UNUSED_PARAM(nFrames);
    return;
}

static int end_of_stream()
{
    return 1;
}

static void append_more_bitstream(mfxBitstream *bs)
{
    UNUSED_PARAM(bs);
    return;
}

static void find_free_surface_from_the_pool(mfxFrameSurface1 **pool)
{
    UNUSED_PARAM(pool);
    return;
}

static void realloc_surface(mfxFrameSurface1* work, mfxFrameInfo FrameInfo)
{
    UNUSED_PARAM(work);
    UNUSED_PARAM(FrameInfo);
    return;
}

static void do_something_with_decoded_frame(mfxFrameSurface1* disp)
{
    UNUSED_PARAM(disp);
    return;
}

static void free_pool_of_frame_surfaces()
{
    return;
}

static void add_surface_to_pool(mfxFrameSurface1* surf)
{
    UNUSED_PARAM(surf);
    return;
}

static void get_next_surface_from_pool(mfxFrameSurface1 **surface)
{
    UNUSED_PARAM(surface);
    return;
}
/* end of internal stuff */

static void prg_decoding1 () {
/*beg1*/
MFXVideoDECODE_Init(session, &init_param);
sts=MFX_ERR_MORE_DATA;
for (;;) {
   if (sts==MFX_ERR_MORE_DATA && !end_of_stream())
      append_more_bitstream(bitstream);
   bits=(end_of_stream())?NULL:bitstream;
   sts=MFXVideoDECODE_DecodeFrameAsync(session,bits,NULL,&disp,&syncp);
   if (end_of_stream() && sts==MFX_ERR_MORE_DATA) break;
   // skipped other error handling
   if (sts==MFX_ERR_NONE) {
      disp->FrameInterface->Synchronize(disp, INFINITE); // or MFXVideoCORE_SyncOperation(session,syncp,INFINITE)
      do_something_with_decoded_frame(disp);
      disp->FrameInterface->Release(disp);
   }
}
MFXVideoDECODE_Close(session);
/*end1*/
}

static void prg_decoding2 () {
/*beg2*/
MFXVideoDECODE_QueryIOSurf(session, &init_param, &request);
MFXVideoDECODE_Init(session, &init_param);
for (int i = 0; i < request.NumFrameSuggested; i++) {
    MFXMemory_GetSurfaceForDecode(session, &work);
    add_surface_to_pool(work);
}
sts=MFX_ERR_MORE_DATA;
for (;;) {
   if (sts==MFX_ERR_MORE_DATA && !end_of_stream())
      append_more_bitstream(bitstream);
   bits=(end_of_stream())?NULL:bitstream;
   // application logic to distinguish free and busy surfaces
   find_free_surface_from_the_pool(&work);
   sts=MFXVideoDECODE_DecodeFrameAsync(session,bits,work,&disp,&syncp);
   if (end_of_stream() && sts==MFX_ERR_MORE_DATA) break;
   // skipped other error handling
   if (sts==MFX_ERR_NONE) {
      disp->FrameInterface->Synchronize(disp, INFINITE); // or MFXVideoCORE_SyncOperation(session,syncp,INFINITE)
      do_something_with_decoded_frame(disp);
      disp->FrameInterface->Release(disp);
   }
}
for (int i = 0; i < request.NumFrameSuggested; i++) {
    get_next_surface_from_pool(&work);
    work->FrameInterface->Release(work);
}
MFXVideoDECODE_Close(session);
/*end2*/
}


static void prg_decoding3 () {
/*beg3*/
MFXVideoDECODE_DecodeHeader(session, bitstream, &init_param);
MFXVideoDECODE_QueryIOSurf(session, &init_param, &request);
allocate_pool_of_frame_surfaces(request.NumFrameSuggested);
MFXVideoDECODE_Init(session, &init_param);
sts=MFX_ERR_MORE_DATA;
for (;;) {
   if (sts==MFX_ERR_MORE_DATA && !end_of_stream())
      append_more_bitstream(bitstream);
   find_free_surface_from_the_pool(&work);
   bits=(end_of_stream())?NULL:bitstream;
   sts=MFXVideoDECODE_DecodeFrameAsync(session,bits,work,&disp,&syncp);
   if (sts==MFX_ERR_MORE_SURFACE) continue;
   if (end_of_stream() && sts==MFX_ERR_MORE_DATA) break;
   if (sts==MFX_ERR_REALLOC_SURFACE) {
      MFXVideoDECODE_GetVideoParam(session, &param);
      realloc_surface(work, param.mfx.FrameInfo);
      continue;
   }
   // skipped other error handling
   if (sts==MFX_ERR_NONE) {
      disp->FrameInterface->Synchronize(disp, INFINITE); // or MFXVideoCORE_SyncOperation(session,syncp,INFINITE)
      do_something_with_decoded_frame(disp);
   }
}
MFXVideoDECODE_Close(session);
free_pool_of_frame_surfaces();
/*end3*/
}

static void prg_decoding4 () {
/*beg4*/
sts=MFX_ERR_MORE_DATA;
for (;;) {
   if (sts==MFX_ERR_MORE_DATA && !end_of_stream())
      append_more_bitstream(bitstream);
   bits=(end_of_stream())?NULL:bitstream;
   sts=MFXVideoDECODE_DecodeFrameAsync(session,bits,NULL,&disp,&syncp);
   if (sts==MFX_ERR_MORE_SURFACE) continue;
   if (end_of_stream() && sts==MFX_ERR_MORE_DATA) break;
   // skipped other error handling
   if (sts==MFX_ERR_NONE) {
      disp->FrameInterface->Synchronize(disp, INFINITE); // or MFXVideoCORE_SyncOperation(session,syncp,INFINITE)
      do_something_with_decoded_frame(disp);
      disp->FrameInterface->Release(disp);
   }
}
/*end4*/
}

static void prg_decoding5 () {
/*beg5*/
mfxExtBuffer *eb[2];
mfxExtMVCSeqDesc  seq_desc;
mfxVideoParam init_param;

init_param.ExtParam=(mfxExtBuffer **)&eb;
init_param.NumExtParam=1;
eb[0]=(mfxExtBuffer *)&seq_desc;
MFXVideoDECODE_DecodeHeader(session, bitstream, &init_param);

/* select views to decode */
mfxExtMVCTargetViews tv;
init_param.NumExtParam=2;
eb[1]=(mfxExtBuffer *)&tv;

/* initialize decoder */
MFXVideoDECODE_Init(session, &init_param);

/* perform decoding */
for (;;) {
    MFXVideoDECODE_DecodeFrameAsync(session, bits, work, &disp, &syncp);
    disp->FrameInterface->Synchronize(disp, INFINITE); // or MFXVideoCORE_SyncOperation(session,syncp,INFINITE)
}

/* close decoder */
MFXVideoDECODE_Close(session);
/*end5*/
}

static void prg_decoding6 () {
/*beg6*/
MFXVideoDECODE_Init(session, &init_param);
sts=MFX_ERR_MORE_DATA;
for (;;) {
   if (sts==MFX_ERR_MORE_DATA && !end_of_stream())
      append_more_bitstream(bitstream);
   bits=(end_of_stream())?NULL:bitstream;
   MFXMemory_GetSurfaceForDecode(session, &work);
   sts=MFXVideoDECODE_DecodeFrameAsync(session,bits,work,&disp,&syncp);
   work->FrameInterface->Release(work);
   if (end_of_stream() && sts==MFX_ERR_MORE_DATA) break;
   // skipped other error handling
   if (sts==MFX_ERR_NONE) {
      disp->FrameInterface->Synchronize(disp, INFINITE); // or MFXVideoCORE_SyncOperation(session,syncp,INFINITE)
      do_something_with_decoded_frame(disp);
      disp->FrameInterface->Release(disp);
   }
}
MFXVideoDECODE_Close(session);
/*end6*/
}

