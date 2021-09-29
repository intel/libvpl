/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

///
/// Example of a minimal oneAPI Video Processing Library (oneVPL) application.
///
/// @file

#include <gtest/gtest.h>

#include "vpl/mfxvideo++.h"

TEST(LegacycppCreateSession, InitSucceeds) {
    mfxIMPL impl   = MFX_IMPL_AUTO_ANY;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

TEST(LegacycppCreateSession, DoubleInitSucceeds) {
    mfxIMPL impl   = MFX_IMPL_AUTO_ANY;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
    sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

TEST(LegacycppCreateSession, CloseSucceeds) {
    mfxIMPL impl   = MFX_IMPL_AUTO_ANY;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
    sts = session.Close();
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Close failed with code " << sts;
}

TEST(LegacycppCreateSession, DoubleCloseSucceeds) {
    mfxIMPL impl   = MFX_IMPL_AUTO_ANY;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
    sts = session.Close();
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Close failed with code " << sts;
    sts = session.Close();
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Close failed with code " << sts;
}

TEST(LegacycppCreateSession, InitSucceedsImplAuto) {
    mfxIMPL impl   = MFX_IMPL_AUTO;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

TEST(LegacycppCreateSession, InitSucceedsImplSoftware) {
    mfxIMPL impl   = MFX_IMPL_SOFTWARE;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

TEST(LegacycppCreateSession, InitSucceedsImplHardware) {
    mfxIMPL impl   = MFX_IMPL_HARDWARE;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

TEST(LegacycppCreateSession, InitSucceedsImplHardwareAny) {
    mfxIMPL impl   = MFX_IMPL_HARDWARE_ANY;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

//Tests for HARDWARE2,3,4 require specific hardware setup and are not suitable for
//general unit tests which can be run anywhere

TEST(LegacycppCreateSession, InitFailsImplInvalid) {
    mfxIMPL impl   = MFX_IMPL_RUNTIME;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_INVALID_VIDEO_PARAM) << "mfxvideo++ Init failed with code " << sts;
}
