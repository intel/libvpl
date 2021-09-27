/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <string>

#include "gtest/gtest.h"

#include "src/unit_api.h"

// clang-format off

#define SET_UTEST_PARAMETER(param, val)                               \
{                                                                 \
    (param) = (val);                                              \
    printf("Setting utest parameter: " #param " --> " #val "\n"); \
}

static void Usage(void) {
    printf("\nCustom parameters for oneVPLTests:\n");
    printf("   -disp:sw        ....  run dispatcher tests with CPU runtime (default)\n");
    printf("   -disp:gpu-all   ....  run dispatcher tests with GPU oneVPL runtime\n");
    printf("   -disp:gpu-msdk  ....  run dispatcher tests with GPU MSDK runtime (exclude tests only applicable to oneVPL RT)\n");

    printf("\nNote: standard gtest flags (e.g. --gtest_filter) may be used along with custom parameters\n");
}

// clang-format on

// globals controlling test configuration
bool g_bDispInclSW       = false;
bool g_bDispInclGPU_VPL  = false;
bool g_bDispInclGPU_MSDK = false;

int main(int argc, char **argv) {
    // InitGoogleTest() removes switches that gtest recognizes
    ::testing::InitGoogleTest(&argc, argv);

    // parse custom switches, if any
    bool bDispCustom = false;
    for (int idx = 1; idx < argc; idx++) {
        std::string nextArg = argv[idx];

        if (nextArg == "-disp:sw") {
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
        else {
            Usage();
            return -1;
        }
    }

    if (!bDispCustom) {
        // default dispatcher behavior - only run common + SW tests
        g_bDispInclSW = true;
    }

    return RUN_ALL_TESTS();
}
