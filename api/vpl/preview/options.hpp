/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <exception>
#include <map>
#include <string>
#include <vector>
#include <algorithm>

#include "vpl/preview/detail/variant.hpp"
#include "vpl/preview/defs.hpp"
#include "vpl/preview/exception.hpp"

#include "vpl/mfxcommon.h"
#include "vpl/mfxdefs.h"

namespace oneapi {
namespace vpl {

/// @brief Represents @p implementation_selector config property. Single
/// instance of the class holds one or two C-based properties. Application
/// is not allowed to create instance of this class directly and must use one of
/// the underlying classes.
class property {
  public:
    /// @brief Default ctor
    property() : values_(), property_name_() {}

    /// @brief Copy ctor
    /// @param[in] other another object to use as data source
    property(const property &other) : values_(other.values_), property_name_(other.property_name_) {}

    /// @brief Assignment operator.
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    property &operator=(const property &other) {
        values_ = other.values_;
        property_name_ = other.property_name_;
        return *this;
    }

    /// @brief Dtor
    virtual ~property() {}

    /// @brief Return name of the child class.
    /// @return Name of the child class.
    std::string get_leaf_property_name() const {
      return property_name_;
    }

    /// @brief Returns list of C-based properties in a form of pair: property path
    /// and property value.
    /// @return List of C-based properties
    std::vector<std::pair<std::string, detail::variant>> get_properties() const {
        return values_;
    }

  protected:
    /// @brief Protected ctor.
    /// @param[in] name Path (or subpath) of the property.
    /// @param[in] value Property value.
    property(std::string name, uint16_t value) {
        values_.push_back(std::make_pair(name, detail::variant(value)));
    }

    /// @brief Protected ctor.
    /// @param[in] name Path (or subpath) of the property.
    /// @param[in] value Property value.
    property(std::string name, uint32_t value)  {
        values_.push_back(std::make_pair(name, detail::variant(value)));
    }

    /// @brief Protected ctor.
    /// @param[in] name Path (or subpath) of the property.
    /// @param[in] value Property value.
    property(std::string name, std::string_view value) {
        values_.push_back(std::make_pair(name, detail::variant((void*)value.data())));
    }

    /// @brief Protected ctor.
    /// @param[in] name Path (or subpath) of the property.
    /// @param[in] value Property value.
    property(std::string name, void* value)  {
        values_.push_back(std::make_pair(name, detail::variant(value)));
    }

    /// @brief Add connected property.
    /// @param[in] name Path (or subpath) of the property.
    /// @param[in] value Property value.
    void add_property(std::string name, uint16_t value) {
        values_.push_back(std::make_pair(name, detail::variant(value)));
    }

    /// @brief Add connected property.
    /// @param[in] name Path (or subpath) of the property.
    /// @param[in] value Property value.
    void add_property(std::string name, uint32_t value)  {
        values_.push_back(std::make_pair(name, detail::variant(value)));
    }

    /// @brief Add connected property.
    /// @param[in] name Path (or subpath) of the property.
    /// @param[in] value Property value.
    void add_property(std::string name, std::string_view value) {
        values_.push_back(std::make_pair(name, detail::variant((void*)value.data())));
    }

    /// @brief Add connected property.
    /// @param[in] name Path (or subpath) of the property.
    /// @param[in] value Property value.
    void add_property(std::string name, void* value)  {
        values_.push_back(std::make_pair(name, detail::variant(value)));
    }

    /// @brief Add connected property with different prefix.
    /// @param[in] other Another property object to add.
    /// @param[in] prefix Property prefix path.
    void add_property(const property &other, std::string prefix) {
        auto f = other.get_properties();
        std::for_each(f.begin(), f.end(), [&](auto value) {
            value.first.insert(0, prefix);
            values_.push_back(value);
        });
    }

    /// List of C-based properties in a form of pair: property path
    /// and property value.
    std::vector<std::pair<std::string, detail::variant>> values_;

