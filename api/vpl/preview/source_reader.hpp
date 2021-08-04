/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <cstring>
#include <fstream>
#include <memory>
#include <string>

#include "vpl/preview/bitstream.hpp"
#include "vpl/preview/defs.hpp"
#include "vpl/preview/frame_surface.hpp"

namespace oneapi {
namespace vpl {

/// @brief Interface for the source data reader
/// @todo add externally requested reset and reposition
class source_reader {
public:
    /// @brief Default ctor
    source_reader() {}

    /// @brief Default dtor
    virtual ~source_reader() {}

    /// @brief Checks and retrieve end of stream status
    /// @return True if EOS reached
    virtual bool is_EOS() const = 0;
};

/// @brief Interface for the frame source data reader
class frame_source_reader : public source_reader {
public:
    /// @brief Default ctor
    frame_source_reader() {}

    /// @brief Default dtor
    virtual ~frame_source_reader() {}

    /// @brief Read and store portion of data into the @p bitstream object
    /// @param[out] frame data storage
    /// @return True if data was read
    virtual bool get_data(std::shared_ptr<frame_surface> frame) = 0;
};

/// @brief File based reader of uncomressed frames
class raw_frame_file_reader : public frame_source_reader {
public:
    /// @brief Default ctor
    /// @param[in] width Width of the frames.
    /// @param[in] heigth Heigh of the frames.
    /// @param[in] format Color format of the frames.
    /// @param[in] ifl Input stream to read from.
    raw_frame_file_reader(uint16_t width,
                          uint16_t heigth,
                          color_format_fourcc format,
                          std::ifstream& ifl)
            : frame_source_reader(),
              width_(width),
              heigth_(heigth),
              format_(format),
              ifl_(ifl),
              eof_(false) {}

    /// @brief Default dtor
    virtual ~raw_frame_file_reader() {}

    /// @brief Read and store portion of data into the @p bitstream object
    /// @param[out] frame data storage
    /// @return True if data was read
    virtual bool get_data(std::shared_ptr<frame_surface> frame) {
        auto data = frame->map_data(memory_access::write);

        /// @todo verify buffer availability

        uint32_t pitch = 0;
        switch (format_) {
            case oneapi::vpl::color_format_fourcc::i420: {
                pitch = data.get_pitch();
                auto [Y, U, V] = data.get_plane_ptrs_3();

                // read luminance plane (Y)
                read_blob(Y, pitch, width_, heigth_);

                // read chrominance (U, V)
                read_blob(U, pitch / 2, width_ / 2, heigth_ / 2);
                read_blob(V, pitch / 2, width_ / 2, heigth_ / 2);
                break;
            }
            case oneapi::vpl::color_format_fourcc::nv12: {
                pitch = data.get_pitch();
                auto [Y, UV]   = data.get_plane_ptrs_2();

                // read luminance plane (Y)
                read_blob(Y, pitch, width_, heigth_);

                // read chrominance (UV)
                read_blob(UV, pitch, width_, heigth_ / 2);
                break;
            }
            case oneapi::vpl::color_format_fourcc::bgra: {
                pitch    = data.get_pitch();
                auto B = data.get_plane_ptrs_1_BGRA();

                read_blob(B, pitch, width_ * 4, heigth_);
            }
            default:
                throw base_exception("raw_frame_file_reader unsupported format",
                                     MFX_ERR_NOT_IMPLEMENTED);
        }
        frame->unmap();
        return !eof_;
    }

    /// @brief Checks and retrieve end of stream status
    /// @return True if EOS reached
    bool is_EOS() const {
        return eof_;
    }

protected:
    /// @brief Read continuous chunk of data from the stream
    /// @param[in] ptr Pointer to the buffer to store the data
    /// @param[in] pitch Pitch in the buffer
    /// @param[in] b_width Width of the blob
    /// @param[in] b_height Height of the blob
    void read_blob(uint8_t* ptr, uint32_t pitch, uint16_t b_width, uint16_t b_height) {
        for (uint16_t i = 0; i < b_height; i++) {
            ifl_.read(reinterpret_cast<char*>(ptr + i * pitch), b_width);
            if (ifl_.gcount() != b_width)
                eof_ = true;
        }
    }
    /// @brief Width of frame.
    uint16_t width_;
    /// @brief Height of frame.
    uint16_t heigth_;
    /// @brief Color format of frame.
    color_format_fourcc format_;
    /// @brief File handle.
    std::ifstream& ifl_;
    /// @brief End of stream flag.
    bool eof_;
};

/// @brief File based reder of uncomressed frames
class raw_frame_file_reader_by_name : public frame_source_reader {
public:
    /// @brief Default ctor
    /// @param[in] width Width of the frames.
    /// @param[in] heigth Heigh of the frames.
    /// @param[in] format Color format of the frames.
    /// @param[in] ifl Input stream to read from.
    raw_frame_file_reader_by_name(uint16_t width,
                                  uint16_t heigth,
                                  color_format_fourcc format,
                                  const std::string& name)
            : frame_source_reader(),
              width_(width),
              heigth_(heigth),
              format_(format),
              eof_(false) {
        if_.open(name, std::ios_base::in | std::ios_base::binary);
        if (!if_) {
            throw file_exception(std::string("Couldn't open ") + name);
        }
        if (if_.fail()) {
            throw file_exception(std::string("Error opening ") + name);
        }
    }

