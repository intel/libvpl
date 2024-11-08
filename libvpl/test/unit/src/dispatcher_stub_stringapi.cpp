/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "src/dispatcher_common.h"

// Fixture class to allow reuse of objects across tests
class StringAPITest : public ::testing::Test {
protected:
    void SetUp() override {
        SKIP_IF_DISP_STUB_DISABLED();
        loader_ = MFXLoad();
        ASSERT_NE(loader_, nullptr);
        mfxStatus sts = SetConfigImpl(loader_, MFX_IMPL_TYPE_STUB);
        ASSERT_EQ(sts, MFX_ERR_NONE);
        // create session with first implementation
        sts = MFXCreateSession(loader_, 0, &session_);
        ASSERT_EQ(sts, MFX_ERR_NONE);

        sts = MFXGetConfigInterface(session_, &config_interface_);
        ASSERT_EQ(sts, MFX_ERR_NONE);
        ASSERT_NE(config_interface_, nullptr);
    }

    void TearDown() override {
        MFXClose(session_);
        MFXUnload(loader_);
    }

    mfxStatus SetVideoParameter(mfxU8 *key,
                                mfxU8 *value,
                                mfxVideoParam *par,
                                mfxExtBuffer *extBuf) {
        return config_interface_->SetParameter(config_interface_,
                                               key,
                                               value,
                                               MFX_STRUCTURE_TYPE_VIDEO_PARAM,
                                               par,
                                               extBuf);
    }

    mfxLoader loader_                     = nullptr;
    mfxSession session_                   = nullptr;
    mfxConfigInterface *config_interface_ = nullptr;
};

TEST_F(StringAPITest, SetParameterErrNotFound) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    mfxU8 *key   = (mfxU8 *)"BadParameter";
    mfxU8 *value = (mfxU8 *)"5000";

    // key is an unknown parameter
    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);
}

TEST_F(StringAPITest, SetParameterErrUnsupported) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    mfxU8 *key   = (mfxU8 *)"MaxKbps";
    mfxU8 *value = (mfxU8 *)"ABCD";

    // wrong value Type
    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);
}

TEST_F(StringAPITest, SetParameterExtBufValidNeedAlloc1) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    mfxU8 *key   = (mfxU8 *)"mfxExtHEVCParam.PicWidthInLumaSamples";
    mfxU8 *value = (mfxU8 *)"640";

    std::vector<mfxExtBuffer *> extBufVector = {};

    // set valid parameter, expect error because extBuf not allocated
    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    // allocate a new extBuf of requested type and append to mfxVideoParam
    sts = AllocateExtBuf(param, extBufVector, extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // try again now that we have attached the correct extBuf
    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxExtHEVCParam *extBufOut = (mfxExtHEVCParam *)FindExtBuf(param, MFX_EXTBUFF_HEVC_PARAM);
    EXPECT_NE(extBufOut, nullptr);

    // check that value was set correctly in extBuf
    if (extBufOut && extBufOut->Header.BufferId == MFX_EXTBUFF_HEVC_PARAM) {
        EXPECT_EQ(extBufOut->PicWidthInLumaSamples, 640);
    }

    // free allocated extBufs
    ReleaseExtBufs(extBufVector);
}
TEST_F(StringAPITest, SetParameterExtBufValidNeedAlloc2) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    std::vector<mfxExtBuffer *> extBufVector = {};

    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.RateDistortionOpt";
    mfxU8 *value = (mfxU8 *)"1";

    // set valid parameter, expect error because extBuf not allocated
    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    // allocate a new extBuf of requested type and append to mfxVideoParam
    sts = AllocateExtBuf(param, extBufVector, extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // try again now that we have attached the correct extBuf
    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxExtCodingOption *extBufOut =
        (mfxExtCodingOption *)FindExtBuf(param, MFX_EXTBUFF_CODING_OPTION);
    EXPECT_NE(extBufOut, nullptr);

    // check that value was set correctly in extBuf
    if (extBufOut && extBufOut->Header.BufferId == MFX_EXTBUFF_CODING_OPTION) {
        EXPECT_EQ(extBufOut->RateDistortionOpt, 1);
    }

    // free allocated extBufs
    ReleaseExtBufs(extBufVector);
}

TEST_F(StringAPITest, SetParameterArrayValid) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    mfxU8 *key   = (mfxU8 *)"SamplingFactorH[]";
    mfxU8 *value = (mfxU8 *)"2,5,  19  ,    40"; // extra whitespace is ignored

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    EXPECT_EQ(param.mfx.SamplingFactorH[0], 2);
    EXPECT_EQ(param.mfx.SamplingFactorH[1], 5);
    EXPECT_EQ(param.mfx.SamplingFactorH[2], 19);
    EXPECT_EQ(param.mfx.SamplingFactorH[3], 40);
}

TEST_F(StringAPITest, SetCodecIdWithFourCC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    mfxU8 *key   = (mfxU8 *)"CodecId";
    mfxU8 *value = (mfxU8 *)"NV12";

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.CodecId, 842094158);

    key   = (mfxU8 *)"CodecId";
    value = (mfxU8 *)"1111";

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.CodecId, 825307441);

    key   = (mfxU8 *)"CodecId";
    value = (mfxU8 *)"+1111";

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.CodecId, 1111);

    key   = (mfxU8 *)"CodecId";
    value = (mfxU8 *)"AVC ";

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.CodecId, 541283905);
}

TEST_F(StringAPITest, SetFourCCWithFourCC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    mfxU8 *key   = (mfxU8 *)"FourCC";
    mfxU8 *value = (mfxU8 *)"NV12";

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.FourCC, 842094158);
}

TEST_F(StringAPITest, SetvppInFourCCWithFourCC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    mfxU8 *key   = (mfxU8 *)"vpp.In.FourCC";
    mfxU8 *value = (mfxU8 *)"NV12";

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.FourCC, 842094158);
}

TEST_F(StringAPITest, SetvppOutFourCCWithFourCC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    mfxU8 *key   = (mfxU8 *)"vpp.Out.FourCC";
    mfxU8 *value = (mfxU8 *)"NV12";

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.FourCC, 842094158);
}

TEST_F(StringAPITest, SetmfxExtDecVideoProcessingOutFourCCWithFourCC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    mfxU8 *key   = (mfxU8 *)"mfxExtDecVideoProcessing.Out.FourCC";
    mfxU8 *value = (mfxU8 *)"NV12";

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecVideoProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Out.FourCC, 842094158);
}

/*

TEST(Dispatcher_Stub_StringAPI, SetParameterErrNotFound) {
    SKIP_IF_DISP_STUB_DISABLED();

    StubSession session;
    mfxVideoParam par   = {};
    mfxExtBuffer extBuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    mfxConfigInterface *iface = nullptr;
    sts                       = MFXGetConfigInterface(session.handle(), &iface);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(iface, nullptr);

    // key is an unknown parameter
    sts = iface->SetParameter(iface,
                              (mfxU8 *)"BadParameter",
                              (mfxU8 *)"5000",
                              MFX_STRUCTURE_TYPE_VIDEO_PARAM,
                              &par,
                              &extBuf);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);
}

TEST(Dispatcher_Stub_StringAPI, SetParameterErrUnsupported) {
    SKIP_IF_DISP_STUB_DISABLED();

    StubSession session;
    mfxVideoParam par   = {};
    mfxExtBuffer extBuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    mfxConfigInterface *iface = nullptr;
    sts                       = MFXGetConfigInterface(session.handle(), &iface);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_NE(iface, nullptr);

    // wrong value Type
    sts = iface->SetParameter(iface,
                              (mfxU8 *)"MaxKbps",
                              (mfxU8 *)"ABCD",
                              MFX_STRUCTURE_TYPE_VIDEO_PARAM,
                              &par,
                              &extBuf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);
}
*/

// mfxU16,mfxExtPredWeightTable.LumaWeightFlag[2][32],nd-array
TEST_F(StringAPITest, SetmfxExtPredWeightTableLumaWeightFlag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPredWeightTable.LumaWeightFlag[]";
    mfxU8 *value = (mfxU8 *)"16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 16, 17, 18, 19, 20, 21, 22, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 16, 17, 18, 19, 20, 21, 22";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPredWeightTable *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 32; j++) {
            EXPECT_EQ(ext->LumaWeightFlag[i][j], 16 + j % 25);
        }
    }
}

// mfxU16,mfxExtPredWeightTable.ChromaWeightFlag[2][32],nd-array
TEST_F(StringAPITest, SetmfxExtPredWeightTableChromaWeightFlag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPredWeightTable.ChromaWeightFlag[]";
    mfxU8 *value = (mfxU8 *)"16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 16, 17, 18, 19, 20, 21, 22, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 16, 17, 18, 19, 20, 21, 22";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPredWeightTable *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 32; j++) {
            EXPECT_EQ(ext->ChromaWeightFlag[i][j], 16 + j % 25);
        }
    }
}

// mfxI16,mfxExtPredWeightTable.Weights[2][32][3][2],nd-array
TEST_F(StringAPITest, SetmfxExtPredWeightTableWeights) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPredWeightTable.Weights[]";
    mfxU8 *value = (mfxU8 *)"-384, -383, -382, -381, -380, -379, -378, -377, -376, -375, -374, -373, -372, -371, -370, -369, -368, -367, -366, -365, -364, -363, -362, -361, -360, -359, -358, -357, -356, -355, -354, -353, -352, -351, -350, -349, -348, -347, -346, -345, -344, -343, -342, -341, -340, -339, -338, -337, -336, -335, -334, -333, -332, -331, -330, -329, -328, -327, -326, -325, -324, -323, -322, -321, -320, -319, -318, -317, -316, -315, -314, -313, -312, -311, -310, -309, -308, -307, -306, -305, -304, -303, -302, -301, -300, -299, -298, -297, -296, -295, -294, -293, -292, -291, -290, -289, -288, -287, -286, -285, -284, -283, -282, -281, -280, -279, -278, -277, -276, -275, -274, -273, -272, -271, -270, -269, -268, -267, -266, -265, -264, -263, -262, -261, -260, -259, -258, -257, -256, -255, -254, -253, -252, -251, -250, -249, -248, -247, -246, -245, -244, -243, -242, -241, -240, -239, -238, -237, -236, -235, -234, -233, -232, -231, -230, -229, -228, -227, -226, -225, -224, -223, -222, -221, -220, -219, -218, -217, -216, -215, -214, -213, -212, -211, -210, -209, -208, -207, -206, -205, -204, -203, -202, -201, -200, -199, -198, -197, -196, -195, -194, -193, -192, -191, -190, -189, -188, -187, -186, -185, -184, -183, -182, -181, -180, -179, -178, -177, -176, -175, -174, -173, -172, -171, -170, -169, -168, -167, -166, -165, -164, -163, -162, -161, -160, -159, -158, -157, -156, -155, -154, -153, -152, -151, -150, -149, -148, -147, -146, -145, -144, -143, -142, -141, -140, -139, -138, -137, -136, -135, -134, -133, -132, -131, -130, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -104, -103, -102, -101, -100, -99, -98, -97, -96, -95, -94, -93, -92, -91, -90, -89, -88, -87, -86, -85, -84, -83, -82, -81, -80, -79, -78, -77, -76, -75, -74, -73, -72, -71, -70, -69, -68, -67, -66, -65, -64, -63, -62, -61, -60, -59, -58, -57, -56, -55, -54, -53, -52, -51, -50, -49, -48, -47, -46, -45, -44, -43, -42, -41, -40, -39, -38, -37, -36, -35, -34, -33, -32, -31, -30, -29, -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1 ";
    // clang-format on

    mfxI16 expected[2][32][3][2] = {
        -384, -383, -382, -381, -380, -379, -378, -377, -376, -375, -374, -373, -372, -371, -370,
        -369, -368, -367, -366, -365, -364, -363, -362, -361, -360, -359, -358, -357, -356, -355,
        -354, -353, -352, -351, -350, -349, -348, -347, -346, -345, -344, -343, -342, -341, -340,
        -339, -338, -337, -336, -335, -334, -333, -332, -331, -330, -329, -328, -327, -326, -325,
        -324, -323, -322, -321, -320, -319, -318, -317, -316, -315, -314, -313, -312, -311, -310,
        -309, -308, -307, -306, -305, -304, -303, -302, -301, -300, -299, -298, -297, -296, -295,
        -294, -293, -292, -291, -290, -289, -288, -287, -286, -285, -284, -283, -282, -281, -280,
        -279, -278, -277, -276, -275, -274, -273, -272, -271, -270, -269, -268, -267, -266, -265,
        -264, -263, -262, -261, -260, -259, -258, -257, -256, -255, -254, -253, -252, -251, -250,
        -249, -248, -247, -246, -245, -244, -243, -242, -241, -240, -239, -238, -237, -236, -235,
        -234, -233, -232, -231, -230, -229, -228, -227, -226, -225, -224, -223, -222, -221, -220,
        -219, -218, -217, -216, -215, -214, -213, -212, -211, -210, -209, -208, -207, -206, -205,
        -204, -203, -202, -201, -200, -199, -198, -197, -196, -195, -194, -193, -192, -191, -190,
        -189, -188, -187, -186, -185, -184, -183, -182, -181, -180, -179, -178, -177, -176, -175,
        -174, -173, -172, -171, -170, -169, -168, -167, -166, -165, -164, -163, -162, -161, -160,
        -159, -158, -157, -156, -155, -154, -153, -152, -151, -150, -149, -148, -147, -146, -145,
        -144, -143, -142, -141, -140, -139, -138, -137, -136, -135, -134, -133, -132, -131, -130,
        -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115,
        -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -104, -103, -102, -101, -100,
        -99,  -98,  -97,  -96,  -95,  -94,  -93,  -92,  -91,  -90,  -89,  -88,  -87,  -86,  -85,
        -84,  -83,  -82,  -81,  -80,  -79,  -78,  -77,  -76,  -75,  -74,  -73,  -72,  -71,  -70,
        -69,  -68,  -67,  -66,  -65,  -64,  -63,  -62,  -61,  -60,  -59,  -58,  -57,  -56,  -55,
        -54,  -53,  -52,  -51,  -50,  -49,  -48,  -47,  -46,  -45,  -44,  -43,  -42,  -41,  -40,
        -39,  -38,  -37,  -36,  -35,  -34,  -33,  -32,  -31,  -30,  -29,  -28,  -27,  -26,  -25,
        -24,  -23,  -22,  -21,  -20,  -19,  -18,  -17,  -16,  -15,  -14,  -13,  -12,  -11,  -10,
        -9,   -8,   -7,   -6,   -5,   -4,   -3,   -2,   -1
    };

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPredWeightTable *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 3; k++) {
                for (int l = 0; l < 2; l++) {
                    EXPECT_EQ(ext->Weights[i][j][k][l], expected[i][j][k][l]);
                }
            }
        }
    }
}

