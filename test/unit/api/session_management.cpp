/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include <tuple>
#include "vpl/mfxvideo.h"

// MFXInit tests
TEST(Init, AutoImplReturnsErrNone) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_AUTO, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Init, SoftwareImplReturnsErrNone) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// MFXInitEx tests
TEST(InitEx, ValidInputReturnsErrNone) {
    mfxInitParam initPar   = { 0 };
    initPar.Version.Major  = 2;
    initPar.Version.Minor  = 0;
    initPar.Implementation = MFX_IMPL_SOFTWARE;

    // Initialize the session.
    mfxSession session;
    mfxStatus sts = MFXInitEx(initPar, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// MFXClose tests

TEST(Close, InitializedSessionReturnsErrNone) {
    mfxInitParam initPar   = { 0 };
    initPar.Version.Major  = 2;
    initPar.Version.Minor  = 0;
    initPar.Implementation = MFX_IMPL_SOFTWARE;

    // Initialize the session.
    mfxSession session;
    mfxStatus sts = MFXInitEx(initPar, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(session);
    ASSERT_EQ(sts, MFX_ERR_NONE);
}

TEST(Close, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXClose(nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

//MFXQueryIMPL
TEST(QueryIMPL, SessionImplForSWReturnsSW) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxIMPL impl;
    sts = MFXQueryIMPL(session, &impl);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    ASSERT_EQ(impl, MFX_IMPL_SOFTWARE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(QueryIMPL, NullImplInReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXQueryIMPL(session, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(QueryIMPL, NullSessionReturnsInvalidHandle) {
    mfxIMPL impl;
    mfxStatus sts = MFXQueryIMPL(nullptr, &impl);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

//MFXQueryVersion
TEST(QueryVersion, InitializedSessionReturnsCorrectVersion) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVersion version;
    sts = MFXQueryVersion(session, &version);
    ASSERT_EQ(sts, MFX_ERR_NONE);
    ASSERT_EQ(version.Major, MFX_VERSION_MAJOR);
    ASSERT_EQ(version.Minor, MFX_VERSION_MINOR);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(QueryVersion, NullVersionInReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXQueryVersion(session, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(QueryVersion, NullSessionReturnsInvalidHandle) {
    mfxVersion ver;
    mfxStatus sts = MFXQueryVersion(nullptr, &ver);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}
