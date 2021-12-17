//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/impl_selector.hpp"
#include "vpl/preview/option_tree.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

void init_impl_selector(const py::module &m) {
    // Note: we are handling a pointer to the base class. All instances of default_selector
    // simply have alternate constructors.
    py::class_<vpl::implementation_selector, std::shared_ptr<vpl::implementation_selector>>(
        m,
        "default_selector")
        .def(py::init<>([]() {
            return new vpl::default_selector<vpl::property_list>();
        }))
        .def(py::init<>([](vpl::property_list &props) {
            return new vpl::default_selector<vpl::property_list>(props);
        }))
        .def(py::init<>([](vpl::properties &props) {
            return new vpl::default_selector<vpl::properties>(props);
        }))
        .def(py::init<>([](std::vector<vpl::property *> props) {
            std::vector<vpl::property> list;
            for (auto prop : props) {
                list.push_back(*prop);
            }
            return new vpl::default_selector<vpl::property_list>(list);
        }));

    py::class_<vpl::cpu_selector, vpl::implementation_selector, std::shared_ptr<vpl::cpu_selector>>(
        m,
        "cpu_selector")
        .def(py::init<>());

    py::class_<vpl::gpu_selector, vpl::implementation_selector, std::shared_ptr<vpl::gpu_selector>>(
        m,
        "gpu_selector")
        .def(py::init<>());
}