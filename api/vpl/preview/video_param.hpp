/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <algorithm>
#include <iostream>
#include <utility>
#include  <tuple>

#include "vpl/preview/detail/string_helpers.hpp"

namespace oneapi {
namespace vpl {

#define DECLARE_MEMBER_ACCESS(father, type, name) \
    /*! @brief Returns name value. */             \
    /*! @return name value. */                    \
    type get_##name() const {                     \
        return param_.name;                       \
    }                                             \
    /*! @brief Sets name value. */                \
    /*! @param[in] name Value. */                 \
    /*! @return Reference to the instance. */     \
    father &set_##name(type name) {               \
        param_.name = name;                       \
        return *this;                             \
    }

#define DECLARE_INNER_MEMBER_ACCESS(father, type, parent, name) \
    /*! @brief Returns name value. */                           \
    /*! @return name value. */                                  \
    type get_##name() const {                                   \
        return param_.parent.name;                              \
    }                                                           \
    /*! @brief Sets name value. */                              \
    /*! @param[in] name Value. */                               \
    /*! @return Reference to the instance. */                   \
    father &set_##name(type name) {                             \
        param_.parent.name = name;                              \
        return *this;                                           \
    }

#define DECLARE_INNER_MEMBER_ARRAY_ACCESS(father, type, size, parent, name) \
    /*! @brief Returns name value. */                                       \
    /*! @return name value. */                                              \
    auto get_##name() const {                                               \
        return param_.parent.name;                                          \
    }                                                                       \
    /*! @brief Sets name value. */                                          \
    /*! @param[in] name Value. */                                           \
    /*! @return Reference to the instance. */                               \
    father &set_##name(type name[size]) {                                   \
        std::copy(name, name + size, param_.parent.name);                   \
        return *this;                                                       \
    }

/// @brief Holds general video params applicable for all kind of sessions.
class video_param {
public:
    /// @brief Constructs params and initialize them with default values.
    video_param() : param_() {}
    video_param(const video_param &param) : param_(param.param_) {
        clear_extension_buffers();
    }

    video_param& operator=(const video_param& other){
        param_ = other.param_;
        clear_extension_buffers();
        return *this;
    }

    /// @brief Dtor
    virtual ~video_param() {
        param_ = {};
    }

public:
    /// @brief Returns pointer to raw data
    /// @return Pointer to raw data
    mfxVideoParam *getMfx() {
        return &param_;
    }

public:
    DECLARE_MEMBER_ACCESS(video_param, uint32_t, AllocId)
    DECLARE_MEMBER_ACCESS(video_param, uint16_t, AsyncDepth)
    DECLARE_MEMBER_ACCESS(video_param, uint16_t, Protected)

    /// @brief Returns i/o memory pattern value.
    /// @return i/o memory pattern value.
    io_pattern get_IOPattern() const {
        return (io_pattern)param_.IOPattern;
    }

    /// @brief Sets i/o memory pattern value.
    /// @param[in] IOPattern i/o memory pattern.
    /// @return Reference to this object
    video_param &set_IOPattern(io_pattern IOPattern) {
        param_.IOPattern = (uint32_t)IOPattern;
        return *this;
    }

    /// @brief Attaches extension buffers to the video params
    /// @param[in] buffer Array of extension buffers
    /// @param[in] num Number of extension buffers
    /// @return Reference to this object
    video_param &set_extension_buffers(mfxExtBuffer **buffer, uint16_t num) {
        param_.ExtParam    = buffer;
        param_.NumExtParam = num;
        return *this;
    }

    /// @brief Clear extension buffers from the video params
    /// @param[in] buffer Array of extension buffers
    /// @param[in] num Number of extension buffers
    /// @return Reference to this object
    video_param &clear_extension_buffers() {
        param_.ExtParam    = nullptr;
        param_.NumExtParam = 0;
        return *this;
    }

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] p Referebce to the video_param instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream &operator<<(std::ostream &out, const video_param &p);

protected:
    /// @brief Raw data
    mfxVideoParam param_;
};

inline std::ostream &operator<<(std::ostream &out, const video_param &p) {
    out << "Base:" << std::endl;
    out << detail::space(detail::INTENT, out, "AllocId    = ") << p.param_.AllocId << std::endl;
    out << detail::space(detail::INTENT, out, "AsyncDepth = ")
        << detail::NotSpecifyed0(p.param_.AsyncDepth) << std::endl;
    out << detail::space(detail::INTENT, out, "Protected  = ") << p.param_.Protected << std::endl;
    out << detail::space(detail::INTENT, out, "IOPattern  = ")
        << detail::IOPattern2String(p.param_.IOPattern) << std::endl;
    return out;
}

