/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

///
/// Unit tests for low-latency initialization.
///
/// @file

#include <gtest/gtest.h>

#include "src/dispatcher_common.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

enum ConfigTypesLowLatency {
    LL_SINGLE_CONFIG = 0,
    LL_MULTI_CONFIG,
};

enum TestTypesLowLatency {
    LL_VALID = 0,

    LL_ERR_MISSING_PROP,
    LL_ERR_OVERWRITE_PROP,
    LL_ERR_WRONG_VALUE,

    LL_CONFIG_ONLY,
};

// enable low latency mode, or fail intentionally with different causes
static mfxStatus EnableLowLatency(mfxLoader loader,
                                  ConfigTypesLowLatency configType,
                                  TestTypesLowLatency testType) {
    mfxStatus sts;

    mfxConfig config1 = nullptr;
    mfxConfig config2 = nullptr;
    mfxConfig config3 = nullptr;
    mfxConfig config4 = nullptr;

    config1 = MFXCreateConfig(loader);
    EXPECT_FALSE(config1 == nullptr);

    if (configType == LL_MULTI_CONFIG) {
        // use different config object for each prop
        config2 = MFXCreateConfig(loader);
        EXPECT_FALSE(config2 == nullptr);

        config3 = MFXCreateConfig(loader);
        EXPECT_FALSE(config3 == nullptr);

        config4 = MFXCreateConfig(loader);
        EXPECT_FALSE(config4 == nullptr);
    }
    else {
        // use single config object for all props
        config2 = config1;
        config3 = config1;
        config4 = config1;
    }

    // enable low latency mode
    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;

    var.Type     = MFX_VARIANT_TYPE_U32;
    var.Data.U32 = MFX_IMPL_TYPE_HARDWARE;
    sts = MFXSetConfigFilterProperty(config1, (const mfxU8 *)"mfxImplDescription.Impl", var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    var.Type     = MFX_VARIANT_TYPE_PTR;
    var.Data.Ptr = (mfxHDL)(testType == LL_ERR_WRONG_VALUE ? "mfx-invalid" : "mfx-gen");
    sts = MFXSetConfigFilterProperty(config2, (const mfxU8 *)"mfxImplDescription.ImplName", var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    if (testType != LL_ERR_MISSING_PROP) {
        var.Type     = MFX_VARIANT_TYPE_U32;
        var.Data.U32 = 0x8086;
        sts =
            MFXSetConfigFilterProperty(config3, (const mfxU8 *)"mfxImplDescription.VendorID", var);
        EXPECT_EQ(sts, MFX_ERR_NONE);
    }

    if (testType == LL_ERR_OVERWRITE_PROP) {
        var.Type     = MFX_VARIANT_TYPE_U32;
        var.Data.U32 = 0xabcd;
        sts =
            MFXSetConfigFilterProperty(config3, (const mfxU8 *)"mfxImplDescription.VendorID", var);
        EXPECT_EQ(sts, MFX_ERR_NONE);
    }

    var.Type = MFX_VARIANT_TYPE_U32;
#if defined(_WIN32) || defined(_WIN64)
    var.Data.U32 = MFX_ACCEL_MODE_VIA_D3D11;
#else
    var.Data.U32 = MFX_ACCEL_MODE_VIA_VAAPI;
#endif
    sts = MFXSetConfigFilterProperty(config4,
                                     (const mfxU8 *)"mfxImplDescription.AccelerationMode",
                                     var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // don't test session creation, just set the config props
    // in this case, caller needs to call MFXUnload()
    if (testType == LL_CONFIG_ONLY)
        return sts;

    // we don't actually care if MFXCreateSession succeeds or not here
    //   since all we need is the log output that low latency is enabled (or not),
    //   which will be printed whether or not the session is created successfully
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    if (sts == MFX_ERR_NONE && session != nullptr)
        MFXClose(session);

    // call unload here so that dispatcher log file is closed and contents may then be checked
    MFXUnload(loader);

    if (testType == LL_VALID)
        CheckOutputLog("message:  low latency mode enabled");
    else
        CheckOutputLog("message:  low latency mode disabled");

    CleanupOutputLog();

    return MFX_ERR_NONE;
}

// tests for low-latency mode configuration - single mfxConfig object
TEST(Dispatcher_LowLatency, Valid_SingleConfig) {
    // capture dispatcher log
    CaptureOutputLog(CAPTURE_LOG_DISPATCHER);

    mfxLoader loader = MFXLoad();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_SINGLE_CONFIG, LL_VALID);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Dispatcher_LowLatency, Invalid_SingleConfig_MissingProp) {
    // capture dispatcher log
    CaptureOutputLog(CAPTURE_LOG_DISPATCHER);

    mfxLoader loader = MFXLoad();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_SINGLE_CONFIG, LL_ERR_MISSING_PROP);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Dispatcher_LowLatency, Invalid_SingleConfig_OverwriteProp) {
    // capture dispatcher log
    CaptureOutputLog(CAPTURE_LOG_DISPATCHER);

    mfxLoader loader = MFXLoad();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_SINGLE_CONFIG, LL_ERR_OVERWRITE_PROP);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Dispatcher_LowLatency, Invalid_SingleConfig_WrongValue) {
    // capture dispatcher log
    CaptureOutputLog(CAPTURE_LOG_DISPATCHER);

    mfxLoader loader = MFXLoad();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_SINGLE_CONFIG, LL_ERR_WRONG_VALUE);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// tests for low-latency mode configuration - multiple mfxConfig objects
