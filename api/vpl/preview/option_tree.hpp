//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include <vector>
#include <optional>
#include <string>
#include <utility>
#include "vpl/preview/defs.hpp"
#include "vpl/preview/detail/string_helpers.hpp"
#include "vpl/preview/detail/variant.hpp"

#pragma once

namespace oneapi {
namespace vpl {

namespace detail {

/// @brief Common operations on properties.
class property_details{
public:
    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    virtual void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest) const = 0;

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    virtual void print(std::ostream& out, int indent) const = 0;
};

}

/// @brief Represents a single settable property.
template<typename item_type, typename dest_type=item_type> class property_value : public std::optional<item_type>, detail::property_details{
public:
    /// @brief Full property value type
    using value_type = item_type;

    /// @brief Class type
    using this_type = property_value<item_type, dest_type>;

    /// @brief Class base type
    using base_type = std::optional<value_type>;

    // declare usages from base class
    using base_type::value;
    using base_type::has_value;

    /// @brief ctor
    property_value(std::string name, std::string prop_name) : name_(name), prop_name_(prop_name){}

    /// @brief Assignment operator.
    /// @param[in] value value of object
    /// @returns Reference to this object
    this_type& operator=(value_type const& value){
        base_type::operator=(value);
        return *this;
    }

public:  // detail::property_details
    /// @brief Write value to vector of C Properties.
    /// @param[in] dest destination vector
    void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest) const {
        if(has_value()){
            dest.push_back(std::pair(prefix + prop_name_, detail::variant((dest_type)value())));
        }
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        // this using is scoped to just this method, it pulls
        // operator<< implementations for tuple and pair into scope
        // so that they canbe printed out with this common code.
        using namespace detail;
        if(has_value()){
            out << std::string(indent, ' ') << std::string(indent, ' ')
                << name_
                << " = ";
            out << value();
            out << std::endl;
        }
    }

protected:
    /// @brief UI Name
    std::string name_;

    /// @brief Property name as specificed in spec
    std::string prop_name_;
};

/// @brief Represents a pair of settable properties that are always set together.
template<
    typename item_type_1,
    typename item_type_2=item_type_1,
    typename dest_type_1=item_type_1,
    typename dest_type_2=item_type_2> class property_pair_value : public std::optional<std::pair<item_type_1, item_type_2>>, detail::property_details{
public:
    /// @brief Single property value type
    using item_type = std::pair<item_type_1, item_type_2>;

    /// @brief Full property value type
    using value_type = item_type;

    /// @brief Class type
    using this_type = property_pair_value<item_type_1, item_type_2, dest_type_1, dest_type_2>;

    /// @brief Class base type
    using base_type = std::optional<value_type>;

    // declare usages from base class
    using base_type::value;
    using base_type::has_value;

    /// @brief ctor
    property_pair_value(std::string name, std::string prop_name_1, std::string prop_name_2)
        : name_(name), 
        prop_name_1_(prop_name_1), 
        prop_name_2_(prop_name_2){}

    /// @brief Assignment operator.
    /// @param[in] value value of object
    /// @returns Reference to this object
    this_type& operator=(value_type const& value){
        base_type::operator=(value);
        return *this;
    }

public:  // detail::property_details
    /// @brief Write value to vector of C Properties.
    /// @param[in] dest destination vector
    void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest) const {
        if(has_value()){
            dest.push_back(std::pair(prefix + prop_name_1_, detail::variant((dest_type_1)value().first)));
            dest.push_back(std::pair(prefix + prop_name_2_, detail::variant((dest_type_2)value().second)));
        }
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        // this using is scoped to just this method, it pulls
        // operator<< implementations for tuple and pair into scope
        // so that they canbe printed out with this common code.
        using namespace detail;
        if(has_value()){
            out << std::string(indent, ' ') << std::string(indent, ' ')
                << name_
                << " = ";
            out << value();
            out << std::endl;
        }
    }

