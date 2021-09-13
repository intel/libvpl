/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <exception>
#include <string>

#include "vpl/mfxdefs.h"

namespace oneapi {
namespace vpl {

/// @brief Base class to represents oneVPL exception (negative) C API error codes
class base_exception : virtual public std::exception {
public:
    /// @brief Default ctor
    /// @param[in] status Operation ststus code
    explicit base_exception(mfxStatus status) : status_(status), text_(), res_() {
        res_ = "oneVPL: " + text_ + ": " + status_to_string();
    }

    /// @brief Default ctor
    /// @param[in] text Additional text to add to status string
    /// @param[in] status Operation status code
    explicit base_exception(std::string text, mfxStatus status)
            : status_(status),
              text_(text),
              res_() {
        res_ = "oneVPL: " + text_ + ": " + status_to_string();
    }

    base_exception(const base_exception& ex)
            : status_(ex.status_),
              text_(ex.text_),
              res_(ex.res_) {}

    /// @brief Assignment operator
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    base_exception& operator=(const base_exception& other) = default;

    /// @brief Move ctor
    /// @param[in] other another object to use as data source
    base_exception(base_exception&& other) = default;

    /// @brief Move operator
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    base_exception& operator=(base_exception&& other) = default;

    /// @brief Returns exception description in text form
    /// @return null terminated string with the exception details
    virtual const char* what() const noexcept {
        return res_.c_str();
    }
    /// @brief Provides C API status code
    /// @return C API status code
    const mfxStatus get_status() const noexcept {
        return status_;
    }

protected:
    /// @brief Converts C API status code in to the text form
    /// @return C API status code min text form
    std::string status_to_string() const noexcept {
        switch (status_) {
            case MFX_ERR_NONE:
                return "No error.";
            case MFX_ERR_UNKNOWN:
                return "Unknown error.";
            case MFX_ERR_NULL_PTR:
                return "NULL pointer.";
            case MFX_ERR_UNSUPPORTED:
                return "Unsupported.";
            case MFX_ERR_MEMORY_ALLOC:
                return "Memory allocation error.";
            case MFX_WRN_ALLOC_TIMEOUT_EXPIRED:
                return "Memory allocation timeout expired.";
            case MFX_ERR_NOT_ENOUGH_BUFFER:
                return "Not enough buffer.";
            case MFX_ERR_INVALID_HANDLE:
                return "Invalide handle.";
            case MFX_ERR_LOCK_MEMORY:
                return "Lock memory error.";
            case MFX_ERR_NOT_INITIALIZED:
                return "Not initialized.";
            case MFX_ERR_NOT_FOUND:
                return "Not found.";
            case MFX_ERR_MORE_DATA:
                return "Not enough data.";
            case MFX_ERR_MORE_SURFACE:
                return "Not enough surfaces.";
            case MFX_ERR_ABORTED:
                return "Operation aborted.";
            case MFX_ERR_DEVICE_LOST:
                return "Decice lost.";
            case MFX_ERR_INCOMPATIBLE_VIDEO_PARAM:
                return "Incompartible video parameter.";
            case MFX_ERR_INVALID_VIDEO_PARAM:
                return "Invlid video parameter.";
            case MFX_ERR_UNDEFINED_BEHAVIOR:
                return "Behaviour is undefined.";
            case MFX_ERR_DEVICE_FAILED:
                return "Device failed.";
            case MFX_ERR_MORE_BITSTREAM:
                return "Not enough bitstream.";
            case MFX_ERR_GPU_HANG:
                return "Device hang.";
            case MFX_ERR_REALLOC_SURFACE:
                return "Bigger output surface required.";
            case MFX_ERR_RESOURCE_MAPPED:
                return "Resource mapping failed.";
            case MFX_ERR_NOT_IMPLEMENTED:
                return "Not implemetned.";
            default:
                return "Unregistered unknown error.";
        }
        return "Unregistered unknown error.";
    }
    /// @brief C API status code
    mfxStatus status_;
    /// @brief String with additional error text
    std::string text_;
    /// @brief String with error code
    std::string res_;
};

/// @brief File exception class to represent oneVPL exception (negative) C API error codes
class file_exception : virtual public std::exception {
public:
    /// @brief Default ctor
    /// @param[in] text Additional text to add to status string
    explicit file_exception(const std::string& text) : res_(text) {
        res_ = "oneVPL File IO: " + text;
    }

    file_exception(const file_exception& ex) : res_(ex.res_) {}

    /// @brief Assignment operator
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    file_exception& operator=(const file_exception& other) = default;

    /// @brief Move ctor
    /// @param[in] other another object to use as data source
    file_exception(file_exception&& other) = default;

    /// @brief Move operator
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    file_exception& operator=(file_exception&& other) = default;

    /// @brief Returns exception description in text form
    /// @return null terminated string with the exception details
    virtual const char* what() const noexcept {
        return res_.c_str();
    }

protected:
    /// @brief String with error code
    std::string res_;
};

} // namespace vpl
} // namespace oneapi
