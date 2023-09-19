#include <stdlib.h>

#include "mfxdefs.h"
#include "mfxstructures.h"
#include "mfxsession.h"
#include "mfxvideo.h"

/* These macro required for code compilation. */
#define INFINITE 0
#define UNUSED_PARAM(x) (void)(x)

mfxSession session;
mfxBitstream *bs, *bits2;
mfxFrameSurface1 *work, *vin, *vout;
int going_through_vpp=1;

static void allocate_surfaces(mfxU32 num_surfaces) {
   UNUSED_PARAM(num_surfaces);
}

/* end of internal stuff */

static void prg_transcoding1 () {
/*beg1*/
mfxSyncPoint sp_d, sp_e;
MFXVideoDECODE_DecodeFrameAsync(session,bs,work,&vin, &sp_d);
if (going_through_vpp) {
   MFXVideoVPP_RunFrameVPPAsync(session,vin,vout, NULL, &sp_d);
   MFXVideoENCODE_EncodeFrameAsync(session,NULL,vout,bits2,&sp_e);
} else {
   MFXVideoENCODE_EncodeFrameAsync(session,NULL,vin,bits2,&sp_e);
}
MFXVideoCORE_SyncOperation(session,sp_e,INFINITE);
/*end1*/
}

static void prg_transcoding2 () {
/*beg2*/
mfxVideoParam init_param_v, init_param_e;
mfxFrameAllocRequest response_v[2], response_e;

// Desired depth
mfxU16 async_depth=4;

init_param_v.AsyncDepth=async_depth;
MFXVideoVPP_QueryIOSurf(session, &init_param_v, response_v);
init_param_e.AsyncDepth=async_depth;
MFXVideoENCODE_QueryIOSurf(session, &init_param_e, &response_e);
mfxU32 num_surfaces = response_v[1].NumFrameSuggested
         + response_e.NumFrameSuggested
         - async_depth; /* double counted in ENCODE & VPP */

allocate_surfaces(num_surfaces);
/*end2*/
}
