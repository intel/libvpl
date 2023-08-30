/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

// before including any header files, undefine ONEVPL_EXPERIMENTAL
// the tests in this file are used to confirm that elements which have
//   been promoted to the production API are accessible without it

// clang-format off
// avoid reformatting test code
#ifdef ONEVPL_EXPERIMENTAL
#undef ONEVPL_EXPERIMENTAL
#endif

#include <gtest/gtest.h>

#include "vpl/mfx.h"

#define MIN_VERSION_EXPECTED_MAJOR 2
#define MIN_VERSION_EXPECTED_MINOR 7

TEST(Experimental_API, CheckMinimumAPI) {
    if ((MFX_VERSION_MAJOR < MIN_VERSION_EXPECTED_MAJOR) ||
        (MFX_VERSION_MAJOR == MIN_VERSION_EXPECTED_MAJOR && MFX_VERSION_MINOR < MIN_VERSION_EXPECTED_MINOR)) {
        fprintf(stderr, "Error: Experimental_API tests require API version %d.%d or higher\n", MIN_VERSION_EXPECTED_MAJOR, MIN_VERSION_EXPECTED_MINOR);
        fprintf(stderr, "       Test suite was built with API version %d.%d\n", MFX_VERSION_MAJOR, MFX_VERSION_MINOR);
        fprintf(stderr, "       To exclude these tests add the following option to the command line: --gtest_filter=-Experimental_API*\n");
        FAIL();
    }
}

// confirm that the following enumerated types are available for API >= 2.7
//   and resolve to the expected values without defining ONEVPL_EXPERIMENTAL
// if unavailable in headers, this should fail at compile time
TEST(Experimental_API, PromotedEnumsAvailable_2_7) {

    // mfxstructures.h
    mfxHandleType t_mfxHandleType = MFX_HANDLE_PXP_CONTEXT;
    EXPECT_EQ(t_mfxHandleType, 10);

    // mfxstructures.h
    mfxExtDecodeErrorReport t_mfxExtDecodeErrorReport = {};
    t_mfxExtDecodeErrorReport.ErrorTypes              = MFX_ERROR_JPEG_APP0_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 5));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP1_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 6));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP2_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 7));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP3_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 8));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP4_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 9));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP5_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 10));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP6_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 11));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP7_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 12));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP8_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 13));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP9_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 14));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP10_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 15));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP11_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 16));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP12_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 17));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP13_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 18));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_APP14_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 19));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_DQT_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 20));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_SOF0_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 21));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_DHT_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 22));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_DRI_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 23));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_SOS_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 24));

    t_mfxExtDecodeErrorReport.ErrorTypes = MFX_ERROR_JPEG_UNKNOWN_MARKER;
    EXPECT_EQ(t_mfxExtDecodeErrorReport.ErrorTypes, (1 << 25));
}

// clang-format on