class codec_video_param;
class vpp_video_param;

/// @brief Holds general frame related params.
class frame_info {
public:
    /// @brief Default ctor.
    frame_info() : param_() {}

    /// @brief Copy ctor.
    /// @param[in] other another object to use as data source
    frame_info(const frame_info &other) {
        param_ = other.param_;
    }

    /// @brief Constructs object from the raw data.
    /// @param[in] other another object to use as data source
    explicit frame_info(const mfxFrameInfo &other) {
        param_ = other;
    }

    /// @brief Copy operator.
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    frame_info &operator=(const frame_info &other) {
        param_ = other.param_;
        return *this;
    }

    DECLARE_MEMBER_ACCESS(frame_info, uint16_t, BitDepthLuma)
    DECLARE_MEMBER_ACCESS(frame_info, uint16_t, BitDepthChroma)
    DECLARE_MEMBER_ACCESS(frame_info, uint16_t, Shift)
    DECLARE_MEMBER_ACCESS(frame_info, mfxFrameId, FrameId)

    /// @brief Returns color format fourCC value.
    /// @return color format fourCC value.
    color_format_fourcc get_FourCC() const {
        return (color_format_fourcc)param_.FourCC;
    }

    /// @brief Sets color format fourCC value.
    /// @param[in] FourCC color format fourCC.
    /// @return Reference to this object
    frame_info &set_FourCC(color_format_fourcc FourCC) {
        param_.FourCC = (uint32_t)FourCC;
        return *this;
    }

    /// @todo below group valid for formats != P8

    /// @brief Returns frame size.
    /// @return Pair of width and height.
    auto get_frame_size() const {
        return std::pair(param_.Width, param_.Height);
    }

    /// @brief Sets frame size value.
    /// @param[in] size pair of width and height.
    /// @return Reference to this object
    frame_info &set_frame_size(std::pair<uint32_t, uint32_t> size) {
        param_.Width  = std::get<0>(size);
        param_.Height = std::get<1>(size);
        return *this;
    }

    /// @brief Returns ROI.
    /// @return Two pairs: pair of left corner and pair of size.
    auto get_ROI() const {
        return std::pair(std::pair(param_.CropX, param_.CropY),
                         std::pair(param_.CropW, param_.CropH));
    }

    /// @brief Sets ROI.
    /// @param[in] roi Two pairs: pair of left corner and pair of size.
    /// @return Reference to this object
    frame_info &set_ROI(
        std::pair<std::pair<uint16_t, uint16_t>, std::pair<uint16_t, uint16_t>> roi) {
        param_.CropX = std::get<0>(std::get<0>(roi));
        param_.CropY = std::get<1>(std::get<0>(roi));

        param_.CropW = std::get<0>(std::get<1>(roi));
        param_.CropH = std::get<1>(std::get<1>(roi));
        return *this;
    }

    /// @todo below method is valid for P8 format only
    DECLARE_MEMBER_ACCESS(frame_info, uint64_t, BufferSize)

    /// @brief Returns frame rate value.
    /// @return Pair of numerator and denominator.
    auto get_frame_rate() const {
        return std::pair(param_.FrameRateExtN, param_.FrameRateExtD);
    }

    /// @brief Sets frame rate value.
    /// @param[in] rate pair of numerator and denominator.
    /// @return Reference to this object
    frame_info &set_frame_rate(std::pair<uint32_t, uint32_t> rate) {
        param_.FrameRateExtN = std::get<0>(rate);
        param_.FrameRateExtD = std::get<1>(rate);
        return *this;
    }

    /// @brief Returns aspect ratio.
    /// @return Pair of width and height of aspect ratio.
    auto get_aspect_ratio() const {
        return std::pair(param_.AspectRatioW, param_.AspectRatioH);
    }

    /// @brief Sets aspect ratio.
    /// @param[in] ratio pair of width and height of aspect ratio.
    /// @return Reference to this object
    frame_info &set_aspect_ratio(std::pair<uint32_t, uint32_t> ratio) {
        param_.AspectRatioW = std::get<0>(ratio);
        param_.AspectRatioH = std::get<1>(ratio);
        return *this;
    }

