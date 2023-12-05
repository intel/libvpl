/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "src/unit_api.h"

#if LEGACY_TEST_VER == 2
    // disambiguate classes if utest app includes both original and preview implementations
    #define MFXVideoSession    MFXVideoSession_Preview
    #define MFXVideoDECODE     MFXVideoDECODE_Preview
    #define MFXVideoENCODE     MFXVideoENCODE_Preview
    #define MFXVideoVPP        MFXVideoVPP_Preview
    #define MFXVideoDECODE_VPP MFXVideoDECODE_VPP_Preview

    #include "vpl/preview/legacy/mfxvideo++.h"
#else
    #include "vpl/mfxvideo++.h"
#endif

#define MFX_IMPL_ACCELMODE(x) (0xff00 & (x))

#define TEST_NAME2(suite, ver)        suite##ver##x
#define TEST_NAME1(suite, ver)        TEST_NAME2(suite, ver)
#define TEST_CUSTOM(suite, ver, name) TEST(TEST_NAME1(suite, ver), name)

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitSucceeds) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    // MFXVideoSession dtor calls MFXClose() so no need to call it in these tests
    mfxIMPL impl   = MFX_IMPL_AUTO_ANY;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, DoubleInitSucceeds) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    mfxIMPL impl   = MFX_IMPL_AUTO_ANY;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
    sts = session.Close(); // avoid memleak
    sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, CloseSucceeds) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    mfxIMPL impl   = MFX_IMPL_AUTO_ANY;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
    sts = session.Close();
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Close failed with code " << sts;
}

// update when mfxvideo++.h switches to MFXLoad() instead of MFXInitEx()
TEST_CUSTOM(DISABLED_LegacycppCreateSession, LEGACY_TEST_VER, DoubleCloseSucceeds) {
    SKIP_IF_LEGACY_TEST_DISABLED();

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

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitSucceedsImplAuto) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    mfxIMPL impl   = MFX_IMPL_AUTO;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

// update when mfxvideo++.h switches to MFXLoad() instead of MFXInitEx()
TEST_CUSTOM(DISABLED_LegacycppCreateSession, LEGACY_TEST_VER, InitSucceedsImplSoftware) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    mfxIMPL impl   = MFX_IMPL_SOFTWARE;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitSucceedsImplHardware) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    mfxIMPL impl   = MFX_IMPL_HARDWARE;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitSucceedsImplHardwareAny) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    mfxIMPL impl   = MFX_IMPL_HARDWARE_ANY;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ Init failed with code " << sts;
}

//Tests for HARDWARE2,3,4 require specific hardware setup and are not suitable for
//general unit tests which can be run anywhere

// update when mfxvideo++.h switches to MFXLoad() instead of MFXInitEx()
TEST_CUSTOM(DISABLED_LegacycppCreateSession, LEGACY_TEST_VER, InitFailsImplInvalid) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    mfxIMPL impl   = MFX_IMPL_RUNTIME;
    mfxVersion ver = { { 0, 1 } };
    MFXVideoSession session;
    mfxStatus sts = session.Init(impl, &ver);
    ASSERT_EQ(sts, MFX_ERR_INVALID_VIDEO_PARAM) << "mfxvideo++ Init failed with code " << sts;
}

// return default baseType defined by global utest settings
static mfxU32 GetDefaultBaseType(void) {
    mfxIMPL defaultImpl = 0xffff;

    if (g_bLegacyTestImplHW)
        defaultImpl = MFX_IMPL_HARDWARE;
    else if (g_bLegacyTestImplHW2)
        defaultImpl = MFX_IMPL_HARDWARE2;
    else if (g_bLegacyTestImplHW3)
        defaultImpl = MFX_IMPL_HARDWARE3;
    else if (g_bLegacyTestImplHW4)
        defaultImpl = MFX_IMPL_HARDWARE4;
    else if (g_bDispInclSW)
        defaultImpl = MFX_IMPL_SOFTWARE;

    return MFX_IMPL_BASETYPE(defaultImpl);
}

