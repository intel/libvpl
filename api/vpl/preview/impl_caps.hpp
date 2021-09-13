/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "vpl/preview/defs.hpp"
#include "vpl/preview/exception.hpp"

#include "vpl/mfxcommon.h"

#include "vpl/preview/detail/string_helpers.hpp"

namespace oneapi {
namespace vpl {

/// @brief Base class to store implementation capabilities
class base_implementation_capabilities {
public:
    /// @brief Default ctor
    /// @param ID of the implementation capabilities format ID.
    explicit base_implementation_capabilities(uint32_t ID) : id_(ID) {}

    /// @brief returns implementation capabilities format ID.
    /// @return ID of the implementation capabilities format type.
    uint32_t get_id() const {
        return id_;
    }

protected:
    /// ID of the implementation capabilities format type.
    uint32_t id_;
};

/// @brief Provides information about supported pool policies
class pool_policies {
public:
    /// @brief Default ctor
    /// @param pool policies
    explicit pool_policies(mfxPoolPolicyDescription* base) : base_(base) {}

    /// @brief Provides list of supported policies.
    /// @return list of policies.
    std::vector<mfxPoolAllocationPolicy> policies() const {
        std::vector<mfxPoolAllocationPolicy> result;

        std::for_each(base_->Policy,
                        base_->Policy + base_->NumPoolPolicies,
                        [&](auto item) {
                            result.push_back(item);
                        });

        return result;
    }
protected:
    mfxPoolPolicyDescription* base_;
};

/// @brief Store and parse implementation capabilities in a form of implementation description structure
class implementation_capabilities : public base_implementation_capabilities {
public:
    /// @brief Default ctor
    /// @param caps pointer to raw data.
    explicit implementation_capabilities(mfxImplDescription *caps)
            : base_implementation_capabilities(MFX_IMPLCAPS_IMPLDESCSTRUCTURE),
              caps_(caps) {}

public:
    /// @brief Provides information about supported memory for codecs
    /// @tparam T Encoder of decoder identifier.
    template <typename T>
    class codec_memory {
    public:
        /// @brief Default ctor
        /// @param memdesc Memory description
        explicit codec_memory(T memdesc) : memdesc_(memdesc) {}

        /// @brief provides supported memory type.
        /// @return Memory type.
        mfxResourceType get_mem_type() const {
            return memdesc_.MemHandleType;
        }

        /// @brief provides minimal supported width and height.
        /// @return minimal supported width and height.
        auto get_min_size() const {
            return std::pair(memdesc_.Width.Min, memdesc_.Height.Min);
        }

        /// @brief provides maximal supported width and height.
        /// @return maximal supported width and height.
        auto get_max_size() const {
            return std::pair(memdesc_.Width.Max, memdesc_.Height.Max);
        }

        /// @brief provides increment value for width and height.
        /// @return increment value for width and height.
        auto get_size_step() const {
            return std::pair(memdesc_.Width.Step, memdesc_.Height.Step);
        }

        /// @brief Provides list of supported output memory types.
        /// @return List of supported output memory types.
        std::vector<uint32_t> get_out_mem_types() const {
            return std::vector<uint32_t>(memdesc_.ColorFormats,
                                         memdesc_.ColorFormats + memdesc_.NumColorFormats);
        }

    protected:
        /// Raw data
        T memdesc_;
    };

    /// Defines decoder memory description class
    typedef codec_memory<mfxDecoderDescription::decoder::decprofile::decmemdesc> decoder_memory;
    /// Defines encoder memory description class
    typedef codec_memory<mfxEncoderDescription::encoder::encprofile::encmemdesc> encoder_memory;

    /// @brief Provides information about supported memory formats for VPP filters
    class vpp_memory_format {
    public:
        /// @brief Default ctor
        /// @param fmt Memory format
        explicit vpp_memory_format(mfxVPPDescription::filter::memdesc::format fmt) : fmt_(fmt) {}

        /// @brief Returns input memory type in a form of FourCC code
        /// @return input memory type in a form of FourCC code
        uint32_t get_input_format() const {
            return fmt_.InFormat;
        }

        /// @brief Returns iterable list of output memory types in a form of FourCC code
        /// @return list of output memory types
        std::vector<uint32_t> get_out_format() const {
            return std::vector<uint32_t>(fmt_.OutFormats, fmt_.OutFormats + fmt_.NumOutFormat);
        }

