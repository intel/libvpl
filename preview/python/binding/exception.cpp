//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/exception.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

void init_exception(const py::module &m) {
    auto base_exception =
        py::register_exception<vpl::base_exception>(m, "base_exception", PyExc_RuntimeError);

    auto file_exception = py::register_exception<vpl::file_exception>(m, "file_exception");
}