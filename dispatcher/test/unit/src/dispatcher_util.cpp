/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "src/dispatcher_common.h"

void CaptureOutputLog(bool bEnableDispatcherLog) {
    if (bEnableDispatcherLog) {
#if defined(_WIN32) || defined(_WIN64)
        SetEnvironmentVariable("ONEVPL_DISPATCHER_LOG", "ON");
#else
        setenv("ONEVPL_DISPATCHER_LOG", "ON", 1);
#endif
    }

    // start capturing log output
    testing::internal::CaptureStdout();
}

void GetOutputLog(std::string &outputLog) {
#if defined(_WIN32) || defined(_WIN64)
    SetEnvironmentVariable("ONEVPL_DISPATCHER_LOG", NULL);
#else
    unsetenv("ONEVPL_DISPATCHER_LOG");
#endif

    outputLog = testing::internal::GetCapturedStdout();
}

void CheckOutputLog(std::string outputLog, const char *expectedString, bool expectMatch) {
    size_t logPos = outputLog.find(expectedString);

    printf("Info: CheckOutputLog(%s) -- string %s\n",
           (expectMatch ? "true" : "false"),
           expectedString);

    if (expectMatch)
        EXPECT_NE(logPos, std::string::npos);
    else
        EXPECT_EQ(logPos, std::string::npos);
}

// helper functions to get/set global string to working dir
static std::string g_workDirPath; // NOLINT

void SetWorkingDirectoryPath(std::string workDirPath) {
    g_workDirPath = workDirPath;
}

void GetWorkingDirectoryPath(std::string &workDirPath) {
    workDirPath = g_workDirPath;
}

// helper functions add/check deviceID's of this system
static std::list<std::string> g_deviceIDList; // NOLINT

// return number of elements in deviceID list
int AddDeviceID(std::string deviceID) {
    g_deviceIDList.push_back(deviceID);

    return (int)g_deviceIDList.size();
}

void GetDeviceIDList(std::list<std::string> &deviceIDList) {
    deviceIDList = g_deviceIDList;
}

// check if working directory exists at 'dirPath/oneVPLTests-work'
//   if not, attempt to create it
// return zero on success, non-zero if dirPath is invalid or not writable
// for simplicity, does not check if working directory itself is writable
//   (assuming it already exists) -- tests which try to write to it will
//   just fail in that case
int CreateWorkingDirectory(const char *dirPath) {
    // check null pointer or empty string
    if (!dirPath || !dirPath[0])
        return -1;

    std::string workDirPath = dirPath;

    // erase any trailing backslash
    size_t lastSep = workDirPath.find_last_of(PATH_SEPARATOR);
    if (lastSep == workDirPath.size() - 1)
        workDirPath.erase(lastSep);

    // create path to working directory
    workDirPath = workDirPath + std::string(PATH_SEPARATOR) + std::string("oneVPLTests-work");

#if defined(_WIN32) || defined(_WIN64)
    // check whether working directory exists
    // if not, attempt to create it
    if (!PathIsDirectoryA(workDirPath.c_str())) {
        // check that parent directory exists
        if (!PathIsDirectoryA(dirPath))
            return -1;

        // create working directory
        if (CreateDirectoryA(workDirPath.c_str(), NULL) == 0)
            return -1; // CreateDirectory returns 0 on fail

        // verify working directory exists
        if (!PathIsDirectoryA(workDirPath.c_str()))
            return -1;
    }

    // convert to canonical, absolute path (ASCII version limited to MAX_PATH - see WinAPI docs)
    char fullPath[MAX_PATH + 1] = {};
    int n = GetFullPathNameA(workDirPath.c_str(), sizeof(fullPath), fullPath, NULL);
    if (n > MAX_PATH) {
        printf("Error - absolute workDir path is too long (max length %d chars)\n", MAX_PATH);
        return -1;
    }
    else if (n == 0) {
        return -1;
    }
    workDirPath = fullPath;
#else
    // check whether working directory exists
    // if not, attempt to create it
    DIR *workDir = opendir(workDirPath.c_str());
    if (workDir) {
        closedir(workDir);
    }
    else {
        // check that parent directory exists
        DIR *parentDir = opendir(dirPath);
        if (parentDir == NULL)
            return -1;
        closedir(parentDir);

        // create working directory
        // to make world-writable, need to call chmod as second step
        // both functions return 0 on success, non-zero on error
        if (mkdir(workDirPath.c_str(), 0) || chmod(workDirPath.c_str(), 0666))
            return -1;

        // verify working directory exists
        workDir = opendir(workDirPath.c_str());
        if (workDir == NULL)
            return -1;
        closedir(workDir);
    }

    // convert to canonical, absolute path (limited to PATH_MAX bytes per manual)
    char fullPath[PATH_MAX] = {};
    char *t = realpath(workDirPath.c_str(), fullPath);
    if (!t || t != fullPath)
        return -1;
    workDirPath = fullPath;
#endif

    // success - store working directory in global path
    SetWorkingDirectoryPath(workDirPath);

    return 0;
}

