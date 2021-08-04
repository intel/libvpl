/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <exception>
#include <map>
#include <vector>

#include "vpl/preview/extension_buffer.hpp"

namespace oneapi {
namespace vpl {

#define IGNORE_LIST_LEN 2
static constexpr uint32_t ignore_ID_list[IGNORE_LIST_LEN] = { 0, MFX_EXTBUFF_VPP_AUXDATA };

/// @brief Base class to replerent list of extension buffers.
/// Buffer pointers are stored as a map, so a single occurance of the same
/// extension buffer is possible.
class buffer_list {
public:
    /// @brief default ctor
    buffer_list() : extBuffers_() , mfxBuffers_(0) {}

    buffer_list(const buffer_list& other) : extBuffers_(other.extBuffers_), mfxBuffers_(0){}
    buffer_list& operator=(const buffer_list& other){
        if(mfxBuffers_ != other.mfxBuffers_){
            if(mfxBuffers_){
                delete[] mfxBuffers_;
                mfxBuffers_ = nullptr;
            }
            extBuffers_ = other.extBuffers_;
        }
        return *this;
    }	

    /// @brief dtor
    virtual ~buffer_list() {
        delete[] mfxBuffers_;
    }

    /// @brief returns reference to the map with extension buffers. Key is buffer ID in the form of FourCC codes
    /// value is the pointer to the extension buffer.
    /// @return reference to the map with the pointers to the extension buffers
    std::map<uint32_t, extension_buffer_base*>& get_buffers() {
        return extBuffers_;
    }

    /// @brief Reurns number of extension buffers in the map.
    /// @return Number of extension buffers in the map.
    std::size_t get_size() const {
        return extBuffers_.size();
    }
    /// @brief adds extension buffer pointer to the map
    /// @param[in] o pointer to the extension buffer
    void add_buffer(extension_buffer_base* o) {
        // extBuffers_.insert(std::pair<uint32_t,eb>(o.get_ID(),o)); // this is language hack. ref can't be in the std::map
        extBuffers_[o->get_ID()] = o;
    }

    /// @brief verifies that map contains given key (extension buffer)
    /// @tparam ID extension buffer ID in the form of FourCC code.
    /// @return true if buffer exists in the map.
    template <uint32_t ID>
    bool has_buffer() {
        if (extBuffers_.find(ID) != extBuffers_.end())
            return true;
        return false;
    }

    /// @brief verifies that map contains given key (extension buffer)
    /// @tparam ID extension buffer ID in the form of FourCC code.
    /// @return true if buffer exists in the map.
    bool has_buffer(uint32_t ID) {
        if (extBuffers_.find(ID) != extBuffers_.end())
            return true;
        return false;
    }

    /// @brief returns extension buffer of given type and ID.
    /// @tparam T extension buffer class.
    /// @tparam ID extension buffer ID in the form of FourCC code.
    /// @return pointer to the extension buffer or nullptr is that buffer doesn't in the map
    template <typename T, uint32_t ID>
    T* get_buffer() {
        auto buff = extBuffers_.find(ID);
        if (buff != extBuffers_.end()) {
            return *buff;
        }
        return nullptr;
    }

    /// @brief returns extension buffer of given type and ID.
    /// @tparam T extension buffer class.
    /// @tparam ID extension buffer ID in the form of FourCC code.
    /// @return pointer to the extension buffer or nullptr if that buffer isn't in the map
    template <typename T>
    T* get_buffer(uint32_t ID) {
        auto buff = extBuffers_.find(ID);
        if (buff != extBuffers_.end()) {
            return buff->second;
        }
        return nullptr;
    }

    /// @brief returns pair of array of pointers to the extension buffer and number of buffers
    /// @return pair of array of pointers to the extension buffer and number of buffers
    /// @todo output unique_ptr instead of pointer to the member variable
    auto get_raw_ext_buffers() {
        uint32_t ignoreNum = 0;

        std::for_each(ignore_ID_list, ignore_ID_list + IGNORE_LIST_LEN, [&](uint32_t id) {
            if (has_buffer(id))
                ignoreNum++;
        });

        delete[] mfxBuffers_;
        mfxBuffers_ = new mfxExtBuffer*[extBuffers_.size() - ignoreNum];
        int i       = 0;

        for (const auto& buf : extBuffers_) {
            auto& id           = buf.first;
            auto& value        = buf.second;
            auto exists_ignore = std::any_of(ignore_ID_list,
                                             ignore_ID_list + IGNORE_LIST_LEN,
                                             [&id](uint32_t ignore_id) {
                                                 return id == ignore_id;
                                             });
            if (exists_ignore)
                continue;
            mfxBuffers_[i] = value->get_ptr();
            i++;
        }
        return std::pair(mfxBuffers_, extBuffers_.size() - ignoreNum);
    }

protected:
    /// Map of extension buffers with key equal to the buffer ID
    /// @tparam extension_buffer_base* pointer to the extension buffer class.
    /// @tparam uint32_t extension buffer ID in the form of FourCC code.
    std::map<uint32_t, extension_buffer_base*> extBuffers_;
    /// @todo remove
    mfxExtBuffer** mfxBuffers_;
};

/// @brief This class hold list of extension buffers used during decoder's initialization stage
/// Those buffers are attached to the mfxVideoParam structure before Init call.
class decoder_init_reset_list : public buffer_list {
    template <typename... Tail>
    struct AllBuffers : std::true_type {};
    template <typename T, typename... Tail>
    struct AllBuffers<T, Tail...>
            : std::conditional<std::is_same<ExtDecVideoProcessing*, T>::value ||
                                   std::is_same<ExtHEVCParam*, T>::value ||
                                   std::is_same<ExtDecodeErrorReport*, T>::value ||
                                   std::is_same<ExtDecodedFrameInfo*, T>::value ||
                                   std::is_same<ExtVP9Param*, T>::value ||
                                   std::is_same<ExtDeviceAffinityMask*, T>::value,
                               AllBuffers<Tail...>,
                               std::false_type>::type {};

public:
    decoder_init_reset_list() {
        ctor_helper();
    }