    /// @brief Returns picture structure value.
    /// @return picture structure value.
    pic_struct get_PicStruct() const {
        return (pic_struct)param_.PicStruct;
    }

    /// @brief Sets picture structure value.
    /// @param[in] PicStruct picture structure.
    /// @return Reference to this object
    frame_info &set_PicStruct(pic_struct PicStruct) {
        param_.PicStruct = (uint16_t)PicStruct;
        return *this;
    }

    /// @brief Returns chroma format value.
    /// @return chroma format value.
    chroma_format_idc get_ChromaFormat() const {
        return (chroma_format_idc)param_.ChromaFormat;
    }

    /// @brief Sets chroma format value.
    /// @param[in] ChromaFormat chroma format.
    /// @return Reference to this object
    frame_info &set_ChromaFormat(chroma_format_idc ChromaFormat) {
        param_.ChromaFormat = (uint32_t)ChromaFormat;
        return *this;
    }

    /// @brief Friend class
    friend class codec_video_param;
    /// @brief Friend class
    friend class vpp_video_param;

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] p Reference to the codec_video_param instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream &operator<<(std::ostream &out, const codec_video_param &p);

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] v Reference to the vpp_video_param instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream &operator<<(std::ostream &out, const vpp_video_param &v);

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] f Reference to the frame_info instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream &operator<<(std::ostream &out, const frame_info &f);

    /// @brief Provides raw data.
    /// @return Raw data.
    mfxFrameInfo operator()() const {
        return param_;
    }

protected:
    /// @brief Raw data
    mfxFrameInfo param_;
};

inline std::ostream &operator<<(std::ostream &out, const frame_info &f) {
    out << detail::space(detail::INTENT, out, "BitDepthLuma   = ") << f.param_.BitDepthLuma
        << std::endl;
    out << detail::space(detail::INTENT, out, "BitDepthChroma = ") << f.param_.BitDepthChroma
        << std::endl;
    out << detail::space(detail::INTENT, out, "Shift          = ")
        << detail::NotSpecifyed0(f.param_.Shift) << std::endl;
    out << detail::space(detail::INTENT, out, "Color Format   = ")
        << detail::FourCC2String(f.param_.FourCC) << std::endl;
    if (f.param_.FourCC == MFX_FOURCC_P8) {
        out << detail::space(detail::INTENT, out, "BufferSize     = ") << f.param_.BufferSize
            << std::endl;
    }
    else {
        out << detail::space(detail::INTENT, out, "Size [W,H]     = [") << f.param_.Width << ","
            << f.param_.Height << "]" << std::endl;
        out << detail::space(detail::INTENT, out, "ROI [X,Y,W,H]  = [") << f.param_.CropX << ","
            << f.param_.CropY << "," << f.param_.CropW << "," << f.param_.CropH << "]" << std::endl;
    }

    out << detail::space(detail::INTENT, out, "FrameRate [N:D]= ")
        << detail::NotSpecifyed0(f.param_.FrameRateExtN) << ":"
        << detail::NotSpecifyed0(f.param_.FrameRateExtD) << std::endl;

    if (0 == f.param_.AspectRatioW && 0 == f.param_.AspectRatioH) {
        out << detail::space(detail::INTENT, out, "AspecRato [W,H]= [") << "Unset"
            << "]" << std::endl;
    }
    else {
        out << detail::space(detail::INTENT, out, "AspecRato [W,H]= [") << f.param_.AspectRatioW
            << "," << f.param_.AspectRatioH << "]" << std::endl;
    }

    out << detail::space(detail::INTENT, out, "PicStruct      = ")
        << detail::PicStruct2String(f.param_.PicStruct) << std::endl;
    out << detail::space(detail::INTENT, out, "ChromaFormat   = ")
        << detail::ChromaFormat2String(f.param_.ChromaFormat) << std::endl;

    return out;
}

/// @brief Holds general frame related params.
class frame_data {
public:
    /// @brief Default ctor.
    frame_data() : param_() {}

    /// @brief Copy ctor.
    /// @param[in] other another object to use as data source
    frame_data(const frame_data &other) {
        param_ = other.param_;
    }

    /// @brief Constructs object from the raw data.
    /// @param[in] other another object to use as data source
    explicit frame_data(const mfxFrameData &other) {
        param_ = other;
    }

