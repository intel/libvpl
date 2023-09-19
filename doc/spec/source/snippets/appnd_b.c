/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
  
#include "mfxdefs.h"
#include "mfxvideo.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* These macro required for code compilation. */
#define sps_buffer NULL
#define sps_buffer_length 0
#define pps_buffer NULL
#define pps_buffer_length 0
mfxSession session;
/* end of internal stuff */

/*beg*/
mfxStatus init_encoder() {
   mfxExtCodingOptionSPSPPS option, *option_array;

   /* configure mfxExtCodingOptionSPSPPS */
   memset(&option,0,sizeof(option));
   option.Header.BufferId=MFX_EXTBUFF_CODING_OPTION_SPSPPS;
   option.Header.BufferSz=sizeof(option);
   option.SPSBuffer=sps_buffer;
   option.SPSBufSize=sps_buffer_length;
   option.PPSBuffer=pps_buffer;
   option.PPSBufSize=pps_buffer_length;

   /* configure mfxVideoParam */
   mfxVideoParam param;
   //...
   param.NumExtParam=1;
   option_array=&option;
   param.ExtParam=(mfxExtBuffer**)&option_array;

   /* encoder initialization */
   mfxStatus status;
   status=MFXVideoENCODE_Init(session, &param);
   if (status==MFX_ERR_INCOMPATIBLE_VIDEO_PARAM) {
      printf("Initialization failed.\n");
   } else {
      printf("Initialized.\n");
   }
   return status;
}
/*end*/
