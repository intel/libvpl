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
#define MIN_VERSION_EXPECTED_MINOR 13

TEST(Experimental_API, CheckMinimumAPI) {
    if ((MFX_VERSION_MAJOR < MIN_VERSION_EXPECTED_MAJOR) ||
        (MFX_VERSION_MAJOR == MIN_VERSION_EXPECTED_MAJOR && MFX_VERSION_MINOR < MIN_VERSION_EXPECTED_MINOR)) {
        fprintf(stderr, "Error: Experimental_API tests require API version %d.%d or higher\n", MIN_VERSION_EXPECTED_MAJOR, MIN_VERSION_EXPECTED_MINOR);
        fprintf(stderr, "       Test suite was built with API version %d.%d\n", MFX_VERSION_MAJOR, MFX_VERSION_MINOR);
        fprintf(stderr, "       To exclude these tests add the following option to the command line: --gtest_filter=-Experimental_API*\n");
        FAIL();
    }
}
// clang-format on

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

// if unavailable in headers, this should fail at compile time
TEST(Experimental_API, PromotedEnumsAvailable_2_8) {
    // mfxcommon.h
    mfxExtRefListCtrl t_mfxExtRefListCtrl = {};
    t_mfxExtRefListCtrl.Header.BufferId   = MFX_EXTBUFF_UNIVERSAL_REFLIST_CTRL;
    EXPECT_EQ(t_mfxExtRefListCtrl.Header.BufferId, MFX_EXTBUFF_UNIVERSAL_REFLIST_CTRL);
}

// NOTE: no APIs were moved to production in API 2.9

#include "vpl/mfxcamera.h"

// if unavailable in headers, this should fail at compile time
TEST(Experimental_API, PromotedEnumsAvailable_2_10) {
    // mfxcommon.h
    mfxExtendedDeviceId t_mfxExtendedDeviceId = {};
    t_mfxExtendedDeviceId.DeviceID            = 40;
    EXPECT_EQ(t_mfxExtendedDeviceId.DeviceID, 40);

    t_mfxExtendedDeviceId.RevisionID = 60;
    EXPECT_EQ(t_mfxExtendedDeviceId.RevisionID, 60);

    mfxU32 t_u32;
    mfxRefInterface t_mfxRefInterface = {};
    t_mfxRefInterface.Context         = (mfxHDL)(&t_u32);
    EXPECT_EQ(t_mfxRefInterface.Context, (mfxHDL)(&t_u32));

    extDeviceUUID t_extDeviceUUID = {};
    t_extDeviceUUID.device_id     = 70;
    EXPECT_EQ(t_extDeviceUUID.device_id, 70);

    t_u32 = MFX_GPUCOPY_SAFE;
    EXPECT_EQ(t_u32, MFX_GPUCOPY_SAFE);

    mfxImplCapsDeliveryFormat t_capsFormat = MFX_IMPLCAPS_DEVICE_ID_EXTENDED;
    EXPECT_EQ(t_capsFormat, MFX_IMPLCAPS_DEVICE_ID_EXTENDED);

    // mfxdispatcher.h
    MFX_UUID_COMPUTE_DEVICE_ID(&t_mfxExtendedDeviceId, 5, &t_extDeviceUUID); // macro
    EXPECT_EQ(t_extDeviceUUID.sub_device_id, 5);

    // mfxstructures.h
    t_u32 = MFX_FOURCC_ABGR16F;
    EXPECT_EQ(t_u32, MFX_FOURCC_ABGR16F);

    t_u32 = MFX_CONTENT_NOISY_VIDEO;
    EXPECT_EQ(t_u32, MFX_CONTENT_NOISY_VIDEO);

    t_u32 = MFX_FOURCC_XYUV;
    EXPECT_EQ(t_u32, MFX_FOURCC_XYUV);

    mfxExtMBQP t_mfxExtMBQP = {};
    t_mfxExtMBQP.Pitch      = 20;
    EXPECT_EQ(t_mfxExtMBQP.Pitch, 20);

    mfxABGR16FP t_mfxABGR16FP = {};
    t_mfxABGR16FP.R           = 5;
    EXPECT_EQ(t_mfxABGR16FP.R, 5);

    mfxFrameData t_mfxFrameData = {};
    t_mfxFrameData.ABGRFP16     = &t_mfxABGR16FP;
    EXPECT_EQ(t_mfxFrameData.ABGRFP16, &t_mfxABGR16FP);

    // mfxdefs.h
    mfxFP16 t_fp16 = 3;
    EXPECT_EQ(t_fp16, 3);

    mfxVariant t_variant = {};
    t_variant.Type       = MFX_VARIANT_TYPE_FP16;
    EXPECT_EQ(t_variant.Type, MFX_VARIANT_TYPE_FP16);

    t_variant.Type = (mfxVariantType)MFX_DATA_TYPE_FP16;
    EXPECT_EQ(t_variant.Type, MFX_DATA_TYPE_FP16);

    t_variant.Data.FP16 = 4;
    EXPECT_EQ(t_variant.Data.FP16, 4);

    // mfxcamera.h
    mfxExtCamWhiteBalance t_mfxExtCamWhiteBalance = {};
    t_mfxExtCamWhiteBalance.R                     = 6;
    EXPECT_EQ(t_mfxExtCamWhiteBalance.R, 6);
}

