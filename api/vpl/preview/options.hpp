/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <exception>
#include <map>
#include <string>

#include "vpl/preview/detail/variant.hpp"
#include "vpl/preview/property_name.hpp"

#include "vpl/mfxcommon.h"
#include "vpl/mfxdefs.h"

namespace oneapi {
namespace vpl {

/// @brief Represents implementation properties for the dispatcher
/// @todo Rename class
class property {
public:
    /// @brief Default ctor
    property() : name_(), value_() {}
    
    /// @brief Constucts property with given name and the value
    /// @tparam T Type of the value
    /// @param[in] name Name of the property
    /// @param[in] value Value
    template <typename T>
    property(property_name &name, T value) : value_(value),
                                             name_(name.get_name()) {}
    /// @brief Constucts property with given name and the value
    /// @tparam T Type of the value
    /// @param[in] name Name of the property
    /// @param[in] value Value
    template <typename T>
    property(std::string &name, T value) : value_(value),
                                           name_(name) {}
    /// @brief Copy ctor
    /// @param[in] other another object to use as data source
    property(const property &other) : value_(other.value_), name_(other.name_) {}
    /// @brief Dtor
    virtual ~property() {}

    /// @brief Assignment operator.
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    property &operator=(const property &other) {
        name_  = other.get_name();
        value_ = other.value_;
        return *this;
    }
    /// @brief Returns type of the property's value
    /// @return Type of the property's value
    /// @todo Consider remove
    mfxVariantType get_type() const {
        return value_.get_variant().Type;
    }
    /// @brief Returns name of the property's value
    /// @return Name of the property's value
    std::string get_name() const {
        return name_;
    }
    /// @brief Returns property's value
    /// @return Property's value
    mfxVariant::data get_data() const {
        return value_.get_variant().Data;
    }
    /// @brief Converts propery into mfxVariant type
    /// @return Property type and value
    mfxVariant GetValue() const {
        return value_.get_variant();
    }

protected:
    /// @brief Name of the property
    std::string name_;
    /// @brief Value of the property
    detail::variant value_;
};

} // namespace vpl
} // namespace oneapi