// set implementation type
mfxStatus SetConfigImpl(mfxLoader loader, mfxU32 implType, bool bRequire2xGPU) {
    mfxVariant ImplValue;
    mfxConfig cfg = MFXCreateConfig(loader);
    if (!cfg)
        return MFX_ERR_UNSUPPORTED;

    mfxStatus sts = MFX_ERR_NONE;
    if (implType == MFX_IMPL_TYPE_STUB) {
        // for stub library, filter by ImplName
        ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
        ImplValue.Type            = MFX_VARIANT_TYPE_PTR;
        ImplValue.Data.Ptr        = (mfxHDL) "Stub Implementation";

        sts = MFXSetConfigFilterProperty(
            cfg,
            reinterpret_cast<const mfxU8 *>("mfxImplDescription.ImplName"),
            ImplValue);
    }
    else if (implType == MFX_IMPL_TYPE_STUB_1X) {
        // for 1.x API stub library, filter by ImplName (1.x API version)
        ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
        ImplValue.Type            = MFX_VARIANT_TYPE_PTR;
        ImplValue.Data.Ptr        = (mfxHDL) "Stub Implementation 1X";

        sts = MFXSetConfigFilterProperty(
            cfg,
            reinterpret_cast<const mfxU8 *>("mfxImplDescription.ImplName"),
            ImplValue);
    }
    else if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        // for SW, filter by ImplName and ImplType (to exclude stub SW lib)
        ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
        ImplValue.Type            = MFX_VARIANT_TYPE_PTR;
        ImplValue.Data.Ptr        = (mfxHDL) "oneAPI VPL CPU Implementation";

        sts = MFXSetConfigFilterProperty(
            cfg,
            reinterpret_cast<const mfxU8 *>("mfxImplDescription.ImplName"),
            ImplValue);

        ImplValue.Type     = MFX_VARIANT_TYPE_U32;
        ImplValue.Data.U32 = implType;

        sts = MFXSetConfigFilterProperty(cfg,
                                         reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                         ImplValue);
    }
    else if (implType == MFX_IMPL_TYPE_HARDWARE) {
        // for HW, filter by ImplType (any is okay)
        ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
        ImplValue.Type            = MFX_VARIANT_TYPE_U32;
        ImplValue.Data.U32        = implType;

        sts = MFXSetConfigFilterProperty(cfg,
                                         reinterpret_cast<const mfxU8 *>("mfxImplDescription.Impl"),
                                         ImplValue);

        if (bRequire2xGPU) {
            // for systems with both MSDK and oneVPL GPU's, limit the test to only the VPL device
            ImplValue.Version.Version = (mfxU16)MFX_VARIANT_VERSION;
            ImplValue.Type            = MFX_VARIANT_TYPE_PTR;
            ImplValue.Data.Ptr        = (mfxHDL) "mfx-gen";

            sts = MFXSetConfigFilterProperty(
                cfg,
                reinterpret_cast<const mfxU8 *>("mfxImplDescription.ImplName"),
                ImplValue);
        }
    }

    return sts;
}

