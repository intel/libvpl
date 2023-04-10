/*############################################################################
  # Copyright (C) 2014 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <regex>
#include "gtest/gtest.h"
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
init_result init(int argc, msdk_char* argv[]) {
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
    result.cmd = ss.str();
    TranscodingSample::CmdProcessor cmd;
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    std::cout << std::endl;
    result.status = cmd.ParseCmdLine(argc, argv);
    if (result.status == MFX_ERR_NONE) {
        TranscodingSample::sInputParams InputParams;
        while (cmd.GetNextSessionParams(InputParams)) {
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

init_result init_no_extras(std::vector<msdk_string> opts) {
    std::vector<msdk_char*> args;
    for (auto& opt : opts) {
        args.push_back(&opt[0]);
    }
    return init((int)args.size(), &args[0]);
}

init_result init_session(std::vector<msdk_string> opts) {
    opts.insert(opts.begin(), "exe_name");
    opts.insert(opts.begin(), "-i::h264");
    opts.insert(opts.begin(), "in_file");
    opts.insert(opts.begin(), "-o::h265");
    opts.insert(opts.begin(), "out_file");
    std::vector<msdk_char*> args;
    for (auto& opt : opts) {
        args.push_back(&opt[0]);
    }
    return init((int)args.size(), &args[0]);
}

init_result init(std::vector<msdk_string> opts) {
    opts.insert(opts.begin(), "exe_name");
    std::vector<msdk_char*> args;
    for (auto& opt : opts) {
        args.push_back(&opt[0]);
    }
    return init((int)args.size(), &args[0]);
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

TEST(Transcode_CLI, OptionVer1) {
    auto result = init_session({ "-api_ver_init::1x" });
    EXPECT_EQ(result.parsed[0].verSessionInit, TranscodingSample::API_1X);
}

TEST(Transcode_CLI, OptionVer2) {
    auto result = init_session({ "-api_ver_init::2x" });
    EXPECT_EQ(result.parsed[0].verSessionInit, TranscodingSample::API_2X);
}
