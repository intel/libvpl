//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/defs.hpp"
#include <sstream>
#include "vpl/preview/detail/string_helpers.hpp"
#include "vpl_python.hpp"

namespace vpl = oneapi::vpl;

void init_defs(const py::module &m) {
    auto component = py::enum_<vpl::component>(m, "component")
                         .value("decoder", vpl::component::decoder)
                         .value("encoder", vpl::component::encoder)
                         .value("vpp", vpl::component::vpp)
                         .value("decoder_vpp", vpl::component::decoder_vpp)
                         .value("unknown", vpl::component::unknown);
    component.attr("__str__") = py::cpp_function(
        [](vpl::component self) {
            return vpl::detail::ChromaFormat2String((mfxImplType)self);
        },
        py::name("__str__"),
        py::is_method(component));

    auto async_op_status = py::enum_<vpl::async_op_status>(m, "async_op_status")
                               .value("ready", vpl::async_op_status::ready)
                               .value("timeout", vpl::async_op_status::timeout)
                               .value("aborted", vpl::async_op_status::aborted)
                               .value("unknown", vpl::async_op_status::unknown)
                               .value("cancelled", vpl::async_op_status::cancelled);

    auto status = py::enum_<vpl::status>(m, "status")
                      .value("Ok", vpl::status::Ok)
                      .value("ExecutionInProgress", vpl::status::ExecutionInProgress)
                      .value("DeviceBusy", vpl::status::DeviceBusy)
                      .value("VideoParamChanged", vpl::status::VideoParamChanged)
                      .value("PartialAcceleration", vpl::status::PartialAcceleration)
                      .value("IncompartibleVideoParam", vpl::status::IncompartibleVideoParam)
                      .value("ValueNotChanged", vpl::status::ValueNotChanged)
                      .value("OutOfRange", vpl::status::OutOfRange)
                      .value("TaskWorking", vpl::status::TaskWorking)
                      .value("TaskBusy", vpl::status::TaskBusy)
                      .value("FilterSkipped", vpl::status::FilterSkipped)
                      .value("PartialOutput", vpl::status::PartialOutput)
                      .value("NotEnoughData", vpl::status::NotEnoughData)
                      .value("NotEnoughSurface", vpl::status::NotEnoughSurface)
                      .value("NotEnoughBuffer", vpl::status::NotEnoughBuffer)
                      .value("EndOfStreamReached", vpl::status::EndOfStreamReached)
                      .value("Unknown", vpl::status::Unknown);
    status.attr("__str__") = py::cpp_function(
        [](vpl::status self) {
            return vpl::detail::status2Str(self);
        },
        py::name("__str__"),
        py::is_method(status));

    auto codec_format_fourcc = py::enum_<vpl::codec_format_fourcc>(m, "codec_format_fourcc")
                                   .value("avc", vpl::codec_format_fourcc::avc)
                                   .value("hevc", vpl::codec_format_fourcc::hevc)
                                   .value("mpeg2", vpl::codec_format_fourcc::mpeg2)
                                   .value("vc1", vpl::codec_format_fourcc::vc1)
                                   .value("capture", vpl::codec_format_fourcc::capture)
                                   .value("vp9", vpl::codec_format_fourcc::vp9)
                                   .value("av1", vpl::codec_format_fourcc::av1);
    codec_format_fourcc.attr("__str__") = py::cpp_function(
        [](vpl::codec_format_fourcc self) {
            return vpl::detail::FourCC2String((uint32_t)self);
        },
        py::name("__str__"),
        py::is_method(codec_format_fourcc));

    auto color_format_fourcc = py::enum_<vpl::color_format_fourcc>(m, "color_format_fourcc")
                                   .value("nv12", vpl::color_format_fourcc::nv12)
                                   .value("yv12", vpl::color_format_fourcc::yv12)
                                   .value("nv16", vpl::color_format_fourcc::nv16)
                                   .value("yuy2", vpl::color_format_fourcc::yuy2)
                                   .value("rgb465", vpl::color_format_fourcc::rgb465)
                                   .value("rgbp", vpl::color_format_fourcc::rgbp)
                                   .value("rgb3", vpl::color_format_fourcc::rgb3)
                                   .value("bgra", vpl::color_format_fourcc::bgra)
                                   .value("p8", vpl::color_format_fourcc::p8)
                                   .value("p8_texture", vpl::color_format_fourcc::p8_texture)
                                   .value("p010", vpl::color_format_fourcc::p010)
                                   .value("p016", vpl::color_format_fourcc::p016)
                                   .value("p210", vpl::color_format_fourcc::p210)
                                   .value("bgr4", vpl::color_format_fourcc::bgr4)
                                   .value("a2rgb10", vpl::color_format_fourcc::a2rgb10)
                                   .value("argb16", vpl::color_format_fourcc::argb16)
                                   .value("abgr16", vpl::color_format_fourcc::abgr16)
                                   .value("r16", vpl::color_format_fourcc::r16)
                                   .value("ayuv", vpl::color_format_fourcc::ayuv)
                                   .value("ayuv_rgb4", vpl::color_format_fourcc::ayuv_rgb4)
                                   .value("uyvy", vpl::color_format_fourcc::uyvy)
                                   .value("y210", vpl::color_format_fourcc::y210)
                                   .value("y410", vpl::color_format_fourcc::y410)
                                   .value("y216", vpl::color_format_fourcc::y216)
                                   .value("y416", vpl::color_format_fourcc::y416)
                                   .value("nv21", vpl::color_format_fourcc::nv21)
                                   .value("i420", vpl::color_format_fourcc::i420)
                                   .value("i010", vpl::color_format_fourcc::i010);
    color_format_fourcc.attr("__str__") = py::cpp_function(
        [](vpl::color_format_fourcc self) {
            return vpl::detail::FourCC2String((uint32_t)self);
        },
        py::name("__str__"),
        py::is_method(color_format_fourcc));

    auto chroma_format_idc = py::enum_<vpl::chroma_format_idc>(m, "chroma_format_idc")
                                 .value("monochrome", vpl::chroma_format_idc::monochrome)
                                 .value("yuv420", vpl::chroma_format_idc::yuv420)
                                 .value("yuv422", vpl::chroma_format_idc::yuv422)
                                 .value("yuv444", vpl::chroma_format_idc::yuv444)
                                 .value("yuv400", vpl::chroma_format_idc::yuv400)
                                 .value("yuv411", vpl::chroma_format_idc::yuv411)
                                 .value("yuv422h", vpl::chroma_format_idc::yuv422h)
                                 .value("yuv422v", vpl::chroma_format_idc::yuv422v);
    chroma_format_idc.attr("__str__") = py::cpp_function(
        [](vpl::chroma_format_idc self) {
            return vpl::detail::ChromaFormat2String((uint16_t)self);
        },
        py::name("__str__"),
        py::is_method(chroma_format_idc));

    auto target_usage = py::enum_<vpl::target_usage>(m, "target_usage")
                            .value("t1", vpl::target_usage::t1)
                            .value("t2", vpl::target_usage::t2)
                            .value("t3", vpl::target_usage::t3)
                            .value("t4", vpl::target_usage::t4)
                            .value("t5", vpl::target_usage::t5)
                            .value("t6", vpl::target_usage::t6)
                            .value("t7", vpl::target_usage::t7)
                            .value("unknown", vpl::target_usage::unknown)
                            .value("best_quality", vpl::target_usage::best_quality)
                            .value("balanced", vpl::target_usage::balanced)
                            .value("best_speed", vpl::target_usage::best_speed);

    auto rate_control_method = py::enum_<vpl::rate_control_method>(m, "rate_control_method")
                                   .value("cbr", vpl::rate_control_method::cbr)
                                   .value("vbr", vpl::rate_control_method::vbr)
                                   .value("cqp", vpl::rate_control_method::cqp)
                                   .value("avbr", vpl::rate_control_method::avbr)
                                   .value("la", vpl::rate_control_method::la)
                                   .value("icq", vpl::rate_control_method::icq)
                                   .value("vcm", vpl::rate_control_method::vcm)
                                   .value("la_icq", vpl::rate_control_method::la_icq)
                                   .value("la_hrd", vpl::rate_control_method::la_hrd)
                                   .value("qvbr", vpl::rate_control_method::qvbr);
    rate_control_method.attr("__str__") = py::cpp_function(
        [](vpl::rate_control_method self) {
            return vpl::detail::RateControlMethod2String((uint16_t)self);
        },
        py::name("__str__"),
        py::is_method(rate_control_method));

    auto io_pattern = py::enum_<vpl::io_pattern>(m, "io_pattern", py::arithmetic())
                          .value("in_device_memory", vpl::io_pattern::in_device_memory)
                          .value("in_system_memory", vpl::io_pattern::in_system_memory)
                          .value("out_device_memory", vpl::io_pattern::out_device_memory)
                          .value("out_system_memory", vpl::io_pattern::out_system_memory)
                          .value("io_system_memory", vpl::io_pattern::io_system_memory);
    io_pattern.attr("__str__") = py::cpp_function(
        [](vpl::io_pattern self) {
            return vpl::detail::IOPattern2String((uint16_t)self);
        },
        py::name("__str__"),
        py::is_method(io_pattern));

    auto pic_struct = py::enum_<vpl::pic_struct>(m, "pic_struct")
                          .value("unknown", vpl::pic_struct::unknown)
                          .value("progressive", vpl::pic_struct::progressive)
                          .value("field_tff", vpl::pic_struct::field_tff)
                          .value("field_bff", vpl::pic_struct::field_bff)
                          .value("field_repeated", vpl::pic_struct::field_repeated)
                          .value("frame_doubling", vpl::pic_struct::frame_doubling)
                          .value("frame_tripling", vpl::pic_struct::frame_tripling)
                          .value("field_single", vpl::pic_struct::field_single)
                          .value("field_top", vpl::pic_struct::field_top)
                          .value("field_bottom", vpl::pic_struct::field_bottom)
                          .value("field_paired_prev", vpl::pic_struct::field_paired_prev)
                          .value("field_paired_next", vpl::pic_struct::field_paired_next);
    pic_struct.attr("__str__") = py::cpp_function(
        [](vpl::pic_struct self) {
            return vpl::detail::PicStruct2String((uint16_t)self);
        },
        py::name("__str__"),
        py::is_method(pic_struct));

    auto memory_access = py::enum_<vpl::memory_access>(m, "memory_access")
                             .value("read", vpl::memory_access::read)
                             .value("write", vpl::memory_access::write)
                             .value("read_write", vpl::memory_access::read_write)
                             .value("nowait", vpl::memory_access::nowait);

    auto hevc_nal_unit_type = py::enum_<vpl::hevc_nal_unit_type>(m, "hevc_nal_unit_type")
                                  .value("unknown", vpl::hevc_nal_unit_type::unknown)
                                  .value("trail_n", vpl::hevc_nal_unit_type::trail_n)
                                  .value("trail_r", vpl::hevc_nal_unit_type::trail_r)
                                  .value("radl_n", vpl::hevc_nal_unit_type::radl_n)
                                  .value("radl_r", vpl::hevc_nal_unit_type::radl_r)
                                  .value("rasl_n", vpl::hevc_nal_unit_type::rasl_n)
                                  .value("rasl_r", vpl::hevc_nal_unit_type::rasl_r)
                                  .value("idr_w_radl", vpl::hevc_nal_unit_type::idr_w_radl)
                                  .value("idr_n_lp", vpl::hevc_nal_unit_type::idr_n_lp)
                                  .value("cra_nut", vpl::hevc_nal_unit_type::cra_nut);

    auto frame_type = py::enum_<vpl::frame_type>(m, "frame_type")
                          .value("unknown", vpl::frame_type::unknown)
                          .value("i", vpl::frame_type::i)
                          .value("p", vpl::frame_type::p)
                          .value("b", vpl::frame_type::b)
                          .value("s", vpl::frame_type::s)
                          .value("ref", vpl::frame_type::ref)
                          .value("idr", vpl::frame_type::idr)
                          .value("xi", vpl::frame_type::xi)
                          .value("xp", vpl::frame_type::xp)
                          .value("xb", vpl::frame_type::xb)
                          .value("xs", vpl::frame_type::xs)
                          .value("xref", vpl::frame_type::xref)
                          .value("xidr", vpl::frame_type::xidr);

    auto implementation = py::enum_<vpl::implementation>(m, "implementation")
                              .value("automatic", vpl::implementation::automatic)
                              .value("unsupported", vpl::implementation::unsupported)
                              .value("software", vpl::implementation::software)
                              .value("hardware", vpl::implementation::hardware)
                              .value("auto_any", vpl::implementation::auto_any)
                              .value("hardware_any", vpl::implementation::hardware_any)
                              .value("hardware_2", vpl::implementation::hardware_2)
                              .value("hardware_3", vpl::implementation::hardware_3)
                              .value("hardware_4", vpl::implementation::hardware_4)
                              .value("hardware_runtime", vpl::implementation::hardware_runtime);
    implementation.attr("__str__") = py::cpp_function(
        [](vpl::implementation self) {
            return vpl::detail::ImplType2String((mfxImplType)self);
        },
        py::name("__str__"),
        py::is_method(implementation));

    auto implementation_via = py::enum_<vpl::implementation_via>(m, "implementation_via")
                                  .value("any", vpl::implementation_via::any)
                                  .value("d3d9", vpl::implementation_via::d3d9)
                                  .value("d3d11", vpl::implementation_via::d3d11)
                                  .value("vaapi", vpl::implementation_via::vaapi)
                                  .value("hddlunite", vpl::implementation_via::hddlunite)
                                  .value("none", vpl::implementation_via::none);
    implementation_via.attr("__str__") = py::cpp_function(
        [](vpl::implementation_via self) {
            return vpl::detail::AccelerationMode2String((mfxAccelerationMode)self);
        },
        py::name("__str__"),
        py::is_method(implementation_via));

    auto handle_type = py::enum_<vpl::handle_type>(m, "handle_type")
                           .value("d3d9_device_manager", vpl::handle_type::d3d9_device_manager)
                           .value("d3d11_device_manager", vpl::handle_type::d3d11_device_manager)
                           .value("va_display", vpl::handle_type::va_display)
                           .value("va_config_id", vpl::handle_type::va_config_id)
                           .value("va_context_id", vpl::handle_type::va_context_id)
                           .value("cm_device", vpl::handle_type::cm_device);
}
