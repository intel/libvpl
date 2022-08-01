/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <algorithm>
#include <exception>
#include <initializer_list>
#include <string>
#include <vector>
#include <array>

#include "vpl/mfxstructures.h"

#include "vpl/preview/payload.hpp"

namespace oneapi {
namespace vpl {

/// @brief Interface definition for extension buffers. Extension buffers are used to provide additional data
/// for the encoders/decoders/vpp instances or to retrieve additional data.
/// Each extension buffer reflects corresppnding structure from C API.
class extension_buffer_base {
public:
    /// @brief Default ctor.
    extension_buffer_base() {}

    /// @brief Default copy ctor.
    /// @param[in] other another object to use as data source
    extension_buffer_base(const extension_buffer_base& other) = default;

    /// @brief Default copy operator.
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    extension_buffer_base& operator=(const extension_buffer_base& other) = default;

    /// @brief Dtor.
    virtual ~extension_buffer_base() {}

    /// @brief Interface to retieve extension buffer ID in a form of FourCC code.
    /// @return Extension buffer ID.
    virtual uint32_t get_ID() const = 0;

    /// @brief Interface to retieve raw pointer to the C header of C sructure.
    /// @return Pointer to the header of extension buffer C strucure.
    virtual mfxExtBuffer* get_base_ptr() = 0;
};

/// @brief Utilitary intermediate class to typify extension buffer interface with
/// assotiated ID and C structure
/// @tparam T C structure
/// @tparam ID Assotiated ID with C structure
template <typename T, uint32_t ID>
class extension_buffer : public extension_buffer_base {
protected:
    /// @brief Utilitary meta class to verify that @p T type is real C type of extension buffers.
    class is_extension_buffer {
    private:
        typedef char Yes;
        typedef int No;

        template <typename C>
        static Yes& check(decltype(&C::Header));
        template <typename C>
        static No& check(...);

    public:
        /// @brief Meta enum for type check
        enum { value = sizeof(check<T>(0)) == sizeof(Yes) };
    };

public:
    /// @brief Default ctor
    template <
        typename check = typename std::enable_if<is_extension_buffer::value, mfxExtBuffer>::type>
    extension_buffer() : buffer_() {
        buffer_.Header.BufferSz = sizeof(T);
        buffer_.Header.BufferId = ID;
    }

    /// @brief Default copy ctor
    /// @param[in] other another object to use as data source
    extension_buffer(const extension_buffer& other) = default;

    /// @brief Default copy operator
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    extension_buffer& operator=(const extension_buffer& other) = default;
    virtual ~extension_buffer() {}

public:
    /// @brief Returns ID of the extension buffer in a form of FourCC code.
    /// @return Buffer ID
    uint32_t get_ID() const {
        return buffer_.Header.BufferId;
    }

    /// @brief Returns size of the extension buffer in bytes.
    /// @return Size in bytes
    uint32_t get_size() const {
        return buffer_.Header.BufferSz;
    }

    /// @brief Returns reference to underlying C structure with the extension buffer.
    /// @return Reference to underlying C structure.
    virtual T& get_ref() {
        return buffer_;
    }

    /// @brief Returns instance of underlying C structure with the extension buffer.
    /// @return Instance underlying C structure.
    T get() const {
        return buffer_;
    }

    /// @brief Returns raw pointer to @p mfxExtBuffer structure with the extension buffer.
    /// @return Raw pointer to underlying C structure.
    mfxExtBuffer* get_base_ptr() {
        return reinterpret_cast<mfxExtBuffer*>(&buffer_);
    }

    /// @brief Returns raw pointer to underlying C structure with the extension buffer.
    /// @return Raw pointer to underlying C structure.
    T* get_ptr() {
        return &buffer_;
    }

protected:
    /// @brief Underlying C structure
    T buffer_;
};

/// @brief Utilitary intermediate class to keep extension buffers without pointers. For such classes
/// default copy ctor and operator is used.
/// @tparam T C structure
/// @tparam ID Assotiated ID with C structure
template <typename T, uint32_t ID>
class extension_buffer_trival : public extension_buffer<T, ID> {
public:
    /// @brief Default ctor
    extension_buffer_trival() {}

    /// @brief Default copy ctor
    /// @param[in] other another object to use as data source
    extension_buffer_trival(const extension_buffer_trival& other) = default;

    /// @brief Default copy operator
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    extension_buffer_trival& operator=(const extension_buffer_trival& other) = default;
};

/// @brief Utilitary intermediate class to keep extension buffers with pointers. For such classes
/// copy ctor and operator must be implemented.
/// @tparam T C structure
/// @tparam ID Assotiated ID with C structure
template <typename T, uint32_t ID>
class extension_buffer_with_ptrs : public extension_buffer<T, ID> {
public:
    /// @brief Default ctor
    extension_buffer_with_ptrs() {}

    /// @brief Default copy ctor
    /// @param[in] other another object to use as data source
    extension_buffer_with_ptrs(const extension_buffer_with_ptrs& other) = default;

    /// @brief Default copy operator
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    extension_buffer_with_ptrs& operator=(const extension_buffer_with_ptrs& other) = default;

protected:
    /// @brief Prohibited method for structure with pointers
    /// @returns Reference to the raw object
    virtual T& get_ref() {
        return extension_buffer<T, ID>::get_ref(); // this is dangerouse for pointers!
    }
};

#define REGISTER_TRIVIAL_EXT_BUFFER(className, extBuffer, bufferID)                                 \
    /*! @brief Instantiation of the extension_buffer template class for given C structure and ID.*/ \
    class className : public extension_buffer_trival<extBuffer, bufferID> {                         \
    public:                                                                                         \
        /*! @brief Default ctor. */                                                                 \
        className() : extension_buffer_trival() {}                                                  \
    };

/*! @brief Instantiation of the extension_buffer template class for given C structure and ID.*/
class ExtCodingOption : public extension_buffer_trival<mfxExtCodingOption, MFX_EXTBUFF_CODING_OPTION> {
public:
    /*! @brief Default ctor. */
    ExtCodingOption() : extension_buffer_trival() {}

    /*! @brief Sets rate distortion option. */
    /*! @param[in] option Rate distortion option. */
    void set_rate_distortion_opt(coding_option option) {
        this->buffer_.RateDistortionOpt = (mfxU16)option;
    }

    /*! @brief Sets ME cost type. */
    /*! @param[in] type ME cost type. */
    void set_me_cost_type(uint16_t type) {
        this->buffer_.MECostType = type;
    }

    /*! @brief Sets ME search type. */
    /*! @param[in] type ME search type. */
    void set_me_search_type(uint16_t type) {
        this->buffer_.MESearchType = type;
    }

    /*! @brief Sets ME search window. */
    /*! @param[in] wnd ME search window. */
    void set_me_search_window(std::pair<int16_t,int16_t> wnd) {
        this->buffer_.MVSearchWindow.x = std::get<0>(wnd);
        this->buffer_.MVSearchWindow.y = std::get<1>(wnd);
    }

    /*! @brief Sets frame picture option. */
    /*! @param[in] option Frame picture option. */
    void set_frame_picture_opt(coding_option option) {
        this->buffer_.FramePicture = (mfxU16)option;
    }

    /*! @brief Sets CAVLC option. */
    /*! @param[in] option CAVLC option. */
    void set_cavlc_opt(coding_option option) {
        this->buffer_.CAVLC = (mfxU16)option;
    }

    /*! @brief Sets recovery point SEI option. */
    /*! @param[in] option Recovery point SEI option. */
    void set_recovery_point_sei_opt(coding_option option) {
        this->buffer_.RecoveryPointSEI = (mfxU16)option;
    }

    /*! @brief Sets MVC view output option. */
    /*! @param[in] option MVC view output option. */
    void set_view_output_opt(coding_option option) {
        this->buffer_.ViewOutput = (mfxU16)option;
    }

    /*! @brief Sets NAL HRD conformance option. */
    /*! @param[in] option NAL HRD conformance option. */
    void set_nal_hrd_conformance_opt(coding_option option) {
        this->buffer_.NalHrdConformance = (mfxU16)option;
    }

    /*! @brief Sets single SEI NAL unit option. */
    /*! @param[in] option Single SEI NAL unit option. */
    void set_single_sei_nal_unit_opt(coding_option option) {
        this->buffer_.SingleSeiNalUnit = (mfxU16)option;
    }

    /*! @brief Sets VUI VCL HRD parameters option. */
    /*! @param[in] option VUI VCL HRD parameters option. */
    void set_vui_vcl_hrd_parameters_opt(coding_option option) {
        this->buffer_.VuiVclHrdParameters = (mfxU16)option;
    }

    /*! @brief Sets reset ref list option. */
    /*! @param[in] option reset ref list option. */
    void set_reset_ref_list_opt(coding_option option) {
        this->buffer_.ResetRefList = (mfxU16)option;
    }

    /*! @brief Sets refernce picture marking repetition option. */
    /*! @param[in] option Refernce picture marking repetition option. */
    void set_ref_pic_mark_rep_opt(coding_option option) {
        this->buffer_.RefPicMarkRep = (mfxU16)option;
    }

    /*! @brief Sets AVC field output option. */
    /*! @param[in] option Field output option. */
    void set_field_output_opt(coding_option option) {
        this->buffer_.FieldOutput = (mfxU16)option;
    }

    /*! @brief Specifies the maximum number of frames buffered in a DPB. */
    /*! @param[in] frames specifies the maximum number of frames buffered in a DPB. */
    void set_max_dec_frame_buffering(uint16_t frames) {
        this->buffer_.MaxDecFrameBuffering = frames;
    }

    /*! @brief Sets access unit delimiter NAL option. */
    /*! @param[in] option Access unit delimiter NAL option. */
    void set_au_delimiter_opt(coding_option option) {
        this->buffer_.AUDelimiter = (mfxU16)option;
    }

