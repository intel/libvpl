//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/stat.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

void init_stat(const py::module &m) {
    py::class_<vpl::stat>(m, "stat")
        .def_property_readonly("num_frame",
                               &vpl::stat::get_num_frame,
                               "Retrieves number of processed frames")
        .def_property_readonly("num_cached_frame",
                               &vpl::stat::get_num_cached_frame,
                               "Retrieves number of cached frames");

    py::class_<vpl::decode_stat, vpl::stat>(m, "decode_stat")
        .def(py::init<>())
        .def_property_readonly("num_error",
                               &vpl::decode_stat::get_num_error,
                               "Retrieves number of errors recovered")
        .def_property_readonly("num_skipped_frame",
                               &vpl::decode_stat::get_num_skipped_frame,
                               "Retrieves number of skipped frames")
        .def_property_readonly("raw", &vpl::decode_stat::get_raw, "Retrieves raw data pointer");

    py::class_<vpl::encode_stat, vpl::stat>(m, "encode_stat")
        .def(py::init<>())
        .def_property_readonly("num_bit",
                               &vpl::encode_stat::get_num_bit,
                               "Retrieves number of bits for all encoded frames")
        .def_property_readonly("raw", &vpl::encode_stat::get_raw, "Retrieves raw data pointer");

    py::class_<vpl::vpp_stat, vpl::stat>(m, "vpp_stat")
        .def(py::init<>())
        .def_property_readonly("raw", &vpl::vpp_stat::get_raw, "Retrieves raw data pointer");
}