TEST_F(StringAPITest, RangeCheckMfxU8) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param                 = {};
    mfxExtBuffer dummy_buf              = {};
    mfxExtCodingOption2 extbuf          = {};
    extbuf.Header.BufferId              = MFX_EXTBUFF_CODING_OPTION2;
    extbuf.Header.BufferSz              = sizeof(extbuf);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)(&extbuf) };
    param.NumExtParam                   = static_cast<mfxU16>(extbufs.size());
    param.ExtParam                      = extbufs.data();
    mfxStatus sts                       = MFX_ERR_NONE;
    mfxU8 *key                          = (mfxU8 *)"mfxExtCodingOption2.MinQPI";
    mfxU8 *value                        = NULL;

    value = (mfxU8 *)"0";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(extbuf.MinQPI, 0);

    value = (mfxU8 *)"255";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(extbuf.MinQPI, 255);

    value = (mfxU8 *)"-1";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    value = (mfxU8 *)"256";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);
}

TEST_F(StringAPITest, RangeCheckMfxU16) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param                 = {};
    mfxExtBuffer dummy_buf              = {};
    mfxExtCodingOption2 extbuf          = {};
    extbuf.Header.BufferId              = MFX_EXTBUFF_CODING_OPTION2;
    extbuf.Header.BufferSz              = sizeof(extbuf);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)(&extbuf) };
    param.NumExtParam                   = static_cast<mfxU16>(extbufs.size());
    param.ExtParam                      = extbufs.data();
    mfxStatus sts                       = MFX_ERR_NONE;
    mfxU8 *key                          = (mfxU8 *)"mfxExtCodingOption2.SkipFrame";
    mfxU8 *value                        = NULL;

    value = (mfxU8 *)"0";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(extbuf.SkipFrame, 0);

    value = (mfxU8 *)"65535";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(extbuf.SkipFrame, 65535);

    value = (mfxU8 *)"-1";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    value = (mfxU8 *)"65536";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);
}

TEST_F(StringAPITest, RangeCheckMfxU32) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param    = {};
    mfxExtBuffer dummy_buf = {};
    mfxStatus sts          = MFX_ERR_NONE;
    mfxU8 *key             = (mfxU8 *)"AllocId";
    mfxU8 *value           = NULL;

    value = (mfxU8 *)"0";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.AllocId, 0);

    value = (mfxU8 *)"4294967295";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.AllocId, 4294967295);

    value = (mfxU8 *)"-1";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    value = (mfxU8 *)"4294967296";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);
}

TEST_F(StringAPITest, RangeCheckMfxU64) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param                 = {};
    mfxExtBuffer dummy_buf              = {};
    mfxExtHEVCParam extbuf              = {};
    extbuf.Header.BufferId              = MFX_EXTBUFF_HEVC_PARAM;
    extbuf.Header.BufferSz              = sizeof(extbuf);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)(&extbuf) };
    param.NumExtParam                   = static_cast<mfxU16>(extbufs.size());
    param.ExtParam                      = extbufs.data();
    mfxStatus sts                       = MFX_ERR_NONE;
    mfxU8 *key                          = (mfxU8 *)"mfxExtHEVCParam.GeneralConstraintFlags";
    mfxU8 *value                        = NULL;

    value = (mfxU8 *)"0";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(extbuf.GeneralConstraintFlags, 0);

    value = (mfxU8 *)"18446744073709551615";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(extbuf.GeneralConstraintFlags, 18446744073709551615ull);

    value = (mfxU8 *)"-1";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    value = (mfxU8 *)"18446744073709551616";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);
}

TEST_F(StringAPITest, RangeCheckMfxI8) {
    SKIP_IF_DISP_STUB_DISABLED();
    GTEST_SKIP() << "No known mfxI8 fields";
}

TEST_F(StringAPITest, RangeCheckMfxI16) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param                 = {};
    mfxExtBuffer dummy_buf              = {};
    mfxExtVP9Param extbuf               = {};
    extbuf.Header.BufferId              = MFX_EXTBUFF_VP9_PARAM;
    extbuf.Header.BufferSz              = sizeof(extbuf);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)(&extbuf) };
    param.NumExtParam                   = static_cast<mfxU16>(extbufs.size());
    param.ExtParam                      = extbufs.data();
    mfxStatus sts                       = MFX_ERR_NONE;
    mfxU8 *key                          = (mfxU8 *)"mfxExtVP9Param.QIndexDeltaLumaDC";
    mfxU8 *value                        = NULL;

    value = (mfxU8 *)"-32768";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(extbuf.QIndexDeltaLumaDC, -32768);

    value = (mfxU8 *)"32767";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(extbuf.QIndexDeltaLumaDC, 32767);

    value = (mfxU8 *)"-32769";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    value = (mfxU8 *)"32768";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);
}

TEST_F(StringAPITest, RangeCheckMfxI32) {
    SKIP_IF_DISP_STUB_DISABLED();
    GTEST_SKIP() << "No known mfxI32 fields";
}

TEST_F(StringAPITest, RangeCheckMfxI64) {
    SKIP_IF_DISP_STUB_DISABLED();
    GTEST_SKIP() << "No known mfxI64 fields";
}

TEST_F(StringAPITest, RangeCheckMfxL32) {
    SKIP_IF_DISP_STUB_DISABLED();
    GTEST_SKIP() << "No known mfxL32 fields";
}

TEST_F(StringAPITest, RangeCheckMfxUL32) {
    SKIP_IF_DISP_STUB_DISABLED();
    GTEST_SKIP() << "No known mfxUL32 fields";
}

TEST_F(StringAPITest, RangeCheckMfxF32) {
    SKIP_IF_DISP_STUB_DISABLED();
    GTEST_SKIP() << "No known mfxF32 fields";
}

TEST_F(StringAPITest, RangeCheckMfxF64) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param                 = {};
    mfxExtBuffer dummy_buf              = {};
    mfxExtVPPProcAmp extbuf             = {};
    extbuf.Header.BufferId              = MFX_EXTBUFF_VPP_PROCAMP;
    extbuf.Header.BufferSz              = sizeof(extbuf);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)(&extbuf) };
    param.NumExtParam                   = static_cast<mfxU16>(extbufs.size());
    param.ExtParam                      = extbufs.data();
    mfxStatus sts                       = MFX_ERR_NONE;
    mfxU8 *key                          = (mfxU8 *)"mfxExtVPPProcAmp.Brightness";
    mfxU8 *value                        = NULL;
    value                               = (mfxU8 *)"0x1.fffffffffffffp+1023";
    sts                                 = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(extbuf.Brightness, 0x1.fffffffffffffp+1023);

    value = (mfxU8 *)"-0x1.fffffffffffffp+1023";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(extbuf.Brightness, -0x1.fffffffffffffp+1023);

    value = (mfxU8 *)"0x2p+1023";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    value = (mfxU8 *)"-0x2p+1023";
    sts   = this->SetVideoParameter(key, value, &param, &dummy_buf);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);
}

// regenerate the following code with:
//
// cog -cPr libvpl/test/unit/src/dispatcher_stub_stringapi.cpp
//
// to install cog:
//
// pip install cogapp

