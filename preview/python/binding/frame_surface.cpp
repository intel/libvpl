//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/frame_surface.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

void init_frame_surface(const py::module &m) {
    py::class_<vpl::frame_surface, std::shared_ptr<vpl::frame_surface>>(m, "frame_surface")
        .def(py::init<>())
        .def(
            "inject",
            &vpl::frame_surface::inject,
            "Inject mfxFrameSurface1 object to take care of it. This is temporal method until VPL RT will support all functions for the internal memory allocation")
        .def("wait", &vpl::frame_surface::wait, "Indefinitely wait for operation completion.")
        .def(
            "wait_for",
            [](vpl::frame_surface &s, int milliseconds) {
                std::chrono::duration<int, std::milli> waitduration(milliseconds);
                return s.wait_for(waitduration);
            },
            "Waits for the operation completion. Waits for the result to become available. Blocks until specified timeout_duration has elapsed or the result becomes available, whichever comes first. Returns value identifying the state of the result.")
        .def_property_readonly("frame_info",
                               &vpl::frame_surface::get_frame_info,
                               "Provide frame information.")
        .def_property_readonly("frame_data",
                               &vpl::frame_surface::get_frame_data,
                               "Provide frame data information.")
        .def("map", &vpl::frame_surface::map, "Maps data to the system memory.")
        .def("unmap", &vpl::frame_surface::unmap, "Unmaps data to the system memory.")
        .def_property_readonly("native_handle",
                               &vpl::frame_surface::get_native_handle,
                               "native surface handle of the surface.")
        .def_property_readonly("device_handle",
                               &vpl::frame_surface::get_device_handle,
                               "native device handle of the surface.")
        .def_property_readonly("ref_counter",
                               &vpl::frame_surface::get_ref_counter,
                               "current reference counter value.")
        .def_property_readonly("raw_ptr",
                               &vpl::frame_surface::get_raw_ptr,
                               "pointer to the raw data.")
        // .def("associate_context", &vpl::frame_surface::associate_context)
        ;
}
