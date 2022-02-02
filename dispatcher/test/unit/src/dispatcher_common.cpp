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

// currently the ExtDeviceID functions are only supported for implType == MFX_IMPL_TYPE_STUB
// to use with other runtimes, adjust the values of the filter properties accordingly (based on implType)
void Dispatcher_CreateSession_ExtDeviceID_VendorID_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU16>(loader, "mfxExtendedDeviceId.VendorID", 0x8086);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_VendorID_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU16>(loader, "mfxExtendedDeviceId.VendorID", 0x9999);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_DeviceID_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU16>(loader, "mfxExtendedDeviceId.DeviceID", 0x1595);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_DeviceID_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU16>(loader, "mfxExtendedDeviceId.DeviceID", 0x88AA);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_PCIDomain_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.PCIDomain", 1);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_PCIDomain_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.PCIDomain", 2);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_PCIBus_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.PCIBus", 3);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_PCIBus_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.PCIBus", 4);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_PCIDevice_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.PCIDevice", 7);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_PCIDevice_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.PCIDevice", 8);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_PCIFunction_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.PCIFunction", 9);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_PCIFunction_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.PCIFunction", 99);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_DeviceLUID_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxU8 validLUID[8] = { 0x4f, 0x59, 0x2e, 0xa1, 0x33, 0x78, 0xdb, 0x29 };

    SetConfigFilterProperty<mfxHDL>(loader, "mfxExtendedDeviceId.DeviceLUID", &validLUID);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_DeviceLUID_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // change one byte from the valid LUID - see config.cpp for vplstubrt
    mfxU8 invalidLUID[8] = { 0x4f, 0x59, 0x2e, 0xa1, 0x34, 0x78, 0xdb, 0x29 };

    SetConfigFilterProperty<mfxHDL>(loader, "mfxExtendedDeviceId.DeviceLUID", &invalidLUID);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_LUIDDeviceNodeMask_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.LUIDDeviceNodeMask", 0x0000AEAE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_LUIDDeviceNodeMask_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.LUIDDeviceNodeMask", 0XEEFF);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_DRMRenderNodeNum_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.DRMRenderNodeNum", 130);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_DRMRenderNodeNum_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.DRMRenderNodeNum", 131);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_DRMPrimaryNodeNum_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.DRMPrimaryNodeNum", 2);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_DRMPrimaryNodeNum_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.DRMPrimaryNodeNum", 1);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_DeviceName_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxHDL>(loader,
                                    "mfxExtendedDeviceId.DeviceName",
                                    const_cast<char *>("stub-extDev"));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_DeviceName_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxHDL>(loader,
                                    "mfxExtendedDeviceId.DeviceName",
                                    const_cast<char *>("stub-invalid"));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}