    protected:
        /// Raw data
        mfxVPPDescription::filter::memdesc::format fmt_;
    };

    /// @brief Provides information about supported memory for VPP filters
    class vpp_memory {
    public:
        /// @brief Default ctor
        /// @param memdesc Memory description
        explicit vpp_memory(mfxVPPDescription::filter::memdesc memdesc) : memdesc_(memdesc) {}

        /// @brief provides supported memory type.
        /// @return Memory type.
        mfxResourceType get_mem_type() const {
            return memdesc_.MemHandleType;
        }

        /// @brief provides minimal supported width and height.
        /// @return minimal supported width and height.
        auto get_min_size() const {
            return std::pair(memdesc_.Width.Min, memdesc_.Height.Min);
        }

        /// @brief provides maximal supported width and height.
        /// @return maximal supported width and height.
        auto get_max_size() const {
            return std::pair(memdesc_.Width.Max, memdesc_.Height.Max);
        }

        /// @brief provides increment value for width and height.
        /// @return increment value for width and height.
        auto get_size_step() const {
            return std::pair(memdesc_.Width.Step, memdesc_.Height.Step);
        }

        /// @brief Provides list of supported memory formats.
        /// @return List of supported memory formats.
        std::vector<vpp_memory_format> get_memory_formats() const {
            std::vector<vpp_memory_format> formats;

            std::for_each(memdesc_.Formats,
                          memdesc_.Formats + memdesc_.NumInFormats,
                          [&](mfxVPPDescription::filter::memdesc::format fmt) {
                              formats.push_back(vpp_memory_format(fmt));
                          });

            return formats;
        }

    protected:
        /// Raw data
        mfxVPPDescription::filter::memdesc memdesc_;
    };

    /// @brief Provides information about supported profiles by the decoder
    class decoder_profile {
    public:
        /// @brief Default ctor
        /// @param profile Decoder profile
        explicit decoder_profile(mfxDecoderDescription::decoder::decprofile profile)
                : profile_(profile) {}

        /// @brief provides ID of the profile.
        /// @return ID of the profile.
        uint16_t get_profile() const {
            return profile_.Profile;
        }

        /// @brief Provides list of memory types.
        /// @return list of memory types.
        std::vector<decoder_memory> get_decoder_mem_types() const {
            std::vector<decoder_memory> memories;

            std::for_each(profile_.MemDesc,
                          profile_.MemDesc + profile_.NumMemTypes,
                          [&](mfxDecoderDescription::decoder::decprofile::decmemdesc memdesc) {
                              memories.push_back(decoder_memory(memdesc));
                          });

            return memories;
        }

    protected:
        /// Raw data
        mfxDecoderDescription::decoder::decprofile profile_;
    };

    /// @brief Provides information about supported profiles by the encoder
    class encoder_profile {
    public:
        /// @brief Default ctor
        /// @param profile Encoder profile
        explicit encoder_profile(mfxEncoderDescription::encoder::encprofile profile)
                : profile_(profile) {}

        /// @brief provides ID of the profile.
        /// @return ID of the profile.
        uint16_t get_profile() const {
            return profile_.Profile;
        }

        /// @brief Provides list of memory types.
        /// @return list of memory types.
        std::vector<encoder_memory> get_encoder_mem_types() const {
            std::vector<encoder_memory> memories;

            std::for_each(profile_.MemDesc,
                          profile_.MemDesc + profile_.NumMemTypes,
                          [&](mfxEncoderDescription::encoder::encprofile::encmemdesc memdesc) {
                              memories.push_back(encoder_memory(memdesc));
                          });

            return memories;
        }

    protected:
        /// Raw data
        mfxEncoderDescription::encoder::encprofile profile_;
    };

    /// @brief Provides information about supported decoders
    class decoder {
    public:
        /// @brief Default ctor
        /// @param dec decoder
        explicit decoder(mfxDecoderDescription::decoder dec) : dec_(dec) {}

        /// @brief Provides codec ID.
        /// @return Codec ID.
        uint32_t get_codec_id() const {
            return dec_.CodecID;
        }

        /// @brief Provides maximum supported codec's level.
        /// @return maximum supported codec's level.
        uint16_t get_max_codec_level() const {
            return dec_.MaxcodecLevel;
        }

