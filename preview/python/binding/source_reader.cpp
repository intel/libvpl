//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/source_reader.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

#if !defined(WIN32) && !defined(memcpy_s)
    // memcpy_s proxy to allow use safe version where supported
    #define memcpy_s(dest, destsz, src, count) memcpy(dest, src, count)
#endif

// Buffer that writes to Python instead of C++
class pythonibuf : public std::streambuf {
private:
    py::object pyreadinto;
    size_t readinto(char *s, size_t n) {
        py::bytes wrapper(s, n);
        py::object bytes_read = pyreadinto(wrapper);
        if (bytes_read.is_none()) {
            return 0;
        }
        else {
            return bytes_read.cast<size_t>();
        }
    }

    std::array<char, 1024> data;

    bool reload() {
        size_t count = readinto(&data[0], data.size());
        setg(&data[0], &data[0], &data[count]);
        return count;
    }

    std::streamsize xsgetn(char *s, std::streamsize n) {
        std::streamsize bytes_read = 0;
        while (n) {
            char *cptr = gptr();
            char *eptr = egptr();
            if (cptr == eptr && !reload()) {
                return bytes_read;
            }
            std::streamsize len = eptr - cptr;
            if (len > n) {
                len = n;
            }
            memcpy_s(s, n, cptr, len);
            gbump(static_cast<int>(len));
            n -= len;
            bytes_read += len;
        }
        return bytes_read;
    }

    int_type underflow() {
        return (gptr() == egptr() && !reload()) ? traits_type::eof() : *gptr();
    }

    int_type uflow() {
        return (gptr() == egptr() && !reload()) ? traits_type::eof() : *gptr();
    }

public:
    explicit pythonibuf(py::object pyistream, size_t buffer_size = 1024)
            : pyreadinto(pyistream.attr("readinto")) {}
};

void init_source_reader(const py::module &m) {
    py::register_exception<std::ios_base::failure>(m,
                                                   "vpl_file_exception",
                                                   PyExc_FileNotFoundError);

    py::class_<vpl::source_reader, std::shared_ptr<vpl::source_reader>>(m, "source_reader")
        .def("is_EOS", &vpl::source_reader::is_EOS, "Checks and retrieve end of stream status")
        .def("__enter__",
             [](vpl::source_reader *self) -> vpl::source_reader & {
                 return *self;
             })
        .def("__exit__",
             [](vpl::source_reader *self,
                pybind11::object exc_type,
                pybind11::object exc_value,
                pybind11::object traceback) {
             });

    py::class_<vpl::frame_source_reader,
               vpl::source_reader,
               std::shared_ptr<vpl::frame_source_reader>>(m, "frame_source_reader")
        .def_property_readonly("data",
                               &vpl::frame_source_reader::get_data,
                               "Read and store portion of data into the @p bitstream object");

    py::class_<vpl::raw_frame_file_reader,
               vpl::frame_source_reader,
               std::shared_ptr<vpl::raw_frame_file_reader>>(m, "raw_frame_file_reader")
        .def(py::init<uint16_t, uint16_t, vpl::color_format_fourcc, std::ifstream &>())
        .def_property_readonly("data",
                               &vpl::raw_frame_file_reader::get_data,
                               "Read and store portion of data into the @p bitstream object");

    py::class_<vpl::raw_frame_file_reader_by_name,
               vpl::frame_source_reader,
               std::shared_ptr<vpl::raw_frame_file_reader_by_name>>(m,
                                                                    "raw_frame_file_reader_by_name")
        .def(py::init<uint16_t, uint16_t, vpl::color_format_fourcc, std::string &>())
        .def_property_readonly("data",
                               &vpl::raw_frame_file_reader_by_name::get_data,
                               "Read and store portion of data into the @p bitstream object");

    py::class_<vpl::bitstream_source_reader,
               vpl::source_reader,
               std::shared_ptr<vpl::bitstream_source_reader>>(m, "bitstream_source_reader")
        .def_property_readonly("data",
                               &vpl::bitstream_source_reader::get_data,
                               "Read and store portion of data into the @p bitstream object");

    py::class_<vpl::bitstream_file_reader,
               vpl::bitstream_source_reader,
               std::shared_ptr<vpl::bitstream_file_reader>>(m, "bitstream_file_reader")
        // .def(py::init<std::ifstream>())
        .def_property_readonly("data",
                               &vpl::bitstream_file_reader::get_data,
                               "Read and store portion of data into the @p bitstream object");

    py::class_<vpl::bitstream_file_reader_name,
               vpl::bitstream_source_reader,
               std::shared_ptr<vpl::bitstream_file_reader_name>>(m, "bitstream_file_reader_name")
        .def(py::init<std::string &>())
        .def_property_readonly("data",
                               &vpl::bitstream_file_reader_name::get_data,
                               "Read and store portion of data into the @p bitstream object");
}