    /// @brief Variadic length ctor.
    /// This ctor accepts list of pointers to the extension buffers of pre-defined type:
    /// ExtDecVideoProcessing,
    /// ExtHEVCParam,
    /// ExtDecodeErrorReport,
    /// ExtDecodedFrameInfo,
    /// ExtVP9Param,
    /// ExtDeviceAffinityMask
    /// @param[in] Opts List of property objects
    template <typename... OptsT,
              typename = typename std::enable_if<AllBuffers<OptsT...>::value>::type>
    explicit decoder_init_reset_list(OptsT... Opts) {
        ctor_helper(Opts...);
    }

    /// @brief ctor
    /// @param[in] Opts Vector of Extension Buffers
    explicit decoder_init_reset_list(std::vector<extension_buffer_base*> Opts) {
        for (auto opt : Opts) {
            add_buffer(opt);
        }
    }

    /// @brief Add extension buffer to list
    /// @param[in] o Extension Buffer
    void add_buffer(extension_buffer_base* o) {
        switch (o->get_ID()) {
            case MFX_EXTBUFF_DEC_VIDEO_PROCESSING:
            case MFX_EXTBUFF_HEVC_PARAM:
            case MFX_EXTBUFF_DECODE_ERROR_REPORT:
            case MFX_EXTBUFF_DECODED_FRAME_INFO:
            case MFX_EXTBUFF_VP9_PARAM:
            case MFX_EXTBUFF_DEVICE_AFFINITY_MASK:
                buffer_list::add_buffer(o);
                return;
        }
        throw base_exception("Invalid buffer type", MFX_ERR_UNKNOWN);
    }

protected:
    /// @brief Empty ctor helper
    void ctor_helper() {}

    /// @brief Variadic length ctor helper. It adds extension buffers one by one to the map of buffers
    /// @param[in] Opt First property
    /// @param[in] Opts Remaining properties
    /// @return Nothing
    template <typename... OptsT, class OptT>
    typename std::enable_if<std::is_same<ExtDecVideoProcessing*, OptT>::value ||
                            std::is_same<ExtHEVCParam*, OptT>::value ||
                            std::is_same<ExtDecodeErrorReport*, OptT>::value ||
                            std::is_same<ExtDecodedFrameInfo*, OptT>::value ||
                            std::is_same<ExtVP9Param*, OptT>::value ||
                            std::is_same<ExtDeviceAffinityMask*, OptT>::value>::type
    ctor_helper(OptT Opt, OptsT... Opts) {
        buffer_list::add_buffer(Opt);
        ctor_helper(Opts...);
    }
};

/// @brief This class hold list of extension buffers used during decoder's initialization stage
/// Those buffers are attached to the mfxVideoParam structure before DecodeHeader call.
class decoder_init_header_list : public buffer_list {
    template <typename... Tail>
    struct AllBuffers : std::true_type {};
    template <typename T, typename... Tail>
    struct AllBuffers<T, Tail...> : std::conditional<std::is_same<ExtCodingOptionSPSPPS*, T>::value,
                                                     AllBuffers<Tail...>,
                                                     std::false_type>::type {};

public:
    decoder_init_header_list() {
        ctor_helper();
    }

    /// @brief Variadic length ctor.
    /// This ctor accepts list of pointers to the extension buffers of pre-defined type:
    /// ExtCodingOptionSPSPPS
    /// @param[in] Opts List of property objects
    template <typename... OptsT,
              typename = typename std::enable_if<AllBuffers<OptsT...>::value>::type>
    explicit decoder_init_header_list(OptsT... Opts) {
        ctor_helper(Opts...);
    }

    /// @brief ctor
    /// @param[in] Opts Vector of Extension Buffers
    explicit decoder_init_header_list(std::vector<extension_buffer_base*> Opts) {
        for (auto opt : Opts) {
            add_buffer(opt);
        }
    }

    /// @brief Add extension buffer to list
    /// @param[in] o Extension Buffer
    void add_buffer(extension_buffer_base* o) {
        switch (o->get_ID()) {
            case MFX_EXTBUFF_CODING_OPTION_SPSPPS:
                buffer_list::add_buffer(o);
                return;
        }
        throw base_exception("Invalid buffer type", MFX_ERR_UNKNOWN);
    }

protected:
    /// @brief Empty ctor helper
    void ctor_helper() {}

