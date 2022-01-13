/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "src/dispatcher_common.h"

void Dispatcher_CreateSession_SimpleConfigCanCreateSession(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation of implType
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXUnload(loader);
}
