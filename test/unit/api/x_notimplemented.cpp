/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

// These optional functions for encode, decode, and VPP are not implemented
// in the CPU reference implementation

// MFXJoinSession not implemented
TEST(JoinSession, AlwaysReturnsNotImplemented) {
    mfxSession session1, session2;
    mfxVersion ver = {};
    mfxStatus sts  = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session1);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session2);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXJoinSession(session1, session2);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// MFXDisjoinSession not implemented
TEST(DisjoinSession, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXDisjoinSession(session);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// MFXCloneSession not implemented
TEST(CloneSession, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session, session2;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXCloneSession(session, &session2);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// MFXSetPriority not implemented
TEST(SetPriority, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXSetPriority(session, MFX_PRIORITY_LOW);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// MFXGetPriority not implemented
TEST(GetPriority, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXGetPriority(session, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(GetEncodeStat, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoENCODE_GetEncodeStat(session, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(GetDecodeStat, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoDECODE_GetDecodeStat(session, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(GetVPPStat, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoVPP_GetVPPStat(session, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeSetSkipMode, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxSkipMode M = MFX_SKIPMODE_NOSKIP;
    sts           = MFXVideoDECODE_SetSkipMode(session, M);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeGetPayload, AlwaysReturnsNotImplemented) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoDECODE_GetPayload(session, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NOT_IMPLEMENTED);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}
