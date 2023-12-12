/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

// Example using Intel® Video Processing Library (Intel® VPL)
#include <vector>

#include <stdlib.h>

#include "mfxdefs.h"
#include "mfxstructures.h"
#include "mfxsession.h"
#include "mfxvideo.h"

/* These macro required for code compilation. */
#define UNUSED_PARAM(x) (void)(x)

static mfxSession session;
static mfxStatus sts;
static std::vector<mfxExtBuffer *>extBufVector;

static void prg_config1() {
/*beg1*/

// call MFXVideoCORE_GetHandle to obtain mfxConfigInterface
mfxConfigInterface *iface = nullptr;
sts = MFXVideoCORE_GetHandle(session, MFX_HANDLE_CONFIG_INTERFACE, (mfxHDL *)(&iface));

// Alternately, we could use the following alias:
// sts = MFXGetConfigInterface(session, &iface);

// pass string parameter which maps to mfxVideoParam
mfxVideoParam par   = {};
mfxExtBuffer extBuf = {};
sts = iface->SetParameter(iface, (mfxU8 *)"TargetKbps", (mfxU8 *)"1650", MFX_STRUCTURE_TYPE_VIDEO_PARAM, &par, &extBuf);

/*end1*/
}

static mfxStatus prg_config2 () {
/*beg2*/

// call MFXGetConfigInterface() to obtain mfxConfigInterface
mfxConfigInterface *iface = nullptr;
sts = MFXGetConfigInterface(session, &iface);

// pass string parameter which maps to an Intel® VPL extension buffer
mfxVideoParam par   = {};
mfxExtBuffer extBuf = {};
sts = iface->SetParameter(iface, (mfxU8 *)"mfxExtHEVCParam.PicWidthInLumaSamples", (mfxU8 *)"640", MFX_STRUCTURE_TYPE_VIDEO_PARAM, &par, &extBuf);

// if extension buffer has not already been attached, allocate it and call again
if (sts == MFX_ERR_MORE_EXTBUFFER) {
    // the first call to SetParameter filled in extBuf with the buffer ID and size to allocate
    mfxExtBuffer *extBufNew = (mfxExtBuffer *)calloc(extBuf.BufferSz, 1);
    if (!extBufNew)
        return MFX_ERR_MEMORY_ALLOC;

    extBufNew->BufferId = extBuf.BufferId;
    extBufNew->BufferSz = extBuf.BufferSz;

    extBufVector.push_back(extBufNew);
    par.NumExtParam = static_cast<mfxU16>(extBufVector.size());
    par.ExtParam    = extBufVector.data();

    // the correct extension buffer is now attached, so the call should succeed this time
    sts = iface->SetParameter(iface, (mfxU8 *)"mfxExtHEVCParam.PicWidthInLumaSamples", (mfxU8 *)"640", MFX_STRUCTURE_TYPE_VIDEO_PARAM, &par, &extBuf);

    if (sts != MFX_ERR_NONE)
        return sts;
    }

return MFX_ERR_NONE;
/*end2*/
}