    /// Name of the child class.
    std::string property_name_;
};

namespace dprops {

/// @brief Holds "implementation type" property. This is first level property.
class impl : public property {
  public:
    /// @brief Property value type
    typedef implementation_type value_type;

    /// @brief Constructs property.
    /// @param[in] implType Type of the implementation.
    impl(implementation_type implType) : property("mfxImplDescription.Impl", (uint32_t)implType) {
        property_name_ = "impl";
    }
};

/// @brief Holds "acceleration" property. This is first level property.
class acceleration_mode : public property {
  public:
    /// @brief Property value type
    typedef implementation_via value_type;

    /// @brief Constructs property.
    /// @param[in] implementation_via Acceleration mode.
    acceleration_mode(implementation_via implVia) : property("mfxImplDescription.AccelerationMode", (uint32_t)implVia) {
        property_name_ = "acceleration_mode";
    }
};

/// @brief Holds API version property. This is first level property.
class api_version : public property {
  public:
    /// @brief Property value type
    typedef std::pair<uint16_t, uint16_t> value_type;

    /// @brief Constructs property.
    /// @param[in] major Major API version.
    /// @param[in] minor Minor API version.
    api_version(uint16_t major, uint16_t minor) : property("mfxImplDescription.ApiVersion.Major", major) {
        add_property("mfxImplDescription.ApiVersion.Minor", minor);
        property_name_ = "api_version";
    }
    /// @brief Constructs property.
    /// @param[in] version API version as {Major, Minor} pair
    api_version(std::pair<uint16_t, uint16_t> version) : property("mfxImplDescription.ApiVersion.Major", std::get<0>(version)) {
        add_property("mfxImplDescription.ApiVersion.Minor", std::get<1>(version));
        property_name_ = "api_version";
    }
};

/// @brief Holds "implementation name" property. This is first level property.
class impl_name : public property {
  public:
    /// @brief Property value type
    typedef std::string_view value_type;

    /// @brief Constructs property.
    /// @param[in] implName Name of the implementation.
    impl_name(std::string_view implName) : property("mfxImplDescription.ImplName", implName) {
        property_name_ = "impl_name";
    }
};

/// @brief Holds licence property. This is first level property.
class license : public property {
  public:
    /// @brief Property value type
    typedef std::string value_type;

    /// @brief Constructs property.
    /// @param[in] License Licenses of the implementation.
    license(std::string License) : property("mfxImplDescription.License", License) {
        property_name_ = "license";
    }
};

/// @brief Holds "keywords" property. This is first level property.
class keywords : public property {
  public:
    /// @brief Property value type
    typedef std::string value_type;

    /// @brief Constructs property.
    /// @param[in] Keywords Keywords.
    keywords(std::string Keywords) : property("mfxImplDescription.Keywords", Keywords) {
        property_name_ = "keywords";
    }
};

/// @brief Holds "vendor ID" property. This is first level property.
class vendor_id : public property {
  public:
    /// @brief Property value type
    typedef uint32_t value_type;

    /// @brief Constructs property.
    /// @param[in] VendorID vendor ID.
    vendor_id(uint32_t VendorID) : property("mfxImplDescription.VendorID", VendorID) {
        property_name_ = "vendor_id";
    }
};

/// @brief Holds "vendor implementation ID" property. This is first level property.
class vendor_impl_id : public property {
  public:
    /// @brief Property value type
    typedef uint32_t value_type;

    /// @brief Constructs property.
    /// @param[in] VendorImplID vendor's implementation ID.
    vendor_impl_id(uint32_t VendorImplID) : property("mfxImplDescription.VendorImplID", VendorImplID) {
        property_name_ = "vendor_impl_id";
    }
};

/// @brief Holds "codec ID" property. This is nested property.
class codec_id : public property {
  public:
    /// @brief Property value type
    typedef codec_format_fourcc value_type;

    /// @brief Constructs property.
    /// @param[in] fourcc Codec ID.
    codec_id(codec_format_fourcc codecID) : property("CodecID", (uint32_t)codecID) {
        property_name_ = "codec_id";
    }
};

/// @brief Holds "max codec level" property. This is nested property.
class max_codec_level : public property {
  public:
    /// @brief Property value type
    typedef uint32_t value_type;

