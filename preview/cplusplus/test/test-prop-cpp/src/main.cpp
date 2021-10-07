//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include <fstream>
#include <iostream>
#include <sstream>

#include "vpl/preview/vpl.hpp"

bool TestPath(oneapi::vpl::property const &prop, const char *expectedPath) {
    auto val = prop.get_properties();
    if (val.size() != 1) {
        return false;
    }
    if (val[0].first != expectedPath) {
        std::cout << std::endl;
        std::cout << "Expected path: " << expectedPath << std::endl;
        std::cout << "Actual path: " << val[0].first << std::endl;
        return false;
    }
    return true;
}

bool TestPath(oneapi::vpl::property const &prop,
              const char *expectedPath1,
              const char *expectedPath2) {
    auto val = prop.get_properties();
    if (val.size() != 2) {
        return false;
    }
    if (val[0].first != expectedPath1) {
        std::cout << std::endl;
        std::cout << "Expected path: " << expectedPath1 << std::endl;
        std::cout << "Actual path: " << val[0].first << std::endl;
        return false;
    }
    if (val[1].first != expectedPath2) {
        std::cout << std::endl;
        std::cout << "Expected path: " << expectedPath2 << std::endl;
        std::cout << "Actual path: " << val[1].first << std::endl;
        return false;
    }
    return true;
}

#define TEST_CASE(dprop, val, expected_path)    \
    std::cout << "Test " << #dprop;             \
    std::cout << "[" << expected_path << "]";   \
    try {                                       \
        oneapi::vpl::dprops::dprop _p(val);     \
        if (!TestPath(_p, expected_path)) {     \
            printf("\n   Error!\n");            \
            return -1;                          \
        }                                       \
    }                                           \
    catch (...) {                               \
        printf("\n   Got exception. Error!\n"); \
        return -1;                              \
    }                                           \
    std::cout << " ...OK" << std::endl;

#define TEST_CASE2(dprop, val1, val2, expected_path1, expected_path2) \
    std::cout << "Test " << #dprop;                                   \
    std::cout << "[" << expected_path1 << ", " expected_path2 << "]"; \
    try {                                                             \
        oneapi::vpl::dprops::dprop _p(val1, val2);                    \
        if (!TestPath(_p, expected_path1, expected_path2)) {          \
            printf("\n   Error!\n");                                  \
            return -1;                                                \
        }                                                             \
    }                                                                 \
    catch (...) {                                                     \
        printf("\n   Got exception. Error!\n");                       \
        return -1;                                                    \
    }                                                                 \
    std::cout << " ...OK" << std::endl;

#define TEST_CASE3(dprop, val, expected_path1, expected_path2)        \
    std::cout << "Test " << #dprop;                                   \
    std::cout << "[" << expected_path1 << ", " expected_path2 << "]"; \
    try {                                                             \
        oneapi::vpl::dprops::dprop _p(val);                           \
        if (!TestPath(_p, expected_path1, expected_path2)) {          \
            printf("\n   Error!\n");                                  \
            return -1;                                                \
        }                                                             \
    }                                                                 \
    catch (...) {                                                     \
        printf("\n   Got exception. Error!\n");                       \
        return -1;                                                    \
    }                                                                 \
    std::cout << " ...OK" << std::endl;

using namespace oneapi::vpl; // NOLINT

