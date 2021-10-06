//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include <vector>
#include <optional>
#include "vpl/preview/options.hpp"
#include "vpl/preview/defs.hpp"
#include "vpl/preview/detail/string_helpers.hpp"

#pragma once

namespace oneapi {
namespace vpl {

/// @brief Represents a settable property.
template<typename PT> class property_value : public std::optional<typename PT::value_type>{
public:
    /// @brief This class type
    using this_type = property_value<PT>;

    /// @brief Property type
    using property_type = PT;

    /// @brief Item type
    using item_type = typename PT::value_type;

    /// @brief Property value type
    using value_type = typename PT::value_type;

    using std::optional<value_type>::value;

    using std::optional<value_type>::optional;

    /// @brief Assignment operator.
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    this_type& operator=(this_type const& other){
        std::optional<value_type>::operator=(other);
        return *this;
    }

    /// @brief Assignment operator.
    /// @param[in] value value of object
    /// @returns Reference to this object
    this_type& operator=(value_type const& value){
        std::optional<value_type>::operator=(value);
        return *this;
    }

    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    void write(std::vector<property>& dest) const { 
        if(this->has_value()){
            dest.push_back(property_type(value()));
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
                << property_type(item_type()).get_leaf_property_name()
                << " = ";
            out << this->value();
            out << std::endl;
        }
    }
};

/// @brief Represents a settable property.
template<typename PT> class list_property_value : public std::optional<std::vector<typename PT::value_type>>{
public:
    /// @brief This class type
    using this_type = list_property_value<PT>;

    /// @brief Property type
    using property_type = PT;

    /// @brief Item type
    using item_type = typename PT::value_type;

    /// @brief Property value type
    using value_type = std::vector<typename PT::value_type>;

    using std::optional<value_type>::value;

    using std::optional<value_type>::optional;

    /// @brief Assignment operator.
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    this_type& operator=(this_type const& other){
        std::optional<value_type>::operator=(other);
        return *this;
    }

    /// @brief Assignment operator.
    /// @param[in] value value of object
    /// @returns Reference to this object
    this_type& operator=(value_type const& value){
        std::optional<value_type>::operator=(value);
        return *this;
    }

    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    void write(std::vector<property>& dest) const { 
        if(this->has_value()){
            for(auto&v : value()){
                dest.push_back(property_type(v));
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
                << property_type(item_type()).get_leaf_property_name()
                << " = ";
            out << this->value();
            out << std::endl;
        }
    }
};

#define VPL_OPT_TREE_PROPERTY(NAME) \
    property_value<dprops::NAME> NAME; \
    decltype(NAME)::value_type& get_##NAME(){ return NAME.value(); } \
    void set_##NAME(decltype(NAME)::value_type v){ NAME = v; }

#define VPL_OPT_TREE_LIST_PROPERTY(NAME) \
    list_property_value<dprops::NAME> NAME; \
    decltype(NAME)::value_type& get_##NAME(){ return NAME.value(); } \
    void set_##NAME(decltype(NAME)::value_type v){ NAME = v; }

/// @brief Represents the namespace of configurable encode/decode memory decriptor properties. See
/// @p implementation_selector properties for more detail.
class codec_mem_desc_properties {
public:
    /// @brief mem_type property
    VPL_OPT_TREE_PROPERTY(mem_type)

    /// @brief frame_size property
    VPL_OPT_TREE_PROPERTY(frame_size)

    /// @brief color_format property
    VPL_OPT_TREE_PROPERTY(color_format)
};

/// @brief Represents the namespace of configurable encoder specific memory decriptor properties. See
/// @p implementation_selector properties for more detail.
class enc_mem_desc_properties : public codec_mem_desc_properties{
public:
    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    void write(std::vector<property>& dest) const { 
        std::vector<property> props;
        mem_type.write(props);
        frame_size.write(props);
        color_format.write(props);
        dest.push_back(dprops::enc_mem_desc(props));
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
    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    void write(std::vector<property>& dest) const { 
        std::vector<property> props;
        mem_type.write(props);
        frame_size.write(props);
        color_format.write(props);
        dest.push_back(dprops::dec_mem_desc(props));
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
class dec_profile_properties {
public:
    /// @brief profile property
    VPL_OPT_TREE_PROPERTY(profile)

    /// @brief dec_mem_desc property namespace
    dec_mem_desc_properties dec_mem_desc;

    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    void write(std::vector<property>& dest) const { 
        std::vector<property> props;
        profile.write(props);
        dec_mem_desc.write(props);
        dest.push_back(dprops::dec_profile(props));
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
class memdesc_properties {
public:
    /// @brief mem_type property
    VPL_OPT_TREE_PROPERTY(mem_type)

    /// @brief frame_size property
    VPL_OPT_TREE_PROPERTY(frame_size)

    /// @brief in_color_format property
    VPL_OPT_TREE_PROPERTY(in_color_format)

    /// @brief out_color_format property
    VPL_OPT_TREE_PROPERTY(out_color_format)

    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    void write(std::vector<property>& dest) const { 
        std::vector<property> props;
        mem_type.write(props);
        frame_size.write(props);
        in_color_format.write(props);
        out_color_format.write(props);
        dest.push_back(dprops::memdesc(props));
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
class enc_profile_properties {
public:
    /// @brief profile property
    VPL_OPT_TREE_PROPERTY(profile)

    /// @brief enc_mem_desc property namespace
    enc_mem_desc_properties enc_mem_desc;

    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    void write(std::vector<property>& dest) const { 
        std::vector<property> props;
        profile.write(props);
        enc_mem_desc.write(props);
        dest.push_back(dprops::enc_profile(props));
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
class decoder_properties {
public:
    /// @brief codec_id property
    VPL_OPT_TREE_LIST_PROPERTY(codec_id)

    /// @brief max_codec_level property
    VPL_OPT_TREE_PROPERTY(max_codec_level)

    /// @brief dec_profile property namespace
    dec_profile_properties dec_profile;

    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    void write(std::vector<property>& dest) const { 
        std::vector<property> props;
        codec_id.write(props);
        max_codec_level.write(props);
        dec_profile.write(props);
        dest.push_back(dprops::decoder(props));
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
class encoder_properties {
public:
    /// @brief codec_id property
    VPL_OPT_TREE_LIST_PROPERTY(codec_id)

    /// @brief max_codec_level property
    VPL_OPT_TREE_PROPERTY(max_codec_level)

    /// @brief bidirectional_prediction property
    VPL_OPT_TREE_PROPERTY(bidirectional_prediction)

    /// @brief enc_profile property namespace
    enc_profile_properties enc_profile;

    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    void write(std::vector<property>& dest) const { 
        std::vector<property> props;
        codec_id.write(props);
        max_codec_level.write(props);
        bidirectional_prediction.write(props);
        enc_profile.write(props);
        dest.push_back(dprops::encoder(props));
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
class filter_properties {
public:
    /// @brief filter_id property
    VPL_OPT_TREE_LIST_PROPERTY(filter_id)

    /// @brief max_delay_in_frames property
    VPL_OPT_TREE_PROPERTY(max_delay_in_frames)

    /// @brief memdesc property namespace
    memdesc_properties memdesc;

    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    void write(std::vector<property>& dest) const {
        std::vector<property> props;
        filter_id.write(props);
        max_delay_in_frames.write(props);
        memdesc.write(props);
        dest.push_back(dprops::filter(props));
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
class device_properties {
public:
    /// @brief device_id property
    VPL_OPT_TREE_PROPERTY(device_id)

    /// @brief media_adapter property
    VPL_OPT_TREE_PROPERTY(media_adapter)

    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    void write(std::vector<property>& dest) const { 
        std::vector<property> props;
        device_id.write(props);
        media_adapter.write(props);
        dest.push_back(dprops::device(props));
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
class properties {
public:
    /// @brief Default ctor
    properties(){};

    /// @brief impl_name property
    VPL_OPT_TREE_PROPERTY(impl_name)

    /// @brief impl property
    VPL_OPT_TREE_PROPERTY(impl)

    /// @brief acceleration_mode property
    VPL_OPT_TREE_PROPERTY(acceleration_mode)

    /// @brief api_version property
    VPL_OPT_TREE_PROPERTY(api_version)

    /// @brief license property
    VPL_OPT_TREE_PROPERTY(license)

    /// @brief keywords property
    VPL_OPT_TREE_PROPERTY(keywords)

    /// @brief vendor_id property
    VPL_OPT_TREE_PROPERTY(vendor_id)

    /// @brief vendor_impl_id property
    VPL_OPT_TREE_PROPERTY(vendor_impl_id)

    /// @brief dxgi_adapter_index property
    VPL_OPT_TREE_PROPERTY(dxgi_adapter_index)

    /// @brief implemented_function property
    VPL_OPT_TREE_LIST_PROPERTY(implemented_function)

    /// @brief pool_alloc_properties property
    VPL_OPT_TREE_PROPERTY(pool_alloc_properties)

    /// @brief set_handle property
    VPL_OPT_TREE_LIST_PROPERTY(set_handle)

    /// @brief decoder property namespace
    decoder_properties decoder;

    /// @brief encoder property namespace
    encoder_properties encoder;

    /// @brief filter property namespace
    filter_properties filter;

    /// @brief device property namespace
    device_properties device;

    /// @brief Type conversion to property_list
    /// @returns property_list representing the current property settings
    operator property_list(){
        std::vector<property> props;
        write(props);
        return oneapi::vpl::property_list(props);
    }

    /// @brief Write value to vector of @p property.
    /// @param[in] dest destination vector
    void write(std::vector<property>& dest) const { 
        impl_name.write(dest);
        impl.write(dest);
        acceleration_mode.write(dest);
        api_version.write(dest);
        license.write(dest);
        keywords.write(dest);
        vendor_id.write(dest);
        vendor_impl_id.write(dest);
        dxgi_adapter_index.write(dest);
        implemented_function.write(dest);
        pool_alloc_properties.write(dest);
        set_handle.write(dest);
        decoder.write(dest);
        encoder.write(dest);
        filter.write(dest);
        device.write(dest);
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
        set_handle.print(out, indent);
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