    /// @brief Variadic length ctor helper. It adds extension buffers one by one to the map of buffers
    /// @param[in] Opt First property
    /// @param[in] Opts Remaining properties
    /// @return Nothing
    template <typename... OptsT, class OptT>
    typename std::enable_if<std::is_same<ExtCodingOptionSPSPPS*, OptT>::value>::type ctor_helper(
        OptT Opt,
        OptsT... Opts) {
        add_buffer(Opt);
        ctor_helper(Opts...);
    }
};

/// @brief This class hold list of extension buffers used during decoder's processing stage
/// Those buffers are attached to the mfxVideoParam structure before DecodeFrameAsync call.
class decoder_process_list : public buffer_list {
    template <typename... Tail>
    struct AllBuffers : std::true_type {};
    template <typename T, typename... Tail>
    struct AllBuffers<T, Tail...> : std::conditional<std::is_same<ExtDecodeErrorReport*, T>::value,
                                                     AllBuffers<Tail...>,
                                                     std::false_type>::type {};

public:
    decoder_process_list() {
        ctor_helper();
    }

    /// @brief Variadic length ctor.
    /// This ctor accepts list of pointers to the extension buffers of pre-defined type:
    /// ExtDecodeErrorReport
    /// @param[in] Opts List of property objects
    template <typename... OptsT,
              typename = typename std::enable_if<AllBuffers<OptsT...>::value>::type>
    explicit decoder_process_list(OptsT... Opts) {
        ctor_helper(Opts...);
    }

    /// @brief ctor
    /// @param[in] Opts Vector of Extension Buffers
    explicit decoder_process_list(std::vector<extension_buffer_base*> Opts) {
        for (auto opt : Opts) {
            add_buffer(opt);
        }
    }

    /// @brief Add extension buffer to list
    /// @param[in] o Extension Buffer
    void add_buffer(extension_buffer_base* o) {
        switch (o->get_ID()) {
            case MFX_EXTBUFF_DECODE_ERROR_REPORT:
                buffer_list::add_buffer(o);
                return;
        }
        throw base_exception("Invalid buffer type", MFX_ERR_UNKNOWN);
    }

protected:
    /// @brief Empty ctor helper
    void ctor_helper() {}

    /// @brief Variadic length ctor helper. It adds extension buffers one by one to the map of buffers
    /// @param[in] Opt First property
    /// @param[in] Opts Remaining properties
    /// @return Nothing
    template <typename... OptsT, class OptT>
    typename std::enable_if<std::is_same<ExtDecodeErrorReport*, OptT>::value>::type ctor_helper(
        OptT Opt,
        OptsT... Opts) {
        add_buffer(Opt);
        ctor_helper(Opts...);
    }
};

// Encoders ext buffers lists

/// @brief This class hold list of extension buffers used during encoder's initialization stage
/// Those buffers are attached to the mfxVideoParam structure before Init call.
class encoder_init_list : public buffer_list {
    template <typename... Tail>
    struct AllBuffers : std::true_type {};
    template <typename T, typename... Tail>
    struct AllBuffers<T, Tail...>
            : std::conditional<std::is_same<ExtCodingOption*, T>::value ||
                                   std::is_same<ExtCodingOption2*, T>::value ||
                                   std::is_same<ExtCodingOption3*, T>::value ||
                                   std::is_same<ExtCodingOptionSPSPPS*, T>::value ||
                                   std::is_same<ExtCodingOptionVPS*, T>::value ||
                                   std::is_same<ExtMasteringDisplayColourVolume*, T>::value ||
                                   std::is_same<ExtContentLightLevelInfo*, T>::value ||
                                   std::is_same<ExtPictureTimingSEI*, T>::value ||
                                   std::is_same<ExtAvcTemporalLayers*, T>::value ||
                                   std::is_same<ExtEncoderROI*, T>::value ||
                                   std::is_same<ExtHEVCTiles*, T>::value ||
                                   std::is_same<ExtHEVCParam*, T>::value ||
                                   std::is_same<ExtHEVCRegion*, T>::value ||
                                   std::is_same<ExtDirtyRect*, T>::value ||
                                   std::is_same<ExtMVOverPicBoundaries*, T>::value ||
                                   std::is_same<ExtVP9Segmentation*, T>::value ||
                                   std::is_same<ExtVP9TemporalLayers*, T>::value ||
                                   std::is_same<ExtVP9Param*, T>::value ||
                                   std::is_same<ExtPartialBitstreamParam*, T>::value ||
                                   std::is_same<ExtDeviceAffinityMask*, T>::value,
                               AllBuffers<Tail...>,
                               std::false_type>::type {};

public:
    encoder_init_list() {
        ctor_helper();
    }

    /// @brief Variadic length ctor.
    /// This ctor accepts list of pointers to the extension buffers of pre-defined type:
    /// ExtCodingOption,
    /// ExtCodingOption2,
    /// ExtCodingOption3,
    /// ExtCodingOptionSPSPPS,
    /// ExtCodingOptionVPS,
    /// ExtMasteringDisplayColourVolume,
    /// ExtContentLightLevelInfo,
    /// ExtPictureTimingSEI,
    /// ExtAvcTemporalLayers,
    /// ExtEncoderROI,
    /// ExtHEVCTiles,
    /// ExtHEVCParam,
    /// ExtHEVCRegion,
    /// ExtDirtyRect,
    /// ExtMVOverPicBoundaries,
    /// ExtVP9Segmentation,
    /// ExtVP9TemporalLayers,
    /// ExtVP9Param,
    /// ExtPartialBitstreamParam,
    /// ExtDeviceAffinityMask
    /// @param[in] Opts List of property objects
    template <typename... OptsT,
              typename = typename std::enable_if<AllBuffers<OptsT...>::value>::type>
    explicit encoder_init_list(OptsT... Opts) {
        ctor_helper(Opts...);
    }

