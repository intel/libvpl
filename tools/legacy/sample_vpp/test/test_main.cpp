/*############################################################################
  # Copyright (C) 2014 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <regex>
#include "gtest/gtest.h"
#include "sample_vpp_utils.h"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

struct init_result {
    mfxStatus status;
    std::string cmd;
    std::string out;
    std::string err;
    sInputParams pParams;
    sFiltersParam pDefaultFiltersParam;
};

#define EXPECT_CONTAINS(ACTUAL, EXPECTED) EXPECT_TRUE(ACTUAL.find(EXPECTED) != ACTUAL.npos);
static void testDefaultInitParams(sInputParams* pParams, sFiltersParam* pDefaultFiltersParam) {
    pParams->frameInfoIn.clear();
    pParams->frameInfoIn.push_back(*pDefaultFiltersParam->pOwnFrameInfo);
    pParams->frameInfoOut.clear();
    pParams->frameInfoOut.push_back(*pDefaultFiltersParam->pOwnFrameInfo);

    pParams->IOPattern   = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    pParams->ImpLib      = MFX_IMPL_HARDWARE;
    pParams->asyncNum    = 1;
    pParams->bPerf       = false;
    pParams->isOutput    = false;
    pParams->ptsCheck    = false;
    pParams->ptsAdvanced = false;
    pParams->ptsFR       = 0;
    pParams->vaType      = ALLOC_IMPL_VIA_SYS;
    pParams->rotate.clear();
    pParams->rotate.push_back(0);
    pParams->bScaling            = false;
    pParams->scalingMode         = MFX_SCALING_MODE_DEFAULT;
    pParams->interpolationMethod = MFX_INTERPOLATION_DEFAULT;
    pParams->bChromaSiting       = false;
    pParams->uChromaSiting       = 0;
    pParams->numFrames           = 0;
    pParams->fccSource           = MFX_FOURCC_NV12;

    // Optional video processing features
    pParams->mirroringParam.clear();
    pParams->mirroringParam.push_back(*pDefaultFiltersParam->pMirroringParam);
    pParams->videoSignalInfoParam.clear();
    pParams->videoSignalInfoParam.push_back(*pDefaultFiltersParam->pVideoSignalInfo);
    pParams->deinterlaceParam.clear();
    pParams->deinterlaceParam.push_back(*pDefaultFiltersParam->pDIParam);
    pParams->denoiseParam.clear();
    pParams->denoiseParam.push_back(*pDefaultFiltersParam->pDenoiseParam);
#ifdef ENABLE_MCTF
    pParams->mctfParam.clear();
    pParams->mctfParam.push_back(*pDefaultFiltersParam->pMctfParam);
#endif
    pParams->detailParam.clear();
    pParams->detailParam.push_back(*pDefaultFiltersParam->pDetailParam);
    pParams->procampParam.clear();
    pParams->procampParam.push_back(*pDefaultFiltersParam->pProcAmpParam);
    // analytics
    pParams->frcParam.clear();
    pParams->frcParam.push_back(*pDefaultFiltersParam->pFRCParam);
    // MSDK 3.0
    pParams->multiViewParam.clear();
    pParams->multiViewParam.push_back(*pDefaultFiltersParam->pMultiViewParam);
    // MSDK API 1.5
    pParams->gamutParam.clear();
    pParams->gamutParam.push_back(*pDefaultFiltersParam->pGamutParam);
    pParams->tccParam.clear();
    pParams->tccParam.push_back(*pDefaultFiltersParam->pClrSaturationParam);
    pParams->aceParam.clear();
    pParams->aceParam.push_back(*pDefaultFiltersParam->pContrastParam);
    pParams->steParam.clear();
    pParams->steParam.push_back(*pDefaultFiltersParam->pSkinParam);
    pParams->istabParam.clear();
    pParams->istabParam.push_back(*pDefaultFiltersParam->pImgStabParam);

    pParams->colorfillParam.clear();
    pParams->colorfillParam.push_back(*pDefaultFiltersParam->pColorfillParam);

    // ROI check
    pParams->roiCheckParam.mode    = ROI_FIX_TO_FIX; // ROI check is disabled
    pParams->roiCheckParam.srcSeed = 0;
    pParams->roiCheckParam.dstSeed = 0;
    pParams->forcedOutputFourcc    = 0;

    // Do not call MFXVideoVPP_Reset
    pParams->resetFrmNums.clear();

    pParams->GPUCopyValue = MFX_GPUCOPY_DEFAULT;

    pParams->b3dLut = false;

    return;

} // void testDefaultInitParams( sInputParams* pParams )

// TranscodingSample::CmdProcessor::ParseCmdLine expects non-const char*
init_result init(int argc, char* argv[]) {
    init_result result;
    std::stringstream ss;

    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    std::cout << std::endl;
    /* default parameters */
    sOwnFrameInfo defaultOwnFrameInfo = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, MFX_FOURCC_NV12, MFX_PICSTRUCT_PROGRESSIVE, 30.0
    };
    sDIParam defaultDIParam           = { 0, 0, 0, VPP_FILTER_DISABLED };
    sProcAmpParam defaultProcAmpParam = { 0.0, 1.0, 1.0, 0.0, VPP_FILTER_DISABLED };
    sDetailParam defaultDetailParam   = { 1, VPP_FILTER_DISABLED };
    sDenoiseParam defaultDenoiseParam = { 1, VPP_FILTER_DISABLED };