TEST(Dispatcher_LowLatency, ValidPropsEnable_MultiConfig) {
    // capture dispatcher log
    CaptureOutputLog(CAPTURE_LOG_DISPATCHER);

    mfxLoader loader = MFXLoad();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_MULTI_CONFIG, LL_VALID);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Dispatcher_LowLatency, Invalid_MultiConfig_MissingProp) {
    // capture dispatcher log
    CaptureOutputLog(CAPTURE_LOG_DISPATCHER);

    mfxLoader loader = MFXLoad();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_MULTI_CONFIG, LL_ERR_MISSING_PROP);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Dispatcher_LowLatency, Invalid_MultiConfig_OverwriteProp) {
    // capture dispatcher log
    CaptureOutputLog(CAPTURE_LOG_DISPATCHER);

    mfxLoader loader = MFXLoad();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_MULTI_CONFIG, LL_ERR_OVERWRITE_PROP);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(Dispatcher_LowLatency, Invalid_MultiConfig_WrongValue) {
    // capture dispatcher log
    CaptureOutputLog(CAPTURE_LOG_DISPATCHER);

    mfxLoader loader = MFXLoad();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_MULTI_CONFIG, LL_ERR_WRONG_VALUE);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

// below tests should only be run on systems with GPU RT installed

TEST(Dispatcher_LowLatency, Create_SingleLoader_SingleSession) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_SINGLE_CONFIG, LL_CONFIG_ONLY);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    MFXClose(session);

    MFXUnload(loader);
}

TEST(Dispatcher_LowLatency, Create_SingleLoader_MultipleSessions) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_SINGLE_CONFIG, LL_CONFIG_ONLY);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create first session
    mfxSession session1 = nullptr;
    sts                 = MFXCreateSession(loader, 0, &session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session1, nullptr);
    MFXClose(session1);

    // create second session
    mfxSession session2 = nullptr;
    sts                 = MFXCreateSession(loader, 0, &session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session2, nullptr);
    MFXClose(session2);

    // create third session
    mfxSession session3 = nullptr;
    sts                 = MFXCreateSession(loader, 0, &session3);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session3, nullptr);
    MFXClose(session3);

    MFXUnload(loader);
}

TEST(Dispatcher_LowLatency, Create_MultipleLoaders_MultipleSessions) {
    SKIP_IF_DISP_GPU_DISABLED();

    // create first loader and first session
    mfxLoader loader1 = MFXLoad();
    EXPECT_FALSE(loader1 == nullptr);

    mfxStatus sts = EnableLowLatency(loader1, LL_SINGLE_CONFIG, LL_CONFIG_ONLY);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSession session1 = nullptr;
    sts                 = MFXCreateSession(loader1, 0, &session1);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session1, nullptr);

    MFXClose(session1);
    MFXUnload(loader1);

    // create second loader and second session
    mfxLoader loader2 = MFXLoad();
    EXPECT_FALSE(loader2 == nullptr);

    sts = EnableLowLatency(loader2, LL_SINGLE_CONFIG, LL_CONFIG_ONLY);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSession session2 = nullptr;
    sts                 = MFXCreateSession(loader2, 0, &session2);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session2, nullptr);

    MFXClose(session2);
    MFXUnload(loader2);
}