    /// @brief ctor
    /// @param[in] Opts Vector of Extension Buffers
    explicit encoder_init_list(std::vector<extension_buffer_base*> Opts) {
        for (auto opt : Opts) {
            add_buffer(opt);
        }
    }

    /// @brief Add extension buffer to list
    /// @param[in] o Extension Buffer
    void add_buffer(extension_buffer_base* o) {
        switch (o->get_ID()) {
            case MFX_EXTBUFF_CODING_OPTION:
            case MFX_EXTBUFF_CODING_OPTION2:
            case MFX_EXTBUFF_CODING_OPTION3:
            case MFX_EXTBUFF_CODING_OPTION_SPSPPS:
            case MFX_EXTBUFF_CODING_OPTION_VPS:
            case MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME:
            case MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO:
            case MFX_EXTBUFF_PICTURE_TIMING_SEI:
            case MFX_EXTBUFF_AVC_TEMPORAL_LAYERS:
            case MFX_EXTBUFF_ENCODER_ROI:
            case MFX_EXTBUFF_HEVC_TILES:
            case MFX_EXTBUFF_HEVC_PARAM:
            case MFX_EXTBUFF_HEVC_REGION:
            case MFX_EXTBUFF_DIRTY_RECTANGLES:
            case MFX_EXTBUFF_MV_OVER_PIC_BOUNDARIES:
            case MFX_EXTBUFF_VP9_SEGMENTATION:
            case MFX_EXTBUFF_VP9_TEMPORAL_LAYERS:
            case MFX_EXTBUFF_VP9_PARAM:
            case MFX_EXTBUFF_PARTIAL_BITSTREAM_PARAM:
            case MFX_EXTBUFF_DEVICE_AFFINITY_MASK:
                buffer_list::add_buffer(o);
                return;
        }
        throw base_exception("Invalid buffer type", MFX_ERR_UNKNOWN);
    }

protected:
    /// @brief Empty ctor helper
    void ctor_helper() {}

    /// @brief Variadic length ctor helper. It adds extension buffers one by one to the map of buffers
    /// @param[in] Opt First property
    /// @param[in] Opts Remaining properties
    /// @return Nothing
    template <typename... OptsT, class OptT>
    typename std::enable_if<
        std::is_same<ExtCodingOption*, OptT>::value ||
        std::is_same<ExtCodingOption2*, OptT>::value ||
        std::is_same<ExtCodingOption3*, OptT>::value ||
        std::is_same<ExtCodingOptionSPSPPS*, OptT>::value ||
        std::is_same<ExtCodingOptionVPS*, OptT>::value ||
        std::is_same<ExtMasteringDisplayColourVolume*, OptT>::value ||
        std::is_same<ExtContentLightLevelInfo*, OptT>::value ||
        std::is_same<ExtPictureTimingSEI*, OptT>::value ||
        std::is_same<ExtAvcTemporalLayers*, OptT>::value ||
        std::is_same<ExtEncoderROI*, OptT>::value || std::is_same<ExtHEVCTiles*, OptT>::value ||
        std::is_same<ExtHEVCParam*, OptT>::value || std::is_same<ExtHEVCRegion*, OptT>::value ||
        std::is_same<ExtDirtyRect*, OptT>::value ||
        std::is_same<ExtMVOverPicBoundaries*, OptT>::value ||
        std::is_same<ExtVP9Segmentation*, OptT>::value ||
        std::is_same<ExtVP9TemporalLayers*, OptT>::value ||
        std::is_same<ExtVP9Param*, OptT>::value ||
        std::is_same<ExtPartialBitstreamParam*, OptT>::value ||
        std::is_same<ExtDeviceAffinityMask*, OptT>::value>::type
    ctor_helper(OptT Opt, OptsT... Opts) {
        add_buffer(Opt);
        ctor_helper(Opts...);
    }
};

/// @brief This class hold list of extension buffers used during encoder's reset stage
/// Those buffers are attached to the mfxVideoParam structure before Init call.
class encoder_reset_list : public buffer_list {
    template <typename... Tail>
    struct AllBuffers : std::true_type {};
    template <typename T, typename... Tail>
    struct AllBuffers<T, Tail...>
            : std::conditional<std::is_same<ExtCodingOption*, T>::value ||
                                   std::is_same<ExtCodingOption2*, T>::value ||
                                   std::is_same<ExtCodingOption3*, T>::value ||
                                   std::is_same<ExtCodingOptionSPSPPS*, T>::value ||
                                   std::is_same<ExtCodingOptionVPS*, T>::value ||
                                   std::is_same<ExtMasteringDisplayColourVolume*, T>::value ||
                                   std::is_same<ExtContentLightLevelInfo*, T>::value ||
                                   std::is_same<ExtPictureTimingSEI*, T>::value ||
                                   std::is_same<ExtAvcTemporalLayers*, T>::value ||
                                   std::is_same<ExtEncoderROI*, T>::value ||
                                   std::is_same<ExtHEVCTiles*, T>::value ||
                                   std::is_same<ExtHEVCParam*, T>::value ||
                                   std::is_same<ExtHEVCRegion*, T>::value ||
                                   std::is_same<ExtDirtyRect*, T>::value ||
                                   std::is_same<ExtMVOverPicBoundaries*, T>::value ||
                                   std::is_same<ExtVP9Segmentation*, T>::value ||
                                   std::is_same<ExtVP9TemporalLayers*, T>::value ||
                                   std::is_same<ExtVP9Param*, T>::value ||
                                   std::is_same<ExtPartialBitstreamParam*, T>::value ||
                                   std::is_same<ExtEncoderResetOption*, T>::value ||
                                   std::is_same<ExtDeviceAffinityMask*, T>::value,
                               AllBuffers<Tail...>,
                               std::false_type>::type {};

public:
    encoder_reset_list() {
        ctor_helper();
    }