static int TestProperty() {
    mfxRange32U width{ 10, 10, 1 };
    mfxRange32U height{ 20, 20, 1 };
    TEST_CASE(impl, implementation_type::sw, "mfxImplDescription.Impl");
    TEST_CASE2(api_version,
               2,
               5,
               "mfxImplDescription.ApiVersion.Major",
               "mfxImplDescription.ApiVersion.Minor");
    TEST_CASE(acceleration_mode, implementation_via::d3d11, "mfxImplDescription.AccelerationMode");
    TEST_CASE(impl_name, "Cool implementation", "mfxImplDescription.ImplName");
    TEST_CASE(license, "MIT", "mfxImplDescription.License");
    TEST_CASE(keywords, "keyword", "mfxImplDescription.Keywords");
    TEST_CASE(vendor_id, 0x8086, "mfxImplDescription.VendorID");
    TEST_CASE(vendor_impl_id, 0x1, "mfxImplDescription.VendorImplID");
    TEST_CASE(implemented_function, "MyFunc", "mfxImplementedFunctions.FunctionsName");
    TEST_CASE(dxgi_adapter_index, 1, "DXGIAdapterIndex");
    TEST_CASE2(set_handle, handle_type::va_display, 0, "mfxHandleType", "mfxHDL");
    TEST_CASE(pool_alloc_properties,
              pool_alloction_policy::optimal,
              "mfxImplDescription.mfxSurfacePoolMode");
    TEST_CASE(decoder,
              { dprops::codec_id(codec_format_fourcc::av1) },
              "mfxImplDescription.mfxDecoderDescription.decoder.CodecID");
    TEST_CASE(decoder,
              { dprops::max_codec_level(10) },
              "mfxImplDescription.mfxDecoderDescription.decoder.MaxcodecLevel");
    TEST_CASE(encoder,
              { dprops::codec_id(codec_format_fourcc::av1) },
              "mfxImplDescription.mfxEncoderDescription.encoder.CodecID");
    TEST_CASE(encoder,
              { dprops::max_codec_level(10) },
              "mfxImplDescription.mfxEncoderDescription.encoder.MaxcodecLevel");
    TEST_CASE(encoder,
              { dprops::bidirectional_prediction(10) },
              "mfxImplDescription.mfxEncoderDescription.encoder.BiDirectionalPrediction");
    TEST_CASE(filter,
              { dprops::filter_id(10) },
              "mfxImplDescription.mfxVPPDescription.filter.FilterFourCC");
    TEST_CASE(filter,
              { dprops::max_delay_in_frames(10) },
              "mfxImplDescription.mfxVPPDescription.filter.MaxDelayInFrames");
    TEST_CASE(device,
              { dprops::device_id("0x8086") },
              "mfxImplDescription.mfxDeviceDescription.device.DeviceID");
    TEST_CASE(device,
              { dprops::media_adapter(media_adapter_type::discrete) },
              "mfxImplDescription.mfxDeviceDescription.device.MediaAdapterType");

    TEST_CASE(filter,
              { dprops::memdesc{ dprops::mem_type(resource_type::system_surface) } },
              "mfxImplDescription.mfxVPPDescription.filter.memdesc.MemHandleType");
    TEST_CASE3(filter,
               { dprops::memdesc{ dprops::frame_size(width, height) } },
               "mfxImplDescription.mfxVPPDescription.filter.memdesc.Width",
               "mfxImplDescription.mfxVPPDescription.filter.memdesc.Height");
    TEST_CASE(filter,
              { dprops::memdesc{ dprops::in_color_format(color_format_fourcc::bgr4) } },
              "mfxImplDescription.mfxVPPDescription.filter.memdesc.format.InFormat");
    TEST_CASE(filter,
              { dprops::memdesc{ dprops::out_color_format(color_format_fourcc::bgra) } },
              "mfxImplDescription.mfxVPPDescription.filter.memdesc.format.OutFormats");

    TEST_CASE(decoder,
              { dprops::dec_profile{ dprops::profile(10) } },
              "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile");
    TEST_CASE(
        decoder,
        { dprops::dec_profile{
            dprops::dec_mem_desc{ dprops::mem_type(resource_type::dx11_texture) } } },
        "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.MemHandleType");
    TEST_CASE(
        decoder,
        { dprops::dec_profile{
            dprops::dec_mem_desc{ dprops::color_format(color_format_fourcc::bgr4) } } },
        "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.ColorFormats");
    TEST_CASE3(decoder,
               { dprops::dec_profile{ dprops::dec_mem_desc{ dprops::frame_size(width, height) } } },
               "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.Width",
               "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.Height");

    TEST_CASE(encoder,
              { dprops::enc_profile{ dprops::profile(10) } },
              "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.Profile");
    TEST_CASE(
        encoder,
        { dprops::enc_profile{
            dprops::enc_mem_desc{ dprops::mem_type(resource_type::va_surface_ptr) } } },
        "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.MemHandleType");
    TEST_CASE(
        encoder,
        { dprops::enc_profile{
            dprops::enc_mem_desc{ dprops::color_format(color_format_fourcc::bgr4) } } },
        "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.ColorFormats");
    TEST_CASE3(encoder,
               { dprops::enc_profile{ dprops::enc_mem_desc{ dprops::frame_size(width, height) } } },
               "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.Width",
               "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.Height");

    printf(" ... OK properties\n");

    return 0;
}

int main(int argc, char *argv[]) {
    int res;

    res = 0;
    res |= TestProperty();

    if (res)
        printf("\nErrors in property string parsing\n");
    else
        printf("\nSuccess!\n");

    return res;
}