    /// @brief Default dtor
    virtual ~raw_frame_file_reader_by_name() {}

    /// @brief Read and store portion of data into the @p bitstream object
    /// @param[out] frame data storage
    /// @return True if data was read
    virtual bool get_data(std::shared_ptr<frame_surface> frame) {
        auto data = frame->map_data(memory_access::write);

        /// @todo verify buffer avialability
        uint32_t pitch = 0;
        switch (format_) {
            case oneapi::vpl::color_format_fourcc::i420: {
                pitch = data.get_pitch();
                auto [Y, U, V] = data.get_plane_ptrs_3();

                // read luminance plane (Y)
                read_blob(Y, pitch, width_, heigth_);

                // read chrominance (U, V)
                read_blob(U, pitch / 2, width_ / 2, heigth_ / 2);
                read_blob(V, pitch / 2, width_ / 2, heigth_ / 2);
                break;
            }
            case oneapi::vpl::color_format_fourcc::nv12: {
                pitch = data.get_pitch();
                auto [Y, UV]   = data.get_plane_ptrs_2();

                // read luminance plane (Y)
                read_blob(Y, pitch, width_, heigth_);

                // read chrominance (UV)
                read_blob(UV, pitch, width_, heigth_ / 2);
                break;
            }
            case oneapi::vpl::color_format_fourcc::bgra: {
                pitch    = data.get_pitch();
                auto B = data.get_plane_ptrs_1_BGRA();

                read_blob(B, pitch, width_ * 4, heigth_);
            }
            default:
                throw base_exception("raw_frame_file_reader_by_name unsupported format",
                                     MFX_ERR_NOT_IMPLEMENTED);
        }
        frame->unmap();
        return !eof_;
    }

    /// @brief Checks and retrieve end of stream status
    /// @return True if EOS reached
    bool is_EOS() const {
        return eof_;
    }

protected:
    /// @brief Reads continiouse chunk of data from the stream
    /// @param[in] ptr Pointer to the buffere to store the data
    /// @param[in] pitch Pitch in the buffer
    /// @param[in] b_width Width of the blob
    /// @param[in] b_height Height of the blob
    void read_blob(uint8_t* ptr, uint32_t pitch, uint16_t b_width, uint16_t b_height) {
        for (uint16_t i = 0; i < b_height; i++) {
            if_.read(reinterpret_cast<char*>(ptr + i * pitch), b_width);
            if (if_.gcount() != b_width)
                eof_ = true;
        }
    }
    /// @brief Width of frame.
    uint16_t width_;
    /// @brief Height of frame.
    uint16_t heigth_;
    /// @brief Color format of frame.
    color_format_fourcc format_;
    /// @brief File handle
    std::ifstream if_;
    /// @brief End of stream flag.
    bool eof_;
};

/// @brief Interface for the bitstream source data reader
class bitstream_source_reader : public source_reader {
public:
    /// @brief Default ctor
    bitstream_source_reader() {}

    /// @brief Default dtor
    virtual ~bitstream_source_reader() {}

    /// @brief Read and store portion of data into the @p bitstream object
    /// @param[out] bits data storage
    /// @return True if data was read
    virtual bool get_data(bitstream_as_src* bits) = 0;
};

/// @brief File based source data reader
class bitstream_file_reader : public bitstream_source_reader {
public:
    /// @brief Constructs reader with given file handle
    /// @param[in] ifl Reference to the file handle
    explicit bitstream_file_reader(std::ifstream& ifl) : bitstream_source_reader(), if_(ifl) {}

    /// @brief Read and store portion of data into the @p bitstream object
    /// @param[out] bits data storage
    /// @return True if data was read
    bool get_data(bitstream_as_src* bits) {
        auto lambda = [&](uint8_t* ptr, uint32_t max, bool& eos) {
            if_.read(reinterpret_cast<char*>(ptr), max);
            if (if_.eof())
                eos = true;
            return (uint32_t)if_.gcount();
        };
        bits->pull_in(lambda);
        return true;
    }

    /// @brief Checks and retrieve end of stream status
    /// @return True if EOS reached
    bool is_EOS() const {
        return if_.eof();
    }

protected:
    /// @brief File handle
    std::ifstream& if_;
};

class bitstream_file_reader_name : public bitstream_source_reader {
public:
    /// @brief Constructs reader with given file handle
    /// @param[in] ifl Reference to the file handle
    explicit bitstream_file_reader_name(std::string& name) : bitstream_source_reader() {
        if_.open(name, std::ios_base::in | std::ios_base::binary);
        if (!if_) {
            throw file_exception(std::string("Couldn't open ") + name);
        }
        if (if_.fail()) {
            throw file_exception(std::string("Error opening ") + name);
        }
    }

    /// @brief Read and store portion of data into the @p bitstream object
    /// @param[out] bits data storage
    /// @return True if data was read
    bool get_data(bitstream_as_src* bits) {
        auto lambda = [&](uint8_t* ptr, uint32_t max, bool& eos) {
            if_.read(reinterpret_cast<char*>(ptr), max);
            if (if_.eof())
                eos = true;
            return (uint32_t)if_.gcount();
        };
        bits->pull_in(lambda);
        return true;
    }

    /// @brief Checks and retrieve end of stream status
    /// @return True if EOS reached
    bool is_EOS() const {
        return if_.eof();
    }

protected:
    /// @brief File handle
    std::ifstream if_;
};

} // namespace vpl
} // namespace oneapi