#ifdef ENABLE_MCTF
    sMCTFParam defaultMctfParam;
    defaultMctfParam.mode                  = VPP_FILTER_DISABLED;
    defaultMctfParam.params.FilterStrength = 0;
#endif
    sVideoAnalysisParam defaultVAParam        = { VPP_FILTER_DISABLED };
    sIDetectParam defaultIDetectParam         = { VPP_FILTER_DISABLED };
    sFrameRateConversionParam defaultFRCParam = { MFX_FRCALGM_PRESERVE_TIMESTAMP,
                                                  VPP_FILTER_DISABLED };
    //MSDK 3.0
    sMultiViewParam defaultMultiViewParam = { 1, VPP_FILTER_DISABLED };
    //MSDK API 1.5
    sGamutMappingParam defaultGamutParam = { false, VPP_FILTER_DISABLED };
    sTccParam defaultClrSaturationParam  = { 160, 160, 160, 160, VPP_FILTER_DISABLED };
    sAceParam defaultContrastParam       = { VPP_FILTER_DISABLED };
    sSteParam defaultSkinParam           = { 4, VPP_FILTER_DISABLED };
    sIStabParam defaultImgStabParam      = { MFX_IMAGESTAB_MODE_BOXING, VPP_FILTER_DISABLED };
    sSVCParam defaultSVCParam            = { {}, VPP_FILTER_DISABLED };
    sVideoSignalInfoParam defaultVideoSignalInfoParam;
    sMirroringParam defaultMirroringParam;
    sColorFillParam defaultColorfillParam;
    result.pDefaultFiltersParam = { &defaultOwnFrameInfo,
                                    &defaultDIParam,
                                    &defaultProcAmpParam,
                                    &defaultDetailParam,
                                    &defaultDenoiseParam,
#ifdef ENABLE_MCTF
                                    &defaultMctfParam,
#endif
                                    &defaultVAParam,
                                    &defaultIDetectParam,
                                    &defaultFRCParam,
                                    &defaultMultiViewParam,
                                    &defaultGamutParam,
                                    &defaultClrSaturationParam,
                                    &defaultContrastParam,
                                    &defaultSkinParam,
                                    &defaultImgStabParam,
                                    &defaultSVCParam,
                                    &defaultVideoSignalInfoParam,
                                    &defaultMirroringParam,
                                    &defaultColorfillParam };

    testDefaultInitParams(&result.pParams, &result.pDefaultFiltersParam);
    result.status =
        vppParseInputString(argv, (mfxU32)argc, &result.pParams, &result.pDefaultFiltersParam);
    for (int i = 0; i < argc; i++) {
        if (!argv) {
            ss << "<null args>"
               << " ";
        }
        else if (!argv[i]) {
            ss << "<null arg>"
               << " ";
        }
        else {
            std::string arg = argv[i];
            arg             = std::regex_replace(arg, std::regex(R"(\\)"), R"(\\)");
            arg             = std::regex_replace(arg, std::regex("\""), "\\\"");
            arg             = std::regex_replace(arg, std::regex(" "), "\\ ");
            ss << arg << " ";
        }
    }
    result.cmd = ss.str();
    result.out = testing::internal::GetCapturedStdout();
    result.err = testing::internal::GetCapturedStderr();
    return result;
}