protected:
    /// @brief UI Name
    std::string name_;

    /// @brief Property 1 name as specificed in spec
    std::string prop_name_1_;

    /// @brief Property 2 name as specificed in spec
    std::string prop_name_2_;
};

/// @brief Represents a list of settings for properties that can have multiple values.
template<typename item_type, typename dest_type=item_type> class list_property_value : public std::optional<std::vector<item_type>>, detail::property_details{
public:
    /// @brief Full property value type
    using value_type = std::vector<item_type>;

    /// @brief Class type
    using this_type = list_property_value<item_type, dest_type>;

    /// @brief Class base type
    using base_type = std::optional<value_type>;

    // declare usages from base class
    using base_type::value;
    using base_type::has_value;

    /// @brief ctor
    list_property_value(std::string name, std::string prop_name) : name_(name), prop_name_(prop_name){}

    /// @brief Assignment operator.
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    this_type& operator=(this_type const& other){
        base_type::operator=(other);
        return *this;
    }

    /// @brief Assignment operator.
    /// @param[in] value value of object
    /// @returns Reference to this object
    this_type& operator=(value_type const& value){
        base_type::operator=(value);
        return *this;
    }

    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest) const {
        if(this->has_value()){
            for(auto&v : value()){
                auto variant_value = detail::variant((dest_type)v);
                dest.push_back(std::pair(prefix + prop_name_, variant_value));
            }
        }
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        // this using is scoped to just this method, it pulls
        // operator<< implementations for tuple and pair into scope
        // so that they can be printed out with this common code.
        using namespace detail;
        if(this->has_value()){
            out << std::string(indent, ' ') << std::string(indent, ' ')
                << name_
                << " = ";
            out << this->value();
            out << std::endl;
        }
    }
protected:
    /// @brief UI Name
    std::string name_;

    /// @brief Property name as specificed in spec
    std::string prop_name_;
};

/// @brief Represents a list of settings for pairs of properties that can have multiple values that are set together.
template<
    typename item_type_1,
    typename item_type_2=item_type_1,
    typename dest_type_1=item_type_1,
    typename dest_type_2=item_type_2> class list_property_pair_value : public std::optional<std::vector<std::pair<item_type_1, item_type_2>>>, detail::property_details{
public:
    /// @brief Single property value type
    using item_type = std::pair<item_type_1, item_type_2>;

    /// @brief Full property value type
    using value_type = std::vector<item_type>;

    /// @brief Class type
    using this_type = list_property_pair_value<item_type_1, item_type_2, dest_type_1, dest_type_2>;

    /// @brief Class base type
    using base_type = std::optional<value_type>;

    // declare usages from base class
    using base_type::value;
    using base_type::has_value;

    /// @brief ctor
    list_property_pair_value(std::string name, std::string prop_name_1, std::string prop_name_2)
        : name_(name), 
        prop_name_1_(prop_name_1), 
        prop_name_2_(prop_name_2){}

    /// @brief Assignment operator.
    /// @param[in] value value of object
    /// @returns Reference to this object
    this_type& operator=(value_type const& value){
        base_type::operator=(value);
        return *this;
    }

public:  // detail::property_details
    /// @brief Write value to vector of C Properties.
    /// @param[in] dest destination vector
    void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest) const {
        if(has_value()){
            for(auto&v : value()){
                dest.push_back(std::pair(prefix + prop_name_1_, detail::variant((dest_type_1)v.first)));
                dest.push_back(std::pair(prefix + prop_name_2_, detail::variant((dest_type_2)v.second)));
            }
        }
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        // this using is scoped to just this method, it pulls
        // operator<< implementations for tuple and pair into scope
        // so that they canbe printed out with this common code.
        using namespace detail;
        if(has_value()){
            out << std::string(indent, ' ') << std::string(indent, ' ')
                << name_
                << " = ";
            out << value();
            out << std::endl;
        }
    }

