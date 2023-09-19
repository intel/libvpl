/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "mfxdefs.h"
#include "mfxvideo.h"

/* These macro required for code compilation. */
#define INFINITE 0

mfxSession session;
mfxBitstream *bitstream;
mfxFrameSurface1 *surface_work, *surface_disp;
mfxSyncPoint buffered_syncp, syncp;
/* end of internal stuff */

static void prg_handle_device_busy (mfxSession session, mfxSyncPoint sp) {
    if (sp) {
        MFXVideoCORE_SyncOperation(session, sp, INFINITE);
    } else {
        sleep(5);
    }
}



static void prg_err () {
/*beg1*/
mfxStatus sts=MFX_ERR_NONE;
for (;;) {
   // do something
   sts=MFXVideoDECODE_DecodeFrameAsync(session, bitstream,  surface_work, &surface_disp, &syncp);
   if (sts == MFX_ERR_NONE) buffered_syncp = syncp;
   else if (sts == MFX_WRN_DEVICE_BUSY) prg_handle_device_busy(session, syncp ? syncp : buffered_syncp);

}
/*end1*/
}
