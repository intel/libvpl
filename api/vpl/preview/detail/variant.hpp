/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <string>
#include <ostream>
// #include <variant>
#include <exception>

#include "vpl/mfxcommon.h"
#include "vpl/mfxdefs.h"

namespace oneapi {
namespace vpl {
namespace detail {

/// @brief This class represents variant-like datatype of selected mfx* types, including pointer
class variant {
public:
    /// @brief Default ctor. Initializes variand with value of unset data type.
    variant() {
        v_.Type     = MFX_VARIANT_TYPE_UNSET;
        v_.Data.U64 = 0;
    }

    /// @brief Parametrized ctor. Accepts values of fixed set of mfx* types, including pointer.
    /// @tparam T Type of value
    /// @param[in] t Value
    template <typename T>
    explicit variant(T t) : v_({ { { 0 } } }){
        type_init(t);
    }

    /// @brief Returns instance of mfxVariant C type.
    /// @return instance of mfxVariant C type.
    mfxVariant get_variant() const {
        return v_;
    }

private:
    /// @brief Ctor's helper to check of value of uint8_t data type.
    template <typename T>
    typename std::enable_if<std::is_same<char, T>::value>::type type_init(T t) {
        v_.Data.U8 = t;
        v_.Type    = MFX_VARIANT_TYPE_U8;
    }

    template <typename T>
    typename std::enable_if<std::is_same<uint8_t, T>::value>::type type_init(T t) {
        v_.Data.U8 = t;
        v_.Type    = MFX_VARIANT_TYPE_U8;
    }

    template <typename T>
    typename std::enable_if<std::is_same<int8_t, T>::value>::type type_init(T t) {
        v_.Data.I8 = t;
        v_.Type    = MFX_VARIANT_TYPE_I8;
    }

    template <typename T>
    typename std::enable_if<std::is_same<uint16_t, T>::value>::type type_init(T t) {
        v_.Data.U16 = t;
        v_.Type     = MFX_VARIANT_TYPE_U16;
    }

    template <typename T>
    typename std::enable_if<std::is_same<int16_t, T>::value>::type type_init(T t) {
        v_.Data.I16 = t;
        v_.Type     = MFX_VARIANT_TYPE_I16;
    }

    template <typename T>
    typename std::enable_if<std::is_same<uint32_t, T>::value>::type type_init(T t) {
        v_.Data.U32 = t;
        v_.Type     = MFX_VARIANT_TYPE_U32;
    }

    template <typename T>
    typename std::enable_if<std::is_same<int32_t, T>::value>::type type_init(T t) {
        v_.Data.I32 = t;
        v_.Type     = MFX_VARIANT_TYPE_I32;
    }

    template <typename T>
    typename std::enable_if<std::is_same<uint64_t, T>::value>::type type_init(T t) {
        v_.Data.U64 = t;
        v_.Type     = MFX_VARIANT_TYPE_U64;
    }

    template <typename T>
    typename std::enable_if<std::is_same<int64_t, T>::value>::type type_init(T t) {
        v_.Data.I64 = t;
        v_.Type     = MFX_VARIANT_TYPE_I64;
    }

    template <typename T>
    typename std::enable_if<std::is_same<mfxF32, T>::value>::type type_init(T t) {
        v_.Data.F32 = t;
        v_.Type     = MFX_VARIANT_TYPE_F32;
    }

    template <typename T>
    typename std::enable_if<std::is_same<mfxF64, T>::value>::type type_init(T t) {
        v_.Data.F64 = t;
        v_.Type     = MFX_VARIANT_TYPE_F64;
    }

    template <typename T>
    typename std::enable_if<std::is_same<void*, T>::value>::type type_init(T t) {
        v_.Data.Ptr = t;
        v_.Type     = MFX_VARIANT_TYPE_PTR;
    }

    template <typename T>
    typename std::enable_if<std::is_same<std::string_view, T>::value>::type type_init(T t) {
        v_.Data.Ptr = (void*)t.data();
        v_.Type     = MFX_VARIANT_TYPE_PTR;
    }

    template <typename T>
    typename std::enable_if<std::is_same<std::string, T>::value>::type type_init(T& t) {
        v_.Data.Ptr = (void*)t.data();
        v_.Type     = MFX_VARIANT_TYPE_PTR;
    }

protected:
    /// Instance of mfxVariant where all data is stored and used by C API
    mfxVariant v_;
};


inline std::ostream& operator<<(std::ostream& out, const variant& p) {
    mfxVariant v = p.get_variant();
    switch(v.Type){
    case MFX_VARIANT_TYPE_UNSET:
        out << "UNSET";
        break;
    case MFX_VARIANT_TYPE_U8:
        out << "(uint8_t)" << v.Data.U8;
        break;
    case MFX_VARIANT_TYPE_I8:
        out << "(int8_t)" << v.Data.I8;
        break;
    case MFX_VARIANT_TYPE_U16:
        out << "(uint16_t)" << v.Data.U16;
        break;
    case MFX_VARIANT_TYPE_I16:
        out << "(int16_t)" << v.Data.I16;
        break;
    case MFX_VARIANT_TYPE_U32:
        out << "(uint32_t)" << v.Data.U32;
        break;
    case MFX_VARIANT_TYPE_I32:
        out << "(int32_t)" << v.Data.I32;
        break;
    case MFX_VARIANT_TYPE_U64:
        out << "(uint64_t)" << v.Data.U64;
        break;
    case MFX_VARIANT_TYPE_I64:
        out << "(int64_t)" << v.Data.I64;
        break;
    case MFX_VARIANT_TYPE_F32:
        out << "(float32_t)" << v.Data.F32;
        break;
    case MFX_VARIANT_TYPE_F64:
        out << "(float64_t)" << v.Data.F64;
        break;
    case MFX_VARIANT_TYPE_PTR:
        out << "(void*)" << v.Data.Ptr;
        break;

    }
    return out;
}

} // namespace detail
} // namespace vpl
} // namespace oneapi