        /// @brief Provides list of supported profiles.
        /// @return list of profiles.
        std::vector<decoder_profile> get_profiles() const {
            std::vector<decoder_profile> profiles;

            std::for_each(dec_.Profiles,
                          dec_.Profiles + dec_.NumProfiles,
                          [&](mfxDecoderDescription::decoder::decprofile profile) {
                              profiles.push_back(decoder_profile(profile));
                          });

            return profiles;
        }

    protected:
        /// Raw data
        mfxDecoderDescription::decoder dec_;
    };

    /// @brief Provides information about supported encoders
    class encoder {
    public:
        /// @brief Default ctor
        /// @param enc encoder
        explicit encoder(mfxEncoderDescription::encoder enc) : enc_(enc) {}

        /// @brief Provides codec ID.
        /// @return Codec ID.
        uint32_t get_codec_id() const {
            return enc_.CodecID;
        }

        /// @brief Provides maximum supported codec's level.
        /// @return maximum supported codec's level.
        uint16_t get_max_codec_level() const {
            return enc_.MaxcodecLevel;
        }

        /// @brief Provides information about bidirectional prediction support.
        /// @return True if bidirectional prediction supported.
        uint16_t get_bidirectional_prediction_support() const {
            return enc_.BiDirectionalPrediction;
        }

        /// @brief Provides list of supported profiles.
        /// @return list of profiles.
        std::vector<encoder_profile> get_profiles() const {
            std::vector<encoder_profile> profiles;

            std::for_each(enc_.Profiles,
                          enc_.Profiles + enc_.NumProfiles,
                          [&](mfxEncoderDescription::encoder::encprofile profile) {
                              profiles.push_back(encoder_profile(profile));
                          });

            return profiles;
        }

    protected:
        /// Raw data
        mfxEncoderDescription::encoder enc_;
    };

    /// @brief Provides information about supported VPP filters
    class vpp_filter {
    public:
        /// @brief Default ctor
        /// @param flt filter
        explicit vpp_filter(mfxVPPDescription::filter flt) : flt_(flt) {}

        /// @brief Provides VPP filter ID.
        /// @return Filter ID.
        uint32_t get_filter_id() const {
            return flt_.FilterFourCC;
        }

        /// @brief Provides maximum introduced delay by this filter.
        /// @return Delay in frames.
        uint16_t get_max_delay_in_frames() const {
            return flt_.MaxDelayInFrames;
        }

        /// @brief Provides list of supported memory type.
        /// @return list of memory types.
        std::vector<vpp_memory> get_memory_types() const {
            std::vector<vpp_memory> memories;

            std::for_each(flt_.MemDesc,
                          flt_.MemDesc + flt_.NumMemTypes,
                          [&](mfxVPPDescription::filter::memdesc memdesc) {
                              memories.push_back(vpp_memory(memdesc));
                          });

            return memories;
        }

    protected:
        /// Raw data
        mfxVPPDescription::filter flt_;
    };

    /// @brief Provides type of the implementation: SW or HW.
    /// @return Type of the implementation.
    mfxImplType get_impl_type() const {
        return caps_->Impl;
    }

    /// @brief Provides acceleration mode of the implementation.
    /// @return Type of the implementation.
    /// @todo Port to api 2.1 is reqired
    mfxAccelerationMode get_acceleration_mode() const {
        return caps_->AccelerationMode;
    }

    /// @brief Provides supported API version of the implementation.
    /// @return API version.
    mfxVersion get_api_version() const {
        return caps_->ApiVersion;
    }

    /// @brief Provides name of the implementation.
    /// @return Null terminated string with the name of the implementation.
    std::string get_impl_name() const {
        return std::string(caps_->ImplName);
    }

    /// @brief Provides license of the implementation.
    /// @return Null terminated string with the license of the implementation.
    std::string get_license_name() const {
        return std::string(caps_->License);
    }

    /// @brief Provides keywords of the implementation.
    /// @return Null terminated string with the keywords of the implementation.
    std::string get_keywords_name() const {
        return std::string(caps_->Keywords);
    }

    /// @brief Provides vendor ID.
    /// @return Vendor ID.
    uint32_t get_vendor_id() const {
        return caps_->VendorID;
    }

    /// @brief Provides vendor's implementation ID.
    /// @return vendor's implementation ID.
    uint32_t get_vendor_impl_id() const {
        return caps_->VendorImplID;
    }

    /// @brief Provides target device for the implementation.
    /// @return Null terminated string with device ID (or name).
    std::string get_target_device_id() const {
        return std::string(caps_->Dev.DeviceID);
    }