    /// @brief Copy operator.
    /// @param[in] other another object to use as data source
    /// @return Reference to this object
    frame_data &operator=(const frame_data &other) {
        param_ = other.param_;
        return *this;
    }

    DECLARE_MEMBER_ACCESS(frame_data, uint16_t, MemType)

    /// @brief Returns pitch value.
    /// @return Pitch value.
    uint32_t get_pitch() const {
        return ((uint32_t)param_.PitchHigh << 16) | (uint32_t)(uint32_t)param_.PitchLow;
    }

    /// @brief Sets pitch value.
    /// @param[in] pitch Pitch.
    void set_pitch(uint32_t pitch) {
        param_.PitchHigh = (uint16_t)(pitch >> 16);
        param_.PitchLow  = (uint16_t)(pitch & 0xFFFF);
    }

    DECLARE_MEMBER_ACCESS(frame_data, uint64_t, TimeStamp)
    DECLARE_MEMBER_ACCESS(frame_data, uint32_t, FrameOrder)
    DECLARE_MEMBER_ACCESS(frame_data, uint16_t, Locked)
    DECLARE_MEMBER_ACCESS(frame_data, uint16_t, Corrupted)
    DECLARE_MEMBER_ACCESS(frame_data, uint16_t, DataFlag)

    /// @brief Gets pointer for formats with 1 plane.
    /// @return Pointer for formats with 1 plane.
    auto get_plane_ptrs_1() const {
        return param_.R;
    }

    /// @brief Gets pointer for formats with 2 planes.
    /// @return Pointers for formats with 2 planes. Pointers for planes layout is: Y, UV.
    auto get_plane_ptrs_2() const {
        return std::pair(param_.R, param_.G);
    }

    /// @brief Gets pointer for formats with 3 planes.
    /// @return Pointers for formats with 3 planes. Pointers for planes layout is: R, G, B or Y, U, V.
    auto get_plane_ptrs_3() const {
        return std::make_tuple(param_.R, param_.G, param_.B);
    }

    /// @brief Gets pointer for formats with 4 planes.
    /// @return Pointers for formats with 4 planes. Pointers for planes layout is: R, G, B, A.
    auto get_plane_ptrs_4() const {
        return std::make_tuple(param_.R, param_.G, param_.B, param_.A);
    }

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] f Reference to the frame_data instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream &operator<<(std::ostream &out, const frame_data &f);

protected:
    /// @brief Raw data
    mfxFrameData param_;
};

inline std::ostream &operator<<(std::ostream &out, const frame_data &f) {
    out << detail::space(detail::INTENT, out, "MemType    = ")
        << detail::MemType2String(f.param_.MemType) << std::endl;
    out << detail::space(detail::INTENT, out, "PitchHigh  = ") << f.param_.PitchHigh << std::endl;
    out << detail::space(detail::INTENT, out, "PitchLow   = ") << f.param_.PitchLow << std::endl;
    out << detail::space(detail::INTENT, out, "TimeStamp  = ")
        << detail::TimeStamp2String(static_cast<uint64_t>(f.param_.TimeStamp)) << std::endl;
    out << detail::space(detail::INTENT, out, "FrameOrder = ") << f.param_.FrameOrder << std::endl;
    out << detail::space(detail::INTENT, out, "Locked     = ") << f.param_.Locked << std::endl;
    out << detail::space(detail::INTENT, out, "Corrupted  = ")
        << detail::Corruption2String(f.param_.Corrupted) << std::endl;
    out << detail::space(detail::INTENT, out, "DataFlag   = ")
        << detail::TimeStamp2String(f.param_.DataFlag) << std::endl;

    return out;
}

/// @brief Holds general codec-specific params applicable for any decoder and encoder.
class codec_video_param : public video_param {
protected:
    /// @brief Constructs params and initialize them with default values.
    codec_video_param() : video_param() {}
    codec_video_param(const codec_video_param &param) : video_param(param) {}
    codec_video_param& operator=(const codec_video_param &param){
        video_param::operator=(param);
        return *this;
    }

public:
    DECLARE_INNER_MEMBER_ACCESS(codec_video_param, uint16_t, mfx, LowPower);
    DECLARE_INNER_MEMBER_ACCESS(codec_video_param, uint16_t, mfx, BRCParamMultiplier);

    /// @brief Returns codec fourCC value.
    /// @return codec fourCC value.
    codec_format_fourcc get_CodecId() const {
        return (codec_format_fourcc)param_.mfx.CodecId;
    }

