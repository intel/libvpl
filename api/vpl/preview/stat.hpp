/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <cinttypes>
#include "vpl/mfxstructures.h"
namespace oneapi {
namespace vpl {

/// @brief Interface for the component's (session) run-time statistic
class stat {
public:
    /// @brief Default ctor
    stat() {}

    /// @brief Default dtor
    virtual ~stat() {}

    /// @brief Retrieves number of processed frames
    /// @return Number of processed frames
    virtual uint32_t get_num_frame() const = 0;

    /// @brief Retrieves number of cached frames
    /// @return Number of cached frames
    virtual uint32_t get_num_cached_frame() const = 0;
};

/// @brief Decoder's session run-time statistic
class decode_stat : public stat {
public:
    /// @brief Default ctor
    decode_stat() : stat(), stat_() {}

    /// @brief Default dtor
    virtual ~decode_stat() {}

    /// @brief Retrieves number of processed frames
    /// @return Number of processed frames
    virtual uint32_t get_num_frame() const {
        return stat_.NumFrame;
    }

    /// @brief Retrieves number of cached frames
    /// @return Number of cached frames
    virtual uint32_t get_num_cached_frame() const {
        return stat_.NumCachedFrame;
    }

    /// @brief Retrieves number of errors recovered
    /// @return Number of errors recovered
    virtual uint32_t get_num_error() const {
        return stat_.NumError;
    }

    /// @brief Retrieves number of skipped frames
    /// @return Number of skipped frames
    virtual uint32_t get_num_skipped_frame() const {
        return stat_.NumSkippedFrame;
    }

    /// @brief Retrieves raw data pointer
    /// @return Raw data pointer
    mfxDecodeStat* get_raw() {
        return &(this->stat_);
    }

protected:
    /// @brief Raw statistic data
    mfxDecodeStat stat_;
};

/// @brief Encoder's session run-time statistic
class encode_stat : public stat {
public:
    /// @brief Default ctor
    encode_stat() : stat(), stat_() {}

    /// @brief Default dtor
    virtual ~encode_stat() {}

    /// @brief Retrieves number of processed frames
    /// @return Number of processed frames
    virtual uint32_t get_num_frame() const {
        return stat_.NumFrame;
    }

    /// @brief Retrieves number of cached frames
    /// @return Number of cached frames
    virtual uint32_t get_num_cached_frame() const {
        return stat_.NumCachedFrame;
    }

    /// @brief Retrieves number of bits for all encoded frames
    /// @return Number of bits for all encoded frames
    virtual uint64_t get_num_bit() const {
        return stat_.NumBit;
    }

    /// @brief Retrieves raw data pointer
    /// @return Raw data pointer
    mfxEncodeStat* get_raw() {
        return &(this->stat_);
    }

protected:
    /// @brief Raw statistic data
    mfxEncodeStat stat_;
};

/// @brief VPP's session run-time statistic
class vpp_stat : public stat {
public:
    /// @brief Default ctor
    vpp_stat() : stat(), stat_() {}

    /// @brief Default dtor
    virtual ~vpp_stat() {}

    /// @brief Retrieves number of processed frames
    /// @return Number of processed frames
    virtual uint32_t get_num_frame() const {
        return stat_.NumFrame;
    }

    /// @brief Retrieves number of cached frames
    /// @return Number of cached frames
    virtual uint32_t get_num_cached_frame() const {
        return stat_.NumCachedFrame;
    }

    /// @brief Retrieves raw data pointer
    /// @return Raw data pointer
    mfxVPPStat* get_raw() {
        return &(this->stat_);
    }

protected:
    /// @brief Raw statistic data
    mfxVPPStat stat_;
};

} // namespace vpl
} // namespace oneapi