    /*! @brief Sets picture timing SEI option. */
    /*! @param[in] option Picture timing SEI option. */
    void set_pic_timing_sei_opt(coding_option option) {
        this->buffer_.PicTimingSEI = (mfxU16)option;
    }

    /*! @brief Sets insert NAL HRD parameters in the VUI header option. */
    /*! @param[in] option Insert NAL HRD parameters in the VUI header option. */
    void set_vui_nal_hrd_parameters_opt(coding_option option) {
        this->buffer_.VuiNalHrdParameters = (mfxU16)option;
    }
};

enum class intra_refresh_types : uint16_t {
    no         = MFX_REFRESH_NO,
    vertical   = MFX_REFRESH_VERTICAL,
    horizontal = MFX_REFRESH_HORIZONTAL,
    slice      = MFX_REFRESH_SLICE,
};

enum class trellis_control : uint16_t {
    unknown = MFX_TRELLIS_UNKNOWN,
    off     = MFX_TRELLIS_OFF,
    i       = MFX_TRELLIS_I,
    p       = MFX_TRELLIS_P,
    b       = MFX_TRELLIS_B,
};
ENABLE_BIT_OPERATORS_WITH_ENUM(trellis_control);

enum class bref_control : uint16_t {
    unknown = MFX_B_REF_UNKNOWN,
    off     = MFX_B_REF_OFF,
    pyramid = MFX_B_REF_PYRAMID,
};

enum class look_ahead_down_sampling : uint16_t {
    unknown = MFX_LOOKAHEAD_DS_UNKNOWN,
    off     = MFX_LOOKAHEAD_DS_OFF,
    x2      = MFX_LOOKAHEAD_DS_2x,
    x4      = MFX_LOOKAHEAD_DS_4x,
};

enum class bp_sei_control : uint16_t {
    bydefault = MFX_BPSEI_DEFAULT,
    iframe  = MFX_BPSEI_IFRAME,
};

/*! @brief Instantiation of the extension_buffer template class for given C structure and ID.*/
class ExtCodingOption2 : public extension_buffer_trival<mfxExtCodingOption2, MFX_EXTBUFF_CODING_OPTION2> {
public:
    /*! @brief Default ctor. */
    ExtCodingOption2() : extension_buffer_trival() {}

    /*! @brief Sets intera refresh type. */
    /*! @param[in] type Intra refresh type. */
    void set_int_ref_type(intra_refresh_types type) {
        this->buffer_.IntRefType = (uint16_t)type;
    }

    /*! @brief Specifies number of pictures within refresh cycle starting from 2. */
    /*! @param[in] pictures Number of pictures within refresh cycle starting from 2. */
    void set_int_ref_cycle_size(uint16_t pictures) {
        this->buffer_.IntRefCycleSize = pictures;
    }

    /*! @brief Specifies QP difference for inserted intra MBs. */
    /*! @param[in] QPDelta QP difference for inserted intra MBs. */
    void set_int_ref_qp_delta(int16_t QPDelta) {
        this->buffer_.IntRefQPDelta = QPDelta;
    }

    /*! @brief Specifies maximum encoded frame size in bytes. */
    /*! @param[in] size Maximum encoded frame size in bytes. */
    void set_max_frame_size(uint32_t size) {
        this->buffer_.MaxFrameSize = size;
    }

    /*! @brief Specifies maximum slice size in bytes. */
    /*! @param[in] size Maximum slice size in bytes. */
    void set_max_slice_size(uint32_t size) {
        this->buffer_.MaxSliceSize = size;
    }

    /*! @brief Sets bitrate limit option. */
    /*! @param[in] option Bitrate limit option. */
    void set_bitrate_limit_opt(coding_option option) {
        this->buffer_.BitrateLimit = (mfxU16)option;
    }

    /*! @brief Sets macroblock level bitrate control option. */
    /*! @param[in] option macroblock level bitrate control option. */
    void set_mbbrc_opt(coding_option option) {
        this->buffer_.MBBRC = (mfxU16)option;
    }

    /*! @brief Sets external BRC option. */
    /*! @param[in] option external BRC option. */
    void set_extbrc_opt(coding_option option) {
        this->buffer_.ExtBRC = (mfxU16)option;
    }

    /*! @brief Specifies look ahead depth. */
    /*! @param[in] depth Look ahead depth. */
    void set_look_ahead_depth(int16_t depth) {
        this->buffer_.LookAheadDepth = depth;
    }

    /*! @brief Specifies trellis quantization cotrol. */
    /*! @param[in] tr Trellis quantization control. */
    void set_trellis(trellis_control tr) {
        this->buffer_.Trellis = (uint16_t)tr;
    }

    /*! @brief Sets picture parameter set repetition option. */
    /*! @param[in] option Picture parameter set repetition option. */
    void set_repeat_pps_opt(coding_option option) {
        this->buffer_.RepeatPPS = (mfxU16)option;
    }

    /*! @brief Controls usage of B-frames as reference. */
    /*! @param[in] type Usage of B-frames as reference. */
    void set_bref_type(bref_control type) {
        this->buffer_.BRefType = (uint16_t)type;
    }

    /*! @brief Sets adaptive I option. */
    /*! @param[in] option Adaptive I option. */
    void set_adaptive_i_opt(coding_option option) {
        this->buffer_.AdaptiveI = (mfxU16)option;
    }

    /*! @brief Sets adaptive B option. */
    /*! @param[in] option Adaptive B option. */
    void set_adaptive_b_opt(coding_option option) {
        this->buffer_.AdaptiveB = (mfxU16)option;
    }

    /*! @brief Controls Controls down sampling in look ahead bitrate control mode. */
    /*! @param[in] ds Down sampling in look ahead bitrate control mode. */
    void set_look_ahead_ds(look_ahead_down_sampling ds) {
        this->buffer_.LookAheadDS = (uint16_t)ds;
    }

    /*! @brief Specifies suggested slice size in number of macroblocks. */
    /*! @param[in] num Suggested slice size in number of macroblocks. */
    void set_num_mb_per_slice(uint16_t num) {
        this->buffer_.NumMbPerSlice = num;
    }

    /*! @brief Enables usage of skip frame parameter. */
    /*! @param[in] skip Skip frame parameter. */
    void set_skip_frame(skip_frame skip) {
        this->buffer_.SkipFrame = (uint16_t)skip;
    }

    /*! @brief Specifies minimum allowed QP value for I-frame types. */
    /*! @param[in] val Minimum allowed QP value for I-frame types. */
    void set_min_qpi(uint8_t val) {
        this->buffer_.MinQPI = val;
    }

    /*! @brief Specifies maximum allowed QP value for I-frame types. */
    /*! @param[in] val Maximum allowed QP value for I-frame types. */
    void set_max_qpi(uint8_t val) {
        this->buffer_.MaxQPI = val;
    }

    /*! @brief Specifies minimum allowed QP value for B-frame types. */
    /*! @param[in] val Minimum allowed QP value for B-frame types. */
    void set_min_qpb(uint8_t val) {
        this->buffer_.MinQPB = val;
    }

    /*! @brief Specifies maximum allowed QP value for B-frame types. */
    /*! @param[in] val Maximum allowed QP value for B-frame types. */
    void set_max_qpb(uint8_t val) {
        this->buffer_.MaxQPB = val;
    }

    /*! @brief Specifies minimum allowed QP value for P-frame types. */
    /*! @param[in] val Minimum allowed QP value for P-frame types. */
    void set_min_qpp(uint8_t val) {
        this->buffer_.MinQPP = val;
    }

    /*! @brief Specifies maximum allowed QP value for P-frame types. */
    /*! @param[in] val Maximum allowed QP value for P-frame types. */
    void set_max_qpp(uint8_t val) {
        this->buffer_.MaxQPP = val;
    }

    /*! @brief Sets fixed_frame_rate_flag in VUI. */
    /*! @param[in] option fixed_frame_rate_flag. */
    void set_fixed_framerate_opt(coding_option option) {
        this->buffer_.FixedFrameRate = (mfxU16)option;
    }

    /*! @brief Disables deblocking. */
    /*! @param[in] option Option. */
    void set_disable_deblocking_idc_opt(coding_option option) {
        this->buffer_.DisableDeblockingIdc = (mfxU16)option;
    }

    /*! @brief Completely disables VUI in the output bitstream. */
    /*! @param[in] option Option. */
    void set_disable_vui_opt(coding_option option) {
        this->buffer_.DisableVUI = (mfxU16)option;
    }

    /*! @brief Controls insertion of buffering period SEI in the encoded bitstream. */
    /*! @param[in] control Control. */
    void set_disable_vui_opt(bp_sei_control control) {
        this->buffer_.DisableVUI = (mfxU16)control;
    }

    /*! @brief Enables per-frame reporting of Mean Absolute Difference. */
    /*! @param[in] option Option. */
    void set_enable_mad_opt(coding_option option) {
        this->buffer_.EnableMAD = (mfxU16)option;
    }

