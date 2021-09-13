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

#include "vpl/mfxdispatcher.h"

#if defined(_WIN32) || defined(_WIN64)

    #include <windows.h>

static void EnableDispatcherLog() {
    #if defined(_WIN32) || defined(_WIN64)
    SetEnvironmentVariable("ONEVPL_DISPATCHER_LOG", "ON");
    #else
    setenv("ONEVPL_DISPATCHER_LOG", "ON", 1);
    #endif
}

static void DisableDispatcherLog() {
    #if defined(_WIN32) || defined(_WIN64)
    SetEnvironmentVariable("ONEVPL_DISPATCHER_LOG", NULL);
    #else
    unsetenv("ONEVPL_DISPATCHER_LOG");
    #endif
}

enum ConfigTypesLowLatency {
    LL_SINGLE_CONFIG = 0,
    LL_MULTI_CONFIG,
};

enum TestTypesLowLatency {
    LL_VALID = 0,

    LL_ERR_MISSING_PROP,
    LL_ERR_OVERWRITE_PROP,
    LL_ERR_WRONG_VALUE,
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

    testing::internal::CaptureStdout();

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

    // we don't actually care if MFXCreateSession succeeds or not here
    //   since all we need is the log output that low latency is enabled (or not),
    //   which will be printed whether or not the session is created successfully
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    if (sts == MFX_ERR_NONE && session != nullptr)
        MFXClose(session);

    // check for dispatcher log string which indicates that low latency mode was enabled
    std::string consoleOutput = testing::internal::GetCapturedStdout();
    size_t logPos             = consoleOutput.find("message:  low latency mode enabled");

    if (testType == LL_VALID)
        EXPECT_NE(logPos, std::string::npos);
    else
        EXPECT_EQ(logPos, std::string::npos);

    return MFX_ERR_NONE;
}

// tests for low-latency mode configuration - single mfxConfig object
TEST(Dispatcher_LowLatency, Valid_SingleConfig) {
    EnableDispatcherLog();
    mfxLoader loader = MFXLoad();
    DisableDispatcherLog();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_SINGLE_CONFIG, LL_VALID);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

TEST(Dispatcher_LowLatency, Invalid_SingleConfig_MissingProp) {
    EnableDispatcherLog();
    mfxLoader loader = MFXLoad();
    DisableDispatcherLog();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_SINGLE_CONFIG, LL_ERR_MISSING_PROP);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

TEST(Dispatcher_LowLatency, Invalid_SingleConfig_OverwriteProp) {
    EnableDispatcherLog();
    mfxLoader loader = MFXLoad();
    DisableDispatcherLog();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_SINGLE_CONFIG, LL_ERR_OVERWRITE_PROP);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

TEST(Dispatcher_LowLatency, Invalid_SingleConfig_WrongValue) {
    EnableDispatcherLog();
    mfxLoader loader = MFXLoad();
    DisableDispatcherLog();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_SINGLE_CONFIG, LL_ERR_WRONG_VALUE);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

// tests for low-latency mode configuration - multiple mfxConfig objects
TEST(Dispatcher_LowLatency, ValidPropsEnable_MultiConfig) {
    EnableDispatcherLog();
    mfxLoader loader = MFXLoad();
    DisableDispatcherLog();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_MULTI_CONFIG, LL_VALID);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

TEST(Dispatcher_LowLatency, Invalid_MultiConfig_MissingProp) {
    EnableDispatcherLog();
    mfxLoader loader = MFXLoad();
    DisableDispatcherLog();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_MULTI_CONFIG, LL_ERR_MISSING_PROP);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

TEST(Dispatcher_LowLatency, Invalid_MultiConfig_OverwriteProp) {
    EnableDispatcherLog();
    mfxLoader loader = MFXLoad();
    DisableDispatcherLog();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_MULTI_CONFIG, LL_ERR_OVERWRITE_PROP);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

TEST(Dispatcher_LowLatency, Invalid_MultiConfig_WrongValue) {
    EnableDispatcherLog();
    mfxLoader loader = MFXLoad();
    DisableDispatcherLog();

    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = EnableLowLatency(loader, LL_MULTI_CONFIG, LL_ERR_WRONG_VALUE);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

#endif // defined(_WIN32) || defined(_WIN64)