    /// @brief Sets codec fourCC value.
    /// @param[in] CodecID codec fourCC.
    /// @return Reference to this object
    codec_video_param &set_CodecId(codec_format_fourcc CodecID) {
        param_.mfx.CodecId = (uint32_t)CodecID;
        return *this;
    }

    DECLARE_INNER_MEMBER_ACCESS(codec_video_param, uint16_t, mfx, CodecProfile);
    DECLARE_INNER_MEMBER_ACCESS(codec_video_param, uint16_t, mfx, CodecLevel);
    DECLARE_INNER_MEMBER_ACCESS(codec_video_param, uint16_t, mfx, NumThread);

    /// @brief Returns frame_info value.
    /// @return frame info value.
    frame_info get_frame_info() const {
        return frame_info(param_.mfx.FrameInfo);
    }

    /// @brief Sets name value.
    /// @param[in] name Value.
    /// @return Reference to this object
    codec_video_param &set_frame_info(frame_info name) {
        param_.mfx.FrameInfo = name();
        return *this;
    }

    /// Friend operator to print out state of the class in human readable form.
    friend std::ostream &operator<<(std::ostream &out, const codec_video_param &p);
};

inline std::ostream &operator<<(std::ostream &out, const codec_video_param &p) {
    const video_param &v = dynamic_cast<const video_param &>(p);
    out << v;
    out << "Codec:" << std::endl;
    out << detail::space(detail::INTENT, out, "LowPower           = ")
        << detail::TriState2String(p.param_.mfx.LowPower) << std::endl;
    out << detail::space(detail::INTENT, out, "BRCParamMultiplier = ")
        << p.param_.mfx.BRCParamMultiplier << std::endl;
    out << detail::space(detail::INTENT, out, "CodecId            = ")
        << detail::FourCC2String(p.param_.mfx.CodecId) << std::endl;
    out << detail::space(detail::INTENT, out, "CodecProfile       = ") << p.param_.mfx.CodecProfile
        << std::endl;
    out << detail::space(detail::INTENT, out, "CodecLevel         = ") << p.param_.mfx.CodecLevel
        << std::endl;
    out << detail::space(detail::INTENT, out, "NumThread          = ") << p.param_.mfx.NumThread
        << std::endl;
    out << "FrameInfo:" << std::endl;
    out << frame_info(p.param_.mfx.FrameInfo) << std::endl;

    return out;
}

/// @brief Holds encoder specific params.
class encoder_video_param : public codec_video_param {
public:
    /// @brief Constructs params and initialize them with default values.
    encoder_video_param() : codec_video_param() {}

    /// @brief Returns TargetUsage value.
    /// @return TargetUsage Target Usage value.
    target_usage get_TargetUsage() const {
        return (target_usage)param_.mfx.TargetUsage;
    }

    /// @brief Sets Target Usage value.
    /// @param[in] TargetUsage Target Usage.
    /// @return Reference to this object
    encoder_video_param &set_TargetUsage(target_usage TargetUsage) {
        param_.mfx.CodecId = (uint32_t)TargetUsage;
        return *this;
    }

    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, GopPicSize);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint32_t, mfx, GopRefDist);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, GopOptFlag);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, IdrInterval);

    /// @brief Returns rate control method value.
    /// @return rate control method value.
    rate_control_method get_RateControlMethod() const {
        return (rate_control_method)param_.mfx.RateControlMethod;
    }

    /// @brief Sets rate control method value.
    /// @param[in] RateControlMethod rate control method.
    /// @return Reference to this object
    encoder_video_param &set_RateControlMethod(rate_control_method RateControlMethod) {
        param_.mfx.RateControlMethod = (uint32_t)RateControlMethod;
        return *this;
    }

    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, InitialDelayInKB);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, QPI);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, Accuracy);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, BufferSizeInKB);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, TargetKbps);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, QPP);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, ICQQuality);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, MaxKbps);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, QPB);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, Convergence);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, NumSlice);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, NumRefFrame);
    DECLARE_INNER_MEMBER_ACCESS(encoder_video_param, uint16_t, mfx, EncodedOrder);

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] e Reference to the encoder_video_param instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream &operator<<(std::ostream &out, const encoder_video_param &e);
};

