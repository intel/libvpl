/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

/* Query function overview:
     If the input parameter set is a null pointer: T
     The function returns configurability info in the output parameter set. 
     Parameters set to non zero values are configurable with init.
     For decode and encode, the CodecID must be set in the output parameter set to identify the coding standard.

     If the in parameter set is non-zero, the function checks the validity of the fields in the input structure. 
     Corrected values are returned in the output structure. 
     The function will zero fields if there is insufficient information to determine validity or
     correction is not possible. 

     This feature can verify whether the SDK implementation supports certain profiles, levels or bitrates.
     For decode and encode, the CodecID must be set in the input parameter set to identify the coding standard.

     The application can call this function before or after it initializes the operation.

    Return states:
     MFX_ERR_NONE  The function completed successfully. \n
     MFX_ERR_UNSUPPORTED  The function failed to identify a specific implementation for the required features. \n
     MFX_WRN_INCOMPATIBLE_VIDEO_PARAM  The function detected some video parameters were incompatible with others;
        incompatibility resolved.
*/

//EncodeQuery
TEST(EncodeQuery, NullParamsInReturnsConfigurable) {
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

    sts = MFXVideoENCODE_Query(session, nullptr, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    ASSERT_EQ(0xFFFFFFFF, par.mfx.FrameInfo.FourCC);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(EncodeQuery, PopulatedParamsInReturnsCorrected) {
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
    ASSERT_EQ(128, par.mfx.FrameInfo.Width);
    ASSERT_EQ(96, par.mfx.FrameInfo.Height);
    ASSERT_EQ(MFX_RATECONTROL_VBR, par.mfx.RateControlMethod);
    ASSERT_EQ(MFX_IOPATTERN_IN_SYSTEM_MEMORY, par.IOPattern);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(EncodeQuery, InvalidParamsReturnsInvalid) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));
    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoENCODE_Query(session, &mfxEncParams, &par);
    ASSERT_EQ(sts, MFX_ERR_INVALID_VIDEO_PARAM);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(EncodeQuery, DISABLED_IncompatibleParamsReturnIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(EncodeQuery, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXVideoENCODE_Query(nullptr, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(EncodeQuery, NullParamsOutReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoENCODE_Query(session, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//DecodeQuery
TEST(DecodeQuery, NullParamsInReturnsConfigurable) {
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
    sts = MFXVideoDECODE_Query(session, nullptr, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    ASSERT_EQ(0xFFFFFFFF, par.mfx.FrameInfo.FourCC);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeQuery, PopulatedParamsInReturnsCorrected) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxDecParams;
    memset(&mfxDecParams, 0, sizeof(mfxDecParams));
    mfxDecParams.mfx.CodecId          = MFX_CODEC_HEVC;
    mfxDecParams.mfx.FrameInfo.Width  = 128;
    mfxDecParams.mfx.FrameInfo.Height = 96;

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoDECODE_Query(session, &mfxDecParams, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    ASSERT_EQ(128, par.mfx.FrameInfo.Width);
    ASSERT_EQ(96, par.mfx.FrameInfo.Height);
    ASSERT_EQ(MFX_IOPATTERN_OUT_SYSTEM_MEMORY, par.IOPattern);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeQuery, InvalidParamsReturnsInvalid) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxDecParams;
    memset(&mfxDecParams, 0, sizeof(mfxDecParams));

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoDECODE_Query(session, &mfxDecParams, &par);
    ASSERT_EQ(sts, MFX_ERR_INVALID_VIDEO_PARAM);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeQuery, DISABLED_IncompatibleParamsReturnIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(DecodeQuery, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXVideoDECODE_Query(nullptr, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(DecodeQuery, NullParamsOutReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoDECODE_Query(session, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//VPPQuery
TEST(VPPQuery, NullParamsInReturnsConfigurable) {
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
    sts = MFXVideoVPP_Query(session, nullptr, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    ASSERT_EQ(0xFFFFFFFF, par.mfx.FrameInfo.FourCC);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(VPPQuery, PopulatedParamsInReturnsCorrected) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxVPPParams;
    memset(&mfxVPPParams, 0, sizeof(mfxVPPParams));
    mfxVPPParams.vpp.In.Width  = 128;
    mfxVPPParams.vpp.In.Height = 96;

    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoVPP_Query(session, &mfxVPPParams, &par);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    ASSERT_EQ(128, par.vpp.Out.Width);
    ASSERT_EQ(96, par.vpp.Out.Height);
    ASSERT_EQ(MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY,
              par.IOPattern);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(VPPQuery, DISABLED_InvalidParamsReturnsInvalid) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxVPPParams;
    memset(&mfxVPPParams, 0, sizeof(mfxVPPParams));
    mfxVPPParams.vpp.In.FourCC = MFX_FOURCC_NV12;
    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = MFXVideoVPP_Query(session, &mfxVPPParams, &par);
    ASSERT_EQ(sts, MFX_ERR_INVALID_VIDEO_PARAM);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(VPPQuery, DISABLED_IncompatibleParamsReturnIncompatibleVideoParam) {
    FAIL() << "Test not implemented";
}

TEST(VPPQuery, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXVideoVPP_Query(nullptr, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(VPPQuery, NullParamsOutReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoVPP_Query(session, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}
