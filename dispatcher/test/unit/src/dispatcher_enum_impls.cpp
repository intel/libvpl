/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include <regex>

#include "src/dispatcher_common.h"

typedef enum {
    TEST_CMD_VPLINSPECT,
    TEST_CMD_VPLTIMING,
    TEST_CMD_COPY_STUB_TO_WORKDIR,
} ShellTestCmd;

#if defined(_WIN32) || defined(_WIN64)
    #if defined _DEBUG
        #if defined _M_IX86
            #define STUB_RT "libvplstubrt32d.dll"
        #else
            #define STUB_RT "libvplstubrt64d.dll"
        #endif
    #else
        #if defined _M_IX86
            #define STUB_RT "libvplstubrt32.dll"
        #else
            #define STUB_RT "libvplstubrt64.dll"
        #endif
    #endif
#elif defined(__linux__)
    #define STUB_RT "libvplstubrt1x64.so"
#endif

static int ExecuteShellTestCmd(ShellTestCmd cmd) {
    int err = 0;

    std::string cmdString;
    std::string workDirPath;
    GetWorkingDirectoryPath(workDirPath);

#if defined(_WIN32) || defined(_WIN64)
    switch (cmd) {
        case TEST_CMD_VPLINSPECT:
            cmdString = "call vpl-inspect.exe > ";
            cmdString += CAPTURE_LOG_DEF_FILENAME;
            break;
        case TEST_CMD_VPLTIMING:
            return -1;
        case TEST_CMD_COPY_STUB_TO_WORKDIR:
            if (workDirPath.empty())
                return -1;

            cmdString = "copy /Y ";
            cmdString += STUB_RT;
            cmdString += " ";
            cmdString += workDirPath;
            break;
        default:
            return -1;
    }
#else
    switch (cmd) {
        case TEST_CMD_VPLINSPECT:
            cmdString = "./vpl-inspect > ";
            cmdString += CAPTURE_LOG_DEF_FILENAME;
            break;
        case TEST_CMD_VPLTIMING:
            return -1;
        case TEST_CMD_COPY_STUB_TO_WORKDIR:
            if (workDirPath.empty())
                return -1;

            cmdString = "cp -f ";
            cmdString += STUB_RT;
            cmdString += " ";
            cmdString += workDirPath;
            break;
        default:
            return -1;
    }
#endif

    // returns value from the cmd interpreter, or -1 if error
    err = system(cmdString.c_str());

    return err;
}

TEST(Dispatcher_EnumImpls, VPLInspectFindsStub) {
    SKIP_IF_DISP_STUB_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    // capture output of vpl-inspect
    CaptureOutputLog(CAPTURE_LOG_FILE);

    int err = ExecuteShellTestCmd(TEST_CMD_VPLINSPECT);
    EXPECT_EQ(err, 0);

    // check that the stub implementation was found
    CheckOutputLog("ImplName: Stub Implementation");
    CleanupOutputLog();
}

TEST(Dispatcher_EnumImpls, VPLInspectFindsStub1x) {
    SKIP_IF_DISP_STUB_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    // capture output of vpl-inspect
    CaptureOutputLog(CAPTURE_LOG_FILE);

    int err = ExecuteShellTestCmd(TEST_CMD_VPLINSPECT);
    EXPECT_EQ(err, 0);

    // check that the stub implementation was found
    CheckOutputLog("ImplName: Stub Implementation 1X");
    CleanupOutputLog();
}

TEST(Dispatcher_EnumImpls, VPLInspectFindsInputDeviceIDs) {
    SKIP_IF_DISP_GPU_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    // get list of input deviceID's from cmd-line
    // skip test if not specified (size of list == 0)
    std::list<std::string> deviceIDList;
    GetDeviceIDList(deviceIDList);
    if (deviceIDList.size() == 0) {
        GTEST_SKIP();
    }

    // capture output of vpl-inspect
    CaptureOutputLog(CAPTURE_LOG_FILE);

    int err = ExecuteShellTestCmd(TEST_CMD_VPLINSPECT);
    EXPECT_EQ(err, 0);

    // check that vpl-inspect returned all of the expected DeviceID's
    for (std::string &t : deviceIDList) {
        std::string expectedID = "  DeviceID: " + t;
        CheckOutputLog(expectedID.c_str());
    }
    CleanupOutputLog();
}

