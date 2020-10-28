/*###########################################################################
  # Copyright (C) 2005-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#ifndef __SAMPLE_VPP_PTS_H
#define __SAMPLE_VPP_PTS_H

#include <list>
#include <memory>
#include "vpl/mfxvideo.h"

#include "sample_vpp_frc.h"
#include "sample_vpp_frc_adv.h"

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

/* ************************************************************************* */
class PTSMaker {
public:
    PTSMaker();
    virtual ~PTSMaker(){};
    // isAdvancedMode - enable/disable FRC checking based on PTS
    mfxStatus Init(mfxVideoParam *par,
                   mfxU32 asyncDeep,
                   bool isAdvancedMode    = false,
                   bool isFrameCorrespond = false);
    // need to set current time stamp for input surface
    bool SetPTS(mfxFrameSurface1 *pSurface);
    // need to check
    bool CheckPTS(mfxFrameSurface1 *pSurface);
    // sometimes need to pts jumping
    void JumpPTS();

protected:
    void PrintDumpInfo();

    // FRC based on Init parameters
    bool CheckBasicPTS(mfxFrameSurface1 *pSurface);
    // FRC based on pts
    bool CheckAdvancedPTS(mfxFrameSurface1 *pSurface);

    std::unique_ptr<BaseFRCChecker> m_pFRCChecker;

    mfxU32 m_FRateExtN_In;
    mfxU32 m_FRateExtD_In;
    mfxU32 m_FRateExtN_Out;
    mfxU32 m_FRateExtD_Out;

    // we can offset initial time stamp
    mfxF64 m_TimeOffset;

    // starting value for random offset
    mfxF64 m_CurrTime;

    // maximum difference from reference
    mfxF64 m_MaxDiff;
    mfxF64 m_CurrDiff;

    // frame counter for input frames
    mfxU32 m_NumFrame_In;
    // frame counter for output frames
    mfxU32 m_NumFrame_Out;

    bool m_IsJump;

    // FRC based on PTS mode
    bool m_bIsAdvancedMode;

    std::list<mfxU64> m_ptsList;
};

#endif /* __SAMPLE_VPP_PTS_H*/
