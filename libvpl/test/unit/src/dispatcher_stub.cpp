/*############################################################################
  # Copyright (C) Intel Corporation
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

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_RevisionID_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_RevisionID_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_RevisionID_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_RevisionID_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_DeviceName_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_DeviceName_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, ExtDeviceID_DeviceName_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_ExtDeviceID_DeviceName_Invalid(MFX_IMPL_TYPE_STUB);
}

#ifdef ONEVPL_EXPERIMENTAL

// mfxSurfaceTypesSupported tests
TEST(Dispatcher_Stub_CreateSession, SurfaceSupport_SurfaceType_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SurfaceSupport_SurfaceType_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, SurfaceSupport_SurfaceType_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SurfaceSupport_SurfaceType_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, SurfaceSupport_SurfaceComponent_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SurfaceSupport_SurfaceComponent_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, SurfaceSupport_SurfaceComponent_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SurfaceSupport_SurfaceComponent_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, SurfaceSupport_SurfaceFlags_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SurfaceSupport_SurfaceFlags_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, SurfaceSupport_SurfaceFlags_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SurfaceSupport_SurfaceFlags_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, SurfaceSupport_InvalidProperty) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SurfaceSupport_InvalidProperty(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, SurfaceSupport_SingleFramework_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SurfaceSupport_SingleFramework_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, SurfaceSupport_SingleFramework_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SurfaceSupport_SingleFramework_Invalid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, SurfaceSupport_TwoFrameworks_Valid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SurfaceSupport_TwoFrameworks_Valid(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, SurfaceSupport_TwoFrameworks_Invalid) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_SurfaceSupport_TwoFrameworks_Invalid(MFX_IMPL_TYPE_STUB);
}

#endif // ONEVPL_EXPERIMENTAL

// test using NumThread filter property during initialization
static void Dispatcher_CreateSession_RuntimeParsesExtBuf(mfxImplType implType) {
    // stub RT logs results from parsing extBuf
    CaptureOutputLog(CAPTURE_LOG_COUT);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - NumThread
    SetConfigFilterProperty<mfxU32>(loader, "NumThread", 4);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);

    // check for RT log string which indicates that extBuf was parsed properly
    if (implType == MFX_IMPL_TYPE_STUB_1X) {
        CheckOutputLog("[STUB RT]: message -- MFXInitEx -- extBuf enabled -- NumThread (4)");
    }
    else {
        CheckOutputLog("[STUB RT]: message -- MFXInitialize -- extBuf enabled -- NumThread (4)");
    }

    CleanupOutputLog();
}

TEST(Dispatcher_Stub_CreateSession, RuntimeParsesExtBuf) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_RuntimeParsesExtBuf(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, LegacyRuntimeParsesExtBuf) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_RuntimeParsesExtBuf(MFX_IMPL_TYPE_STUB_1X);
}

// test using ExtBuffer filter property during initialization to pass one extBuffer
static void Dispatcher_CreateSession_RuntimeParsesSingleExtBufViaMFXConfig(mfxImplType implType) {
    // stub RT logs results from parsing extBuf
    CaptureOutputLog(CAPTURE_LOG_COUT);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - ExtBuffer with ptr to buffer
    mfxExtThreadsParam extThreadsParam = {};
    extThreadsParam.Header.BufferId    = MFX_EXTBUFF_THREADS_PARAM;
    extThreadsParam.Header.BufferSz    = sizeof(mfxExtThreadsParam);
    extThreadsParam.NumThread          = 8;

    SetConfigFilterProperty<mfxHDL>(loader, "ExtBuffer", (void *)&extThreadsParam);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);

    // check for RT log string which indicates that extBuf was parsed properly
    if (implType == MFX_IMPL_TYPE_STUB_1X) {
        CheckOutputLog("[STUB RT]: message -- MFXInitEx -- extBuf enabled -- NumThread (8)");
    }
    else {
        CheckOutputLog("[STUB RT]: message -- MFXInitialize -- extBuf enabled -- NumThread (8)");
    }

    CleanupOutputLog();
}

TEST(Dispatcher_Stub_CreateSession, RuntimeParsesSingleExtBufViaMFXConfig) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_RuntimeParsesSingleExtBufViaMFXConfig(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, LegacyRuntimeParsesSingleExtBufViaMFXConfig) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_RuntimeParsesSingleExtBufViaMFXConfig(MFX_IMPL_TYPE_STUB_1X);
}

// test using ExtBuffer filter property during initialization to pass multiple extBuffers
//  in separate mfxConfig objects
static void Dispatcher_CreateSession_RuntimeParsesMultipleExtBufsViaMFXConfig(
    mfxImplType implType) {
    // stub RT logs results from parsing extBuf
    CaptureOutputLog(CAPTURE_LOG_COUT);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config properties - ExtBuffer with ptr to buffer
    mfxExtThreadsParam extThreadsParam = {};
    extThreadsParam.Header.BufferId    = MFX_EXTBUFF_THREADS_PARAM;
    extThreadsParam.Header.BufferSz    = sizeof(mfxExtThreadsParam);
    extThreadsParam.NumThread          = 6;

    SetConfigFilterProperty<mfxHDL>(loader, "ExtBuffer", (void *)&extThreadsParam);

    // would not be valid at init time in a real RT, but use to validate logic with stub impls
    mfxExtVPPProcAmp extVPPProcAmp = {};
    extVPPProcAmp.Header.BufferId  = MFX_EXTBUFF_VPP_PROCAMP;
    extVPPProcAmp.Header.BufferSz  = sizeof(mfxExtVPPProcAmp);
    extVPPProcAmp.Contrast         = 31.7f;
    extVPPProcAmp.Brightness       = 16.14f;
    extVPPProcAmp.Hue              = 54.2f;
    extVPPProcAmp.Saturation       = -15.63f;

    // creates a new mfxConfig object
    SetConfigFilterProperty<mfxHDL>(loader, "ExtBuffer", (void *)&extVPPProcAmp);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);

    // check for RT log strings which indicate that extBufs were parsed properly
    if (implType == MFX_IMPL_TYPE_STUB_1X) {
        CheckOutputLog("[STUB RT]: message -- MFXInitEx -- extBuf enabled -- NumThread (6)");

        CheckOutputLog(
            "[STUB RT]: message -- MFXInitEx -- extBuf enabled -- ProcAmp (31.7 16.14 54.2 -15.63)");
    }
    else {
        CheckOutputLog("[STUB RT]: message -- MFXInitialize -- extBuf enabled -- NumThread (6)");

        CheckOutputLog(
            "[STUB RT]: message -- MFXInitialize -- extBuf enabled -- ProcAmp (31.7 16.14 54.2 -15.63)");
    }

    CleanupOutputLog();
}

TEST(Dispatcher_Stub_CreateSession, RuntimeParsesMultipleExtBufsViaMFXConfig) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_RuntimeParsesMultipleExtBufsViaMFXConfig(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, LegacyRuntimeParsesMultipleExtBufsViaMFXConfig) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_RuntimeParsesMultipleExtBufsViaMFXConfig(MFX_IMPL_TYPE_STUB_1X);
}

// test using ExtBuffer filter property during initialization to pass a single extBuffer
//   by overwriting the same mfxConfig object
static void Dispatcher_CreateSession_RuntimeParsesSingleExtBufOverwriteViaMFXConfig(
    mfxImplType implType) {
    // stub RT logs results from parsing extBuf
    CaptureOutputLog(CAPTURE_LOG_COUT);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config properties - ExtBuffer with ptr to buffer
    mfxExtThreadsParam extThreadsParam = {};
    extThreadsParam.Header.BufferId    = MFX_EXTBUFF_THREADS_PARAM;
    extThreadsParam.Header.BufferSz    = sizeof(mfxExtThreadsParam);
    extThreadsParam.NumThread          = 6;

    mfxConfig threadsConfig = MFXCreateConfig(loader);
    SetConfigFilterProperty<mfxHDL>(loader, threadsConfig, "ExtBuffer", (void *)&extThreadsParam);

    // would not be valid at init time in a real RT, but use to validate logic with stub impls
    mfxExtVPPProcAmp extVPPProcAmp = {};
    extVPPProcAmp.Header.BufferId  = MFX_EXTBUFF_VPP_PROCAMP;
    extVPPProcAmp.Header.BufferSz  = sizeof(mfxExtVPPProcAmp);
    extVPPProcAmp.Contrast         = 31.7f;
    extVPPProcAmp.Brightness       = 16.14f;
    extVPPProcAmp.Hue              = 54.2f;
    extVPPProcAmp.Saturation       = -15.63f;

    // overwrite threadsConfig with extVPPProcAmp
    SetConfigFilterProperty<mfxHDL>(loader, threadsConfig, "ExtBuffer", (void *)&extVPPProcAmp);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);

    // check for RT log strings which indicate that extBufs were parsed properly
    // expect that NumThread is NOT present, since the buffer should have been overwritten
    if (implType == MFX_IMPL_TYPE_STUB_1X) {
        CheckOutputLog("[STUB RT]: message -- MFXInitEx -- extBuf enabled -- NumThread (6)", false);

        CheckOutputLog(
            "[STUB RT]: message -- MFXInitEx -- extBuf enabled -- ProcAmp (31.7 16.14 54.2 -15.63)");
    }
    else {
        CheckOutputLog("[STUB RT]: message -- MFXInitialize -- extBuf enabled -- NumThread (6)",
                       false);

        CheckOutputLog(
            "[STUB RT]: message -- MFXInitialize -- extBuf enabled -- ProcAmp (31.7 16.14 54.2 -15.63)");
    }

    CleanupOutputLog();
}

TEST(Dispatcher_Stub_CreateSession, RuntimeParsesSingleExtBufOverwriteViaMFXConfig) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_RuntimeParsesSingleExtBufOverwriteViaMFXConfig(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, LegacyRuntimeParsesSingleExtBufOverwriteViaMFXConfig) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_RuntimeParsesSingleExtBufOverwriteViaMFXConfig(MFX_IMPL_TYPE_STUB_1X);
}

// test using ExtBuffer filter property during initialization to pass multiple extBuffers
//  in separate mfxConfig objects, plus overwriting one mfxConfig object with a new value
static void Dispatcher_CreateSession_RuntimeParsesMultipleExtBufsOverwriteViaMFXConfig(
    mfxImplType implType) {
    // stub RT logs results from parsing extBuf
    CaptureOutputLog(CAPTURE_LOG_COUT);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config properties - ExtBuffer with ptr to buffer
    mfxExtThreadsParam extThreadsParam = {};
    extThreadsParam.Header.BufferId    = MFX_EXTBUFF_THREADS_PARAM;
    extThreadsParam.Header.BufferSz    = sizeof(mfxExtThreadsParam);
    extThreadsParam.NumThread          = 6;

    mfxConfig threadsConfig = MFXCreateConfig(loader);
    SetConfigFilterProperty<mfxHDL>(loader, threadsConfig, "ExtBuffer", (void *)&extThreadsParam);

    mfxExtThreadsParam extThreadsParam2 = {};
    extThreadsParam2.Header.BufferId    = MFX_EXTBUFF_THREADS_PARAM;
    extThreadsParam2.Header.BufferSz    = sizeof(mfxExtThreadsParam);
    extThreadsParam2.NumThread          = 9;

    // reuse same mfxConfig object - previous extBuf should be overwritten
    SetConfigFilterProperty<mfxHDL>(loader, threadsConfig, "ExtBuffer", (void *)&extThreadsParam2);

    // would not be valid at init time in a real RT, but use to validate logic with stub impls
    mfxExtVPPProcAmp extVPPProcAmp = {};
    extVPPProcAmp.Header.BufferId  = MFX_EXTBUFF_VPP_PROCAMP;
    extVPPProcAmp.Header.BufferSz  = sizeof(mfxExtVPPProcAmp);
    extVPPProcAmp.Contrast         = 31.7f;
    extVPPProcAmp.Brightness       = 16.14f;
    extVPPProcAmp.Hue              = 54.2f;
    extVPPProcAmp.Saturation       = -15.63f;

    SetConfigFilterProperty<mfxHDL>(loader, "ExtBuffer", (void *)&extVPPProcAmp);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);

    // check for RT log strings which indicate that extBufs were parsed properly
    if (implType == MFX_IMPL_TYPE_STUB_1X) {
        CheckOutputLog("[STUB RT]: message -- MFXInitEx -- extBuf enabled -- NumThread (9)");

        CheckOutputLog(
            "[STUB RT]: message -- MFXInitEx -- extBuf enabled -- ProcAmp (31.7 16.14 54.2 -15.63)");
    }
    else {
        CheckOutputLog("[STUB RT]: message -- MFXInitialize -- extBuf enabled -- NumThread (9)");

        CheckOutputLog(
            "[STUB RT]: message -- MFXInitialize -- extBuf enabled -- ProcAmp (31.7 16.14 54.2 -15.63)");
    }

    CleanupOutputLog();
}

TEST(Dispatcher_Stub_CreateSession, RuntimeParsesMultipleExtBufsOverwriteViaMFXConfig) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_RuntimeParsesMultipleExtBufsOverwriteViaMFXConfig(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, LegacyRuntimeParsesMultipleExtBufsOverwriteViaMFXConfig) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_RuntimeParsesMultipleExtBufsOverwriteViaMFXConfig(
        MFX_IMPL_TYPE_STUB_1X);
}

static void Dispatcher_CreateSession_RuntimeParsesExtBufsAndNumThreadViaMFXConfig(
    mfxImplType implType) {
    // stub RT logs results from parsing extBuf
    CaptureOutputLog(CAPTURE_LOG_COUT);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config properties - ExtBuffer with ptr to buffer
    mfxExtThreadsParam extThreadsParam = {};
    extThreadsParam.Header.BufferId    = MFX_EXTBUFF_THREADS_PARAM;
    extThreadsParam.Header.BufferSz    = sizeof(mfxExtThreadsParam);
    extThreadsParam.NumThread          = 12;

    SetConfigFilterProperty<mfxHDL>(loader, "ExtBuffer", (void *)&extThreadsParam);

    // pass special config property - NumThread
    SetConfigFilterProperty<mfxU32>(loader, "NumThread", 24);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);

    // check for RT log strings which indicate that extBufs were parsed properly
    // effectively we are passing two of the same type of extBuf - it is up to the RT
    //   how to handle this situation
    // with the stubs, we just expect to print a valid log message for both
    if (implType == MFX_IMPL_TYPE_STUB_1X) {
        CheckOutputLog("[STUB RT]: message -- MFXInitEx -- extBuf enabled -- NumThread (12)");

        CheckOutputLog("[STUB RT]: message -- MFXInitEx -- extBuf enabled -- NumThread (24)");
    }
    else {
        CheckOutputLog("[STUB RT]: message -- MFXInitialize -- extBuf enabled -- NumThread (12)");

        CheckOutputLog("[STUB RT]: message -- MFXInitialize -- extBuf enabled -- NumThread (24)");
    }

    CleanupOutputLog();
}

TEST(Dispatcher_Stub_CreateSession, RuntimeParsesExtBufsAndNumThreadViaMFXConfig) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_RuntimeParsesExtBufsAndNumThreadViaMFXConfig(MFX_IMPL_TYPE_STUB);
}

TEST(Dispatcher_Stub_CreateSession, LegacyRuntimeParsesExtBufsAndNumThreadViaMFXConfig) {
    SKIP_IF_DISP_STUB_DISABLED();
    Dispatcher_CreateSession_RuntimeParsesExtBufsAndNumThreadViaMFXConfig(MFX_IMPL_TYPE_STUB_1X);
}

TEST(Dispatcher_Stub_CreateSession, NullExtBufReturnsErrNull) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg = MFXCreateConfig(loader);
    mfxVariant ImplValue;
    ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
    ImplValue.Type            = MFX_VARIANT_TYPE_PTR;
    ImplValue.Data.Ptr        = nullptr;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"ExtBuffer", ImplValue);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    MFXUnload(loader);
}

TEST(Dispatcher_Stub_CreateSession, UnsupportedExtBufReturnsUnsupported) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass extBuf which the stub RT does not support
    // spec actually does not address this case so may be runtime
    //   dependent (RT could ignore unsupported ones and continue)
    mfxExtVPPDetail extVPPDetail = {};
    extVPPDetail.Header.BufferId = MFX_EXTBUFF_VPP_DETAIL;
    extVPPDetail.Header.BufferSz = sizeof(mfxExtVPPDetail);

    SetConfigFilterProperty<mfxHDL>(loader, "ExtBuffer", (void *)&extVPPDetail);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

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

TEST(Dispatcher_Stub_CloneSession, Basic_Clone_Succeeds1x) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB_1X);
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

#ifdef ONEVPL_EXPERIMENTAL

TEST(Dispatcher_Stub_CreateSession, DeviceCopySetOn) {
    SKIP_IF_DISP_STUB_DISABLED();

    // stub RT logs results from MFXInitialize
    CaptureOutputLog(CAPTURE_LOG_COUT);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - NumThread
    SetConfigFilterProperty<mfxU16>(loader, "DeviceCopy", MFX_GPUCOPY_ON);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);

    // check for RT log string which indicates that DeviceCopy was set properly
    CheckOutputLog("[STUB RT]: message -- MFXInitialize -- DeviceCopy set (1)");
    CleanupOutputLog();
}

TEST(Dispatcher_Stub_CreateSession, DeviceCopySetOff) {
    SKIP_IF_DISP_STUB_DISABLED();

    // stub RT logs results from MFXInitialize
    CaptureOutputLog(CAPTURE_LOG_COUT);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - NumThread
    SetConfigFilterProperty<mfxU16>(loader, "DeviceCopy", MFX_GPUCOPY_OFF);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check for RT log string which indicates that DeviceCopy was set properly
    CheckOutputLog("[STUB RT]: message -- MFXInitialize -- DeviceCopy set (2)");
    CleanupOutputLog();

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_CreateSession, DeviceCopySetInvalid) {
    SKIP_IF_DISP_STUB_DISABLED();

    // stub RT logs results from MFXInitialize
    CaptureOutputLog(CAPTURE_LOG_COUT);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - NumThread
    SetConfigFilterProperty<mfxU16>(loader, "DeviceCopy", 999);

    // expect to fail
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    // free internal resources
    MFXUnload(loader);

    // check for RT log string which indicates that DeviceCopy was set to invalid value
    CheckOutputLog("[STUB RT]: message -- MFXInitialize -- DeviceCopy set to invalid value (999)");
    CleanupOutputLog();
}

TEST(Dispatcher_Stub_CreateSession, DeviceCopySetOn1x) {
    SKIP_IF_DISP_STUB_DISABLED();

    // stub RT logs results from MFXInitialize
    CaptureOutputLog(CAPTURE_LOG_COUT);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB_1X);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - NumThread
    SetConfigFilterProperty<mfxU16>(loader, "DeviceCopy", MFX_GPUCOPY_ON);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);

    // check for RT log string which indicates that DeviceCopy was set properly
    CheckOutputLog("[STUB RT]: message -- MFXInitEx -- GPUCopy set (1)");
    CleanupOutputLog();
}

TEST(Dispatcher_Stub_CreateSession, DeviceCopySetOff1x) {
    SKIP_IF_DISP_STUB_DISABLED();

    // stub RT logs results from MFXInitialize
    CaptureOutputLog(CAPTURE_LOG_COUT);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB_1X);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - NumThread
    sts = SetConfigFilterProperty<mfxU16>(loader, "DeviceCopy", MFX_GPUCOPY_OFF);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);

    // check for RT log string which indicates that DeviceCopy was set properly
    CheckOutputLog("[STUB RT]: message -- MFXInitEx -- GPUCopy set (2)");
    CleanupOutputLog();
}

TEST(Dispatcher_Stub_CreateSession, DeviceCopySetInvalid1x) {
    SKIP_IF_DISP_STUB_DISABLED();

    // stub RT logs results from MFXInitialize
    CaptureOutputLog(CAPTURE_LOG_COUT);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB_1X);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass special config property - NumThread
    sts = SetConfigFilterProperty<mfxU16>(loader, "DeviceCopy", 999);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // expect to fail
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    // free internal resources
    MFXUnload(loader);

    // check for RT log string which indicates that DeviceCopy was set to invalid value
    CheckOutputLog("[STUB RT]: message -- MFXInitEx -- GPUCopy set to invalid value (999)");
    CleanupOutputLog();
}

TEST(Dispatcher_Memory, ImportFrameSurfaceReturnsInvalidHandleOnNull) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxMemoryInterface *iface = nullptr;

    // If no session is passed in dispatcher should return Invalid Handle
    sts = MFXGetMemoryInterface(NULL, &iface);
    EXPECT_EQ(sts, MFX_ERR_INVALID_HANDLE);

    MFXUnload(loader);
}

TEST(Dispatcher_Memory, ImportFrameSurfaceReturnsUndefinedBehavior) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxMemoryInterface *iface = nullptr;

    // Stub will return Undefined Behavior for unknown mfxHandleType
    sts = MFXGetMemoryInterface(session, &iface);
    EXPECT_EQ(sts, MFX_ERR_UNDEFINED_BEHAVIOR);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}
#endif // ONEVPL_EXPERIMENTAL
