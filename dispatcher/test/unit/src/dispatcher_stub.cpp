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

// mfxExtendedDeviceId tests
TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_VendorID_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_VendorID_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_VendorID_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_VendorID_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_DeviceID_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_DeviceID_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_DeviceID_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_DeviceID_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_PCIDomain_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_PCIDomain_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_PCIDomain_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_PCIDomain_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_PCIBus_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_PCIBus_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_PCIBus_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_PCIBus_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_PCIDevice_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_PCIDevice_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_PCIDevice_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_PCIDevice_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_PCIFunction_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_PCIFunction_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_PCIFunction_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_PCIFunction_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_DeviceLUID_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_DeviceLUID_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_DeviceLUID_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_DeviceLUID_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_LUIDDeviceNodeMask_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_LUIDDeviceNodeMask_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_LUIDDeviceNodeMask_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_LUIDDeviceNodeMask_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_DRMRenderNodeNum_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_DRMRenderNodeNum_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_DRMRenderNodeNum_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_DRMRenderNodeNum_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_DRMPrimaryNodeNum_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_DRMPrimaryNodeNum_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_DRMPrimaryNodeNum_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_DRMPrimaryNodeNum_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_DeviceName_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_DeviceName_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_DeviceName_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_DeviceName_Invalid(MFX_IMPL_TYPE_STUB);
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

// test 1.x initialization path (MFXInitEx)
TEST(Dispatcher_Stub_CreateSession, LegacyRuntimeParsesExtBuf) {
    SKIP_IF_DISP_STUB_DISABLED();

    // stub RT logs results from parsing extBuf
    CaptureRuntimeLog();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB_1X);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - NumThread
    SetConfigFilterProperty<mfxU32>(loader, "NumThread", 5);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check for RT log string which indicates that extBuf was parsed properly
    CheckRuntimeLog("[STUB RT]: message -- MFXInitEx -- extBuf enabled -- NumThread (5)");

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_EnumImpl_ValidStub) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxExtendedDeviceId *idescDevice;

    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_DEVICE_ID_EXTENDED,
                                 reinterpret_cast<mfxHDL *>(&idescDevice));

    // expect EnumImplementations to succeed for 2.x stub runtime, return valid descriptor
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(idescDevice, nullptr);
    EXPECT_EQ(idescDevice->DeviceID, 0x1595);

    // free internal resources
    MFXDispReleaseImplDescription(loader, idescDevice);
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_EnumImpl_InvalidStub) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB_1X);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxExtendedDeviceId *idescDevice;

    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_DEVICE_ID_EXTENDED,
                                 reinterpret_cast<mfxHDL *>(&idescDevice));

    // expect EnumImplementations to fail for 1.x stub runtime
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    // free internal resources
    MFXDispReleaseImplDescription(loader, idescDevice);
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_CloneSession, Basic_Clone_Succeeds) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSession cloneSession = nullptr;
    sts                     = MFXCloneSession(session, &cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // disjoin the child (cloned) session
    sts = MFXDisjoinSession(cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}
