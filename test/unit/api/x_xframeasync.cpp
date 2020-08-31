/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "api/test_bitstreams.h"
#include "vpl/mfxjpeg.h"
#include "vpl/mfxvideo.h"

/*!
    EncodeFrame overview
    Takes a single input frame in and generates its output bitstream. 

    If the encoder needs to cache the frame, the function locks the frame. 
    
    The BufferSizeInKB in the mfxVideoParam structure specifies maximum size for
    compressed frames. This value can also be obtained from MFXVideoENCODE_GetVideoParam.
       
    To mark the end of the encoding sequence, call this function with a NULL surface pointer. Repeat the call to drain any remaining internally cached bitstreams
    (one frame at a time) until MFX_ERR_MORE_DATA is returned.



   @return 
   MFX_ERR_NONE The function completed successfully. \n
   MFX_ERR_NOT_ENOUGH_BUFFER  The bitstream buffer size is insufficient. \n
   MFX_ERR_MORE_DATA   The function requires more data to generate any output. \n

  mfxStatus MFXVideoENCODE_EncodeFrameAsync(mfxSession session, mfxEncodeCtrl *ctrl, mfxFrameSurface1 *surface, mfxBitstream *bs, mfxSyncPoint *syncp);

*/

TEST(EncodeFrameAsync, ValidInputsReturnsErrNone) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));
    mfxEncParams.mfx.CodecId                = MFX_CODEC_JPEG;
    mfxEncParams.mfx.FrameInfo.FourCC       = MFX_FOURCC_I420;
    mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    mfxEncParams.mfx.FrameInfo.CropW        = 128;
    mfxEncParams.mfx.FrameInfo.CropH        = 96;
    mfxEncParams.mfx.FrameInfo.Width        = 128;
    mfxEncParams.mfx.FrameInfo.Height       = 96;
    mfxEncParams.IOPattern                  = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    mfxU16 nEncSurfNum = 16;
    mfxU32 lumaSize =
        mfxEncParams.mfx.FrameInfo.Width * mfxEncParams.mfx.FrameInfo.Height;

    mfxU8 *surfaceBuffers = new mfxU8[(mfxU32)(lumaSize * 1.5 * nEncSurfNum)];
    memset(surfaceBuffers, 0, (mfxU32)(lumaSize * 1.5 * nEncSurfNum));

    mfxFrameSurface1 *encSurfaces = new mfxFrameSurface1[nEncSurfNum];
    for (mfxI32 i = 0; i < nEncSurfNum; i++) {
        encSurfaces[i]        = { 0 };
        encSurfaces[i].Info   = mfxEncParams.mfx.FrameInfo;
        encSurfaces[i].Data.Y = &surfaceBuffers[(mfxU32)(lumaSize * 1.5 * i)];
        encSurfaces[i].Data.U = encSurfaces[i].Data.Y + lumaSize;
        encSurfaces[i].Data.V = encSurfaces[i].Data.U + lumaSize / 4;
        encSurfaces[i].Data.Pitch = mfxEncParams.mfx.FrameInfo.Width;
    }

    sts = MFXVideoENCODE_Init(session, &mfxEncParams);
    if (sts != MFX_ERR_NONE) {
        if (encSurfaces)
            delete[] encSurfaces;
        ASSERT_EQ(sts, MFX_ERR_NONE);
    }

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = 20000;
    mfxBS.Data         = new mfxU8[mfxBS.MaxLength];

    mfxI32 nEncSurfIdx = 0;
    mfxSyncPoint syncp;

    while (true) {
        // Encode a frame asynchronously (returns immediately)
        sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                              NULL,
                                              &encSurfaces[nEncSurfIdx],
                                              &mfxBS,
                                              &syncp);

        if (sts != MFX_ERR_MORE_DATA)
            break;
        nEncSurfIdx++;
    }
    ASSERT_GT(mfxBS.DataLength, 0);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    MFXClose(session);

    delete[] surfaceBuffers;
    delete[] encSurfaces;
    delete[] mfxBS.Data;
}

