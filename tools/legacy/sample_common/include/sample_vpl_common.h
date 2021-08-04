/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __SAMPLE_VPL_COMMON_H__
#define __SAMPLE_VPL_COMMON_H__

#include "vpl/mfxstructures.h"
#include "vpl/mfxvideo.h"

#if (MFX_VERSION >= 2000)
class MFXMemory {
public:
    explicit MFXMemory(mfxSession session) {
        m_session = session;
    }
    virtual ~MFXMemory(void) {}

    virtual mfxStatus GetSurfaceForVPP(mfxFrameSurface1** surface) {
        return MFXMemory_GetSurfaceForVPP(m_session, surface);
    }

    virtual mfxStatus GetSurfaceForVPPIn(mfxFrameSurface1** surface) {
        return GetSurfaceForVPP(surface);
    }

    virtual mfxStatus GetSurfaceForVPPOut(mfxFrameSurface1** surface) {
        return MFXMemory_GetSurfaceForVPPOut(m_session, surface);
    }

    virtual mfxStatus GetSurfaceForEncode(mfxFrameSurface1** surface) {
        return MFXMemory_GetSurfaceForEncode(m_session, surface);
    }

    virtual mfxStatus GetSurfaceForDecode(mfxFrameSurface1** surface) {
        return MFXMemory_GetSurfaceForDecode(m_session, surface);
    }

protected:
    mfxSession m_session; // (mfxSession) handle to the owning session
};

class MFXVideoSession2 : public MFXVideoSession {
public:
    mfxSession* getSessionPtr() {
        return &m_session;
    }
};

#endif

#endif //__SAMPLE_VPL_COMMON_H__