    /*! @brief Enables use of raw frames for reference instead of reconstructed frames. */
    /*! @param[in] option Option. */
    void set_use_raw_ref_opt(coding_option option) {
        this->buffer_.UseRawRef = (mfxU16)option;
    }
};

/*! The WeightedPred enumerator itemizes weighted prediction modes. */
enum class weighted_pred : uint16_t {
    unknown   = MFX_WEIGHTED_PRED_UNKNOWN,
    bydefault = MFX_WEIGHTED_PRED_DEFAULT,
    expl      = MFX_WEIGHTED_PRED_EXPLICIT,
    impl      = MFX_WEIGHTED_PRED_IMPLICIT,
};

enum class scenario_info : uint16_t {
    unknown            = MFX_SCENARIO_UNKNOWN,
    display_remoting   = MFX_SCENARIO_DISPLAY_REMOTING,
    video_conference   = MFX_SCENARIO_VIDEO_CONFERENCE,
    archive            = MFX_SCENARIO_ARCHIVE,
    live_streaming     = MFX_SCENARIO_LIVE_STREAMING,
    camera_capture     = MFX_SCENARIO_CAMERA_CAPTURE,
    video_surveillance = MFX_SCENARIO_VIDEO_SURVEILLANCE,
    game_streaming     = MFX_SCENARIO_GAME_STREAMING,
    remote_gaming      = MFX_SCENARIO_REMOTE_GAMING,
};

enum class content_info : uint16_t {
    unknown           = MFX_CONTENT_UNKNOWN,
    full_screen_video = MFX_CONTENT_FULL_SCREEN_VIDEO,
    non_video_screen  = MFX_CONTENT_NON_VIDEO_SCREEN,
};

enum class p_ref_type : uint16_t {
    bydefault = MFX_P_REF_DEFAULT,
    simple    = MFX_P_REF_SIMPLE,
    pyramid   = MFX_P_REF_PYRAMID,
};

/*! @brief Instantiation of the extension_buffer template class for given C structure and ID.*/
class ExtCodingOption3 : public extension_buffer_trival<mfxExtCodingOption3, MFX_EXTBUFF_CODING_OPTION3> {
public:
    /*! @brief Default ctor. */
    ExtCodingOption3() : extension_buffer_trival() {}

    /*! @brief Sets the number of slices for I-frames. */
    /*! @param[in] num The number of slices for I-frames. */
    void set_num_slice_i(uint16_t num) {
        this->buffer_.NumSliceI = num;
    }

    /*! @brief Sets the number of slices for P-frames. */
    /*! @param[in] num The number of slices for P-frames. */
    void set_num_slice_p(uint16_t num) {
        this->buffer_.NumSliceP = num;
    }

    /*! @brief Sets the number of slices for B-frames. */
    /*! @param[in] num The number of slices for B-frames. */
    void set_num_slice_b(uint16_t num) {
        this->buffer_.NumSliceB = num;
    }

    /*! @brief Specifies the maximum bitrate averaged over a sliding window. */
    /*! @param[in] bitrate The maximum bitrate averaged over a sliding window. */
    void set_win_brc_max_avg_kbps(uint16_t bitrate) {
        this->buffer_.WinBRCMaxAvgKbps = bitrate;
    }

    /*! @brief Specifies sliding window size in frames. */
    /*! @param[in] size Sliding window size in frames. */
    void set_win_brc_size(uint16_t size) {
        this->buffer_.WinBRCSize = size;
    }

    /*! @brief Specifies quality factor. */
    /*! @param[in] qfactor Quality factor. */
    void set_qvbr_factor(uint16_t qfactor) {
        this->buffer_.QVBRQuality = qfactor;
    }

    /*! @brief Enables per-macroblock QP control. */
    /*! @param[in] option Option. */
    void set_enable_mbqp_opt(coding_option option) {
        this->buffer_.EnableMBQP = (mfxU16)option;
    }

    /*! @brief Specifies distance between the beginnings of the intra-refresh cycles in frames. */
    /*! @param[in] distance Distance between the beginnings of the intra-refresh cycles in frames.*/
    void set_int_ref_cycle_dist(uint16_t distance) {
        this->buffer_.IntRefCycleDist = distance;
    }

    /*! @brief Enables the ENC mode decision algorithm to bias to fewer B Direct/Skip types. */
    /*! @param[in] option Option. */
    void set_direct_bias_adjustment_opt(coding_option option) {
        this->buffer_.DirectBiasAdjustment = (mfxU16)option;
    }

    /*! @brief Enables global motion bias. */
    /*! @param[in] option Option. */
    void set_global_motion_bias_adjustment_opt(coding_option option) {
        this->buffer_.GlobalMotionBiasAdjustment = (mfxU16)option;
    }

    /*! @brief Specifies MV cost scaling factor. */
    /*! @param[in] sfactor Quality factor. */
    void set_mv_cost_scaling_factor(uint16_t sfactor) {
        this->buffer_.MVCostScalingFactor = sfactor;
    }

    /*! @brief Enables usage of @p ExtMBDisableSkipMap extension buffer. */
    /*! @param[in] option Option. */
    void set_mb_disable_skip_map_opt(coding_option option) {
        this->buffer_.MBDisableSkipMap = (mfxU16)option;
    }

    /*! @brief Controls weighted prediction mode. */
    /*! @param[in] mode Weighted prediction mode. */
    void set_weighted_pred(weighted_pred mode) {
        this->buffer_.WeightedPred = (uint16_t)mode;
    }

    /*! @brief Controls weighted prediction mode. */
    /*! @param[in] mode Weighted prediction mode. */
    void set_weighted_bi_pred(weighted_pred mode) {
        this->buffer_.WeightedBiPred = (uint16_t)mode;
    }

    /*! @brief Instructs encoder whether aspect ratio info should present in VUI parameters. */
    /*! @param[in] option Option. */
    void set_aspect_ratio_info_present_opt(coding_option option) {
        this->buffer_.AspectRatioInfoPresent = (mfxU16)option;
    }

    /*! @brief Instructs encoder whether overscan info should present in VUI parameters. */
    /*! @param[in] option Option. */
    void set_overscan_info_present_opt(coding_option option) {
        this->buffer_.OverscanInfoPresent = (mfxU16)option;
    }

    /*! @brief ON indicates that the cropped decoded pictures output are suitable for display using overscan. */
    /*! @param[in] option Option. */
    void set_overscan_appropriate_opt(coding_option option) {
        this->buffer_.OverscanAppropriate = (mfxU16)option;
    }

    /*! @brief Instructs encoder whether frame rate info should present in VUI parameters. */
    /*! @param[in] option Option. */
    void set_timing_info_present_opt(coding_option option) {
        this->buffer_.TimingInfoPresent = (mfxU16)option;
    }

    /*! @brief Instructs encoder whether bitstream restriction info should present in VUI parameters. */
    /*! @param[in] option Option. */
    void set_bitstream_restriction_opt(coding_option option) {
        this->buffer_.BitstreamRestriction = (mfxU16)option;
    }

    /*! @brief Corresponds to AVC syntax element low_delay_hrd_flag (VUI). */
    /*! @param[in] option Option. */
    void set_low_delay_hrd_opt(coding_option option) {
        this->buffer_.LowDelayHrd = (mfxU16)option;
    }

    /*! @brief When set to ON, one or more samples outside picture boundaries may be used in inter prediction. */
    /*! @param[in] option Option. */
    void set_motion_vectors_over_pic_boundaries_opt(coding_option option) {
        this->buffer_.MotionVectorsOverPicBoundaries = (mfxU16)option;
    }

    /*! @brief Provides a hint to encoder about the scenario for the encoding session. */
    /*! @param[in] info Scenario info. */
    void set_scenario_info(scenario_info info) {
        this->buffer_.ScenarioInfo = (uint16_t)info;
    }

    /*! @brief Provides a hint to encoder about the content for the encoding session. */
    /*! @param[in] info Content info. */
    void set_content_info(content_info info) {
        this->buffer_.ContentInfo = (uint16_t)info;
    }

    /*! @brief specifies the model of reference list construction and DPB management. */
    /*! @param[in] model Model. */
    void set_p_ref_type(p_ref_type model) {
        this->buffer_.PRefType = (uint16_t)model;
    }

    /*! @brief Instructs encoder whether internal fade detection algorithm should be used. */
    /*! @param[in] option Option. */
    void set_fade_detection_opt(coding_option option) {
        this->buffer_.FadeDetection = (mfxU16)option;
    }

    /*! @brief Set this flag to OFF to make HEVC encoder use regular P-frames instead of GPB. */
    /*! @param[in] option Option. */
    void set_gpb_opt(coding_option option) {
        this->buffer_.GPB = (mfxU16)option;
    }

    /*! @brief Specifies maximum encoded I-frame size in bytes. */
    /*! @param[in] size Maximum encoded I-frame size in bytes. */
    void set_max_frame_size_i(uint32_t size) {
        this->buffer_.MaxFrameSizeI = size;
    }

    /*! @brief Specifies maximum encoded P-frame size in bytes. */
    /*! @param[in] size Maximum encoded P-frame size in bytes. */
    void set_max_frame_size_p(uint32_t size) {
        this->buffer_.MaxFrameSizeP = size;
    }

    /*! @brief Enables QPOffset control. */
    /*! @param[in] option Option. */
    void set_enable_qp_offset_opt(coding_option option) {
        this->buffer_.EnableQPOffset = (mfxU16)option;
    }

    /*! @brief Specifies QP offset per pyramid layer when EnableQPOffset is set to ON and RateControlMethod is CQP. */
    /*! @param[in] QPOffset QP offsets. */
    void set_qp_offset(std::array<int16_t, 8> QPOffset) {
        std::copy(QPOffset.begin(), QPOffset.end(), this->buffer_.QPOffset);
    }

    /*! @brief Max number of active references for P-frames. */
    /*! @param[in] numRefs Max number of active references for P-frames. */
    void set_num_ref_active_p(std::array<uint16_t, 8> numRefs) {
        std::copy(numRefs.begin(), numRefs.end(), this->buffer_.NumRefActiveP);
    }

    /*! @brief Max number of active references for B-frames in reference picture list 0. */
    /*! @param[in] numRefs Max number of active references for B-frames in reference picture list 0. */
    void set_num_ref_active_b_l0(std::array<uint16_t, 8> numRefs) {
        std::copy(numRefs.begin(), numRefs.end(), this->buffer_.NumRefActiveBL0);
    }