// implement templatized helper functions for dispatcher tests

// utility functions to fill mfxVariant
template <typename varDataType>
void FillVariant(mfxVariant *var, varDataType data);

#define CREATE_FILL_VARIANT_FN(x_field, x_type, x_varDataType) \
    template <>                                                \
    void FillVariant(mfxVariant *var, x_type data) {           \
        var->Version.Version = (mfxU16)MFX_VARIANT_VERSION;    \
        var->Type            = x_varDataType;                  \
        var->Data.x_field    = data;                           \
    }

// expand into a unique implementation for each type
CREATE_FILL_VARIANT_FN(U8, mfxU8, MFX_VARIANT_TYPE_U8)
CREATE_FILL_VARIANT_FN(U16, mfxU16, MFX_VARIANT_TYPE_U16)
CREATE_FILL_VARIANT_FN(U32, mfxU32, MFX_VARIANT_TYPE_U32)
CREATE_FILL_VARIANT_FN(U64, mfxU64, MFX_VARIANT_TYPE_U64)
CREATE_FILL_VARIANT_FN(I8, mfxI8, MFX_VARIANT_TYPE_I8)
CREATE_FILL_VARIANT_FN(I16, mfxI16, MFX_VARIANT_TYPE_I16)
CREATE_FILL_VARIANT_FN(I32, mfxI32, MFX_VARIANT_TYPE_I32)
CREATE_FILL_VARIANT_FN(I64, mfxI64, MFX_VARIANT_TYPE_I64)
CREATE_FILL_VARIANT_FN(F32, mfxF32, MFX_VARIANT_TYPE_F32)
CREATE_FILL_VARIANT_FN(F64, mfxF64, MFX_VARIANT_TYPE_F64)
CREATE_FILL_VARIANT_FN(Ptr, mfxHDL, MFX_VARIANT_TYPE_PTR)

template <typename varDataType>
mfxStatus SetConfigFilterProperty(mfxLoader loader, const char *name, varDataType data) {
    mfxConfig cfg = MFXCreateConfig(loader);
    EXPECT_FALSE(cfg == nullptr);

    mfxVariant var;
    FillVariant(&var, data);

    mfxStatus sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)name, var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    return sts;
}

template mfxStatus SetConfigFilterProperty<mfxU8>(mfxLoader, const char *, mfxU8);
template mfxStatus SetConfigFilterProperty<mfxU16>(mfxLoader, const char *, mfxU16);
template mfxStatus SetConfigFilterProperty<mfxU32>(mfxLoader, const char *, mfxU32);
template mfxStatus SetConfigFilterProperty<mfxU64>(mfxLoader, const char *, mfxU64);
template mfxStatus SetConfigFilterProperty<mfxI8>(mfxLoader, const char *, mfxI8);
template mfxStatus SetConfigFilterProperty<mfxI16>(mfxLoader, const char *, mfxI16);
template mfxStatus SetConfigFilterProperty<mfxI32>(mfxLoader, const char *, mfxI32);
template mfxStatus SetConfigFilterProperty<mfxI64>(mfxLoader, const char *, mfxI64);
template mfxStatus SetConfigFilterProperty<mfxF32>(mfxLoader, const char *, mfxF32);
template mfxStatus SetConfigFilterProperty<mfxF64>(mfxLoader, const char *, mfxF64);
template mfxStatus SetConfigFilterProperty<mfxHDL>(mfxLoader, const char *, mfxHDL);

// in this version we use the mfxConfig obj which is passed in vs. creating a new one
template <typename varDataType>
mfxStatus SetConfigFilterProperty(mfxLoader loader,
                                  mfxConfig cfg,
                                  const char *name,
                                  varDataType data) {
    mfxVariant var;
    FillVariant(&var, data);

    mfxStatus sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)name, var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    return sts;
}

