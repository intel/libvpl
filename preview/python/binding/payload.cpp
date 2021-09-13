//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/payload.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

void init_payload(const py::module &m) {
    py::class_<vpl::payload, std::shared_ptr<vpl::payload>>(m, "payload")
        .def(py::init<std::vector<uint8_t>, uint32_t>())
        .def(py::init<std::vector<uint8_t>, uint32_t, uint16_t>())
        .def(py::init<std::vector<uint8_t>, uint32_t, uint16_t, bool>())
        .def_property_readonly("codec_id", &vpl::payload::get_codec_id, "Codec FourCC code")
        .def_property_readonly("type", &vpl::payload::get_type, "SEI message type")
        .def("is_suffix",
             &vpl::payload::is_suffix,
             "boolean suffix flag. Only enabled for HEVC codec")
        .def_property_readonly("num_bits",
                               &vpl::payload::get_num_bits,
                               "Number of bits in the payload")
        .def_property_readonly("payload_data",
                               &vpl::payload::get_payload_data,
                               "Container with payload data")
        .def_property_readonly("raw_ptr", &vpl::payload::get_raw_ptr, "Pointer to the raw data");
}