    /// @brief Constructs property.
    /// @param[in] MaxLevel Maximum supported codec level.
    max_codec_level(uint32_t MaxLevel) : property("MaxcodecLevel", MaxLevel) {
        property_name_ = "max_codec_level";
    }
};

/// @brief Holds bi-directional prediction encoder's property. This is nested property.
class bidirectional_prediction : public property {
  public:
    /// @brief Property value type
    typedef uint16_t value_type;

    /// @brief Constructs property.
    /// @param[in] pred Ability to perform bi-direction prediction during encoding.
    bidirectional_prediction(uint16_t pred) : property("BiDirectionalPrediction", pred) {
        property_name_ = "bidirectional_prediction";
    }
};

/// @brief Holds VPP "filter ID" property. This is nested property.
class filter_id : public property {
  public:
    /// @brief Property value type
    typedef uint32_t value_type;

    /// @brief Constructs property.
    /// @param[in] filter_id VPP filter ID.
    /// @todo Introduce enum.
    filter_id(uint32_t fourcc) : property("FilterFourCC", (uint32_t)fourcc) {
        property_name_ = "filter_id";
    }
};

/// @brief Holds VPP's filter "max delay in frames" property. This is nested property.
class max_delay_in_frames : public property {
  public:
    /// @brief Property value type
    typedef uint16_t value_type;

    /// @brief Constructs property.
    /// @param[in] delay Delay in frames.
    max_delay_in_frames(uint16_t delay) : property("MaxDelayInFrames", delay) {
        property_name_ = "max_delay_in_frames";
    }
};

/// @brief Holds "device ID" property. This is nested property.
class device_id : public property {
  public:
    /// @brief Property value type
    typedef std::string value_type;

    /// @brief Constructs property.
    /// @param[in] deviceid Device ID.
    device_id(std::string deviceid) : property("DeviceID", deviceid) {
        property_name_ = "device_id";
    }
};

/// @brief Holds media adapter property. This is nested property.
class media_adapter : public property {
  public:
    /// @brief Property value type
    typedef media_adapter_type value_type;

    /// @brief Constructs property.
    /// @param[in] type Media adapter type.
    media_adapter(media_adapter_type type) : property("MediaAdapterType", (uint16_t)type) {
        property_name_ = "media_adapter";
    }
};

/// @brief Holds "DXGI adapter index" property. This is first level property.
/// Applicable only on Windows OS.
class dxgi_adapter_index : public property {
  public:
    /// @brief Property value type
    typedef uint32_t value_type;

    /// @brief Constructs property.
    /// @param[in] index DXGI adapter index.
    dxgi_adapter_index(uint32_t index) : property("DXGIAdapterIndex", index) {
        property_name_ = "dxgi_adapter_index";
    }
};

/// @brief Holds "implemented function" property. This is first level property.
class implemented_function : public property {
  public:
    /// @brief Property value type
    typedef std::string value_type;

    /// @brief Constructs property.
    /// @param[in] funcationName Name of the oneVPL C API function.
    implemented_function(std::string funcationName) : property("mfxImplementedFunctions.FunctionsName", funcationName) {
        property_name_ = "implemented_function";
    }
};

/// @brief Holds surface "pool allocation mode" property. This is first level property.
class pool_alloc_properties : public property {
  public:
    /// @brief Property value type
    typedef pool_alloction_policy value_type;

    /// @brief Constructs property.
    /// @param[in] policy Pool allocation mode.
    pool_alloc_properties(pool_alloction_policy policy) : property("mfxImplDescription.mfxSurfacePoolMode", (uint32_t)policy) {
        property_name_ = "pool_alloc_properties";
    }
};

/// @brief Sets handle. This is first level property.
class set_handle : public property {
  public:
    /// @brief Property value type
    typedef std::pair<handle_type, void*> value_type;

