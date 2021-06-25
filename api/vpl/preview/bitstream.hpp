/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <algorithm>
#include <chrono>
#include <functional>
#include <utility>

#include "vpl/preview/defs.hpp"
#include "vpl/preview/exception.hpp"

#include "vpl/preview/detail/sdk_callable.hpp"
#include "vpl/preview/detail/string_helpers.hpp"
#include "vpl/mfxstructures.h"

namespace oneapi {
namespace vpl {

#define iDECLARE_MEMBER_ACCESS(type, owner, name) \
    /*! @brief Returns name value. */             \
    /*! @return name value. */                    \
    type get_##name() const {                     \
        return owner.name;                        \
    }                                             \
    /*! @brief Sets name value. */                \
    /*! @param[in] name Value. */                 \
    void set_##name(type name) {                  \
        owner.name = name;                        \
    }

/// @brief Defines the circlular buffer that holds compressed video data.
/// @todo Consider split of this class for decoder and for encoder as
/// there are different fields in use i decoder and encoder
class bitstream {
public:
    /// Default compressed video data buffer lenght
    enum buffer_len : uint32_t { DEFAULT_LENGHT = 2000000 };
    /// @brief Default ctor
    bitstream() : bits_() {
        bits_.TimeStamp       = MFX_TIMESTAMP_UNKNOWN;
        bits_.DecodeTimeStamp = MFX_TIMESTAMP_UNKNOWN;
        bits_.Data            = new uint8_t[bitstream::buffer_len::DEFAULT_LENGHT];
        bits_.MaxLength       = bitstream::buffer_len::DEFAULT_LENGHT;
    }
    /// @brief Constructs bitstream object with given codec ID and default buffer length
    /// @param[in] codecID codec's fourCC code
    explicit bitstream(codec_format_fourcc codecID) : bits_() {
        bits_.TimeStamp       = MFX_TIMESTAMP_UNKNOWN;
        bits_.DecodeTimeStamp = MFX_TIMESTAMP_UNKNOWN;
        bits_.Data            = new uint8_t[bitstream::buffer_len::DEFAULT_LENGHT];
        bits_.MaxLength       = bitstream::buffer_len::DEFAULT_LENGHT;
        bits_.CodecId         = (uint32_t)codecID;
    }
    /// @brief Constructs bitstream object with given codec ID and given buffer length
    /// @param[in] codecID codec's fourCC code
    /// @param[in] buffersize circular buffer size in bytes
    bitstream(codec_format_fourcc codecID, uint32_t buffersize) : bits_() {
        bits_.TimeStamp       = MFX_TIMESTAMP_UNKNOWN;
        bits_.DecodeTimeStamp = MFX_TIMESTAMP_UNKNOWN;
        bits_.Data            = new uint8_t[buffersize];
        bits_.MaxLength       = buffersize;
        bits_.CodecId         = (uint32_t)codecID;
    }

    /// @brief default ctor
    virtual ~bitstream() {}

    /// @brief Reallocs internal buffer with the given buffer size increase value. Valid data is copied into new buffer
    /// @param[in] bufferinc Number of bytes to increase the buffer.
    void realloc(uint32_t bufferinc = buffer_len::DEFAULT_LENGHT) {
        uint8_t* new_buffer = new uint8_t[bufferinc + bits_.MaxLength];

        if (bits_.DataOffset) {
            std::copy(bits_.Data + bits_.DataOffset,
                      bits_.Data + bits_.DataOffset + bits_.DataLength,
                      bits_.Data);
            bits_.DataOffset = 0;
        }

        std::copy(bits_.Data, bits_.Data + bits_.DataLength, new_buffer);
        delete[] bits_.Data;
        bits_.Data      = new_buffer;
        bits_.MaxLength = bufferinc + bits_.MaxLength;
    }

    /*! @brief Returns codec fourCC value. */
    /*! @return codec fourCC value. */
    codec_format_fourcc get_CodecId() const {
        return (codec_format_fourcc)bits_.CodecId;
    }
    /*! @brief Sets codec fourCC value. */
    /*! @param[in] CodecID codec fourCC. */
    void set_CodecId(codec_format_fourcc CodecID) {
        bits_.CodecId = (uint32_t)CodecID;
    }

