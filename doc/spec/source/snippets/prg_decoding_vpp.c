/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <stdlib.h>
#include <stdio.h>

#include "mfxdefs.h"
#include "mfxvideo.h"

#define UNUSED_PARAM(x) (void)(x)

mfxSession session;
mfxBitstream *bitstream;
mfxSurfaceArray *surf_array_out;
mfxVideoParam* decode_par;
mfxVideoChannelParam** vpp_par_array;
mfxU32 num_channel_par;
mfxStatus sts;

static void do_smth(mfxFrameSurface1* surf) {
  UNUSED_PARAM(surf);
}

static void prg_decoding_vpp1 () {
/*beg1*/
num_channel_par = 2;
// first video processing channel with resize
vpp_par_array[0]->VPP.Width = 400;
vpp_par_array[0]->VPP.Height = 400;

// second video channel with color conversion filter
vpp_par_array[1]->VPP.FourCC = MFX_FOURCC_UYVY;

sts = MFXVideoDECODE_VPP_Init(session, decode_par, vpp_par_array, num_channel_par);

sts = MFXVideoDECODE_VPP_DecodeFrameAsync(session, bitstream, NULL, 0, &surf_array_out);

//surf_array_out layout is
do_smth(surf_array_out->Surfaces[0]); //The first channel contains decoded frames.
do_smth(surf_array_out->Surfaces[1]); //The second channel contains resized frames after decode. 
do_smth(surf_array_out->Surfaces[2]); //The third channel contains color converted frames after decode. 
/*end1*/
}


static void prg_decoding_vpp2 () {
/*beg2*/

//1st call
sts = MFXVideoDECODE_VPP_DecodeFrameAsync(session, bitstream, NULL, 0, &surf_array_out);
//surf_array_out layout is
do_smth(surf_array_out->Surfaces[0]); //decoded frame
do_smth(surf_array_out->Surfaces[1]); //resized frame (ChannelId = 1). The first frame from channel with resize available 
// no output from channel with ADI output since it has one frame delay

//2nd call
sts = MFXVideoDECODE_VPP_DecodeFrameAsync(session, bitstream, NULL, 0, &surf_array_out);
//surf_array_out layout is
do_smth(surf_array_out->Surfaces[0]); //decoded frame
do_smth(surf_array_out->Surfaces[1]); //resized frame (ChannelId = 1)
do_smth(surf_array_out->Surfaces[2]); //ADI output (ChannelId = 2). The first frame from ADI channel 
/*end2*/
}