    /// @brief Provides list of supported target subdevice IDs.
    /// @return list of supported target subdevice IDs.
    std::vector<std::string> get_target_subdevice_ids() const {
        std::vector<std::string> subdevices;

        std::for_each(caps_->Dev.SubDevices,
                      caps_->Dev.SubDevices + caps_->Dev.NumSubDevices,
                      [&](mfxDeviceDescription::subdevices sb) {
                          subdevices.push_back(std::string(sb.SubDeviceID));
                      });

        return subdevices;
    }

    /// @brief Provides list of supported acceleration modes.
    /// @return list of supported modes.
    std::vector<std::string> get_accel_modes() const {
        std::vector<std::string> accelmodes;

        if (caps_->AccelerationModeDescription.NumAccelerationModes > 0) {
            std::for_each(caps_->AccelerationModeDescription.Mode,
                          caps_->AccelerationModeDescription.Mode + 
                                caps_->AccelerationModeDescription.NumAccelerationModes,
                          [&](mfxAccelerationMode mode) {
                              accelmodes.push_back(detail::AccelerationMode2String(mode));
                          });
        }

        return accelmodes;
    }

    /// @brief Provides list of supported decoders.
    /// @return list of supported decoders.
    std::vector<decoder> get_decoders() const {
        std::vector<decoder> decoders;

        std::for_each(caps_->Dec.Codecs,
                      caps_->Dec.Codecs + caps_->Dec.NumCodecs,
                      [&](mfxDecoderDescription::decoder dec) {
                          decoders.push_back(decoder(dec));
                      });

        return decoders;
    }

    /// @brief Provides list of supported encoders.
    /// @return list of supported encoders.
    std::vector<encoder> get_encoders() const {
        std::vector<encoder> encoders;

        std::for_each(caps_->Enc.Codecs,
                      caps_->Enc.Codecs + caps_->Enc.NumCodecs,
                      [&](mfxEncoderDescription::encoder enc) {
                          encoders.push_back(encoder(enc));
                      });

        return encoders;
    }

    /// @brief Provides list of supported VPP filters.
    /// @return list of supported VPP filters.
    std::vector<vpp_filter> get_vpp_filters() const {
        std::vector<vpp_filter> filters;

        std::for_each(caps_->VPP.Filters,
                      caps_->VPP.Filters + caps_->VPP.NumFilters,
                      [&](mfxVPPDescription::filter flt) {
                          filters.push_back(vpp_filter(flt));
                      });

        return filters;
    }

    /// @brief Provides list of supported pool policies.
    /// @return list of supported pool policies.
    pool_policies get_pool_policies() const {
        return pool_policies(&caps_->PoolPolicies);
    }

protected:
    /// Raw data
    mfxImplDescription *caps_;

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] f Referebce to the implementation_capabilities instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream &operator<<(std::ostream &out, const implementation_capabilities &f);
};

/// @brief Store and parse names of the implemented functions
class implemented_functions : public base_implementation_capabilities {
public:
    /// @brief Default ctor
    /// @param caps pointer to raw data.
    explicit implemented_functions(mfxImplementedFunctions *caps)
            : base_implementation_capabilities(MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS),
              caps_(caps) {}

public:
    /// @brief Provides list of implemented functions.
    /// @return list of implemented functions.
    std::vector<std::string> get_functions_name() const {
        std::vector<std::string> functions;

        std::for_each(caps_->FunctionsName,
                      caps_->FunctionsName + caps_->NumFunctions,
                      [&](mfxChar* func) {
                          functions.push_back(func);
                      });

        return functions;
    }
protected:
    /// Raw data
    mfxImplementedFunctions *caps_;

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] f Referebce to the implemented_functions instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream &operator<<(std::ostream &out, const implemented_functions &f);
};

/// @brief Store and parse path to the implementation
class implementation_path : public base_implementation_capabilities {
public:
    /// @brief Default ctor
    /// @param caps pointer to raw data.
    explicit implementation_path(mfxChar *path)
            : base_implementation_capabilities(MFX_IMPLCAPS_IMPLPATH),
              path_(path) {}

public:
    /// @brief Provides path to the implementation's shared library.
    /// @return path to the implementation's shared library.
    std::string get_path() const {
        return std::string(path_);
    }
protected:
    /// Raw data
    mfxChar *path_;

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] f Referebce to the implemented_functions instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream &operator<<(std::ostream &out, const implementation_path &f);
};