protected:
    /// @brief UI Name
    std::string name_;

    /// @brief Property 1 name as specificed in spec
    std::string prop_name_1_;

    /// @brief Property 2 name as specificed in spec
    std::string prop_name_2_;
};

#define VPL_OPT_TREE_PROPERTY(PUBLIC_TYPE, IMPL_TYPE, NAME) \
    property_value<PUBLIC_TYPE, IMPL_TYPE> NAME; \
    property_value<PUBLIC_TYPE, IMPL_TYPE>::value_type& get_##NAME(){ return NAME.value(); } \
    void set_##NAME(property_value<PUBLIC_TYPE, IMPL_TYPE>::value_type v){ NAME = v; }

#define VPL_OPT_TREE_PROPERTY_PAIR(PUBLIC_TYPE_1, PUBLIC_TYPE_2, IMPL_TYPE_1, IMPL_TYPE_2, NAME) \
    property_pair_value<PUBLIC_TYPE_1, PUBLIC_TYPE_2, IMPL_TYPE_1, IMPL_TYPE_2> NAME; \
    property_pair_value<PUBLIC_TYPE_1, PUBLIC_TYPE_2, IMPL_TYPE_1, IMPL_TYPE_2>::value_type& get_##NAME(){ return NAME.value(); } \
    void set_##NAME(property_pair_value<PUBLIC_TYPE_1, PUBLIC_TYPE_2, IMPL_TYPE_1, IMPL_TYPE_2>::value_type v){ NAME = v; }

#define VPL_OPT_TREE_LIST_PROPERTY(PUBLIC_TYPE, IMPL_TYPE, NAME) \
    list_property_value<PUBLIC_TYPE, IMPL_TYPE> NAME; \
    list_property_value<PUBLIC_TYPE, IMPL_TYPE>::value_type& get_##NAME(){ return NAME.value(); } \
    void set_##NAME(list_property_value<PUBLIC_TYPE, IMPL_TYPE>::value_type v){ NAME = v; }

#define VPL_OPT_TREE_LIST_PROPERTY_PAIR(PUBLIC_TYPE_1, PUBLIC_TYPE_2, IMPL_TYPE_1, IMPL_TYPE_2, NAME) \
    list_property_pair_value<PUBLIC_TYPE_1, PUBLIC_TYPE_2, IMPL_TYPE_1, IMPL_TYPE_2> NAME; \
    list_property_pair_value<PUBLIC_TYPE_1, PUBLIC_TYPE_2, IMPL_TYPE_1, IMPL_TYPE_2>::value_type& get_##NAME(){ return NAME.value(); } \
    void set_##NAME(list_property_pair_value<PUBLIC_TYPE_1, PUBLIC_TYPE_2, IMPL_TYPE_1, IMPL_TYPE_2>::value_type v){ NAME = v; }

/// @brief Represents the namespace of configurable encode/decode memory decriptor properties. See
/// @p implementation_selector properties for more detail.
class codec_mem_desc_properties {
public:
    /// @brief mem_type property
    VPL_OPT_TREE_PROPERTY(resource_type, uint32_t, mem_type)

    /// @brief frame_size property
    VPL_OPT_TREE_PROPERTY_PAIR(uint32_t, uint32_t, uint32_t, uint32_t, frame_size)

    /// @brief color_format property
    VPL_OPT_TREE_PROPERTY(color_format_fourcc, uint32_t, color_format)

    codec_mem_desc_properties() 
    : mem_type("mem_type", "MemHandleType")
    , frame_size("frame_size", "Width", "Height")
    , color_format("color_format", "ColorFormats") {}
};