TEST(EncodeFrameAsync, InsufficientOutBufferReturnsNotEnoughBuffer) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));
    mfxEncParams.mfx.CodecId                = MFX_CODEC_JPEG;
    mfxEncParams.mfx.FrameInfo.FourCC       = MFX_FOURCC_I420;
    mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    mfxEncParams.mfx.FrameInfo.CropW        = 128;
    mfxEncParams.mfx.FrameInfo.CropH        = 96;
    mfxEncParams.mfx.FrameInfo.Width        = 128;
    mfxEncParams.mfx.FrameInfo.Height       = 96;
    mfxEncParams.IOPattern                  = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    mfxU16 nEncSurfNum = 16;
    mfxU32 lumaSize =
        mfxEncParams.mfx.FrameInfo.Width * mfxEncParams.mfx.FrameInfo.Height;

    mfxU8 *surfaceBuffers = new mfxU8[(mfxU32)(lumaSize * 1.5 * nEncSurfNum)];
    memset(surfaceBuffers, 0, (mfxU32)(lumaSize * 1.5 * nEncSurfNum));

    mfxFrameSurface1 *encSurfaces = new mfxFrameSurface1[nEncSurfNum];
    for (mfxI32 i = 0; i < nEncSurfNum; i++) {
        encSurfaces[i]        = { 0 };
        encSurfaces[i].Info   = mfxEncParams.mfx.FrameInfo;
        encSurfaces[i].Data.Y = &surfaceBuffers[(mfxU32)(lumaSize * 1.5 * i)];
        encSurfaces[i].Data.U = encSurfaces[i].Data.Y + lumaSize;
        encSurfaces[i].Data.V = encSurfaces[i].Data.U + lumaSize / 4;
        encSurfaces[i].Data.Pitch = mfxEncParams.mfx.FrameInfo.Width;
    }

    sts = MFXVideoENCODE_Init(session, &mfxEncParams);
    if (sts != MFX_ERR_NONE) {
        if (encSurfaces)
            delete[] encSurfaces;
        ASSERT_EQ(sts, MFX_ERR_NONE);
    }

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = 20;
    mfxBS.Data         = new mfxU8[mfxBS.MaxLength];

    mfxI32 nEncSurfIdx = 0;
    mfxSyncPoint syncp;

    while (true) {
        // Encode a frame asynchronously (returns immediately)
        sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                              NULL,
                                              &encSurfaces[nEncSurfIdx],
                                              &mfxBS,
                                              &syncp);

        if (sts != MFX_ERR_MORE_DATA)
            break;
        nEncSurfIdx++;
    }
    ASSERT_EQ(sts, MFX_ERR_NOT_ENOUGH_BUFFER);

    MFXClose(session);

    delete[] surfaceBuffers;
    delete[] encSurfaces;
    delete[] mfxBS.Data;
}

TEST(EncodeFrameAsync, NullSessionReturnsInvalidHandle) {
    mfxStatus sts =
        MFXVideoENCODE_EncodeFrameAsync(0, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(EncodeFrameAsync, NullBitstreamReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));
    mfxEncParams.mfx.CodecId                = MFX_CODEC_JPEG;
    mfxEncParams.mfx.FrameInfo.FourCC       = MFX_FOURCC_I420;
    mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    mfxEncParams.mfx.FrameInfo.CropW        = 128;
    mfxEncParams.mfx.FrameInfo.CropH        = 96;
    mfxEncParams.mfx.FrameInfo.Width        = 128;
    mfxEncParams.mfx.FrameInfo.Height       = 96;
    mfxEncParams.IOPattern                  = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    mfxU16 nEncSurfNum = 16;
    mfxU32 lumaSize =
        mfxEncParams.mfx.FrameInfo.Width * mfxEncParams.mfx.FrameInfo.Height;

    mfxU8 *surfaceBuffers = new mfxU8[(mfxU32)(lumaSize * 1.5 * nEncSurfNum)];
    memset(surfaceBuffers, 0, (mfxU32)(lumaSize * 1.5 * nEncSurfNum));

    mfxFrameSurface1 *encSurfaces = new mfxFrameSurface1[nEncSurfNum];
    for (mfxI32 i = 0; i < nEncSurfNum; i++) {
        encSurfaces[i]        = { 0 };
        encSurfaces[i].Info   = mfxEncParams.mfx.FrameInfo;
        encSurfaces[i].Data.Y = &surfaceBuffers[(mfxU32)(lumaSize * 1.5 * i)];
        encSurfaces[i].Data.U = encSurfaces[i].Data.Y + lumaSize;
        encSurfaces[i].Data.V = encSurfaces[i].Data.U + lumaSize / 4;
        encSurfaces[i].Data.Pitch = mfxEncParams.mfx.FrameInfo.Width;
    }

    sts = MFXVideoENCODE_Init(session, &mfxEncParams);
    if (sts != MFX_ERR_NONE) {
        if (encSurfaces)
            delete[] encSurfaces;
        ASSERT_EQ(sts, MFX_ERR_NONE);
    }

    mfxI32 nEncSurfIdx = 0;
    mfxSyncPoint syncp;

    // Encode a frame asynchronously (returns immediately)
    sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                          NULL,
                                          &encSurfaces[nEncSurfIdx],
                                          nullptr,
                                          &syncp);

    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    MFXClose(session);

    delete[] surfaceBuffers;
    delete[] encSurfaces;
}

TEST(EncodeFrameAsync, EncodeUninitializedReturnsNotInitialized) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxFrameSurface1 encSurfaces;

    mfxBitstream mfxBS = { 0 };

    mfxSyncPoint syncp;

    sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                          NULL,
                                          &encSurfaces,
                                          &mfxBS,
                                          &syncp);

    ASSERT_EQ(sts, MFX_ERR_NOT_INITIALIZED);

    MFXClose(session);
}