static void TestInitEx_SingleImpl(mfxIMPL requestedImpl) {
    mfxInitParam par   = {};
    mfxIMPL actualImpl = {};

    par.Version.Major = 1;
    par.Version.Minor = 0;

    par.Implementation = requestedImpl;

    // create new session with requestedImpl
    MFXVideoSession session;
    mfxStatus sts = session.InitEx(par);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ InitEx failed with code " << sts;

    // query for actualImpl
    sts = session.QueryIMPL(&actualImpl);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ QueryIMPL failed with code " << sts;

    // validate actual implementation indicates correct basetype
    mfxU32 requestedBaseType = MFX_IMPL_BASETYPE(requestedImpl);
    mfxU32 actualBaseType    = MFX_IMPL_BASETYPE(actualImpl);
    mfxU32 defaultBaseType   = GetDefaultBaseType();

    bool bIsCorrectBaseType = false;
    switch (requestedBaseType) {
        case MFX_IMPL_HARDWARE:
        case MFX_IMPL_HARDWARE2:
        case MFX_IMPL_HARDWARE3:
        case MFX_IMPL_HARDWARE4:
        case MFX_IMPL_SOFTWARE:
            bIsCorrectBaseType = (actualBaseType == requestedBaseType ? true : false);
            break;

        case MFX_IMPL_AUTO:
        case MFX_IMPL_AUTO_ANY:
            bIsCorrectBaseType = (actualBaseType == defaultBaseType ? true : false);
            break;

        case MFX_IMPL_HARDWARE_ANY:
            bIsCorrectBaseType =
                ((actualBaseType == defaultBaseType) && (actualBaseType != MFX_IMPL_SOFTWARE)
                     ? true
                     : false);
            break;

        default:
            bIsCorrectBaseType = false; // unknown error
    }

    ASSERT_EQ(bIsCorrectBaseType, true) << "mfxvideo++ MFXInitEx failed with wrong baseType";

    // accelerator mode N/A for SW implementation
    if (actualBaseType == MFX_IMPL_SOFTWARE)
        return;

    // validate actual implementation indicates correct accelMode
    // validate actual implementation indicates correct basetype
    mfxU32 requestedAccelMode = MFX_IMPL_ACCELMODE(requestedImpl);
    mfxU32 actualAccelMode    = MFX_IMPL_ACCELMODE(actualImpl);

    // if not specified, VIA_ANY is the default
    if (requestedAccelMode == 0)
        requestedAccelMode = MFX_IMPL_VIA_ANY;

    bool bIsCorrectAccelMode = true;
    switch (requestedAccelMode) {
        case MFX_IMPL_VIA_D3D9:
        case MFX_IMPL_VIA_D3D11:
        case MFX_IMPL_VIA_VAAPI:
            bIsCorrectAccelMode = (actualAccelMode == requestedAccelMode ? true : false);
            break;

        case MFX_IMPL_VIA_ANY:
#if defined(_WIN32) || defined(_WIN64)
            bIsCorrectAccelMode =
                ((actualAccelMode == MFX_IMPL_VIA_D3D9) || (actualAccelMode == MFX_IMPL_VIA_D3D11)
                     ? true
                     : false);
#else
            bIsCorrectAccelMode = (actualAccelMode == MFX_IMPL_VIA_VAAPI);
#endif
            break;

        default:
            bIsCorrectAccelMode = false; // unknown error
    }

    ASSERT_EQ(bIsCorrectAccelMode, true) << "mfxvideo++ MFXInitEx failed with wrong accelMode";
}

// test HW 1-4 + ANY, accelMode = not specified (equivalent to VIA_ANY)
TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware) {
    SKIP_IF_LEGACY_IMPL_HW_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware2) {
    SKIP_IF_LEGACY_IMPL_HW2_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE2);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware3) {
    SKIP_IF_LEGACY_IMPL_HW3_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE3);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware4) {
    SKIP_IF_LEGACY_IMPL_HW4_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE4);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardwareAny) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE_ANY);
}

// test HW 1-4 + ANY, accelMode = VIA_ANY
TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware_AccelAny) {
    SKIP_IF_LEGACY_IMPL_HW_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE | MFX_IMPL_VIA_ANY);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware2_AccelAny) {
    SKIP_IF_LEGACY_IMPL_HW2_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE2 | MFX_IMPL_VIA_ANY);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware3_AccelAny) {
    SKIP_IF_LEGACY_IMPL_HW3_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE3 | MFX_IMPL_VIA_ANY);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware4_AccelAny) {
    SKIP_IF_LEGACY_IMPL_HW4_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE4 | MFX_IMPL_VIA_ANY);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardwareAny_AccelAny) {
    SKIP_IF_LEGACY_TEST_DISABLED();
    SKIP_IF_LEGACY_D3D9_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_ANY);
}

