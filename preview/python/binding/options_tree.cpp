//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/defs.hpp"
#include "vpl/preview/option_tree.hpp"
#include "vpl/preview/options.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

void init_option_tree(const py::module &m) {
    py::class_<vpl::codec_mem_desc_properties, std::shared_ptr<vpl::codec_mem_desc_properties>>(
        m,
        "codec_mem_desc_properties")
        .def_property("mem_type",
                      &vpl::codec_mem_desc_properties::get_mem_type,
                      &vpl::codec_mem_desc_properties::set_mem_type)
        .def_property("frame_size",
                      &vpl::codec_mem_desc_properties::get_frame_size,
                      &vpl::codec_mem_desc_properties::set_frame_size)
        .def_property("color_format",
                      &vpl::codec_mem_desc_properties::get_color_format,
                      &vpl::codec_mem_desc_properties::set_color_format);

    py::class_<vpl::enc_mem_desc_properties,
               vpl::codec_mem_desc_properties,
               std::shared_ptr<vpl::enc_mem_desc_properties>>(m, "enc_mem_desc_properties");

    py::class_<vpl::dec_mem_desc_properties,
               vpl::codec_mem_desc_properties,
               std::shared_ptr<vpl::dec_mem_desc_properties>>(m, "dec_mem_desc_properties");

    py::class_<vpl::dec_profile_properties, std::shared_ptr<vpl::dec_profile_properties>>(
        m,
        "dec_profile_properties")
        .def_property("profile",
                      &vpl::dec_profile_properties::get_profile,
                      &vpl::dec_profile_properties::set_profile)
        .def_readonly("dec_mem_desc", &vpl::dec_profile_properties::dec_mem_desc);

    py::class_<vpl::memdesc_properties, std::shared_ptr<vpl::memdesc_properties>>(
        m,
        "memdesc_properties")
        .def_property("mem_type",
                      &vpl::memdesc_properties::get_mem_type,
                      &vpl::memdesc_properties::set_mem_type)
        .def_property("frame_size",
                      &vpl::memdesc_properties::get_frame_size,
                      &vpl::memdesc_properties::set_frame_size)
        .def_property("in_color_format",
                      &vpl::memdesc_properties::get_in_color_format,
                      &vpl::memdesc_properties::set_in_color_format)
        .def_property("out_color_format",
                      &vpl::memdesc_properties::get_out_color_format,
                      &vpl::memdesc_properties::set_out_color_format);

    py::class_<vpl::enc_profile_properties, std::shared_ptr<vpl::enc_profile_properties>>(
        m,
        "enc_profile_properties")
        .def_property("profile",
                      &vpl::enc_profile_properties::get_profile,
                      &vpl::enc_profile_properties::set_profile)
        .def_readonly("enc_mem_desc", &vpl::enc_profile_properties::enc_mem_desc);

    py::class_<vpl::decoder_properties, std::shared_ptr<vpl::decoder_properties>>(
        m,
        "decoder_properties")
        .def_property("codec_id",
                      &vpl::decoder_properties::get_codec_id,
                      &vpl::decoder_properties::set_codec_id)
        .def_property("max_codec_level",
                      &vpl::decoder_properties::get_max_codec_level,
                      &vpl::decoder_properties::set_max_codec_level)
        .def_readonly("dec_profile", &vpl::decoder_properties::dec_profile);

    py::class_<vpl::encoder_properties, std::shared_ptr<vpl::encoder_properties>>(
        m,
        "encoder_properties")
        .def_property("profile",
                      &vpl::encoder_properties::get_codec_id,
                      &vpl::encoder_properties::set_codec_id)
        .def_property("max_codec_level",
                      &vpl::encoder_properties::get_max_codec_level,
                      &vpl::encoder_properties::set_max_codec_level)
        .def_property("bidirectional_prediction",
                      &vpl::encoder_properties::get_bidirectional_prediction,
                      &vpl::encoder_properties::set_bidirectional_prediction)
        .def_readonly("enc_profile", &vpl::encoder_properties::enc_profile);

    py::class_<vpl::filter_properties, std::shared_ptr<vpl::filter_properties>>(m,
                                                                                "filter_properties")
        .def_property("filter_id",
                      &vpl::filter_properties::get_filter_id,
                      &vpl::filter_properties::set_filter_id)
        .def_property("max_delay_in_frames",
                      &vpl::filter_properties::get_max_delay_in_frames,
                      &vpl::filter_properties::set_max_delay_in_frames)
        .def_readonly("memdesc", &vpl::filter_properties::memdesc);

    py::class_<vpl::device_properties, std::shared_ptr<vpl::device_properties>>(m,
                                                                                "device_properties")
        .def_property("device_id",
                      &vpl::device_properties::get_device_id,
                      &vpl::device_properties::set_device_id)
        .def_property("media_adapter",
                      &vpl::device_properties::get_media_adapter,
                      &vpl::device_properties::set_media_adapter);

    py::class_<vpl::properties, std::shared_ptr<vpl::properties>>(m, "properties")
        .def(py::init<>())
        .def_property("impl_name", &vpl::properties::get_impl_name, &vpl::properties::set_impl_name)
        .def_property("impl", &vpl::properties::get_impl, &vpl::properties::set_impl)
        .def_property("acceleration_mode",
                      &vpl::properties::get_acceleration_mode,
                      &vpl::properties::set_acceleration_mode)
        .def_property("api_version",
                      &vpl::properties::get_api_version,
                      &vpl::properties::set_api_version)
        .def_property("license", &vpl::properties::get_license, &vpl::properties::set_license)
        .def_property("keywords", &vpl::properties::get_keywords, &vpl::properties::set_keywords)
        .def_property("vendor_id", &vpl::properties::get_vendor_id, &vpl::properties::set_vendor_id)
        .def_property("vendor_impl_id",
                      &vpl::properties::get_vendor_impl_id,
                      &vpl::properties::set_vendor_impl_id)
        .def_property("dxgi_adapter_index",
                      &vpl::properties::get_dxgi_adapter_index,
                      &vpl::properties::set_dxgi_adapter_index)
        .def_property("implemented_function",
                      &vpl::properties::get_implemented_function,
                      &vpl::properties::set_implemented_function)
        .def_property("pool_alloc_properties",
                      &vpl::properties::get_pool_alloc_properties,
                      &vpl::properties::set_pool_alloc_properties)
        .def_property("set_handle",
                      &vpl::properties::get_set_handle,
                      &vpl::properties::set_set_handle)
        .def_readonly("decoder", &vpl::properties::decoder)
        .def_readonly("encoder", &vpl::properties::encoder)
        .def_readonly("filter", &vpl::properties::filter)
        .def_readonly("device", &vpl::properties::device)
        .def("__str__", [](const vpl::properties *self) {
            std::stringstream strs;
            strs << *self;
            return strs.str();
        });

    py::implicitly_convertible<vpl::properties, vpl::property_list>();
}