    /// @brief Constructs property.
    /// @param[in] type Type of the handle.
    /// @param[in] handle Handle.
    set_handle(handle_type type, void* handle) : property("mfxHandleType", (uint32_t)type) {
        add_property("mfxHDL", handle);
        property_name_ = "set_handle";
    }
    /// @brief Constructs property.
    /// @param[in] version handle as {Type, Handle} pair
    set_handle(std::pair<handle_type, void*> handle) : property("mfxHandleType", (uint32_t)std::get<0>(handle)) {
        add_property("mfxHDL", std::get<1>(handle));
        property_name_ = "frame_size";
    }
};

/// @brief Holds surface's "memory type" property. This is nested property.
class mem_type : public property {
  public:
    /// @brief Property value type
    typedef resource_type value_type;

    /// @brief Constructs property.
    /// @param[in] MemType Memory type.
    mem_type(resource_type MemType) : property("MemHandleType", (uint32_t)MemType) {
        property_name_ = "mem_type";
    }
};

/// @brief Holds surface's "frame size" property. This is nested property.
class frame_size : public property {
  public:
    /// @brief Property value type
    typedef std::pair<uint32_t, uint32_t> value_type;

    /// @brief Constructs property.
    /// @param[in] width Width of the frame.
    /// @param[in] height Height of the frame.
    frame_size(uint32_t width, uint32_t height) : property("Width", width) {
        add_property("Height", height);
        property_name_ = "frame_size";
    }
    /// @brief Constructs property.
    /// @param[in] version Size as {Width, Height} pair
    frame_size(std::pair<uint32_t, uint32_t> size) : property("Width", std::get<0>(size)) {
        add_property("Height", std::get<1>(size));
        property_name_ = "frame_size";
    }
};

/// @brief Holds encoder's or decoder's "color format" property. This is nested property.
class color_format : public property {
  public:
    /// @brief Property value type
    typedef color_format_fourcc value_type;

    /// @brief Constructs property.
    /// @param[in] format Color format.
    color_format(color_format_fourcc format) : property("ColorFormats", (uint32_t)format) {
        property_name_ = "color_format";
    }
};

/// @brief Holds input for VPP "color format" property. This is nested property.
class in_color_format : public property {
  public:
    /// @brief Property value type
    typedef color_format_fourcc value_type;

    /// @brief Constructs property.
    /// @param[in] format Color format.
    in_color_format(color_format_fourcc format) : property("format.InFormat", (uint32_t)format) {
        property_name_ = "in_color_format";
    }
};

/// @brief Holds output from VPP "color format" property. This is nested property.
class out_color_format : public property {
  public:
    /// @brief Property value type
    typedef color_format_fourcc value_type;

    /// @brief Constructs property.
    /// @param[in] format Color format.
    out_color_format(color_format_fourcc format) : property("format.OutFormats", (uint32_t)format) {
        property_name_ = "out_color_format";
    }
};

/// @brief Codec profile property. This is nested property.
class profile : public property {
  public:
    /// @brief Property value type
    typedef uint32_t value_type;

    /// @brief Constructs property.
    /// @param[in] profile Codec's profile.
    profile(uint32_t profile) : property("Profile", profile) {
        property_name_ = "profile";
    }
};

/// @brief Constructs and holds list of allowed properties. Application
/// should not directly create instance of this class. One of the classes
/// below must be used for this operation.
class container : public property {
  protected:
    /// @brief Default ctor.
    /// @param[in] props List of properties.
    /// @param[in] prefix Properties base prefix.
    /// @param[in] allowed_list List of allowed properties.
    container(std::initializer_list<property> props, std::string prefix, std::vector<std::string> allowed_list = {}) : property(), props_(props), prefix_(prefix), allowed_list_(allowed_list) {
        ctor_helper();
    }

    /// @brief Ctor dedicated for the Python binding.
    /// @param[in] props List of properties.
    /// @param[in] prefix Properties base prefix.
    /// @param[in] allowed_list List of allowed properties.
    container(std::vector<property> props, std::string prefix, std::vector<std::string> allowed_list = {}) : property(), props_(props), prefix_(prefix), allowed_list_(allowed_list) {
        ctor_helper();
    }