/*!
   DecodeFrame overview
   Decodes the input bitstream to a single output frame.

   MFX_ERR_NONE The function completed successfully and the output surface is ready for decoding \n
   MFX_ERR_MORE_DATA The function requires more bitstream at input before decoding can proceed. \n
   MFX_ERR_MORE_SURFACE The function requires more frame surface at output before decoding can proceed. \n
   MFX_WRN_VIDEO_PARAM_CHANGED  The decoder detected a new sequence header in the bitstream. Video parameters may have changed. \n

*/
//mfxStatus MFX_CDECL MFXVideoDECODE_DecodeFrameAsync(mfxSession session, mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 **surface_out, mfxSyncPoint *syncp);

TEST(DecodeFrameAsync, ValidInputsReturnsErrNone) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxDecParams;
    memset(&mfxDecParams, 0, sizeof(mfxDecParams));
    mfxDecParams.mfx.CodecId = MFX_CODEC_HEVC;
    mfxDecParams.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = mfxBS.DataLength =
        test_bitstream_96x64_8bit_hevc::getlen();
    mfxBS.Data = test_bitstream_96x64_8bit_hevc::getdata();

    sts = MFXVideoDECODE_DecodeHeader(session, &mfxBS, &mfxDecParams);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxU32 nSurfNumDec            = 8;
    mfxFrameSurface1 *decSurfaces = new mfxFrameSurface1[nSurfNumDec];
    mfxU32 surfW                  = mfxDecParams.mfx.FrameInfo.Width;
    mfxU32 surfH                  = mfxDecParams.mfx.FrameInfo.Height;

    mfxU8 *DECoutbuf = new mfxU8[(mfxU32)(surfW * surfH * nSurfNumDec * 1.5)];

    for (mfxU32 i = 0; i < nSurfNumDec; i++) {
        decSurfaces[i]        = { 0 };
        decSurfaces[i].Info   = mfxDecParams.mfx.FrameInfo;
        int buf_offset        = i * surfW * surfH;
        decSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        decSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        decSurfaces[i].Data.V =
            decSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        decSurfaces[i].Data.Pitch = surfW;
    }

    sts = MFXVideoDECODE_Init(session, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        if (decSurfaces)
            delete[] decSurfaces;
        ASSERT_EQ(sts, MFX_ERR_NONE);
    }

    mfxSyncPoint syncp;
    int nIndex                       = 0;
    mfxFrameSurface1 *pmfxOutSurface = nullptr;
    sts                              = MFXVideoDECODE_DecodeFrameAsync(session,
                                          &mfxBS,
                                          &decSurfaces[nIndex++],
                                          &pmfxOutSurface,
                                          &syncp);
    EXPECT_EQ(sts, MFX_ERR_MORE_DATA);

    sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                          nullptr,
                                          &decSurfaces[nIndex++],
                                          &pmfxOutSurface,
                                          &syncp);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(session);

    delete[] DECoutbuf;
    delete[] decSurfaces;
}

TEST(DecodeFrameAsync, CompleteFrameJPEGReturnsFrame) {
    mfxStatus sts = MFX_ERR_NONE;

    mfxVersion ver = {};
    mfxSession session;
    sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxDecParams = { 0 };
    mfxDecParams.mfx.CodecId   = MFX_CODEC_JPEG;
    mfxDecParams.IOPattern     = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength = mfxBS.DataLength = test_bitstream_32x32_mjpeg::getlen();
    mfxBS.Data                         = test_bitstream_32x32_mjpeg::getdata();

    sts = MFXVideoDECODE_DecodeHeader(session, &mfxBS, &mfxDecParams);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxU32 nSurfNumDec            = 8;
    mfxFrameSurface1 *decSurfaces = new mfxFrameSurface1[nSurfNumDec];
    mfxU32 surfW                  = mfxDecParams.mfx.FrameInfo.Width;
    mfxU32 surfH                  = mfxDecParams.mfx.FrameInfo.Height;

    mfxU8 *DECoutbuf = new mfxU8[(mfxU32)(surfW * surfH * nSurfNumDec * 1.5)];

    for (mfxU32 i = 0; i < nSurfNumDec; i++) {
        decSurfaces[i]        = { 0 };
        decSurfaces[i].Info   = mfxDecParams.mfx.FrameInfo;
        int buf_offset        = i * surfW * surfH;
        decSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        decSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        decSurfaces[i].Data.V =
            decSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        decSurfaces[i].Data.Pitch = surfW;
    }

    sts = MFXVideoDECODE_Init(session, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        if (decSurfaces)
            delete[] decSurfaces;
        ASSERT_EQ(sts, MFX_ERR_NONE);
    }

    mfxFrameSurface1 *pmfxOutSurface = nullptr;
    mfxSyncPoint syncp               = {};
    int nIndex                       = 0;

    mfxBS.DataFlag = MFX_BITSTREAM_COMPLETE_FRAME;

    int frame = 2; //use the 3rd frame in the bitstream

    mfxBS.Data = test_bitstream_32x32_mjpeg::getdata() +
                 test_bitstream_32x32_mjpeg::getpos(frame);
    mfxBS.DataLength = test_bitstream_32x32_mjpeg::getpos(frame + 1) -
                       test_bitstream_32x32_mjpeg::getpos(frame);
    mfxBS.DataOffset = 0;

    sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                          &mfxBS,
                                          &decSurfaces[nIndex++],
                                          &pmfxOutSurface,
                                          &syncp);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    delete[] DECoutbuf;
    delete[] decSurfaces;
}

