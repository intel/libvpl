/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <string>

#include "gtest/gtest.h"

#include "src/dispatcher_common.h"

// clang-format off

#define SET_UTEST_PARAMETER(param, val)                               \
{                                                                 \
    (param) = (val);                                              \
    printf("Setting utest parameter: " #param " --> " #val "\n"); \
}

static void Usage(void) {
    printf("\nCustom parameters for oneVPLTests:\n");
    printf("   -disp:stub      ....  run dispatcher tests with stub runtime (default)\n");
    printf("   -disp:sw        ....  run dispatcher tests with CPU runtime\n");
    printf("   -disp:gpu-all   ....  run dispatcher tests with all GPU runtimes\n");
    printf("   -disp:gpu-msdk  ....  run dispatcher tests with GPU MSDK runtime (exclude tests only applicable to oneVPL RT)\n");
    printf("   -disp:gpu-vpl   ....  run dispatcher tests with GPU oneVPL runtime (exclude tests only applicable to MSDK RT)\n");
    printf("   -disp:enumimpl  ....  run dispatcher implementation enumeration tests (requires vpl-inspect installed)\n");
    printf("   -device:gpu ID  ....  add implementation of type GPU with ID corresponding to mfxDeviceDescription:DeviceID (ex: 9a49/0)\n");
    printf("   -workDir path   ....  location in which to create working directory (full path or relative to 'oneVPLTests' application)\n");
    printf("                         directory 'path/oneVPLTests-work' will be created if it does not exist\n");

    printf("\nNote: standard gtest flags (e.g. --gtest_filter) may be used along with custom parameters\n");
}

// clang-format on

// globals controlling test configuration
bool g_bDispInclStub     = false;
bool g_bDispInclSW       = false;
bool g_bDispInclGPU_VPL  = false;
bool g_bDispInclGPU_MSDK = false;
bool g_bDispEnumImpl     = false;
bool g_bDeviceAdded      = false;
bool g_bWorkDirAvailable = false;

int main(int argc, char **argv) {
    // InitGoogleTest() removes switches that gtest recognizes
    ::testing::InitGoogleTest(&argc, argv);

    // parse custom switches, if any
    bool bDispCustom = false;
    for (int idx = 1; idx < argc; idx++) {
        std::string nextArg = argv[idx];

        if (nextArg == "-disp:stub") {
            bDispCustom = true;
            SET_UTEST_PARAMETER(g_bDispInclStub, true);
        }
        else if (nextArg == "-disp:sw") {
            bDispCustom = true;
            SET_UTEST_PARAMETER(g_bDispInclSW, true);
        }
        else if (nextArg == "-disp:gpu-all") {
            // run all GPU tests
            bDispCustom = true;
            SET_UTEST_PARAMETER(g_bDispInclGPU_VPL, true);
            SET_UTEST_PARAMETER(g_bDispInclGPU_MSDK, true);
        }
        else if (nextArg == "-disp:gpu-msdk") {
            // only run GPU tests that are expected to pass in MSDK compatibility mode
            //   (i.e. no filtering on 2.x properties)
            bDispCustom = true;
            SET_UTEST_PARAMETER(g_bDispInclGPU_VPL, false);
            SET_UTEST_PARAMETER(g_bDispInclGPU_MSDK, true);
        }
        else if (nextArg == "-disp:gpu-vpl") {
            // only run GPU tests that are expected to pass with oneVPL (2.x) runtimes
            bDispCustom = true;
            SET_UTEST_PARAMETER(g_bDispInclGPU_VPL, true);
            SET_UTEST_PARAMETER(g_bDispInclGPU_MSDK, false);
        }
        else if (nextArg == "-disp:enumimpl") {
            bDispCustom = true;
            SET_UTEST_PARAMETER(g_bDispEnumImpl, true);
        }
        else if (nextArg == "-device:gpu") {
            // add deviceID to list
            idx++;
            if (idx >= argc) {
                Usage();
                return -1;
            }

            int numDevices = AddDeviceID(argv[idx]);
            if (numDevices == 0) {
                printf("Error - unable to add device %s to list\n", argv[idx]);
                return -1;
            }

            SET_UTEST_PARAMETER(g_bDeviceAdded, true);
        }
        else if (nextArg == "-workDir") {
            // create a writable working directory for tests which require it
            idx++;
            if (idx >= argc) {
                Usage();
                return -1;
            }

            // create working directory, returns 0 on success
            int res = CreateWorkingDirectory(argv[idx]);
            if (res) {
                printf("Error - unable to create working directory in %s\n", argv[idx]);
                return -1;
            }

            SET_UTEST_PARAMETER(g_bWorkDirAvailable, true);
        }
        else {
            Usage();
            return -1;
        }
    }

    if (g_bWorkDirAvailable) {
        std::string t;
        GetWorkingDirectoryPath(t);
        printf("Using working directory: %s\n", t.c_str());
    }

    if (g_bDeviceAdded) {
        std::list<std::string> deviceIDList;
        GetDeviceIDList(deviceIDList);
        printf("List of deviceID's:\n");
        for (std::string &t : deviceIDList) {
            printf("  %s\n", t.c_str());
        }
    }

    if (!bDispCustom) {
        // default dispatcher behavior - only run stub implementation tests
        g_bDispInclStub = true;
    }

    return RUN_ALL_TESTS();
}