TEST(Dispatcher_EnumImpls, VPLInspectFindsRTFromPriorityPath) {
    SKIP_IF_DISP_STUB_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    // we will copy a stub RT into the working directory and set ONEVPL_PRIORITY_PATH to point there
    // if '-workDir path' was not provided on cmd-line, return error
    if (!g_bWorkDirAvailable) {
        GTEST_SKIP();
    }

    std::string workDirPath;
    GetWorkingDirectoryPath(workDirPath);

    int err = ExecuteShellTestCmd(TEST_CMD_COPY_STUB_TO_WORKDIR);
    EXPECT_EQ(err, 0);

    // capture output of vpl-inspect
    CaptureOutputLog(CAPTURE_LOG_FILE);

#if defined(_WIN32) || defined(_WIN64)
    SetEnvironmentVariable("ONEVPL_PRIORITY_PATH", workDirPath.c_str());
#else
    setenv("ONEVPL_PRIORITY_PATH", workDirPath.c_str(), 1);
#endif

    err = ExecuteShellTestCmd(TEST_CMD_VPLINSPECT);

#if defined(_WIN32) || defined(_WIN64)
    SetEnvironmentVariable("ONEVPL_PRIORITY_PATH", NULL);
#else
    unsetenv("ONEVPL_PRIORITY_PATH");
#endif

    EXPECT_EQ(err, 0);

    std::string priorityLibPath = workDirPath + PATH_SEPARATOR;
    priorityLibPath += STUB_RT;

    // check that the stub implementation was found
    CheckOutputLog(priorityLibPath.c_str());
    CleanupOutputLog();
}

// priorityPath = what we set ONEVPL_PRIORITY_PATH to, i.e. what dispatcher will use in search
// expectedPath = what we expect vpl-inspect to report with "Library Path: ...", i.e. the canonicalized full path which is used to open the lib
// bExpectPass = set to true if test expected to pass (default), false if priorityPath is something invalid
static mfxStatus CheckExpectedPath(const std::string &priorityPath,
                                   const std::string &expectedPath,
                                   bool bExpectPass = true) {
    // capture output of vpl-inspect
    CaptureOutputLog(CAPTURE_LOG_FILE);

#if defined(_WIN32) || defined(_WIN64)
    SetEnvironmentVariable("ONEVPL_PRIORITY_PATH", priorityPath.c_str());
#else
    setenv("ONEVPL_PRIORITY_PATH", priorityPath.c_str(), 1);
#endif

    int err = ExecuteShellTestCmd(TEST_CMD_VPLINSPECT);

#if defined(_WIN32) || defined(_WIN64)
    SetEnvironmentVariable("ONEVPL_PRIORITY_PATH", NULL);
#else
    unsetenv("ONEVPL_PRIORITY_PATH");
#endif

    EXPECT_EQ(err, 0);

    // check that the stub implementation was found and reports the expected path
    CheckOutputLog(expectedPath.c_str(), bExpectPass);
    CleanupOutputLog();

    return MFX_ERR_NONE;
}

TEST(Dispatcher_EnumImpls, VPLInspectReportsExpectedPath_01) {
    SKIP_IF_DISP_STUB_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    if (!g_bWorkDirAvailable) {
        GTEST_SKIP();
    }

    std::string workDirPath;
    GetWorkingDirectoryPath(workDirPath);

    int err = ExecuteShellTestCmd(TEST_CMD_COPY_STUB_TO_WORKDIR);
    EXPECT_EQ(err, 0);

    // search path has no trailing slashes, should be correctly added in dispatcher
    std::string priorityPath = workDirPath;

    std::string expectedPath = "Library path: ";
    expectedPath += workDirPath;
    expectedPath += PATH_SEPARATOR;
    expectedPath += STUB_RT;

    CheckExpectedPath(priorityPath, expectedPath);
}