    /*! @brief Max number of active references for B-frames in reference picture list 1. */
    /*! @param[in] numRefs Max number of active references for B-frames in reference picture list 1. */
    void set_num_ref_active_b_l1(std::array<uint16_t, 8> numRefs) {
        std::copy(numRefs.begin(), numRefs.end(), this->buffer_.NumRefActiveBL1);
    }

    /*! @brief For HEVC if this option is turned ON, the transform_skip_enabled_flag will be set to 1 in PPS. */
    /*! @param[in] option Option. */
    void set_transform_skip_opt(coding_option option) {
        this->buffer_.TransformSkip = (mfxU16)option;
    }

    /*! @brief Minus 1 specifies target encoding chroma format (see ChromaFormatIdc enumerator). */
    /*! @param[in] format Target format + 1. */
    void set_target_chroma_format_plus_1(uint16_t format) {
        this->buffer_.TargetChromaFormatPlus1 = format;
    }

    /*! @brief Sets target encoding bit-depth for luma samples. May differ from source bit-depth. */
    /*! @param[in] depth Target encoding bit-depth for luma samples. */
    void set_target_bit_depth_luma(uint16_t depth) {
        this->buffer_.TargetBitDepthLuma = depth;
    }

    /*! @brief Sets target encoding bit-depth for chroma samples. May differ from source bit-depth. */
    /*! @param[in] depth Target encoding bit-depth for chroma samples. */
    void set_target_bit_depth_chroma(uint16_t depth) {
        this->buffer_.TargetBitDepthChroma = depth;
    }

    /*! @brief Controls panic mode in AVC and MPEG2 encoders. */
    /*! @param[in] option Option. */
    void set_brc_panic_mode_opt(coding_option option) {
        this->buffer_.BRCPanicMode = (mfxU16)option;
    }

    /*! @brief Controls frame size tolerance. */
    /*! @param[in] option Option. */
    void set_low_delay_brc_opt(coding_option option) {
        this->buffer_.LowDelayBRC = (mfxU16)option;
    }

    /*! @brief Set this flag to ON to enable usage of @p ExtMBForceIntra for AVC encoder. */
    /*! @param[in] option Option. */
    void set_enable_mb_force_intra_opt(coding_option option) {
        this->buffer_.EnableMBForceIntra = (mfxU16)option;
    }

    /*! @brief Controls AVC encoder attempts to predict from small partitions. */
    /*! @param[in] option Option. */
    void set_repartition_check_enable_opt(coding_option option) {
        this->buffer_.RepartitionCheckEnable = (mfxU16)option;
    }

    /*! @brief Enables encoded units info available in ExtEncodedUnitsInfo. */
    /*! @param[in] option Option. */
    void set_encoded_units_info_opt(coding_option option) {
        this->buffer_.EncodedUnitsInfo = (mfxU16)option;
    }

    /*! @brief Enables in the HEVC encoder usage of the NAL unit type provided by the application. */
    /*! @param[in] option Option. */
    void set_enable_nal_unit_type_opt(coding_option option) {
        this->buffer_.EnableNalUnitType = (mfxU16)option;
    }

    /*! @brief Enables encoder to mark, modify, or remove LTR frames based on encoding parameters
               and content properties. */
    /*! @param[in] option Option. */
    void set_adaptive_ltr_opt(coding_option option) {
        this->buffer_.AdaptiveLTR = (mfxU16)option;
    }

    /*! @brief Enables adaptive selection of one of implementation-defined quantization matrices for each frame. */
    /*! @param[in] option Option. */
    void set_adaptive_cqm_opt(coding_option option) {
        this->buffer_.AdaptiveCQM = (mfxU16)option;
    }

    /*! @brief Enables adaptive selection of list of reference frames to imrove encoding quality. */
    /*! @param[in] option Option. */
    void set_adaptive_ref_opt(coding_option option) {
        this->buffer_.AdaptiveRef = (mfxU16)option;
    }
};

REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPDenoise2, mfxExtVPPDenoise2, MFX_EXTBUFF_VPP_DENOISE2)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPDetail, mfxExtVPPDetail, MFX_EXTBUFF_VPP_DETAIL)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPProcAmp, mfxExtVPPProcAmp, MFX_EXTBUFF_VPP_PROCAMP)

/*! @brief Instantiation of the extension_buffer template class for given C structure and ID.*/
class ExtVppAuxData : public extension_buffer_trival<mfxExtVppAuxData, MFX_EXTBUFF_VPP_AUXDATA> {
public:
    explicit ExtVppAuxData(pic_struct picStruct) : extension_buffer_trival() {
        this->buffer_.PicStruct = (uint16_t)picStruct;
    }
};

// I would assign special status for mfxEncodeCtrl structure for now
REGISTER_TRIVIAL_EXT_BUFFER(ExtVideoSignalInfo,
                            mfxExtVideoSignalInfo,
                            MFX_EXTBUFF_VIDEO_SIGNAL_INFO)
REGISTER_TRIVIAL_EXT_BUFFER(ExtAVCRefListCtrl, mfxExtAVCRefListCtrl, MFX_EXTBUFF_AVC_REFLIST_CTRL)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPFrameRateConversion,
                            mfxExtVPPFrameRateConversion,
                            MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPImageStab,
                            mfxExtVPPImageStab,
                            MFX_EXTBUFF_VPP_IMAGE_STABILIZATION)
REGISTER_TRIVIAL_EXT_BUFFER(ExtMasteringDisplayColourVolume,
                            mfxExtMasteringDisplayColourVolume,
                            MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME)
REGISTER_TRIVIAL_EXT_BUFFER(ExtContentLightLevelInfo,
                            mfxExtContentLightLevelInfo,
                            MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO)
REGISTER_TRIVIAL_EXT_BUFFER(ExtPictureTimingSEI,
                            mfxExtPictureTimingSEI,
                            MFX_EXTBUFF_PICTURE_TIMING_SEI)
REGISTER_TRIVIAL_EXT_BUFFER(ExtAvcTemporalLayers,
                            mfxExtAvcTemporalLayers,
                            MFX_EXTBUFF_AVC_TEMPORAL_LAYERS)
REGISTER_TRIVIAL_EXT_BUFFER(ExtEncoderCapability,
                            mfxExtEncoderCapability,
                            MFX_EXTBUFF_ENCODER_CAPABILITY)

/*! @brief Instantiation of the extension_buffer template class for given C structure and ID.*/
class ExtEncoderResetOption :
        public extension_buffer_trival<mfxExtEncoderResetOption, MFX_EXTBUFF_ENCODER_RESET_OPTION> {
public:
    explicit ExtEncoderResetOption(coding_option start_new_seq) : extension_buffer_trival() {
        this->buffer_.StartNewSequence = (uint16_t)start_new_seq;
    }
};

REGISTER_TRIVIAL_EXT_BUFFER(ExtAVCEncodedFrameInfo,
                            mfxExtAVCEncodedFrameInfo,
                            MFX_EXTBUFF_ENCODED_FRAME_INFO)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPVideoSignalInfo,
                            mfxExtVPPVideoSignalInfo,
                            MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO)
REGISTER_TRIVIAL_EXT_BUFFER(ExtEncoderROI, mfxExtEncoderROI, MFX_EXTBUFF_ENCODER_ROI)
REGISTER_TRIVIAL_EXT_BUFFER(ExtAVCRefLists, mfxExtAVCRefLists, MFX_EXTBUFF_AVC_REFLISTS)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPDeinterlacing,
                            mfxExtVPPDeinterlacing,
                            MFX_EXTBUFF_VPP_DEINTERLACING)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPFieldProcessing,
                            mfxExtVPPFieldProcessing,
                            MFX_EXTBUFF_VPP_FIELD_PROCESSING)
REGISTER_TRIVIAL_EXT_BUFFER(ExtDecVideoProcessing,
                            mfxExtDecVideoProcessing,
                            MFX_EXTBUFF_DEC_VIDEO_PROCESSING)
REGISTER_TRIVIAL_EXT_BUFFER(ExtChromaLocInfo, mfxExtChromaLocInfo, MFX_EXTBUFF_CHROMA_LOC_INFO)
REGISTER_TRIVIAL_EXT_BUFFER(ExtInsertHeaders, mfxExtInsertHeaders, MFX_EXTBUFF_INSERT_HEADERS)
REGISTER_TRIVIAL_EXT_BUFFER(ExtHEVCTiles, mfxExtHEVCTiles, MFX_EXTBUFF_HEVC_TILES)
REGISTER_TRIVIAL_EXT_BUFFER(ExtHEVCParam, mfxExtHEVCParam, MFX_EXTBUFF_HEVC_PARAM)
REGISTER_TRIVIAL_EXT_BUFFER(ExtDecodeErrorReport,
                            mfxExtDecodeErrorReport,
                            MFX_EXTBUFF_DECODE_ERROR_REPORT)

/*! @brief Instantiation of the extension_buffer template class for given C structure and ID.*/
class ExtDecodedFrameInfo
        : public extension_buffer_trival<mfxExtDecodedFrameInfo, MFX_EXTBUFF_DECODED_FRAME_INFO> {
public:
    /*! @brief Default ctor. */
    ExtDecodedFrameInfo() : extension_buffer_trival() {}

    explicit ExtDecodedFrameInfo(frame_type frameType) : extension_buffer_trival() {
        this->buffer_.FrameType = (uint16_t)frameType;
    }
};

REGISTER_TRIVIAL_EXT_BUFFER(ExtTimeCode, mfxExtTimeCode, MFX_EXTBUFF_TIME_CODE)
REGISTER_TRIVIAL_EXT_BUFFER(ExtHEVCRegion, mfxExtHEVCRegion, MFX_EXTBUFF_HEVC_REGION)
REGISTER_TRIVIAL_EXT_BUFFER(ExtPredWeightTable,
                            mfxExtPredWeightTable,
                            MFX_EXTBUFF_PRED_WEIGHT_TABLE)