    /// @brief Variadic length ctor.
    /// This ctor accepts list of pointers to the extension buffers of pre-defined type:
    /// ExtCodingOption,
    /// ExtCodingOption2,
    /// ExtCodingOption3,
    /// ExtCodingOptionSPSPPS,
    /// ExtCodingOptionVPS,
    /// ExtMasteringDisplayColourVolume,
    /// ExtContentLightLevelInfo,
    /// ExtPictureTimingSEI,
    /// ExtAvcTemporalLayers,
    /// ExtEncoderROI,
    /// ExtHEVCTiles,
    /// ExtHEVCParam,
    /// ExtHEVCRegion,
    /// ExtDirtyRect,
    /// ExtMVOverPicBoundaries,
    /// ExtVP9Segmentation,
    /// ExtVP9TemporalLayers,
    /// ExtVP9Param,
    /// ExtPartialBitstreamParam,
    /// mfxExtEncoderResetOption,
    /// ExtDeviceAffinityMask
    /// @param[in] Opts List of property objects
    template <typename... OptsT,
              typename = typename std::enable_if<AllBuffers<OptsT...>::value>::type>
    explicit encoder_reset_list(OptsT... Opts) {
        ctor_helper(Opts...);
    }

    /// @brief ctor
    /// @param[in] Opts Vector of Extension Buffers
    explicit encoder_reset_list(std::vector<extension_buffer_base*> Opts) {
        for (auto opt : Opts) {
            add_buffer(opt);
        }
    }

    /// @brief Add extension buffer to list
    /// @param[in] o Extension Buffer
    void add_buffer(extension_buffer_base* o) {
        switch (o->get_ID()) {
            case MFX_EXTBUFF_CODING_OPTION:
            case MFX_EXTBUFF_CODING_OPTION2:
            case MFX_EXTBUFF_CODING_OPTION3:
            case MFX_EXTBUFF_CODING_OPTION_SPSPPS:
            case MFX_EXTBUFF_CODING_OPTION_VPS:
            case MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME:
            case MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO:
            case MFX_EXTBUFF_PICTURE_TIMING_SEI:
            case MFX_EXTBUFF_AVC_TEMPORAL_LAYERS:
            case MFX_EXTBUFF_ENCODER_ROI:
            case MFX_EXTBUFF_HEVC_TILES:
            case MFX_EXTBUFF_HEVC_PARAM:
            case MFX_EXTBUFF_HEVC_REGION:
            case MFX_EXTBUFF_DIRTY_RECTANGLES:
            case MFX_EXTBUFF_MV_OVER_PIC_BOUNDARIES:
            case MFX_EXTBUFF_VP9_SEGMENTATION:
            case MFX_EXTBUFF_VP9_TEMPORAL_LAYERS:
            case MFX_EXTBUFF_VP9_PARAM:
            case MFX_EXTBUFF_PARTIAL_BITSTREAM_PARAM:
            case MFX_EXTBUFF_ENCODER_RESET_OPTION:
            case MFX_EXTBUFF_DEVICE_AFFINITY_MASK:
                buffer_list::add_buffer(o);
                return;
        }
        throw base_exception("Invalid buffer type", MFX_ERR_UNKNOWN);
    }

protected:
    /// @brief Empty ctor helper
    void ctor_helper() {}