/// @brief Factory class to create implementation capabilities report class based on the format ID.
class implementation_capabilities_factory {
public:
    /// @brief Default ctor
    implementation_capabilities_factory() {
        map_[MFX_IMPLCAPS_IMPLDESCSTRUCTURE] = [](void *handle) {
            return std::make_shared<implementation_capabilities>(
                reinterpret_cast<mfxImplDescription *>(handle));
        };
        map_[MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS] = [](void *handle) {
            return std::make_shared<implemented_functions>(
                reinterpret_cast<mfxImplementedFunctions *>(handle));
        };
        map_[MFX_IMPLCAPS_IMPLPATH] = [](void *handle) {
            return std::make_shared<implementation_path>(
                reinterpret_cast<mfxChar *>(handle));
        };
    }

    /// @brief Creates instance of implementation capabilities report class based on the format ID
    /// @param[in] id Requested format ID
    /// @param[in] handle Handle to read-only raw data to interpret by the implementation capabilities report class
    /// @return instance of implementation capabilities report class based on the format ID
    std::shared_ptr<base_implementation_capabilities> create(uint32_t id, void *handle) {
        if (auto it = map_.find(id); it != map_.end())
            return it->second(handle);
        throw base_exception(MFX_ERR_UNSUPPORTED);
    }

protected:
    /// @brief Map of format ID and interpreter instance class creator
    std::map<uint32_t,
             std::function<std::shared_ptr<base_implementation_capabilities>(void *handle)>>
        map_;
};

inline std::ostream &operator<<(std::ostream &out,
                                const implementation_capabilities::decoder_memory &m) {
    out << detail::space(detail::INTENT * 5, out, "MemHandleType = ")
        << detail::ResourceType2String(m.get_mem_type()) << std::endl;
    out << detail::space(detail::INTENT * 5, out, "Frame size range = [")
        << std::get<0>(m.get_min_size()) << "," << std::get<1>(m.get_min_size()) << "]:["
        << std::get<0>(m.get_max_size()) << "," << std::get<1>(m.get_max_size()) << "]"
        << std::endl;
    std::vector<uint32_t> fmts = m.get_out_mem_types();
    out << detail::space(detail::INTENT * 5, out, "# of color formats = ") << fmts.size()
        << std::endl;
    detail::space(detail::INTENT * 6, out, "color format = ");
    std::for_each(fmts.begin(), fmts.end(), [&](uint32_t fmt) {
        out << detail::FourCC2String(fmt) << ", ";
    });
    out << "\b\b  " << std::endl;

    return out;
}

inline std::ostream &operator<<(std::ostream &out,
                                const implementation_capabilities::encoder_memory &m) {
    out << detail::space(detail::INTENT * 5, out, "MemHandleType = ")
        << detail::ResourceType2String(m.get_mem_type()) << std::endl;
    out << detail::space(detail::INTENT * 5, out, "Frame size range = [")
        << std::get<0>(m.get_min_size()) << "," << std::get<1>(m.get_min_size()) << "]:["
        << std::get<0>(m.get_max_size()) << "," << std::get<1>(m.get_max_size()) << "]"
        << std::endl;
    std::vector<uint32_t> fmts = m.get_out_mem_types();
    out << detail::space(detail::INTENT * 5, out, "# of color formats = ") << fmts.size()
        << std::endl;
    
    out << detail::space(detail::INTENT * 6, out, "color formats = ");
    std::for_each(fmts.begin(), fmts.end(), [&](uint32_t fmt) {
        out << detail::FourCC2String(fmt) << ", ";
    });
    out << "\b\b  " << std::endl;

    return out;
}

inline std::ostream &operator<<(std::ostream &out,
                                const implementation_capabilities::decoder_profile &p) {
    out << detail::space(detail::INTENT * 4, out, "Profile = ") << p.get_profile() << std::endl;

    std::vector<implementation_capabilities::decoder_memory> dm = p.get_decoder_mem_types();
    out << detail::space(detail::INTENT * 4, out, "# memory types = ") << dm.size() << std::endl;
    std::for_each(dm.begin(), dm.end(), [&](implementation_capabilities::decoder_memory dec_m) {
        out << dec_m;
    });

    return out;
}