// test HW 1-4 + ANY, accelMode = VIA_D3D9
TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware_D3D9) {
    SKIP_IF_LEGACY_IMPL_HW_DISABLED();
    SKIP_IF_LEGACY_D3D9_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE | MFX_IMPL_VIA_D3D9);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware2_D3D9) {
    SKIP_IF_LEGACY_IMPL_HW2_DISABLED();
    SKIP_IF_LEGACY_D3D9_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE2 | MFX_IMPL_VIA_D3D9);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware3_D3D9) {
    SKIP_IF_LEGACY_IMPL_HW3_DISABLED();
    SKIP_IF_LEGACY_D3D9_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE3 | MFX_IMPL_VIA_D3D9);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware4_D3D9) {
    SKIP_IF_LEGACY_IMPL_HW4_DISABLED();
    SKIP_IF_LEGACY_D3D9_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE4 | MFX_IMPL_VIA_D3D9);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardwareAny_D3D9) {
    SKIP_IF_LEGACY_TEST_DISABLED();
    SKIP_IF_LEGACY_D3D9_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D9);
}

// test HW 1-4 + ANY, accelMode = VIA_D3D11
TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware_D3D11) {
    SKIP_IF_LEGACY_IMPL_HW_DISABLED();
    SKIP_IF_LEGACY_D3D11_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE | MFX_IMPL_VIA_D3D11);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware2_D3D11) {
    SKIP_IF_LEGACY_IMPL_HW2_DISABLED();
    SKIP_IF_LEGACY_D3D11_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE2 | MFX_IMPL_VIA_D3D11);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware3_D3D11) {
    SKIP_IF_LEGACY_IMPL_HW3_DISABLED();
    SKIP_IF_LEGACY_D3D11_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE3 | MFX_IMPL_VIA_D3D11);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware4_D3D11) {
    SKIP_IF_LEGACY_IMPL_HW4_DISABLED();
    SKIP_IF_LEGACY_D3D11_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE4 | MFX_IMPL_VIA_D3D11);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardwareAny_D3D11) {
    SKIP_IF_LEGACY_TEST_DISABLED();
    SKIP_IF_LEGACY_D3D11_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D11);
}

// test HW 1-4 + ANY, accelMode = VIA_VAAPI
TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware_VAAPI) {
    SKIP_IF_LEGACY_IMPL_HW_DISABLED();
    SKIP_IF_LEGACY_VAAPI_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE | MFX_IMPL_VIA_VAAPI);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware2_VAAPI) {
    SKIP_IF_LEGACY_IMPL_HW2_DISABLED();
    SKIP_IF_LEGACY_VAAPI_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE2 | MFX_IMPL_VIA_VAAPI);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware3_VAAPI) {
    SKIP_IF_LEGACY_IMPL_HW3_DISABLED();
    SKIP_IF_LEGACY_VAAPI_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE3 | MFX_IMPL_VIA_VAAPI);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardware4_VAAPI) {
    SKIP_IF_LEGACY_IMPL_HW4_DISABLED();
    SKIP_IF_LEGACY_VAAPI_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE4 | MFX_IMPL_VIA_VAAPI);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplHardwareAny_VAAPI) {
    SKIP_IF_LEGACY_TEST_DISABLED();
    SKIP_IF_LEGACY_VAAPI_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_VAAPI);
}

// test SW
TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplSoftware) {
    SKIP_IF_LEGACY_TEST_DISABLED();
    SKIP_IF_DISP_SW_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_SOFTWARE);
}

// test AUTO and AUTO_ANY
TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplAuto) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_AUTO);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplAutoAny) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_AUTO_ANY);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplAuto_AccelAny) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_AUTO | MFX_IMPL_VIA_ANY);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExSucceedsImplAutoAny_AccelAny) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    TestInitEx_SingleImpl(MFX_IMPL_AUTO_ANY | MFX_IMPL_VIA_ANY);
}

TEST_CUSTOM(LegacycppCreateSession, LEGACY_TEST_VER, InitExReturns1xAPI) {
    SKIP_IF_LEGACY_TEST_DISABLED();

    mfxInitParam par = {};

    par.Version.Major = 1;
    par.Version.Minor = 0;

    par.Implementation = MFX_IMPL_HARDWARE;

    MFXVideoSession session;
    mfxStatus sts = session.InitEx(par);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "mfxvideo++ InitEx failed with code " << sts;

    // expect initialization via MFXInitEx to return API of 1.x
    // for legacy HW, will be the actual MSDK API version (ex. 1.35)
    // for Intel® Video Processing Library (Intel® VPL) HW, will always be 1.255
    mfxVersion actualVersion = {};
    session.QueryVersion(&actualVersion);
    ASSERT_EQ(actualVersion.Major, 1)
        << "mfxvideo++ QueryVersion expects version = 1.x, actual version =  "
        << actualVersion.Major << "." << actualVersion.Minor;
}