init_result init_no_extras(std::vector<std::string> opts) {
    std::vector<char*> args;
    for (auto& opt : opts) {
        args.push_back(&opt[0]);
    }
    return init((int)args.size(), &args[0]);
}

init_result init(std::vector<std::string> opts) {
    opts.insert(opts.begin(), "exe_name");
    std::vector<char*> args;
    for (auto& opt : opts) {
        args.push_back(&opt[0]);
    }
    return init((int)args.size(), &args[0]);
}

void print_result(const init_result& result) {
    std::cout << result.status << " <= " << result.cmd << std::endl;
    std::cout << "====================" << std::endl;
    if (!result.out.empty()) {
        std::cout << result.out << std::endl;
        std::cout << "====================" << std::endl;
    }
    if (!result.err.empty()) {
        std::cout << result.err << std::endl;
        std::cout << "====================" << std::endl;
    }
}

TEST(VPP_CLI, build_env) {
    std::cout << "char size: " << sizeof(char) << std::endl;
}

TEST(VPP_CLI, Optionsx0) {
    GTEST_SKIP() << "This case is failing and raises an SEH error in debug mode on windows";
    auto result = init_no_extras({});
    EXPECT_EQ(result.status, MFX_ERR_NULL_PTR);
}

TEST(VPP_CLI, NullArgvx0) {
    auto result = init(0, nullptr);
    EXPECT_EQ(result.status, MFX_ERR_NULL_PTR);
}

TEST(VPP_CLI, NullArgvx1) {
    auto result = init(1, nullptr);
    EXPECT_EQ(result.status, MFX_ERR_NULL_PTR);
}

TEST(VPP_CLI, NullArgvx2) {
    auto result = init(2, nullptr);
    EXPECT_EQ(result.status, MFX_ERR_NULL_PTR);
}

TEST(VPP_CLI, NoOptions) {
    auto result = init({});
    EXPECT_EQ(result.status, MFX_ERR_MORE_DATA);
    EXPECT_CONTAINS(result.out, "Not enough parameters");
}

TEST(VPP_CLI, OptionHelp) {
    auto result = init({ "-?" });
    EXPECT_EQ(result.status, MFX_ERR_MORE_DATA);
    EXPECT_CONTAINS(result.out, "Usage:");
    EXPECT_CONTAINS(result.out, "Options:");
}

TEST(VPP_CLI, Escape1) {
    auto result =
        init({ "-i",      "build\\win_x64\\output\\sample_vpp_detail\\0001.blur.nv12",
               "-o",      "build\\win_x64\\output\\sample_vpp_detail\\0001.blur.nv12.test",
               "-lib",    "hw",
               "-sw",     "176",
               "-sh",     "144",
               "-dw",     "176",
               "-dh",     "144",
               "-scc",    "nv12",
               "-dcc",    "nv12",
               "-spic",   "1",
               "-dpic",   "1",
               "-detail", "2",
               "-n",      "100" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
}