template mfxStatus SetConfigFilterProperty<mfxU8>(mfxLoader, mfxConfig, const char *, mfxU8);
template mfxStatus SetConfigFilterProperty<mfxU16>(mfxLoader, mfxConfig, const char *, mfxU16);
template mfxStatus SetConfigFilterProperty<mfxU32>(mfxLoader, mfxConfig, const char *, mfxU32);
template mfxStatus SetConfigFilterProperty<mfxU64>(mfxLoader, mfxConfig, const char *, mfxU64);
template mfxStatus SetConfigFilterProperty<mfxI8>(mfxLoader, mfxConfig, const char *, mfxI8);
template mfxStatus SetConfigFilterProperty<mfxI16>(mfxLoader, mfxConfig, const char *, mfxI16);
template mfxStatus SetConfigFilterProperty<mfxI32>(mfxLoader, mfxConfig, const char *, mfxI32);
template mfxStatus SetConfigFilterProperty<mfxI64>(mfxLoader, mfxConfig, const char *, mfxI64);
template mfxStatus SetConfigFilterProperty<mfxF32>(mfxLoader, mfxConfig, const char *, mfxF32);
template mfxStatus SetConfigFilterProperty<mfxF64>(mfxLoader, mfxConfig, const char *, mfxF64);
template mfxStatus SetConfigFilterProperty<mfxHDL>(mfxLoader, mfxConfig, const char *, mfxHDL);

