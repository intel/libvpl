/*############################################################################
  # Copyright (C) Intel Corporation
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
    CaptureOutputLog(CAPTURE_LOG_DISPATCHER);

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

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);

    // check for dispatcher log string which indicates that extBuf was set properly
    CheckOutputLog("message:  extBuf enabled -- NumThread (2)");
    CleanupOutputLog();
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

void Dispatcher_CreateSession_ExtDeviceID_RevisionID_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU16>(loader, "mfxExtendedDeviceId.RevisionID", 0x1034);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ExtDeviceID_RevisionID_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU16>(loader, "mfxExtendedDeviceId.RevisionID", 0XBCBC);

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

#ifdef ONEVPL_EXPERIMENTAL

// mfxSurfaceTypesSupported tests
void Dispatcher_CreateSession_SurfaceSupport_SurfaceType_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSurfaceType surfaceTypeValid = MFX_SURFACE_TYPE_UNKNOWN;
    #if defined(_WIN32) || defined(_WIN64)
    surfaceTypeValid = MFX_SURFACE_TYPE_D3D11_TEX2D;
    #else
    surfaceTypeValid   = MFX_SURFACE_TYPE_VAAPI;
    #endif

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxSurfaceTypesSupported.surftype.SurfaceType",
                                    surfaceTypeValid);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_SurfaceSupport_SurfaceType_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSurfaceType surfaceTypeInvalid = MFX_SURFACE_TYPE_UNKNOWN;
    #if defined(_WIN32) || defined(_WIN64)
    surfaceTypeInvalid = MFX_SURFACE_TYPE_VAAPI;
    #else
    surfaceTypeInvalid = MFX_SURFACE_TYPE_D3D11_TEX2D;
    #endif

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxSurfaceTypesSupported.surftype.SurfaceType",
                                    surfaceTypeInvalid);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_SurfaceSupport_SurfaceComponent_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceComponent",
                                    MFX_SURFACE_COMPONENT_ENCODE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_SurfaceSupport_SurfaceComponent_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceComponent",
                                    100);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_SurfaceSupport_SurfaceFlags_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // in a real application, flags alone aren't very useful (need type and component). But test the logic regardless.
    SetConfigFilterProperty<mfxU32>(
        loader,
        "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceFlags",
        (MFX_SURFACE_FLAG_IMPORT_SHARED | MFX_SURFACE_FLAG_IMPORT_COPY));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_SurfaceSupport_SurfaceFlags_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(
        loader,
        "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceFlags",
        (MFX_SURFACE_FLAG_IMPORT_SHARED | MFX_SURFACE_FLAG_EXPORT_COPY));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_SurfaceSupport_InvalidProperty(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxVariant var;
    var.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
    var.Type            = MFX_VARIANT_TYPE_U32;
    var.Data.U32        = 1;

    sts =
        MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxSurfaceTypesSupported.InvalidProp", var);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_SurfaceSupport_SingleFramework_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // need to use same cfg object for multiple surface props
    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    // full description for a single framework + component + flags
    mfxSurfaceType surfaceTypeValid = MFX_SURFACE_TYPE_UNKNOWN;
    #if defined(_WIN32) || defined(_WIN64)
    surfaceTypeValid = MFX_SURFACE_TYPE_D3D11_TEX2D;
    #else
    surfaceTypeValid   = MFX_SURFACE_TYPE_VAAPI;
    #endif

    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg,
                                    "mfxSurfaceTypesSupported.surftype.SurfaceType",
                                    surfaceTypeValid);

    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg,
                                    "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceComponent",
                                    MFX_SURFACE_COMPONENT_ENCODE);

    SetConfigFilterProperty<mfxU32>(
        loader,
        cfg,
        "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceFlags",
        (MFX_SURFACE_FLAG_IMPORT_SHARED | MFX_SURFACE_FLAG_IMPORT_COPY));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_SurfaceSupport_SingleFramework_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // need to use same cfg object for multiple surface props
    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    // full description for a single framework + component + flags
    mfxSurfaceType surfaceTypeValid = MFX_SURFACE_TYPE_UNKNOWN;
    #if defined(_WIN32) || defined(_WIN64)
    surfaceTypeValid = MFX_SURFACE_TYPE_D3D11_TEX2D;
    #else
    surfaceTypeValid   = MFX_SURFACE_TYPE_VAAPI;
    #endif

    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg,
                                    "mfxSurfaceTypesSupported.surftype.SurfaceType",
                                    surfaceTypeValid);

    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg,
                                    "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceComponent",
                                    MFX_SURFACE_COMPONENT_VPP_INPUT);

    // in the stub, VPP does not support IMPORT_SHARED
    SetConfigFilterProperty<mfxU32>(
        loader,
        cfg,
        "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceFlags",
        (MFX_SURFACE_FLAG_IMPORT_SHARED | MFX_SURFACE_FLAG_IMPORT_COPY));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_SurfaceSupport_TwoFrameworks_Valid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // full description for a single framework + component + flags
    mfxSurfaceType surfaceTypeValid = MFX_SURFACE_TYPE_UNKNOWN;
    #if defined(_WIN32) || defined(_WIN64)
    surfaceTypeValid = MFX_SURFACE_TYPE_D3D11_TEX2D;
    #else
    surfaceTypeValid   = MFX_SURFACE_TYPE_VAAPI;
    #endif

    // need to use same cfg object for multiple surface props
    // DX11 Import for Encode
    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg,
                                    "mfxSurfaceTypesSupported.surftype.SurfaceType",
                                    surfaceTypeValid);

    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg,
                                    "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceComponent",
                                    MFX_SURFACE_COMPONENT_ENCODE);

    SetConfigFilterProperty<mfxU32>(
        loader,
        cfg,
        "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceFlags",
        (MFX_SURFACE_FLAG_IMPORT_SHARED | MFX_SURFACE_FLAG_IMPORT_COPY));

    // create a new cfg object for the second framework
    // OCL Export for Decode
    mfxConfig cfg2 = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg2 == nullptr);

    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg2,
                                    "mfxSurfaceTypesSupported.surftype.SurfaceType",
                                    MFX_SURFACE_TYPE_OPENCL_IMG2D);

    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg2,
                                    "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceComponent",
                                    MFX_SURFACE_COMPONENT_DECODE);

    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg2,
                                    "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceFlags",
                                    MFX_SURFACE_FLAG_EXPORT_SHARED);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_SurfaceSupport_TwoFrameworks_Invalid(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // full description for a single framework + component + flags
    mfxSurfaceType surfaceTypeValid = MFX_SURFACE_TYPE_UNKNOWN;
    #if defined(_WIN32) || defined(_WIN64)
    surfaceTypeValid = MFX_SURFACE_TYPE_D3D11_TEX2D;
    #else
    surfaceTypeValid   = MFX_SURFACE_TYPE_VAAPI;
    #endif

    // need to use same cfg object for multiple surface props
    // DX11 Import for Encode
    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg,
                                    "mfxSurfaceTypesSupported.surftype.SurfaceType",
                                    surfaceTypeValid);

    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg,
                                    "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceComponent",
                                    MFX_SURFACE_COMPONENT_ENCODE);

    SetConfigFilterProperty<mfxU32>(
        loader,
        cfg,
        "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceFlags",
        (MFX_SURFACE_FLAG_IMPORT_SHARED | MFX_SURFACE_FLAG_IMPORT_COPY));

    // create a new cfg object for the second framework
    // OCL Export for VPP (out)
    mfxConfig cfg2 = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg2 == nullptr);

    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg2,
                                    "mfxSurfaceTypesSupported.surftype.SurfaceType",
                                    MFX_SURFACE_TYPE_OPENCL_IMG2D);

    // stub does not support VPP output
    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg2,
                                    "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceComponent",
                                    MFX_SURFACE_COMPONENT_VPP_OUTPUT);

    SetConfigFilterProperty<mfxU32>(loader,
                                    cfg2,
                                    "mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceFlags",
                                    MFX_SURFACE_FLAG_EXPORT_SHARED);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

#endif // ONEVPL_EXPERIMENTAL

// MFXEnumImplementations
void Dispatcher_EnumImplementations_ValidInputsReturnValidDesc(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;

    sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    ASSERT_EQ(sts, MFX_ERR_NONE);

    // confirm correct Impl type was found
    ASSERT_EQ(implDesc->Impl, implType);

    sts = MFXDispReleaseImplDescription(loader, implDesc);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_EnumImplementations_NullLoaderReturnsErrNull(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;

    sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(nullptr,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_EnumImplementations_NullDescReturnsErrNull(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;

    sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXEnumImplementations(loader, 0, MFX_IMPLCAPS_IMPLDESCSTRUCTURE, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_EnumImplementations_IndexOutOfRangeReturnsNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;

    sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(loader,
                                 999999,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

// MFXCreateSession
void Dispatcher_CreateSession_UnusedCfgCreatesSession(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    // create mfxConfig but do not apply filter property
    // should still create session with default props
    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    // create session with first implementation
    mfxSession session = nullptr;
    mfxStatus sts      = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestSWImplCreatesSession(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;

    sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_DoubleLoadersCreatesTwoSWSessions(mfxImplType implType) {
    mfxStatus sts;

    // first loader/session
    mfxLoader loader1 = MFXLoad();
    EXPECT_FALSE(loader1 == nullptr);

    SetConfigFilterProperty<mfxU32>(loader1, "mfxImplDescription.Impl", implType);

    mfxSession session1 = nullptr;
    sts                 = MFXCreateSession(loader1, 0, &session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session1, nullptr);

    // second loader/session
    mfxLoader loader2 = MFXLoad();
    EXPECT_FALSE(loader2 == nullptr);

    SetConfigFilterProperty<mfxU32>(loader2, "mfxImplDescription.Impl", implType);

    mfxSession session2 = nullptr;
    sts                 = MFXCreateSession(loader2, 0, &session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session2, nullptr);

    // teardown
    sts = MFXClose(session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    sts = MFXClose(session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader1);
    MFXUnload(loader2);
}

void Dispatcher_CreateSession_DoubleConfigObjsCreatesTwoSessions(mfxImplType implType) {
    mfxStatus sts;

    // first loader/session
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
                                    MFX_CODEC_AVC);

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
                                    MFX_CODEC_HEVC);

    // create two sessions
    mfxSession session1 = nullptr;
    sts                 = MFXCreateSession(loader, 0, &session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session1, nullptr);

    mfxSession session2 = nullptr;
    sts                 = MFXCreateSession(loader, 0, &session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session2, nullptr);

    // teardown
    sts = MFXClose(session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    sts = MFXClose(session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

void Dispatcher_CreateSession_NullLoaderReturnsErrNull(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxSession session = nullptr;
    mfxStatus sts      = MFXCreateSession(nullptr, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_NullSessionReturnsErrNull(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = MFXCreateSession(loader, 0, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_InvalidIndexReturnsErrNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxSession session = nullptr;
    mfxStatus sts      = MFXCreateSession(loader, 999999, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestSupportedDecoderCreatesSession(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;

    sts = SetConfigImpl(loader, implType, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
                                    MFX_CODEC_AVC);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestSupportedEncoderCreatesSession(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;

    sts = SetConfigImpl(loader, implType, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxImplDescription.mfxEncoderDescription.encoder.CodecID",
                                    MFX_CODEC_HEVC);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestSupportedVPPCreatesSession(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;

    sts = SetConfigImpl(loader, implType, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxU32 inFormat, outFormat;
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        inFormat  = MFX_FOURCC_I010;
        outFormat = MFX_FOURCC_I420;
    }
    else {
        inFormat  = MFX_FOURCC_P010;
        outFormat = MFX_FOURCC_NV12;
    }

    SetConfigFilterProperty<mfxU32>(
        loader,
        "mfxImplDescription.mfxVPPDescription.filter.memdesc.format.InFormat",
        inFormat);

    SetConfigFilterProperty<mfxU32>(
        loader,
        "mfxImplDescription.mfxVPPDescription.filter.memdesc.format.OutFormat",
        outFormat);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestUnsupportedDecoderReturnsErrNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;

    sts = SetConfigImpl(loader, implType, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxU32 inCodec;
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        inCodec = MFX_CODEC_VP9;
    }
    else {
        inCodec = MFX_CODEC_CAPTURE;
    }

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
                                    inCodec);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestUnsupportedEncoderReturnsErrNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxImplDescription.mfxEncoderDescription.encoder.CodecID",
                                    MFX_CODEC_VC1);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestTwoSupportedDecodersReturnsErrNone(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
                                    MFX_CODEC_AVC);

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
                                    MFX_CODEC_HEVC);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXClose(session);
    MFXUnload(loader);
}

// request two decoders, one supported format and one unsupported format
void Dispatcher_CreateSession_RequestMixedDecodersReturnsErrNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
                                    MFX_CODEC_AVC);

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
                                    MFX_CODEC_CAPTURE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestSupportedAccelModeCreatesSession(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxAccelerationMode accelMode;
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        accelMode = MFX_ACCEL_MODE_NA;
    }
    else {
#if defined(_WIN32) || defined(_WIN64)
        accelMode = MFX_ACCEL_MODE_VIA_D3D11;
#else
        accelMode = MFX_ACCEL_MODE_VIA_VAAPI;
#endif
    }

    SetConfigFilterProperty<mfxU32>(loader, "mfxImplDescription.AccelerationMode", accelMode);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxIMPL actualImpl = {};

    sts = MFXQueryIMPL(session, &actualImpl);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        // CPU RT should set impl = MFX_IMPL_TYPE_SOFTWARE, no other flags
        EXPECT_EQ(actualImpl, MFX_IMPL_TYPE_SOFTWARE);
    }
    else {
        // GPU RT should set impl = requested accelMode
        EXPECT_EQ((actualImpl & 0x0F00), accelMode);
    }

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestUnsupportedAccelModeReturnsNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxAccelerationMode accelMode;
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        accelMode = MFX_ACCEL_MODE_VIA_D3D11;
    }
    else {
        accelMode = MFX_ACCEL_MODE_NA;
    }

    SetConfigFilterProperty<mfxU32>(loader, "mfxImplDescription.AccelerationMode", accelMode);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestCurrentAPIVersionCreatesSession(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxVersion ver = {};
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        ver.Major = CPU_VERSION_MAJOR;
        ver.Minor = CPU_VERSION_MINOR;
    }
    else {
        // MSDK may have 1.x API
        ver.Major = 1;
        ver.Minor = 0;
    }

    SetConfigFilterProperty<mfxU32>(loader, "mfxImplDescription.ApiVersion.Version", ver.Version);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestLowerAPIVersionCreatesSession(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // request version (API - 0.1), which should pass
    mfxVersion ver = {};
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        ver.Major = CPU_VERSION_MAJOR;
        ver.Minor = CPU_VERSION_MINOR - 1;
    }
    else {
        // MSDK may have 1.x API
        ver.Major = 1;
        ver.Minor = 0;
    }

    SetConfigFilterProperty<mfxU32>(loader, "mfxImplDescription.ApiVersion.Version", ver.Version);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestHigherAPIVersionReturnsNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // request version (API + 0.5), which should fail
    mfxVersion ver = {};
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        ver.Major = CPU_VERSION_MAJOR;
        ver.Minor = CPU_VERSION_MINOR + 5;
    }
    else {
        ver.Major = MFX_VERSION_MAJOR;
        ver.Minor = MFX_VERSION_MINOR + 5;
    }

    SetConfigFilterProperty<mfxU32>(loader, "mfxImplDescription.ApiVersion.Version", ver.Version);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestImplementedFunctionCreatesSession(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // request an implemented function, which should pass
    SetConfigFilterProperty<mfxHDL>(loader,
                                    "mfxImplementedFunctions.FunctionsName",
                                    const_cast<char *>("MFXQueryVersion"));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestNotImplementedFunctionReturnsNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // request a non-existent function, which should fail
    SetConfigFilterProperty<mfxHDL>(loader,
                                    "mfxImplementedFunctions.FunctionsName",
                                    const_cast<char *>("MFXVideoDECODE_VPP_NOT_A_FUNCTION"));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestCurrentAPIMajorMinorCreatesSession(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxVersion ver = {};
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        ver.Major = CPU_VERSION_MAJOR;
        ver.Minor = CPU_VERSION_MINOR;
    }
    else {
        // MSDK may have 1.x API
        ver.Major = 1;
        ver.Minor = 0;
    }

    // set major value
    SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Major", ver.Major);

    // set minor value
    SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Minor", ver.Minor);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestHigherAPIMajorReturnsNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxVersion ver = {};
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        ver.Major = CPU_VERSION_MAJOR;
        ver.Minor = CPU_VERSION_MINOR;
    }
    else {
        ver.Major = MFX_VERSION_MAJOR;
        ver.Minor = MFX_VERSION_MINOR;
    }

    // set major value = MFX_VERSION_MAJOR + 1, which should fail
    SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Major", ver.Major + 1);

    // set minor value
    SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Minor", ver.Minor);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestHigherAPIMinorReturnsNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxVersion ver = {};
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        ver.Major = CPU_VERSION_MAJOR;
        ver.Minor = CPU_VERSION_MINOR;
    }
    else {
        ver.Major = MFX_VERSION_MAJOR;
        ver.Minor = MFX_VERSION_MINOR;
    }

    // set major value
    SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Major", ver.Major);

    // set minor value = MFX_VERSION_MINOR + 5, which should fail
    SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Minor", ver.Minor + 5);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestDeviceIDValidReturnsErrNone(mfxImplType implType) {
    // no way of knowing what deviceID we might run on in HW
    if (implType != MFX_IMPL_TYPE_SOFTWARE)
        GTEST_SKIP();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass deviceID = 0x0000 for CPU
    SetConfigFilterProperty<mfxU16>(loader,
                                    "mfxImplDescription.mfxDeviceDescription.device.DeviceID",
                                    0x0000);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXClose(session);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestDeviceIDInvalidReturnsErrNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass invalid device ID
    SetConfigFilterProperty<mfxU16>(loader,
                                    "mfxImplDescription.mfxDeviceDescription.device.DeviceID",
                                    0xafaf);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

// tests for MediaAdapterType also require mfxDeviceDescription struct version >= 1.1
void Dispatcher_CreateSession_RequestMediaAdapterTypeValidReturnsErrNone(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxU16 mediaAdapterType = 0xafaf;
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        mediaAdapterType = MFX_MEDIA_UNKNOWN;
    }
    else {
        // assume HW has integrated device
        // need another configuration of unit tests for discrete-only systems
        mediaAdapterType = MFX_MEDIA_INTEGRATED;
    }

    SetConfigFilterProperty<mfxU16>(
        loader,
        "mfxImplDescription.mfxDeviceDescription.device.MediaAdapterType",
        mediaAdapterType);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXClose(session);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestMediaAdapterTypeInvalidReturnsErrNotFound(
    mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // invalid value
    mfxU16 mediaAdapterType = 0xafaf;

    SetConfigFilterProperty<mfxU16>(
        loader,
        "mfxImplDescription.mfxDeviceDescription.device.MediaAdapterType",
        mediaAdapterType);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

// tests for PoolAllocationPolicy require mfxImplDescription struct version >= 1.2
void Dispatcher_CreateSession_RequestPoolAllocationPolicyValidReturnsErrNone(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxPoolAllocationPolicy poolAllocationPolicy = MFX_ALLOCATION_UNLIMITED;
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        poolAllocationPolicy = MFX_ALLOCATION_UNLIMITED;
    }
    else {
        // assume HW supports optimal policy
        poolAllocationPolicy = MFX_ALLOCATION_OPTIMAL;
    }

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxImplDescription.mfxSurfacePoolMode",
                                    poolAllocationPolicy);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXClose(session);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestPoolAllocationPolicyInvalidReturnsErrNotFound(
    mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxPoolAllocationPolicy poolAllocationPolicy =
        (mfxPoolAllocationPolicy)0xfafa; // invalid policy;

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxImplDescription.mfxSurfacePoolMode",
                                    poolAllocationPolicy);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestImplNameValidReturnsErrNone(mfxImplType implType) {
    if (implType != MFX_IMPL_TYPE_HARDWARE)
        GTEST_SKIP();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass the correct ImplName for RT
    SetConfigFilterProperty<mfxHDL>(loader,
                                    "mfxImplDescription.ImplName",
                                    const_cast<char *>("mfx-gen"));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXClose(session);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestImplNameInvalidReturnsErrNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass invalid ImplName
    SetConfigFilterProperty<mfxHDL>(loader,
                                    "mfxImplDescription.ImplName",
                                    const_cast<char *>("Unsupported Impl"));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXClose(session);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestLicenseValidReturnsErrNone(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass the correct License type
    SetConfigFilterProperty<mfxHDL>(loader,
                                    "mfxImplDescription.License",
                                    const_cast<char *>("MIT"));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXClose(session);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestLicenseInvalidReturnsErrNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass the incorrect License type
    SetConfigFilterProperty<mfxHDL>(loader,
                                    "mfxImplDescription.License",
                                    const_cast<char *>("ABC"));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestLicenseMixedReturnsErrNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass a correct and incorrect License type
    SetConfigFilterProperty<mfxHDL>(loader,
                                    "mfxImplDescription.License",
                                    const_cast<char *>("MIT,ABC"));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestKeywordsValidReturnsErrNone(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass supported Keywords
    SetConfigFilterProperty<mfxHDL>(loader,
                                    "mfxImplDescription.Keywords",
                                    const_cast<char *>("CPU,VPL"));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXClose(session);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestKeywordsMixedReturnsErrNotFound(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass unsupported Keywords
    SetConfigFilterProperty<mfxHDL>(loader,
                                    "mfxImplDescription.Keywords",
                                    const_cast<char *>("GPU,VPL"));

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_ConfigHandleReturnsHandle(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxHDL testHandle = (mfxHDL)0xabcd1234eeff5566;
#if defined(_WIN32) || defined(_WIN64)
    mfxHandleType testHandleType = MFX_HANDLE_D3D11_DEVICE;
#else
    mfxHandleType testHandleType = MFX_HANDLE_VA_DISPLAY;
#endif

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - mfxHandleType
    SetConfigFilterProperty<mfxU32>(loader, "mfxHandleType", testHandleType);

    // pass special config property - mfxHDL
    SetConfigFilterProperty<mfxHDL>(loader, "mfxHDL", testHandle);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // verify that handle was set properly
    mfxHDL returnHandle = nullptr;
    sts                 = MFXVideoCORE_GetHandle(session, testHandleType, &returnHandle);
    EXPECT_EQ(returnHandle, testHandle);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestValidDXGIAdapterCreatesSession(mfxImplType implType) {
    if (implType != MFX_IMPL_TYPE_HARDWARE)
        GTEST_SKIP();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;

    sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    // assume that test system has an x86 GPU as adapter 0
    mfxVariant ImplValue;

    ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
    ImplValue.Type            = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32        = 0;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"DXGIAdapterIndex", ImplValue);

#if defined(_WIN32) || defined(_WIN64)
    // property is only valid on Windows
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
#else
    // property should return ERR_NOT_FOUND on non-Win
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);
#endif

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_CreateSession_RequestInvalidDXGIAdapterReturnsErrNotFound(mfxImplType implType) {
    if (implType != MFX_IMPL_TYPE_HARDWARE)
        GTEST_SKIP();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;

    sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    // request adapter index which is far out of normal range
    mfxVariant ImplValue;

    ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
    ImplValue.Type            = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32        = 999999;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"DXGIAdapterIndex", ImplValue);

#if defined(_WIN32) || defined(_WIN64)
    // property is only valid on Windows
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXClose(session);
#else
    // property should return ERR_NOT_FOUND on non-Win
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);
#endif

    MFXUnload(loader);
}

// MFXDispReleaseImplDescription
void Dispatcher_DispReleaseImplDescription_ValidInputReturnsErrNone(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;

    sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    ASSERT_EQ(sts, MFX_ERR_NONE);

    // confirm correct Impl type was found
    ASSERT_EQ(implDesc->Impl, implType);

    sts = MFXDispReleaseImplDescription(loader, implDesc);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_DispReleaseImplDescription_NullLoaderReturnsErrNull(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXDispReleaseImplDescription(nullptr, implDesc);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_DispReleaseImplDescription_NullDescReturnsErrNull(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc;
    mfxStatus sts = MFXEnumImplementations(loader,
                                           0,
                                           MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                           reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXDispReleaseImplDescription(loader, nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    MFXUnload(loader);
}

void Dispatcher_DispReleaseImplDescription_HandleMismatchReturnsInvalidHandle(
    mfxImplType implType) {
    mfxLoader loader1 = MFXLoad();
    EXPECT_FALSE(loader1 == nullptr);

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc1;
    mfxStatus sts = MFXEnumImplementations(loader1,
                                           0,
                                           MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                           reinterpret_cast<mfxHDL *>(&implDesc1));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass invalid handle (address of a local struct)
    mfxImplDescription implDescLocal;
    sts = MFXDispReleaseImplDescription(loader1, &implDescLocal);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    // free internal resources
    MFXUnload(loader1);
}

void Dispatcher_DispReleaseImplDescription_ReleaseTwiceReturnsErrNone(mfxImplType implType) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // first pass - Enum/CreateSession/DispRelease

    // enumerate implementations, check capabilities of first one
    mfxImplDescription *implDesc;
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    ASSERT_EQ(sts, MFX_ERR_NONE);

    // confirm correct Impl type was found
    ASSERT_NE(implDesc, nullptr);
    ASSERT_EQ(implDesc->Impl, implType);

    // create session
    mfxSession session1 = nullptr;
    sts                 = MFXCreateSession(loader, 0, &session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXDispReleaseImplDescription(loader, implDesc);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // second pass - Enum/CreateSession/DispRelease
    // we are testing that calling EnumImplementations again on the same impl
    //   (after calling DispRelease) returns a valid implDesc and creates a new session

    // enumerate implementations, check capabilities of first one
    implDesc = nullptr;
    sts      = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    ASSERT_EQ(sts, MFX_ERR_NONE);

    // confirm correct Impl type was found
    ASSERT_NE(implDesc, nullptr);
    ASSERT_EQ(implDesc->Impl, implType);

    // create session
    mfxSession session2 = nullptr;
    sts                 = MFXCreateSession(loader, 0, &session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXDispReleaseImplDescription(loader, implDesc);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    sts = MFXClose(session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

// MFXLoad
TEST(Dispatcher_Common_Load, CallReturnsLoader) {
    mfxLoader loader = MFXLoad();
    EXPECT_NE(loader, nullptr);

    // free internal resources
    MFXUnload(loader);
}

// MFXCreateConfig
TEST(Dispatcher_Common_CreateConfig, InitializedLoaderReturnsConfig) {
    mfxLoader loader = MFXLoad();
    EXPECT_NE(loader, nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_NE(cfg, nullptr);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Common_CreateConfig, NullLoaderReturnsErrNull) {
    mfxConfig cfg = MFXCreateConfig(nullptr);
    EXPECT_EQ(cfg, nullptr);
}

// MFXSetConfigFilterProperty
TEST(Dispatcher_Common_SetConfigFilterProperty, VPLImplInReturnsErrNone) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
    ImplValue.Type            = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32        = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Common_SetConfigFilterProperty, NullConfigReturnsErrNull) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
    ImplValue.Type            = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32        = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(nullptr, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Common_SetConfigFilterProperty, NullNameReturnsErrNull) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
    ImplValue.Type            = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32        = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg, nullptr, ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Common_SetConfigFilterProperty, UnknownParamReturnsNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
    ImplValue.Type            = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32        = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Unknown", ImplValue);

    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Common_SetConfigFilterProperty, ValueTypeMismatchReturnsErrUnsupported) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
    ImplValue.Type            = MFX_VARIANT_TYPE_U8;
    ImplValue.Data.U32        = MFX_IMPL_TYPE_SOFTWARE;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Common_SetConfigFilterProperty, PartialFilterReturnsErrNotFound) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
    ImplValue.Type            = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32        = 0;
    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription", ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Common_SetConfigFilterProperty, OutOfRangeValueReturnsErrNone) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxStatus sts;
    mfxVariant ImplValue;

    ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
    ImplValue.Type            = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32        = 9999;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.Impl", ImplValue);

    ASSERT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    MFXUnload(loader);
}

// dispatcher logging
TEST(Dispatcher_Common_Logger, LogEnabledReturnsMessages) {
    // capture dispatcher log
    CaptureOutputLog(CAPTURE_LOG_DISPATCHER);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
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

    // check for some expected dispatcher log strings
#if defined(_WIN32) || defined(_WIN64)
    CheckOutputLog("function: MFXCreateSession (enter)", true);
    CheckOutputLog("function: MFXCreateSession (return)", true);
#elif defined(__clang__)
    CheckOutputLog("function: mfxStatus MFXCreateSession(mfxLoader, mfxU32, mfxSession *) (enter)",
                   true);
    CheckOutputLog("function: mfxStatus MFXCreateSession(mfxLoader, mfxU32, mfxSession *) (return)",
                   true);
#else
    CheckOutputLog("function: mfxStatus MFXCreateSession(mfxLoader, mfxU32, _mfxSession**) (enter)",
                   true);
    CheckOutputLog(
        "function: mfxStatus MFXCreateSession(mfxLoader, mfxU32, _mfxSession**) (return)",
        true);
#endif
    CleanupOutputLog();
}

TEST(Dispatcher_Common_Logger, LogDisabledReturnsNoMessages) {
    // capture all output, but do not enable dispatcher log
    CaptureOutputLog(CAPTURE_LOG_COUT);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
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

    // check for some expected dispatcher log strings
#if defined(_WIN32) || defined(_WIN64)
    CheckOutputLog("function: MFXCreateSession (enter)", false);
    CheckOutputLog("function: MFXCreateSession (return)", false);
#else
    CheckOutputLog("function: mfxStatus MFXCreateSession(mfxLoader, mfxU32, _mfxSession**) (enter)",
                   false);
    CheckOutputLog("function: mfxStatus MFXCreateSession(mfxLoader, mfxU32, _mfxSession**) (enter)",
                   false);
#endif
    CleanupOutputLog();
}
