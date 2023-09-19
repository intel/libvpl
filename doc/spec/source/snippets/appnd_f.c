/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
  
#include <string.h>

#include "mfxdefs.h"
#include "mfxvideo.h"

/* These macro required for code compilation. */
mfxSession session;
#define valid_non_zero_value 1
#define frame_qp 4
int write_cbr_flag;
mfxFrameSurface1 *surface2;
mfxBitstream *bits;
mfxSyncPoint syncp;
/* end of internal stuff */

static void appnd_f1()
{
/*beg1*/
mfxExtCodingOption option, *option_array;

/* configure mfxExtCodingOption */
memset(&option,0,sizeof(option));
option.Header.BufferId         = MFX_EXTBUFF_CODING_OPTION;
option.Header.BufferSz         = sizeof(option);
option.VuiNalHrdParameters     = MFX_CODINGOPTION_ON;
option.NalHrdConformance       = MFX_CODINGOPTION_OFF;

/* configure mfxVideoParam */
mfxVideoParam param;

// ...

param.mfx.RateControlMethod         = MFX_RATECONTROL_CQP;
param.mfx.FrameInfo.FrameRateExtN   = valid_non_zero_value;
param.mfx.FrameInfo.FrameRateExtD   = valid_non_zero_value;
param.mfx.BufferSizeInKB            = valid_non_zero_value;
param.mfx.InitialDelayInKB          = valid_non_zero_value;
param.mfx.TargetKbps                = valid_non_zero_value;

if (write_cbr_flag == 1)
   param.mfx.MaxKbps = 0;
else /* write_cbr_flag = 0 */
   param.mfx.MaxKbps = valid_non_zero_value;

param.NumExtParam = 1;
option_array     = &option;
param.ExtParam     = (mfxExtBuffer **)&option_array;

/* encoder initialization */
mfxStatus sts;
sts = MFXVideoENCODE_Init(session, &param);

// ...

/* encoding */
mfxEncodeCtrl ctrl;
memset(&ctrl,0,sizeof(ctrl));
ctrl.QP = frame_qp;

sts=MFXVideoENCODE_EncodeFrameAsync(session,&ctrl,surface2,bits,&syncp);
/*end1*/
}
