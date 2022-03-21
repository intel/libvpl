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

#include "src/unit_api.h"

#include "vpl/mfxvideo++.h"

TEST(LegacycppCreateSession, InitSucceeds) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxIMPL impl   = MFX_IMPL_AUTO_ANY;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

TEST(LegacycppCreateSession, DoubleInitSucceeds) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxIMPL impl   = MFX_IMPL_AUTO_ANY;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
    sts = session.Close(); // avoid memleak
    sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

TEST(LegacycppCreateSession, CloseSucceeds) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxIMPL impl   = MFX_IMPL_AUTO_ANY;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
    sts = session.Close();
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Close failed with code " << sts;
}

// update when mfxvideo++.h switches to MFXLoad() instead of MFXInitEx()
TEST(DISABLED_LegacycppCreateSession, DoubleCloseSucceeds) {
    SKIP_IF_DISP_GPU_DISABLED();

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
    SKIP_IF_DISP_GPU_DISABLED();

    mfxIMPL impl   = MFX_IMPL_AUTO;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

// update when mfxvideo++.h switches to MFXLoad() instead of MFXInitEx()
TEST(DISABLED_LegacycppCreateSession, InitSucceedsImplSoftware) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxIMPL impl   = MFX_IMPL_SOFTWARE;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

TEST(LegacycppCreateSession, InitSucceedsImplHardware) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxIMPL impl   = MFX_IMPL_HARDWARE;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

TEST(LegacycppCreateSession, InitSucceedsImplHardwareAny) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxIMPL impl   = MFX_IMPL_HARDWARE_ANY;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

//Tests for HARDWARE2,3,4 require specific hardware setup and are not suitable for
//general unit tests which can be run anywhere

// update when mfxvideo++.h switches to MFXLoad() instead of MFXInitEx()
TEST(DISABLED_LegacycppCreateSession, InitFailsImplInvalid) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxIMPL impl   = MFX_IMPL_RUNTIME;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_INVALID_VIDEO_PARAM) << "mfxvideo++ Init failed with code " << sts;
}
