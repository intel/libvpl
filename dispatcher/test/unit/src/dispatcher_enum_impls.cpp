/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

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
            cmdString = "call vpl-inspect.exe";
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
            cmdString = "./vpl-inspect";
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
    CaptureOutputLog();

    int err = ExecuteShellTestCmd(TEST_CMD_VPLINSPECT);
    EXPECT_EQ(err, 0);

    // check that the stub implementation was found
    std::string outputLog;
    GetOutputLog(outputLog);
    CheckOutputLog(outputLog, "ImplName: Stub Implementation");
}

TEST(Dispatcher_EnumImpls, VPLInspectFindsStub1x) {
    SKIP_IF_DISP_STUB_DISABLED();
    SKIP_IF_DISP_ENUM_IMPL_DISABLED();

    // capture output of vpl-inspect
    CaptureOutputLog();

    int err = ExecuteShellTestCmd(TEST_CMD_VPLINSPECT);
    EXPECT_EQ(err, 0);

    // check that the stub implementation was found
    std::string outputLog;
    GetOutputLog(outputLog);
    CheckOutputLog(outputLog, "ImplName: Stub Implementation 1X");
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
    CaptureOutputLog();

    int err = ExecuteShellTestCmd(TEST_CMD_VPLINSPECT);
    EXPECT_EQ(err, 0);

    // check that the stub implementation was found
    std::string outputLog;
    GetOutputLog(outputLog);

    // check that vpl-inspect returned all of the expected DeviceID's
    for (std::string &t : deviceIDList) {
        std::string expectedID = "  DeviceID: " + t;
        CheckOutputLog(outputLog, expectedID.c_str());
    }
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

#if defined(_WIN32) || defined(_WIN64)
    SetEnvironmentVariable("ONEVPL_PRIORITY_PATH", workDirPath.c_str());
#else
    setenv("ONEVPL_PRIORITY_PATH", workDirPath.c_str(), 1);
#endif

    int err = ExecuteShellTestCmd(TEST_CMD_COPY_STUB_TO_WORKDIR);
    EXPECT_EQ(err, 0);

    // capture output of vpl-inspect
    CaptureOutputLog();

    err = ExecuteShellTestCmd(TEST_CMD_VPLINSPECT);
    EXPECT_EQ(err, 0);

    std::string priorityLibPath = workDirPath + PATH_SEPARATOR;
    priorityLibPath += STUB_RT;

    // check that the stub implementation was found
    std::string outputLog;
    GetOutputLog(outputLog);
    CheckOutputLog(outputLog, priorityLibPath.c_str());
}
