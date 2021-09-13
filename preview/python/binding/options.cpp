//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/options.hpp"
#include "vpl/preview/defs.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

void init_options(const py::module &m) {
#define DEF_PROP_TYPE(T)                                   \
    def(py::init<>([](vpl::property_name &name, T value) { \
        return new vpl::property(name, value);             \
    })).def(py::init<>([](std::string &name, T value) {    \
        return new vpl::property(name, value);             \
    }))

#define DEF_PROP_TYPE_AS(T, D)                             \
    def(py::init<>([](vpl::property_name &name, T value) { \
        return new vpl::property(name, (D)value);          \
    })).def(py::init<>([](std::string &name, T value) {    \
        return new vpl::property(name, (D)value);          \
    }))

    py::class_<vpl::property, std::shared_ptr<vpl::property>>(m, "property")
        .def(py::init<>())
        // Enum overloads must be defined before base types to ensure
        // they are tried for compatibility first.
        .DEF_PROP_TYPE_AS(vpl::implementation, uint32_t)
        .DEF_PROP_TYPE_AS(vpl::codec_format_fourcc, uint32_t)
        .DEF_PROP_TYPE_AS(vpl::color_format_fourcc, uint32_t)
        .DEF_PROP_TYPE_AS(vpl::chroma_format_idc, uint32_t)
        .DEF_PROP_TYPE_AS(vpl::target_usage, uint32_t)
        .DEF_PROP_TYPE_AS(vpl::rate_control_method, uint32_t)
        .DEF_PROP_TYPE_AS(vpl::io_pattern, uint32_t)
        .DEF_PROP_TYPE_AS(vpl::pic_struct, uint32_t)
        .DEF_PROP_TYPE_AS(vpl::memory_access, uint32_t)
        .DEF_PROP_TYPE_AS(vpl::hevc_nal_unit_type, uint32_t)
        .DEF_PROP_TYPE_AS(vpl::frame_type, uint32_t)
        .DEF_PROP_TYPE_AS(vpl::implementation, uint32_t)
        // Base type inits are defined last to ensure
        // they are tried for compatibility last.
        .DEF_PROP_TYPE(uint8_t)
        .DEF_PROP_TYPE(int8_t)
        .DEF_PROP_TYPE(uint16_t)
        .DEF_PROP_TYPE(int16_t)
        .DEF_PROP_TYPE(uint32_t)
        .DEF_PROP_TYPE(int32_t)
        .DEF_PROP_TYPE(uint64_t)
        .DEF_PROP_TYPE(int64_t)
        .DEF_PROP_TYPE(mfxF32)
        .DEF_PROP_TYPE(mfxF64)
        .def_property_readonly(
            "type",
            [](vpl::property *self) {
                return static_cast<int>(self->get_type());
            },
            "Type of the property's value")
        .def_property_readonly("name", &vpl::property::get_name, "Name of the property's value")
        .def_property_readonly("data", &vpl::property::get_data, "Property's value")
        .def("GetValue", &vpl::property::GetValue, "Property type and value");
}