TEST(DecodeFrameAsync, CompleteFrameHEVCReturnsFrame) {
    mfxStatus sts = MFX_ERR_NONE;

    mfxVersion ver = {};
    mfxSession session;
    sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
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

    mfxU32 nSurfNumDec            = 8;
    mfxFrameSurface1 *decSurfaces = new mfxFrameSurface1[nSurfNumDec];
    mfxU32 surfW                  = mfxDecParams.mfx.FrameInfo.Width;
    mfxU32 surfH                  = mfxDecParams.mfx.FrameInfo.Height;

    mfxU8 *DECoutbuf = new mfxU8[(mfxU32)(surfW * surfH * nSurfNumDec * 1.5)];

    for (mfxU32 i = 0; i < nSurfNumDec; i++) {
        decSurfaces[i]        = { 0 };
        decSurfaces[i].Info   = mfxDecParams.mfx.FrameInfo;
        int buf_offset        = i * surfW * surfH;
        decSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        decSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        decSurfaces[i].Data.V =
            decSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        decSurfaces[i].Data.Pitch = surfW;
    }

    sts = MFXVideoDECODE_Init(session, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        if (decSurfaces)
            delete[] decSurfaces;
        ASSERT_EQ(sts, MFX_ERR_NONE);
    }

    mfxFrameSurface1 *pmfxOutSurface = nullptr;
    mfxSyncPoint syncp               = {};
    int nIndex                       = 0;

    mfxBS.DataFlag = MFX_BITSTREAM_COMPLETE_FRAME;

    mfxBS.Data       = test_bitstream_96x64_8bit_hevc::getdata();
    mfxBS.DataLength = test_bitstream_96x64_8bit_hevc::getpos(1);
    mfxBS.DataOffset = 0;

    sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                          &mfxBS,
                                          &decSurfaces[nIndex++],
                                          &pmfxOutSurface,
                                          &syncp);
    ASSERT_EQ(sts, MFX_ERR_MORE_DATA);

    sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                          nullptr,
                                          &decSurfaces[nIndex++],
                                          &pmfxOutSurface,
                                          &syncp);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    delete[] DECoutbuf;
    delete[] decSurfaces;
}

TEST(DecodeFrameAsync, EoSFlagReturnsFrame) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxDecParams;
    memset(&mfxDecParams, 0, sizeof(mfxDecParams));
    mfxDecParams.mfx.CodecId = MFX_CODEC_HEVC;
    mfxDecParams.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = mfxBS.DataLength =
        test_bitstream_96x64_8bit_hevc::getlen();
    mfxBS.Data = test_bitstream_96x64_8bit_hevc::getdata();

    sts = MFXVideoDECODE_DecodeHeader(session, &mfxBS, &mfxDecParams);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxU32 nSurfNumDec            = 8;
    mfxFrameSurface1 *decSurfaces = new mfxFrameSurface1[nSurfNumDec];
    mfxU32 surfW                  = mfxDecParams.mfx.FrameInfo.Width;
    mfxU32 surfH                  = mfxDecParams.mfx.FrameInfo.Height;

    mfxU8 *DECoutbuf = new mfxU8[(mfxU32)(surfW * surfH * nSurfNumDec * 1.5)];

    for (mfxU32 i = 0; i < nSurfNumDec; i++) {
        decSurfaces[i]        = { 0 };
        decSurfaces[i].Info   = mfxDecParams.mfx.FrameInfo;
        int buf_offset        = i * surfW * surfH;
        decSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        decSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        decSurfaces[i].Data.V =
            decSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        decSurfaces[i].Data.Pitch = surfW;
    }

    sts = MFXVideoDECODE_Init(session, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        if (decSurfaces)
            delete[] decSurfaces;
        ASSERT_EQ(sts, MFX_ERR_NONE);
    }

    mfxSyncPoint syncp;
    int nIndex = 0;

    mfxFrameSurface1 *pmfxOutSurface = nullptr;
    sts                              = MFXVideoDECODE_DecodeFrameAsync(session,
                                          &mfxBS,
                                          &decSurfaces[nIndex++],
                                          &pmfxOutSurface,
                                          &syncp);
    EXPECT_EQ(sts, MFX_ERR_MORE_DATA);

    mfxBS.DataFlag = MFX_BITSTREAM_EOS;
    sts            = MFXVideoDECODE_DecodeFrameAsync(session,
                                          &mfxBS,
                                          &decSurfaces[nIndex++],
                                          &pmfxOutSurface,
                                          &syncp);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    delete[] DECoutbuf;
    delete[] decSurfaces;
}

