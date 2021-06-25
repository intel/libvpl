//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/property_name.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

void init_property_name(const py::module &m) {
    py::class_<vpl::property_name>(m, "property_name")
        .def(py::init<>())
        .def(py::init<const char *>())
        .def_property_readonly(
            "name",
            &vpl::property_name::get_name,
            "Creates std::string object with the name, where all given subnames are separated by `.`")
        .def("__add__", [](vpl::property_name &p, const char *n) {
            return p.operator/(n);
        });
}