/*[[[cog
import cog
import csv
import re
import textwrap
from functools import reduce


# Valid values generally chosen to be outside the range of the next smaller
# type. In cases where multiple type have the same range slightly different
# values are used.
VALID = {
    "mfxChar": "'a'",
    "mfxU8": "8",      # 0 to 255
    "mfxU16": "256",   # 0 to 65535
    "mfxU32": "65536", # 0 to 4294967295
    "mfxU64": "4294967294", # 0 to 18446744073709551615
    "mfxI8": "-8",     # -128 to 127
    "mfxI16": "-129",   # -32768 to 32767
    "mfxI32": "-32769",   # -2147483648 to  2147483647
    "mfxI64": "-2147483649",   # -9223372036854775808 to 9223372036854775807
    "mfxL32": "-32770",   # -2147483648 to  2147483647
    "mfxUL32": "65537",   # 0 to 4294967295
    "mfxF32": "32.1",   # 1.175494351e-38 to 3.402823466e+38
    "mfxF64": "3.0e+39",   # 2.2250738585072014e-308 to 1.7976931348623158e+308
    "int": "-32771",
}

SUFFIX = {
    "mfxChar": "",
    "mfxU8": "",
    "mfxU16": "",
    "mfxU32": "",
    "mfxU64": "ull",
    "mfxI8": "",
    "mfxI16": "",
    "mfxI32": "",
    "mfxI64": "ll",
    "mfxL32": "",
    "mfxUL32": "",
    "mfxF32": "",
    "mfxF64": "",
    "int": "",
}


def value_as_string(field_type, num_elements):
    string = ""
    if num_elements == 0:
        return VALID[field_type]
    if field_type in ["mfxChar"]:
        for x in range(num_elements):
            value = chr((ord('a')) + x % 25)
            string += value
    else:
        for x in range(num_elements):
            if x != 0:
                string += ", "
            string += str(int(VALID[field_type]) + x % 25)
    return string

def gen_key_name(field_name):
    field_name = re.sub("([\[]).*?([\]])", "\g<1>\g<2>", field_name)
    if field_name.startswith('mfxVideoParam.mfx.FrameInfo.'):
        return field_name[len('mfxVideoParam.mfx.FrameInfo.'):]
    if field_name.startswith('mfxVideoParam.mfx.'):
        return field_name[len('mfxVideoParam.mfx.'):]
    if field_name.startswith('mfxVideoParam.vpp.In.FrameInfo.'):
        return 'VPPIn' + field_name[len('mfxVideoParam.vpp.In.FrameInfo.'):]
    if field_name.startswith('mfxVideoParam.vpp.Out.FrameInfo.'):
        return 'VPPOut' + field_name[len('mfxVideoParam.vpp.Out.FrameInfo.'):]
    if field_name.startswith('mfxVideoParam.'):
        return field_name[len('mfxVideoParam.'):]
    if field_name.startswith('mfxExt'):
        return field_name
    return '!!!!!!!!!!!!!!!!'


def get_dimensions(field_name):
    return [
        int(i) for i in re.sub(".*?([\[])(.*?)([\]]).*?", "\g<2>,",
                               field_name).split(',')[:-1]
    ]


def get_array_element_count(field_name):
    return reduce(lambda x, y: x * y, get_dimensions(field_name))


def check_values(field_name, prop_name, field_type):
    text = ''
    dimensions = get_dimensions(field_name)
    if field_name.startswith('mfxExt'):
        base = 'ext->'
    else:
        base = 'param.'
    if dimensions == []:
        text = textwrap.dedent(
            f'    EXPECT_EQ({base}{prop_name}, {VALID[field_type]}{SUFFIX[field_type]});')
    else:
        index_var = 'h'
        for dimension in dimensions:
            index_var = chr(ord(index_var) + 1)
            element = prop_name.replace(f'[{dimension}]',f'[{index_var}]',1)
            if field_type == "mfxChar":
                text += (f'''for (int {index_var} = 0; {index_var} < {dimension-1}; {index_var}++) {{
                                EXPECT_EQ({base}{element}, {VALID[field_type]}{SUFFIX[field_type]} + i % 25);
                            }}
                            ''')
                element = prop_name.replace(f'[{dimension}]',f'[{dimension-1}]',1)
                text += (f'''EXPECT_EQ({base}{element}, 0);
                            ''')
            else:
                text += (f'''for (int {index_var} = 0; {index_var} < {dimension}; {index_var}++) {{
                                EXPECT_EQ({base}{element}, {VALID[field_type]}{SUFFIX[field_type]} + i % 25);
                            }}
                            ''')
    return text


def code():
    with open('api/strings.csv') as csv_file:
        reader = csv.DictReader(csv_file)
        for row in reader:
            field_type = row['type']
            field_name = row['name']
            field_category = row['category']

            # translate the field name to the String API name
            key_name = gen_key_name(field_name)

            # filter out categories of field that we don't know how to handle
            if field_category not in ['', 'array']:
                continue

            if field_category in ['nd-array', 'array']:
                dimensions = get_array_element_count(field_name)
            else:
                dimensions = 0

            # identify the name that will hold the value
            field_names = field_name.split('.')
            prop_name = '.'.join(field_names[1:])
            if field_name.startswith('mfxExt'):
                field_names = field_name.split('.')
                ext_type = field_names[0].split('[')[0]
            # Generate a C compatible  name for the test
            test_name = ''.join(ch for ch in key_name if ch.isalnum())

            # Write the test
            test = ''
            if field_name.startswith('mfxExt'):
                test = textwrap.dedent(f'''\
                    TEST_F(StringAPITest, Set{test_name}) {{
                        SKIP_IF_DISP_STUB_DISABLED();
                        mfxVideoParam param = {{}};
                        mfxExtBuffer extbuf = {{}};
                        mfxStatus sts       = MFX_ERR_NONE;

                        // clang-format off
                        mfxU8 *key   = (mfxU8 *)"{key_name}";
                        mfxU8 *value = (mfxU8 *)"{value_as_string(field_type, dimensions)}";
                        // clang-format on

                        sts = this->SetVideoParameter(key, value, &param, &extbuf);
                        ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

                        std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
                        std::vector<mfxExtBuffer *> extbufs = {{ (mfxExtBuffer *)buffer.data() }};

                        extbufs[0]->BufferId = extbuf.BufferId;
                        extbufs[0]->BufferSz = extbuf.BufferSz;
                        param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
                        param.ExtParam       = extbufs.data();

                        sts = this->SetVideoParameter(key, value, &param, &extbuf);

                        auto ext = ({ext_type} *)(param.ExtParam[0]);
                        EXPECT_EQ(sts, MFX_ERR_NONE);
                        {check_values(field_name, prop_name, field_type)}
                    }}''')
            else:
                test = textwrap.dedent(f'''\
                    TEST_F(StringAPITest, Set{test_name}) {{
                        SKIP_IF_DISP_STUB_DISABLED();
                        mfxVideoParam param = {{}};
                        mfxExtBuffer extbuf = {{}};
                        mfxStatus sts       = MFX_ERR_NONE;

                        // clang-format off
                        mfxU8 *key   = (mfxU8 *)"{key_name}";
                        mfxU8 *value = (mfxU8 *)"{value_as_string(field_type, dimensions)}";
                        // clang-format on

                        sts = this->SetVideoParameter(key, value, &param, &extbuf);
                        EXPECT_EQ(sts, MFX_ERR_NONE);
                        {check_values(field_name, prop_name, field_type)}
                    }}''')
            print("")
            print(test)
code()
]]]*/

TEST_F(StringAPITest, SetAllocId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"AllocId";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.AllocId, 65536);
}

TEST_F(StringAPITest, SetAsyncDepth) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"AsyncDepth";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.AsyncDepth, 256);
}

TEST_F(StringAPITest, SetLowPower) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"LowPower";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.LowPower, 256);
}

TEST_F(StringAPITest, SetBRCParamMultiplier) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"BRCParamMultiplier";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.BRCParamMultiplier, 256);
}

TEST_F(StringAPITest, SetChannelId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"ChannelId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.ChannelId, 256);
}

TEST_F(StringAPITest, SetBitDepthLuma) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"BitDepthLuma";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.BitDepthLuma, 256);
}

TEST_F(StringAPITest, SetBitDepthChroma) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"BitDepthChroma";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.BitDepthChroma, 256);
}

TEST_F(StringAPITest, SetShift) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"Shift";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.Shift, 256);
}

TEST_F(StringAPITest, SetFrameIdTemporalId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"FrameId.TemporalId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.FrameId.TemporalId, 256);
}

TEST_F(StringAPITest, SetFrameIdPriorityId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"FrameId.PriorityId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.FrameId.PriorityId, 256);
}

TEST_F(StringAPITest, SetFrameIdDependencyId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"FrameId.DependencyId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.FrameId.DependencyId, 256);
}

TEST_F(StringAPITest, SetFrameIdQualityId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"FrameId.QualityId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.FrameId.QualityId, 256);
}

TEST_F(StringAPITest, SetFrameIdViewId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"FrameId.ViewId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.FrameId.ViewId, 256);
}

TEST_F(StringAPITest, SetFourCC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"FourCC";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.FourCC, 65536);
}

TEST_F(StringAPITest, SetWidth) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"Width";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.Width, 256);
}

TEST_F(StringAPITest, SetHeight) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"Height";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.Height, 256);
}

TEST_F(StringAPITest, SetCropX) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"CropX";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.CropX, 256);
}

TEST_F(StringAPITest, SetCropY) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"CropY";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.CropY, 256);
}

TEST_F(StringAPITest, SetCropW) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"CropW";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.CropW, 256);
}

TEST_F(StringAPITest, SetCropH) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"CropH";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.CropH, 256);
}

TEST_F(StringAPITest, SetBufferSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"BufferSize";
    mfxU8 *value = (mfxU8 *)"4294967294";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.BufferSize, 4294967294ull);
}

TEST_F(StringAPITest, SetFrameRateExtN) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"FrameRateExtN";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.FrameRateExtN, 65536);
}

TEST_F(StringAPITest, SetFrameRateExtD) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"FrameRateExtD";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.FrameRateExtD, 65536);
}

TEST_F(StringAPITest, SetAspectRatioW) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"AspectRatioW";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.AspectRatioW, 256);
}

TEST_F(StringAPITest, SetAspectRatioH) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"AspectRatioH";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.AspectRatioH, 256);
}

TEST_F(StringAPITest, SetPicStruct) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"PicStruct";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.PicStruct, 256);
}

TEST_F(StringAPITest, SetChromaFormat) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"ChromaFormat";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FrameInfo.ChromaFormat, 256);
}

TEST_F(StringAPITest, SetCodecId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"CodecId";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.CodecId, 65536);
}

TEST_F(StringAPITest, SetCodecProfile) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"CodecProfile";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.CodecProfile, 256);
}

TEST_F(StringAPITest, SetCodecLevel) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"CodecLevel";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.CodecLevel, 256);
}

TEST_F(StringAPITest, SetNumThread) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"NumThread";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.NumThread, 256);
}

TEST_F(StringAPITest, SetTargetUsage) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"TargetUsage";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.TargetUsage, 256);
}

TEST_F(StringAPITest, SetGopPicSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"GopPicSize";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.GopPicSize, 256);
}

TEST_F(StringAPITest, SetGopRefDist) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"GopRefDist";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.GopRefDist, 256);
}

TEST_F(StringAPITest, SetGopOptFlag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"GopOptFlag";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.GopOptFlag, 256);
}

TEST_F(StringAPITest, SetIdrInterval) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"IdrInterval";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.IdrInterval, 256);
}

TEST_F(StringAPITest, SetRateControlMethod) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"RateControlMethod";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.RateControlMethod, 256);
}

TEST_F(StringAPITest, SetInitialDelayInKB) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"InitialDelayInKB";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.InitialDelayInKB, 256);
}

TEST_F(StringAPITest, SetQPI) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"QPI";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.QPI, 256);
}

TEST_F(StringAPITest, SetAccuracy) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"Accuracy";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.Accuracy, 256);
}

TEST_F(StringAPITest, SetBufferSizeInKB) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"BufferSizeInKB";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.BufferSizeInKB, 256);
}

TEST_F(StringAPITest, SetTargetKbps) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"TargetKbps";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.TargetKbps, 256);
}

TEST_F(StringAPITest, SetQPP) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"QPP";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.QPP, 256);
}

TEST_F(StringAPITest, SetICQQuality) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"ICQQuality";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.ICQQuality, 256);
}

TEST_F(StringAPITest, SetMaxKbps) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"MaxKbps";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.MaxKbps, 256);
}

TEST_F(StringAPITest, SetQPB) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"QPB";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.QPB, 256);
}

TEST_F(StringAPITest, SetConvergence) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"Convergence";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.Convergence, 256);
}

TEST_F(StringAPITest, SetNumSlice) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"NumSlice";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.NumSlice, 256);
}

TEST_F(StringAPITest, SetNumRefFrame) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"NumRefFrame";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.NumRefFrame, 256);
}

TEST_F(StringAPITest, SetEncodedOrder) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"EncodedOrder";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.EncodedOrder, 256);
}

TEST_F(StringAPITest, SetDecodedOrder) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"DecodedOrder";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.DecodedOrder, 256);
}

TEST_F(StringAPITest, SetExtendedPicStruct) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"ExtendedPicStruct";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.ExtendedPicStruct, 256);
}

TEST_F(StringAPITest, SetTimeStampCalc) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"TimeStampCalc";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.TimeStampCalc, 256);
}

TEST_F(StringAPITest, SetSliceGroupsPresent) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"SliceGroupsPresent";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.SliceGroupsPresent, 256);
}

TEST_F(StringAPITest, SetMaxDecFrameBuffering) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"MaxDecFrameBuffering";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.MaxDecFrameBuffering, 256);
}

TEST_F(StringAPITest, SetEnableReallocRequest) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"EnableReallocRequest";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.EnableReallocRequest, 256);
}

TEST_F(StringAPITest, SetFilmGrain) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"FilmGrain";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.FilmGrain, 256);
}

TEST_F(StringAPITest, SetIgnoreLevelConstrain) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"IgnoreLevelConstrain";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.IgnoreLevelConstrain, 256);
}

TEST_F(StringAPITest, SetSkipOutput) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"SkipOutput";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.SkipOutput, 256);
}

TEST_F(StringAPITest, SetJPEGChromaFormat) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"JPEGChromaFormat";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.JPEGChromaFormat, 256);
}

TEST_F(StringAPITest, SetRotation) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"Rotation";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.Rotation, 256);
}

TEST_F(StringAPITest, SetJPEGColorFormat) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"JPEGColorFormat";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.JPEGColorFormat, 256);
}

TEST_F(StringAPITest, SetInterleavedDec) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"InterleavedDec";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.InterleavedDec, 256);
}

TEST_F(StringAPITest, SetSamplingFactorH) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"SamplingFactorH[]";
    mfxU8 *value = (mfxU8 *)"8, 9, 10, 11";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 4; i++) {
        EXPECT_EQ(param.mfx.SamplingFactorH[i], 8 + i % 25);
    }
}

TEST_F(StringAPITest, SetSamplingFactorV) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"SamplingFactorV[]";
    mfxU8 *value = (mfxU8 *)"8, 9, 10, 11";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 4; i++) {
        EXPECT_EQ(param.mfx.SamplingFactorV[i], 8 + i % 25);
    }
}

TEST_F(StringAPITest, SetInterleaved) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"Interleaved";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.Interleaved, 256);
}

TEST_F(StringAPITest, SetQuality) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"Quality";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.Quality, 256);
}

