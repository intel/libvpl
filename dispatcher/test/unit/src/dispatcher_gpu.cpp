/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "src/dispatcher_common.h"

TEST(Dispatcher_GPU_CreateSession, SimpleConfigCanCreateSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_SimpleConfigCanCreateSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, SetValidNumThreadCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_SetValidNumThreadCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, SetInvalidNumThreadTypeReturnsErrUnsupported) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_SetInvalidNumThreadTypeReturnsErrUnsupported(MFX_IMPL_TYPE_HARDWARE);
}

// fully-implemented test cases (not using common kernels)
TEST(Dispatcher_GPU_CreateSession, ExtDeviceID_ValidProps) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_HARDWARE);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // set valid, basic properties
    SetConfigFilterProperty<mfxU16>(loader, "mfxExtendedDeviceId.VendorID", 0x8086);
#if defined(_WIN32) || defined(_WIN64)
    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.LUIDDeviceNodeMask", 1);
#else
    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.DRMRenderNodeNum", 128);
    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.DRMPrimaryNodeNum", 0);
#endif
    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

TEST(Dispatcher_GPU_CreateSession, ExtDeviceID_InvalidProps) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_HARDWARE);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // set valid, basic properties
    SetConfigFilterProperty<mfxU16>(loader, "mfxExtendedDeviceId.VendorID", 0x8086);
#if defined(_WIN32) || defined(_WIN64)
    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.LUIDDeviceNodeMask", 333);
#else
    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.DRMRenderNodeNum", 999);
    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.DRMPrimaryNodeNum", 555);
#endif
    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}
