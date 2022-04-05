/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef DISPATCHER_TEST_UNIT_SRC_UNIT_API_H_
#define DISPATCHER_TEST_UNIT_SRC_UNIT_API_H_

#include "vpl/mfxdispatcher.h"
#include "vpl/mfximplcaps.h"
#include "vpl/mfxvideo.h"

// globals controlling test configuration
extern bool g_bDispInclStub;
extern bool g_bDispInclSW;
extern bool g_bDispInclGPU_VPL;
extern bool g_bDispInclGPU_MSDK;
extern bool g_bDispEnumImpl;
extern bool g_bDeviceAdded;
extern bool g_bWorkDirAvailable;

#define SKIP_IF_DISP_STUB_DISABLED()    \
    {                                   \
        if (g_bDispInclStub == false) { \
            GTEST_SKIP();               \
        }                               \
    }

#define SKIP_IF_DISP_SW_DISABLED()    \
    {                                 \
        if (g_bDispInclSW == false) { \
            GTEST_SKIP();             \
        }                             \
    }

#define SKIP_IF_DISP_GPU_DISABLED()                                        \
    {                                                                      \
        if (g_bDispInclGPU_VPL == false && g_bDispInclGPU_MSDK == false) { \
            GTEST_SKIP();                                                  \
        }                                                                  \
    }

#define SKIP_IF_DISP_GPU_VPL_DISABLED()    \
    {                                      \
        if (g_bDispInclGPU_VPL == false) { \
            GTEST_SKIP();                  \
        }                                  \
    }

#define SKIP_IF_DISP_GPU_MSDK_DISABLED()    \
    {                                       \
        if (g_bDispInclGPU_MSDK == false) { \
            GTEST_SKIP();                   \
        }                                   \
    }

#define SKIP_IF_DISP_ENUM_IMPL_DISABLED() \
    {                                     \
        if (g_bDispEnumImpl == false) {   \
            GTEST_SKIP();                 \
        }                                 \
    }

#endif // DISPATCHER_TEST_UNIT_SRC_UNIT_API_H_
