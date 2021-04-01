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

    virtual mfxStatus GetSurfaceForVPP(mfxFrameSurface1 **surface) {
        return MFXMemory_GetSurfaceForVPP(m_session, surface);
    }

    virtual mfxStatus GetSurfaceForVPPIn(mfxFrameSurface1 **surface) {
        return GetSurfaceForVPP(surface);
    }

    virtual mfxStatus GetSurfaceForVPPOut(mfxFrameSurface1 **surface) {
        return MFXMemory_GetSurfaceForVPPOut(m_session, surface);
    }

    virtual mfxStatus GetSurfaceForEncode(mfxFrameSurface1 **surface) {
        return MFXMemory_GetSurfaceForEncode(m_session, surface);
    }

    virtual mfxStatus GetSurfaceForDecode(mfxFrameSurface1 **surface) {
        return MFXMemory_GetSurfaceForDecode(m_session, surface);
    }

protected:
    mfxSession m_session; // (mfxSession) handle to the owning session
};

class MFXVideoSession2 : public MFXVideoSession {
public:
    mfxSession *getSessionPtr() {
        return &m_session;
    }
};

class MFXVideoDECODE_VPP {
public:
    explicit MFXVideoDECODE_VPP(mfxSession session) {
        m_session = session;
    }
    virtual ~MFXVideoDECODE_VPP(void) {}

    virtual mfxStatus Init(mfxVideoParam *decode_par,
                           mfxVideoChannelParam **vpp_par_array,
                           mfxU32 num_vpp_par) {
        return MFXVideoDECODE_VPP_Init(m_session, decode_par, vpp_par_array, num_vpp_par);
    }

    virtual mfxStatus DecodeFrameAsync(mfxBitstream *bs,
                                       mfxU32 *skip_channels,
                                       mfxU32 num_skip_channels,
                                       mfxSurfaceArray **surf_array_out) {
        return MFXVideoDECODE_VPP_DecodeFrameAsync(m_session,
                                                   bs,
                                                   skip_channels,
                                                   num_skip_channels,
                                                   surf_array_out);
    }

    virtual mfxStatus Reset(mfxVideoParam *decode_par,
                            mfxVideoChannelParam **vpp_par_array,
                            mfxU32 num_vpp_par) {
        return MFXVideoDECODE_VPP_Reset(m_session, decode_par, vpp_par_array, num_vpp_par);
    }

    virtual mfxStatus GetChannelParam(mfxVideoChannelParam *par, mfxU32 channel_id) {
        return MFXVideoDECODE_VPP_GetChannelParam(m_session, par, channel_id);
    }

protected:
    mfxSession m_session; // (mfxSession) handle to the owning session
};

#endif

#endif //__SAMPLE_VPL_COMMON_H__
