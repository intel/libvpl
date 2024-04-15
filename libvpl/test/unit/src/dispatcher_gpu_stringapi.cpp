/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "src/dispatcher_common.h"

static void InitBasicEncodeSession(mfxLoader &loader, mfxSession &session, mfxVideoParam &par) {
    mfxStatus sts = MFX_ERR_NONE;

    loader = nullptr;
    loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    sts = SetConfigImpl(loader, MFX_IMPL_TYPE_HARDWARE);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    session = nullptr;
    sts     = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // set to some basic encode parameters
    memset(&par, 0, sizeof(mfxVideoParam));
    par.mfx.CodecId                  = MFX_CODEC_AVC;
    par.mfx.TargetUsage              = MFX_TARGETUSAGE_BALANCED;
    par.mfx.TargetKbps               = 1000;
    par.mfx.RateControlMethod        = MFX_RATECONTROL_VBR;
    par.mfx.FrameInfo.FrameRateExtN  = 30;
    par.mfx.FrameInfo.FrameRateExtD  = 1;
    par.mfx.FrameInfo.FourCC         = MFX_FOURCC_NV12;
    par.mfx.FrameInfo.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
    par.mfx.FrameInfo.Width          = 1280;
    par.mfx.FrameInfo.Height         = 720;
    par.mfx.FrameInfo.CropX          = 0;
    par.mfx.FrameInfo.CropY          = 0;
    par.mfx.FrameInfo.CropW          = par.mfx.FrameInfo.Width;
    par.mfx.FrameInfo.CropH          = par.mfx.FrameInfo.Height;
    par.mfx.GopRefDist               = 4;
    par.mfx.GopPicSize               = 0;
    par.mfx.FrameInfo.BitDepthLuma   = 8;
    par.mfx.FrameInfo.BitDepthChroma = 8;
    par.mfx.FrameInfo.PicStruct      = 1;
    par.IOPattern                    = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
}

static void FreeBasicEncodeSession(mfxLoader &loader, mfxSession &session) {
    mfxStatus sts = MFXVideoENCODE_Close(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

TEST(Dispatcher_GPU_StringAPI, SetParameterValidEncode) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxStatus sts = MFX_ERR_NONE;

    mfxLoader loader    = nullptr;
    mfxSession session  = nullptr;
    mfxVideoParam par   = {};
    mfxExtBuffer extBuf = {};

    std::vector<mfxU8 *> extBufVector;
    InitBasicEncodeSession(loader, session, par);

    mfxConfigInterface *iface = nullptr;
    sts                       = MFXGetConfigInterface(session, &iface);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(iface, nullptr);

    sts = iface->SetParameter(iface,
                              (mfxU8 *)"TargetKbps",
                              (mfxU8 *)"1650",
                              MFX_STRUCTURE_TYPE_VIDEO_PARAM,
                              &par,
                              &extBuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXVideoENCODE_Init(session, &par);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxVideoParam parOut = {};
    sts                  = MFXVideoENCODE_GetVideoParam(session, &parOut);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    EXPECT_EQ(par.mfx.TargetKbps, 1650);
    EXPECT_EQ(parOut.mfx.TargetKbps, 1650);

    FreeBasicEncodeSession(loader, session);
}

// example of full init with videoParam, extBuf param, and checking with GetVideoParam
TEST(Dispatcher_GPU_StringAPI, SetExtBufValidNeedAllocEncode) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxStatus sts = MFX_ERR_NONE;

    mfxLoader loader    = nullptr;
    mfxSession session  = nullptr;
    mfxVideoParam par   = {};
    mfxExtBuffer extBuf = {};

    std::vector<mfxExtBuffer *> extBufVector = {};
    InitBasicEncodeSession(loader, session, par);

    mfxConfigInterface *iface = nullptr;
    sts                       = MFXGetConfigInterface(session, &iface);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(iface, nullptr);

    // set valid parameter in par
    sts = iface->SetParameter(iface,
                              (mfxU8 *)"TargetKbps",
                              (mfxU8 *)"3000",
                              MFX_STRUCTURE_TYPE_VIDEO_PARAM,
                              &par,
                              &extBuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // set valid parameter in an extBuf which is not yet allocated
    sts = iface->SetParameter(iface,
                              (mfxU8 *)"mfxExtCodingOption2.MaxQPI",
                              (mfxU8 *)"37",
                              MFX_STRUCTURE_TYPE_VIDEO_PARAM,
                              &par,
                              &extBuf);
    EXPECT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    // allocate a new extBuf of requested type and append to mfxVideoParam
    sts = AllocateExtBuf(par, extBufVector, extBuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // now try setting the extBuf parameter again
    sts = iface->SetParameter(iface,
                              (mfxU8 *)"mfxExtCodingOption2.MaxQPI",
                              (mfxU8 *)"37",
                              MFX_STRUCTURE_TYPE_VIDEO_PARAM,
                              &par,
                              &extBuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // encode init
    sts = MFXVideoENCODE_Init(session, &par);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // allocate extBuf to check output with GetVideoParam
    mfxVideoParam parOut           = {};
    mfxExtCodingOption2 extBufTest = {};
    extBufTest.Header.BufferId     = MFX_EXTBUFF_CODING_OPTION2;
    extBufTest.Header.BufferSz     = sizeof(mfxExtCodingOption2);
    mfxExtBuffer *extBufs[1]       = { (mfxExtBuffer *)(&extBufTest) };
    parOut.NumExtParam             = 1;
    parOut.ExtParam                = extBufs;

    sts = MFXVideoENCODE_GetVideoParam(session, &parOut);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    EXPECT_EQ(par.mfx.TargetKbps, 3000);
    EXPECT_EQ(parOut.mfx.TargetKbps, 3000);
    EXPECT_EQ(extBufTest.MaxQPI, 37);

    sts = MFXVideoENCODE_Close(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free allocated extBufs
    ReleaseExtBufs(extBufVector);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}