TEST(Dispatcher_EnumImpls, VPLInspectReportsExpectedPath_02) {
    SKIP_IF_DISP_STUB_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    if (!g_bWorkDirAvailable) {
        GTEST_SKIP();
    }

    std::string workDirPath;
    GetWorkingDirectoryPath(workDirPath);

    int err = ExecuteShellTestCmd(TEST_CMD_COPY_STUB_TO_WORKDIR);
    EXPECT_EQ(err, 0);

    // modify search path with trailing fwd-slash
    std::string priorityPath = workDirPath;
    priorityPath += "/";

    std::string expectedPath = "Library path: ";
    expectedPath += workDirPath;
    expectedPath += PATH_SEPARATOR;
    expectedPath += STUB_RT;

    CheckExpectedPath(priorityPath, expectedPath);
}

TEST(Dispatcher_EnumImpls, VPLInspectReportsExpectedPath_03) {
    SKIP_IF_DISP_STUB_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    if (!g_bWorkDirAvailable) {
        GTEST_SKIP();
    }

    std::string workDirPath;
    GetWorkingDirectoryPath(workDirPath);

    int err = ExecuteShellTestCmd(TEST_CMD_COPY_STUB_TO_WORKDIR);
    EXPECT_EQ(err, 0);

    // modify search path with trailing back-slash
    std::string priorityPath = workDirPath;
    priorityPath += "\\";

    std::string expectedPath = "Library path: ";
    expectedPath += workDirPath;
    expectedPath += PATH_SEPARATOR;
    expectedPath += STUB_RT;

    // Windows can handle both fwd and back-slash, Linux requires fwd-slash only
#if defined(_WIN32) || defined(_WIN64)
    CheckExpectedPath(priorityPath, expectedPath);
#else
    CheckExpectedPath(priorityPath, expectedPath, false);
#endif
}

TEST(Dispatcher_EnumImpls, VPLInspectReportsExpectedPath_04) {
    SKIP_IF_DISP_STUB_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    if (!g_bWorkDirAvailable) {
        GTEST_SKIP();
    }

    std::string workDirPath;
    GetWorkingDirectoryPath(workDirPath);

    int err = ExecuteShellTestCmd(TEST_CMD_COPY_STUB_TO_WORKDIR);
    EXPECT_EQ(err, 0);

    // modify search path with two trailing fwd-slashes
    std::string priorityPath = workDirPath;
    priorityPath += "//";

    std::string expectedPath = "Library path: ";
    expectedPath += workDirPath;
    expectedPath += PATH_SEPARATOR;
    expectedPath += STUB_RT;

    CheckExpectedPath(priorityPath, expectedPath);
}

TEST(Dispatcher_EnumImpls, VPLInspectReportsExpectedPath_05) {
    SKIP_IF_DISP_STUB_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    if (!g_bWorkDirAvailable) {
        GTEST_SKIP();
    }

    std::string workDirPath;
    GetWorkingDirectoryPath(workDirPath);

    int err = ExecuteShellTestCmd(TEST_CMD_COPY_STUB_TO_WORKDIR);
    EXPECT_EQ(err, 0);

    // modify search path with two trailing back-slashes
    std::string priorityPath = workDirPath;
    priorityPath += "\\\\";

    std::string expectedPath = "Library path: ";
    expectedPath += workDirPath;
    expectedPath += PATH_SEPARATOR;
    expectedPath += STUB_RT;

    // Windows can handle both fwd and back-slash, Linux requires fwd-slash only
#if defined(_WIN32) || defined(_WIN64)
    CheckExpectedPath(priorityPath, expectedPath);
#else
    CheckExpectedPath(priorityPath, expectedPath, false);
#endif
}