    iDECLARE_MEMBER_ACCESS(int64_t, bits_, DecodeTimeStamp);
    iDECLARE_MEMBER_ACCESS(uint64_t, bits_, TimeStamp);
    iDECLARE_MEMBER_ACCESS(uint16_t, bits_, PicStruct);
    iDECLARE_MEMBER_ACCESS(uint16_t, bits_, FrameType);
    iDECLARE_MEMBER_ACCESS(uint16_t, bits_, DataFlag);
    iDECLARE_MEMBER_ACCESS(uint16_t, bits_, DataOffset);
    iDECLARE_MEMBER_ACCESS(uint16_t, bits_, DataLength);

    /// @brief Returns pointer to the head of internal circular buffer.
    /// @return Pointer to the head of internal circular buffer.
    uint8_t* get_buffer_ptr() const {
        return bits_.Data;
    }

    /// @brief Returns internal circular buffer in bytes.
    /// @return internal circular buffer in bytes.
    uint32_t get_max_buffer_length() const {
        return bits_.MaxLength;
    }

    /// @brief Returns pair with pointer to the first valid byte in the bitstream and
    /// length of the valid data in bytes.
    /// @return Pair with pointer to the first valid byte in the bitstream and
    /// length of the valid data in bytes.
    auto get_valid_data() {
        return std::pair(bits_.Data + bits_.DataOffset, bits_.DataLength);
    }

    /// @brief Resets data in the buffer.
    void reset() {
        bits_.DataLength = 0;
        bits_.DataOffset = 0;
        return;
    }

    /// @brief Returns pointer to the mfxBitream oneVPL's structure.
    /// @return Pointer to the mfxBitream oneVPL's structure.
    mfxBitstream* operator()() {
        return &(this->bits_);
    }

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] b Reference to the bitstream instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream& operator<<(std::ostream& out, const bitstream& b);

protected:
    /// @brief mfxBitstream structure instance.
    mfxBitstream bits_;
};

inline std::ostream& operator<<(std::ostream& out, const bitstream& b) {
    out << detail::space(detail::INTENT, out, "CodecId         = ")
        << detail::FourCC2String(b.bits_.CodecId) << std::endl;
    out << detail::space(detail::INTENT, out, "DecodeTimeStamp = ")
        << detail::TimeStamp2String(static_cast<int64_t>(b.bits_.DecodeTimeStamp)) << std::endl;
    out << detail::space(detail::INTENT, out, "TimeStamp       = ") << b.bits_.TimeStamp
        << std::endl;
    out << detail::space(detail::INTENT, out, "DataOffset      = ") << b.bits_.DataOffset
        << std::endl;
    out << detail::space(detail::INTENT, out, "DataLength      = ") << b.bits_.DataLength
        << std::endl;
    out << detail::space(detail::INTENT, out, "PicStruct       = ")
        << detail::PicStruct2String(b.bits_.PicStruct) << std::endl;
    out << detail::space(detail::INTENT, out, "FrameType       = ") << b.bits_.FrameType
        << std::endl;
    out << detail::space(detail::INTENT, out, "DataFlag        = ") << b.bits_.DataFlag
        << std::endl;

    return out;
}

/// @brief Defines the circular buffer that holds compressed video data. Used as the input to decoder.
class bitstream_as_src : public bitstream {
public:
    /// @brief Default ctor
    bitstream_as_src() : bitstream() {}
    /// @brief Constructs bitstream object with given codec ID and default buffer length
    /// @param[in] codecID codec's fourCC code
    explicit bitstream_as_src(codec_format_fourcc codecID) : bitstream(codecID) {}
    /// @brief Constructs bitstream object with given codec ID and given buffer length
    /// @param[in] codecID codec's fourCC code
    /// @param[in] buffersize circular buffer size in bytes
    bitstream_as_src(codec_format_fourcc codecID, uint32_t buffersize)
            : bitstream(codecID, buffersize) {}