/*! @brief Instantiation of the extension_buffer template class for given C structure and ID.*/
class ExtAVCRoundingOffset
        : public extension_buffer_trival<mfxExtAVCRoundingOffset, MFX_EXTBUFF_AVC_ROUNDING_OFFSET> {
public:
    /*! @brief Default ctor. */
    ExtAVCRoundingOffset() : extension_buffer_trival() {}

    /*! @brief Sets intra rounding offset. */
    /*! @param[in] offset Ofset from 0 to 7 inclusively. */
    void set_rounding_offset_intra(uint16_t offset) {
        this->buffer_.EnableRoundingIntra = MFX_CODINGOPTION_ON;
        this->buffer_.RoundingOffsetIntra = offset;
    }

    /*! @brief Sets inter rounding offset. */
    /*! @param[in] offset Ofset from 0 to 7 inclusively. */
    void set_rounding_offset_inter(uint16_t offset) {
        this->buffer_.EnableRoundingInter = MFX_CODINGOPTION_ON;
        this->buffer_.RoundingOffsetInter = offset;
    }
};

REGISTER_TRIVIAL_EXT_BUFFER(ExtDirtyRect, mfxExtDirtyRect, MFX_EXTBUFF_DIRTY_RECTANGLES)
REGISTER_TRIVIAL_EXT_BUFFER(ExtMoveRect, mfxExtMoveRect, MFX_EXTBUFF_MOVING_RECTANGLES)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPRotation, mfxExtVPPRotation, MFX_EXTBUFF_VPP_ROTATION)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPScaling, mfxExtVPPScaling, MFX_EXTBUFF_VPP_SCALING)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPMirroring, mfxExtVPPMirroring, MFX_EXTBUFF_VPP_MIRRORING)

/*! @brief Instantiation of the extension_buffer template class for given C structure and ID.*/
class ExtMVOverPicBoundaries
        : public extension_buffer_trival<mfxExtMVOverPicBoundaries, MFX_EXTBUFF_MV_OVER_PIC_BOUNDARIES> {
public:
    /*! @brief Default ctor. */
    ExtMVOverPicBoundaries() : extension_buffer_trival() {}

    /*! @brief Enables stick to the top boundary. */
    /*! @param[in] enable Enable or disable stick to the top boundary. */
    void stick_top(coding_option enable) {
        this->buffer_.StickTop = (uint16_t)enable;
    }

    /*! @brief Enables stick to the bottom boundary. */
    /*! @param[in] enable Enable or disable stick to the bottom boundary. */
    void stick_bottom(coding_option enable) {
        this->buffer_.StickBottom = (uint16_t)enable;
    }

    /*! @brief Enables stick to the left boundary. */
    /*! @param[in] enable Enable or disable stick to the left boundary. */
    void stick_left(coding_option enable) {
        this->buffer_.StickLeft = (uint16_t)enable;
    }

    /*! @brief Enables stick to the right boundary. */
    /*! @param[in] enable Enable or disable stick to the right boundary. */
    void stick_right(coding_option enable) {
        this->buffer_.StickRight = (uint16_t)enable;
    }
};

/*! @brief Instantiation of the extension_buffer template class for given C structure and ID.*/
class ExtVPPColorFill
        : public extension_buffer_trival<mfxExtVPPColorFill, MFX_EXTBUFF_VPP_COLORFILL> {
public:
    /*! @brief Default ctor. */
    ExtVPPColorFill() : extension_buffer_trival() {}

    /*! @brief Default ctor. */
    ExtVPPColorFill(coding_option enable) : extension_buffer_trival() {
        this->buffer_.Enable = (uint16_t)enable;
    }

    /*! @brief Enables stick to the top boundary. */
    /*! @param[in] enable Enable or disable fill outside of crop. */
    void enable_fill_outside(coding_option enable) {
        this->buffer_.Enable = (uint16_t)enable;
    }
};

REGISTER_TRIVIAL_EXT_BUFFER(ExtColorConversion,
                            mfxExtColorConversion,
                            MFX_EXTBUFF_VPP_COLOR_CONVERSION)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVP9TemporalLayers,
                            mfxExtVP9TemporalLayers,
                            MFX_EXTBUFF_VP9_TEMPORAL_LAYERS)

/*! @brief Instantiation of the extension_buffer template class for given C structure and ID.*/
class ExtVP9Param
        : public extension_buffer_trival<mfxExtVP9Param, MFX_EXTBUFF_VP9_PARAM> {
public:
    /*! @brief Default ctor. */
    ExtVP9Param() : extension_buffer_trival() {}

    /*! @brief Sets coded frame size in pixels. */
    /*! @param[in] size Coded frame size in pixels. */
    void set_frame_size(std::pair<uint16_t, uint16_t> size) {
        this->buffer_.FrameWidth = size.first;
        this->buffer_.FrameHeight = size.second;
    }

    /*! @brief Enables/disable encoder insert IVF container headers. */
    /*! @param[in] enable Enable or disable encoder insert IVF container headers. */
    void set_write_ivf_headers(coding_option enable) {
        this->buffer_.WriteIVFHeaders = (uint16_t)enable;
    }

    /*! @brief Specifies an offset for a particular quantization parameter. */
    /*! @param[in] offset Offset. */
    void set_offset_luma_dc(int16_t offset) {
        this->buffer_.QIndexDeltaLumaDC = offset;
    }

    /*! @brief Specifies an offset for a particular quantization parameter. */
    /*! @param[in] offset Offset. */
    void set_offset_chroma_dc(int16_t offset) {
        this->buffer_.QIndexDeltaChromaDC = offset;
    }

    /*! @brief Specifies an offset for a particular quantization parameter. */
    /*! @param[in] offset Offset. */
    void set_offset_chroma_ac(int16_t offset) {
        this->buffer_.QIndexDeltaChromaAC = offset;
    }

    /*! @brief Sets tile raws and columns. */
    /*! @param[in] size Coded frame size in pixels. */
    void set_tile_size(std::pair<uint16_t, uint16_t> size) {
        this->buffer_.NumTileRows = size.first;
        this->buffer_.NumTileColumns = size.second;
    }
};

REGISTER_TRIVIAL_EXT_BUFFER(ExtVppMctf, mfxExtVppMctf, MFX_EXTBUFF_VPP_MCTF)
REGISTER_TRIVIAL_EXT_BUFFER(ExtPartialBitstreamParam,
                            mfxExtPartialBitstreamParam,
                            MFX_EXTBUFF_PARTIAL_BITSTREAM_PARAM)
REGISTER_TRIVIAL_EXT_BUFFER(ExtAV1BitstreamParam,
                            mfxExtAV1BitstreamParam,
                            MFX_EXTBUFF_AV1_BITSTREAM_PARAM)
REGISTER_TRIVIAL_EXT_BUFFER(ExtAV1ResolutionParam,
                            mfxExtAV1ResolutionParam,
                            MFX_EXTBUFF_AV1_RESOLUTION_PARAM)
REGISTER_TRIVIAL_EXT_BUFFER(ExtAV1TileParam,
                            mfxExtAV1TileParam,
                            MFX_EXTBUFF_AV1_TILE_PARAM)
REGISTER_TRIVIAL_EXT_BUFFER(ExtAV1FilmGrainParam,
                            mfxExtAV1FilmGrainParam,
                            MFX_EXTBUFF_AV1_FILM_GRAIN_PARAM)
// extension buffers with pointers below

#define SCALAR_SETTER(type, name)  \
    /*! @brief Sets name value. */ \
    /*! @param[in] name Value. */  \
    void set_##name(type name) {   \
        this->buffer_.name = name; \
    }

#define ARRAY_SETTER(type, name, len)                    \
    /*! @brief Sets name value. */                       \
    /*! @param[in] name Value. */                        \
    void set_##name(const type name[len]) {              \
        std::copy(name, name + len, this->buffer_.name); \
    }

#define REGISTER_SINGLE_POINTER_EXT_BUFFER_BEGIN(className,                                         \
                                                 extBuffer,                                         \
                                                 bufferID,                                          \
                                                 ptr,                                               \
                                                 ptrType,                                           \
                                                 numElems)                                          \
    /*! @brief Instantiation of the extension_buffer template class for given C structure and ID.*/ \
    class className : public extension_buffer_with_ptrs<extBuffer, bufferID> {                      \
    public:                                                                                         \
        /*! @brief Creates object and initialize underlying strucure with data from the pointer. */ \
        /*! @param[in] ptr Ponter to the data.*/                                                    \
        explicit className(const std::vector<ptrType>& ptr) : extension_buffer_with_ptrs() {        \
            if (ptr.size()) {                                                                       \
                buffer_.numElems = (decltype(buffer_.numElems))ptr.size();                          \
                buffer_.ptr      = new ptrType[buffer_.numElems];                                   \
                std::copy(ptr.begin(), ptr.end(), buffer_.ptr);                                     \
            }                                                                                       \
            else {                                                                                  \
                buffer_.numElems = 0;                                                               \
                buffer_.ptr      = NULL;                                                            \
            }                                                                                       \
        }                                                                                           \
                                                                                                    \
        /*! @brief Copy ctor. */                                                                    \
        /*! @param[in] other another object to use as data source. */                               \
        className(const className& other) {                                                         \
            this->buffer_ = other.buffer_;                                                          \
            if (other.buffer_.numElems) {                                                           \
                this->buffer_.numElems = other.buffer_.numElems;                                    \
                this->buffer_.ptr      = new ptrType[buffer_.numElems];                             \
                std::copy(other.buffer_.ptr,                                                        \
                          other.buffer_.ptr + other.buffer_.numElems,                               \
                          this->buffer_.ptr);                                                       \
            }                                                                                       \
        }                                                                                           \
                                                                                                    \
        /*! @brief Dtor. */                                                                         \
        ~className() {                                                                              \
            buffer_.numElems = 0;                                                                   \
            delete[] buffer_.ptr;                                                                   \
        }                                                                                           \
        /*! @brief Copy operator. */                                                                \
        /*! @param[in] other another object to use as data source. */                               \
        /*! @returns Reference to this object */                                                    \
        virtual className& operator=(const className& other) {                                      \
            if (&other == this)                                                                     \
                return *this;                                                                       \
                                                                                                    \
            if (this->buffer_.ptr) {                                                                \
                delete[] this->buffer_.ptr;                                                         \
                this->buffer_.ptr = NULL;                                                           \
            }                                                                                       \
                                                                                                    \
            if (other.buffer_.numElems) {                                                           \
                this->buffer_.numElems = other.buffer_.numElems;                                    \
                this->buffer_.ptr      = new ptrType[buffer_.numElems];                             \
                std::copy(other.buffer_.ptr,                                                        \
                          other.buffer_.ptr + other.buffer_.numElems,                               \
                          this->buffer_.ptr);                                                       \
            }                                                                                       \
            else {                                                                                  \
                buffer_.numElems = 0;                                                               \
            }                                                                                       \
                                                                                                    \
            return *this;                                                                           \
        }

#define REGISTER_SINGLE_POINTER_EXT_BUFFER_END \
    }                                          \
    ;