TEST_F(StringAPITest, SetRestartInterval) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"RestartInterval";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.mfx.RestartInterval, 256);
}

TEST_F(StringAPITest, SetvppInChannelId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.ChannelId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.ChannelId, 256);
}

TEST_F(StringAPITest, SetvppInBitDepthLuma) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.BitDepthLuma";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.BitDepthLuma, 256);
}

TEST_F(StringAPITest, SetvppInBitDepthChroma) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.BitDepthChroma";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.BitDepthChroma, 256);
}

TEST_F(StringAPITest, SetvppInShift) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.Shift";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.Shift, 256);
}

TEST_F(StringAPITest, SetvppInFrameIdTemporalId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.FrameId.TemporalId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.FrameId.TemporalId, 256);
}

TEST_F(StringAPITest, SetvppInFrameIdPriorityId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.FrameId.PriorityId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.FrameId.PriorityId, 256);
}

TEST_F(StringAPITest, SetvppInFrameIdDependencyId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.FrameId.DependencyId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.FrameId.DependencyId, 256);
}

TEST_F(StringAPITest, SetvppInFrameIdQualityId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.FrameId.QualityId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.FrameId.QualityId, 256);
}

TEST_F(StringAPITest, SetvppInFrameIdViewId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.FrameId.ViewId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.FrameId.ViewId, 256);
}

TEST_F(StringAPITest, SetvppInFourCC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.FourCC";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.FourCC, 65536);
}

TEST_F(StringAPITest, SetvppInWidth) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.Width";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.Width, 256);
}

TEST_F(StringAPITest, SetvppInHeight) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.Height";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.Height, 256);
}

TEST_F(StringAPITest, SetvppInCropX) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.CropX";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.CropX, 256);
}

TEST_F(StringAPITest, SetvppInCropY) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.CropY";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.CropY, 256);
}

TEST_F(StringAPITest, SetvppInCropW) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.CropW";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.CropW, 256);
}

TEST_F(StringAPITest, SetvppInCropH) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.CropH";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.CropH, 256);
}

TEST_F(StringAPITest, SetvppInBufferSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.BufferSize";
    mfxU8 *value = (mfxU8 *)"4294967294";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.BufferSize, 4294967294ull);
}

TEST_F(StringAPITest, SetvppInFrameRateExtN) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.FrameRateExtN";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.FrameRateExtN, 65536);
}

TEST_F(StringAPITest, SetvppInFrameRateExtD) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.FrameRateExtD";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.FrameRateExtD, 65536);
}

TEST_F(StringAPITest, SetvppInAspectRatioW) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.AspectRatioW";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.AspectRatioW, 256);
}

TEST_F(StringAPITest, SetvppInAspectRatioH) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.AspectRatioH";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.AspectRatioH, 256);
}

TEST_F(StringAPITest, SetvppInPicStruct) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.PicStruct";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.PicStruct, 256);
}

TEST_F(StringAPITest, SetvppInChromaFormat) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.In.ChromaFormat";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.In.ChromaFormat, 256);
}

TEST_F(StringAPITest, SetvppOutChannelId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.ChannelId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.ChannelId, 256);
}

TEST_F(StringAPITest, SetvppOutBitDepthLuma) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.BitDepthLuma";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.BitDepthLuma, 256);
}

TEST_F(StringAPITest, SetvppOutBitDepthChroma) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.BitDepthChroma";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.BitDepthChroma, 256);
}

TEST_F(StringAPITest, SetvppOutShift) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.Shift";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.Shift, 256);
}

TEST_F(StringAPITest, SetvppOutFrameIdTemporalId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.FrameId.TemporalId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.FrameId.TemporalId, 256);
}

TEST_F(StringAPITest, SetvppOutFrameIdPriorityId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.FrameId.PriorityId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.FrameId.PriorityId, 256);
}

TEST_F(StringAPITest, SetvppOutFrameIdDependencyId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.FrameId.DependencyId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.FrameId.DependencyId, 256);
}

TEST_F(StringAPITest, SetvppOutFrameIdQualityId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.FrameId.QualityId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.FrameId.QualityId, 256);
}

TEST_F(StringAPITest, SetvppOutFrameIdViewId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.FrameId.ViewId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.FrameId.ViewId, 256);
}

TEST_F(StringAPITest, SetvppOutFourCC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.FourCC";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.FourCC, 65536);
}

TEST_F(StringAPITest, SetvppOutWidth) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.Width";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.Width, 256);
}

TEST_F(StringAPITest, SetvppOutHeight) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.Height";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.Height, 256);
}

TEST_F(StringAPITest, SetvppOutCropX) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.CropX";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.CropX, 256);
}

TEST_F(StringAPITest, SetvppOutCropY) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.CropY";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.CropY, 256);
}

TEST_F(StringAPITest, SetvppOutCropW) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.CropW";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.CropW, 256);
}

TEST_F(StringAPITest, SetvppOutCropH) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.CropH";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.CropH, 256);
}

TEST_F(StringAPITest, SetvppOutBufferSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.BufferSize";
    mfxU8 *value = (mfxU8 *)"4294967294";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.BufferSize, 4294967294ull);
}

TEST_F(StringAPITest, SetvppOutFrameRateExtN) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.FrameRateExtN";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.FrameRateExtN, 65536);
}

TEST_F(StringAPITest, SetvppOutFrameRateExtD) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.FrameRateExtD";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.FrameRateExtD, 65536);
}

TEST_F(StringAPITest, SetvppOutAspectRatioW) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.AspectRatioW";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.AspectRatioW, 256);
}

TEST_F(StringAPITest, SetvppOutAspectRatioH) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.AspectRatioH";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.AspectRatioH, 256);
}

TEST_F(StringAPITest, SetvppOutPicStruct) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.PicStruct";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.PicStruct, 256);
}

TEST_F(StringAPITest, SetvppOutChromaFormat) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"vpp.Out.ChromaFormat";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.vpp.Out.ChromaFormat, 256);
}

TEST_F(StringAPITest, SetProtected) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"Protected";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.Protected, 256);
}

TEST_F(StringAPITest, SetIOPattern) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"IOPattern";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.IOPattern, 256);
}

TEST_F(StringAPITest, SetNumExtParam) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"NumExtParam";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(param.NumExtParam, 256);
}

TEST_F(StringAPITest, SetmfxExtAV1BitstreamParamWriteIVFHeaders) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1BitstreamParam.WriteIVFHeaders";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1BitstreamParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->WriteIVFHeaders, 256);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamFilmGrainFlags) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.FilmGrainFlags";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->FilmGrainFlags, 256);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamGrainSeed) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.GrainSeed";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->GrainSeed, 256);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamRefIdx) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.RefIdx";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->RefIdx, 8);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamNumYPoints) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.NumYPoints";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumYPoints, 8);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamNumCbPoints) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.NumCbPoints";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumCbPoints, 8);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamNumCrPoints) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.NumCrPoints";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumCrPoints, 8);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamPointYValue) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.PointY[].Value";
    mfxU8 *value = (mfxU8 *)"8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 14; i++) {
        EXPECT_EQ(ext->PointY[i].Value, 8 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamPointYScaling) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.PointY[].Scaling";
    mfxU8 *value = (mfxU8 *)"8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 14; i++) {
        EXPECT_EQ(ext->PointY[i].Scaling, 8 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamPointCbValue) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.PointCb[].Value";
    mfxU8 *value = (mfxU8 *)"8, 9, 10, 11, 12, 13, 14, 15, 16, 17";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(ext->PointCb[i].Value, 8 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamPointCbScaling) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.PointCb[].Scaling";
    mfxU8 *value = (mfxU8 *)"8, 9, 10, 11, 12, 13, 14, 15, 16, 17";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(ext->PointCb[i].Scaling, 8 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamPointCrValue) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.PointCr[].Value";
    mfxU8 *value = (mfxU8 *)"8, 9, 10, 11, 12, 13, 14, 15, 16, 17";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(ext->PointCr[i].Value, 8 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamPointCrScaling) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.PointCr[].Scaling";
    mfxU8 *value = (mfxU8 *)"8, 9, 10, 11, 12, 13, 14, 15, 16, 17";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(ext->PointCr[i].Scaling, 8 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamGrainScalingMinus8) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.GrainScalingMinus8";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->GrainScalingMinus8, 8);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamArCoeffLag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.ArCoeffLag";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ArCoeffLag, 8);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamArCoeffsYPlus128) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.ArCoeffsYPlus128[]";
    mfxU8 *value = (mfxU8 *)"8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 24; i++) {
        EXPECT_EQ(ext->ArCoeffsYPlus128[i], 8 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamArCoeffsCbPlus128) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.ArCoeffsCbPlus128[]";
    mfxU8 *value = (mfxU8 *)"8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 25; i++) {
        EXPECT_EQ(ext->ArCoeffsCbPlus128[i], 8 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamArCoeffsCrPlus128) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.ArCoeffsCrPlus128[]";
    mfxU8 *value = (mfxU8 *)"8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 25; i++) {
        EXPECT_EQ(ext->ArCoeffsCrPlus128[i], 8 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamArCoeffShiftMinus6) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.ArCoeffShiftMinus6";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ArCoeffShiftMinus6, 8);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamGrainScaleShift) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.GrainScaleShift";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->GrainScaleShift, 8);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamCbMult) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.CbMult";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->CbMult, 8);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamCbLumaMult) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.CbLumaMult";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->CbLumaMult, 8);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamCbOffset) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.CbOffset";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->CbOffset, 256);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamCrMult) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.CrMult";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->CrMult, 8);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamCrLumaMult) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.CrLumaMult";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->CrLumaMult, 8);
}

TEST_F(StringAPITest, SetmfxExtAV1FilmGrainParamCrOffset) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1FilmGrainParam.CrOffset";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1FilmGrainParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->CrOffset, 256);
}

TEST_F(StringAPITest, SetmfxExtAV1ResolutionParamFrameWidth) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1ResolutionParam.FrameWidth";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1ResolutionParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->FrameWidth, 65536);
}

TEST_F(StringAPITest, SetmfxExtAV1ResolutionParamFrameHeight) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1ResolutionParam.FrameHeight";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1ResolutionParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->FrameHeight, 65536);
}

TEST_F(StringAPITest, SetmfxExtAV1SegmentationNumSegments) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1Segmentation.NumSegments";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1Segmentation *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumSegments, 8);
}

TEST_F(StringAPITest, SetmfxExtAV1SegmentationSegmentFeatureEnabled) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1Segmentation.Segment[].FeatureEnabled";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1Segmentation *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(ext->Segment[i].FeatureEnabled, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAV1SegmentationSegmentAltQIndex) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1Segmentation.Segment[].AltQIndex";
    mfxU8 *value = (mfxU8 *)"-129, -128, -127, -126, -125, -124, -123, -122";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1Segmentation *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(ext->Segment[i].AltQIndex, -129 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAV1SegmentationSegmentIdBlockSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1Segmentation.SegmentIdBlockSize";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1Segmentation *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->SegmentIdBlockSize, 256);
}

TEST_F(StringAPITest, SetmfxExtAV1SegmentationNumSegmentIdAlloc) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1Segmentation.NumSegmentIdAlloc";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1Segmentation *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumSegmentIdAlloc, 65536);
}

TEST_F(StringAPITest, SetmfxExtAV1TileParamNumTileRows) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1TileParam.NumTileRows";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1TileParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumTileRows, 256);
}

TEST_F(StringAPITest, SetmfxExtAV1TileParamNumTileColumns) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1TileParam.NumTileColumns";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1TileParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumTileColumns, 256);
}

TEST_F(StringAPITest, SetmfxExtAV1TileParamNumTileGroups) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAV1TileParam.NumTileGroups";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAV1TileParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumTileGroups, 256);
}

TEST_F(StringAPITest, SetmfxExtAVCEncodedFrameInfoFrameOrder) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCEncodedFrameInfo.FrameOrder";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCEncodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->FrameOrder, 65536);
}

TEST_F(StringAPITest, SetmfxExtAVCEncodedFrameInfoPicStruct) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCEncodedFrameInfo.PicStruct";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCEncodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->PicStruct, 256);
}

TEST_F(StringAPITest, SetmfxExtAVCEncodedFrameInfoLongTermIdx) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCEncodedFrameInfo.LongTermIdx";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCEncodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->LongTermIdx, 256);
}

TEST_F(StringAPITest, SetmfxExtAVCEncodedFrameInfoMAD) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCEncodedFrameInfo.MAD";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCEncodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MAD, 65536);
}