TEST(Dispatcher_EnumImpls, VPLInspectReportsExpectedPath_06) {
    SKIP_IF_DISP_STUB_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    if (!g_bWorkDirAvailable) {
        GTEST_SKIP();
    }

    std::string workDirPath;
    GetWorkingDirectoryPath(workDirPath);

    int err = ExecuteShellTestCmd(TEST_CMD_COPY_STUB_TO_WORKDIR);
    EXPECT_EQ(err, 0);

    // modify search path with fwd and back-slashes
    std::string priorityPath = workDirPath;
    priorityPath += "/\\";

    std::string expectedPath = "Library path: ";
    expectedPath += workDirPath;
    expectedPath += PATH_SEPARATOR;
    expectedPath += STUB_RT;

    // Windows can handle both fwd and back-slash, Linux requires fwd-slash only
#if defined(_WIN32) || defined(_WIN64)
    CheckExpectedPath(priorityPath, expectedPath);
#else
    CheckExpectedPath(priorityPath, expectedPath, false);
#endif
}

TEST(Dispatcher_EnumImpls, VPLInspectReportsExpectedPath_07) {
    SKIP_IF_DISP_STUB_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    if (!g_bWorkDirAvailable) {
        GTEST_SKIP();
    }

    std::string workDirPath;
    GetWorkingDirectoryPath(workDirPath);

    int err = ExecuteShellTestCmd(TEST_CMD_COPY_STUB_TO_WORKDIR);
    EXPECT_EQ(err, 0);

    // modify search path with back and fwd-slashes
    std::string priorityPath = workDirPath;
    priorityPath += "\\/";

    std::string expectedPath = "Library path: ";
    expectedPath += workDirPath;
    expectedPath += PATH_SEPARATOR;
    expectedPath += STUB_RT;

    // Windows can handle both fwd and back-slash, Linux requires fwd-slash only
#if defined(_WIN32) || defined(_WIN64)
    CheckExpectedPath(priorityPath, expectedPath);
#else
    CheckExpectedPath(priorityPath, expectedPath, false);
#endif
}

TEST(Dispatcher_EnumImpls, VPLInspectReportsExpectedPath_08) {
    SKIP_IF_DISP_STUB_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    if (!g_bWorkDirAvailable) {
        GTEST_SKIP();
    }

    std::string workDirPath;
    GetWorkingDirectoryPath(workDirPath);

    int err = ExecuteShellTestCmd(TEST_CMD_COPY_STUB_TO_WORKDIR);
    EXPECT_EQ(err, 0);

    // modify search path with invalid separator - expect to fail
    std::string priorityPath = workDirPath;
    priorityPath += "+";

    std::string expectedPath = "Library path: ";
    expectedPath += workDirPath;
    expectedPath += PATH_SEPARATOR;
    expectedPath += STUB_RT;

    CheckExpectedPath(priorityPath, expectedPath, false);
}

TEST(Dispatcher_EnumImpls, VPLInspectReportsExpectedPath_09) {
    SKIP_IF_DISP_STUB_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    if (!g_bWorkDirAvailable) {
        GTEST_SKIP();
    }

    std::string workDirPath;
    GetWorkingDirectoryPath(workDirPath);

    int err = ExecuteShellTestCmd(TEST_CMD_COPY_STUB_TO_WORKDIR);
    EXPECT_EQ(err, 0);

    // convert all back-slash (default on Windows) to fwd-slash
    std::string priorityPath = std::regex_replace(workDirPath, std::regex("\\\\"), "/");

    std::string expectedPath = "Library path: ";
    expectedPath += workDirPath;
    expectedPath += PATH_SEPARATOR;
    expectedPath += STUB_RT;

    CheckExpectedPath(priorityPath, expectedPath);
}