/// @brief Represents the namespace of configurable encoder specific memory decriptor properties. See
/// @p implementation_selector properties for more detail.
class enc_mem_desc_properties : public codec_mem_desc_properties {
public:

public:  // detail::property_details
    /// @brief Write value to vector of C Properties.
    /// @param[in] dest destination vector
    void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest) const {
        prefix = prefix + "encmemdesc.";
        mem_type.write(prefix, dest);
        frame_size.write(prefix, dest);
        color_format.write(prefix, dest);
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        out << std::string(indent, ' ') << std::string(indent, ' ')
            << "enc_mem_desc" << std::endl;
        mem_type.print(out, indent+1);
        frame_size.print(out, indent+1);
        color_format.print(out, indent+1);
    }
};

/// @brief Represents the namespace of configurable decoder specific memory decriptor properties. See
/// @p implementation_selector properties for more detail.
class dec_mem_desc_properties : public codec_mem_desc_properties {
public:

public:  // detail::property_details
    /// @brief Write value to vector of C Properties.
    /// @param[in] dest destination vector
    void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest) const { 
        prefix = prefix + "decmemdesc.";
        mem_type.write(prefix, dest);
        frame_size.write(prefix, dest);
        color_format.write(prefix, dest);
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        out << std::string(indent, ' ') << std::string(indent, ' ')
            << "dec_mem_desc" << std::endl;
        mem_type.print(out, indent+1);
        frame_size.print(out, indent+1);
        color_format.print(out, indent+1);
    }
};

/// @brief Represents the namespace of configurable decoder profile properties. See
/// @p implementation_selector properties for more detail.
class dec_profile_properties : detail::property_details {
public:
    /// @brief profile property
    VPL_OPT_TREE_PROPERTY(uint32_t, uint32_t, profile)

    /// @brief dec_mem_desc property namespace
    dec_mem_desc_properties dec_mem_desc;

    /// @brief Default ctor
    dec_profile_properties()
    : profile("profile", "Profile")
    {}

public:  // detail::property_details
    /// @brief Write value to vector of C Properties.
    /// @param[in] dest destination vector
    void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest) const { 
        prefix = prefix + "decprofile.";
        profile.write(prefix, dest);
        dec_mem_desc.write(prefix, dest);
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        out << std::string(indent, ' ') << std::string(indent, ' ')
            << "dec_profile" << std::endl;
        profile.print(out, indent+1);
        dec_mem_desc.print(out, indent+1);
    }
};

/// @brief Represents the namespace of configurable memory decriptor properties. See
/// @p implementation_selector properties for more detail.
class memdesc_properties : detail::property_details{
public:
    /// @brief mem_type property
    VPL_OPT_TREE_PROPERTY(resource_type, uint32_t, mem_type)

    /// @brief frame_size property
    VPL_OPT_TREE_PROPERTY_PAIR(uint32_t, uint32_t, uint32_t, uint32_t, frame_size)

    /// @brief in_color_format property
    VPL_OPT_TREE_PROPERTY(color_format_fourcc, uint32_t, in_color_format)

    /// @brief out_color_format property
    VPL_OPT_TREE_PROPERTY(color_format_fourcc, uint32_t, out_color_format)

    /// @brief Default ctor
    memdesc_properties() 
        : mem_type("mem_type", "MemHandleType")
        , frame_size("frame_size", "Width", "Height")
        , in_color_format("in_color_format", "format.InFormat")
        , out_color_format("out_color_format", "format.OutFormats")
        {}

public:  // detail::property_details
    /// @brief Write value to vector of C Properties.
    /// @param[in] dest destination vector
    void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest) const { 
        prefix = prefix + "memdesc.";
        mem_type.write(prefix, dest);
        frame_size.write(prefix, dest);
        in_color_format.write(prefix, dest);
        out_color_format.write(prefix, dest);
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        out << std::string(indent, ' ') << std::string(indent, ' ')
            << "memdesc" << std::endl;
        mem_type.print(out, indent+1);
        frame_size.print(out, indent+1);
        in_color_format.print(out, indent+1);
        out_color_format.print(out, indent+1);
    }
};

