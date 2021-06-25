//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/impl_selector.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

void init_impl_selector(const py::module &m) {
    py::class_<vpl::implemetation_selector>(m, "implemetation_selector");

    py::class_<vpl::default_selector, vpl::implemetation_selector>(m, "default_selector")
        .def(py::init<const std::vector<vpl::property> &>());

    py::class_<vpl::cpu_selector, vpl::default_selector>(m, "cpu_selector").def(py::init<>());

    py::class_<vpl::gpu_selector, vpl::default_selector>(m, "gpu_selector").def(py::init<>());
}