//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/frame_pool.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

void init_frame_pool(const py::module &m) {
    py::class_<vpl::temporal_frame_allocator, std::shared_ptr<vpl::temporal_frame_allocator>>(
        m,
        "temporal_frame_allocator")
        .def(py::init<>())
        .def(
            "attach_frame_info",
            &vpl::temporal_frame_allocator::attach_frame_info,
            "Update class instance with the frame information which is used to allocate proper buffer for surface data.")
        .def("acquire", &vpl::temporal_frame_allocator::acquire, "Returns available surface.");
}