/// @brief Represents the namespace of configurable encoder profile properties. See
/// @p implementation_selector properties for more detail.
class enc_profile_properties : detail::property_details{
public:
    /// @brief profile property
    VPL_OPT_TREE_PROPERTY(uint32_t, uint32_t, profile)

    /// @brief enc_mem_desc property namespace
    enc_mem_desc_properties enc_mem_desc;

    /// @brief Default ctor
    enc_profile_properties()
        : profile("profile", "Profile")
        {}

public:  // detail::property_details
    /// @brief Write value to vector of C Properties.
    /// @param[in] dest destination vector
    void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest) const { 
        prefix = prefix + "encprofile.";
        profile.write(prefix, dest);
        enc_mem_desc.write(prefix, dest);
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        out << std::string(indent, ' ') << std::string(indent, ' ')
            << "enc_profile" << std::endl;
        profile.print(out, indent+1);
        enc_mem_desc.print(out, indent+1);
    }
};

/// @brief Represents the namespace of configurable decoder properties. See
/// @p implementation_selector properties for more detail.
class decoder_properties : detail::property_details{
public:
    /// @brief codec_id property
    VPL_OPT_TREE_LIST_PROPERTY(codec_format_fourcc, uint32_t, codec_id)

    /// @brief max_codec_level property
    VPL_OPT_TREE_PROPERTY(uint32_t, uint32_t, max_codec_level)

    /// @brief dec_profile property namespace
    dec_profile_properties dec_profile;

    /// @brief Default ctor
    decoder_properties()
        : codec_id("codec_id", "CodecID")
        , max_codec_level("max_codec_level", "MaxcodecLevel")
        {}

public:  // detail::property_details
    /// @brief Write value to vector of C Properties.
    /// @param[in] dest destination vector
    void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest) const { 
        prefix = prefix + "mfxImplDescription.mfxDecoderDescription.decoder.";
        codec_id.write(prefix, dest);
        max_codec_level.write(prefix, dest);
        dec_profile.write(prefix, dest);
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        out << std::string(indent, ' ') << std::string(indent, ' ')
            << "decoder" << std::endl;
        codec_id.print(out, indent+1);
        max_codec_level.print(out, indent+1);
        dec_profile.print(out, indent+1);
    }
};

/// @brief Represents the namespace of configurable encoder properties. See
/// @p implementation_selector properties for more detail.
class encoder_properties : detail::property_details{
public:
    /// @brief codec_id property
    VPL_OPT_TREE_LIST_PROPERTY(codec_format_fourcc, uint32_t, codec_id)

    /// @brief max_codec_level property
    VPL_OPT_TREE_PROPERTY(uint32_t, uint32_t, max_codec_level)

    /// @brief bidirectional_prediction property
    VPL_OPT_TREE_PROPERTY(uint16_t, uint16_t, bidirectional_prediction)

    /// @brief enc_profile property namespace
    enc_profile_properties enc_profile;

    /// @brief Default ctor
    encoder_properties()
        : codec_id("codec_id", "CodecID")
        , max_codec_level("max_codec_level", "MaxcodecLevel")
        , bidirectional_prediction("bidirectional_prediction", "BiDirectionalPrediction")
        {}

public:  // detail::property_details
    /// @brief Write value to vector of C Properties.
    /// @param[in] dest destination vector
    void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest)const { 
        prefix = prefix + "mfxImplDescription.mfxEncoderDescription.encoder.";
        codec_id.write(prefix, dest);
        max_codec_level.write(prefix, dest);
        bidirectional_prediction.write(prefix, dest);
        enc_profile.write(prefix, dest);
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        out << std::string(indent, ' ') << std::string(indent, ' ')
            << "encoder" << std::endl;
        codec_id.print(out, indent+1);
        max_codec_level.print(out, indent+1);
        bidirectional_prediction.print(out, indent+1);
        enc_profile.print(out, indent+1);
    }
};

