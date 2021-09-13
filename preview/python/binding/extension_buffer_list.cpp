//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/extension_buffer_list.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

void init_extension_buffer_list(const py::module &m) {
    py::class_<vpl::buffer_list, std::shared_ptr<vpl::buffer_list>>(m, "buffer_list")
        .def(py::init<>())
        .def_property_readonly(
            "buffers",
            &vpl::buffer_list::get_buffers,
            "reference to the map with extension buffers. Key is buffer ID in the form of FourCC codes. Value is the pointer to the extension buffer.")
        .def_property_readonly("size",
                               &vpl::buffer_list::get_size,
                               "number of extension buffers in the map.")
        .def("add_buffer",
             &vpl::buffer_list::add_buffer,
             "adds extension buffer pointer to the map")
        .def(
            "has_buffer",
            [](vpl::buffer_list *self, uint32_t ID) {
                return self->has_buffer(ID);
            },
            "check if list has buffer")
        .def("get_buffer",
             &vpl::buffer_list::get_buffer<vpl::extension_buffer_base>,
             "get buffer from list");

    py::class_<vpl::decoder_init_reset_list,
               vpl::buffer_list,
               std::shared_ptr<vpl::decoder_init_reset_list>>(m, "decoder_init_reset_list")
        .def(py::init<>())
        .def(py::init<std::vector<vpl::extension_buffer_base *>>())
        .def("add_buffer",
             &vpl::decoder_init_reset_list::add_buffer,
             "adds extension buffer pointer to the map");

    py::class_<vpl::decoder_init_header_list,
               vpl::buffer_list,
               std::shared_ptr<vpl::decoder_init_header_list>>(m, "decoder_init_header_list")
        .def(py::init<>())
        .def(py::init<std::vector<vpl::extension_buffer_base *>>())
        .def("add_buffer",
             &vpl::decoder_init_header_list::add_buffer,
             "adds extension buffer pointer to the map");

    py::class_<vpl::decoder_process_list,
               vpl::buffer_list,
               std::shared_ptr<vpl::decoder_process_list>>(m, "decoder_process_list")
        .def(py::init<>())
        .def(py::init<std::vector<vpl::extension_buffer_base *>>())
        .def("add_buffer",
             &vpl::decoder_process_list::add_buffer,
             "adds extension buffer pointer to the map");

    py::class_<vpl::encoder_init_list, vpl::buffer_list, std::shared_ptr<vpl::encoder_init_list>>(
        m,
        "encoder_init_list")
        .def(py::init<>())
        .def(py::init<std::vector<vpl::extension_buffer_base *>>())
        .def("add_buffer",
             &vpl::encoder_init_list::add_buffer,
             "adds extension buffer pointer to the map");

    py::class_<vpl::encoder_reset_list, vpl::buffer_list, std::shared_ptr<vpl::encoder_reset_list>>(
        m,
        "encoder_reset_list")
        .def(py::init<>())
        .def(py::init<std::vector<vpl::extension_buffer_base *>>())
        .def("add_buffer",
             &vpl::encoder_reset_list::add_buffer,
             "adds extension buffer pointer to the map");

    py::class_<vpl::encoder_process_list,
               vpl::buffer_list,
               std::shared_ptr<vpl::encoder_process_list>>(m, "encoder_process_list")
        .def(py::init<>())
        .def(py::init<std::vector<vpl::extension_buffer_base *>>())
        .def("add_buffer",
             &vpl::encoder_process_list::add_buffer,
             "adds extension buffer pointer to the map");

    py::class_<vpl::vpp_init_reset_list,
               vpl::buffer_list,
               std::shared_ptr<vpl::vpp_init_reset_list>>(m, "vpp_init_reset_list")
        .def(py::init<>())
        .def(py::init<std::vector<vpl::extension_buffer_base *>>())
        .def("add_buffer",
             &vpl::vpp_init_reset_list::add_buffer,
             "adds extension buffer pointer to the map");

    py::class_<vpl::vpp_process_list, vpl::buffer_list, std::shared_ptr<vpl::vpp_process_list>>(
        m,
        "vpp_process_list")
        .def(py::init<>())
        .def(py::init<std::vector<vpl::extension_buffer_base *>>())
        .def("add_buffer",
             &vpl::vpp_process_list::add_buffer,
             "adds extension buffer pointer to the map");
}