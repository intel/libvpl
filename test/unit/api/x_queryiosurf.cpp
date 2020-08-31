/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

/* QueryIOSurf Overview

   For legacy external allocation.  Returns minimum and suggested numbers
       for encode init
       for decode output
       for vpp input and output
   This function does not validate I/O parameters except those used in calculating the number of surfaces.


   Returns:
   MFX_ERR_NONE  The function completed successfully. \n
   MFX_ERR_INVALID_VIDEO_PARAM  The function detected invalid video parameters. These parameters may be out of the valid range, or the combination of them 
                                resulted in incompatibility. Incompatibility not resolved. \n
   MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others; incompatibility resolved.

*/

//EncodeQueryIOSurf
TEST(EncodeQueryIOSurf, PopulatedParamsInReturnsRequest) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));

    mfxEncParams.mfx.CodecId          = MFX_CODEC_HEVC;
    mfxEncParams.mfx.FrameInfo.Width  = 128;
    mfxEncParams.mfx.FrameInfo.Height = 96;

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoENCODE_Query(session, &mfxEncParams, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest request;
    sts = MFXVideoENCODE_QueryIOSurf(session, &par, &request);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    ASSERT_GE(request.NumFrameSuggested, 1);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(EncodeQueryIOSurf, DISABLED_InvalidParamsReturnInvalidVideoParam) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    par.mfx.CodecId          = MFX_CODEC_HEVC;
    par.mfx.FrameInfo.Width  = 128;
    par.mfx.FrameInfo.Height = 96;
    par.IOPattern            = MFX_IOPATTERN_IN_VIDEO_MEMORY;

    mfxFrameAllocRequest request;
    sts = MFXVideoENCODE_QueryIOSurf(session, &par, &request);
    ASSERT_EQ(sts, MFX_ERR_INVALID_VIDEO_PARAM);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(EncodeQueryIOSurf,
     DISABLED_IncompatibleParamsReturnIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(EncodeQueryIOSurf, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXVideoENCODE_QueryIOSurf(0, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(EncodeQueryIOSurf, NullParamsInReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest R;
    sts = MFXVideoENCODE_QueryIOSurf(session, nullptr, &R);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//DecodeQueryIOSurf
TEST(DecodeQueryIOSurf, PopulatedParamsInReturnsRequest) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));

    mfxEncParams.mfx.CodecId          = MFX_CODEC_HEVC;
    mfxEncParams.mfx.FrameInfo.Width  = 128;
    mfxEncParams.mfx.FrameInfo.Height = 96;

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoDECODE_Query(session, &mfxEncParams, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest request;
    sts = MFXVideoDECODE_QueryIOSurf(session, &par, &request);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    ASSERT_GE(request.NumFrameSuggested, 1);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeQueryIOSurf, DISABLED_InvalidParamsReturnInvalidVideoParam) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    par.mfx.CodecId          = MFX_CODEC_HEVC;
    par.mfx.FrameInfo.Width  = 128;
    par.mfx.FrameInfo.Height = 96;
    par.IOPattern            = MFX_IOPATTERN_OUT_VIDEO_MEMORY;

    mfxFrameAllocRequest request;
    sts = MFXVideoDECODE_QueryIOSurf(session, &par, &request);
    ASSERT_EQ(sts, MFX_ERR_INVALID_VIDEO_PARAM);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeQueryIOSurf,
     DISABLED_IncompatibleParamsReturnIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(DecodeQueryIOSurf, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXVideoDECODE_QueryIOSurf(0, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(DecodeQueryIOSurf, NullParamsInReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest R;
    sts = MFXVideoDECODE_QueryIOSurf(session, nullptr, &R);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//VPPQueryIOSurf
TEST(VPPQueryIOSurf, DISABLED_PopulatedParamsInReturnsRequest) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxVPPParams;
    memset(&mfxVPPParams, 0, sizeof(mfxVPPParams));

    mfxVPPParams.mfx.CodecId          = MFX_CODEC_HEVC;
    mfxVPPParams.mfx.FrameInfo.Width  = 128;
    mfxVPPParams.mfx.FrameInfo.Height = 96;

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoVPP_Query(session, &mfxVPPParams, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest request;
    sts = MFXVideoVPP_QueryIOSurf(session, &par, &request);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    ASSERT_GE(request.NumFrameSuggested, 1);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(VPPQueryIOSurf, DISABLED_InvalidParamsReturnInvalidVideoParam) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxVPPParams;
    memset(&mfxVPPParams, 0, sizeof(mfxVPPParams));
    mfxVPPParams.vpp.In.Width  = 128;
    mfxVPPParams.vpp.In.Height = 96;
    mfxVPPParams.vpp.In.FourCC = MFX_FOURCC_I420;
    mfxVPPParams.vpp.Out       = mfxVPPParams.vpp.In;
    mfxVPPParams.IOPattern =
        MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;

    mfxFrameAllocRequest request;
    sts = MFXVideoVPP_QueryIOSurf(session, &mfxVPPParams, &request);
    ASSERT_EQ(sts, MFX_ERR_INVALID_VIDEO_PARAM);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(VPPQueryIOSurf, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXVideoVPP_QueryIOSurf(0, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(VPPQueryIOSurf, NullParamsInReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocRequest R;
    sts = MFXVideoVPP_QueryIOSurf(session, nullptr, &R);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}