    /// @brief Variadic length ctor helper. It adds extension buffers one by one to the map of buffers
    /// @param[in] Opt First property
    /// @param[in] Opts Remaining properties
    /// @return Nothing
    template <typename... OptsT, class OptT>
    typename std::enable_if<
        std::is_same<ExtCodingOption*, OptT>::value ||
        std::is_same<ExtCodingOption2*, OptT>::value ||
        std::is_same<ExtCodingOption3*, OptT>::value ||
        std::is_same<ExtCodingOptionSPSPPS*, OptT>::value ||
        std::is_same<ExtCodingOptionVPS*, OptT>::value ||
        std::is_same<ExtMasteringDisplayColourVolume*, OptT>::value ||
        std::is_same<ExtContentLightLevelInfo*, OptT>::value ||
        std::is_same<ExtPictureTimingSEI*, OptT>::value ||
        std::is_same<ExtAvcTemporalLayers*, OptT>::value ||
        std::is_same<ExtEncoderROI*, OptT>::value || std::is_same<ExtHEVCTiles*, OptT>::value ||
        std::is_same<ExtHEVCParam*, OptT>::value || std::is_same<ExtHEVCRegion*, OptT>::value ||
        std::is_same<ExtDirtyRect*, OptT>::value ||
        std::is_same<ExtMVOverPicBoundaries*, OptT>::value ||
        std::is_same<ExtVP9Segmentation*, OptT>::value ||
        std::is_same<ExtVP9TemporalLayers*, OptT>::value ||
        std::is_same<ExtVP9Param*, OptT>::value ||
        std::is_same<ExtPartialBitstreamParam*, OptT>::value ||
        std::is_same<ExtEncoderResetOption*, OptT>::value ||
        std::is_same<ExtDeviceAffinityMask*, OptT>::value>::type
    ctor_helper(OptT Opt, OptsT... Opts) {
        add_buffer(Opt);
        ctor_helper(Opts...);
    }
};

/// @brief This class hold list of extension buffers used during encoder's processing stage
/// Those buffers are attached to the mfxVideoParam structure before Init call.
class encoder_process_list : public buffer_list {
    template <typename... Tail>
    struct AllBuffers : std::true_type {};
    template <typename T, typename... Tail>
    struct AllBuffers<T, Tail...>
            : std::conditional<std::is_same<ExtCodingOption2*, T>::value ||
                                   std::is_same<ExtCodingOption3*, T>::value ||
                                   std::is_same<ExtMasteringDisplayColourVolume*, T>::value ||
                                   std::is_same<ExtContentLightLevelInfo*, T>::value ||
                                   std::is_same<ExtPictureTimingSEI*, T>::value ||
                                   std::is_same<ExtAVCEncodedFrameInfo*, T>::value ||
                                   std::is_same<ExtEncoderROI*, T>::value ||
                                   std::is_same<ExtMBQP*, T>::value ||
                                   std::is_same<ExtInsertHeaders*, T>::value ||
                                   std::is_same<ExtMBForceIntra*, T>::value ||
                                   std::is_same<ExtMBDisableSkipMap*, T>::value ||
                                   std::is_same<ExtPredWeightTable*, T>::value ||
                                   std::is_same<ExtAVCRoundingOffset*, T>::value ||
                                   std::is_same<ExtDirtyRect*, T>::value ||
                                   std::is_same<ExtMoveRect*, T>::value ||
                                   std::is_same<ExtEncodedSlicesInfo*, T>::value ||
                                   std::is_same<ExtVP9Segmentation*, T>::value ||
                                   std::is_same<ExtEncodedUnitsInfo*, T>::value ||
                                   std::is_same<ExtDeviceAffinityMask*, T>::value,
                               AllBuffers<Tail...>,
                               std::false_type>::type {};

public:
    encoder_process_list() {
        ctor_helper();
    }

    /// @brief Variadic length ctor.
    /// This ctor accepts list of pointers to the extension buffers of pre-defined type:
    /// ExtCodingOption2,
    /// ExtCodingOption3,
    /// ExtMasteringDisplayColourVolume,
    /// ExtContentLightLevelInfo,
    /// ExtPictureTimingSEI,
    /// ExtAVCEncodedFrameInfo,
    /// ExtEncoderROI,
    /// ExtMBQP,
    /// ExtInsertHeaders,
    /// ExtMBForceIntra,
    /// ExtMBDisableSkipMap,
    /// ExtPredWeightTable,
    /// ExtAVCRoundingOffset,
    /// ExtDirtyRect,
    /// ExtMoveRect,
    /// ExtEncodedSlicesInfo,
    /// ExtVP9Segmentation,
    /// ExtEncodedUnitsInfo,
    /// ExtDeviceAffinityMask
    /// @param[in] Opts List of property objects
    template <typename... OptsT,
              typename = typename std::enable_if<AllBuffers<OptsT...>::value>::type>
    explicit encoder_process_list(OptsT... Opts) {
        ctor_helper(Opts...);
    }

    /// @brief ctor
    /// @param[in] Opts Vector of Extension Buffers
    explicit encoder_process_list(std::vector<extension_buffer_base*> Opts) {
        for (auto opt : Opts) {
            add_buffer(opt);
        }
    }

    /// @brief Add extension buffer to list
    /// @param[in] o Extension Buffer
    void add_buffer(extension_buffer_base* o) {
        switch (o->get_ID()) {
            case MFX_EXTBUFF_CODING_OPTION2:
            case MFX_EXTBUFF_CODING_OPTION3:
            case MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME:
            case MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO:
            case MFX_EXTBUFF_PICTURE_TIMING_SEI:
            case MFX_EXTBUFF_ENCODED_FRAME_INFO:
            case MFX_EXTBUFF_AVC_TEMPORAL_LAYERS:
            case MFX_EXTBUFF_ENCODER_ROI:
            case MFX_EXTBUFF_MBQP:
            case MFX_EXTBUFF_INSERT_HEADERS:
            case MFX_EXTBUFF_MB_FORCE_INTRA:
            case MFX_EXTBUFF_MB_DISABLE_SKIP_MAP:
            case MFX_EXTBUFF_PRED_WEIGHT_TABLE:
            case MFX_EXTBUFF_AVC_ROUNDING_OFFSET:
            case MFX_EXTBUFF_DIRTY_RECTANGLES:
            case MFX_EXTBUFF_MOVING_RECTANGLES:
            case MFX_EXTBUFF_ENCODED_SLICES_INFO:
            case MFX_EXTBUFF_VP9_SEGMENTATION:
            case MFX_EXTBUFF_ENCODED_UNITS_INFO:
            case MFX_EXTBUFF_DEVICE_AFFINITY_MASK:
                buffer_list::add_buffer(o);
                return;
        }
        throw base_exception("Invalid buffer type", MFX_ERR_UNKNOWN);
    }

protected:
    /// @brief Empty ctor helper
    void ctor_helper() {}

