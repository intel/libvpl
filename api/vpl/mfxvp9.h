/*############################################################################
  # Copyright (C) 2018-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __MFXVP9_H__
#define __MFXVP9_H__

#include "mfxdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (MFX_VERSION >= MFX_VERSION_NEXT)

/* Extended Buffer Ids */
enum {
    MFX_EXTBUFF_VP9_DECODED_FRAME_INFO = MFX_MAKEFOURCC('9','D','F','I')
};

MFX_PACK_BEGIN_USUAL_STRUCT()
typedef struct {
    mfxExtBuffer Header;

    mfxU16       DisplayWidth;
    mfxU16       DisplayHeight;
    mfxU16       reserved[58];
} mfxExtVP9DecodedFrameInfo;
MFX_PACK_END()

#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif

