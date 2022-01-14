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

void Dispatcher_CreateSession_SetValidNumThreadCreatesSession(mfxImplType implType) {
    // dispatcher logs extBufs that are passed to MFXInitialize
    CaptureDispatcherLog();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - NumThread
    SetConfigFilterProperty<mfxU32>(loader, "NumThread", 2);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check for dispatcher log string which indicates that extBuf was set properly
    CheckDispatcherLog("message:  extBuf enabled -- NumThread (2)");

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_SetInvalidNumThreadTypeReturnsErrUnsupported(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    // use wrong variant type
    mfxVariant ImplValue;
    ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
    ImplValue.Type            = MFX_VARIANT_TYPE_U16;
    ImplValue.Data.U16        = 2;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"NumThread", ImplValue);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    // free internal resources
    MFXUnload(loader);
}