    /// @brief Variadic length ctor helper. It adds extension buffers one by one to the map of buffers
    /// @param[in] Opt First property
    /// @param[in] Opts Remaining properties
    /// @return Nothing
    template <typename... OptsT, class OptT>
    typename std::enable_if<
        std::is_same<ExtCodingOption2*, OptT>::value ||
        std::is_same<ExtCodingOption3*, OptT>::value ||
        std::is_same<ExtMasteringDisplayColourVolume*, OptT>::value ||
        std::is_same<ExtContentLightLevelInfo*, OptT>::value ||
        std::is_same<ExtPictureTimingSEI*, OptT>::value ||
        std::is_same<ExtAVCEncodedFrameInfo*, OptT>::value ||
        std::is_same<ExtEncoderROI*, OptT>::value || std::is_same<ExtMBQP*, OptT>::value ||
        std::is_same<ExtInsertHeaders*, OptT>::value ||
        std::is_same<ExtMBForceIntra*, OptT>::value ||
        std::is_same<ExtMBDisableSkipMap*, OptT>::value ||
        std::is_same<ExtPredWeightTable*, OptT>::value ||
        std::is_same<ExtAVCRoundingOffset*, OptT>::value ||
        std::is_same<ExtDirtyRect*, OptT>::value || std::is_same<ExtMoveRect*, OptT>::value ||
        std::is_same<ExtEncodedSlicesInfo*, OptT>::value ||
        std::is_same<ExtVP9Segmentation*, OptT>::value ||
        std::is_same<ExtEncodedUnitsInfo*, OptT>::value ||
        std::is_same<ExtDeviceAffinityMask*, OptT>::value>::type
    ctor_helper(OptT Opt, OptsT... Opts) {
        add_buffer(Opt);
        ctor_helper(Opts...);
    }
};

// VPPs ext buffers lists

/// @brief This class hold list of extension buffers used during VPP's initialization and/or reset stage
/// Those buffers are attached to the mfxVideoParam structure before Init call.
class vpp_init_reset_list : public buffer_list {
    template <typename... Tail>
    struct AllBuffers : std::true_type {};
    template <typename T, typename... Tail>
    struct AllBuffers<T, Tail...>
            : std::conditional<std::is_same<ExtVPPDoNotUse*, T>::value ||
                                   std::is_same<ExtVPPDenoise*, T>::value ||
                                   std::is_same<ExtVPPDetail*, T>::value ||
                                   std::is_same<ExtVPPProcAmp*, T>::value ||
                                   std::is_same<ExtVPPDoUse*, T>::value ||
                                   std::is_same<ExtVPPFrameRateConversion*, T>::value ||
                                   std::is_same<ExtVPPImageStab*, T>::value ||
                                   std::is_same<ExtVPPComposite*, T>::value ||
                                   std::is_same<ExtVPPVideoSignalInfo*, T>::value ||
                                   std::is_same<ExtVPPDeinterlacing*, T>::value ||
                                   std::is_same<ExtVPPFieldProcessing*, T>::value ||
                                   std::is_same<ExtVPPRotation*, T>::value ||
                                   std::is_same<ExtVPPScaling*, T>::value ||
                                   std::is_same<ExtVPPMirroring*, T>::value ||
                                   std::is_same<ExtVPPColorFill*, T>::value ||
                                   std::is_same<ExtColorConversion*, T>::value ||
                                   std::is_same<ExtVppMctf*, T>::value ||
                                   std::is_same<ExtDeviceAffinityMask*, T>::value,
                               AllBuffers<Tail...>,
                               std::false_type>::type {};

public:
    vpp_init_reset_list() {
        ctor_helper();
    }

    /// @brief Variadic length ctor.
    /// This ctor accepts list of pointers to the extension buffers of pre-defined type:
    /// ExtVPPDoNotUse,
    /// ExtVPPDenoise,
    /// ExtVPPDetail,
    /// ExtVPPProcAmp,
    /// ExtVPPDoUse,
    /// ExtVPPFrameRateConversion,
    /// ExtVPPImageStab,
    /// ExtVPPComposite,
    /// ExtVPPVideoSignalInfo,
    /// ExtVPPDeinterlacing,
    /// ExtVPPFieldProcessing,
    /// ExtVPPRotation,
    /// ExtVPPScaling,
    /// ExtVPPMirroring,
    /// ExtVPPColorFill,
    /// ExtColorConversion,
    /// ExtVppMctf,
    /// ExtDeviceAffinityMask
    /// @param[in] Opts List of property objects
    template <typename... OptsT,
              typename = typename std::enable_if<AllBuffers<OptsT...>::value>::type>
    explicit vpp_init_reset_list(OptsT... Opts) {
        ctor_helper(Opts...);
    }

    /// @brief ctor
    /// @param[in] Opts Vector of Extension Buffers
    explicit vpp_init_reset_list(std::vector<extension_buffer_base*> Opts) {
        for (auto opt : Opts) {
            add_buffer(opt);
        }
    }

