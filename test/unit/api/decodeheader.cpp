
/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
#include <gtest/gtest.h>
#include "api/test_bitstreams.h"
#include "vpl/mfxvideo.h"

/* DecodeHeader overview
  Parses the input bitstream and fills the mfxVideoParam structure with appropriate values, 
  such as resolution and frame rate, for the Init function. 
  The application can then pass the resulting structure to the MFXVideoDECODE_Init function for decoder initialization.

  An application can call this function at any time before or after decoder initialization. 
  If the SDK finds a sequence header in the bitstream, the function moves the bitstream pointer to 
  the first bit of the sequence header. 
   MFX_ERR_NONE The function successfully filled structure. It does not mean that the stream can be decoded by SDK. 
                The application should call MFXVideoDECODE_Query function to check if decoding of the stream is supported. \n
   MFX_ERR_MORE_DATA   The function requires more bitstream data \n
   MFX_ERR_UNSUPPORTED  CodecId field of the mfxVideoParam structure indicates some unsupported codec. \n
   MFX_ERR_INVALID_HANDLE  session is not initialized \n
   MFX_ERR_NULL_PTR  bs or par pointer is NULL.
*/

TEST(DecodeHeader, EightBitInReturnsCorrectMetadata) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxDecParams = { 0 };
    mfxDecParams.mfx.CodecId   = MFX_CODEC_HEVC;
    mfxDecParams.IOPattern     = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = mfxBS.DataLength =
        test_bitstream_96x64_8bit_hevc::getlen();
    mfxBS.Data = test_bitstream_96x64_8bit_hevc::getdata();

    sts = MFXVideoDECODE_DecodeHeader(session, &mfxBS, &mfxDecParams);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    //check metadata
    ASSERT_EQ(96, mfxDecParams.mfx.FrameInfo.Width);
    ASSERT_EQ(64, mfxDecParams.mfx.FrameInfo.Height);
    ASSERT_EQ(96, mfxDecParams.mfx.FrameInfo.CropW);
    ASSERT_EQ(64, mfxDecParams.mfx.FrameInfo.CropH);
    ASSERT_EQ(MFX_FOURCC_I420, mfxDecParams.mfx.FrameInfo.FourCC);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeHeader, TenBitInReturnsCorrectMetadata) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxDecParams = { 0 };
    mfxDecParams.mfx.CodecId   = MFX_CODEC_HEVC;
    mfxDecParams.IOPattern     = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = mfxBS.DataLength =
        test_bitstream_96x64_10bit_hevc::getlen();
    mfxBS.Data = test_bitstream_96x64_10bit_hevc::getdata();

    sts = MFXVideoDECODE_DecodeHeader(session, &mfxBS, &mfxDecParams);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    //check metadata
    ASSERT_EQ(96, mfxDecParams.mfx.FrameInfo.Width);
    ASSERT_EQ(64, mfxDecParams.mfx.FrameInfo.Height);
    ASSERT_EQ(96, mfxDecParams.mfx.FrameInfo.CropW);
    ASSERT_EQ(64, mfxDecParams.mfx.FrameInfo.CropH);
    ASSERT_EQ(MFX_FOURCC_I010, mfxDecParams.mfx.FrameInfo.FourCC);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeHeader, NullSessionReturnsInvalidHandle) {
    mfxStatus sts = MFXVideoDECODE_DecodeHeader(0, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(DecodeHeader, NullBitstreamInReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxDecParams = { 0 };
    sts = MFXVideoDECODE_DecodeHeader(session, nullptr, &mfxDecParams);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(DecodeHeader, NullParamsInReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxBitstream mfxBS = { 0 };
    sts                = MFXVideoDECODE_DecodeHeader(session, &mfxBS, nullptr);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}
