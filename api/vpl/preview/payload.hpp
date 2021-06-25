/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <algorithm>
#include <vector>

#include "vpl/preview/defs.hpp"
#include "vpl/preview/exception.hpp"
#include "vpl/mfxstructures.h"

namespace oneapi {
namespace vpl {

/// @brief Represents user's payload data. For encoding, these payloads can be
/// inserted into the bitstream. The payload buffer must contain a valid formatted payload.
/// For decoding, these payloads can be retrieved as the decoder parses the bitstream and
/// caches them in an internal buffer.
class payload {
public:
    /// @brief Specialized contructor for the MPEG-2 codec's payload data.
    /// @param[in] data Payload data
    /// @param[in] numBit Payload data length in bits
    payload(const std::vector<uint8_t> &data, uint32_t numBit)
            : payload_(),
              codec_id_(codec_format_fourcc::mpeg2) {
        ctor_helper(data, numBit);
    }

    /// @brief Specialized contructor for the AVC codec's payload data.
    /// @param[in] data Payload data
    /// @param[in] numBit Payload data length in bits
    /// @param[in] type SEI message type
    payload(const std::vector<uint8_t> &data, uint32_t numBit, uint16_t type)
            : payload_(),
              codec_id_(codec_format_fourcc::avc) {
        ctor_helper(data, numBit, type);
    }

    /// @brief Specialized contructor for the HEVC codec's payload data.
    /// @tparam CID codec FourCC code
    /// @param[in] data Payload data
    /// @param[in] numBit Payload data length in bits
    /// @param[in] type SEI message type
    /// @param[in] suffix Boolean flag to attach data as suffix.
    payload(const std::vector<uint8_t> &data, uint32_t numBit, uint16_t type, bool suffix)
            : payload_(),
              codec_id_(codec_format_fourcc::hevc) {
        ctor_helper(data, numBit, type, suffix);
    }

    /// @brief Dtor
    virtual ~payload() {
        delete[] payload_.Data;
        payload_.Data = 0;
    }

    /// @brief Copy ctor.
    /// @param[in] other Object to copy.
    payload(const payload& other) {
        ctor_helper(other.get_payload_data(),
                    other.payload_.NumBit,
                    other.payload_.Type,
                    other.payload_.CtrlFlags);
    }

    /// @brief Move ctor.
    /// @param[in] other Object to move
    payload(payload&& other) {
        payload_.Type      = other.payload_.Type;
        payload_.CtrlFlags = other.payload_.CtrlFlags;
        payload_.NumBit    = other.payload_.NumBit;
        payload_.BufSize   = other.payload_.BufSize;
        payload_.Data      = other.payload_.Data;

        other.payload_.Data = 0;
    }

    /// @brief Assignment operator.
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    payload& operator=(const payload& other) {
        if (&other == this)
            return *this;

        if (this->payload_.Data)
            delete[] this->payload_.Data;

        ctor_helper(other.get_payload_data(),
                    other.payload_.NumBit,
                    other.payload_.Type,
                    other.payload_.CtrlFlags);
        return *this;
    }

    /// @brief Move operator.
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    payload& operator=(payload&& other) {
        if (&other == this)
            return *this;

        if (this->payload_.Data)
            delete[] this->payload_.Data;

        payload_.Type      = other.payload_.Type;
        payload_.CtrlFlags = other.payload_.CtrlFlags;
        payload_.NumBit    = other.payload_.NumBit;
        payload_.BufSize   = other.payload_.BufSize;
        payload_.Data      = other.payload_.Data;

        other.payload_.Data = 0;
        return *this;
    }

    /// @brief Returns codec FourCC code
    /// @return Codec FourCC code
    codec_format_fourcc get_codec_id() const {
        return codec_id_;
    }

    /// @brief Returns SEI message type. Only enables for AVC and HEVC codecs
    /// @return SEI message type.
    uint16_t get_type() const {
        if (codec_id_ == codec_format_fourcc::hevc || codec_id_ == codec_format_fourcc::avc)
            return payload_.Type;
        throw(base_exception(MFX_ERR_UNDEFINED_BEHAVIOR));
    }

    /// @brief Returns boolean suffix flag. Only enables for HEVC codec
    /// @return boolean suffix flag.
    bool is_suffix() const {
        if (codec_id_ == codec_format_fourcc::hevc) {
            if (MFX_PAYLOAD_CTRL_SUFFIX == payload_.CtrlFlags)
                return true;
            return false;
        }
        throw(base_exception(MFX_ERR_UNDEFINED_BEHAVIOR));
    }

    /// @brief Returns number of bits in the payload
    /// @return Number of bits in the payload
    uint16_t get_num_bits() const {
        return payload_.NumBit;
    }

    /// @brief Returns container with payload data
    /// @return Number Container with payload data
    std::vector<uint8_t> get_payload_data() const {
        return std::vector<uint8_t>(payload_.Data, payload_.Data + payload_.BufSize);
    }

    /// @brief Returns pointer to the raw data
    /// @return Pointer to the raw data
    mfxPayload* get_raw_ptr() {
        return &payload_;
    }

protected:
    /// @brief Internal helper function for ctor.
    /// @param[in] data Payload data
    /// @param[in] numBit Payload data length in bits
    /// @param[in] type SEI message type
    /// @param[in] suffix Boolean flag to attach data as suffix.
    void ctor_helper(std::vector<uint8_t> data,
                     uint32_t numBit,
                     uint16_t type = 0x01B2,
                     bool suffix   = false) {
        payload_.Type = type;
        if (suffix) {
            payload_.CtrlFlags = MFX_PAYLOAD_CTRL_SUFFIX;
        }
        payload_.NumBit  = numBit;
        payload_.BufSize = static_cast<uint16_t>(data.size());
        payload_.Data    = new uint8_t[payload_.BufSize];
        std::copy(data.begin(), data.end(), payload_.Data);
    }
    /// @brief Raw data
    mfxPayload payload_;

    /// @brief Codec FourCC code
    codec_format_fourcc codec_id_;
};

} // namespace vpl
} // namespace oneapi