REGISTER_SINGLE_POINTER_EXT_BUFFER_BEGIN(ExtAV1Segmentation,
                                         mfxExtAV1Segmentation,
                                         MFX_EXTBUFF_AV1_SEGMENTATION,
                                         SegmentIds,
                                         uint8_t,
                                         NumSegmentIdAlloc)
SCALAR_SETTER(uint8_t, NumSegments)
ARRAY_SETTER(mfxAV1SegmentParam, Segment, 8)
SCALAR_SETTER(uint16_t, SegmentIdBlockSize)
REGISTER_SINGLE_POINTER_EXT_BUFFER_END

REGISTER_SINGLE_POINTER_EXT_BUFFER_BEGIN(ExtTemporalLayers,
                                         mfxExtTemporalLayers,
                                         MFX_EXTBUFF_UNIVERSAL_TEMPORAL_LAYERS,
                                         Layers,
                                         mfxTemporalLayer,
                                         NumLayers)
SCALAR_SETTER(uint16_t, BaseLayerPID)
REGISTER_SINGLE_POINTER_EXT_BUFFER_END


REGISTER_SINGLE_POINTER_EXT_BUFFER_BEGIN(ExtCodingOptionVPS,
                                         mfxExtCodingOptionVPS,
                                         MFX_EXTBUFF_CODING_OPTION_VPS,
                                         VPSBuffer,
                                         uint8_t,
                                         VPSBufSize)
SCALAR_SETTER(uint16_t, VPSId)
REGISTER_SINGLE_POINTER_EXT_BUFFER_END

REGISTER_SINGLE_POINTER_EXT_BUFFER_BEGIN(ExtVPPComposite,
                                         mfxExtVPPComposite,
                                         MFX_EXTBUFF_CODING_OPTION_VPS,
                                         InputStream,
                                         mfxVPPCompInputStream,
                                         NumInputStream)
SCALAR_SETTER(uint16_t, Y)
SCALAR_SETTER(uint16_t, R)
SCALAR_SETTER(uint16_t, U)
SCALAR_SETTER(uint16_t, G)
SCALAR_SETTER(uint16_t, V)
SCALAR_SETTER(uint16_t, B)
SCALAR_SETTER(uint16_t, NumTiles)
REGISTER_SINGLE_POINTER_EXT_BUFFER_END

REGISTER_SINGLE_POINTER_EXT_BUFFER_BEGIN(ExtEncoderIPCMArea,
                                         mfxExtEncoderIPCMArea,
                                         MFX_EXTBUFF_ENCODER_IPCM_AREA,
                                         Areas,
                                         mfxExtEncoderIPCMArea::area,
                                         NumArea);
REGISTER_SINGLE_POINTER_EXT_BUFFER_END

REGISTER_SINGLE_POINTER_EXT_BUFFER_BEGIN(ExtMBForceIntra,
                                         mfxExtMBForceIntra,
                                         MFX_EXTBUFF_MB_FORCE_INTRA,
                                         Map,
                                         uint8_t,
                                         MapSize)
REGISTER_SINGLE_POINTER_EXT_BUFFER_END

REGISTER_SINGLE_POINTER_EXT_BUFFER_BEGIN(ExtMBDisableSkipMap,
                                         mfxExtMBDisableSkipMap,
                                         MFX_EXTBUFF_MB_DISABLE_SKIP_MAP,
                                         Map,
                                         uint8_t,
                                         MapSize)
REGISTER_SINGLE_POINTER_EXT_BUFFER_END

REGISTER_SINGLE_POINTER_EXT_BUFFER_BEGIN(ExtEncodedSlicesInfo,
                                         mfxExtEncodedSlicesInfo,
                                         MFX_EXTBUFF_ENCODED_SLICES_INFO,
                                         SliceSize,
                                         uint16_t,
                                         NumSliceSizeAlloc)
SCALAR_SETTER(uint16_t, SliceSizeOverflow)
SCALAR_SETTER(uint16_t, NumSliceNonCopliant)
SCALAR_SETTER(uint16_t, NumEncodedSlice)
REGISTER_SINGLE_POINTER_EXT_BUFFER_END

REGISTER_SINGLE_POINTER_EXT_BUFFER_BEGIN(ExtVP9Segmentation,
                                         mfxExtVP9Segmentation,
                                         MFX_EXTBUFF_VP9_SEGMENTATION,
                                         SegmentId,
                                         uint8_t,
                                         NumSegmentIdAlloc)
SCALAR_SETTER(uint16_t, NumSegments)
SCALAR_SETTER(uint16_t, SegmentIdBlockSize)
ARRAY_SETTER(mfxVP9SegmentParam, Segment, 8)
REGISTER_SINGLE_POINTER_EXT_BUFFER_END

REGISTER_SINGLE_POINTER_EXT_BUFFER_BEGIN(ExtEncodedUnitsInfo,
                                         mfxExtEncodedUnitsInfo,
                                         MFX_EXTBUFF_ENCODED_UNITS_INFO,
                                         UnitInfo,
                                         mfxEncodedUnitInfo,
                                         NumUnitsAlloc)
SCALAR_SETTER(uint16_t, NumUnitsEncoded)
REGISTER_SINGLE_POINTER_EXT_BUFFER_END

// Complex extensions with more than 1 pointer

/// @brief represents class to manage VPP algorithms exclude list
class ExtVPPDoNotUse
        : public extension_buffer_with_ptrs<mfxExtVPPDoNotUse, MFX_EXTBUFF_VPP_DONOTUSE> {
public:
    /// @brief Constructs object with the list of VPP algorithms to exclude from the processing
    /// @param[in] algos List of VPP algorithms
    ExtVPPDoNotUse(const std::initializer_list<uint32_t>& algos) : extension_buffer_with_ptrs() {
        buffer_.NumAlg  = static_cast<uint32_t>(algos.size());
        buffer_.AlgList = new uint32_t[buffer_.NumAlg];

        std::copy(std::begin(algos), std::end(algos), this->buffer_.AlgList);
    }

    /// @brief Constructs object with the list of VPP algorithms to exclude from the processing
    /// @param[in] algos List of VPP algorithms
    explicit ExtVPPDoNotUse(const std::vector<uint32_t>& algos) : extension_buffer_with_ptrs() {
        buffer_.NumAlg  = static_cast<uint32_t>(algos.size());
        buffer_.AlgList = new uint32_t[buffer_.NumAlg];

        std::copy(std::begin(algos), std::end(algos), this->buffer_.AlgList);
    }

    /// @brief Copy ctor
    /// @param[in] other another object to use as data source
    explicit ExtVPPDoNotUse(const ExtVPPDoNotUse& other) {
        this->buffer_.NumAlg  = other.buffer_.NumAlg;
        this->buffer_.AlgList = new uint32_t[other.buffer_.NumAlg];

        std::copy(other.buffer_.AlgList,
                  other.buffer_.AlgList + other.buffer_.NumAlg,
                  this->buffer_.AlgList);
    }

    /// @brief Dtor
    ~ExtVPPDoNotUse() {
        delete[] buffer_.AlgList;
        buffer_.NumAlg = 0;
    }

    /// @brief Copy operator
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    virtual ExtVPPDoNotUse& operator=(const ExtVPPDoNotUse& other) {
        if (&other == this)
            return *this;

        if (this->buffer_.AlgList)
            delete[] this->buffer_.AlgList;

        this->buffer_.NumAlg  = other.buffer_.NumAlg;
        this->buffer_.AlgList = new uint32_t[other.buffer_.NumAlg];

        std::copy(other.buffer_.AlgList,
                  other.buffer_.AlgList + other.buffer_.NumAlg,
                  this->buffer_.AlgList);
        return *this;
    }
};

/// @brief Manages VPP algorithms exclude list
class ExtVPPDoUse : public extension_buffer_with_ptrs<mfxExtVPPDoUse, MFX_EXTBUFF_VPP_DOUSE> {
public:
    /// @brief Constructs object with the list of VPP algorithms to include into the processing
    /// @param[in] algos List of VPP algorithms
    ExtVPPDoUse(const std::initializer_list<uint32_t>& algos) : extension_buffer_with_ptrs() {
        buffer_.NumAlg  = static_cast<uint32_t>(algos.size());
        buffer_.AlgList = new uint32_t[buffer_.NumAlg];

        std::copy(std::begin(algos), std::end(algos), this->buffer_.AlgList);
    }