TEST(DecodeFrameAsync, InsufficientInBitstreamReturnsMoreData) {
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

    mfxU32 nSurfNumDec            = 8;
    mfxFrameSurface1 *decSurfaces = new mfxFrameSurface1[nSurfNumDec];
    mfxU32 surfW                  = mfxDecParams.mfx.FrameInfo.Width;
    mfxU32 surfH                  = mfxDecParams.mfx.FrameInfo.Height;

    mfxU8 *DECoutbuf = new mfxU8[(mfxU32)(surfW * surfH * nSurfNumDec * 1.5)];

    for (mfxU32 i = 0; i < nSurfNumDec; i++) {
        decSurfaces[i]        = { 0 };
        decSurfaces[i].Info   = mfxDecParams.mfx.FrameInfo;
        int buf_offset        = i * surfW * surfH;
        decSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        decSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        decSurfaces[i].Data.V =
            decSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        decSurfaces[i].Data.Pitch = surfW;
    }

    sts = MFXVideoDECODE_Init(session, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        if (decSurfaces)
            delete[] decSurfaces;
        ASSERT_EQ(sts, MFX_ERR_NONE);
    }

    mfxBS.MaxLength = mfxBS.DataLength = 1;
    mfxSyncPoint syncp;
    int nIndex                       = 0;
    mfxFrameSurface1 *pmfxOutSurface = nullptr;
    sts                              = MFXVideoDECODE_DecodeFrameAsync(session,
                                          &mfxBS,
                                          &decSurfaces[nIndex++],
                                          &pmfxOutSurface,
                                          &syncp);
    EXPECT_EQ(sts, MFX_ERR_MORE_DATA);

    sts = MFXClose(session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    delete[] DECoutbuf;
    delete[] decSurfaces;
}

TEST(DecodeFrameAsync, DISABLED_InsufficientSurfacesReturnsMoreSurface) {
    FAIL() << "Test not implemented";
}

TEST(DecodeFrameAsync, NullSessionReturnsInvalidHandle) {
    mfxStatus sts =
        MFXVideoDECODE_DecodeFrameAsync(0, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(DecodeFrameAsync, DISABLED_NullSurfaceWorkReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxDecParams;
    memset(&mfxDecParams, 0, sizeof(mfxDecParams));
    mfxDecParams.mfx.CodecId = MFX_CODEC_HEVC;
    mfxDecParams.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = mfxBS.DataLength =
        test_bitstream_96x64_8bit_hevc::getlen();
    mfxBS.Data = test_bitstream_96x64_8bit_hevc::getdata();

    sts = MFXVideoDECODE_DecodeHeader(session, &mfxBS, &mfxDecParams);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxU32 nSurfNumDec            = 8;
    mfxFrameSurface1 *decSurfaces = new mfxFrameSurface1[nSurfNumDec];
    mfxU32 surfW                  = mfxDecParams.mfx.FrameInfo.Width;
    mfxU32 surfH                  = mfxDecParams.mfx.FrameInfo.Height;

    mfxU8 *DECoutbuf = new mfxU8[(mfxU32)(surfW * surfH * nSurfNumDec * 1.5)];

    for (mfxU32 i = 0; i < nSurfNumDec; i++) {
        decSurfaces[i]        = { 0 };
        decSurfaces[i].Info   = mfxDecParams.mfx.FrameInfo;
        int buf_offset        = i * surfW * surfH;
        decSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        decSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        decSurfaces[i].Data.V =
            decSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        decSurfaces[i].Data.Pitch = surfW;
    }

    sts = MFXVideoDECODE_Init(session, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        if (decSurfaces)
            delete[] decSurfaces;
        ASSERT_EQ(sts, MFX_ERR_NONE);
    }

    mfxBS.MaxLength = mfxBS.DataLength = 1;
    mfxSyncPoint syncp;
    mfxFrameSurface1 *pmfxOutSurface = nullptr;
    sts                              = MFXVideoDECODE_DecodeFrameAsync(session,
                                          &mfxBS,
                                          nullptr,
                                          &pmfxOutSurface,
                                          &syncp);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);

    delete[] DECoutbuf;
    delete[] decSurfaces;
}

TEST(DecodeFrameAsync, NullSurfaceOutReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxDecParams;
    memset(&mfxDecParams, 0, sizeof(mfxDecParams));
    mfxDecParams.mfx.CodecId = MFX_CODEC_HEVC;
    mfxDecParams.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = mfxBS.DataLength =
        test_bitstream_96x64_8bit_hevc::getlen();
    mfxBS.Data = test_bitstream_96x64_8bit_hevc::getdata();

    sts = MFXVideoDECODE_DecodeHeader(session, &mfxBS, &mfxDecParams);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxU32 nSurfNumDec            = 8;
    mfxFrameSurface1 *decSurfaces = new mfxFrameSurface1[nSurfNumDec];
    mfxU32 surfW                  = mfxDecParams.mfx.FrameInfo.Width;
    mfxU32 surfH                  = mfxDecParams.mfx.FrameInfo.Height;

    mfxU8 *DECoutbuf = new mfxU8[(mfxU32)(surfW * surfH * nSurfNumDec * 1.5)];

    for (mfxU32 i = 0; i < nSurfNumDec; i++) {
        decSurfaces[i]        = { 0 };
        decSurfaces[i].Info   = mfxDecParams.mfx.FrameInfo;
        int buf_offset        = i * surfW * surfH;
        decSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        decSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        decSurfaces[i].Data.V =
            decSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        decSurfaces[i].Data.Pitch = surfW;
    }

    sts = MFXVideoDECODE_Init(session, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        if (decSurfaces)
            delete[] decSurfaces;
        ASSERT_EQ(sts, MFX_ERR_NONE);
    }

    mfxBS.MaxLength = mfxBS.DataLength = 1;
    int nIndex                         = 0;
    mfxFrameSurface1 *pmfxOutSurface   = nullptr;
    sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                          &mfxBS,
                                          &decSurfaces[nIndex++],
                                          &pmfxOutSurface,
                                          nullptr);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);

    delete[] DECoutbuf;
    delete[] decSurfaces;
}

