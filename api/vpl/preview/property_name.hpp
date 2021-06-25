/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <utility>

// leave table formatting alone
// clang-format off

namespace oneapi {
namespace vpl {

#define MARKER 0x7fffffff

/// @brief Item of the layer.
/// @details
/// We parse name of the value to extrac parents structures and verify correctness of the parameter. Let's imagine
/// we have parameter: "mfxImplDescription.Impl". Here we have two layers: "mfxImplDescription" with ID = 1 and "Impl"
/// with ID = 2. "Impl" has parentID = 1. Finally we will have two instances of the structure.
struct layer {
    unsigned int ID; /// Unique ID of the parameter.
    unsigned int parentID; /// ID of the parent name.
    const char* name; /// Name of the parameter.
};

/// @brief This structure represents array of layers in the params hierarchy,
/// @todo Should go to the detail namespace.
struct layers {
    const struct layer* pLayer; /// Pointer to the layer
};

#define ENTRY(id, parent, propname) \
    { id, parent, propname }

#define END_OF_LAYER \
    { MARKER, MARKER, "end" }

const struct layer layer_0[] = {
    ENTRY(0, MARKER, "mfxImplDescription"),
    ENTRY(1, MARKER, "mfxImplementedFunctions"),
    ENTRY(2, MARKER, "DXGIAdapterIndex"),
    ENTRY(3, MARKER, "mfxHandleType"),
    ENTRY(4, MARKER, "mfxHDL"),
    END_OF_LAYER
};

const struct layer layer_1[] = { 
    ENTRY(101, 0, "Impl"),
    ENTRY(102, 0, "AccelerationMode"),
    ENTRY(103, 0, "ApiVersion"),
    ENTRY(104, 0, "ImplName"),
    ENTRY(105, 0, "License"),
    ENTRY(106, 0, "Keywords"),
    ENTRY(107, 0, "VendorID"),
    ENTRY(108, 0, "VendorImplID"),
    ENTRY(109, 0, "mfxDeviceDescription"),
    ENTRY(110, 0, "mfxDecoderDescription"),
    ENTRY(111, 0, "mfxEncoderDescription"),
    ENTRY(112, 0, "mfxVPPDescription"),
    ENTRY(113, 1, "FunctionsName"),
    END_OF_LAYER
};

const struct layer layer_2[] = {
    ENTRY(201, 103, "Minor"),
    ENTRY(202, 103, "Major"),
    ENTRY(203, 109, "device"),
    ENTRY(204, 110, "decoder"), // Small letter in use by dispatcher
    ENTRY(205, 111, "encoder"),
    ENTRY(206, 112, "filter"),
    ENTRY(207, 103, "Version"),
    END_OF_LAYER
};

const struct layer layer_3[] = { 
    ENTRY(301, 203, "DeviceID"),
    ENTRY(302, 204, "CodecID"),
    ENTRY(303, 204, "MaxcodecLevel"),
    ENTRY(304, 204, "decprofile"),
    ENTRY(305, 205, "CodecID"),
    ENTRY(306, 205, "MaxcodecLevel"),
    ENTRY(307, 205, "BiDirectionalPrediction"),
    ENTRY(308, 205, "encprofile"),
    ENTRY(309, 206, "FilterFourCC"),
    ENTRY(310, 206, "MaxDelayInFrames"),
    ENTRY(311, 206, "memdesc"),
    END_OF_LAYER
};

const struct layer layer_4[] = {
    ENTRY(401, 304, "Profile"),
    ENTRY(402, 304, "decmemdesc"),
    ENTRY(403, 308, "Profile"),
    ENTRY(404, 308, "encmemdesc"),
    ENTRY(405, 311, "MemHandleType"),
    ENTRY(406, 311, "Width"),
    ENTRY(407, 311, "Height"),
    ENTRY(408, 311, "format"),
    END_OF_LAYER
};

const struct layer layer_5[] = { 
    ENTRY(501, 402, "MemHandleType"),
    ENTRY(502, 402, "Width"),
    ENTRY(503, 402, "Height"),
    ENTRY(504, 402, "ColorFormats"),
    ENTRY(505, 404, "MemHandleType"),
    ENTRY(506, 404, "Width"),
    ENTRY(507, 404, "Height"),
    ENTRY(508, 404, "ColorFormats"),
    ENTRY(509, 408, "InFormat"),
    ENTRY(510, 408, "OutFormats"),
    END_OF_LAYER
};

constexpr struct layers layers_[] = { layer_0, layer_1, layer_2, layer_3, layer_4, layer_5 };

#define MAX_LEVEL 6

// end table formatting
// clang-format on

/// @brief Forms string at compile time for dispatcher with the property name. String is built from the series of
/// substrings where nested subnames are separated by the `.`. On each step, subname value is verified that it
/// is allowed for that level. If wrong subname is provided - exception is raised.
/// @details Usage example:
/// @code
/// oneapi::vpl::property_name p1;
/// oneapi::vpl::property opt2(p2 / "mfxImplDescription" / "mfxDecoderDescription" / "decoder" / "CodecID"
///                     , (uint32_t)MFX_CODEC_HEVC);
/// @endcode
class property_name {
public:
    /// @brief Default ctor. Initializes empty object
    constexpr property_name() : names_(), level_(0), ID_(MARKER) {}
    /// @brief Construct object by assuming that @p name is the first level name,
    /// @param name Allpowed first level name.
    constexpr explicit property_name(const char* name) : names_(), level_(0), ID_(MARKER) {
        std::string_view n(name);
        size_t end_pos = n.find_first_of(std::string_view("."), 0), curr_read_pos = 0;
        if (end_pos == std::string_view::npos) {
            // Single parameter case. No need to tokenize
            int i = 0;
            ID_   = check_value(name, 0, MARKER);
            for (i = 0; name[i] != '\0'; i++) {
                names_[0][i] = name[i];
            }
            names_[0][i] = '\0';
            level_       = 1;
        }
        else if (end_pos != 0) {
            while (curr_read_pos != std::string_view::npos) {
                std::string_view n1 = n.substr(curr_read_pos, (end_pos - curr_read_pos));
                add_item(n1);
                if (end_pos == std::string_view::npos) {
                    curr_read_pos = std::string_view::npos;
                }
                else {
                    curr_read_pos = end_pos + 1;
                }
                end_pos = n.find_first_of(std::string_view(".\0"), end_pos + 1);
            }
        }
        else {
            error_happened(0); // This is workaround for GCC bug.
        }
    }
    /// @brief Adds next subname to the name. Subname value is verified that it is valid for the current
    /// level of hierarchy.
    /// @param subname Constant string with the subname
    /// @return Reference to the updated object.
    constexpr property_name& operator/(const char* subname) {
        int i = 0;
        ID_   = check_value(subname, level_, ID_);

        for (i = 0; subname[i] != '\0'; i++) {
            names_[level_][i] = subname[i];
        }
        names_[level_][i] = '\0';
        level_++;
        return *this;
    }
    /// @brief Non compile time member. Creates std::string object with the name, where all given
    /// subnames are separated by `.`.
    /// @return Dot separated string.
    std::string get_name() {
        std::string n;
        for (unsigned int i = 0; i < level_; i++) {
            if (i)
                n += ".";
            n += names_[i];
        }
        return n;
    }

protected:
    /// @brief Array withe subnames for each level.
    char names_[MAX_LEVEL][128];
    /// @brief Current hierarchy level
    unsigned int level_;
    /// @brief ID of the previous subname
    unsigned int ID_;

private:
    /// @brief Throw logic error
    /// @param n error value
    /// @return Throw error or 1
    constexpr auto error_happened(int n) -> int {
        return n <= 0 ? throw std::logic_error("absent property") : 1;
    }