    /// @brief Constructs object with the list of VPP algorithms to include into the processing
    /// @param[in] algos List of VPP algorithms
    explicit ExtVPPDoUse(const std::vector<uint32_t>& algos) : extension_buffer_with_ptrs() {
        buffer_.NumAlg  = static_cast<uint32_t>(algos.size());
        buffer_.AlgList = new uint32_t[buffer_.NumAlg];

        std::copy(std::begin(algos), std::end(algos), this->buffer_.AlgList);
    }

    /// @brief Copy ctor
    /// @param[in] other another object to use as data source
    explicit ExtVPPDoUse(const ExtVPPDoUse& other) {
        this->buffer_.NumAlg  = other.buffer_.NumAlg;
        this->buffer_.AlgList = new uint32_t[other.buffer_.NumAlg];

        std::copy(other.buffer_.AlgList,
                  other.buffer_.AlgList + other.buffer_.NumAlg,
                  this->buffer_.AlgList);
    }

    /// @brief Dtor
    ~ExtVPPDoUse() {
        delete[] buffer_.AlgList;
        buffer_.NumAlg = 0;
    }

    /// @brief Copy operator
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    virtual ExtVPPDoUse& operator=(const ExtVPPDoUse& other) {
        if (&other == this)
            return *this;

        if (this->buffer_.AlgList)
            delete[] this->buffer_.AlgList;

        this->buffer_.NumAlg  = other.buffer_.NumAlg;
        this->buffer_.AlgList = new uint32_t[other.buffer_.NumAlg];

        std::copy(other.buffer_.AlgList,
                  other.buffer_.AlgList + other.buffer_.NumAlg,
                  this->buffer_.AlgList);
        return *this;
    }
};

/// @brief Manages SPS and PPS buffers
class ExtCodingOptionSPSPPS : public extension_buffer_with_ptrs<mfxExtCodingOptionSPSPPS,
                                                                MFX_EXTBUFF_CODING_OPTION_SPSPPS> {
public:
    /// @brief Constructs object with SPS and PPB vectors
    /// @param[in] SPS SPS buffer
    /// @param[in] PPS PPS buffer
    ExtCodingOptionSPSPPS(const std::vector<uint8_t>& SPS, const std::vector<uint8_t>& PPS)
            : extension_buffer_with_ptrs() {
        // make a copy for now. Behaviour TBD later.
        if (SPS.size()) {
            buffer_.SPSBufSize = (uint16_t)SPS.size();
            buffer_.SPSBuffer  = new uint8_t[buffer_.SPSBufSize];
            std::copy(SPS.begin(), SPS.end(), buffer_.SPSBuffer);
        }
        else {
            buffer_.SPSBufSize = 0;
            buffer_.SPSBuffer  = NULL;
        }

        if (PPS.size()) {
            buffer_.PPSBufSize = (uint16_t)PPS.size();
            buffer_.PPSBuffer  = new uint8_t[buffer_.PPSBufSize];
            std::copy(PPS.begin(), PPS.end(), buffer_.PPSBuffer);
        }
        else {
            buffer_.PPSBufSize = 0;
            buffer_.PPSBuffer  = NULL;
        }
    }

    /// @brief Copy ctor
    /// @param[in] other another object to use as data source
    ExtCodingOptionSPSPPS(const ExtCodingOptionSPSPPS& other) {
        if (other.buffer_.SPSBufSize) {
            this->buffer_.SPSBufSize = other.buffer_.SPSBufSize;
            this->buffer_.SPSBuffer  = new uint8_t[buffer_.SPSBufSize];
            std::copy(other.buffer_.SPSBuffer,
                      other.buffer_.SPSBuffer + other.buffer_.SPSBufSize,
                      this->buffer_.SPSBuffer);
        }

        if (other.buffer_.PPSBufSize) {
            this->buffer_.PPSBufSize = other.buffer_.PPSBufSize;
            this->buffer_.PPSBuffer  = new uint8_t[buffer_.PPSBufSize];
            std::copy(other.buffer_.PPSBuffer,
                      other.buffer_.PPSBuffer + other.buffer_.PPSBufSize,
                      this->buffer_.PPSBuffer);
        }

        this->buffer_.SPSId = other.buffer_.SPSId;
        this->buffer_.PPSId = other.buffer_.PPSId;
    }

    /// @brief Dtor
    ~ExtCodingOptionSPSPPS() {
        delete[] buffer_.SPSBuffer;
        buffer_.SPSBufSize = 0;
        delete[] buffer_.PPSBuffer;
        buffer_.PPSBufSize = 0;
    }

    /// @brief Copy operator
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    virtual ExtCodingOptionSPSPPS& operator=(const ExtCodingOptionSPSPPS& other) {
        if (&other == this)
            return *this;

        if (this->buffer_.SPSBuffer)
            delete[] this->buffer_.SPSBuffer;
        if (this->buffer_.PPSBuffer)
            delete[] this->buffer_.PPSBuffer;

        if (other.buffer_.SPSBufSize) {
            this->buffer_.SPSBufSize = other.buffer_.SPSBufSize;
            this->buffer_.SPSBuffer  = new uint8_t[buffer_.SPSBufSize];
            std::copy(other.buffer_.SPSBuffer,
                      other.buffer_.SPSBuffer + other.buffer_.SPSBufSize,
                      this->buffer_.SPSBuffer);
        }
        else {
            buffer_.SPSBufSize = 0;
            buffer_.SPSBuffer  = NULL;
        }

        if (other.buffer_.PPSBufSize) {
            this->buffer_.PPSBufSize = other.buffer_.PPSBufSize;
            this->buffer_.PPSBuffer  = new uint8_t[buffer_.PPSBufSize];
            std::copy(other.buffer_.PPSBuffer,
                      other.buffer_.PPSBuffer + other.buffer_.PPSBufSize,
                      this->buffer_.PPSBuffer);
        }
        else {
            buffer_.PPSBufSize = 0;
            buffer_.PPSBuffer  = NULL;
        }

        this->buffer_.SPSId = other.buffer_.SPSId;
        this->buffer_.PPSId = other.buffer_.PPSId;

        return *this;
    }
    SCALAR_SETTER(uint16_t, SPSId)
    SCALAR_SETTER(uint16_t, PPSId)
};

/// @brief Manages per macro-block QP use strategies
class ExtMBQP : public extension_buffer_with_ptrs<mfxExtMBQP, MFX_EXTBUFF_MBQP> {
public:
    /// @brief Constructs object with QP array
    /// @param[in] Qp QP value
    explicit ExtMBQP(const std::vector<uint8_t>& Qp) : extension_buffer_with_ptrs() {
        // make a copy for now. Behaviour TBD later.
        buffer_.Mode = MFX_MBQP_MODE_QP_VALUE;
        if (Qp.size()) {
            buffer_.NumQPAlloc = static_cast<uint32_t>(Qp.size());
            buffer_.QP         = new uint8_t[buffer_.NumQPAlloc];
            std::copy(Qp.begin(), Qp.end(), buffer_.QP);
        }
        else {
            buffer_.NumQPAlloc = 0;
            buffer_.QP         = NULL;
        }
    }

    /// @brief Constructs object with array of QP deltas
    /// @param[in] DeltaQP Delta QP value
    explicit ExtMBQP(const std::vector<int8_t>& DeltaQP) : extension_buffer_with_ptrs() {
        // make a copy for now. Behaviour TBD later.
        buffer_.Mode = MFX_MBQP_MODE_QP_DELTA;
        if (DeltaQP.size()) {
            buffer_.NumQPAlloc = static_cast<uint32_t>(DeltaQP.size());
            buffer_.DeltaQP    = new mfxI8[buffer_.NumQPAlloc];
            std::copy(DeltaQP.begin(), DeltaQP.end(), buffer_.DeltaQP);
        }
        else {
            buffer_.NumQPAlloc = 0;
            buffer_.DeltaQP    = NULL;
        }
    }

    /// @brief Constructs object with QP mode values: QP or Delta QP
    /// @param[in] QPmode QP mode value
    explicit ExtMBQP(const std::vector<mfxQPandMode>& QPmode) : extension_buffer_with_ptrs() {
        // make a copy for now. Behaviour TBD later.
        buffer_.Mode = MFX_MBQP_MODE_QP_ADAPTIVE;
        if (QPmode.size()) {
            buffer_.NumQPAlloc = static_cast<uint32_t>(QPmode.size());
            buffer_.QPmode     = new mfxQPandMode[buffer_.NumQPAlloc];
            std::copy(QPmode.begin(), QPmode.end(), buffer_.QPmode);
        }
        else {
            buffer_.NumQPAlloc = 0;
            buffer_.QPmode     = NULL;
        }
    }

    /// @brief Copy ctor
    /// @param[in] other another object to use as data source
    explicit ExtMBQP(const ExtMBQP& other) {
        buffer_.Mode = other.buffer_.Mode;

        if (other.buffer_.NumQPAlloc) {
            this->buffer_.NumQPAlloc = other.buffer_.NumQPAlloc;
            if (buffer_.Mode == MFX_MBQP_MODE_QP_VALUE) {
                this->buffer_.QP = new uint8_t[buffer_.NumQPAlloc];
                std::copy(other.buffer_.QP,
                          other.buffer_.QP + other.buffer_.NumQPAlloc,
                          this->buffer_.QP);
            }
            else if (buffer_.Mode == MFX_MBQP_MODE_QP_DELTA) {
                this->buffer_.DeltaQP = new mfxI8[buffer_.NumQPAlloc];
                std::copy(other.buffer_.DeltaQP,
                          other.buffer_.DeltaQP + other.buffer_.NumQPAlloc,
                          this->buffer_.DeltaQP);
            }
            else if (buffer_.Mode == MFX_MBQP_MODE_QP_ADAPTIVE) {
                this->buffer_.QPmode = new mfxQPandMode[buffer_.NumQPAlloc];
                std::copy(other.buffer_.QPmode,
                          other.buffer_.QPmode + other.buffer_.NumQPAlloc,
                          this->buffer_.QPmode);
            }
            else {
                this->buffer_.QPmode = NULL;
            }
        }

        this->buffer_.BlockSize = other.buffer_.BlockSize;
    }