inline std::ostream &operator<<(std::ostream &out, const encoder_video_param &e) {
    const codec_video_param &c = dynamic_cast<const codec_video_param &>(e);
    out << c;
    out << "Encoder:" << std::endl;
    out << detail::space(detail::INTENT, out, "TargetUsage       = ")
        << detail::NotSpecifyed0(e.param_.mfx.TargetUsage) << std::endl;
    out << detail::space(detail::INTENT, out, "GopPicSize        = ")
        << detail::NotSpecifyed0(e.param_.mfx.GopPicSize) << std::endl;
    out << detail::space(detail::INTENT, out, "GopRefDist        = ")
        << detail::NotSpecifyed0(e.param_.mfx.GopRefDist) << std::endl;
    out << detail::space(detail::INTENT, out, "GopOptFlag        = ")
        << detail::GopOptFlag2String(e.param_.mfx.GopOptFlag) << std::endl;
    out << detail::space(detail::INTENT, out, "IdrInterval       = ") << e.param_.mfx.IdrInterval
        << std::endl;
    out << detail::space(detail::INTENT, out, "RateControlMethod = ")
        << detail::RateControlMethod2String(e.param_.mfx.RateControlMethod) << std::endl;
    out << detail::space(detail::INTENT, out, "InitialDelayInKB  = ")
        << e.param_.mfx.InitialDelayInKB << std::endl;
    out << detail::space(detail::INTENT, out, "QPI               = ") << e.param_.mfx.QPI
        << std::endl;
    out << detail::space(detail::INTENT, out, "Accuracy          = ") << e.param_.mfx.Accuracy
        << std::endl;
    out << detail::space(detail::INTENT, out, "BufferSizeInKB    = ") << e.param_.mfx.BufferSizeInKB
        << std::endl;
    out << detail::space(detail::INTENT, out, "TargetKbps        = ") << e.param_.mfx.TargetKbps
        << std::endl;
    out << detail::space(detail::INTENT, out, "QPP               = ") << e.param_.mfx.QPP
        << std::endl;
    out << detail::space(detail::INTENT, out, "ICQQuality        = ") << e.param_.mfx.ICQQuality
        << std::endl;
    out << detail::space(detail::INTENT, out, "MaxKbps           = ") << e.param_.mfx.MaxKbps
        << std::endl;
    out << detail::space(detail::INTENT, out, "QPB               = ") << e.param_.mfx.QPB
        << std::endl;
    out << detail::space(detail::INTENT, out, "Convergence       = ") << e.param_.mfx.Convergence
        << std::endl;
    out << detail::space(detail::INTENT, out, "NumSlice          = ")
        << detail::NotSpecifyed0(e.param_.mfx.NumSlice) << std::endl;
    out << detail::space(detail::INTENT, out, "NumRefFrame       = ")
        << detail::NotSpecifyed0(e.param_.mfx.NumRefFrame) << std::endl;
    out << detail::space(detail::INTENT, out, "EncodedOrder      = ")
        << detail::Boolean2String(e.param_.mfx.EncodedOrder) << std::endl;

    return out;
}

/// @brief Holds decoder specific params.
class decoder_video_param : public codec_video_param {
public:
    /// @brief Constructs params and initialize them with default values.
    decoder_video_param() : codec_video_param() {}
    explicit decoder_video_param(const codec_video_param &other) : codec_video_param(other) {}

    DECLARE_INNER_MEMBER_ACCESS(decoder_video_param, uint16_t, mfx, DecodedOrder);
    DECLARE_INNER_MEMBER_ACCESS(decoder_video_param, uint16_t, mfx, ExtendedPicStruct);
    DECLARE_INNER_MEMBER_ACCESS(decoder_video_param, uint32_t, mfx, TimeStampCalc);
    DECLARE_INNER_MEMBER_ACCESS(decoder_video_param, uint16_t, mfx, SliceGroupsPresent);
    DECLARE_INNER_MEMBER_ACCESS(decoder_video_param, uint16_t, mfx, MaxDecFrameBuffering);
    DECLARE_INNER_MEMBER_ACCESS(decoder_video_param, uint16_t, mfx, EnableReallocRequest);

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] d Reference to the decoder_video_param instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream &operator<<(std::ostream &out, const decoder_video_param &d);
};

