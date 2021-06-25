//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/impl_caps.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

void init_impl_caps(const py::module &m) {
    py::class_<vpl::base_implementation_capabilities>(m, "base_implementation_capabilities")
        .def(py::init<uint32_t>())
        .def_property_readonly("id",
                               &vpl::base_implementation_capabilities::get_id,
                               "implementation capabilities format ID.")
        .def("__str__", [](const vpl::implementation_capabilities *self, std::string) {
            std::stringstream strs;
            strs << *self;
            return strs.str();
        });

    py::class_<vpl::implementation_capabilities, vpl::base_implementation_capabilities>(
        m,
        "implementation_capabilities")
        .def(py::init<mfxImplDescription *>())
        .def_property_readonly("impl_type",
                               &vpl::implementation_capabilities::get_impl_type,
                               "Provides type of the implementation: SW or HW.")
        .def_property_readonly("acceleration_mode",
                               &vpl::implementation_capabilities::get_acceleration_mode,
                               "Provides acceleration mode of the implementation.")
        .def_property_readonly("api_version",
                               &vpl::implementation_capabilities::get_api_version,
                               "Provides supported API version of the implementation.")
        .def_property_readonly("impl_name",
                               &vpl::implementation_capabilities::get_impl_name,
                               "Provides name of the implementation.")
        .def_property_readonly("license_name",
                               &vpl::implementation_capabilities::get_license_name,
                               "Provides license of the implementation.")
        .def_property_readonly("keywords_name",
                               &vpl::implementation_capabilities::get_keywords_name,
                               "Provides keywords of the implementation.")
        .def_property_readonly("vendor_id",
                               &vpl::implementation_capabilities::get_vendor_id,
                               "Provides vendor ID.")
        .def_property_readonly("vendor_impl_id",
                               &vpl::implementation_capabilities::get_vendor_impl_id,
                               "Provides vendor's implementation ID.")
        .def_property_readonly("target_device_id",
                               &vpl::implementation_capabilities::get_target_device_id,
                               "Provides target device for the implementation.")
        .def_property_readonly("target_subdevice_ids",
                               &vpl::implementation_capabilities::get_target_subdevice_ids,
                               "Provides list of supported target subdevice IDs.")
        .def_property_readonly("decoders",
                               &vpl::implementation_capabilities::get_decoders,
                               "Provides list of supported decoders.")
        .def_property_readonly("encoders",
                               &vpl::implementation_capabilities::get_encoders,
                               "Provides list of supported encoders.")
        .def_property_readonly("vpp_filters",
                               &vpl::implementation_capabilities::get_vpp_filters,
                               "Provides list of supported VPP filters.");

    py::class_<vpl::implementation_capabilities::decoder_memory>(
        m,
        "implementation_capabilities_decoder_memory")
        .def(py::init<mfxDecoderDescription::decoder::decprofile::decmemdesc>())
        .def_property_readonly("mem_type",
                               &vpl::implementation_capabilities::decoder_memory::get_mem_type,
                               "supported memory type.")
        .def_property_readonly("min_size",
                               &vpl::implementation_capabilities::decoder_memory::get_min_size,
                               "minimal supported width and height.")
        .def_property_readonly("max_size",
                               &vpl::implementation_capabilities::decoder_memory::get_max_size,
                               "maximal supported width and height.")
        .def_property_readonly("size_step",
                               &vpl::implementation_capabilities::decoder_memory::get_size_step,
                               "increment value for width and height.")
        .def_property_readonly("out_mem_types",
                               &vpl::implementation_capabilities::decoder_memory::get_out_mem_types,
                               "list of supported output memory types.")
        .def("__str__",
             [](const vpl::implementation_capabilities::decoder_memory *self, std::string) {
                 std::stringstream strs;
                 strs << *self;
                 return strs.str();
             });

    py::class_<vpl::implementation_capabilities::encoder_memory>(
        m,
        "implementation_capabilities_encoder_memory")
        .def(py::init<mfxEncoderDescription::encoder::encprofile::encmemdesc>())
        .def_property_readonly("mem_type",
                               &vpl::implementation_capabilities::encoder_memory::get_mem_type,
                               "supported memory type.")
        .def_property_readonly("min_size",
                               &vpl::implementation_capabilities::encoder_memory::get_min_size,
                               "minimal supported width and height.")
        .def_property_readonly("max_size",
                               &vpl::implementation_capabilities::encoder_memory::get_max_size,
                               "maximal supported width and height.")
        .def_property_readonly("size_step",
                               &vpl::implementation_capabilities::encoder_memory::get_size_step,
                               "increment value for width and height.")
        .def_property_readonly("out_mem_types",
                               &vpl::implementation_capabilities::encoder_memory::get_out_mem_types,
                               "list of supported output memory types.")
        .def("__str__",
             [](const vpl::implementation_capabilities::encoder_memory *self, std::string) {
                 std::stringstream strs;
                 strs << *self;
                 return strs.str();
             });

    py::class_<vpl::implementation_capabilities::vpp_memory_format>(
        m,
        "implementation_capabilities_vpp_memory_format")
        .def(py::init<mfxVPPDescription::filter::memdesc::format>())
        .def_property_readonly(
            "input_format",
            &vpl::implementation_capabilities::vpp_memory_format::get_input_format,
            "input memory type in a form of FourCC code")
        .def_property_readonly("out_format",
                               &vpl::implementation_capabilities::vpp_memory_format::get_out_format,
                               "list of output memory types in a form of FourCC code")
        .def("__str__",
             [](const vpl::implementation_capabilities::vpp_memory_format *self, std::string) {
                 std::stringstream strs;
                 strs << *self;
                 return strs.str();
             });

    py::class_<vpl::implementation_capabilities::vpp_memory>(
        m,
        "implementation_capabilities_vpp_memory")
        .def(py::init<mfxVPPDescription::filter::memdesc>())
        .def_property_readonly("mem_type",
                               &vpl::implementation_capabilities::vpp_memory::get_mem_type,
                               "supported memory type.")
        .def_property_readonly("min_size",
                               &vpl::implementation_capabilities::vpp_memory::get_min_size,
                               "minimal supported width and height.")
        .def_property_readonly("max_size",
                               &vpl::implementation_capabilities::vpp_memory::get_max_size,
                               "maximal supported width and height.")
        .def_property_readonly("size_step",
                               &vpl::implementation_capabilities::vpp_memory::get_size_step,
                               "increment value for width and height.")
        .def_property_readonly("memory_formats",
                               &vpl::implementation_capabilities::vpp_memory::get_memory_formats,
                               "list of supported memory formats.")
        .def("__str__", [](const vpl::implementation_capabilities::vpp_memory *self, std::string) {
            std::stringstream strs;
            strs << *self;
            return strs.str();
        });

    py::class_<vpl::implementation_capabilities::decoder_profile>(
        m,
        "implementation_capabilities_decoder_profile")
        .def(py::init<mfxDecoderDescription::decoder::decprofile>())
        .def_property_readonly("profile",
                               &vpl::implementation_capabilities::decoder_profile::get_profile,
                               "ID of the profile.")
        .def_property_readonly(
            "decoder_mem_types",
            &vpl::implementation_capabilities::decoder_profile::get_decoder_mem_types,
            "list of memory types.");

    py::class_<vpl::implementation_capabilities::encoder_profile>(
        m,
        "implementation_capabilities_encoder_profile")
        .def(py::init<mfxEncoderDescription::encoder::encprofile>())
        .def_property_readonly("profile",
                               &vpl::implementation_capabilities::encoder_profile::get_profile,
                               "ID of the profile.")
        .def_property_readonly(
            "encoder_mem_types",
            &vpl::implementation_capabilities::encoder_profile::get_encoder_mem_types,
            "list of memory types.");

    py::class_<vpl::implementation_capabilities::decoder>(m, "implementation_capabilities_decoder")
        .def(py::init<mfxDecoderDescription::decoder>())
        .def_property_readonly("codec_id",
                               &vpl::implementation_capabilities::decoder::get_codec_id,
                               "Codec ID.")
        .def_property_readonly("max_codec_level",
                               &vpl::implementation_capabilities::decoder::get_max_codec_level,
                               "maximum supported codec's level.")
        .def_property_readonly("profiles",
                               &vpl::implementation_capabilities::decoder::get_profiles,
                               "list of profiles.")
        .def("__str__", [](const vpl::implementation_capabilities::decoder *self, std::string) {
            std::stringstream strs;
            strs << *self;
            return strs.str();
        });

    py::class_<vpl::implementation_capabilities::encoder>(m, "implementation_capabilities_encoder")
        .def(py::init<mfxEncoderDescription::encoder>())
        .def_property_readonly("codec_id",
                               &vpl::implementation_capabilities::encoder::get_codec_id,
                               "Codec ID.")
        .def_property_readonly("max_codec_level",
                               &vpl::implementation_capabilities::encoder::get_max_codec_level,
                               "maximum supported codec's level.")
        .def_property_readonly(
            "bidirectional_prediction_support",
            &vpl::implementation_capabilities::encoder::get_bidirectional_prediction_support,
            "Provides information about bidirectional prediction support.")
        .def_property_readonly("profiles",
                               &vpl::implementation_capabilities::encoder::get_profiles,
                               "list of profiles.")
        .def("__str__", [](const vpl::implementation_capabilities::encoder *self, std::string) {
            std::stringstream strs;
            strs << *self;
            return strs.str();
        });

    py::class_<vpl::implementation_capabilities::vpp_filter>(
        m,
        "implementation_capabilities_vpp_filter")
        .def(py::init<mfxVPPDescription::filter>())
        .def_property_readonly("filter_id",
                               &vpl::implementation_capabilities::vpp_filter::get_filter_id,
                               "VPP filter ID.")
        .def_property_readonly(
            "max_delay_in_frames",
            &vpl::implementation_capabilities::vpp_filter::get_max_delay_in_frames,
            "maximum introduced delay by this filter.")
        .def_property_readonly("memory_types",
                               &vpl::implementation_capabilities::vpp_filter::get_memory_types,
                               "list of supported memory types.")
        .def("__str__", [](const vpl::implementation_capabilities::vpp_filter *self, std::string) {
            std::stringstream strs;
            strs << *self;
            return strs.str();
        });

    py::class_<vpl::implementation_capabilities_factory>(m, "implementation_capabilities_factory")
        .def(py::init<>())
        .def("create",
             &vpl::implementation_capabilities_factory::create,
             "Creates instance of implementation capabilities report class based on the format ID");
}