TEST(DecodeFrameAsync, NullSyncpReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxDecParams;
    memset(&mfxDecParams, 0, sizeof(mfxDecParams));
    mfxDecParams.mfx.CodecId = MFX_CODEC_HEVC;
    mfxDecParams.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = mfxBS.DataLength =
        test_bitstream_96x64_8bit_hevc::getlen();
    mfxBS.Data = test_bitstream_96x64_8bit_hevc::getdata();

    sts = MFXVideoDECODE_DecodeHeader(session, &mfxBS, &mfxDecParams);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxU32 nSurfNumDec            = 8;
    mfxFrameSurface1 *decSurfaces = new mfxFrameSurface1[nSurfNumDec];
    mfxU32 surfW                  = mfxDecParams.mfx.FrameInfo.Width;
    mfxU32 surfH                  = mfxDecParams.mfx.FrameInfo.Height;

    mfxU8 *DECoutbuf = new mfxU8[(mfxU32)(surfW * surfH * nSurfNumDec * 1.5)];

    for (mfxU32 i = 0; i < nSurfNumDec; i++) {
        decSurfaces[i]        = { 0 };
        decSurfaces[i].Info   = mfxDecParams.mfx.FrameInfo;
        int buf_offset        = i * surfW * surfH;
        decSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        decSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        decSurfaces[i].Data.V =
            decSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        decSurfaces[i].Data.Pitch = surfW;
    }

    sts = MFXVideoDECODE_Init(session, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        if (decSurfaces)
            delete[] decSurfaces;
        ASSERT_EQ(sts, MFX_ERR_NONE);
    }

    mfxBS.MaxLength = mfxBS.DataLength = 1;
    mfxSyncPoint syncp;
    int nIndex = 0;
    sts        = MFXVideoDECODE_DecodeFrameAsync(session,
                                          &mfxBS,
                                          &decSurfaces[nIndex++],
                                          nullptr,
                                          &syncp);
    EXPECT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);

    delete[] DECoutbuf;
    delete[] decSurfaces;
}

TEST(DecodeFrameAsync, DecodeUninitializedReturnsNotInitialized) {
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

    mfxU32 nSurfNumDec            = 8;
    mfxFrameSurface1 *decSurfaces = new mfxFrameSurface1[nSurfNumDec];
    mfxU32 surfW                  = mfxDecParams.mfx.FrameInfo.Width;
    mfxU32 surfH                  = mfxDecParams.mfx.FrameInfo.Height;

    mfxU8 *DECoutbuf = new mfxU8[(mfxU32)(surfW * surfH * nSurfNumDec * 1.5)];

    for (mfxU32 i = 0; i < nSurfNumDec; i++) {
        decSurfaces[i]        = { 0 };
        decSurfaces[i].Info   = mfxDecParams.mfx.FrameInfo;
        int buf_offset        = i * surfW * surfH;
        decSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        decSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        decSurfaces[i].Data.V =
            decSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        decSurfaces[i].Data.Pitch = surfW;
    }

    mfxSyncPoint syncp;
    int nIndex                       = 0;
    mfxFrameSurface1 *pmfxOutSurface = nullptr;
    sts                              = MFXVideoDECODE_DecodeFrameAsync(session,
                                          &mfxBS,
                                          &decSurfaces[nIndex++],
                                          &pmfxOutSurface,
                                          &syncp);
    EXPECT_EQ(sts, MFX_ERR_NOT_INITIALIZED);

    sts = MFXClose(session);

    delete[] DECoutbuf;
    delete[] decSurfaces;
}
/*!
   RunFrameVPPAsync overview
   Processes a single input frame to a single output frame. 

   @param[in] session SDK session handle.
   @param[in] in  Pointer to the input video surface structure
   @param[out] out  Pointer to the output video surface structure
   @param[in] aux  Optional pointer to the auxiliary data structure
   @param[out] syncp  Pointer to the output sync point

   @return 
   MFX_ERR_NONE The output frame is ready after synchronization. \n
   
*/
//mfxStatus MFX_CDECL MFXVideoVPP_RunFrameVPPAsync(mfxSession session, mfxFrameSurface1 *in, mfxFrameSurface1 *out, mfxExtVppAuxData *aux, mfxSyncPoint *syncp);

