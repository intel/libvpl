/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

//SetFrameAllocator
TEST(SetFrameAllocator, SetFrameAllocatorReturnsErrNone) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameAllocator allocator{};
    allocator.pthis                              = reinterpret_cast<void*>(1);
    *reinterpret_cast<void**>(&allocator.Lock)   = reinterpret_cast<void*>(2);
    *reinterpret_cast<void**>(&allocator.Unlock) = reinterpret_cast<void*>(3);
    sts = MFXVideoCORE_SetFrameAllocator(session, &allocator);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//SetHandle
TEST(SetHandle, HandleInReturnsErrNone) {
    // Initialize the session.
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoCORE_SetHandle(session, MFX_HANDLE_VA_DISPLAY, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//  null session
TEST(SetHandle, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXVideoCORE_SetHandle(0, MFX_HANDLE_VA_DISPLAY, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

//  no redefine
TEST(SetHandle, RedefineReturnsUndefinedBehavior) {
    // Initialize the session.
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoCORE_SetHandle(session, MFX_HANDLE_VA_DISPLAY, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoCORE_SetHandle(session, MFX_HANDLE_VA_DISPLAY, nullptr);
    ASSERT_EQ(sts, MFX_ERR_UNDEFINED_BEHAVIOR);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

//GetHandle
// HandleExistsReturnHandle
TEST(SetHandle, HandleExistsReturnHandle) {
    // Initialize the session.
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoCORE_SetHandle(session, MFX_HANDLE_VA_DISPLAY, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxHDL hdl;
    sts = MFXVideoCORE_GetHandle(session, MFX_HANDLE_VA_DISPLAY, &hdl);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// null session
TEST(GetHandle, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXVideoCORE_GetHandle(0, MFX_HANDLE_VA_DISPLAY, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

//Sync
// success
TEST(SyncOperation, ValidInputReturnsErrNone) {
    // Initialize the session.
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxSyncPoint syncp = { 0 };
    sts                = MFXVideoCORE_SyncOperation(session, syncp, 1000);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    //free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// null session
TEST(SyncOperation, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXVideoCORE_SyncOperation(0, 0, 0);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}