    /// @brief Add extension buffer to list
    /// @param[in] o Extension Buffer
    void add_buffer(extension_buffer_base* o) {
        switch (o->get_ID()) {
            case MFX_EXTBUFF_VPP_DONOTUSE:
            case MFX_EXTBUFF_VPP_DENOISE:
            case MFX_EXTBUFF_VPP_DETAIL:
            case MFX_EXTBUFF_VPP_PROCAMP:
            case MFX_EXTBUFF_VPP_DOUSE:
            case MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION:
            case MFX_EXTBUFF_VPP_IMAGE_STABILIZATION:
            case MFX_EXTBUFF_CODING_OPTION_VPS:
            case MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO:
            case MFX_EXTBUFF_VPP_DEINTERLACING:
            case MFX_EXTBUFF_VPP_FIELD_PROCESSING:
            case MFX_EXTBUFF_VPP_ROTATION:
            case MFX_EXTBUFF_VPP_SCALING:
            case MFX_EXTBUFF_VPP_MIRRORING:
            case MFX_EXTBUFF_VPP_COLORFILL:
            case MFX_EXTBUFF_VPP_COLOR_CONVERSION:
            case MFX_EXTBUFF_VPP_MCTF:
            case MFX_EXTBUFF_DEVICE_AFFINITY_MASK:
                buffer_list::add_buffer(o);
                return;
        }
        throw base_exception("Invalid buffer type", MFX_ERR_UNKNOWN);
    }

protected:
    /// @brief Empty ctor helper
    void ctor_helper() {}

    /// @brief Variadic length ctor helper. It adds extension buffers one by one to the map of buffers
    /// @param[in] Opt First property
    /// @param[in] Opts Remaining properties
    /// @return Nothing
    template <typename... OptsT, class OptT>
    typename std::enable_if<
        std::is_same<ExtVPPDoNotUse*, OptT>::value || std::is_same<ExtVPPDenoise*, OptT>::value ||
        std::is_same<ExtVPPDetail*, OptT>::value || std::is_same<ExtVPPProcAmp*, OptT>::value ||
        std::is_same<ExtVPPDoUse*, OptT>::value ||
        std::is_same<ExtVPPFrameRateConversion*, OptT>::value ||
        std::is_same<ExtVPPImageStab*, OptT>::value ||
        std::is_same<ExtVPPComposite*, OptT>::value ||
        std::is_same<ExtVPPVideoSignalInfo*, OptT>::value ||
        std::is_same<ExtVPPDeinterlacing*, OptT>::value ||
        std::is_same<ExtVPPFieldProcessing*, OptT>::value ||
        std::is_same<ExtVPPRotation*, OptT>::value || std::is_same<ExtVPPScaling*, OptT>::value ||
        std::is_same<ExtVPPMirroring*, OptT>::value ||
        std::is_same<ExtVPPColorFill*, OptT>::value ||
        std::is_same<ExtColorConversion*, OptT>::value || std::is_same<ExtVppMctf*, OptT>::value ||
        std::is_same<ExtDeviceAffinityMask*, OptT>::value>::type
    ctor_helper(OptT Opt, OptsT... Opts) {
        add_buffer(Opt);
        ctor_helper(Opts...);
    }
};

/// @brief This class hold list of extension buffers used during VPP's processing stage
/// Those buffers are attached to the mfxVideoParam structure before Init call.
class vpp_process_list : public buffer_list {
    template <typename... Tail>
    struct AllBuffers : std::true_type {};
    template <typename T, typename... Tail>
    struct AllBuffers<T, Tail...>
            : std::conditional<std::is_same<ExtVppAuxData*, T>::value ||
                                   std::is_same<ExtDeviceAffinityMask*, T>::value,
                               AllBuffers<Tail...>,
                               std::false_type>::type {};

public:
    vpp_process_list() {
        ctor_helper();
    }

    /// @brief Variadic length ctor.
    /// This ctor accepts list of pointers to the extension buffers of pre-defined type:
    /// ExtVppAuxData,
    /// ExtDeviceAffinityMask
    /// @param[in] Opts List of property objects
    template <typename... OptsT,
              typename = typename std::enable_if<AllBuffers<OptsT...>::value>::type>
    explicit vpp_process_list(OptsT... Opts) {
        ctor_helper(Opts...);
    }

    /// @brief ctor
    /// @param[in] Opts Vector of Extension Buffers
    explicit vpp_process_list(std::vector<extension_buffer_base*> Opts) {
        for (auto opt : Opts) {
            add_buffer(opt);
        }
    }

    /// @brief Add extension buffer to list
    /// @param[in] o Extension Buffer
    void add_buffer(extension_buffer_base* o) {
        switch (o->get_ID()) {
            case MFX_EXTBUFF_VPP_AUXDATA:
            case MFX_EXTBUFF_DEVICE_AFFINITY_MASK:
                buffer_list::add_buffer(o);
                return;
        }
        throw base_exception("Invalid buffer type", MFX_ERR_UNKNOWN);
    }

protected:
    /// @brief Empty ctor helper
    void ctor_helper() {}

    /// @brief Variadic length ctor helper. It adds extension buffers one by one to the map of buffers
    /// @param[in] Opt First property
    /// @param[in] Opts Remaining properties
    /// @return Nothing
    template <typename... OptsT, class OptT>
    typename std::enable_if<std::is_same<ExtVppAuxData*, OptT>::value ||
                            std::is_same<ExtDeviceAffinityMask*, OptT>::value>::type
    ctor_helper(OptT Opt, OptsT... Opts) {
        add_buffer(Opt);
        ctor_helper(Opts...);
    }
};
} // namespace vpl
} // namespace oneapi
