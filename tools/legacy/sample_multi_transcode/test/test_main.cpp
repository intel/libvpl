/*############################################################################
  # Copyright (C) 2014 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <regex>
#include "gtest/gtest.h"
#include "sample_defs.h"
#include "sample_multi_transcode.h"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

struct init_result {
    mfxStatus status;
    std::string cmd;
    std::string out;
    std::string err;
    std::vector<TranscodingSample::sInputParams> parsed;
};

#define EXPECT_CONTAINS(ACTUAL, EXPECTED) EXPECT_TRUE(ACTUAL.find(EXPECTED) != ACTUAL.npos);

// TranscodingSample::CmdProcessor::ParseCmdLine expects non-const char*
init_result init(int argc, char* argv[], TranscodingSample::CmdProcessor* cmd_override = nullptr) {
    init_result result;
    std::stringstream ss;
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
    result.cmd                                  = ss.str();
    TranscodingSample::CmdProcessor* active_cmd = cmd_override;
    TranscodingSample::CmdProcessor cmd;
    if (!active_cmd) {
        active_cmd = &cmd;
    }
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    std::cout << std::endl;
    result.status = active_cmd->ParseCmdLine(argc, argv);
    if (result.status == MFX_ERR_NONE) {
        TranscodingSample::sInputParams InputParams;
        while (active_cmd->GetNextSessionParams(InputParams)) {
            result.parsed.push_back(InputParams);
        }
    }
    result.out = testing::internal::GetCapturedStdout();
    result.err = testing::internal::GetCapturedStderr();
    return result;
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

init_result init_no_extras(std::vector<std::string> opts,
                           TranscodingSample::CmdProcessor* cmd_override = nullptr) {
    std::vector<char*> args;
    for (auto& opt : opts) {
        args.push_back(&opt[0]);
    }
    return init((int)args.size(), &args[0], cmd_override);
}

init_result init_session(std::vector<std::string> opts,
                         TranscodingSample::CmdProcessor* cmd_override = nullptr) {
    opts.insert(opts.begin(), "out_file");
    opts.insert(opts.begin(), "-o::h265");
    opts.insert(opts.begin(), "in_file");
    opts.insert(opts.begin(), "-i::h264");
    opts.insert(opts.begin(), "exe_name");
    std::vector<char*> args;
    for (auto& opt : opts) {
        args.push_back(&opt[0]);
    }
    return init((int)args.size(), &args[0], cmd_override);
}

init_result init(std::vector<std::string> opts,
                 TranscodingSample::CmdProcessor* cmd_override = nullptr) {
    opts.insert(opts.begin(), "exe_name");
    std::vector<char*> args;
    for (auto& opt : opts) {
        args.push_back(&opt[0]);
    }
    return init((int)args.size(), &args[0], cmd_override);
}

TEST(Transcode_CLI, build_env) {
    std::cout << "char size: " << sizeof(char) << std::endl;
}

TEST(Transcode_CLI, Optionsx0) {
    GTEST_SKIP() << "This case is failing and raises an SEH error";
    auto result = init_no_extras({});
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
    EXPECT_CONTAINS(result.out, "Too few parameters");
    EXPECT_CONTAINS(result.out, "[options]");
}

TEST(Transcode_CLI, NullArgvx0) {
    GTEST_SKIP() << "This case is failing and raises an SEH error";
    auto result = init(0, nullptr);
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
}

TEST(Transcode_CLI, NullArgvx1) {
    auto result = init(1, nullptr);
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
}

TEST(Transcode_CLI, NullArgvx2) {
    GTEST_SKIP() << "This case is failing and raises an SEH error";
    auto result = init(2, nullptr);
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
}

TEST(Transcode_CLI, NoOptions) {
    auto result = init({});
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
    EXPECT_CONTAINS(result.out, "Too few parameters");
    EXPECT_CONTAINS(result.out, "[options]");
}

TEST(Transcode_CLI, OptionHelp) {
    auto result = init({ "-?" });
    EXPECT_EQ(result.status, MFX_WRN_OUT_OF_RANGE);
    EXPECT_CONTAINS(result.out, "Usage:");
    EXPECT_CONTAINS(result.out, "Options:");
}

TEST(Transcode_CLI, TrivialSessionOptions) {
    TranscodingSample::CmdProcessor cmd;
    auto result = init({ "-i::h264", "in_file", "-o::h265", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].TargetID, 0);
    EXPECT_EQ(result.parsed[0].CascadeScaler, false);
    EXPECT_EQ(result.parsed[0].EnableTracing, false);
    EXPECT_EQ(result.parsed[0].TraceBufferSize, 0);
    EXPECT_EQ(result.parsed[0].LatencyType, TranscodingSample::SMTTracer::LatencyType::DEFAULT);
    EXPECT_EQ(result.parsed[0].ParallelEncoding, false);
    EXPECT_EQ(result.parsed[0].bIsJoin, false);
    EXPECT_EQ(result.parsed[0].priority, MFX_PRIORITY_NORMAL);
#if defined(_WIN32) || defined(_WIN64)
    EXPECT_EQ(result.parsed[0].libType, MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D11);
#else
    EXPECT_EQ(result.parsed[0].libType, MFX_IMPL_HARDWARE_ANY);
#endif
#if defined(LINUX32) || defined(LINUX64)
    EXPECT_TRUE(result.parsed[0].strDevicePath.empty());
#endif
#if (defined(_WIN64) || defined(_WIN32))
    EXPECT_EQ(result.parsed[0].luid.LowPart, 0);
    EXPECT_EQ(result.parsed[0].luid.HighPart, 0);
#else
    EXPECT_EQ(result.parsed[0].DRMRenderNodeNum, 0);
#endif
    EXPECT_EQ(result.parsed[0].PCIDomain, 0);
    EXPECT_EQ(result.parsed[0].PCIBus, 0);
    EXPECT_EQ(result.parsed[0].PCIDevice, 0);
    EXPECT_EQ(result.parsed[0].PCIFunction, 0);
    EXPECT_EQ(result.parsed[0].PCIDeviceSetup, false);
#if (defined(_WIN64) || defined(_WIN32))
    EXPECT_EQ(result.parsed[0].isDualMode, false);
    EXPECT_EQ(result.parsed[0].hyperMode, MFX_HYPERMODE_OFF);
#endif
#if (defined(_WIN32) || defined(_WIN64))
    EXPECT_EQ(result.parsed[0].bPreferiGfx, false);
    EXPECT_EQ(result.parsed[0].bPreferdGfx, false);
#endif
    EXPECT_EQ(result.parsed[0].nIdrInterval, 0);
    EXPECT_EQ(result.parsed[0].adapterType, mfxMediaAdapterType::MFX_MEDIA_UNKNOWN);
    EXPECT_EQ(result.parsed[0].dGfxIdx, -1);
    EXPECT_EQ(result.parsed[0].adapterNum, -1);
    EXPECT_EQ(result.parsed[0].dispFullSearch, DEF_DISP_FULLSEARCH);
    EXPECT_EQ(result.parsed[0].nThreadsNum, 0);
    EXPECT_EQ(result.parsed[0].bRobustFlag, false);
    EXPECT_EQ(result.parsed[0].bSoftRobustFlag, false);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_AVC);
    EXPECT_EQ(result.parsed[0].strSrcFile, std::string("in_file"));
    EXPECT_EQ(result.parsed[0].strDstFile, std::string("out_file"));
    EXPECT_TRUE(result.parsed[0].strDumpVppCompFile.empty());
    EXPECT_TRUE(result.parsed[0].dump_file.empty());
    EXPECT_TRUE(result.parsed[0].strTCBRCFilePath.empty());
    EXPECT_EQ(result.parsed[0].nTargetUsage, 0);
    EXPECT_EQ(result.parsed[0].dDecoderFrameRateOverride, 0.0);
    EXPECT_EQ(result.parsed[0].dEncoderFrameRateOverride, 0.0);
    EXPECT_EQ(result.parsed[0].EncoderPicstructOverride, 0);
    EXPECT_EQ(result.parsed[0].dVPPOutFramerate, 0.0);
    EXPECT_EQ(result.parsed[0].nBitRate, 0);
    EXPECT_EQ(result.parsed[0].nBitRateMultiplier, 0);
    EXPECT_EQ(result.parsed[0].nQuality, 0);
    EXPECT_EQ(result.parsed[0].nDstWidth, 0);
    EXPECT_EQ(result.parsed[0].nDstHeight, 0);
    EXPECT_EQ(result.parsed[0].nEncTileRows, 0);
    EXPECT_EQ(result.parsed[0].nEncTileCols, 0);
    EXPECT_EQ(result.parsed[0].bEmbeddedDenoiser, false);
    EXPECT_EQ(result.parsed[0].EmbeddedDenoiseMode, 0);
    EXPECT_EQ(result.parsed[0].EmbeddedDenoiseLevel, -1);
    EXPECT_EQ(result.parsed[0].bEnableDeinterlacing, false);
    EXPECT_EQ(result.parsed[0].DeinterlacingMode, 0);
    EXPECT_EQ(result.parsed[0].bVppDenoiser, false);
    EXPECT_EQ(result.parsed[0].VppDenoiseLevel, -1);
    EXPECT_EQ(result.parsed[0].VppDenoiseMode, 0);
    EXPECT_EQ(result.parsed[0].DetailLevel, -1);
    EXPECT_EQ(result.parsed[0].FRCAlgorithm, 0);
    EXPECT_EQ(result.parsed[0].fieldProcessingMode, TranscodingSample::FC_NONE);
    EXPECT_EQ(result.parsed[0].ScalingMode, 0);
    EXPECT_EQ(result.parsed[0].nAsyncDepth, 0);
    EXPECT_EQ(result.parsed[0].eMode, TranscodingSample::Native);
    EXPECT_EQ(result.parsed[0].eModeExt, TranscodingSample::Native);
    EXPECT_EQ(result.parsed[0].FrameNumberPreference, 0);
    EXPECT_EQ(result.parsed[0].MaxFrameNumber, MFX_INFINITE);
    EXPECT_EQ(result.parsed[0].numSurf4Comp, 0);
    EXPECT_EQ(result.parsed[0].numTiles4Comp, 0);
    EXPECT_EQ(result.parsed[0].nSlices, 0);
    EXPECT_EQ(result.parsed[0].nMaxSliceSize, 0);
    EXPECT_EQ(result.parsed[0].WinBRCMaxAvgKbps, 0);
    EXPECT_EQ(result.parsed[0].WinBRCSize, 0);
    EXPECT_EQ(result.parsed[0].BufferSizeInKB, 0);
    EXPECT_EQ(result.parsed[0].GopPicSize, 0);
    EXPECT_EQ(result.parsed[0].GopRefDist, 0);
    EXPECT_EQ(result.parsed[0].NumRefFrame, 0);
    EXPECT_EQ(result.parsed[0].nNumRefActiveP, 0);
    EXPECT_EQ(result.parsed[0].nBRefType, 0);
    EXPECT_EQ(result.parsed[0].RepartitionCheckMode, 0);
    EXPECT_EQ(result.parsed[0].GPB, 0);
    EXPECT_EQ(result.parsed[0].nTransformSkip, 0);
    EXPECT_EQ(result.parsed[0].CodecLevel, 0);
    EXPECT_EQ(result.parsed[0].CodecProfile, 0);
    EXPECT_EQ(result.parsed[0].MaxKbps, 0);
    EXPECT_EQ(result.parsed[0].InitialDelayInKB, 0);
    EXPECT_EQ(result.parsed[0].GopOptFlag, 0);
    EXPECT_EQ(result.parsed[0].AdaptiveI, 0);
    EXPECT_EQ(result.parsed[0].AdaptiveB, 0);
    EXPECT_EQ(result.parsed[0].WeightedPred, 0);
    EXPECT_EQ(result.parsed[0].WeightedBiPred, 0);
    EXPECT_EQ(result.parsed[0].ExtBrcAdaptiveLTR, 0);
    EXPECT_EQ(result.parsed[0].bExtMBQP, false);
    EXPECT_EQ(result.parsed[0].bIsMVC, false);
    EXPECT_EQ(result.parsed[0].numViews, 0);
    EXPECT_EQ(result.parsed[0].nRotationAngle, 0);
    EXPECT_TRUE(std::string(result.parsed[0].strVPPPluginDLLPath).empty());
    EXPECT_TRUE(std::string(result.parsed[0].decoderPluginParams.strPluginPath).empty());
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(result.parsed[0].decoderPluginParams.pluginGuid.Data[i], 0);
    }
    EXPECT_EQ(result.parsed[0].decoderPluginParams.type, (MfxPluginLoadType)0);
    EXPECT_TRUE(std::string(result.parsed[0].encoderPluginParams.strPluginPath).empty());
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(result.parsed[0].encoderPluginParams.pluginGuid.Data[i], 0);
    }
    EXPECT_EQ(result.parsed[0].encoderPluginParams.type, (MfxPluginLoadType)0);
    EXPECT_EQ(result.parsed[0].nTimeout, 0);
    EXPECT_EQ(result.parsed[0].nFPS, 0);
    EXPECT_EQ(result.parsed[0].statisticsWindowSize, 0);
    EXPECT_EQ(result.parsed[0].statisticsLogFile, nullptr);
    EXPECT_EQ(result.parsed[0].bLABRC, false);
    EXPECT_EQ(result.parsed[0].nLADepth, 0);
    EXPECT_EQ(result.parsed[0].bEnableExtLA, false);
    EXPECT_EQ(result.parsed[0].bEnableBPyramid, false);
    EXPECT_EQ(result.parsed[0].nRateControlMethod, 0);
    EXPECT_EQ(result.parsed[0].nQPI, 0);
    EXPECT_EQ(result.parsed[0].nQPP, 0);
    EXPECT_EQ(result.parsed[0].nQPB, 0);
    EXPECT_EQ(result.parsed[0].bDisableQPOffset, false);
    EXPECT_EQ(result.parsed[0].bSetQPOffset, false);
    EXPECT_EQ(result.parsed[0].nMinQPI, 0);
    EXPECT_EQ(result.parsed[0].nMaxQPI, 0);
    EXPECT_EQ(result.parsed[0].nMinQPP, 0);
    EXPECT_EQ(result.parsed[0].nMaxQPP, 0);
    EXPECT_EQ(result.parsed[0].nMinQPB, 0);
    EXPECT_EQ(result.parsed[0].nMaxQPB, 0);
    EXPECT_EQ(result.parsed[0].nAvcTemp, 0);
    EXPECT_EQ(result.parsed[0].nBaseLayerPID, 0);
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(result.parsed[0].nAvcTemporalLayers[i], 0);
    }
#if defined(_WIN32) || defined(_WIN64)
    EXPECT_EQ(result.parsed[0].bTemporalLayers, 0);
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(result.parsed[0].temporalLayers[i].FrameRateScale, 0);
        EXPECT_EQ(result.parsed[0].temporalLayers[i].reserved[0], 0);
        EXPECT_EQ(result.parsed[0].temporalLayers[i].reserved[1], 0);
        EXPECT_EQ(result.parsed[0].temporalLayers[i].reserved[2], 0);
        EXPECT_EQ(result.parsed[0].temporalLayers[i].InitialDelayInKB, 0);
        EXPECT_EQ(result.parsed[0].temporalLayers[i].BufferSizeInKB, 0);
        EXPECT_EQ(result.parsed[0].temporalLayers[i].TargetKbps, 0);
        EXPECT_EQ(result.parsed[0].temporalLayers[i].MaxKbps, 0);
        for (int j = 0; j < 16; ++j) {
            EXPECT_EQ(result.parsed[0].temporalLayers[i].reserved1[j], 0);
        }
        EXPECT_EQ(result.parsed[0].temporalLayers[i].QPI, 0);
        EXPECT_EQ(result.parsed[0].temporalLayers[i].QPP, 0);
        EXPECT_EQ(result.parsed[0].temporalLayers[i].QPB, 0);
        for (int j = 0; j < 4; ++j) {
            EXPECT_EQ(result.parsed[0].temporalLayers[i].reserved2[j], 0);
        }
    }
#endif
    EXPECT_EQ(result.parsed[0].nSPSId, 0);
    EXPECT_EQ(result.parsed[0].nPPSId, 0);
    EXPECT_EQ(result.parsed[0].nPicTimingSEI, 0);
    EXPECT_EQ(result.parsed[0].nNalHrdConformance, 0);
    EXPECT_EQ(result.parsed[0].nVuiNalHrdParameters, 0);
    EXPECT_EQ(result.parsed[0].nTransferCharacteristics, 0);
    EXPECT_EQ(result.parsed[0].bOpenCL, false);
    EXPECT_EQ(result.parsed[0].reserved[0], 0);
    EXPECT_EQ(result.parsed[0].reserved[1], 0);
    EXPECT_EQ(result.parsed[0].reserved[2], 0);
    EXPECT_EQ(result.parsed[0].reserved[3], 0);
    EXPECT_EQ(result.parsed[0].nVppCompDstX, 0);
    EXPECT_EQ(result.parsed[0].nVppCompDstY, 0);
    EXPECT_EQ(result.parsed[0].nVppCompDstW, 0);
    EXPECT_EQ(result.parsed[0].nVppCompDstH, 0);
    EXPECT_EQ(result.parsed[0].nVppCompSrcW, 0);
    EXPECT_EQ(result.parsed[0].nVppCompSrcH, 0);
    EXPECT_EQ(result.parsed[0].nVppCompTileId, 0);
    EXPECT_EQ(result.parsed[0].DecoderFourCC, 0);
    EXPECT_EQ(result.parsed[0].EncoderFourCC, 0);
    EXPECT_EQ(result.parsed[0].pVppCompDstRects, nullptr);
    EXPECT_EQ(result.parsed[0].bForceSysMem, false);
    EXPECT_EQ(result.parsed[0].DecOutPattern, 0);
    EXPECT_EQ(result.parsed[0].VppOutPattern, 0);
    EXPECT_EQ(result.parsed[0].nGpuCopyMode, 0);
    EXPECT_EQ(result.parsed[0].nRenderColorForamt, 0);
    EXPECT_EQ(result.parsed[0].monitorType, 0);
    EXPECT_EQ(result.parsed[0].shouldUseGreedyFormula, false);
    EXPECT_EQ(result.parsed[0].enableQSVFF, false);
    EXPECT_EQ(result.parsed[0].bSingleTexture, false);
    EXPECT_EQ(result.parsed[0].nExtBRC, TranscodingSample::EXTBRC_DEFAULT);
    EXPECT_EQ(result.parsed[0].nAdaptiveMaxFrameSize, 0);
    EXPECT_EQ(result.parsed[0].LowDelayBRC, 0);
    EXPECT_EQ(result.parsed[0].IntRefType, 0);
    EXPECT_EQ(result.parsed[0].IntRefCycleSize, 0);
    EXPECT_EQ(result.parsed[0].IntRefQPDelta, 0);
    EXPECT_EQ(result.parsed[0].IntRefCycleDist, 0);
    EXPECT_EQ(result.parsed[0].nMaxFrameSize, 0);
    EXPECT_EQ(result.parsed[0].BitrateLimit, MFX_CODINGOPTION_OFF);
    EXPECT_EQ(result.parsed[0].numMFEFrames, 0);
    EXPECT_EQ(result.parsed[0].MFMode, MFX_MF_DEFAULT);
    EXPECT_EQ(result.parsed[0].mfeTimeout, 0);
    EXPECT_EQ(result.parsed[0].SEIMetaMDCV.InsertPayloadToggle, 0);
    EXPECT_EQ(result.parsed[0].SEIMetaMDCV.DisplayPrimariesX[0], 0);
    EXPECT_EQ(result.parsed[0].SEIMetaMDCV.DisplayPrimariesX[1], 0);
    EXPECT_EQ(result.parsed[0].SEIMetaMDCV.DisplayPrimariesX[2], 0);
    EXPECT_EQ(result.parsed[0].SEIMetaMDCV.DisplayPrimariesY[0], 0);
    EXPECT_EQ(result.parsed[0].SEIMetaMDCV.DisplayPrimariesY[1], 0);
    EXPECT_EQ(result.parsed[0].SEIMetaMDCV.DisplayPrimariesY[2], 0);
    EXPECT_EQ(result.parsed[0].SEIMetaMDCV.WhitePointX, 0);
    EXPECT_EQ(result.parsed[0].SEIMetaMDCV.WhitePointY, 0);
    EXPECT_EQ(result.parsed[0].SEIMetaMDCV.MaxDisplayMasteringLuminance, 0);
    EXPECT_EQ(result.parsed[0].SEIMetaMDCV.MinDisplayMasteringLuminance, 0);
    EXPECT_EQ(result.parsed[0].SEIMetaCLLI.InsertPayloadToggle, 0);
    EXPECT_EQ(result.parsed[0].SEIMetaCLLI.MaxContentLightLevel, 0);
    EXPECT_EQ(result.parsed[0].SEIMetaCLLI.MaxPicAverageLightLevel, 0);
    EXPECT_EQ(result.parsed[0].bEnableMDCV, false);
    EXPECT_EQ(result.parsed[0].bEnableCLLI, false);
    EXPECT_EQ(result.parsed[0].SignalInfoIn.Enabled, false);
    EXPECT_EQ(result.parsed[0].SignalInfoIn.VideoFullRange, 0);
    EXPECT_EQ(result.parsed[0].SignalInfoIn.ColourPrimaries, 0);
    EXPECT_EQ(result.parsed[0].SignalInfoOut.Enabled, false);
    EXPECT_EQ(result.parsed[0].SignalInfoOut.VideoFullRange, 0);
    EXPECT_EQ(result.parsed[0].SignalInfoOut.ColourPrimaries, 0);
    EXPECT_EQ(result.parsed[0].TargetBitDepthLuma, 0);
    EXPECT_EQ(result.parsed[0].TargetBitDepthChroma, 0);
#if defined(LIBVA_WAYLAND_SUPPORT)
    EXPECT_EQ(result.parsed[0].nRenderWinX, 0);
    EXPECT_EQ(result.parsed[0].nRenderWinY, 0);
    EXPECT_EQ(result.parsed[0].bPerfMode, 0);
#endif
#if defined(LIBVA_SUPPORT)
    EXPECT_EQ(result.parsed[0].libvaBackend, 0);
#endif
    EXPECT_EQ(result.parsed[0].m_hwdev, nullptr);
    EXPECT_EQ(result.parsed[0].PresetMode, PRESET_DEFAULT);
    EXPECT_EQ(result.parsed[0].shouldPrintPresets, false);
    EXPECT_EQ(result.parsed[0].rawInput, false);
    EXPECT_TRUE(std::string(result.parsed[0].str3DLutFile).empty());
    EXPECT_EQ(result.parsed[0].bEnable3DLut, false);
    EXPECT_EQ(result.parsed[0].nMemoryModel, 0);
    EXPECT_EQ(result.parsed[0].AllocPolicy, MFX_ALLOCATION_UNLIMITED);
    EXPECT_EQ(result.parsed[0].useAllocHints, false);
    EXPECT_EQ(result.parsed[0].preallocate, 0);
    EXPECT_EQ(result.parsed[0].forceSyncAllSession, MFX_CODINGOPTION_UNKNOWN);
    EXPECT_EQ(result.parsed[0].nIVFHeader, 0);
    EXPECT_EQ(result.parsed[0].IsSourceMSB, false);
    EXPECT_EQ(result.parsed[0].nSyncOpTimeout, MSDK_WAIT_INTERVAL);
    EXPECT_EQ(result.parsed[0].TCBRCFileMode, 0);
#ifdef ONEVPL_EXPERIMENTAL
    EXPECT_EQ(result.parsed[0].PercEncPrefilter = false, false);
#endif
    EXPECT_TRUE(result.parsed[0].DumpLogFileName.empty());
    EXPECT_TRUE(result.parsed[0].m_ROIData.empty());
    EXPECT_TRUE(result.parsed[0].m_ROIData.empty());
    EXPECT_EQ(result.parsed[0].bDecoderPostProcessing, false);
    EXPECT_EQ(result.parsed[0].bROIasQPMAP, false);
#ifdef ENABLE_MCTF
    EXPECT_EQ(result.parsed[0].mctfParam.rtParams.CurIdx, 0);
    EXPECT_TRUE(result.parsed[0].mctfParam.rtParams.RunTimeParams.empty());
    // This check is currently failing, for unknown reasons.
    // EXPECT_EQ(result.parsed[0].mctfParam.params.Header.BufferId, MFX_EXTBUFF_VPP_MCTF);
    // EXPECT_EQ(result.parsed[0].mctfParam.params.Header.BufferSz, sizeof(mfxExtVppMctf));
    // for (int i = 0; i < 27; i++) {
    //     EXPECT_EQ(result.parsed[0].mctfParam.params.reserved[i], 0);
    // }
    EXPECT_EQ(result.parsed[0].mctfParam.params.FilterStrength, 0);
    EXPECT_EQ(result.parsed[0].mctfParam.mode, TranscodingSample::VPP_FILTER_DISABLED);
#endif
    EXPECT_EQ(result.parsed[0].verSessionInit, TranscodingSample::API_2X);
}

TEST(Transcode_CLI, InputCodecSelect) {
    TranscodingSample::CmdProcessor cmd;
    auto result = init({ "-i::mpeg2", "in_file", "-o::h265", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_MPEG2);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));

    result = init({ "-i::h264", "in_file", "-o::h265", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_AVC);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));

    result = init({ "-i::h265", "in_file", "-o::h265", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));

    result = init({ "-i::vc1", "in_file", "-o::h265", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_VC1);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));

    result = init({ "-i::jpeg", "in_file", "-o::h265", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_JPEG);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));

    result = init({ "-i::vp8", "in_file", "-o::h265", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_VP8);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));

    result = init({ "-i::vp9", "in_file", "-o::h265", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_VP9);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));

    result = init({ "-i::av1", "in_file", "-o::h265", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_AV1);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));
}

TEST(Transcode_CLI, InputRawCodecSelect) {
    GTEST_SKIP() << "Test not implemented";
    // Test for -i::raw, -i::rgb4_frame, -i::nv12, -i::i420, -i::i422, -i::p010
}

TEST(Transcode_CLI, InputMVCCodecSelect) {
    GTEST_SKIP() << "Test not implemented";
    // Test for -i::mvc
}

TEST(Transcode_CLI, OutputCodecSelect) {
    TranscodingSample::CmdProcessor cmd;
    auto result = init({ "-i::h265", "in_file", "-o::mpeg2", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_MPEG2);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));

    result = init({ "-i::h265", "in_file", "-o::h264", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_AVC);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));

    result = init({ "-i::h265", "in_file", "-o::h265", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));

    result = init({ "-i::h265", "in_file", "-o::jpeg", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_JPEG);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));

    result = init({ "-i::h265", "in_file", "-o::vp9", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_VP9);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));

    result = init({ "-i::h265", "in_file", "-o::av1", "out_file" }, &cmd);
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].EncodeId, MFX_CODEC_AV1);
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_HEVC);
    EXPECT_EQ(std::string(result.parsed[0].strSrcFile), std::string("in_file"));
    EXPECT_EQ(std::string(result.parsed[0].strDstFile), std::string("out_file"));
}

TEST(Transcode_CLI, OutputMVCCodecSelect) {
    GTEST_SKIP() << "Test not implemented";
    // Test for -o::mvc
}

TEST(Transcode_CLI, OutputVC1CodecSelect) {
    GTEST_SKIP() << "Test not implemented";
    // Test for -o::vc1
}

TEST(Transcode_CLI, OutputVP8CodecSelect) {
    GTEST_SKIP() << "Test not implemented";
    // Test for -o::vp8
}

TEST(Transcode_CLI, OutputRawCodecSelect) {
    GTEST_SKIP() << "Test not implemented";
    // Test for -o::raw, -o::rgb4_frame, -o::nv12, -o::i420, -o::i422, -o::i420, -o::p010
}

TEST(Transcode_CLI, OptionITooShort) {
    auto result = init({ "-i::0" });
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
}

TEST(Transcode_CLI, OptionIUnknownFormat) {
    auto result = init({ "-i::0000" });
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
}

TEST(Transcode_CLI, OptionIMissingFile) {
    GTEST_SKIP() << "This case is failing and raises an SEH error";
    auto result = init({ "-i::h264" });
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
    EXPECT_CONTAINS(result.out, "parameters");
    EXPECT_EQ(result.parsed[0].DecodeId, MFX_CODEC_AVC);
    EXPECT_TRUE(std::string(result.parsed[0].strSrcFile).empty());
}

TEST(Transcode_CLI, OptionISpecials) {
    GTEST_SKIP() << "Test not implemented";
    // Test special format handling of -i
}

TEST(Transcode_CLI, OptionISource) {
    GTEST_SKIP() << "Test not implemented";
    // Test special handling of -i::source
}

TEST(Transcode_CLI, OptionOTooShort) {
    auto result = init({ "-o::0" });
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
}

TEST(Transcode_CLI, OptionOUnknownFormat) {
    auto result = init({ "-o::0000" });
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
}

TEST(Transcode_CLI, OptionOSpecials) {
    GTEST_SKIP() << "Test not implemented";
    // Test special format handling of -o
}

TEST(Transcode_CLI, OptionOSink) {
    GTEST_SKIP() << "Test not implemented";
    // Test special handling of -o::sink
}

TEST(Transcode_CLI, OptionISourceOSink) {
    GTEST_SKIP() << "Test not implemented";
    // Test special handling of -i::source -o::sink
}

TEST(Transcode_CLI, OptionVerInvalid) {
    auto result = init_session({ "-api_ver_init::0" });
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
}

TEST(Transcode_CLI, OptionVer1) {
    auto result = init_session({ "-api_ver_init::1x" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].verSessionInit, TranscodingSample::API_1X);
}

TEST(Transcode_CLI, OptionVer2) {
    auto result = init_session({ "-api_ver_init::2x" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].verSessionInit, TranscodingSample::API_2X);
}

TEST(Transcode_CLI, OptionROIFileNoValue) {
    auto result = init_session({ "-roi_file" });
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
}

TEST(Transcode_CLI, OptionROIFileMissingFile) {
    auto result = init_session({ "-roi_file", "roi_file" });
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
    EXPECT_CONTAINS(result.out, "Incorrect ROI file: \"roi_file\"");
}

TEST(Transcode_CLI, OptionROIFile) {
    GTEST_SKIP() << "Test not implemented";
    // Test that an ROI file is recognized
}

TEST(Transcode_CLI, OptionPerfFileNoValue) {
    GTEST_SKIP() << "Test crashes";
    auto result = init_session({ "-p" });
    EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
    EXPECT_CONTAINS(result.out, "error: no argument given for '-p' option");
}

TEST(Transcode_CLI, OptionPerfFileMultiple) {
    const char* name = "temp_perf_file.txt";
    remove(name);
    do {
        TranscodingSample::CmdProcessor cmd;
        auto result = init_session({ "-p", name, "-p", name }, &cmd);
        EXPECT_EQ(result.status, MFX_ERR_UNSUPPORTED);
        EXPECT_CONTAINS(result.out, "error: only one performance file is supported");
    } while (0);
    remove(name);
}

TEST(Transcode_CLI, OptionPerfFile) {
    const char* name = "temp_perf_file.txt";
    remove(name);
    do {
        TranscodingSample::CmdProcessor cmd;
        auto result = init_session({ "-p", name }, &cmd);
        EXPECT_EQ(result.status, MFX_ERR_NONE);
        EXPECT_EQ(cmd.GetPerformanceFile(), std::string(name));
    } while (0);
    remove(name);
}

TEST(Transcode_CLI, OptionMissingPerfFile) {
    GTEST_SKIP() << "Test not implemented";
    // Test that a missing(?) Perf file reports an error
    // This case is called out in the codebase.
}

TEST(Transcode_CLI, OptionROIQPMAP) {
    auto result = init_session({ "-roi_qpmap" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].bROIasQPMAP, true);
}

TEST(Transcode_CLI, OptionExtMBQP) {
    auto result = init_session({ "-extmbqp" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].bExtMBQP, true);
}

TEST(Transcode_CLI, OptionSW) {
    auto result = init_session({ "-sw" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].libType, MFX_IMPL_SOFTWARE);
}

TEST(Transcode_CLI, OptionHW) {
    auto result = init_session({ "-hw" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
#if defined(_WIN32) || defined(_WIN64)
    EXPECT_EQ(result.parsed[0].libType, MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D11);
#else
    EXPECT_EQ(result.parsed[0].libType, MFX_IMPL_HARDWARE_ANY);
#endif
}

TEST(Transcode_CLI, OptionHWD3D11) {
    auto result = init_session({ "-hw_d3d11" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].libType, MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D11);
}

TEST(Transcode_CLI, OptionHWD3D9) {
    auto result = init_session({ "-hw_d3d9" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].libType, MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D9);
}

#if (defined(LINUX32) || defined(LINUX64))

TEST(Transcode_CLI, OptionDevice0) {
    auto result = init_session({ "-device", "foo/bar/renderD0" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].strDevicePath, std::string("foo/bar/renderD0"));
    EXPECT_EQ(result.parsed[0].DRMRenderNodeNum, 0);
}

TEST(Transcode_CLI, OptionDevice128) {
    auto result = init_session({ "-device", "foo/bar/renderD128" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].strDevicePath, std::string("foo/bar/renderD128"));
    EXPECT_EQ(result.parsed[0].DRMRenderNodeNum, 128);
}

TEST(Transcode_CLI, OptionDevice1024) {
    GTEST_SKIP() << "Test failing in existing codebase";
    auto result = init_session({ "-device", "foo/bar/renderD1024" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].strDevicePath, std::string("foo/bar/renderD1024"));
    EXPECT_EQ(result.parsed[0].DRMRenderNodeNum, 128);
}

#endif

TEST(Transcode_CLI, OptionRobust) {
    auto result = init_session({ "-robust" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].bRobustFlag, true);
}

TEST(Transcode_CLI, OptionRobustSoft) {
    auto result = init_session({ "-robust:soft" });
    EXPECT_EQ(result.status, MFX_ERR_NONE);
    EXPECT_EQ(result.parsed[0].bSoftRobustFlag, true);
}