inline std::ostream &operator<<(std::ostream &out,
                                const implementation_capabilities::encoder_profile &p) {
    out << detail::space(detail::INTENT * 4, out, "Profile = ") << p.get_profile() << std::endl;

    std::vector<implementation_capabilities::encoder_memory> em = p.get_encoder_mem_types();
    out << detail::space(detail::INTENT * 4, out, "# memory types = ") << em.size() << std::endl;
    std::for_each(em.begin(), em.end(), [&](implementation_capabilities::encoder_memory enc_m) {
        out << enc_m;
    });

    return out;
}

inline std::ostream &operator<<(std::ostream &out, const implementation_capabilities::decoder &d) {
    out << detail::space(detail::INTENT * 3, out, "CodecID = ")
        << detail::FourCC2String(d.get_codec_id()) << std::endl;
    out << detail::space(detail::INTENT * 3, out, "MaxcodecLevel = ") << d.get_max_codec_level()
        << std::endl;

    std::vector<implementation_capabilities::decoder_profile> dp = d.get_profiles();
    out << detail::space(detail::INTENT * 3, out, "# of profiles = ") << dp.size() << std::endl;
    std::for_each(dp.begin(), dp.end(), [&](implementation_capabilities::decoder_profile dec_p) {
        out << dec_p;
    });
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const implementation_capabilities::encoder &e) {
    out << detail::space(detail::INTENT * 3, out, "CodecID = ")
        << detail::FourCC2String(e.get_codec_id()) << std::endl;
    out << detail::space(detail::INTENT * 3, out, "MaxcodecLevel = ") << e.get_max_codec_level()
        << std::endl;

    std::vector<implementation_capabilities::encoder_profile> ep = e.get_profiles();
    out << detail::space(detail::INTENT * 3, out, "# of profiles = ") << ep.size() << std::endl;
    std::for_each(ep.begin(), ep.end(), [&](implementation_capabilities::encoder_profile enc_p) {
        out << enc_p;
    });
    return out;
}

inline std::ostream &operator<<(std::ostream &out,
                                const implementation_capabilities::vpp_memory_format &mf) {
    out << detail::space(detail::INTENT * 5, out, "Input Format = ")
        << detail::FourCC2String(mf.get_input_format()) << std::endl;

    std::vector<uint32_t> fmts = mf.get_out_format();
    out << detail::space(detail::INTENT * 5, out, "# of output formats = ") << fmts.size()
        << std::endl;

    out << detail::space(detail::INTENT * 6, out, "color formats = ");
    std::for_each(fmts.begin(), fmts.end(), [&](uint32_t fmt) {
        out << detail::FourCC2String(fmt) << ", ";
    });
    out << "\b\b  " << std::endl;
    return out;
}

inline std::ostream &operator<<(std::ostream &out,
                                const implementation_capabilities::vpp_memory &m) {
    out << detail::space(detail::INTENT * 4, out, "MemHandleType = ")
        << detail::ResourceType2String(m.get_mem_type()) << std::endl;
    out << detail::space(detail::INTENT * 4, out, "Frame size range = [")
        << std::get<0>(m.get_min_size()) << "," << std::get<1>(m.get_min_size()) << "]:["
        << std::get<0>(m.get_max_size()) << "," << std::get<1>(m.get_max_size()) << "]"
        << std::endl;
    std::vector<implementation_capabilities::vpp_memory_format> fmts = m.get_memory_formats();
    out << detail::space(detail::INTENT * 4, out, "# of input formats = ") << fmts.size()
        << std::endl;
    std::for_each(fmts.begin(),
                  fmts.end(),
                  [&](implementation_capabilities::vpp_memory_format vpp_mf) {
                      out << vpp_mf;
                  });

    return out;
}

