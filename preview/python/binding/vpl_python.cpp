//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include <cstring>
#include <memory>
#include <sstream>
#include <vector>

#include "vpl_python.hpp"

#include "vpl/preview/vpl.hpp"

namespace vpl = oneapi::vpl;

void init_bitstream(const py::module &m);
void init_defs(const py::module &m);
void init_exception(const py::module &m);
void init_extension_buffer_list(const py::module &m);
void init_extension_buffer(const py::module &m);
void init_frame_surface(const py::module &m);
void init_future(const py::module &m);
void init_impl_caps(const py::module &m);
void init_impl_selector(const py::module &m);
void init_mfxstructures(const py::module &m);
void init_options(const py::module &m);
void init_payload(const py::module &m);
void init_property_name(const py::module &m);
void init_session(const py::module &m);
void init_source_reader(const py::module &m);
void init_stat(const py::module &m);
void init_video_param(const py::module &m);

PYBIND11_MODULE(pyvpl, m) {
    py::options options;
    options.disable_function_signatures();

    init_bitstream(m);
    init_defs(m);
    init_exception(m);
    init_extension_buffer_list(m);
    init_extension_buffer(m);
    init_frame_surface(m);
    init_future(m);
    init_impl_caps(m);
    init_impl_selector(m);
    init_mfxstructures(m);
    init_options(m);
    init_payload(m);
    init_property_name(m);
    init_session(m);
    init_source_reader(m);
    init_stat(m);
    init_video_param(m);

    py::class_<mfxVersion, std::shared_ptr<mfxVersion>>(m, "mfxVersion")
        .def_readwrite("Version", &mfxVersion::Version)
        .def_readwrite("Major", &mfxVersion::Major)
        .def_readwrite("Minor", &mfxVersion::Minor);
}
