//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/session.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

template <typename VideoParams, typename InitList, typename ResetList>
class session_template {
public:
    using Class   = vpl::session<VideoParams, InitList, ResetList>;
    using PyClass = py::class_<Class, std::shared_ptr<Class>>;
    PyClass pyclass;
    session_template(const py::module &m, const std::string &typestr)
            : pyclass(m, typestr.c_str()) {
        pyclass.def("Caps", &Class::Caps)
            .def(
                "Verify",
                &Class::Verify,
                "Verifies that implementation supports such capabilities. On output, corrected capabilities are returned.")
            .def("Init", &Class::Init, "Initializes the session by using provided parameters.")
            .def("Reset", &Class::Reset, "Resets the session by using provided parameters.")
            .def("working_params", &Class::working_params, "Retrieves current session parameters.")
            .def_property_readonly("component_domain",
                                   &Class::get_component_domain,
                                   "Session's domain.")
            .def_property_readonly(
                "implementation",
                [](Class *self) {
                    auto impl = self->get_implementation();
                    return std::pair((vpl::implementation)(impl & 0x00FF),
                                     (vpl::implementation_via)(impl & 0xFF00));
                },
                "Implementation")
            .def_property_readonly("version", &Class::get_version, "Returns version");
    }
};

template <typename Reader>
class decode_session_template {
public:
    using Base    = vpl::session<vpl::decoder_video_param,
                              vpl::decoder_init_reset_list,
                              vpl::decoder_init_reset_list>;
    using Class   = vpl::decode_session<Reader>;
    using PyClass = py::class_<Class, Base, std::shared_ptr<Class>>;
    PyClass pyclass;
    decode_session_template(const py::module &m, const std::string &typestr)
            : pyclass(m, typestr.c_str()) {
        pyclass
            .def(py::init<vpl::implemetation_selector &,
                          const vpl::decoder_video_param &,
                          Reader *>())
            .def(py::init<vpl::implemetation_selector &, vpl::codec_format_fourcc, Reader *>())
            .def(py::init<vpl::implemetation_selector &,
                          const vpl::decoder_video_param &,
                          Reader *,
                          vpl::decoder_process_list &>())
            .def(py::init<vpl::implemetation_selector &,
                          vpl::codec_format_fourcc,
                          Reader *,
                          vpl::decoder_process_list &>())
            .def(
                "init_by_header",
                &Class::init_by_header,
                "Initialize the session by using bitream portion. This step can be omitted if the codec ID is known or we don't need to get SSP or PPS data from the bitstream.")
            .def("decode_frame", &Class::decode_frame, "Decodes frame")
            .def("process", &Class::process, "Decodes frame")
            .def_property_readonly("Stat", &Class::getStat, "Retrieve decoder statistic")
            .def_property_readonly("Params", &Class::getParams, "Get video params")
            .def("__iter__",
                 [](Class *self) -> Class & {
                     return *self;
                 })
            .def("__next__", [](Class *self) {
                bool is_stillgoing = true;
                while (is_stillgoing == true) {
                    std::shared_ptr<vpl::frame_surface> dec_surface_out =
                        std::make_shared<vpl::frame_surface>();
                    vpl::status ret = self->decode_frame(dec_surface_out);
                    vpl::async_op_status st;
                    switch (ret) {
                        case vpl::status::Ok:
                            do {
                                std::chrono::duration<int, std::milli> waitduration(100);
                                st = dec_surface_out->wait_for(waitduration);
                                if (vpl::async_op_status::ready == st) {
                                    return dec_surface_out;
                                }
                            } while (st == vpl::async_op_status::timeout);
                            break;
                        case vpl::status::EndOfStreamReached:
                            is_stillgoing = false;
                            break;
                        case vpl::status::NotEnoughData:
                            break;
                        case vpl::status::DeviceBusy:
                            break;
                        default:
                            is_stillgoing = false;
                            break;
                    }
                }
                throw py::stop_iteration();
            });
    }
};

