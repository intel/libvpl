//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) decode application,
/// using oneVPL internal memory management
///
/// @file

#include <fstream>
#include <iostream>

#include "util.hpp"
#include "vpl/preview/option_tree.hpp"

#define OUTPUT_FILE "out.raw"

namespace vpl = oneapi::vpl;

std::ofstream &operator<<(std::ofstream &f, std::pair<vpl::frame_info, vpl::frame_data> frame);

void Usage(void) {
    std::cout << std::endl;
    std::cout << "   Usage  :  hello-decode \n\n";
    std::cout << "     -sw     use software implementation\n";
    std::cout << "     -hw     use hardware implementation\n";
    std::cout << "     -i      input file name (HEVC elementary stream)\n";
    std::cout << "     -vmem   use video memory\n\n";
    std::cout << "   Example:  hello-decode -sw  -i in.h265\n";
    std::cout
        << "   To view:  ffplay -f rawvideo -pixel_format yuv420p -video_size [width]x[height] "
        << OUTPUT_FILE << "\n\n";
    std::cout << " * Decode HEVC/H265 elementary stream to raw frames in " << OUTPUT_FILE << "\n\n";
    std::cout << "   CPU native color format is I420/yuv420p.  GPU native color format is NV12\n";
    return;
}

int main(int argc, char *argv[]) {
    Params cliParams = {};

    //Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_DECODE) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    std::ifstream source;
    std::ofstream sink;
    uint32_t frame_num = 0;
    bool is_stillgoing = true;
    oneapi::vpl::implementation_type impl_type;

    impl_type = cliParams.implValue;

    // Default implementation selector. Selects first impl based on property list.
    oneapi::vpl::properties opts;
    opts.impl             = impl_type;
    opts.api_version      = { 2, 5 };
    opts.decoder.codec_id = { vpl::codec_format_fourcc::hevc };
    std::cout << opts;
    vpl::default_selector impl_sel(opts);

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

    vpl::status ret = vpl::status::Ok;
    vpl::ExtDecodeErrorReport err_report;

    // File reader
    vpl::bitstream_file_reader fr(source);

    // Load session and initialize decoder
    vpl::decoder_video_param param;
    param.set_IOPattern((cliParams.useVideoMemory) ? vpl::io_pattern::out_device_memory
                                                   : vpl::io_pattern::out_system_memory);
    param.set_CodecId(vpl::codec_format_fourcc::hevc);

    std::shared_ptr<vpl::decode_session<oneapi::vpl::bitstream_file_reader>> decoder(nullptr);
    try {
        decoder =
            std::make_shared<vpl::decode_session<oneapi::vpl::bitstream_file_reader>>(impl_sel,
                                                                                      param,
                                                                                      &fr);
    }
    catch (vpl::base_exception &e) {
        std::cout << "Decoder session create failed: " << e.what() << std::endl;
        return -1;
    }

    vpl::decoder_init_header_list init_header_list;
    ret = decoder->init_by_header(init_header_list);

    if (ret != vpl::status::Ok) {
        std::cout << "Unknown status: " << static_cast<int>(ret) << std::endl;
        return 1;
    }

    std::cout << "Decoding " << cliParams.infileName << " -> " << OUTPUT_FILE << std::endl;

    // main decoder Loop
    while (is_stillgoing == true) {
        std::cout << "Decoding " << frame_num << " frame"
                  << "\r";
        // Decode frame request
        std::shared_ptr<vpl::frame_surface> dec_surface_out =
            std::make_shared<vpl::frame_surface>();
        try {
            ret = decoder->decode_frame(dec_surface_out,
                                        oneapi::vpl::decoder_process_list{ &err_report });
        }
        // if error happened
        catch (vpl::base_exception &e) {
            switch (e.get_status()) {
                case MFX_ERR_DEVICE_LOST:
                    // For non-CPU implementations
                    // Cleanup if device is lost
                    is_stillgoing = false;
                    break;
                default:
                    std::cout << "Error happened: " << e.what() << std::endl;
                    return -1;
                    break;
            }
        }
        catch (...) {
            std::cout << "Handle unknown exeption." << std::endl;
            is_stillgoing = false;
        }
        if (is_stillgoing == false)
            continue;

        // check decoder errors type through extension buffer
        // Retrieve decoded image
        if (err_report.get_ref().ErrorTypes)
            std::cout << "Error type = " << err_report.get_ref().ErrorTypes << std::endl;

        vpl::async_op_status st;
        switch (ret) {
            case vpl::status::Ok:
                do {
                    // sync on surface
                    std::chrono::duration<int, std::milli> waitduration(WAIT_100_MILLISECONDS);
                    st = dec_surface_out->wait_for(waitduration);
                    if (vpl::async_op_status::ready == st) {
                        // read and store ready frame
                        try {
                            sink << dec_surface_out->map(vpl::memory_access::read);
                            frame_num++;
                            dec_surface_out->unmap();
                        }
                        catch (vpl::base_exception &e) {
                            std::cout << "Got exception: " << e.what() << std::endl;
                        }
                    }
                } while (st == vpl::async_op_status::timeout);
                break;
            // Source reader reported EOS
            case vpl::status::EndOfStreamReached:
                std::cout << "All input data is processed." << std::endl;
                is_stillgoing = false;
                break;
            case vpl::status::NotEnoughData:
                std::cout << "Got not enough data." << std::endl;
                break;
            case vpl::status::DeviceBusy:
                // For non-CPU implementations
                // Wait a few milliseconds then try again
                break;
            default:
                std::cout << "Unknown status: " << static_cast<int>(ret) << std::endl;
                is_stillgoing = false;
                break;
        }
    }

    std::cout << "Decoded " << frame_num << " frames" << std::endl;

    std::cout << "\n-- Decode information --\n\n";
    std::shared_ptr<vpl::decoder_video_param> p = decoder->working_params();
    std::cout << *(p.get()) << std::endl;

    return 0;
}

// Write raw I420 frame to file
std::ofstream &operator<<(std::ofstream &f, std::pair<vpl::frame_info, vpl::frame_data> frame) {
    auto [info, data] = frame;

    uint16_t i, pitch;

    auto [w, h] = info.get_frame_size();

    // write the output to disk
    switch (info.get_FourCC()) {
        case vpl::color_format_fourcc::i420: {
            auto [Y, U, V] = data.get_plane_ptrs_3();
            // Y
            pitch = data.get_pitch();
            for (i = 0; i < h; i++) {
                f.write(reinterpret_cast<const char *>(Y + i * pitch), w);
            }
            // U
            pitch /= 2;
            h /= 2;
            w /= 2;
            for (i = 0; i < h; i++) {
                f.write(reinterpret_cast<const char *>(U + i * pitch), w);
            }
            // V
            for (i = 0; i < h; i++) {
                f.write(reinterpret_cast<const char *>(V + i * pitch), w);
            }
            break;
        }
        case vpl::color_format_fourcc::nv12: {
            auto [Y, UV] = data.get_plane_ptrs_2();
            // Y
            pitch = data.get_pitch();
            for (i = 0; i < h; i++) {
                f.write(reinterpret_cast<const char *>(Y + i * pitch), w);
            }
            // UV
            h /= 2;
            for (i = 0; i < h; i++) {
                f.write(reinterpret_cast<const char *>(UV + i * pitch), w);
            }
            break;
        }
        default:
            std::cout << "Unsupported FourCC code, skip writing" << std::endl;
            break;
    }

    return f;
}