/// @brief Represents the namespace of configurable filter (VPP) properties. See
/// @p implementation_selector properties for more detail.
class filter_properties : detail::property_details{
public:
    /// @brief filter_id property
    VPL_OPT_TREE_LIST_PROPERTY(uint32_t, uint32_t, filter_id)

    /// @brief max_delay_in_frames property
    VPL_OPT_TREE_PROPERTY(uint16_t, uint16_t, max_delay_in_frames)

    /// @brief memdesc property namespace
    memdesc_properties memdesc;

    /// @brief Default ctor
    filter_properties()
        : filter_id("filter_id", "FilterFourCC")
        , max_delay_in_frames("max_delay_in_frames", "MaxDelayInFrames")
        {}

public:  // detail::property_details
    /// @brief Write value to vector of C Properties.
    /// @param[in] dest destination vector
    void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest) const{ 
        prefix = prefix + "mfxImplDescription.mfxVPPDescription.filter.";
        filter_id.write(prefix, dest);
        max_delay_in_frames.write(prefix, dest);
        memdesc.write(prefix, dest);
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        out << std::string(indent, ' ') << std::string(indent, ' ')
            << "filter" << std::endl;
        filter_id.print(out, indent+1);
        max_delay_in_frames.print(out, indent+1);
        memdesc.print(out, indent+1);
    }
};

/// @brief Represents the namespace of configurable device properties. See
/// @p implementation_selector properties for more detail.
class device_properties : detail::property_details{
public:
    /// @brief device_id property
    VPL_OPT_TREE_PROPERTY(std::string, std::string, device_id)

    /// @brief media_adapter property
    VPL_OPT_TREE_PROPERTY(media_adapter_type, uint16_t, media_adapter)

    /// @brief Default ctor
    device_properties()
        : device_id("device_id", "DeviceID")
        , media_adapter("media_adapter", "MediaAdapterType")
        {}

public:  // detail::property_details
    /// @brief Write value to vector of C Properties.
    /// @param[in] dest destination vector
    void write(std::string prefix, std::vector<std::pair<std::string, detail::variant>>& dest) const{ 
        prefix = prefix + "mfxImplDescription.mfxDeviceDescription.device.";
        device_id.write(prefix, dest);
        media_adapter.write(prefix, dest);
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        out << std::string(indent, ' ') << std::string(indent, ' ')
            << "device" << std::endl;
        device_id.print(out, indent+1);
        media_adapter.print(out, indent+1);
    }
};

/// @brief Represents the entire namespace of configurable properties. Each
/// property is exposed as a public member (With C++ conversion support)
/// and property get/set methods to allow direct manipulation of the value
/// Each namespace is a public member of a sperate class that follows the
/// same rules as this class.
class properties{
public:
    /// @brief impl_name property
    VPL_OPT_TREE_PROPERTY(std::string, std::string, impl_name)

    /// @brief impl property
    VPL_OPT_TREE_PROPERTY(implementation_type, uint32_t, impl)

    /// @brief acceleration_mode property
    VPL_OPT_TREE_PROPERTY(implementation_via, uint32_t, acceleration_mode)

    /// @brief api_version property
    VPL_OPT_TREE_PROPERTY_PAIR(uint16_t, uint16_t, uint16_t, uint16_t, api_version)

    /// @brief license property
    VPL_OPT_TREE_PROPERTY(std::string, std::string, license)

    /// @brief keywords property
    VPL_OPT_TREE_PROPERTY(std::string, std::string, keywords)

    /// @brief vendor_id property
    VPL_OPT_TREE_PROPERTY(uint32_t, uint32_t, vendor_id)

    /// @brief vendor_impl_id property
    VPL_OPT_TREE_PROPERTY(uint32_t, uint32_t, vendor_impl_id)

    /// @brief dxgi_adapter_index property
    VPL_OPT_TREE_PROPERTY(uint32_t, uint32_t, dxgi_adapter_index)