    /// @brief Stores maximum possible portion of data in the circular buffer. Data is strored after
    /// unused portion of the buffer in the length of avialable space in the buffer.
    /// @param[in] reader source reader callback.
    void pull_in(std::function<uint32_t(uint8_t*, uint32_t, bool&)> reader) {
        bool eosFlag = false;
        if (bits_.DataOffset) {
            std::copy(bits_.Data + bits_.DataOffset,
                      bits_.Data + bits_.DataOffset + bits_.DataLength,
                      bits_.Data);
            bits_.DataOffset = 0;
        }
        bits_.DataLength += (uint32_t)reader(bits_.Data + bits_.DataLength,
                                             bits_.MaxLength - bits_.DataLength,
                                             eosFlag);
        // if(eosFlag) bits_.DataFlag = MFX_BITSTREAM_EOS;
    }
};

/// @brief Defines the buffer that holds compressed video data. Used as the output from encoder.
class bitstream_as_dst : public bitstream {
public:
    /// @brief Default ctor
    bitstream_as_dst() : bitstream(), sp_(nullptr), session_(nullptr), valid_(false) {}
    /// @brief Constructs bitstream object with given codec ID and given buffer length
    /// @param[in] codecID codec's fourCC code
    /// @param[in] buffersize circular buffer size in bytes
    bitstream_as_dst(codec_format_fourcc codecID, uint32_t buffersize)
            : bitstream(codecID, buffersize),
              sp_(nullptr),
              session_(nullptr),
              valid_(false) {}

    /// @brief Indefinitely waits for operation completion.
    void wait() {
        if (sp_) {
            detail::c_api_invoker(detail::default_checker,
                                  MFXVideoCORE_SyncOperation,
                                  session_,
                                  sp_,
                                  MFX_INFINITE);
            valid_ = true;
        }
    }

    /// @brief Temporal method to assotiate externally allocated surface with sync point generated
    /// by the processing function.
    /// @param[in] context Pair of session handle and sync point.
    void associate_context(std::pair<mfxSession, mfxSyncPoint> context) {
        session_ = std::get<0>(context);
        sp_      = std::get<1>(context);
    }
    /// @brief Waits for the operation completion. Waits for the result to become available. Blocks until
    /// specified timeout_duration has elapsed or the result becomes available, whichever comes first. Returns
    /// value identifying the state of the result.
    /// @param timeout_duration Maximum duration to block for.
    /// @return Wait status.
    template <class Rep, class Period>
    async_op_status wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const {
        async_op_status surf_sts;
        auto wait_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeout_duration);

        detail::c_api_invoker e({ [](mfxStatus s) {
                                    switch (s) {
                                        case MFX_ERR_NONE:
                                            return false;
                                        case MFX_WRN_IN_EXECUTION:
                                            return false;
                                        case MFX_ERR_ABORTED:
                                            return false;
                                        case MFX_ERR_UNKNOWN:
                                            return false;
                                        default:
                                            break;
                                    }

                                    bool ret = (s < 0) ? true : false;
                                    return ret;
                                } },
                                MFXVideoCORE_SyncOperation,
                                session_,
                                sp_,
                                (uint32_t)wait_ms.count());

        switch (e.sts_) {
            case MFX_ERR_NONE:
                surf_sts = async_op_status::ready;
                break;
            case MFX_WRN_IN_EXECUTION:
                surf_sts = async_op_status::timeout;
                break;
            case MFX_ERR_ABORTED:
                surf_sts = async_op_status::aborted;
                break;
            case MFX_ERR_UNKNOWN:
                surf_sts = async_op_status::unknown;
                break;
            default:
                throw base_exception(e.sts_);
        }
        return surf_sts;
    }

protected:
    /// Sync point handle
    mfxSyncPoint sp_;
    /// Session handle
    mfxSession session_;
    /// @todo Remove it nafik.
    bool valid_;
};

} // namespace vpl
} // namespace oneapi
