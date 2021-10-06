//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/options.hpp"
#include "vpl/preview/defs.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

#define DPROP(NAME, ...)                                                                     \
    py::class_<vpl::dprops::NAME, vpl::property, std::shared_ptr<vpl::dprops::NAME>>(dprops, \
                                                                                     #NAME)  \
        .def(py::init<__VA_ARGS__>())

#define DCONT(NAME)                                                                            \
    py::class_<vpl::dprops::NAME, vpl::dprops::container, std::shared_ptr<vpl::dprops::NAME>>( \
        dprops,                                                                                \
        #NAME)                                                                                 \
        .def(py::init<>([](std::vector<vpl::property *> props) {                               \
            std::vector<vpl::property> list;                                                   \
            for (auto prop : props) {                                                          \
                list.push_back(*prop);                                                         \
            }                                                                                  \
            return new vpl::dprops::NAME(list);                                                \
        }));

void init_options(py::module &m) { //NOLINT
    py::class_<vpl::property, std::shared_ptr<vpl::property>>(m, "property")
        .def(py::init<vpl::property const &>())
        .def_property_readonly("leaf_property_name",
                               &vpl::property::get_leaf_property_name,
                               "Return name of the child class.")
        .def_property_readonly("properties",
                               &vpl::property::get_properties,
                               "Returns list of  properties in a form of pair: property path");

    auto dprops = m.def_submodule("dprops", "Properties");

    DPROP(impl, vpl::implementation_type);
    DPROP(acceleration_mode, vpl::implementation_via);
    DPROP(api_version, uint16_t, uint16_t);
    DPROP(impl_name, std::string_view);
    DPROP(license, std::string);
    DPROP(keywords, std::string);
    DPROP(vendor_id, uint32_t);
    DPROP(vendor_impl_id, uint32_t);
    DPROP(codec_id, vpl::codec_format_fourcc);
    DPROP(max_codec_level, uint32_t);
    DPROP(bidirectional_prediction, uint16_t);
    DPROP(filter_id, uint16_t);
    DPROP(max_delay_in_frames, uint16_t);
    DPROP(device_id, std::string);
    DPROP(media_adapter, vpl::media_adapter_type);
    DPROP(dxgi_adapter_index, uint32_t);
    DPROP(implemented_function, std::string);
    DPROP(pool_alloc_properties, vpl::pool_alloction_policy);
    DPROP(set_handle, vpl::handle_type, void *);
    DPROP(mem_type, vpl::resource_type);
    DPROP(frame_size, uint32_t, uint32_t);
    DPROP(color_format, vpl::color_format_fourcc);
    DPROP(in_color_format, vpl::color_format_fourcc);
    DPROP(out_color_format, vpl::color_format_fourcc);
    DPROP(profile, uint32_t);

    py::class_<vpl::dprops::container, vpl::property, std::shared_ptr<vpl::dprops::container>>(
        dprops,
        "container");

    DCONT(decoder);
    DCONT(encoder);
    DCONT(filter);
    DCONT(device);
    DCONT(memdesc);
    DCONT(dec_profile);
    DCONT(enc_profile);
    DCONT(dec_mem_desc);
    DCONT(enc_mem_desc);

    py::class_<vpl::property_list, std::shared_ptr<vpl::property_list>>(m, "property_list")
        .def(py::init<>([](std::vector<vpl::property *> props) {
            std::vector<vpl::property> list;
            for (auto prop : props) {
                list.push_back(*prop);
            }
            return new vpl::property_list(list);
        }));
}