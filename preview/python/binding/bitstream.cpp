//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/bitstream.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

void init_bitstream(const py::module &m) {
    class bitstream_data {
    public:
        bitstream_data(void *base,
                       py::ssize_t item_size,
                       std::string format,
                       py::ssize_t len,
                       py::ssize_t sample_pitch)
                : base(base),
                  item_size(item_size),
                  format(format),
                  len(len),
                  sample_pitch(sample_pitch) {}
        explicit bitstream_data(const bitstream_data &other)
                : base(other.base),
                  item_size(other.item_size),
                  format(other.format),
                  len(other.len),
                  sample_pitch(other.sample_pitch) {}
        py::buffer_info buffer_info() {
            return py::buffer_info(base, item_size, format, 1, { len }, { sample_pitch });
        }

    private:
        void *base;
        py::ssize_t item_size;
        std::string format;
        py::ssize_t len;
        py::ssize_t sample_pitch;
    };

    py::class_<bitstream_data>(m, "bitstream_data", py::buffer_protocol())
        .def_buffer(&bitstream_data::buffer_info);

    auto bitstream =
        py::class_<vpl::bitstream>(m, "bitstream", py::buffer_protocol())
            .def(py::init<>())
            .def(py::init<vpl::codec_format_fourcc>())
            .def(py::init<vpl::codec_format_fourcc, uint32_t>())
            .def(
                "realloc",
                &vpl::bitstream::realloc,
                "Reallocs internal buffer with the given buffer size increase value. Valid data is copied into new buffer")
            .def_property(
                "CodecId",
                py::cpp_function(&vpl::bitstream::get_CodecId, py::return_value_policy::copy),
                py::cpp_function(&vpl::bitstream::set_CodecId),
                "CodecId")
            .def_property("DecodeTimeStamp",
                          py::cpp_function(&vpl::bitstream::get_DecodeTimeStamp,
                                           py::return_value_policy::copy),
                          py::cpp_function(&vpl::bitstream::set_DecodeTimeStamp),
                          "DecodeTimeStamp")
            .def_property(
                "TimeStamp",
                py::cpp_function(&vpl::bitstream::get_TimeStamp, py::return_value_policy::copy),
                py::cpp_function(&vpl::bitstream::set_TimeStamp),
                "TimeStamp")
            .def_property(
                "PicStruct",
                py::cpp_function(&vpl::bitstream::get_PicStruct, py::return_value_policy::copy),
                py::cpp_function(&vpl::bitstream::set_PicStruct),
                "PicStruct")
            .def_property(
                "FrameType",
                py::cpp_function(&vpl::bitstream::get_FrameType, py::return_value_policy::copy),
                py::cpp_function(&vpl::bitstream::set_FrameType),
                "FrameType")
            .def_property(
                "DataFlag",
                py::cpp_function(&vpl::bitstream::get_DataFlag, py::return_value_policy::copy),
                py::cpp_function(&vpl::bitstream::set_DataFlag),
                "DataFlag")
            .def_property(
                "DataOffset",
                py::cpp_function(&vpl::bitstream::get_DataOffset, py::return_value_policy::copy),
                py::cpp_function(&vpl::bitstream::set_DataOffset),
                "DataOffset")
            .def_property(
                "DataLength",
                py::cpp_function(&vpl::bitstream::get_DataLength, py::return_value_policy::copy),
                py::cpp_function(&vpl::bitstream::set_DataLength),
                "DataLength")
            .def(
                "buffer",
                [](vpl::bitstream *self) {
                    return bitstream_data(self->get_buffer_ptr(),
                                          sizeof(uint8_t),
                                          py::format_descriptor<uint8_t>::format(),
                                          self->get_max_buffer_length(),
                                          sizeof(uint8_t));
                },
                "Get buffer")
            .def_property_readonly("max_buffer_length",
                                   &vpl::bitstream::get_max_buffer_length,
                                   "Max Buffer Length")
            .def_property_readonly(
                "valid_data",
                [](vpl::bitstream *self) {
                    auto [ptr, len] = self->get_valid_data();
                    return bitstream_data(ptr,
                                          sizeof(uint8_t),
                                          py::format_descriptor<uint8_t>::format(),
                                          len,
                                          sizeof(uint8_t));
                },
                "Valid Data")
            .def_buffer([](vpl::bitstream *self) {
                auto [ptr, len] = self->get_valid_data();
                return py::buffer_info(ptr,
                                       sizeof(uint8_t),
                                       py::format_descriptor<uint8_t>::format(),
                                       1,
                                       { len },
                                       { sizeof(uint8_t) });
            })
            .def("reset", &vpl::bitstream::reset, "Resets data in the buffer.");

    auto bitstream_as_src =
        py::class_<vpl::bitstream_as_src, vpl::bitstream>(m, "bitstream_as_src")
            .def(py::init<>())
            .def(py::init<vpl::codec_format_fourcc>())
            .def(py::init<vpl::codec_format_fourcc, uint32_t>())
            .def(
                "pull_in",
                &vpl::bitstream_as_src::pull_in,
                "Stores maximum possible portion of data in the circular buffer. Data is strored after unused portion of the buffer in the length of avialable space in the buffer.");

    auto bitstream_as_dst =
        py::class_<vpl::bitstream_as_dst, vpl::bitstream>(m,
                                                          "bitstream_as_dst",
                                                          py::buffer_protocol())
            .def(py::init<>())
            .def(py::init<vpl::codec_format_fourcc, uint32_t>())
            .def("wait",
                 &vpl::bitstream_as_dst::wait,
                 "Indefinitely waits for operation completion.")
            .def(
                "wait_for",
                [](vpl::bitstream_as_dst &s, int milliseconds) {
                    std::chrono::duration<int, std::milli> waitduration(milliseconds);
                    return (unsigned int)(s.wait_for(waitduration));
                },
                "Waits for the operation completion. Waits for the result to become available. Blocks until specified timeout_duration has elapsed or the result becomes available, whichever comes first. Returns value identifying the state of the result.");
}
