/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __MFX_MULTI_VPP_H
#define __MFX_MULTI_VPP_H

#include "vpl/mfxvideo++.h"
// An interface for a pipeline consisting of multiple (maximum 3) VPP-like components. Base implementation - for single VPP.
// The application should use this interface to be able to seamlessly switch from MFXVideoVPP to MFXVideoVPPPlugin in the pipeline.

class MFXVideoMultiVPP : public MFXVideoVPP {
public:
    MFXVideoMultiVPP(mfxSession session) : MFXVideoVPP(session) {}
    virtual ~MFXVideoMultiVPP(void) {
        Close();
    }

    // topology methods
    virtual mfxStatus QueryIOSurf(mfxVideoParam* par, mfxFrameAllocRequest* request) override {
        return QueryIOSurfMulti(par, request);
    }
    virtual mfxStatus QueryIOSurfMulti(mfxVideoParam* par,
                                       mfxFrameAllocRequest request[2],
                                       mfxVideoParam* /*par1*/ = NULL,
                                       mfxVideoParam* /*par2*/ = NULL) {
        return MFXVideoVPP_QueryIOSurf(m_session, par, request);
    }

    virtual mfxStatus Init(mfxVideoParam* par) override {
        return InitMulti(par);
    }
    virtual mfxStatus InitMulti(mfxVideoParam* par,
                                mfxVideoParam* /*par1*/ = NULL,
                                mfxVideoParam* /*par2*/ = NULL) {
        return MFXVideoVPP_Init(m_session, par);
    }

    virtual mfxStatus Reset(mfxVideoParam* par) override {
        return ResetMulti(par);
    }
    virtual mfxStatus ResetMulti(mfxVideoParam* par,
                                 mfxVideoParam* /*par1*/ = NULL,
                                 mfxVideoParam* /*par2*/ = NULL) {
        return MFXVideoVPP_Reset(m_session, par);
    }

    virtual mfxStatus RunFrameVPPAsync(mfxFrameSurface1* in,
                                       mfxFrameSurface1* out,
                                       mfxExtVppAuxData* aux,
                                       mfxSyncPoint* syncp) override {
        return MFXVideoVPP_RunFrameVPPAsync(m_session, in, out, aux, syncp);
    }

    virtual mfxStatus SyncOperation(mfxSyncPoint syncp, mfxU32 wait) {
        return MFXVideoCORE_SyncOperation(m_session, syncp, wait);
    }

    virtual mfxStatus Close(void) override {
        return MFXVideoVPP_Close(m_session);
    }

    // per-component methods
    virtual mfxStatus Query(mfxVideoParam* in, mfxVideoParam* out) override {
        return QueryMulti(in, out);
    }
    virtual mfxStatus QueryMulti(mfxVideoParam* in,
                                 mfxVideoParam* out,
                                 mfxU8 /*component_idx*/ = 0) {
        return MFXVideoVPP_Query(m_session, in, out);
    }

    virtual mfxStatus GetVideoParam(mfxVideoParam* par) override {
        return GetVideoParamMulti(par);
    }
    virtual mfxStatus GetVideoParamMulti(mfxVideoParam* par, mfxU8 /*component_idx*/ = 0) {
        return MFXVideoVPP_GetVideoParam(m_session, par);
    }

    virtual mfxStatus GetVPPStat(mfxVPPStat* stat) override {
        return GetVPPStatMulti(stat);
    }
    virtual mfxStatus GetVPPStatMulti(mfxVPPStat* stat, mfxU8 /*component_idx*/ = 0) {
        return MFXVideoVPP_GetVPPStat(m_session, stat);
    }
};

#endif //__MFX_MULTI_VPP_H