TEST(RunFrameVPPAsync, ValidInputsReturnsErrNone) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxVPPParams;
    memset(&mfxVPPParams, 0, sizeof(mfxVPPParams));

    // Input data
    mfxVPPParams.vpp.In.FourCC        = MFX_FOURCC_I420;
    mfxVPPParams.vpp.In.CropW         = 128;
    mfxVPPParams.vpp.In.CropH         = 96;
    mfxVPPParams.vpp.In.FrameRateExtN = 30;
    mfxVPPParams.vpp.In.FrameRateExtD = 1;
    mfxVPPParams.vpp.In.Width         = mfxVPPParams.vpp.In.CropW;
    mfxVPPParams.vpp.In.Height        = mfxVPPParams.vpp.In.CropH;
    // Output data
    mfxVPPParams.vpp.Out = mfxVPPParams.vpp.In;

    mfxVPPParams.IOPattern =
        MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    sts = MFXVideoVPP_Init(session, &mfxVPPParams);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxU32 nSurfNum               = 2;
    mfxFrameSurface1 *vppSurfaces = new mfxFrameSurface1[nSurfNum];
    mfxU32 surfW                  = mfxVPPParams.vpp.In.Width;
    mfxU32 surfH                  = mfxVPPParams.vpp.In.Height;

    mfxU8 *DECoutbuf = new mfxU8[(mfxU32)(surfW * surfH * nSurfNum * 1.5)];

    for (mfxU32 i = 0; i < nSurfNum; i++) {
        vppSurfaces[i]        = { 0 };
        vppSurfaces[i].Info   = mfxVPPParams.mfx.FrameInfo;
        int buf_offset        = i * surfW * surfH;
        vppSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        vppSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        vppSurfaces[i].Data.V =
            vppSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        vppSurfaces[i].Data.Pitch = surfW;
    }

    mfxSyncPoint syncp;
    sts = MFXVideoVPP_RunFrameVPPAsync(session,
                                       &vppSurfaces[0],
                                       &vppSurfaces[1],
                                       nullptr,
                                       &syncp);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(RunFrameVPPAsync, NullSessionReturnsInvalidHandle) {
    mfxStatus sts =
        MFXVideoVPP_RunFrameVPPAsync(0, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(sts, MFX_ERR_INVALID_HANDLE);
}

TEST(RunFrameVPPAsync, NullSurfaceOutReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxVPPParams;
    memset(&mfxVPPParams, 0, sizeof(mfxVPPParams));

    // Input data
    mfxVPPParams.vpp.In.FourCC        = MFX_FOURCC_I420;
    mfxVPPParams.vpp.In.CropW         = 128;
    mfxVPPParams.vpp.In.CropH         = 96;
    mfxVPPParams.vpp.In.FrameRateExtN = 30;
    mfxVPPParams.vpp.In.FrameRateExtD = 1;
    mfxVPPParams.vpp.In.Width         = mfxVPPParams.vpp.In.CropW;
    mfxVPPParams.vpp.In.Height        = mfxVPPParams.vpp.In.CropH;
    // Output data
    mfxVPPParams.vpp.Out       = mfxVPPParams.vpp.In;
    mfxVPPParams.vpp.In.FourCC = MFX_FOURCC_I420;
    mfxVPPParams.IOPattern =
        MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    sts = MFXVideoVPP_Init(session, &mfxVPPParams);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxU32 nSurfNum               = 2;
    mfxFrameSurface1 *vppSurfaces = new mfxFrameSurface1[nSurfNum];
    mfxU32 surfW                  = mfxVPPParams.vpp.In.Width;
    mfxU32 surfH                  = mfxVPPParams.vpp.In.Height;

    mfxU8 *DECoutbuf = new mfxU8[(mfxU32)(surfW * surfH * nSurfNum * 1.5)];

    for (mfxU32 i = 0; i < nSurfNum; i++) {
        vppSurfaces[i]        = { 0 };
        vppSurfaces[i].Info   = mfxVPPParams.mfx.FrameInfo;
        int buf_offset        = i * surfW * surfH;
        vppSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        vppSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        vppSurfaces[i].Data.V =
            vppSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        vppSurfaces[i].Data.Pitch = surfW;
    }

    mfxSyncPoint syncp;
    sts = MFXVideoVPP_RunFrameVPPAsync(session,
                                       &vppSurfaces[0],
                                       nullptr,
                                       nullptr,
                                       &syncp);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(RunFrameVPPAsync, NullSyncpOutReturnsErrNull) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxVPPParams;
    memset(&mfxVPPParams, 0, sizeof(mfxVPPParams));

    // Input data
    mfxVPPParams.vpp.In.FourCC        = MFX_FOURCC_I420;
    mfxVPPParams.vpp.In.CropW         = 128;
    mfxVPPParams.vpp.In.CropH         = 96;
    mfxVPPParams.vpp.In.FrameRateExtN = 30;
    mfxVPPParams.vpp.In.FrameRateExtD = 1;
    mfxVPPParams.vpp.In.Width         = mfxVPPParams.vpp.In.CropW;
    mfxVPPParams.vpp.In.Height        = mfxVPPParams.vpp.In.CropH;
    // Output data
    mfxVPPParams.vpp.Out       = mfxVPPParams.vpp.In;
    mfxVPPParams.vpp.In.FourCC = MFX_FOURCC_I420;
    mfxVPPParams.IOPattern =
        MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    sts = MFXVideoVPP_Init(session, &mfxVPPParams);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxU32 nSurfNum               = 2;
    mfxFrameSurface1 *vppSurfaces = new mfxFrameSurface1[nSurfNum];
    mfxU32 surfW                  = mfxVPPParams.vpp.In.Width;
    mfxU32 surfH                  = mfxVPPParams.vpp.In.Height;

    mfxU8 *DECoutbuf = new mfxU8[(mfxU32)(surfW * surfH * nSurfNum * 1.5)];

    for (mfxU32 i = 0; i < nSurfNum; i++) {
        vppSurfaces[i]        = { 0 };
        vppSurfaces[i].Info   = mfxVPPParams.mfx.FrameInfo;
        int buf_offset        = i * surfW * surfH;
        vppSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        vppSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        vppSurfaces[i].Data.V =
            vppSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        vppSurfaces[i].Data.Pitch = surfW;
    }

    sts = MFXVideoVPP_RunFrameVPPAsync(session,
                                       &vppSurfaces[0],
                                       &vppSurfaces[0],
                                       nullptr,
                                       nullptr);
    ASSERT_EQ(sts, MFX_ERR_NULL_PTR);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

TEST(RunFrameVPPAsync, VPPUninitializedReturnsNotInitialized) {
    mfxVersion ver = {};
    mfxSession session;
    mfxStatus sts = MFXInit(MFX_IMPL_SOFTWARE, &ver, &session);
    ASSERT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam mfxVPPParams;
    memset(&mfxVPPParams, 0, sizeof(mfxVPPParams));

    // Input data
    mfxVPPParams.vpp.In.FourCC        = MFX_FOURCC_I420;
    mfxVPPParams.vpp.In.CropW         = 128;
    mfxVPPParams.vpp.In.CropH         = 96;
    mfxVPPParams.vpp.In.FrameRateExtN = 30;
    mfxVPPParams.vpp.In.FrameRateExtD = 1;
    mfxVPPParams.vpp.In.Width         = mfxVPPParams.vpp.In.CropW;
    mfxVPPParams.vpp.In.Height        = mfxVPPParams.vpp.In.CropH;
    // Output data
    mfxVPPParams.vpp.Out       = mfxVPPParams.vpp.In;
    mfxVPPParams.vpp.In.FourCC = MFX_FOURCC_I420;
    mfxVPPParams.IOPattern =
        MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    mfxU32 nSurfNum               = 2;
    mfxFrameSurface1 *vppSurfaces = new mfxFrameSurface1[nSurfNum];
    mfxU32 surfW                  = mfxVPPParams.vpp.In.Width;
    mfxU32 surfH                  = mfxVPPParams.vpp.In.Height;

    mfxU8 *DECoutbuf = new mfxU8[(mfxU32)(surfW * surfH * nSurfNum * 1.5)];

    for (mfxU32 i = 0; i < nSurfNum; i++) {
        vppSurfaces[i]        = { 0 };
        vppSurfaces[i].Info   = mfxVPPParams.mfx.FrameInfo;
        int buf_offset        = i * surfW * surfH;
        vppSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        vppSurfaces[i].Data.U = DECoutbuf + buf_offset + (surfW * surfH);
        vppSurfaces[i].Data.V =
            vppSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        vppSurfaces[i].Data.Pitch = surfW;
    }

    mfxSyncPoint syncp;
    sts = MFXVideoVPP_RunFrameVPPAsync(session,
                                       &vppSurfaces[0],
                                       &vppSurfaces[1],
                                       nullptr,
                                       &syncp);
    ASSERT_EQ(sts, MFX_ERR_NOT_INITIALIZED);

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}