    /// @brief implemented_function property
    VPL_OPT_TREE_LIST_PROPERTY(std::string, std::string, implemented_function)

    /// @brief pool_alloc_properties property
    VPL_OPT_TREE_PROPERTY(pool_alloction_policy, uint32_t, pool_alloc_properties)

    /// @brief set_handle property
    VPL_OPT_TREE_LIST_PROPERTY_PAIR(handle_type, void*, uint32_t, void*, handle)

    /// @brief decoder property namespace
    decoder_properties decoder;

    /// @brief encoder property namespace
    encoder_properties encoder;

    /// @brief filter property namespace
    filter_properties filter;

    /// @brief device property namespace
    device_properties device;

    /// @brief Default ctor
    properties()
        : impl_name("impl_name", "mfxImplDescription.ImplName")
        , impl("impl", "mfxImplDescription.Impl")
        , acceleration_mode("acceleration_mode", "mfxImplDescription.AccelerationMode")
        , api_version("api_version", "mfxImplDescription.ApiVersion.Major", "mfxImplDescription.ApiVersion.Minor")
        , license("license", "mfxImplDescription.License")
        , keywords("keywords", "mfxImplDescription.Keywords")
        , vendor_id("vendor_id", "mfxImplDescription.VendorID")
        , vendor_impl_id("vendor_impl_id", "mfxImplDescription.VendorImplID")
        , dxgi_adapter_index("dxgi_adapter_index", "DXGIAdapterIndex")
        , implemented_function("implemented_function", "mfxImplementedFunctions.FunctionsName")
        , pool_alloc_properties("pool_alloc_properties", "mfxImplDescription.mfxSurfacePoolMode")
        , handle("handle", "mfxHandleType", "mfxHDL")
        {}

    /// @brief Returns list of C-based properties in a form of pair: property path
    /// and property value.
    /// @return List of C-based properties
    std::vector<std::pair<std::string, detail::variant>> get_properties() const {
        std::vector<std::pair<std::string, detail::variant>> settings;
        write(settings);
        return settings;
    }

protected:
    /// @brief Write value to vector of C Properties.
    /// @param[in] dest destination vector
    void write(std::vector<std::pair<std::string, detail::variant>>& dest) const { 
        std::string prefix = "";
        impl_name.write(prefix, dest);
        impl.write(prefix, dest);
        acceleration_mode.write(prefix, dest);
        api_version.write(prefix, dest);
        license.write(prefix, dest);
        keywords.write(prefix, dest);
        vendor_id.write(prefix, dest);
        vendor_impl_id.write(prefix, dest);
        dxgi_adapter_index.write(prefix, dest);
        implemented_function.write(prefix, dest);
        pool_alloc_properties.write(prefix, dest);
        handle.write(prefix, dest);
        decoder.write(prefix, dest);
        encoder.write(prefix, dest);
        filter.write(prefix, dest);
        device.write(prefix, dest);
    }

    /// @brief Write value in human readable form to a stream
    /// @param[in] out destination stream
    /// @param[in] indent indent level
    void print(std::ostream& out, int indent) const {
        impl_name.print(out, indent);
        impl.print(out, indent);
        acceleration_mode.print(out, indent);
        api_version.print(out, indent);
        license.print(out, indent);
        keywords.print(out, indent);
        vendor_id.print(out, indent);
        vendor_impl_id.print(out, indent);
        dxgi_adapter_index.print(out, indent);
        implemented_function.print(out, indent);
        pool_alloc_properties.print(out, indent);
        handle.print(out, indent);
        decoder.print(out, indent);
        encoder.print(out, indent);
        filter.print(out, indent);
        device.print(out, indent);
    }

    friend std::ostream& operator<<(std::ostream& out, const properties& p);
};

inline std::ostream& operator<<(std::ostream& out, const properties& p) {
    p.print(out, 0);
    return out;
}

}
}