// set any int property - passed as U64 to hold any size
template <>
mfxStatus SetSingleProperty(mfxLoader loader,
                            const char *name,
                            mfxVariantType varType,
                            mfxU64 data) {
    switch (varType) {
        case MFX_VARIANT_TYPE_U8:
            return SetConfigFilterProperty(loader, name, (mfxU8)data);
        case MFX_VARIANT_TYPE_U16:
            return SetConfigFilterProperty(loader, name, (mfxU16)data);
        case MFX_VARIANT_TYPE_U32:
            return SetConfigFilterProperty(loader, name, (mfxU32)data);
        case MFX_VARIANT_TYPE_U64:
            return SetConfigFilterProperty(loader, name, (mfxU64)data);
        case MFX_VARIANT_TYPE_I8:
            return SetConfigFilterProperty(loader, name, (mfxI8)data);
        case MFX_VARIANT_TYPE_I16:
            return SetConfigFilterProperty(loader, name, (mfxI16)data);
        case MFX_VARIANT_TYPE_I32:
            return SetConfigFilterProperty(loader, name, (mfxI32)data);
        case MFX_VARIANT_TYPE_I64:
            return SetConfigFilterProperty(loader, name, (mfxI64)data);

        case MFX_VARIANT_TYPE_F32:
        case MFX_VARIANT_TYPE_F64:
        case MFX_VARIANT_TYPE_PTR:
        case MFX_VARIANT_TYPE_FP16:
        case MFX_VARIANT_TYPE_UNSET:
            return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_UNSUPPORTED;
}

// set ptr or float/double properties (should be cast to a pointer)
template <>
mfxStatus SetSingleProperty(mfxLoader loader,
                            const char *name,
                            mfxVariantType varType,
                            mfxHDL data) {
    mfxF32 fData32 = 0;
    mfxF64 fData64 = 0;
    switch (varType) {
        case MFX_VARIANT_TYPE_F32:
            fData32 = *(reinterpret_cast<mfxF32 *>(data));
            return SetConfigFilterProperty(loader, name, fData32);
        case MFX_VARIANT_TYPE_F64:
            fData64 = *(reinterpret_cast<mfxF64 *>(data));
            return SetConfigFilterProperty(loader, name, fData64);
        case MFX_VARIANT_TYPE_PTR:
            return SetConfigFilterProperty(loader, name, (mfxHDL)data);

        case MFX_VARIANT_TYPE_U8:
        case MFX_VARIANT_TYPE_U16:
        case MFX_VARIANT_TYPE_U32:
        case MFX_VARIANT_TYPE_U64:
        case MFX_VARIANT_TYPE_I8:
        case MFX_VARIANT_TYPE_I16:
        case MFX_VARIANT_TYPE_I32:
        case MFX_VARIANT_TYPE_I64:
        case MFX_VARIANT_TYPE_FP16:
        case MFX_VARIANT_TYPE_UNSET:
            return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_UNSUPPORTED;
}

// set any int property - passed as U64 to hold any size
template <>
mfxStatus SetSingleProperty(mfxLoader loader,
                            mfxConfig cfg,
                            const char *name,
                            mfxVariantType varType,
                            mfxU64 data) {
    switch (varType) {
        case MFX_VARIANT_TYPE_U8:
            return SetConfigFilterProperty(loader, cfg, name, (mfxU8)data);
        case MFX_VARIANT_TYPE_U16:
            return SetConfigFilterProperty(loader, cfg, name, (mfxU16)data);
        case MFX_VARIANT_TYPE_U32:
            return SetConfigFilterProperty(loader, cfg, name, (mfxU32)data);
        case MFX_VARIANT_TYPE_U64:
            return SetConfigFilterProperty(loader, cfg, name, (mfxU64)data);
        case MFX_VARIANT_TYPE_I8:
            return SetConfigFilterProperty(loader, cfg, name, (mfxI8)data);
        case MFX_VARIANT_TYPE_I16:
            return SetConfigFilterProperty(loader, cfg, name, (mfxI16)data);
        case MFX_VARIANT_TYPE_I32:
            return SetConfigFilterProperty(loader, cfg, name, (mfxI32)data);
        case MFX_VARIANT_TYPE_I64:
            return SetConfigFilterProperty(loader, cfg, name, (mfxI64)data);

        case MFX_VARIANT_TYPE_F32:
        case MFX_VARIANT_TYPE_F64:
        case MFX_VARIANT_TYPE_PTR:
        case MFX_VARIANT_TYPE_FP16:
        case MFX_VARIANT_TYPE_UNSET:
            return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_UNSUPPORTED;
}

// set ptr or float/double properties (should be cast to a pointer)
template <>
mfxStatus SetSingleProperty(mfxLoader loader,
                            mfxConfig cfg,
                            const char *name,
                            mfxVariantType varType,
                            mfxHDL data) {
    mfxF32 fData32 = 0;
    mfxF64 fData64 = 0;
    switch (varType) {
        case MFX_VARIANT_TYPE_F32:
            fData32 = *(reinterpret_cast<mfxF32 *>(data));
            return SetConfigFilterProperty(loader, cfg, name, fData32);
        case MFX_VARIANT_TYPE_F64:
            fData64 = *(reinterpret_cast<mfxF64 *>(data));
            return SetConfigFilterProperty(loader, cfg, name, fData64);
        case MFX_VARIANT_TYPE_PTR:
            return SetConfigFilterProperty(loader, cfg, name, (mfxHDL)data);

        case MFX_VARIANT_TYPE_U8:
        case MFX_VARIANT_TYPE_U16:
        case MFX_VARIANT_TYPE_U32:
        case MFX_VARIANT_TYPE_U64:
        case MFX_VARIANT_TYPE_I8:
        case MFX_VARIANT_TYPE_I16:
        case MFX_VARIANT_TYPE_I32:
        case MFX_VARIANT_TYPE_I64:
        case MFX_VARIANT_TYPE_FP16:
        case MFX_VARIANT_TYPE_UNSET:
            return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_UNSUPPORTED;
}

template mfxStatus SetSingleProperty<mfxU64>(mfxLoader, const char *, mfxVariantType, mfxU64);
template mfxStatus SetSingleProperty<mfxHDL>(mfxLoader, const char *, mfxVariantType, mfxHDL);

template mfxStatus SetSingleProperty<mfxU64>(mfxLoader,
                                             mfxConfig,
                                             const char *,
                                             mfxVariantType,
                                             mfxU64);
template mfxStatus SetSingleProperty<mfxHDL>(mfxLoader,
                                             mfxConfig,
                                             const char *,
                                             mfxVariantType,
                                             mfxHDL);