    /// @brief Dtor
    ~ExtMBQP() {
        if (this->buffer_.Mode == MFX_MBQP_MODE_QP_VALUE)
            delete[] this->buffer_.QP;
        if (this->buffer_.Mode == MFX_MBQP_MODE_QP_DELTA)
            delete[] this->buffer_.DeltaQP;
        if (this->buffer_.Mode == MFX_MBQP_MODE_QP_ADAPTIVE)
            delete[] this->buffer_.QPmode;
        buffer_.NumQPAlloc = 0;
    }

    /// @brief Copy operator
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    virtual ExtMBQP& operator=(const ExtMBQP& other) {
        if (&other == this)
            return *this;

        if (this->buffer_.NumQPAlloc) {
            if (this->buffer_.Mode == MFX_MBQP_MODE_QP_VALUE)
                delete[] this->buffer_.QP;
            if (this->buffer_.Mode == MFX_MBQP_MODE_QP_DELTA)
                delete[] this->buffer_.DeltaQP;
            if (this->buffer_.Mode == MFX_MBQP_MODE_QP_ADAPTIVE)
                delete[] this->buffer_.QPmode;
        }

        buffer_.Mode = other.buffer_.Mode;

        if (other.buffer_.NumQPAlloc) {
            this->buffer_.NumQPAlloc = other.buffer_.NumQPAlloc;
            if (buffer_.Mode == MFX_MBQP_MODE_QP_VALUE) {
                this->buffer_.QP = new uint8_t[buffer_.NumQPAlloc];
                std::copy(other.buffer_.QP,
                          other.buffer_.QP + other.buffer_.NumQPAlloc,
                          this->buffer_.QP);
            }
            else if (buffer_.Mode == MFX_MBQP_MODE_QP_DELTA) {
                this->buffer_.DeltaQP = new mfxI8[buffer_.NumQPAlloc];
                std::copy(other.buffer_.DeltaQP,
                          other.buffer_.DeltaQP + other.buffer_.NumQPAlloc,
                          this->buffer_.DeltaQP);
            }
            else if (buffer_.Mode == MFX_MBQP_MODE_QP_ADAPTIVE) {
                this->buffer_.QPmode = new mfxQPandMode[buffer_.NumQPAlloc];
                std::copy(other.buffer_.QPmode,
                          other.buffer_.QPmode + other.buffer_.NumQPAlloc,
                          this->buffer_.QPmode);
            }
            else {
                this->buffer_.QPmode = NULL;
            }
        }

        this->buffer_.BlockSize = other.buffer_.BlockSize;

        return *this;
    }
    SCALAR_SETTER(uint16_t, BlockSize)
};

/// @brief Manages DeviceAffinityMask
class ExtDeviceAffinityMask : public extension_buffer_with_ptrs<mfxExtDeviceAffinityMask,
                                                                MFX_EXTBUFF_DEVICE_AFFINITY_MASK> {
public:
    /// @brief Constructs object with the list of VPP algorithms to inlclude into the processing
    /// @param[in] deviceID String with DeviceID
    /// @param[in] numSubDevices Number of subdevices in DeviceID
    /// @param[in] mask mask buffer
    ExtDeviceAffinityMask(const std::string deviceID,
                          uint32_t numSubDevices,
                          const std::vector<uint8_t>& mask)
            : extension_buffer_with_ptrs(),
              mask_len_(0) {
        mask_len_ = buffer_.NumSubDevices = numSubDevices;
        buffer_.Mask                      = new uint8_t[mask.size()];

        std::copy(mask.begin(), mask.end(), this->buffer_.Mask);
        std::copy_n(std::begin(deviceID),
                    (std::min)(deviceID.size(), (size_t)MFX_STRFIELD_LEN),
                    this->buffer_.DeviceID);
    }

    /// @brief Copy ctor
    /// @param[in] other another object to use as data source
    ExtDeviceAffinityMask(const ExtDeviceAffinityMask& other) {
        this->mask_len_             = other.mask_len_;
        this->buffer_.NumSubDevices = other.buffer_.NumSubDevices;
        buffer_.Mask                = new uint8_t[mask_len_];

        std::copy(other.buffer_.Mask, other.buffer_.Mask + this->mask_len_, this->buffer_.Mask);
        std::copy_n(std::begin(other.buffer_.DeviceID), MFX_STRFIELD_LEN, this->buffer_.DeviceID);
    }

    /// @brief Dtor
    ~ExtDeviceAffinityMask() {
        delete[] buffer_.Mask;
        buffer_.NumSubDevices = 0;
        mask_len_             = 0;
        buffer_.DeviceID[0]   = '\0';
    }

    /// @brief Copy operator
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    virtual ExtDeviceAffinityMask& operator=(const ExtDeviceAffinityMask& other) {
        if (&other == this)
            return *this;

        if (this->buffer_.Mask)
            delete[] this->buffer_.Mask;

        this->mask_len_             = other.mask_len_;
        this->buffer_.NumSubDevices = other.buffer_.NumSubDevices;
        this->buffer_.Mask          = new uint8_t[this->mask_len_];

        std::copy(other.buffer_.Mask, other.buffer_.Mask + this->mask_len_, this->buffer_.Mask);
        std::copy_n(std::begin(other.buffer_.DeviceID), MFX_STRFIELD_LEN, this->buffer_.DeviceID);
        return *this;
    }

protected:
    /// @brief Data buffer length in bytes.
    uint32_t mask_len_;
};

/// @brief Manages Encode controller
class EncodeCtrl : public extension_buffer_with_ptrs<mfxEncodeCtrl, 0> {
public:
    /// @brief Constructs object with the user payload
    /// @param[in] userPayload Payload
    explicit EncodeCtrl(const std::vector<payload> &userPayload = {})
            : extension_buffer_with_ptrs(),
              payload_(userPayload) {
        set_payload();
    }

    /// @brief Constructs object with the user payload
    /// @param[in] frameType Frame Type
    /// @param[in] userPayload Payload
    explicit EncodeCtrl(frame_type frameType, std::vector<payload> userPayload = {})
            : extension_buffer_with_ptrs(),
              payload_(userPayload) {
        this->buffer_.FrameType = (uint16_t)frameType;
        set_payload();
    }

    /// @brief Constructs object with the user payload
    /// @param[in] frameType Frame Type
    /// @param[in] SkipFrame SkipFrame
    /// @param[in] QP QP
    /// @param[in] userPayload Payload
    explicit EncodeCtrl(frame_type frameType,
                        uint16_t SkipFrame,
                        uint16_t QP,
                        std::vector<payload> userPayload = {})
            : extension_buffer_with_ptrs(),
              payload_(userPayload) {
        this->buffer_.FrameType = (uint16_t)frameType;
        this->buffer_.SkipFrame = SkipFrame;
        this->buffer_.QP        = QP;
        set_payload();
    }

    /// @brief Constructs object with the user payload
    /// @param[in] MfxNalUnitType MfxNalUnitType
    /// @param[in] frameType Frame Type
    /// @param[in] SkipFrame SkipFrame
    /// @param[in] QP QP
    /// @param[in] userPayload Payload
    explicit EncodeCtrl(uint16_t MfxNalUnitType,
                        frame_type frameType,
                        uint16_t SkipFrame,
                        uint16_t QP,
                        std::vector<payload> payload = {})
            : extension_buffer_with_ptrs(),
              payload_(payload) {
        this->buffer_.MfxNalUnitType = MfxNalUnitType;
        this->buffer_.FrameType      = (uint16_t)frameType;
        this->buffer_.SkipFrame      = SkipFrame;
        this->buffer_.QP             = QP;
        set_payload();
    }

    /// @brief Copy ctor
    /// @param[in] other another object to use as data source
    EncodeCtrl(const EncodeCtrl& other) {
        this->buffer_.MfxNalUnitType = other.buffer_.MfxNalUnitType;
        this->buffer_.FrameType      = other.buffer_.FrameType;
        this->buffer_.SkipFrame      = other.buffer_.SkipFrame;
        this->buffer_.QP             = other.buffer_.QP;
        this->payload_               = other.payload_;

        set_payload();
    }

    /// @brief Dtor
    ~EncodeCtrl() {
        delete[] buffer_.Payload;
        buffer_.Payload = 0;
    }

    /// @brief Copy operator
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    virtual EncodeCtrl& operator=(const EncodeCtrl& other) {
        if (&other == this)
            return *this;

        if (this->buffer_.Payload)
            delete[] this->buffer_.Payload;

        this->buffer_.MfxNalUnitType = other.buffer_.MfxNalUnitType;
        this->buffer_.FrameType      = other.buffer_.FrameType;
        this->buffer_.SkipFrame      = other.buffer_.SkipFrame;
        this->buffer_.QP             = other.buffer_.QP;
        this->payload_               = other.payload_;

        set_payload();
        return *this;
    }

protected:
    /// @brief Set payload
    void set_payload() {
        this->buffer_.NumPayload = static_cast<uint16_t>(payload_.size());
        if (this->buffer_.NumPayload) {
            int i                 = 0;
            this->buffer_.Payload = new mfxPayload*[this->buffer_.NumPayload];
            for (auto& p : payload_) {
                this->buffer_.Payload[i] = p.get_raw_ptr();
                i++;
            }
        }
    }
    std::vector<payload> payload_;
};

} // namespace vpl
} // namespace oneapi