inline std::ostream &operator<<(std::ostream &out,
                                const implementation_capabilities::vpp_filter &f) {
    out << detail::space(detail::INTENT * 3, out, "FilterID = ")
        << detail::FourCC2String(f.get_filter_id()) << std::endl;
    out << detail::space(detail::INTENT * 3, out, "Max delay In Frames= ")
        << f.get_max_delay_in_frames() << std::endl;

    std::vector<implementation_capabilities::vpp_memory> vm = f.get_memory_types();
    out << detail::space(detail::INTENT * 3, out, "# of memory types = ") << vm.size() << std::endl;
    std::for_each(vm.begin(), vm.end(), [&](implementation_capabilities::vpp_memory vpp_m) {
        out << vpp_m;
    });
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const implementation_capabilities &f) {
    std::cout << "Implementation:" << std::endl;

    out << detail::space(detail::INTENT, out, "Impl    = ")
        << detail::ImplType2String(f.caps_->Impl) << std::endl;
    out << detail::space(detail::INTENT, out, "AccelerationMode = ")
        << detail::AccelerationMode2String(f.caps_->AccelerationMode) << std::endl;

    out << detail::space(detail::INTENT, out, "API Version      = ") << f.caps_->ApiVersion.Major
        << "." << f.caps_->ApiVersion.Minor << std::endl;
    out << detail::space(detail::INTENT, out, "ImplName         = ") << f.caps_->ImplName
        << std::endl;
    out << detail::space(detail::INTENT, out, "License          = ") << f.caps_->License
        << std::endl;
    out << detail::space(detail::INTENT, out, "Keywords         = ") << f.caps_->Keywords
        << std::endl;
    out << detail::space(detail::INTENT, out, "VendorID         = ") << std::hex << std::showbase
        << f.caps_->VendorID << std::endl;
    out << detail::space(detail::INTENT, out, "VendorImplID     = ") << f.caps_->VendorImplID
        << std::dec << std::endl;

    if (f.caps_->AccelerationModeDescription.NumAccelerationModes > 0) {
        std::vector<std::string> accelmodes = f.get_accel_modes();
        out << detail::space(detail::INTENT, out, "AccelerationModes:") << std::endl;
        std::for_each(accelmodes.begin(), accelmodes.end(), [&](std::string mode) {
            out << mode << std::endl;
        });
    }

    out << detail::space(detail::INTENT, out, "Target Device:") << std::endl;
    out << detail::space(detail::INTENT * 2, out, "DeviceID        = ") << f.caps_->Dev.DeviceID
        << std::endl;
    out << detail::space(detail::INTENT * 2, out, "# of subdevices = ")
        << f.caps_->Dev.NumSubDevices << std::endl;
    for (int i = 0; i < f.caps_->Dev.NumSubDevices; i++) {
        out << detail::space(detail::INTENT * 2, out, "SubdeviceID[") << i
            << "] = " << f.caps_->Dev.SubDevices[i].SubDeviceID << std::endl;
    }

    std::vector<implementation_capabilities::decoder> decoders = f.get_decoders();
    out << detail::space(detail::INTENT, out, "Decoders:") << std::endl;
    out << detail::space(detail::INTENT * 2, out, "# of decoders = ") << decoders.size()
        << std::endl;

    std::for_each(decoders.begin(), decoders.end(), [&](implementation_capabilities::decoder dec) {
        out << dec;
    });

    std::vector<implementation_capabilities::encoder> encoders = f.get_encoders();
    out << detail::space(detail::INTENT, out, "Encoders:") << std::endl;
    out << detail::space(detail::INTENT * 2, out, "# of encoders = ") << encoders.size()
        << std::endl;

    std::for_each(encoders.begin(), encoders.end(), [&](implementation_capabilities::encoder enc) {
        out << enc;
    });

    std::vector<implementation_capabilities::vpp_filter> filters = f.get_vpp_filters();
    out << detail::space(detail::INTENT, out, "VPP Filters:") << std::endl;
    out << detail::space(detail::INTENT * 2, out, "# of filters = ") << filters.size() << std::endl;

    std::for_each(filters.begin(), filters.end(), [&](implementation_capabilities::vpp_filter flt) {
        out << flt;
    });

    auto policies = f.get_pool_policies().policies();
    out << detail::space(detail::INTENT, out, "Pool Policies:") << std::endl;
    out << detail::space(detail::INTENT * 2, out, "# of policies = ") << policies.size() << std::endl;

    std::for_each(policies.begin(), policies.end(), [&](auto policy) {
        out << policy;
    });

    return out;
}

inline std::ostream &operator<<(std::ostream &out, const implemented_functions &f) {
    std::cout << "Functions: " << std::endl;
    auto names = f.get_functions_name();

    std::for_each(names.begin(), names.end(), [&](auto name) {
        out << name << ", ";
    });

    out << "\b\b  " << std::endl;
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const implementation_path &f) {
    std::cout << "Implementation path: " << f.get_path() << std::endl;
    return out;
}

} // namespace vpl
} // namespace oneapi