TEST(Dispatcher_LowLatency, Get_ImplDesc_MFXEnumImplementations_AfterSessionCreated) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_SINGLE_CONFIG, LL_CONFIG_ONLY);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    mfxImplDescription *idesc = nullptr;
    sts                       = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&idesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);
    ASSERT_NE(idesc, nullptr);

    bool bIsImplNameCorrect =
        ((strcmp(idesc->ImplName, "mfxhw64") == 0) || (strcmp(idesc->ImplName, "mfx-gen") == 0))
            ? true
            : false;
    EXPECT_EQ(bIsImplNameCorrect, true);

    sts = MFXDispReleaseImplDescription(loader, idesc);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXClose(session);
    MFXUnload(loader);
}

#ifdef ONEVPL_EXPERIMENTAL

TEST(Dispatcher_LowLatency, Get_ImplDesc_WithPropQuery_BeforeSession) {
    SKIP_IF_DISP_GPU_DISABLED();

    // capture dispatcher log
    CaptureOutputLog(CAPTURE_LOG_DISPATCHER);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_SINGLE_CONFIG, LL_CONFIG_ONLY);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // now try props-based query (shallow info only)
    // this should trigger caps query for the first time since session was created in LL mode
    // whether or not the loaded RT supports props-based query, the shallow data should be the same
    // (will fall back to full query for older RT's)
    mfxConfig cfg = MFXCreateConfig(loader);

    // query property - impl only
    std::string propStr = "mfxImplDescription";

    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
    var.Data.U32 = 0;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);

    mfxImplDescription *idesc = nullptr;
    sts                       = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&idesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);
    ASSERT_NE(idesc, nullptr);

    bool bIsImplNameCorrect =
        ((strcmp(idesc->ImplName, "mfxhw64") == 0) || (strcmp(idesc->ImplName, "mfx-gen") == 0))
            ? true
            : false;
    EXPECT_EQ(bIsImplNameCorrect, true);

    sts = MFXDispReleaseImplDescription(loader, idesc);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create a session
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    MFXClose(session);
    MFXUnload(loader);

    // because we triggered caps query before calling MFXCreateSession(), it disabled low-latency
    CheckOutputLog("message:  low latency mode disabled");
    CleanupOutputLog();
}

TEST(Dispatcher_LowLatency, Get_ImplDesc_WithPropQuery_AfterSession) {
    SKIP_IF_DISP_GPU_DISABLED();

    // capture dispatcher log
    CaptureOutputLog(CAPTURE_LOG_DISPATCHER);

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_SINGLE_CONFIG, LL_CONFIG_ONLY);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(session, nullptr);

    // now try props-based query (shallow info only)
    // this should trigger caps query for the first time since session was created in LL mode
    // whether or not the loaded RT supports props-based query, the shallow data should be the same
    // (will fall back to full query for older RT's)
    mfxConfig cfg = MFXCreateConfig(loader);

    // query property - impl only
    std::string propStr = "mfxImplDescription";

    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
    var.Data.U32 = 0;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);

    mfxImplDescription *idesc = nullptr;
    sts                       = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&idesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);
    ASSERT_NE(idesc, nullptr);

    bool bIsImplNameCorrect =
        ((strcmp(idesc->ImplName, "mfxhw64") == 0) || (strcmp(idesc->ImplName, "mfx-gen") == 0))
            ? true
            : false;
    EXPECT_EQ(bIsImplNameCorrect, true);

    sts = MFXDispReleaseImplDescription(loader, idesc);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXClose(session);
    MFXUnload(loader);

    // here we called MFXCreateSession() in low latency mode, then ran the caps query afterwards
    CheckOutputLog("message:  low latency mode enabled");
    CleanupOutputLog();
}

#endif