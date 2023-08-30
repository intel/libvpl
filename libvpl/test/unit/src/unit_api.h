/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef LIBVPL_TEST_UNIT_SRC_UNIT_API_H_
#define LIBVPL_TEST_UNIT_SRC_UNIT_API_H_

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

extern bool g_bLegacyTest;

extern bool g_bLegacyTestAccelD3D9;
extern bool g_bLegacyTestAccelD3D11;
extern bool g_bLegacyTestAccelVAAPI;

extern bool g_bLegacyTestImplHW;
extern bool g_bLegacyTestImplHW2;
extern bool g_bLegacyTestImplHW3;
extern bool g_bLegacyTestImplHW4;

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

#define SKIP_IF_LEGACY_TEST_DISABLED() \
    {                                  \
        if (g_bLegacyTest == false) {  \
            GTEST_SKIP();              \
        }                              \
    }

#define SKIP_IF_LEGACY_D3D9_DISABLED()         \
    {                                          \
        if (g_bLegacyTestAccelD3D9 == false) { \
            GTEST_SKIP();                      \
        }                                      \
    }

#define SKIP_IF_LEGACY_D3D11_DISABLED()         \
    {                                           \
        if (g_bLegacyTestAccelD3D11 == false) { \
            GTEST_SKIP();                       \
        }                                       \
    }

#define SKIP_IF_LEGACY_VAAPI_DISABLED()         \
    {                                           \
        if (g_bLegacyTestAccelVAAPI == false) { \
            GTEST_SKIP();                       \
        }                                       \
    }

#define SKIP_IF_LEGACY_IMPL_HW_DISABLED()   \
    {                                       \
        if (g_bLegacyTestImplHW == false) { \
            GTEST_SKIP();                   \
        }                                   \
    }

#define SKIP_IF_LEGACY_IMPL_HW2_DISABLED()   \
    {                                        \
        if (g_bLegacyTestImplHW2 == false) { \
            GTEST_SKIP();                    \
        }                                    \
    }

#define SKIP_IF_LEGACY_IMPL_HW3_DISABLED()   \
    {                                        \
        if (g_bLegacyTestImplHW3 == false) { \
            GTEST_SKIP();                    \
        }                                    \
    }

#define SKIP_IF_LEGACY_IMPL_HW4_DISABLED()   \
    {                                        \
        if (g_bLegacyTestImplHW4 == false) { \
            GTEST_SKIP();                    \
        }                                    \
    }

#endif // LIBVPL_TEST_UNIT_SRC_UNIT_API_H_