inline std::ostream &operator<<(std::ostream &out, const decoder_video_param &d) {
    const codec_video_param &c = dynamic_cast<const codec_video_param &>(d);
    out << c;
    out << "Decoder:" << std::endl;
    out << detail::space(detail::INTENT, out, "DecodedOrder         = ")
        << detail::Boolean2String(d.param_.mfx.DecodedOrder) << std::endl;

    out << detail::space(detail::INTENT, out, "ExtendedPicStruct    = ")
        << detail::PicStruct2String(d.param_.mfx.ExtendedPicStruct) << std::endl;

    out << detail::space(detail::INTENT, out, "TimeStampCalc        = ")
        << detail::TimeStampCalc2String(d.param_.mfx.TimeStampCalc) << std::endl;

    out << detail::space(detail::INTENT, out, "SliceGroupsPresent   = ")
        << detail::Boolean2String(d.param_.mfx.SliceGroupsPresent) << std::endl;

    out << detail::space(detail::INTENT, out, "MaxDecFrameBuffering = ")
        << detail::NotSpecifyed0(d.param_.mfx.MaxDecFrameBuffering) << std::endl;

    out << detail::space(detail::INTENT, out, "EnableReallocRequest = ")
        << detail::TriState2String(d.param_.mfx.EnableReallocRequest) << std::endl;

    return out;
}

/// @brief Holds JPEG decoder specific params.
class jpeg_decoder_video_param : public codec_video_param {
public:
    /// @brief Constructs params and initialize them with default values.
    jpeg_decoder_video_param() : codec_video_param() {}

    DECLARE_INNER_MEMBER_ACCESS(jpeg_decoder_video_param, uint16_t, mfx, JPEGChromaFormat);
    DECLARE_INNER_MEMBER_ACCESS(jpeg_decoder_video_param, uint16_t, mfx, Rotation);
    DECLARE_INNER_MEMBER_ACCESS(jpeg_decoder_video_param, uint32_t, mfx, JPEGColorFormat);
    DECLARE_INNER_MEMBER_ACCESS(jpeg_decoder_video_param, uint16_t, mfx, InterleavedDec);
    DECLARE_INNER_MEMBER_ARRAY_ACCESS(jpeg_decoder_video_param, uint8_t, 4, mfx, SamplingFactorH);
    DECLARE_INNER_MEMBER_ARRAY_ACCESS(jpeg_decoder_video_param, uint8_t, 4, mfx, SamplingFactorV);
};

/// @brief Holds JPEG encoder specific params.
class jpeg_encoder_video_param : public codec_video_param {
public:
    /// @brief Constructs params and initialize them with default values.
    jpeg_encoder_video_param() : codec_video_param() {}

    DECLARE_INNER_MEMBER_ACCESS(jpeg_encoder_video_param, uint16_t, mfx, Interleaved);
    DECLARE_INNER_MEMBER_ACCESS(jpeg_encoder_video_param, uint16_t, mfx, Quality);
    DECLARE_INNER_MEMBER_ACCESS(jpeg_encoder_video_param, uint32_t, mfx, RestartInterval);
};

/// @brief Holds VPP specific params.
class vpp_video_param : public video_param {
public:
    /// @brief Constructs params and initialize them with default values.
    vpp_video_param() : video_param() {}

public:
    /// @brief Returns frame_info in value.
    /// @return frame info in value.
    frame_info get_in_frame_info() const {
        return frame_info(param_.vpp.In);
    }

    /// @brief Sets name value.
    /// @param[in] name Value.
    /// @return Reference to this object
    vpp_video_param &set_in_frame_info(frame_info name) {
        param_.vpp.In = name();
        return *this;
    }

    /// @brief Returns frame_info out value.
    /// @return frame info out value.
    frame_info get_out_frame_info() const {
        return frame_info(param_.vpp.Out);
    }

    /// @brief Sets name value.
    /// @param[in] name Value.
    /// @return Reference to this object
    vpp_video_param &set_out_frame_info(frame_info name) {
        param_.vpp.Out = name();
        return *this;
    }

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] v Reference to the vpp_video_param instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream &operator<<(std::ostream &out, const vpp_video_param &v);
};

inline std::ostream &operator<<(std::ostream &out, const vpp_video_param &vpp) {
    const video_param &v = dynamic_cast<const video_param &>(vpp);
    out << v;

    out << "Input FrameInfo:" << std::endl;
    out << frame_info(vpp.param_.vpp.In) << std::endl;

    out << "Output FrameInfo:" << std::endl;
    out << frame_info(vpp.param_.vpp.Out) << std::endl;

    return out;
}

} // namespace vpl
} // namespace oneapi
