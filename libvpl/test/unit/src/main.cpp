/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <string>

#include "gtest/gtest.h"

#include "src/dispatcher_common.h"

// clang-format off
//Intel® Video Processing Library (Intel® VPL) legacy cpp session test

#define SET_UTEST_PARAMETER(param, val)                               \
{                                                                 \
    (param) = (val);                                              \
    printf("Setting utest parameter: " #param " --> " #val "\n"); \
}

static void Usage(void) {
    printf("\nCustom parameters:\n");
    printf("   -disp:stub      ....  run dispatcher tests with stub runtime (default)\n");
    printf("   -disp:sw        ....  run dispatcher tests with CPU runtime\n");
    printf("   -disp:gpu-all   ....  run dispatcher tests with all GPU runtimes\n");
    printf("   -disp:gpu-msdk  ....  run dispatcher tests with GPU MSDK runtime (exclude tests only applicable to Intel® VPL RT)\n");
    printf("   -disp:gpu-vpl   ....  run dispatcher tests with GPU Intel® VPL runtime (exclude tests only applicable to MSDK RT)\n");
    printf("   -disp:enumimpl  ....  run dispatcher implementation enumeration tests (requires vpl-inspect installed)\n");
    printf("   -device:gpu ID  ....  add implementation of type GPU with ID corresponding to mfxDeviceDescription:DeviceID (ex: 9a49/0)\n");
    printf("   -workDir path   ....  location in which to create working directory (full path or relative to this application)\n");
    printf("                         directory 'path/utest-work' will be created if it does not exist\n");

    printf("\nLegacy C++ API (mfxvideo++.h) - requires GPU\n");
    printf("   -legacycpp accelModes [adapterList]\n");
    printf("      accelModes   ....  list of accelerator modes to test (separated by spaces): d3d9 d3d11 vaapi\n");
    printf("      adapterList  ....  optional list of adapters to test (separated by spaces): 0 1 2 3\n");


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

bool g_bLegacyTest = false;

bool g_bLegacyTestAccelD3D9  = false;
bool g_bLegacyTestAccelD3D11 = false;
bool g_bLegacyTestAccelVAAPI = false;

bool g_bLegacyTestImplHW  = false;
bool g_bLegacyTestImplHW2 = false;
bool g_bLegacyTestImplHW3 = false;
bool g_bLegacyTestImplHW4 = false;

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
            // only run GPU tests that are expected to pass with Intel® VPL (2.x) runtimes
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
        else if (nextArg == "-legacycpp") {
            // parse list of accelerator modes
            int nextIdx = idx + 1;
            while (nextIdx < argc) {
                std::string accelMode = argv[nextIdx];
                if (accelMode == "d3d9") {
                    SET_UTEST_PARAMETER(g_bLegacyTestAccelD3D9, true);
                    idx = nextIdx++;
                    continue;
                }
                else if (accelMode == "d3d11") {
                    SET_UTEST_PARAMETER(g_bLegacyTestAccelD3D11, true);
                    idx = nextIdx++;
                    continue;
                }
                else if (accelMode == "vaapi") {
                    SET_UTEST_PARAMETER(g_bLegacyTestAccelVAAPI, true);
                    idx = nextIdx++;
                    continue;
                }
                break;
            }

            if (!g_bLegacyTestAccelD3D9 && !g_bLegacyTestAccelD3D11 && !g_bLegacyTestAccelVAAPI) {
                printf("Error - must select at least one valid accelerator mode\n");
                return -1;
            }

            // legacy testing enabled for at least one accelMode
            SET_UTEST_PARAMETER(g_bLegacyTest, true);
            bDispCustom = true;

            // parse list of adapter numbers
            // [0, 1, 2, 3] maps to MFX_IMPL_HARDWARE, MFX_IMPL_HARDWARE2, MFX_IMPL_HARDWARE3, MFX_IMPL_HARDWARE4
            nextIdx = idx + 1;
            while (nextIdx < argc) {
                std::string adapterNum = argv[nextIdx];
                if (adapterNum == "0") {
                    SET_UTEST_PARAMETER(g_bLegacyTestImplHW, true);
                    idx = nextIdx++;
                    continue;
                }
                else if (adapterNum == "1") {
                    SET_UTEST_PARAMETER(g_bLegacyTestImplHW2, true);
                    idx = nextIdx++;
                    continue;
                }
                else if (adapterNum == "2") {
                    SET_UTEST_PARAMETER(g_bLegacyTestImplHW3, true);
                    idx = nextIdx++;
                    continue;
                }
                else if (adapterNum == "3") {
                    SET_UTEST_PARAMETER(g_bLegacyTestImplHW4, true);
                    idx = nextIdx++;
                    continue;
                }

                // new switch (starts with -)
                if (!adapterNum.compare(0, 1, "-"))
                    break;

                printf("Error - invalid adapter index\n");
                return -1;
            }

            // no adapter list provided - default to MFX_IMPL_HARDWARE only
            if (!g_bLegacyTestImplHW && !g_bLegacyTestImplHW2 && !g_bLegacyTestImplHW3 &&
                !g_bLegacyTestImplHW4)
                g_bLegacyTestImplHW = true;
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
