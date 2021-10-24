//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) encode application,
/// using 2.x API with internal memory management
///
/// @file

#include <fstream>
#include <iostream>

#include "util.hpp"

#define TARGETKBPS  4000
#define FRAMERATE   30
#define OUTPUT_FILE "out.h265"

namespace vpl = oneapi::vpl;

void WriteEncodedStream(std::shared_ptr<vpl::bitstream_as_dst> bits, std::ofstream *f);

void Usage(void) {
    std::cout << std::endl;
    std::cout << "   Usage  :  hello-encode\n\n";
    std::cout << "     -hw     use hardware implementation\n";
    std::cout << "     -sw     use software implementation\n";
    std::cout << "     -i      input file name (raw frames)\n";
    std::cout << "     -w      input width\n";
    std::cout << "     -h      input height\n";
    std::cout << "     -vmem   use video memory\n\n";
    std::cout << "   Example:  hello-encode -i in.i420 -w 128 -h 96\n";
    std::cout << "   To view:  ffplay " << OUTPUT_FILE << "\n\n";
    std::cout << " * Encode raw frames to HEVC/H265 elementary stream in " << OUTPUT_FILE << "\n\n";
    std::cout << "   CPU native color format is I420/yuv420p.  GPU native color format is NV12\n";
    return;
}

int main(int argc, char *argv[]) {
    Params cliParams = {};

    //Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_ENCODE) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    std::ifstream source;
    std::ofstream sink;

    // Setup input and output files
    source.open(cliParams.infileName, std::ios_base::in | std::ios_base::binary);
    if (!source) {
        std::cout << "Couldn't open input file" << std::endl;
        return 1;
    }

    sink.open(OUTPUT_FILE, std::ios_base::out | std::ios_base::binary);
    if (!sink) {
        std::cout << "Couldn't open output file" << std::endl;
        return 1;
    }

    int frame_num                              = 0;
    bool is_stillgoing                         = true;
    oneapi::vpl::implementation_type impl_type = cliParams.implValue;

    // Initialize VPL session for any implementation of HEVC/H265 encode
    // Default implementation selector. Selects first impl based on property list.
    vpl::default_selector impl_sel({ oneapi::vpl::dprops::impl(impl_type),
                                     oneapi::vpl::dprops::api_version(2, 5),
                                     oneapi::vpl::dprops::encoder({ oneapi::vpl::dprops::codec_id(
                                         vpl::codec_format_fourcc::hevc) }) });

    vpl::ExtDecodeErrorReport err_report;
    vpl::color_format_fourcc input_fourcc = (impl_type == oneapi::vpl::implementation_type::sw)
                                                ? vpl::color_format_fourcc::i420
                                                : vpl::color_format_fourcc::nv12;

    // create raw freames reader
    vpl::raw_frame_file_reader reader(cliParams.srcWidth,
                                      cliParams.srcHeight,
                                      input_fourcc,
                                      source);

    // Load session and initialize encoder
    std::shared_ptr<vpl::encode_session> encoder(nullptr);
    try {
        encoder = std::make_shared<vpl::encode_session>(impl_sel, &reader);
    }
    catch (vpl::base_exception &e) {
        std::cout << "Encoder session create failed: " << e.what() << std::endl;
        return -1;
    }

    // Initialize encode parameters
    std::shared_ptr<vpl::encoder_video_param> enc_params =
        std::make_shared<vpl::encoder_video_param>();
    vpl::frame_info info;

    info.set_frame_rate({ FRAMERATE, 1 });
    info.set_frame_size({ ALIGN16(cliParams.srcWidth), ALIGN16(cliParams.srcHeight) });
    info.set_FourCC(input_fourcc);
    info.set_ChromaFormat(vpl::chroma_format_idc::yuv420);
    info.set_ROI({ { 0, 0 }, { cliParams.srcWidth, cliParams.srcHeight } });
    info.set_PicStruct(vpl::pic_struct::progressive);

    enc_params->set_RateControlMethod(vpl::rate_control_method::cqp);
    enc_params->set_frame_info(info);
    enc_params->set_CodecId(vpl::codec_format_fourcc::hevc);
    enc_params->set_IOPattern((cliParams.useVideoMemory) ? vpl::io_pattern::in_device_memory
                                                         : vpl::io_pattern::in_system_memory);

    try {
        encoder->Init(enc_params.get());
    }
    catch (vpl::base_exception &e) {
        std::cout << "Encoder init failed: " << e.what() << std::endl;
        return -1;
    }

    std::cout << info << std::endl;

    std::cout << "Init done" << std::endl;

    std::cout << "Encoding " << cliParams.infileName << " -> " << OUTPUT_FILE << std::endl;

    // main encoder Loop
    while (is_stillgoing == true) {
        vpl::status wrn = vpl::status::Ok;

        std::shared_ptr<vpl::bitstream_as_dst> b = std::make_shared<vpl::bitstream_as_dst>();
        try {
            wrn = encoder->encode_frame(b);
        }
        catch (vpl::base_exception &e) {
            std::cout << "Encoder died: " << e.what() << std::endl;
            return -1;
        }

        switch (wrn) {
            case vpl::status::Ok: {
                std::chrono::duration<int, std::milli> waitduration(WAIT_100_MILLISECONDS);
                b->wait_for(waitduration);

                WriteEncodedStream(b, &sink);
                frame_num++;
            } break;
            case vpl::status::EndOfStreamReached:
                std::cout << "EndOfStream Reached" << std::endl;
                is_stillgoing = false;
                break;
            case vpl::status::DeviceBusy:
                // For non-CPU implementations,
                // Wait a few milliseconds then try again
                std::cout << "DeviceBusy" << std::endl;
                break;
            default:
                std::cout << "unknown status: " << static_cast<int>(wrn) << std::endl;
                is_stillgoing = false;
                break;
        }
    }

    std::cout << "Encoded " << frame_num << " frames" << std::endl;

    std::cout << "\n-- Encode information --\n\n";
    std::shared_ptr<vpl::encoder_video_param> p = encoder->working_params();
    std::cout << *(p.get()) << std::endl;

    return 0;
}

// Write encoded stream to file
void WriteEncodedStream(std::shared_ptr<vpl::bitstream_as_dst> bits, std::ofstream *f) {
    auto [ptr, len] = bits->get_valid_data();
    f->write(reinterpret_cast<char *>(ptr), len);
    bits->set_DataLength(0);
    return;
}