TEST_F(StringAPITest, SetmfxExtAVCEncodedFrameInfoBRCPanicMode) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCEncodedFrameInfo.BRCPanicMode";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCEncodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->BRCPanicMode, 256);
}

TEST_F(StringAPITest, SetmfxExtAVCEncodedFrameInfoQP) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCEncodedFrameInfo.QP";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCEncodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->QP, 256);
}

TEST_F(StringAPITest, SetmfxExtAVCEncodedFrameInfoSecondFieldOffset) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCEncodedFrameInfo.SecondFieldOffset";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCEncodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->SecondFieldOffset, 65536);
}

TEST_F(StringAPITest, SetmfxExtAVCEncodedFrameInfoUsedRefListL0FrameOrder) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCEncodedFrameInfo.UsedRefListL0[].FrameOrder";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCEncodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->UsedRefListL0[i].FrameOrder, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCEncodedFrameInfoUsedRefListL0PicStruct) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCEncodedFrameInfo.UsedRefListL0[].PicStruct";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 256, 257, 258, 259, 260, 261, 262";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCEncodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->UsedRefListL0[i].PicStruct, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCEncodedFrameInfoUsedRefListL0LongTermIdx) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCEncodedFrameInfo.UsedRefListL0[].LongTermIdx";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 256, 257, 258, 259, 260, 261, 262";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCEncodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->UsedRefListL0[i].LongTermIdx, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCEncodedFrameInfoUsedRefListL1FrameOrder) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCEncodedFrameInfo.UsedRefListL1[].FrameOrder";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCEncodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->UsedRefListL1[i].FrameOrder, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCEncodedFrameInfoUsedRefListL1PicStruct) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCEncodedFrameInfo.UsedRefListL1[].PicStruct";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 256, 257, 258, 259, 260, 261, 262";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCEncodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->UsedRefListL1[i].PicStruct, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCEncodedFrameInfoUsedRefListL1LongTermIdx) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCEncodedFrameInfo.UsedRefListL1[].LongTermIdx";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 256, 257, 258, 259, 260, 261, 262";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCEncodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->UsedRefListL1[i].LongTermIdx, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlNumRefIdxL0Active) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.NumRefIdxL0Active";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumRefIdxL0Active, 256);
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlNumRefIdxL1Active) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.NumRefIdxL1Active";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumRefIdxL1Active, 256);
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlPreferredRefListFrameOrder) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.PreferredRefList[].FrameOrder";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->PreferredRefList[i].FrameOrder, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlPreferredRefListPicStruct) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.PreferredRefList[].PicStruct";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 256, 257, 258, 259, 260, 261, 262";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->PreferredRefList[i].PicStruct, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlPreferredRefListViewId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.PreferredRefList[].ViewId";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 256, 257, 258, 259, 260, 261, 262";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->PreferredRefList[i].ViewId, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlPreferredRefListLongTermIdx) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.PreferredRefList[].LongTermIdx";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 256, 257, 258, 259, 260, 261, 262";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->PreferredRefList[i].LongTermIdx, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlRejectedRefListFrameOrder) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.RejectedRefList[].FrameOrder";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(ext->RejectedRefList[i].FrameOrder, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlRejectedRefListPicStruct) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.RejectedRefList[].PicStruct";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(ext->RejectedRefList[i].PicStruct, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlRejectedRefListViewId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.RejectedRefList[].ViewId";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(ext->RejectedRefList[i].ViewId, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlRejectedRefListLongTermIdx) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.RejectedRefList[].LongTermIdx";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(ext->RejectedRefList[i].LongTermIdx, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlLongTermRefListFrameOrder) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.LongTermRefList[].FrameOrder";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(ext->LongTermRefList[i].FrameOrder, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlLongTermRefListPicStruct) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.LongTermRefList[].PicStruct";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(ext->LongTermRefList[i].PicStruct, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlLongTermRefListViewId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.LongTermRefList[].ViewId";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(ext->LongTermRefList[i].ViewId, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlLongTermRefListLongTermIdx) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.LongTermRefList[].LongTermIdx";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(ext->LongTermRefList[i].LongTermIdx, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListCtrlApplyLongTermIdx) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefListCtrl.ApplyLongTermIdx";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefListCtrl *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ApplyLongTermIdx, 256);
}

TEST_F(StringAPITest, SetmfxExtAVCRefListsNumRefIdxL0Active) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefLists.NumRefIdxL0Active";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefLists *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumRefIdxL0Active, 256);
}

TEST_F(StringAPITest, SetmfxExtAVCRefListsNumRefIdxL1Active) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefLists.NumRefIdxL1Active";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefLists *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumRefIdxL1Active, 256);
}

TEST_F(StringAPITest, SetmfxExtAVCRefListsRefPicList0FrameOrder) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefLists.RefPicList0[].FrameOrder";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefLists *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->RefPicList0[i].FrameOrder, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListsRefPicList0PicStruct) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefLists.RefPicList0[].PicStruct";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 256, 257, 258, 259, 260, 261, 262";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefLists *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->RefPicList0[i].PicStruct, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListsRefPicList1FrameOrder) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefLists.RefPicList1[].FrameOrder";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefLists *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->RefPicList1[i].FrameOrder, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRefListsRefPicList1PicStruct) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRefLists.RefPicList1[].PicStruct";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 256, 257, 258, 259, 260, 261, 262";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRefLists *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 32; i++) {
        EXPECT_EQ(ext->RefPicList1[i].PicStruct, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtAVCRoundingOffsetEnableRoundingIntra) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRoundingOffset.EnableRoundingIntra";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRoundingOffset *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->EnableRoundingIntra, 256);
}

TEST_F(StringAPITest, SetmfxExtAVCRoundingOffsetRoundingOffsetIntra) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRoundingOffset.RoundingOffsetIntra";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRoundingOffset *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->RoundingOffsetIntra, 256);
}

TEST_F(StringAPITest, SetmfxExtAVCRoundingOffsetEnableRoundingInter) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRoundingOffset.EnableRoundingInter";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRoundingOffset *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->EnableRoundingInter, 256);
}

TEST_F(StringAPITest, SetmfxExtAVCRoundingOffsetRoundingOffsetInter) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAVCRoundingOffset.RoundingOffsetInter";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAVCRoundingOffset *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->RoundingOffsetInter, 256);
}

TEST_F(StringAPITest, SetmfxExtAvcTemporalLayersBaseLayerPID) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAvcTemporalLayers.BaseLayerPID";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAvcTemporalLayers *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->BaseLayerPID, 256);
}

TEST_F(StringAPITest, SetmfxExtAvcTemporalLayersLayerScale) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtAvcTemporalLayers.Layer[].Scale";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtAvcTemporalLayers *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(ext->Layer[i].Scale, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtChromaLocInfoChromaLocInfoPresentFlag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtChromaLocInfo.ChromaLocInfoPresentFlag";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtChromaLocInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ChromaLocInfoPresentFlag, 256);
}

TEST_F(StringAPITest, SetmfxExtChromaLocInfoChromaSampleLocTypeTopField) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtChromaLocInfo.ChromaSampleLocTypeTopField";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtChromaLocInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ChromaSampleLocTypeTopField, 256);
}

TEST_F(StringAPITest, SetmfxExtChromaLocInfoChromaSampleLocTypeBottomField) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtChromaLocInfo.ChromaSampleLocTypeBottomField";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtChromaLocInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ChromaSampleLocTypeBottomField, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionRateDistortionOpt) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.RateDistortionOpt";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->RateDistortionOpt, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionMECostType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.MECostType";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MECostType, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionMESearchType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.MESearchType";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MESearchType, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionMVSearchWindowx) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.MVSearchWindow.x";
    mfxU8 *value = (mfxU8 *)"-129";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MVSearchWindow.x, -129);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionMVSearchWindowy) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.MVSearchWindow.y";
    mfxU8 *value = (mfxU8 *)"-129";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MVSearchWindow.y, -129);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionEndOfSequence) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.EndOfSequence";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->EndOfSequence, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionFramePicture) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.FramePicture";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->FramePicture, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionCAVLC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.CAVLC";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->CAVLC, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionRecoveryPointSEI) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.RecoveryPointSEI";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->RecoveryPointSEI, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionViewOutput) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.ViewOutput";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ViewOutput, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionNalHrdConformance) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.NalHrdConformance";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NalHrdConformance, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionSingleSeiNalUnit) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.SingleSeiNalUnit";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->SingleSeiNalUnit, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionVuiVclHrdParameters) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.VuiVclHrdParameters";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->VuiVclHrdParameters, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionRefPicListReordering) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.RefPicListReordering";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->RefPicListReordering, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionResetRefList) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.ResetRefList";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ResetRefList, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionRefPicMarkRep) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.RefPicMarkRep";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->RefPicMarkRep, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionFieldOutput) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.FieldOutput";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->FieldOutput, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionIntraPredBlockSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.IntraPredBlockSize";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->IntraPredBlockSize, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionInterPredBlockSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.InterPredBlockSize";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->InterPredBlockSize, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionMVPrecision) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.MVPrecision";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MVPrecision, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionMaxDecFrameBuffering) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.MaxDecFrameBuffering";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MaxDecFrameBuffering, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionAUDelimiter) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.AUDelimiter";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->AUDelimiter, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionEndOfStream) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.EndOfStream";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->EndOfStream, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionPicTimingSEI) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.PicTimingSEI";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->PicTimingSEI, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionVuiNalHrdParameters) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption.VuiNalHrdParameters";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->VuiNalHrdParameters, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2IntRefType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.IntRefType";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->IntRefType, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2IntRefCycleSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.IntRefCycleSize";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->IntRefCycleSize, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2IntRefQPDelta) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.IntRefQPDelta";
    mfxU8 *value = (mfxU8 *)"-129";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->IntRefQPDelta, -129);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2MaxFrameSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.MaxFrameSize";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MaxFrameSize, 65536);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2MaxSliceSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.MaxSliceSize";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MaxSliceSize, 65536);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2BitrateLimit) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.BitrateLimit";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->BitrateLimit, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2MBBRC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.MBBRC";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MBBRC, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2ExtBRC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.ExtBRC";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ExtBRC, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2LookAheadDepth) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.LookAheadDepth";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->LookAheadDepth, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2Trellis) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.Trellis";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Trellis, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2RepeatPPS) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.RepeatPPS";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->RepeatPPS, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2BRefType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.BRefType";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->BRefType, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2AdaptiveI) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.AdaptiveI";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->AdaptiveI, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2AdaptiveB) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.AdaptiveB";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->AdaptiveB, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2LookAheadDS) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.LookAheadDS";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->LookAheadDS, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2NumMbPerSlice) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.NumMbPerSlice";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumMbPerSlice, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2SkipFrame) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.SkipFrame";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->SkipFrame, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2MinQPI) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.MinQPI";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MinQPI, 8);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2MaxQPI) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.MaxQPI";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MaxQPI, 8);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2MinQPP) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.MinQPP";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MinQPP, 8);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2MaxQPP) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.MaxQPP";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MaxQPP, 8);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2MinQPB) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.MinQPB";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MinQPB, 8);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2MaxQPB) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.MaxQPB";
    mfxU8 *value = (mfxU8 *)"8";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MaxQPB, 8);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2FixedFrameRate) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.FixedFrameRate";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->FixedFrameRate, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2DisableDeblockingIdc) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.DisableDeblockingIdc";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->DisableDeblockingIdc, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2DisableVUI) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.DisableVUI";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->DisableVUI, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2BufferingPeriodSEI) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.BufferingPeriodSEI";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->BufferingPeriodSEI, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2EnableMAD) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.EnableMAD";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->EnableMAD, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption2UseRawRef) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption2.UseRawRef";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->UseRawRef, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3NumSliceI) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.NumSliceI";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumSliceI, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3NumSliceP) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.NumSliceP";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumSliceP, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3NumSliceB) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.NumSliceB";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumSliceB, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3WinBRCMaxAvgKbps) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.WinBRCMaxAvgKbps";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->WinBRCMaxAvgKbps, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3WinBRCSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.WinBRCSize";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->WinBRCSize, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3QVBRQuality) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.QVBRQuality";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->QVBRQuality, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3EnableMBQP) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.EnableMBQP";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->EnableMBQP, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3IntRefCycleDist) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.IntRefCycleDist";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->IntRefCycleDist, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3DirectBiasAdjustment) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.DirectBiasAdjustment";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->DirectBiasAdjustment, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3GlobalMotionBiasAdjustment) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.GlobalMotionBiasAdjustment";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->GlobalMotionBiasAdjustment, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3MVCostScalingFactor) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.MVCostScalingFactor";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MVCostScalingFactor, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3MBDisableSkipMap) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.MBDisableSkipMap";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MBDisableSkipMap, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3WeightedPred) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.WeightedPred";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->WeightedPred, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3WeightedBiPred) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.WeightedBiPred";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->WeightedBiPred, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3AspectRatioInfoPresent) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.AspectRatioInfoPresent";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->AspectRatioInfoPresent, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3OverscanInfoPresent) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.OverscanInfoPresent";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->OverscanInfoPresent, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3OverscanAppropriate) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.OverscanAppropriate";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->OverscanAppropriate, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3TimingInfoPresent) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.TimingInfoPresent";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TimingInfoPresent, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3BitstreamRestriction) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.BitstreamRestriction";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->BitstreamRestriction, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3LowDelayHrd) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.LowDelayHrd";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->LowDelayHrd, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3MotionVectorsOverPicBoundaries) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.MotionVectorsOverPicBoundaries";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MotionVectorsOverPicBoundaries, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3ScenarioInfo) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.ScenarioInfo";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ScenarioInfo, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3ContentInfo) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.ContentInfo";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ContentInfo, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3PRefType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.PRefType";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->PRefType, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3FadeDetection) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.FadeDetection";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->FadeDetection, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3GPB) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.GPB";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->GPB, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3MaxFrameSizeI) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.MaxFrameSizeI";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MaxFrameSizeI, 65536);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3MaxFrameSizeP) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.MaxFrameSizeP";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MaxFrameSizeP, 65536);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3EnableQPOffset) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.EnableQPOffset";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->EnableQPOffset, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3QPOffset) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.QPOffset[]";
    mfxU8 *value = (mfxU8 *)"-129, -128, -127, -126, -125, -124, -123, -122";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(ext->QPOffset[i], -129 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtCodingOption3NumRefActiveP) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.NumRefActiveP[]";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(ext->NumRefActiveP[i], 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtCodingOption3NumRefActiveBL0) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.NumRefActiveBL0[]";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(ext->NumRefActiveBL0[i], 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtCodingOption3NumRefActiveBL1) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.NumRefActiveBL1[]";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(ext->NumRefActiveBL1[i], 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtCodingOption3TransformSkip) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.TransformSkip";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TransformSkip, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3TargetChromaFormatPlus1) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.TargetChromaFormatPlus1";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TargetChromaFormatPlus1, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3TargetBitDepthLuma) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.TargetBitDepthLuma";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TargetBitDepthLuma, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3TargetBitDepthChroma) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.TargetBitDepthChroma";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TargetBitDepthChroma, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3BRCPanicMode) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.BRCPanicMode";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->BRCPanicMode, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3LowDelayBRC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.LowDelayBRC";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->LowDelayBRC, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3EnableMBForceIntra) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.EnableMBForceIntra";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->EnableMBForceIntra, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3AdaptiveMaxFrameSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.AdaptiveMaxFrameSize";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->AdaptiveMaxFrameSize, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3RepartitionCheckEnable) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.RepartitionCheckEnable";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->RepartitionCheckEnable, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3EncodedUnitsInfo) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.EncodedUnitsInfo";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->EncodedUnitsInfo, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3EnableNalUnitType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.EnableNalUnitType";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->EnableNalUnitType, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3ExtBrcAdaptiveLTR) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.ExtBrcAdaptiveLTR";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ExtBrcAdaptiveLTR, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3AdaptiveLTR) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.AdaptiveLTR";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->AdaptiveLTR, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3AdaptiveCQM) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.AdaptiveCQM";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->AdaptiveCQM, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOption3AdaptiveRef) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOption3.AdaptiveRef";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOption3 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->AdaptiveRef, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionSPSPPSSPSBufSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOptionSPSPPS.SPSBufSize";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOptionSPSPPS *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->SPSBufSize, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionSPSPPSPPSBufSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOptionSPSPPS.PPSBufSize";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOptionSPSPPS *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->PPSBufSize, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionSPSPPSSPSId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOptionSPSPPS.SPSId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOptionSPSPPS *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->SPSId, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionSPSPPSPPSId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOptionSPSPPS.PPSId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOptionSPSPPS *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->PPSId, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionVPSVPSBufSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOptionVPS.VPSBufSize";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOptionVPS *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->VPSBufSize, 256);
}

