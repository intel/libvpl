/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "src/dispatcher_common.h"

TEST(Dispatcher_Stub_CreateSession, SimpleConfigCanCreateSession) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SimpleConfigCanCreateSession(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, SetValidNumThreadCreatesSession) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SetValidNumThreadCreatesSession(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, SetInvalidNumThreadTypeReturnsErrUnsupported) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SetInvalidNumThreadTypeReturnsErrUnsupported(MFX_IMPL_TYPE_STUB);
}

// fully-implemented test cases (not using common kernels)
TEST(Dispatcher_Stub_CreateSession, RuntimeParsesExtBuf) {
    SKIP_IF_DISP_STUB_DISABLED();

    // stub RT logs results from parsing extBuf
    CaptureRuntimeLog();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - NumThread
    SetConfigFilterProperty<mfxU32>(loader, "NumThread", 4);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check for RT log string which indicates that extBuf was parsed properly
    CheckRuntimeLog("[STUB RT]: message -- MFXInitialize -- extBuf enabled -- NumThread (4)");

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}