void init_session(const py::module &m) {
    session_template<vpl::decoder_video_param,
                     vpl::decoder_init_reset_list,
                     vpl::decoder_init_reset_list>(m, "decode_session_base");

    decode_session_template<vpl::bitstream_source_reader>(m, "decode_session");

    session_template<vpl::encoder_video_param, vpl::encoder_init_list, vpl::encoder_reset_list>(
        m,
        "encode_session_base");

    py::class_<
        vpl::encode_session,
        vpl::session<vpl::encoder_video_param, vpl::encoder_init_list, vpl::encoder_reset_list>,
        std::shared_ptr<vpl::encode_session>>(m, "encode_session")
        .def(py::init<vpl::implemetation_selector &>())
        .def(py::init<vpl::implemetation_selector &, vpl::frame_source_reader *>())
        .def("alloc_input",
             &vpl::encode_session::alloc_input,
             "Allocate and return shared pointer to the surface")
        //.def("sync", &vpl::encode_session::sync)
        .def("encode_frame",
             py::overload_cast<std::shared_ptr<vpl::frame_surface>,
                               std::shared_ptr<vpl::bitstream_as_dst>,
                               vpl::encoder_process_list>(&vpl::encode_session::encode_frame),
             "Encodes frame")
        .def("encode_frame",
             py::overload_cast<std::shared_ptr<vpl::bitstream_as_dst>, vpl::encoder_process_list>(
                 &vpl::encode_session::encode_frame),
             "Encodes frame by using provided source reader to get data to encode")
        .def(
            "process",
            &vpl::encode_session::process,
            "Encode frame. Function returns the future object with the bitstream which will hold processed data. User needs to sync up the future object before accessing.")
        .def_property_readonly("Stat", &vpl::encode_session::getStat, "Retrieve encoder statistic")
        .def("__iter__",
             [](vpl::encode_session *self) -> vpl::encode_session & {
                 return *self;
             })
        .def("__next__", [](vpl::encode_session *self) -> std::shared_ptr<vpl::bitstream_as_dst> {
            std::shared_ptr<vpl::bitstream_as_dst> bits = std::make_shared<vpl::bitstream_as_dst>();
            while (true) {
                vpl::status wrn = vpl::status::Ok;
                wrn             = self->encode_frame(bits);
                switch (wrn) {
                    case vpl::status::Ok: {
                        std::chrono::duration<int, std::milli> waitduration(100);
                        bits->wait_for(waitduration);
                        return bits;
                    } break;
                    case vpl::status::EndOfStreamReached:
                        throw py::stop_iteration();
                    case vpl::status::DeviceBusy:
                        continue;
                    default:
                        throw py::stop_iteration();
                }
            }
        });

    session_template<vpl::vpp_video_param, vpl::vpp_init_reset_list, vpl::vpp_init_reset_list>(
        m,
        "vpp_session_base");

    py::class_<
        vpl::vpp_session,
        vpl::session<vpl::vpp_video_param, vpl::vpp_init_reset_list, vpl::vpp_init_reset_list>,
        std::shared_ptr<vpl::vpp_session>>(m, "vpp_session")
        .def(py::init<vpl::implemetation_selector &>())
        .def(py::init<vpl::implemetation_selector &, vpl::frame_source_reader *>())
        .def("alloc_input",
             &vpl::vpp_session::alloc_input,
             "Allocate and return shared pointer to the surface")
        .def("Init",
             &vpl::vpp_session::Init,
             "Initializes session with given parameters and extention buffers.")
        //.def("sync", &vpl::vpp_session::sync)
        .def(
            "process_frame",
            py::overload_cast<std::shared_ptr<vpl::frame_surface>,
                              std::shared_ptr<vpl::frame_surface> &>(
                &vpl::vpp_session::process_frame),
            "Process frame. Function returns the surface which will hold processed data. User need to sync up the surface data before accessing.")
        .def(
            "process_frame",
            py::overload_cast<std::shared_ptr<vpl::frame_surface> &>(
                &vpl::vpp_session::process_frame),
            "Process frame. Function returns the surface which will hold processed data. User need to sync up the surface data before accessing.")
        .def(
            "process",
            &vpl::vpp_session::process,
            "Process frame. Function returns the future object with the surface which will hold processed data. User need to sync up the future object before accessing.")
        .def_property_readonly("Stat", &vpl::vpp_session::getStat, "Retrieve vpp statistic")
        .def("__iter__",
             [](vpl::vpp_session *self) -> vpl::vpp_session & {
                 return *self;
             })
        .def("__next__", [](vpl::vpp_session *self) -> std::shared_ptr<vpl::frame_surface> {
            std::shared_ptr<vpl::frame_surface> proc_surface_out =
                std::make_shared<vpl::frame_surface>();
            oneapi::vpl::status wrn = oneapi::vpl::status::Ok;
            bool is_stillgoing      = true;
            while (is_stillgoing == true) {
                wrn = self->process_frame(proc_surface_out);
                switch (wrn) {
                    case oneapi::vpl::status::Ok: {
                        oneapi::vpl::async_op_status st;
                        do {
                            std::chrono::duration<int, std::milli> waitduration(100);
                            st = proc_surface_out->wait_for(waitduration);
                            if (oneapi::vpl::async_op_status::ready == st) {
                                return proc_surface_out;
                            }
                        } while (st == oneapi::vpl::async_op_status::timeout);
                    } break;
                    case oneapi::vpl::status::NotEnoughBuffer:
                        break;
                    case oneapi::vpl::status::NotEnoughData:
                        throw py::stop_iteration();
                    case oneapi::vpl::status::DeviceBusy:
                        break;
                    default:
                        throw py::stop_iteration();
                }
            }
            throw py::stop_iteration();
        });
}