TEST_F(StringAPITest, SetmfxExtCodingOptionVPSVPSId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtCodingOptionVPS.VPSId";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtCodingOptionVPS *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->VPSId, 256);
}

TEST_F(StringAPITest, SetmfxExtColorConversionChromaSiting) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtColorConversion.ChromaSiting";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtColorConversion *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ChromaSiting, 256);
}

TEST_F(StringAPITest, SetmfxExtContentLightLevelInfoInsertPayloadToggle) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtContentLightLevelInfo.InsertPayloadToggle";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtContentLightLevelInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->InsertPayloadToggle, 256);
}

TEST_F(StringAPITest, SetmfxExtContentLightLevelInfoMaxContentLightLevel) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtContentLightLevelInfo.MaxContentLightLevel";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtContentLightLevelInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MaxContentLightLevel, 256);
}

TEST_F(StringAPITest, SetmfxExtContentLightLevelInfoMaxPicAverageLightLevel) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtContentLightLevelInfo.MaxPicAverageLightLevel";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtContentLightLevelInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MaxPicAverageLightLevel, 256);
}

TEST_F(StringAPITest, SetmfxExtDecVideoProcessingInCropX) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecVideoProcessing.In.CropX";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecVideoProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->In.CropX, 256);
}

TEST_F(StringAPITest, SetmfxExtDecVideoProcessingInCropY) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecVideoProcessing.In.CropY";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecVideoProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->In.CropY, 256);
}

TEST_F(StringAPITest, SetmfxExtDecVideoProcessingInCropW) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecVideoProcessing.In.CropW";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecVideoProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->In.CropW, 256);
}

TEST_F(StringAPITest, SetmfxExtDecVideoProcessingInCropH) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecVideoProcessing.In.CropH";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecVideoProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->In.CropH, 256);
}

TEST_F(StringAPITest, SetmfxExtDecVideoProcessingOutFourCC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecVideoProcessing.Out.FourCC";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecVideoProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Out.FourCC, 65536);
}

TEST_F(StringAPITest, SetmfxExtDecVideoProcessingOutChromaFormat) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecVideoProcessing.Out.ChromaFormat";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecVideoProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Out.ChromaFormat, 256);
}

TEST_F(StringAPITest, SetmfxExtDecVideoProcessingOutWidth) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecVideoProcessing.Out.Width";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecVideoProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Out.Width, 256);
}

TEST_F(StringAPITest, SetmfxExtDecVideoProcessingOutHeight) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecVideoProcessing.Out.Height";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecVideoProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Out.Height, 256);
}

TEST_F(StringAPITest, SetmfxExtDecVideoProcessingOutCropX) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecVideoProcessing.Out.CropX";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecVideoProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Out.CropX, 256);
}

TEST_F(StringAPITest, SetmfxExtDecVideoProcessingOutCropY) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecVideoProcessing.Out.CropY";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecVideoProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Out.CropY, 256);
}

TEST_F(StringAPITest, SetmfxExtDecVideoProcessingOutCropW) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecVideoProcessing.Out.CropW";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecVideoProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Out.CropW, 256);
}

TEST_F(StringAPITest, SetmfxExtDecVideoProcessingOutCropH) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecVideoProcessing.Out.CropH";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecVideoProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Out.CropH, 256);
}

TEST_F(StringAPITest, SetmfxExtDecodeErrorReportErrorTypes) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecodeErrorReport.ErrorTypes";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecodeErrorReport *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ErrorTypes, 65536);
}

TEST_F(StringAPITest, SetmfxExtDecodedFrameInfoFrameType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDecodedFrameInfo.FrameType";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDecodedFrameInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->FrameType, 256);
}

TEST_F(StringAPITest, SetmfxExtDeviceAffinityMaskDeviceID) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDeviceAffinityMask.DeviceID[]";
    mfxU8 *value = (mfxU8 *)"abcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabc";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDeviceAffinityMask *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 127; i++) {
        EXPECT_EQ(ext->DeviceID[i], 'a' + i % 25);
    }
    EXPECT_EQ(ext->DeviceID[127], 0);
}

TEST_F(StringAPITest, SetmfxExtDeviceAffinityMaskNumSubDevices) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDeviceAffinityMask.NumSubDevices";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDeviceAffinityMask *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumSubDevices, 65536);
}

TEST_F(StringAPITest, SetmfxExtDirtyRectNumRect) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDirtyRect.NumRect";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDirtyRect *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumRect, 256);
}

TEST_F(StringAPITest, SetmfxExtDirtyRectRectLeft) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDirtyRect.Rect[].Left";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDirtyRect *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->Rect[i].Left, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtDirtyRectRectTop) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDirtyRect.Rect[].Top";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDirtyRect *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->Rect[i].Top, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtDirtyRectRectRight) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDirtyRect.Rect[].Right";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDirtyRect *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->Rect[i].Right, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtDirtyRectRectBottom) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtDirtyRect.Rect[].Bottom";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtDirtyRect *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->Rect[i].Bottom, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtEncodedSlicesInfoSliceSizeOverflow) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncodedSlicesInfo.SliceSizeOverflow";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncodedSlicesInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->SliceSizeOverflow, 256);
}

TEST_F(StringAPITest, SetmfxExtEncodedSlicesInfoNumSliceNonCopliant) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncodedSlicesInfo.NumSliceNonCopliant";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncodedSlicesInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumSliceNonCopliant, 256);
}

TEST_F(StringAPITest, SetmfxExtEncodedSlicesInfoNumEncodedSlice) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncodedSlicesInfo.NumEncodedSlice";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncodedSlicesInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumEncodedSlice, 256);
}

TEST_F(StringAPITest, SetmfxExtEncodedSlicesInfoNumSliceSizeAlloc) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncodedSlicesInfo.NumSliceSizeAlloc";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncodedSlicesInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumSliceSizeAlloc, 256);
}

TEST_F(StringAPITest, SetmfxExtEncodedUnitsInfoNumUnitsAlloc) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncodedUnitsInfo.NumUnitsAlloc";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncodedUnitsInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumUnitsAlloc, 256);
}

TEST_F(StringAPITest, SetmfxExtEncodedUnitsInfoNumUnitsEncoded) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncodedUnitsInfo.NumUnitsEncoded";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncodedUnitsInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumUnitsEncoded, 256);
}

TEST_F(StringAPITest, SetmfxExtEncoderCapabilityMBPerSec) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncoderCapability.MBPerSec";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncoderCapability *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MBPerSec, 65536);
}

TEST_F(StringAPITest, SetmfxExtEncoderIPCMAreaNumArea) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncoderIPCMArea.NumArea";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncoderIPCMArea *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumArea, 256);
}

TEST_F(StringAPITest, SetmfxExtEncoderROINumROI) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncoderROI.NumROI";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncoderROI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumROI, 256);
}

TEST_F(StringAPITest, SetmfxExtEncoderROIROIMode) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncoderROI.ROIMode";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncoderROI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ROIMode, 256);
}

TEST_F(StringAPITest, SetmfxExtEncoderROIROILeft) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncoderROI.ROI[].Left";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncoderROI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->ROI[i].Left, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtEncoderROIROITop) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncoderROI.ROI[].Top";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncoderROI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->ROI[i].Top, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtEncoderROIROIRight) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncoderROI.ROI[].Right";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncoderROI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->ROI[i].Right, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtEncoderROIROIBottom) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncoderROI.ROI[].Bottom";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncoderROI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->ROI[i].Bottom, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtEncoderROIROIPriority) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncoderROI.ROI[].Priority";
    mfxU8 *value = (mfxU8 *)"-129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncoderROI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->ROI[i].Priority, -129 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtEncoderROIROIDeltaQP) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncoderROI.ROI[].DeltaQP";
    mfxU8 *value = (mfxU8 *)"-129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -129, -128, -127, -126, -125, -124";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncoderROI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->ROI[i].DeltaQP, -129 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtEncoderResetOptionStartNewSequence) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtEncoderResetOption.StartNewSequence";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtEncoderResetOption *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->StartNewSequence, 256);
}