    /// @brief Check if property is allowed to be added to the list.
    void is_valid(property prop)  {
        if(std::any_of(allowed_list_.begin(), allowed_list_.end(), [&] (std::string propName) {
            return (propName == prop.get_leaf_property_name());
        })) {
          return;
        }
        throw base_exception("Unsupported property: " + prop.get_leaf_property_name(), MFX_ERR_UNSUPPORTED);
        return;
    }

    /// @brief Helper method for the constructors.
    void ctor_helper() {
        std::for_each(props_.begin(), props_.end(), [&] (auto prop) {
            is_valid(prop);
            add_property(prop, prefix_);
        });
    }

    /// List of properties.
    std::vector<property> props_;

    /// Properties base prefix.
    std::string prefix_;

    /// List of allowed properties.
    std::vector<std::string> allowed_list_;
};

/// @brief Holds list of properies for the decoder.
class decoder : public container {
  public:
    /// @brief Default ctor.
    /// @param[in] props List of properies to add.
    decoder(std::initializer_list<property> props) : container(props, "mfxImplDescription.mfxDecoderDescription.decoder.", {"codec_id", "max_codec_level", "dec_profile"}) {
        property_name_ = "decoder";
    }

    /// @brief Ctor for Python bindings.
    /// @param[in] props List of properies to add.
    decoder(std::vector<property> props) : container(props, "mfxImplDescription.mfxDecoderDescription.decoder.", {"codec_id", "max_codec_level", "dec_profile"}) {
        property_name_ = "decoder";
    }
};

/// @brief Holds list of properies for the encoder.
class encoder : public container {
  public:
    /// @brief Default ctor.
    /// @param[in] props List of properies to add.
    encoder(std::initializer_list<property> props) : container(props, "mfxImplDescription.mfxEncoderDescription.encoder.", {"codec_id", "max_codec_level", "bidirectional_prediction", "enc_profile"}) {
        property_name_ = "encoder";
    }

    /// @brief Ctor for Python bindings.
    /// @param[in] props List of properies to add.
    encoder(std::vector<property> props) : container(props, "mfxImplDescription.mfxEncoderDescription.encoder.", {"codec_id", "max_codec_level", "bidirectional_prediction", "enc_profile"}) {
        property_name_ = "encoder";
    }
};

/// @brief Holds list of properies for the VPP.
class filter : public container {
  public:
    /// @brief Default ctor.
    /// @param[in] props List of properies to add.
    filter(std::initializer_list<property> props) : container(props, "mfxImplDescription.mfxVPPDescription.filter.", {"filter_id", "max_delay_in_frames", "memdesc"}) {
        property_name_ = "filter";
    }

    /// @brief Ctor for Python bindings.
    /// @param[in] props List of properies to add.
    filter(std::vector<property> props) : container(props, "mfxImplDescription.mfxVPPDescription.filter.", {"filter_id", "max_delay_in_frames", "memdesc"}) {
        property_name_ = "filter";
    }
};

/// @brief Holds list of properies for the device.
class device : public container {
  public:
    /// @brief Default ctor.
    /// @param[in] props List of properies to add.
    device(std::initializer_list<property> props) : container(props, "mfxImplDescription.mfxDeviceDescription.device.", {"device_id", "media_adapter"}) {
        property_name_ = "device";
    }

    /// @brief Ctor for Python bindings.
    /// @param[in] props List of properies to add.
    device(std::vector<property> props) : container(props, "mfxImplDescription.mfxDeviceDescription.device.", {"device_id", "media_adapter"}) {
        property_name_ = "device";
    }
};

/// @brief Holds memory description list of properies for the VPP.
class memdesc : public container {
  public:
    /// @brief Default ctor.
    /// @param[in] props List of properies to add.
    memdesc(std::initializer_list<property> props) : container(props, "memdesc.", {"mem_type", "frame_size", "in_color_format", "out_color_format"}) {
        property_name_ = "memdesc";
    }