    /// @brief Check value in layer
    /// @param name Subname
    /// @param l Layer
    /// @param prevID ID of the previous subname
    /// @return Value in layer
    constexpr unsigned int check_value_in_layer(const char* name,
                                                const struct layer* l,
                                                unsigned int prevID) {
        std::string_view n1(name);
        for (int i = 0;; i++) {
            if (l[i].ID == MARKER)
                break;
            if (prevID == l[i].parentID) {
                std::string_view n2(l[i].name);
                if (n1 == n2) {
                    return l[i].ID;
                }
            }
        }
        error_happened(0); // This is workaround for GCC bug.
        return MARKER;
    }

    /// @brief Check value
    /// @param name Subname
    /// @param level Hierarchy level
    /// @param prevID ID of the previous subname
    /// @return Value
    constexpr unsigned int check_value(const char* name, unsigned int level, unsigned int prevID) {
        if (level >= sizeof(layers_) / sizeof(layers_[0]))
            std::logic_error("Too deep property");
        return check_value_in_layer(name, layers_[level].pLayer, prevID);
    }

    /// @brief Check value in layer
    /// @param n1 string_view
    /// @param l Layer
    /// @param prevID ID of the previous subname
    /// @return Value
    constexpr unsigned int check_value_in_layer(std::string_view n1,
                                                const struct layer* l,
                                                unsigned int prevID) {
        for (int i = 0;; i++) {
            if (l[i].ID == MARKER)
                break;
            if (prevID == l[i].parentID) {
                std::string_view n2(l[i].name);
                if (n1 == n2) {
                    return l[i].ID;
                }
            }
        }
        error_happened(0); // This is workaround for GCC bug.
        return MARKER;
    }

    /// @brief Check value
    /// @param name Subname
    /// @param level Hierarchy level
    /// @param prevID ID of the previous subname
    /// @return Value
    constexpr unsigned int check_value(std::string_view name,
                                       unsigned int level,
                                       unsigned int prevID) {
        if (level >= sizeof(layers_) / sizeof(layers_[0]))
            std::logic_error("Too deep property");
        return check_value_in_layer(name, layers_[level].pLayer, prevID);
    }

    /// @brief Add item
    /// @param subname Subname
    constexpr void add_item(std::string_view subname) {
        int i = 0;
        ID_   = check_value(subname, level_, ID_);

        for (i = 0; i < subname.size(); i++) {
            names_[level_][i] = subname[i];
        }
        names_[level_][i] = '\0';
        level_++;
    }
};

} // namespace vpl
} // namespace oneapi