TEST_F(StringAPITest, SetmfxExtHEVCParamPicWidthInLumaSamples) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtHEVCParam.PicWidthInLumaSamples";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtHEVCParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->PicWidthInLumaSamples, 256);
}

TEST_F(StringAPITest, SetmfxExtHEVCParamPicHeightInLumaSamples) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtHEVCParam.PicHeightInLumaSamples";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtHEVCParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->PicHeightInLumaSamples, 256);
}

TEST_F(StringAPITest, SetmfxExtHEVCParamGeneralConstraintFlags) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtHEVCParam.GeneralConstraintFlags";
    mfxU8 *value = (mfxU8 *)"4294967294";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtHEVCParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->GeneralConstraintFlags, 4294967294ull);
}

TEST_F(StringAPITest, SetmfxExtHEVCParamSampleAdaptiveOffset) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtHEVCParam.SampleAdaptiveOffset";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtHEVCParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->SampleAdaptiveOffset, 256);
}

TEST_F(StringAPITest, SetmfxExtHEVCParamLCUSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtHEVCParam.LCUSize";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtHEVCParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->LCUSize, 256);
}

TEST_F(StringAPITest, SetmfxExtHEVCRegionRegionId) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtHEVCRegion.RegionId";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtHEVCRegion *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->RegionId, 65536);
}

TEST_F(StringAPITest, SetmfxExtHEVCRegionRegionType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtHEVCRegion.RegionType";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtHEVCRegion *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->RegionType, 256);
}

TEST_F(StringAPITest, SetmfxExtHEVCRegionRegionEncoding) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtHEVCRegion.RegionEncoding";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtHEVCRegion *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->RegionEncoding, 256);
}

TEST_F(StringAPITest, SetmfxExtHEVCTilesNumTileRows) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtHEVCTiles.NumTileRows";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtHEVCTiles *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumTileRows, 256);
}

TEST_F(StringAPITest, SetmfxExtHEVCTilesNumTileColumns) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtHEVCTiles.NumTileColumns";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtHEVCTiles *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumTileColumns, 256);
}

TEST_F(StringAPITest, SetmfxExtHyperModeParamMode) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtHyperModeParam.Mode";
    mfxU8 *value = (mfxU8 *)"-32771";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtHyperModeParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Mode, -32771);
}

TEST_F(StringAPITest, SetmfxExtInCropsCropsLeft) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtInCrops.Crops.Left";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtInCrops *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Crops.Left, 256);
}

TEST_F(StringAPITest, SetmfxExtInCropsCropsTop) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtInCrops.Crops.Top";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtInCrops *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Crops.Top, 256);
}

TEST_F(StringAPITest, SetmfxExtInCropsCropsRight) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtInCrops.Crops.Right";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtInCrops *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Crops.Right, 256);
}

TEST_F(StringAPITest, SetmfxExtInCropsCropsBottom) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtInCrops.Crops.Bottom";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtInCrops *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Crops.Bottom, 256);
}

TEST_F(StringAPITest, SetmfxExtInsertHeadersSPS) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtInsertHeaders.SPS";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtInsertHeaders *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->SPS, 256);
}

TEST_F(StringAPITest, SetmfxExtInsertHeadersPPS) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtInsertHeaders.PPS";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtInsertHeaders *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->PPS, 256);
}

TEST_F(StringAPITest, SetmfxExtMBDisableSkipMapMapSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMBDisableSkipMap.MapSize";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMBDisableSkipMap *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MapSize, 65536);
}

TEST_F(StringAPITest, SetmfxExtMBForceIntraMapSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMBForceIntra.MapSize";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMBForceIntra *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MapSize, 65536);
}

TEST_F(StringAPITest, SetmfxExtMBQPMode) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMBQP.Mode";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMBQP *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Mode, 256);
}

TEST_F(StringAPITest, SetmfxExtMBQPBlockSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMBQP.BlockSize";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMBQP *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->BlockSize, 256);
}

TEST_F(StringAPITest, SetmfxExtMBQPNumQPAlloc) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMBQP.NumQPAlloc";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMBQP *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumQPAlloc, 65536);
}

TEST_F(StringAPITest, SetmfxExtMVOverPicBoundariesStickTop) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMVOverPicBoundaries.StickTop";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMVOverPicBoundaries *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->StickTop, 256);
}

TEST_F(StringAPITest, SetmfxExtMVOverPicBoundariesStickBottom) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMVOverPicBoundaries.StickBottom";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMVOverPicBoundaries *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->StickBottom, 256);
}

TEST_F(StringAPITest, SetmfxExtMVOverPicBoundariesStickLeft) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMVOverPicBoundaries.StickLeft";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMVOverPicBoundaries *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->StickLeft, 256);
}

TEST_F(StringAPITest, SetmfxExtMVOverPicBoundariesStickRight) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMVOverPicBoundaries.StickRight";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMVOverPicBoundaries *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->StickRight, 256);
}

TEST_F(StringAPITest, SetmfxExtMasteringDisplayColourVolumeInsertPayloadToggle) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMasteringDisplayColourVolume.InsertPayloadToggle";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMasteringDisplayColourVolume *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->InsertPayloadToggle, 256);
}

TEST_F(StringAPITest, SetmfxExtMasteringDisplayColourVolumeDisplayPrimariesX) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMasteringDisplayColourVolume.DisplayPrimariesX[]";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMasteringDisplayColourVolume *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->DisplayPrimariesX[i], 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtMasteringDisplayColourVolumeDisplayPrimariesY) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMasteringDisplayColourVolume.DisplayPrimariesY[]";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMasteringDisplayColourVolume *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->DisplayPrimariesY[i], 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtMasteringDisplayColourVolumeWhitePointX) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMasteringDisplayColourVolume.WhitePointX";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMasteringDisplayColourVolume *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->WhitePointX, 256);
}

TEST_F(StringAPITest, SetmfxExtMasteringDisplayColourVolumeWhitePointY) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMasteringDisplayColourVolume.WhitePointY";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMasteringDisplayColourVolume *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->WhitePointY, 256);
}

TEST_F(StringAPITest, SetmfxExtMasteringDisplayColourVolumeMaxDisplayMasteringLuminance) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMasteringDisplayColourVolume.MaxDisplayMasteringLuminance";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMasteringDisplayColourVolume *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MaxDisplayMasteringLuminance, 65536);
}

TEST_F(StringAPITest, SetmfxExtMasteringDisplayColourVolumeMinDisplayMasteringLuminance) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMasteringDisplayColourVolume.MinDisplayMasteringLuminance";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMasteringDisplayColourVolume *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MinDisplayMasteringLuminance, 65536);
}

TEST_F(StringAPITest, SetmfxExtMoveRectNumRect) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMoveRect.NumRect";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMoveRect *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumRect, 256);
}

TEST_F(StringAPITest, SetmfxExtMoveRectRectDestLeft) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMoveRect.Rect[].DestLeft";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMoveRect *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->Rect[i].DestLeft, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtMoveRectRectDestTop) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMoveRect.Rect[].DestTop";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMoveRect *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->Rect[i].DestTop, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtMoveRectRectDestRight) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMoveRect.Rect[].DestRight";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMoveRect *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->Rect[i].DestRight, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtMoveRectRectDestBottom) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMoveRect.Rect[].DestBottom";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMoveRect *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->Rect[i].DestBottom, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtMoveRectRectSourceLeft) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMoveRect.Rect[].SourceLeft";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMoveRect *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->Rect[i].SourceLeft, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtMoveRectRectSourceTop) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtMoveRect.Rect[].SourceTop";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541, 65542, 65543, 65544, 65545, 65546, 65547, 65548, 65549, 65550, 65551, 65552, 65553, 65554, 65555, 65556, 65557, 65558, 65559, 65560, 65536, 65537, 65538, 65539, 65540, 65541";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtMoveRect *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ(ext->Rect[i].SourceTop, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPartialBitstreamParamBlockSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPartialBitstreamParam.BlockSize";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPartialBitstreamParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->BlockSize, 65536);
}

TEST_F(StringAPITest, SetmfxExtPartialBitstreamParamGranularity) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPartialBitstreamParam.Granularity";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPartialBitstreamParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Granularity, 256);
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampClockTimestampFlag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].ClockTimestampFlag";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].ClockTimestampFlag, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampCtType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].CtType";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].CtType, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampNuitFieldBasedFlag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].NuitFieldBasedFlag";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].NuitFieldBasedFlag, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampCountingType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].CountingType";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].CountingType, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampFullTimestampFlag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].FullTimestampFlag";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].FullTimestampFlag, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampDiscontinuityFlag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].DiscontinuityFlag";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].DiscontinuityFlag, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampCntDroppedFlag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].CntDroppedFlag";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].CntDroppedFlag, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampNFrames) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].NFrames";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].NFrames, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampSecondsFlag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].SecondsFlag";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].SecondsFlag, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampMinutesFlag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].MinutesFlag";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].MinutesFlag, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampHoursFlag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].HoursFlag";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].HoursFlag, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampSecondsValue) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].SecondsValue";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].SecondsValue, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampMinutesValue) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].MinutesValue";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].MinutesValue, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampHoursValue) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].HoursValue";
    mfxU8 *value = (mfxU8 *)"256, 257, 258";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].HoursValue, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPictureTimingSEITimeStampTimeOffset) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPictureTimingSEI.TimeStamp[].TimeOffset";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPictureTimingSEI *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->TimeStamp[i].TimeOffset, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtPredWeightTableLumaLog2WeightDenom) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPredWeightTable.LumaLog2WeightDenom";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPredWeightTable *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->LumaLog2WeightDenom, 256);
}

TEST_F(StringAPITest, SetmfxExtPredWeightTableChromaLog2WeightDenom) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtPredWeightTable.ChromaLog2WeightDenom";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtPredWeightTable *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ChromaLog2WeightDenom, 256);
}

TEST_F(StringAPITest, SetmfxExtTemporalLayersNumLayers) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtTemporalLayers.NumLayers";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtTemporalLayers *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumLayers, 256);
}

TEST_F(StringAPITest, SetmfxExtTemporalLayersBaseLayerPID) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtTemporalLayers.BaseLayerPID";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtTemporalLayers *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->BaseLayerPID, 256);
}

TEST_F(StringAPITest, SetmfxExtThreadsParamNumThread) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtThreadsParam.NumThread";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtThreadsParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumThread, 256);
}

TEST_F(StringAPITest, SetmfxExtThreadsParamSchedulingType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtThreadsParam.SchedulingType";
    mfxU8 *value = (mfxU8 *)"-32769";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtThreadsParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->SchedulingType, -32769);
}

TEST_F(StringAPITest, SetmfxExtThreadsParamPriority) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtThreadsParam.Priority";
    mfxU8 *value = (mfxU8 *)"-32769";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtThreadsParam *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Priority, -32769);
}

TEST_F(StringAPITest, SetmfxExtTimeCodeDropFrameFlag) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtTimeCode.DropFrameFlag";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtTimeCode *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->DropFrameFlag, 256);
}

TEST_F(StringAPITest, SetmfxExtTimeCodeTimeCodeHours) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtTimeCode.TimeCodeHours";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtTimeCode *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TimeCodeHours, 256);
}

TEST_F(StringAPITest, SetmfxExtTimeCodeTimeCodeMinutes) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtTimeCode.TimeCodeMinutes";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtTimeCode *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TimeCodeMinutes, 256);
}

TEST_F(StringAPITest, SetmfxExtTimeCodeTimeCodeSeconds) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtTimeCode.TimeCodeSeconds";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtTimeCode *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TimeCodeSeconds, 256);
}

TEST_F(StringAPITest, SetmfxExtTimeCodeTimeCodePictures) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtTimeCode.TimeCodePictures";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtTimeCode *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TimeCodePictures, 256);
}

TEST_F(StringAPITest, SetmfxExtVP9ParamFrameWidth) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Param.FrameWidth";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Param *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->FrameWidth, 256);
}

TEST_F(StringAPITest, SetmfxExtVP9ParamFrameHeight) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Param.FrameHeight";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Param *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->FrameHeight, 256);
}

TEST_F(StringAPITest, SetmfxExtVP9ParamWriteIVFHeaders) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Param.WriteIVFHeaders";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Param *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->WriteIVFHeaders, 256);
}

TEST_F(StringAPITest, SetmfxExtVP9ParamQIndexDeltaLumaDC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Param.QIndexDeltaLumaDC";
    mfxU8 *value = (mfxU8 *)"-129";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Param *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->QIndexDeltaLumaDC, -129);
}