    /// @brief Ctor for Python bindings.
    /// @param[in] props List of properies to add.
    memdesc(std::vector<property> props) : container(props, "memdesc.", {"mem_type", "frame_size", "in_color_format", "out_color_format"}) {
        property_name_ = "memdesc";
    }
};

/// @brief Holds decoder profile related properties.
class dec_profile : public container {
  public:
    /// @brief Default ctor.
    /// @param[in] props List of properies to add.
    dec_profile(std::initializer_list<property> props) : container(props, "decprofile.", {"profile", "dec_mem_desc"}) {
        property_name_ = "dec_profile";
    }

    /// @brief Ctor for Python bindings.
    /// @param[in] props List of properies to add.
    dec_profile(std::vector<property> props) : container(props, "decprofile.", {"profile", "dec_mem_desc"}) {
        property_name_ = "dec_profile";
    }
};

/// @brief Holds encoder profile related properties.
class enc_profile : public container {
  public:
    /// @brief Default ctor.
    /// @param[in] props List of properies to add.
    enc_profile(std::initializer_list<property> props) : container(props, "encprofile.", {"profile", "enc_mem_desc"}) {
        property_name_ = "enc_profile";
    }

    /// @brief Ctor for Python bindings.
    /// @param[in] props List of properies to add.
    enc_profile(std::vector<property> props) : container(props, "encprofile.", {"profile", "enc_mem_desc"}) {
        property_name_ = "enc_profile";
    }
};

/// @brief Holds memory description list of properies for the decoder.
class dec_mem_desc : public container {
  public:
    /// @brief Default ctor.
    /// @param[in] props List of properies to add.
    dec_mem_desc(std::initializer_list<property> props) : container(props, "decmemdesc.", {"mem_type", "frame_size", "color_format"}) {
        property_name_ = "dec_mem_desc";
    }

    /// @brief Ctor for Python bindings.
    /// @param[in] props List of properies to add.
    dec_mem_desc(std::vector<property> props) : container(props, "decmemdesc.", {"mem_type", "frame_size", "color_format"}) {
        property_name_ = "dec_mem_desc";
    }
};

/// @brief Holds memory description list of properies for the encoder.
class enc_mem_desc : public container {
  public:
    /// @brief Default ctor.
    /// @param[in] props List of properies to add.
    enc_mem_desc(std::initializer_list<property> props) : container(props, "encmemdesc.", {"mem_type", "frame_size", "color_format"}) {
        property_name_ = "enc_mem_desc";
    }

    /// @brief Ctor for Python bindings.
    /// @param[in] props List of properies to add.
    enc_mem_desc(std::vector<property> props) : container(props, "encmemdesc.", {"mem_type", "frame_size", "color_format"}) {
        property_name_ = "enc_mem_desc";
    }
};

} // namespace dprops


/// List of first class properies allowed to be included in to the @p property_list list.
static const std::vector<std::string> first_class_options = { "impl_name", "impl",
                                                              "acceleration_mode", "api_version",
                                                              "license", "keywords",
                                                              "vendor_id", "vendor_impl_id",
                                                              "dxgi_adapter_index",
                                                              "implemented_function",
                                                              "pool_alloc_properties",
                                                              "set_handle", "decoder", "encoder",
                                                              "filter", "device"};

/// @brief List of propeties for the @p implementation_selector class.
/// This is application's starting point.
class property_list : public dprops::container {
  public:
    /// @brief Default ctor.
    /// @param[in] props List of properies to add.
    property_list(std::initializer_list<property> props) : container(props, "", first_class_options) {
        property_name_ = "property_list";
    }

    /// @brief Ctor for Python bindings.
    /// @param[in] props List of properies to add.
    property_list(std::vector<property> props) : container(props, "", first_class_options) {
        property_name_ = "property_list";
    }
};

} // namespace vpl
} // namespace oneapi