// if unavailable in headers, this should fail at compile time
TEST(Experimental_API, PromotedEnumsAvailable_2_11) {
    // mfxdefs.h
    mfxStatus t_err = MFX_ERR_UNKNOWN;
    t_err           = MFX_ERR_MORE_EXTBUFFER;
    EXPECT_EQ(t_err, MFX_ERR_MORE_EXTBUFFER);

    // mfxvideo.h
    mfxConfigInterface t_mfxConfigInterface = {};
    t_mfxConfigInterface.Version.Version    = 0x1499;
    EXPECT_EQ(t_mfxConfigInterface.Version.Version, 0x1499);

    mfxStructureType t_mfxStructureType = (mfxStructureType)0x18181818;

    t_mfxStructureType = MFX_STRUCTURE_TYPE_UNKNOWN;
    EXPECT_EQ(t_mfxStructureType, MFX_STRUCTURE_TYPE_UNKNOWN);

    t_mfxStructureType = MFX_STRUCTURE_TYPE_VIDEO_PARAM;
    EXPECT_EQ(t_mfxStructureType, MFX_STRUCTURE_TYPE_VIDEO_PARAM);

    // alias to MFXVideoCORE_GetHandle(), so dispatcher will return error with null session
    t_err = MFXGetConfigInterface(nullptr, &t_mfxConfigInterface);
    EXPECT_EQ(t_err, MFX_ERR_INVALID_HANDLE);

    // mfxstructures.h
    mfxU32 t_u32 = 0x27272727;
    t_u32        = MFX_HANDLE_CONFIG_INTERFACE;
    EXPECT_EQ(t_u32, MFX_HANDLE_CONFIG_INTERFACE);
}

// if unavailable in headers, this should fail at compile time
TEST(Experimental_API, PromotedEnumsAvailable_2_13) {
    // mfxstructures.h - new enums
    mfxQualityInfoMode t_mfxQualityInfoMode = (mfxQualityInfoMode)0x2626;

    t_mfxQualityInfoMode = MFX_QUALITY_INFO_DISABLE;
    EXPECT_EQ(t_mfxQualityInfoMode, MFX_QUALITY_INFO_DISABLE);

    t_mfxQualityInfoMode = MFX_QUALITY_INFO_LEVEL_FRAME;
    EXPECT_EQ(t_mfxQualityInfoMode, MFX_QUALITY_INFO_LEVEL_FRAME);

    mfxU32 t_alphaMode = 0x3535;

    t_alphaMode = MFX_ALPHA_MODE_PREMULTIPLIED;
    EXPECT_EQ(t_alphaMode, MFX_ALPHA_MODE_PREMULTIPLIED);

    t_alphaMode = MFX_ALPHA_MODE_STRAIGHT;
    EXPECT_EQ(t_alphaMode, MFX_ALPHA_MODE_STRAIGHT);

    mfxU32 t_extBufID = 0;

    t_extBufID = MFX_EXTBUFF_ENCODED_QUALITY_INFO_MODE;
    EXPECT_EQ(t_extBufID, MFX_EXTBUFF_ENCODED_QUALITY_INFO_MODE);

    t_extBufID = MFX_EXTBUFF_ENCODED_QUALITY_INFO_OUTPUT;
    EXPECT_EQ(t_extBufID, MFX_EXTBUFF_ENCODED_QUALITY_INFO_OUTPUT);

    t_extBufID = MFX_EXTBUFF_AV1_SCREEN_CONTENT_TOOLS;
    EXPECT_EQ(t_extBufID, MFX_EXTBUFF_AV1_SCREEN_CONTENT_TOOLS);

    t_extBufID = MFX_EXTBUFF_ALPHA_CHANNEL_ENC_CTRL;
    EXPECT_EQ(t_extBufID, MFX_EXTBUFF_ALPHA_CHANNEL_ENC_CTRL);

    t_extBufID = MFX_EXTBUFF_ALPHA_CHANNEL_SURFACE;
    EXPECT_EQ(t_extBufID, MFX_EXTBUFF_ALPHA_CHANNEL_SURFACE);

    // mfxstructures.h - new extBufs
    mfxExtQualityInfoMode t_mfxExtQualityInfoMode = {};
    t_mfxExtQualityInfoMode.QualityInfoMode       = MFX_QUALITY_INFO_LEVEL_FRAME;
    EXPECT_EQ(t_mfxExtQualityInfoMode.QualityInfoMode, MFX_QUALITY_INFO_LEVEL_FRAME);

    mfxExtQualityInfoOutput t_mfxExtQualityInfoOutput = {};
    t_mfxExtQualityInfoOutput.FrameOrder              = 12;
    EXPECT_EQ(t_mfxExtQualityInfoOutput.FrameOrder, 12);

    mfxExtAV1ScreenContentTools t_mfxExtAV1ScreenContentTools = {};
    t_mfxExtAV1ScreenContentTools.Palette                     = MFX_CODINGOPTION_ON;
    EXPECT_EQ(t_mfxExtAV1ScreenContentTools.Palette, MFX_CODINGOPTION_ON);

    mfxExtAlphaChannelEncCtrl t_mfxExtAlphaChannelEncCtrl = {};
    t_mfxExtAlphaChannelEncCtrl.AlphaChannelMode          = MFX_ALPHA_MODE_STRAIGHT;
    EXPECT_EQ(t_mfxExtAlphaChannelEncCtrl.AlphaChannelMode, MFX_ALPHA_MODE_STRAIGHT);

    mfxExtAlphaChannelSurface t_mfxExtAlphaChannelSurface;
    t_mfxExtAlphaChannelSurface.AlphaSurface = (mfxFrameSurface1 *)0x1515;
    EXPECT_EQ(t_mfxExtAlphaChannelSurface.AlphaSurface, (mfxFrameSurface1 *)0x1515);
}