TEST_F(StringAPITest, SetmfxExtVP9ParamQIndexDeltaChromaAC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Param.QIndexDeltaChromaAC";
    mfxU8 *value = (mfxU8 *)"-129";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Param *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->QIndexDeltaChromaAC, -129);
}

TEST_F(StringAPITest, SetmfxExtVP9ParamQIndexDeltaChromaDC) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Param.QIndexDeltaChromaDC";
    mfxU8 *value = (mfxU8 *)"-129";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Param *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->QIndexDeltaChromaDC, -129);
}

TEST_F(StringAPITest, SetmfxExtVP9ParamNumTileRows) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Param.NumTileRows";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Param *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumTileRows, 256);
}

TEST_F(StringAPITest, SetmfxExtVP9ParamNumTileColumns) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Param.NumTileColumns";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Param *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumTileColumns, 256);
}

TEST_F(StringAPITest, SetmfxExtVP9SegmentationNumSegments) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Segmentation.NumSegments";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Segmentation *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumSegments, 256);
}

TEST_F(StringAPITest, SetmfxExtVP9SegmentationSegmentFeatureEnabled) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Segmentation.Segment[].FeatureEnabled";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Segmentation *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(ext->Segment[i].FeatureEnabled, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtVP9SegmentationSegmentQIndexDelta) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Segmentation.Segment[].QIndexDelta";
    mfxU8 *value = (mfxU8 *)"-129, -128, -127, -126, -125, -124, -123, -122";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Segmentation *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(ext->Segment[i].QIndexDelta, -129 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtVP9SegmentationSegmentLoopFilterLevelDelta) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Segmentation.Segment[].LoopFilterLevelDelta";
    mfxU8 *value = (mfxU8 *)"-129, -128, -127, -126, -125, -124, -123, -122";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Segmentation *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(ext->Segment[i].LoopFilterLevelDelta, -129 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtVP9SegmentationSegmentReferenceFrame) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Segmentation.Segment[].ReferenceFrame";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Segmentation *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(ext->Segment[i].ReferenceFrame, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtVP9SegmentationSegmentIdBlockSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Segmentation.SegmentIdBlockSize";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Segmentation *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->SegmentIdBlockSize, 256);
}

TEST_F(StringAPITest, SetmfxExtVP9SegmentationNumSegmentIdAlloc) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9Segmentation.NumSegmentIdAlloc";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9Segmentation *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumSegmentIdAlloc, 65536);
}

TEST_F(StringAPITest, SetmfxExtVP9TemporalLayersLayerFrameRateScale) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9TemporalLayers.Layer[].FrameRateScale";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9TemporalLayers *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(ext->Layer[i].FrameRateScale, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtVP9TemporalLayersLayerTargetKbps) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVP9TemporalLayers.Layer[].TargetKbps";
    mfxU8 *value = (mfxU8 *)"256, 257, 258, 259, 260, 261, 262, 263";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVP9TemporalLayers *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(ext->Layer[i].TargetKbps, 256 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtVPP3DLutChannelMapping) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPP3DLut.ChannelMapping";
    mfxU8 *value = (mfxU8 *)"-32771";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPP3DLut *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ChannelMapping, -32771);
}

TEST_F(StringAPITest, SetmfxExtVPP3DLutBufferType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPP3DLut.BufferType";
    mfxU8 *value = (mfxU8 *)"-32771";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPP3DLut *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->BufferType, -32771);
}

TEST_F(StringAPITest, SetmfxExtVPP3DLutSystemBufferChannelDataType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPP3DLut.SystemBuffer.Channel[].DataType";
    mfxU8 *value = (mfxU8 *)"-32771, -32770, -32769";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPP3DLut *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->SystemBuffer.Channel[i].DataType, -32771 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtVPP3DLutSystemBufferChannelSize) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPP3DLut.SystemBuffer.Channel[].Size";
    mfxU8 *value = (mfxU8 *)"65536, 65537, 65538";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPP3DLut *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(ext->SystemBuffer.Channel[i].Size, 65536 + i % 25);
    }
}

TEST_F(StringAPITest, SetmfxExtVPP3DLutVideoBufferDataType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPP3DLut.VideoBuffer.DataType";
    mfxU8 *value = (mfxU8 *)"-32771";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPP3DLut *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->VideoBuffer.DataType, -32771);
}

TEST_F(StringAPITest, SetmfxExtVPP3DLutVideoBufferMemLayout) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPP3DLut.VideoBuffer.MemLayout";
    mfxU8 *value = (mfxU8 *)"-32771";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPP3DLut *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->VideoBuffer.MemLayout, -32771);
}

TEST_F(StringAPITest, SetmfxExtVPPColorFillEnable) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPColorFill.Enable";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPColorFill *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Enable, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPCompositeY) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPComposite.Y";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPComposite *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Y, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPCompositeR) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPComposite.R";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPComposite *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->R, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPCompositeU) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPComposite.U";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPComposite *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->U, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPCompositeG) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPComposite.G";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPComposite *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->G, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPCompositeV) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPComposite.V";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPComposite *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->V, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPCompositeB) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPComposite.B";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPComposite *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->B, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPCompositeNumTiles) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPComposite.NumTiles";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPComposite *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumTiles, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPCompositeNumInputStream) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPComposite.NumInputStream";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPComposite *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumInputStream, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPDeinterlacingMode) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPDeinterlacing.Mode";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPDeinterlacing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Mode, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPDeinterlacingTelecinePattern) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPDeinterlacing.TelecinePattern";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPDeinterlacing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TelecinePattern, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPDeinterlacingTelecineLocation) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPDeinterlacing.TelecineLocation";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPDeinterlacing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TelecineLocation, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPDenoiseDenoiseFactor) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPDenoise.DenoiseFactor";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPDenoise *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->DenoiseFactor, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPDenoise2Mode) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPDenoise2.Mode";
    mfxU8 *value = (mfxU8 *)"-32771";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPDenoise2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Mode, -32771);
}

TEST_F(StringAPITest, SetmfxExtVPPDenoise2Strength) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPDenoise2.Strength";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPDenoise2 *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Strength, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPDetailDetailFactor) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPDetail.DetailFactor";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPDetail *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->DetailFactor, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPDoNotUseNumAlg) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPDoNotUse.NumAlg";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPDoNotUse *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumAlg, 65536);
}

TEST_F(StringAPITest, SetmfxExtVPPDoUseNumAlg) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPDoUse.NumAlg";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPDoUse *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NumAlg, 65536);
}

TEST_F(StringAPITest, SetmfxExtVPPFieldProcessingMode) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPFieldProcessing.Mode";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPFieldProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Mode, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPFieldProcessingInField) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPFieldProcessing.InField";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPFieldProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->InField, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPFieldProcessingOutField) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPFieldProcessing.OutField";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPFieldProcessing *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->OutField, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPFrameRateConversionAlgorithm) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPFrameRateConversion.Algorithm";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPFrameRateConversion *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Algorithm, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPImageStabMode) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPImageStab.Mode";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPImageStab *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Mode, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPMirroringType) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPMirroring.Type";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPMirroring *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Type, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPProcAmpBrightness) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPProcAmp.Brightness";
    mfxU8 *value = (mfxU8 *)"3.0e+39";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPProcAmp *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Brightness, 3.0e+39);
}

TEST_F(StringAPITest, SetmfxExtVPPProcAmpContrast) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPProcAmp.Contrast";
    mfxU8 *value = (mfxU8 *)"3.0e+39";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPProcAmp *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Contrast, 3.0e+39);
}

TEST_F(StringAPITest, SetmfxExtVPPProcAmpSaturation) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPProcAmp.Saturation";
    mfxU8 *value = (mfxU8 *)"3.0e+39";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPProcAmp *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Saturation, 3.0e+39);
}

TEST_F(StringAPITest, SetmfxExtVPPRotationAngle) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPRotation.Angle";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPRotation *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Angle, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPScalingScalingMode) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPScaling.ScalingMode";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPScaling *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ScalingMode, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPScalingInterpolationMethod) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPScaling.InterpolationMethod";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPScaling *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->InterpolationMethod, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPVideoSignalInfoInTransferMatrix) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPVideoSignalInfo.In.TransferMatrix";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPVideoSignalInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->In.TransferMatrix, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPVideoSignalInfoInNominalRange) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPVideoSignalInfo.In.NominalRange";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPVideoSignalInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->In.NominalRange, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPVideoSignalInfoOutTransferMatrix) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPVideoSignalInfo.Out.TransferMatrix";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPVideoSignalInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Out.TransferMatrix, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPVideoSignalInfoOutNominalRange) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPVideoSignalInfo.Out.NominalRange";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPVideoSignalInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->Out.NominalRange, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPVideoSignalInfoTransferMatrix) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPVideoSignalInfo.TransferMatrix";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPVideoSignalInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TransferMatrix, 256);
}

TEST_F(StringAPITest, SetmfxExtVPPVideoSignalInfoNominalRange) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVPPVideoSignalInfo.NominalRange";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVPPVideoSignalInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->NominalRange, 256);
}

TEST_F(StringAPITest, SetmfxExtVideoSignalInfoVideoFormat) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVideoSignalInfo.VideoFormat";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVideoSignalInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->VideoFormat, 256);
}

TEST_F(StringAPITest, SetmfxExtVideoSignalInfoVideoFullRange) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVideoSignalInfo.VideoFullRange";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVideoSignalInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->VideoFullRange, 256);
}

TEST_F(StringAPITest, SetmfxExtVideoSignalInfoColourDescriptionPresent) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVideoSignalInfo.ColourDescriptionPresent";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVideoSignalInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ColourDescriptionPresent, 256);
}

TEST_F(StringAPITest, SetmfxExtVideoSignalInfoColourPrimaries) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVideoSignalInfo.ColourPrimaries";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVideoSignalInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->ColourPrimaries, 256);
}

TEST_F(StringAPITest, SetmfxExtVideoSignalInfoTransferCharacteristics) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVideoSignalInfo.TransferCharacteristics";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVideoSignalInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TransferCharacteristics, 256);
}

TEST_F(StringAPITest, SetmfxExtVideoSignalInfoMatrixCoefficients) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVideoSignalInfo.MatrixCoefficients";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVideoSignalInfo *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->MatrixCoefficients, 256);
}

TEST_F(StringAPITest, SetmfxExtVppAuxDataSpatialComplexity) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVppAuxData.SpatialComplexity";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVppAuxData *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->SpatialComplexity, 65536);
}

TEST_F(StringAPITest, SetmfxExtVppAuxDataTemporalComplexity) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVppAuxData.TemporalComplexity";
    mfxU8 *value = (mfxU8 *)"65536";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVppAuxData *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->TemporalComplexity, 65536);
}

TEST_F(StringAPITest, SetmfxExtVppAuxDataPicStruct) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVppAuxData.PicStruct";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVppAuxData *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->PicStruct, 256);
}

TEST_F(StringAPITest, SetmfxExtVppAuxDataSceneChangeRate) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVppAuxData.SceneChangeRate";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVppAuxData *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->SceneChangeRate, 256);
}

TEST_F(StringAPITest, SetmfxExtVppAuxDataRepeatedFrame) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVppAuxData.RepeatedFrame";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVppAuxData *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->RepeatedFrame, 256);
}

TEST_F(StringAPITest, SetmfxExtVppMctfFilterStrength) {
    SKIP_IF_DISP_STUB_DISABLED();
    mfxVideoParam param = {};
    mfxExtBuffer extbuf = {};
    mfxStatus sts       = MFX_ERR_NONE;

    // clang-format off
    mfxU8 *key   = (mfxU8 *)"mfxExtVppMctf.FilterStrength";
    mfxU8 *value = (mfxU8 *)"256";
    // clang-format on

    sts = this->SetVideoParameter(key, value, &param, &extbuf);
    ASSERT_EQ(sts, MFX_ERR_MORE_EXTBUFFER);

    std::vector<uint8_t> buffer(extbuf.BufferSz, 0);
    std::vector<mfxExtBuffer *> extbufs = { (mfxExtBuffer *)buffer.data() };

    extbufs[0]->BufferId = extbuf.BufferId;
    extbufs[0]->BufferSz = extbuf.BufferSz;
    param.NumExtParam    = static_cast<mfxU16>(extbufs.size());
    param.ExtParam       = extbufs.data();

    sts = this->SetVideoParameter(key, value, &param, &extbuf);

    auto ext = (mfxExtVppMctf *)(param.ExtParam[0]);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    EXPECT_EQ(ext->FilterStrength, 256);
}
//[[[end]]] (checksum: 219e98b619481df6214801c89886fa8e)
