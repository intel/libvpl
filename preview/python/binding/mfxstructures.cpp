//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/mfxstructures.h"
#include "vpl_python.hpp"

// pattern follows: https://github.com/pybind/pybind11/issues/2149
#define DEF_ARRAY_PROP(name, base, field, count)      \
    def_property_readonly(name, [](py::object &obj) { \
        base &self = obj.cast<base &>();              \
        return py::array(count, self.field, obj);     \
    })

template <typename T>
struct CArrayWrapper {
    using Element = T;
    struct List {
        size_t len;
        Element *base;
        List(size_t len, Element *base) : len(len), base(base) {}
        List(const List &other) : len(other.len), base(other.base) {}
    };
    using PyClass = py::class_<List, std::shared_ptr<List>>;
    PyClass pyclass;
    CArrayWrapper(const py::module &m, const std::string &typestr) : pyclass(m, typestr.c_str()) {
        pyclass.def("__getitem__", [](List *self, int i) -> Element & {
            if (i < 0)
                i += static_cast<int>(self->len);
            if (i < 0 || i >= static_cast<int>(self->len))
                throw py::index_error();
            return self->base[i];
        });
    }
};

void init_mfxstructures(const py::module &m) {
    /* ===========================
    * extension buffers
    * ========================= */
    py::class_<mfxExtBuffer, std::shared_ptr<mfxExtBuffer>>(m, "mfxExtBuffer")
        .def(py::init<>())
        .def(py::init<>([](uint32_t id, uint32_t size) {
            return new mfxExtBuffer{ id, size };
        }))
        .def_readwrite(
            "BufferId",
            &mfxExtBuffer::BufferId,
            "Identifier of the buffer content. See the ExtendedBufferID enumerator for a complete list of extended buffers.")
        .def_readwrite("BufferSz", &mfxExtBuffer::BufferSz, "Size of the buffer.")
        .def("__eq__", [](mfxExtBuffer *self, mfxExtBuffer &other) {
            return (self->BufferId == other.BufferId) && (self->BufferSz == other.BufferSz);
        });

    py::class_<mfxI16Pair, std::shared_ptr<mfxI16Pair>>(m, "mfxI16Pair")
        .def(py::init<>())
        .def(py::init<>([](int32_t v) {
            return new mfxI16Pair{ static_cast<int16_t>(v >> 16),
                                   static_cast<int16_t>(v & 0x00ffff) };
        }))
        .def(py::init<>([](int16_t x, int16_t y) {
            return new mfxI16Pair{ x, y };
        }))
        .def(py::init<>([](std::pair<int16_t, int16_t> value) {
            return new mfxI16Pair{ value.first, value.second };
        }))
        .def_readwrite("x", &mfxI16Pair::x, "First number.")
        .def_readwrite("y", &mfxI16Pair::y, "Second number")
        .def("__eq__",
             [](mfxI16Pair *self, mfxI16Pair &other) {
                 return (self->x == other.x) && (self->y == other.y);
             })
        .def("__eq__",
             [](mfxI16Pair *self, std::pair<int16_t, int16_t> value) {
                 return (self->x == value.first) && (self->y == value.second);
             })
        .def("__eq__", [](mfxI16Pair *self, int32_t v) {
            return (self->x == (v >> 16)) && (self->y == (v & 0x00ffff));
        });

    py::class_<mfxExtCodingOption, std::shared_ptr<mfxExtCodingOption>>(m, "mfxExtCodingOption")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtCodingOption::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_CODING_OPTION.")
        .def_readwrite(
            "RateDistortionOpt",
            &mfxExtCodingOption::RateDistortionOpt,
            "Set this flag if rate distortion optimization is needed. See the CodingOptionValue enumerator for values of this option.")
        .def_readwrite("MECostType",
                       &mfxExtCodingOption::MECostType,
                       "Motion estimation cost type. This value is reserved and must be zero.")
        .def_readwrite(
            "MESearchType",
            &mfxExtCodingOption::MESearchType,
            "Motion estimation search algorithm. This value is reserved and must be zero.")
        .def_readwrite(
            "MVSearchWindow",
            &mfxExtCodingOption::MVSearchWindow,
            "Rectangular size of the search window for motion estimation. This parameter is reserved and must be (0, 0).")
        .def_readwrite("EndOfSequence", &mfxExtCodingOption::EndOfSequence, "Deprecated")
        .def_readwrite(
            "FramePicture",
            &mfxExtCodingOption::FramePicture,
            "Set this flag to encode interlaced fields as interlaced frames. This flag does not affect progressive input frames. See the CodingOptionValue enumerator for values of this option.")
        .def_readwrite(
            "CAVLC",
            &mfxExtCodingOption::CAVLC,
            "If set, CAVLC is used; if unset, CABAC is used for encoding. See the CodingOptionValue enumerator for values of this option.")
        .def_readwrite(
            "RecoveryPointSEI",
            &mfxExtCodingOption::RecoveryPointSEI,
            "Set this flag to insert the recovery point SEI message at the beginning of every intra refresh cycle.")
        .def_readwrite(
            "ViewOutput",
            &mfxExtCodingOption::ViewOutput,
            "Set this flag to instruct the MVC encoder to output each view in separate bitstream buffer.")
        .def_readwrite(
            "NalHrdConformance",
            &mfxExtCodingOption::NalHrdConformance,
            "If this option is turned ON, then AVC encoder produces an HRD conformant bitstream.")
        .def_readwrite("SingleSeiNalUnit",
                       &mfxExtCodingOption::SingleSeiNalUnit,
                       "If set, encoder puts all SEI messages in the singe NAL unit.")
        .def_readwrite(
            "VuiVclHrdParameters",
            &mfxExtCodingOption::VuiVclHrdParameters,
            "If set and VBR rate control method is used, then VCL HRD parameters are written in bitstream with values identical to the values of the NAL HRD parameters.")
        .def_readwrite(
            "RefPicListReordering",
            &mfxExtCodingOption::RefPicListReordering,
            "Set this flag to activate reference picture list reordering. This value is reserved and must be zero.")
        .def_readwrite(
            "ResetRefList",
            &mfxExtCodingOption::ResetRefList,
            "Set this flag to reset the reference list to non-IDR I-frames of a GOP sequence.")
        .def_readwrite(
            "RefPicMarkRep",
            &mfxExtCodingOption::RefPicMarkRep,
            "Set this flag to write the reference picture marking repetition SEI message into the output bitstream.")
        .def_readwrite(
            "FieldOutput",
            &mfxExtCodingOption::FieldOutput,
            "Set this flag to instruct the AVC encoder to output bitstreams immediately after the encoder encodes a field, in the field-encoding mode.")
        .def_readwrite(
            "IntraPredBlockSize",
            &mfxExtCodingOption::IntraPredBlockSize,
            "Minimum block size of intra-prediction. This value is reserved and must be zero.")
        .def_readwrite(
            "InterPredBlockSize",
            &mfxExtCodingOption::InterPredBlockSize,
            "Minimum block size of inter-prediction. This value is reserved and must be zero.")
        .def_readwrite(
            "MVPrecision",
            &mfxExtCodingOption::MVPrecision,
            "Specify the motion estimation precision. This parameter is reserved and must be zero.")
        .def_readwrite(
            "MaxDecFrameBuffering",
            &mfxExtCodingOption::MaxDecFrameBuffering,
            "Specifies the maximum number of frames buffered in a DPB. A value of zero means unspecified.")
        .def_readwrite("AUDelimiter",
                       &mfxExtCodingOption::AUDelimiter,
                       "Set this flag to insert the Access Unit Delimiter NAL.")
        .def_readwrite("EndOfStream", &mfxExtCodingOption::EndOfStream, "Deprecated")
        .def_readwrite(
            "PicTimingSEI",
            &mfxExtCodingOption::PicTimingSEI,
            "Set this flag to insert the picture timing SEI with pic_struct syntax element.")
        .def_readwrite("VuiNalHrdParameters",
                       &mfxExtCodingOption::VuiNalHrdParameters,
                       "Set this flag to insert NAL HRD parameters in the VUI header.");

    py::class_<mfxExtCodingOption2, std::shared_ptr<mfxExtCodingOption2>>(m, "mfxExtCodingOption2")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtCodingOption2::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_CODING_OPTION2.")
        .def_readwrite("IntRefType",
                       &mfxExtCodingOption2::IntRefType,
                       "Specifies intra refresh type.")
        .def_readwrite("IntRefCycleSize",
                       &mfxExtCodingOption2::IntRefCycleSize,
                       "Specifies number of pictures within refresh cycle starting from 2.")
        .def_readwrite("IntRefQPDelta",
                       &mfxExtCodingOption2::IntRefQPDelta,
                       "Specifies QP difference for inserted intra MBs.")
        .def_readwrite("MaxFrameSize",
                       &mfxExtCodingOption2::MaxFrameSize,
                       "Specify maximum encoded frame size in byte.")
        .def_readwrite("MaxSliceSize",
                       &mfxExtCodingOption2::MaxSliceSize,
                       "Specify maximum slice size in bytes.")
        .def_readwrite("BitrateLimit",
                       &mfxExtCodingOption2::BitrateLimit,
                       "Modifies bitrate to be in the range imposed by the encoder.")
        .def_readwrite(
            "MBBRC",
            &mfxExtCodingOption2::MBBRC,
            "Setting this flag enables macroblock level bitrate control that generally improves subjective visual quality.")
        .def_readwrite("ExtBRC",
                       &mfxExtCodingOption2::ExtBRC,
                       "Set this option to ON to enable external BRC.")
        .def_readwrite("LookAheadDepth",
                       &mfxExtCodingOption2::LookAheadDepth,
                       "Specifies the depth of the look ahead rate control algorithm.")
        .def_readwrite("Trellis",
                       &mfxExtCodingOption2::Trellis,
                       "Used to control trellis quantization in AVC encoder.")
        .def_readwrite("RepeatPPS",
                       &mfxExtCodingOption2::RepeatPPS,
                       "Controls picture parameter set repetition in AVC encoder.")
        .def_readwrite("BRefType",
                       &mfxExtCodingOption2::BRefType,
                       "Controls usage of B-frames as reference.")
        .def_readwrite("AdaptiveI",
                       &mfxExtCodingOption2::AdaptiveI,
                       "Controls insertion of I-frames by the encoder.")
        .def_readwrite("AdaptiveB",
                       &mfxExtCodingOption2::AdaptiveB,
                       "Controls changing of frame type from B to P.")
        .def_readwrite("LookAheadDS",
                       &mfxExtCodingOption2::LookAheadDS,
                       "Controls down sampling in look ahead bitrate control mode.")
        .def_readwrite("NumMbPerSlice",
                       &mfxExtCodingOption2::NumMbPerSlice,
                       "Specifies suggested slice size in number of macroblocks.")
        .def_readwrite("SkipFrame",
                       &mfxExtCodingOption2::SkipFrame,
                       "Enables usage of mfxEncodeCtrl::SkipFrameparameter.")
        .def_readwrite(
            "MinQPI",
            &mfxExtCodingOption2::MinQPI,
            "Minimum allowed QP value for I-frame types. Valid range is 1 to 51 inclusive. Zero means default value, that is, no limitations on QP.")
        .def_readwrite(
            "MaxQPI",
            &mfxExtCodingOption2::MaxQPI,
            "Maximum allowed QP value for I-frame types. Valid range is 1 to 51 inclusive. Zero means default value, that is, no limitations on QP.")
        .def_readwrite(
            "MinQPP",
            &mfxExtCodingOption2::MinQPP,
            "Minimum allowed QP value for P-frame types. Valid range is 1 to 51 inclusive. Zero means default value, that is, no limitations on QP.")
        .def_readwrite(
            "MaxQPP",
            &mfxExtCodingOption2::MaxQPP,
            "Maximum allowed QP value for P-frame types. Valid range is 1 to 51 inclusive. Zero means default value, that is, no limitations on QP.")
        .def_readwrite(
            "MinQPB",
            &mfxExtCodingOption2::MinQPB,
            "Minimum allowed QP value for B-frame types. Valid range is 1 to 51 inclusive. Zero means default value, that is, no limitations on QP.")
        .def_readwrite(
            "MaxQPB",
            &mfxExtCodingOption2::MaxQPB,
            "Maximum allowed QP value for B-frame types. Valid range is 1 to 51 inclusive. Zero means default value, that is, no limitations on QP.")
        .def_readwrite("FixedFrameRate",
                       &mfxExtCodingOption2::FixedFrameRate,
                       "Sets fixed_frame_rate_flag in VUI.")
        .def_readwrite("DisableDeblockingIdc",
                       &mfxExtCodingOption2::DisableDeblockingIdc,
                       "Disables deblocking.")
        .def_readwrite("DisableVUI",
                       &mfxExtCodingOption2::DisableVUI,
                       "Completely disables VUI in the output bitstream.")
        .def_readwrite("BufferingPeriodSEI",
                       &mfxExtCodingOption2::BufferingPeriodSEI,
                       "Controls insertion of buffering period SEI in the encoded bitstream.")
        .def_readwrite(
            "EnableMAD",
            &mfxExtCodingOption2::EnableMAD,
            "Set this flag to ON to enable per-frame reporting of Mean Absolute Difference. This parameter is valid only during initialization.")
        .def_readwrite(
            "UseRawRef",
            &mfxExtCodingOption2::UseRawRef,
            "Set this flag to ON to use raw frames for reference instead of reconstructed frames.");

    py::class_<mfxExtCodingOption3, std::shared_ptr<mfxExtCodingOption3>>(m, "mfxExtCodingOption3")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtCodingOption3::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_CODING_OPTION3.")
        .def_readwrite("NumSliceI",
                       &mfxExtCodingOption3::NumSliceI,
                       "The number of slices for I-frames.")
        .def_readwrite("NumSliceP",
                       &mfxExtCodingOption3::NumSliceP,
                       "The number of slices for P-frames.")
        .def_readwrite("NumSliceB",
                       &mfxExtCodingOption3::NumSliceB,
                       "The number of slices for B-frames.")
        .def_readwrite(
            "WinBRCMaxAvgKbps",
            &mfxExtCodingOption3::WinBRCMaxAvgKbps,
            "When rate control method is MFX_RATECONTROL_VBR, MFX_RATECONTROL_LA, MFX_RATECONTROL_LA_HRD, or MFX_RATECONTROL_QVBR this parameter specifies the maximum bitrate averaged over a sliding window specified by WinBRCSize.")
        .def_readwrite(
            "WinBRCSize",
            &mfxExtCodingOption3::WinBRCSize,
            "When rate control method is MFX_RATECONTROL_CBR, MFX_RATECONTROL_VBR, MFX_RATECONTROL_LA, MFX_RATECONTROL_LA_HRD, or MFX_RATECONTROL_QVBR this parameter specifies sliding window size in frames. Set this parameter to zero to disable sliding window.")
        .def_readwrite(
            "QVBRQuality",
            &mfxExtCodingOption3::QVBRQuality,
            "When rate control method is MFX_RATECONTROL_QVBR, this parameter specifies quality factor. Values are in the 1 to 51 range, where 1 corresponds to the best quality.")
        .def_readwrite(
            "EnableMBQP",
            &mfxExtCodingOption3::EnableMBQP,
            "Set this flag to ON to enable per-macroblock QP control. Rate control method must be MFX_RATECONTROL_CQP. See the CodingOptionValue enumerator for values of this option. This parameter is valid only during initialization.")
        .def_readwrite(
            "IntRefCycleDist",
            &mfxExtCodingOption3::IntRefCycleDist,
            "Distance between the beginnings of the intra-refresh cycles in frames. Zero means no distance between cycles.")
        .def_readwrite(
            "DirectBiasAdjustment",
            &mfxExtCodingOption3::DirectBiasAdjustment,
            "Set this flag to ON to enable the ENC mode decision algorithm to bias to fewer B Direct/Skip types. Applies only to B-frames, all other frames will ignore this setting.")
        .def_readwrite("GlobalMotionBiasAdjustment",
                       &mfxExtCodingOption3::GlobalMotionBiasAdjustment,
                       "Enables global motion bias.")
        .def_readwrite("MVCostScalingFactor",
                       &mfxExtCodingOption3::MVCostScalingFactor,
                       "Scaling Factor. Values are 0, 1, 2, and 3.")
        .def_readwrite("MBDisableSkipMap",
                       &mfxExtCodingOption3::MBDisableSkipMap,
                       "Set this flag to ON to enable usage of mfxExtMBDisableSkipMap.")
        .def_readwrite("WeightedPred",
                       &mfxExtCodingOption3::WeightedPred,
                       "Weighted prediction mode.")
        .def_readwrite("WeightedBiPred",
                       &mfxExtCodingOption3::WeightedBiPred,
                       "Weighted prediction mode.")
        .def_readwrite(
            "AspectRatioInfoPresent",
            &mfxExtCodingOption3::AspectRatioInfoPresent,
            "Instructs encoder whether aspect ratio info should present in VUI parameters.")
        .def_readwrite("OverscanInfoPresent",
                       &mfxExtCodingOption3::OverscanInfoPresent,
                       "Instructs encoder whether overscan info should present in VUI parameters.")
        .def_readwrite(
            "OverscanAppropriate",
            &mfxExtCodingOption3::OverscanAppropriate,
            " ON indicates that the cropped decoded pictures output are suitable for display using overscan.")
        .def_readwrite(
            "TimingInfoPresent",
            &mfxExtCodingOption3::TimingInfoPresent,
            "Instructs encoder whether frame rate info should present in VUI parameters.")
        .def_readwrite(
            "BitstreamRestriction",
            &mfxExtCodingOption3::BitstreamRestriction,
            "Instructs encoder whether bitstream restriction info should present in VUI parameters.")
        .def_readwrite("LowDelayHrd",
                       &mfxExtCodingOption3::LowDelayHrd,
                       "Corresponds to AVC syntax element low_delay_hrd_flag (VUI).")
        .def_readwrite(
            "MotionVectorsOverPicBoundaries",
            &mfxExtCodingOption3::MotionVectorsOverPicBoundaries,
            "When set to OFF, no sample outside the picture boundaries and no sample at a fractional sample position for which the sample value is derived using one or more samples outside the picture boundaries is used for inter prediction of any sample.")
        .def_readwrite(
            "ScenarioInfo",
            &mfxExtCodingOption3::ScenarioInfo,
            "Provides a hint to encoder about the scenario for the encoding session. See the ScenarioInfo enumerator for values of this option.")
        .def_readwrite(
            "ContentInfo",
            &mfxExtCodingOption3::ContentInfo,
            "Provides a hint to encoder about the content for the encoding session. See the ContentInfo enumerator for values of this option.")
        .def_readwrite(
            "PRefType",
            &mfxExtCodingOption3::PRefType,
            "When GopRefDist=1, specifies the model of reference list construction and DPB management. See the PRefType enumerator for values of this option.")
        .def_readwrite(
            "FadeDetection",
            &mfxExtCodingOption3::FadeDetection,
            "Instructs encoder whether internal fade detection algorithm should be used for calculation of weigh/offset values for pred_weight_table unless application provided mfxExtPredWeightTable for this frame.")
        .def_readwrite(
            "GPB",
            &mfxExtCodingOption3::GPB,
            "Set this flag to OFF to make HEVC encoder use regular P-frames instead of GPB.")
        .def_readwrite("MaxFrameSizeI",
                       &mfxExtCodingOption3::MaxFrameSizeI,
                       "Same as mfxExtCodingOption2::MaxFrameSize but affects only I-frames.")
        .def_readwrite("MaxFrameSizeP",
                       &mfxExtCodingOption3::MaxFrameSizeP,
                       "Same as mfxExtCodingOption2::MaxFrameSize but affects only P/B-frames.")
        .def_readwrite("EnableQPOffset",
                       &mfxExtCodingOption3::EnableQPOffset,
                       " Enables QPOffset control.")
        .DEF_ARRAY_PROP("QPOffset", mfxExtCodingOption3, QPOffset, 8)
        .DEF_ARRAY_PROP("NumRefActiveP", mfxExtCodingOption3, NumRefActiveP, 8)
        .DEF_ARRAY_PROP("NumRefActiveBL0", mfxExtCodingOption3, NumRefActiveBL0, 8)
        .DEF_ARRAY_PROP("NumRefActiveBL1", mfxExtCodingOption3, NumRefActiveBL1, 8)
        .def_readwrite(
            "TransformSkip",
            &mfxExtCodingOption3::TransformSkip,
            "For HEVC if this option is turned ON, the transform_skip_enabled_flag will be set to 1 in PPS.")
        .def_readwrite(
            "TargetChromaFormatPlus1",
            &mfxExtCodingOption3::TargetChromaFormatPlus1,
            "Minus 1 specifies target encoding chroma format (see ChromaFormatIdc enumerator).")
        .def_readwrite("TargetBitDepthLuma",
                       &mfxExtCodingOption3::TargetBitDepthLuma,
                       "Target encoding bit-depth for luma samples.")
        .def_readwrite("TargetBitDepthChroma",
                       &mfxExtCodingOption3::TargetBitDepthChroma,
                       "Target encoding bit-depth for chroma samples.")
        .def_readwrite("BRCPanicMode",
                       &mfxExtCodingOption3::BRCPanicMode,
                       "Controls panic mode in AVC and MPEG2 encoders.")
        .def_readwrite(
            "LowDelayBRC",
            &mfxExtCodingOption3::LowDelayBRC,
            "When rate control method is MFX_RATECONTROL_VBR, MFX_RATECONTROL_QVBR or MFX_RATECONTROL_VCM this parameter specifies frame size tolerance.")
        .def_readwrite("EnableMBForceIntra",
                       &mfxExtCodingOption3::EnableMBForceIntra,
                       "Set this flag to ON to enable usage of mfxExtMBForceIntra for AVC encoder.")
        .def_readwrite(
            "AdaptiveMaxFrameSize",
            &mfxExtCodingOption3::AdaptiveMaxFrameSize,
            "If this flag is set to ON, BRC may decide a larger P- or B-frame size than what MaxFrameSizeP dictates when the scene change is detected.")
        .def_readwrite(
            "RepartitionCheckEnable",
            &mfxExtCodingOption3::RepartitionCheckEnable,
            "Controls AVC encoder attempts to predict from small partitions. Default value allows encoder to choose preferred mode.")
        .def_readwrite(
            "EncodedUnitsInfo",
            &mfxExtCodingOption3::EncodedUnitsInfo,
            "Set this flag to ON to make encoded units info available in mfxExtEncodedUnitsInfo.")
        .def_readwrite(
            "EnableNalUnitType",
            &mfxExtCodingOption3::EnableNalUnitType,
            "If this flag is set to ON, the HEVC encoder uses the NAL unit type provided by the application in the mfxEncodeCtrl::MfxNalUnitType field.")
        .def_readwrite("ExtBrcAdaptiveLTR", &mfxExtCodingOption3::ExtBrcAdaptiveLTR, "Deprecated")
        .def_readwrite(
            "AdaptiveCQM",
            &mfxExtCodingOption3::AdaptiveCQM,
            "If this flag is set to ON, encoder adaptively selects one of implementation-defined quantization matrices for each frame.");

    py::class_<mfxExtVPPDoNotUse, std::shared_ptr<mfxExtVPPDoNotUse>>(m, "mfxExtVPPDoNotUse")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPDoNotUse::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_DONOTUSE.")
        .def_readwrite("NumAlg",
                       &mfxExtVPPDoNotUse::NumAlg,
                       "Number of filters (algorithms) not to use")
        .def_property_readonly(
            "AlgList",
            [](mfxExtVPPDoNotUse *self) {
                return py::array_t<uint32_t>({ self->NumAlg }, { sizeof(uint32_t) }, self->AlgList);
            },
            "Pointer to a list of filters (algorithms) not to use");

    py::class_<mfxExtVPPDenoise, std::shared_ptr<mfxExtVPPDenoise>>(m, "mfxExtVPPDenoise")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPDenoise::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_DENOISE.")
        .def_readwrite(
            "DenoiseFactor",
            &mfxExtVPPDenoise::DenoiseFactor,
            "Indicates the level of noise to remove. Value range of 0 to 100 (inclusive).");

    py::class_<mfxExtVPPDenoise2, std::shared_ptr<mfxExtVPPDenoise2>>(m, "mfxExtVPPDenoise2")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPDenoise2::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_DENOISE2.")
        .def_readwrite("Mode",
                       &mfxExtVPPDenoise2::Mode,
                       "Indicates the mode of denoise. mfxDenoiseMode enumerator.")
        .def_readwrite("Strength",
                       &mfxExtVPPDenoise2::Strength,
                       "Denoise strength in manual mode.");

    py::class_<mfxExtVPPDetail, std::shared_ptr<mfxExtVPPDetail>>(m, "mfxExtVPPDetail")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPDetail::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_DETAIL.")
        .def_readwrite(
            "DetailFactor",
            &mfxExtVPPDetail::DetailFactor,
            "Indicates the level of details to be enhanced. Value range of 0 to 100 (inclusive).");

    py::class_<mfxExtVPPProcAmp, std::shared_ptr<mfxExtVPPProcAmp>>(m, "mfxExtVPPProcAmp")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPProcAmp::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_PROCAMP.")
        .def_readwrite(
            "Brightness",
            &mfxExtVPPProcAmp::Brightness,
            "The brightness parameter is in the range of -100.0F to 100.0F, in increments of 0.1F.")
        .def_readwrite(
            "Contrast",
            &mfxExtVPPProcAmp::Contrast,
            "The contrast parameter in the range of 0.0F to 10.0F, in increments of 0.01F, is used for manual contrast adjustment.")
        .def_readwrite("Hue",
                       &mfxExtVPPProcAmp::Hue,
                       "The hue parameter is in the range of -180F to 180F, in increments of 0.1F.")
        .def_readwrite(
            "Saturation",
            &mfxExtVPPProcAmp::Saturation,
            "The saturation parameter is in the range of 0.0F to 10.0F, in increments of 0.01F.");

    py::class_<mfxExtVppAuxData, std::shared_ptr<mfxExtVppAuxData>>(m, "mfxExtVppAuxData")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVppAuxData::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_AUXDATA.")
        .def_readwrite("SpatialComplexity", &mfxExtVppAuxData::SpatialComplexity, "Deprecated")
        .def_readwrite("TemporalComplexity", &mfxExtVppAuxData::TemporalComplexity, "Deprecated")
        .def_readwrite(
            "PicStruct",
            &mfxExtVppAuxData::PicStruct,
            "Detected picture structure - top field first, bottom field first, progressive or unknown if video processor cannot detect picture structure.")
        .def_readwrite("SceneChangeRate", &mfxExtVppAuxData::SceneChangeRate, "Deprecated")
        .def_readwrite("RepeatedFrame", &mfxExtVppAuxData::RepeatedFrame, "Deprecated");

    py::class_<mfxExtCodingOptionSPSPPS, std::shared_ptr<mfxExtCodingOptionSPSPPS>>(
        m,
        "mfxExtCodingOptionSPSPPS")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtCodingOptionSPSPPS::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_CODING_OPTION_SPSPPS.")
        .def_readwrite("SPSBuffer",
                       &mfxExtCodingOptionSPSPPS::SPSBuffer,
                       "Pointer to a valid bitstream that contains the SPS buffer.")
        .def_readwrite("PPSBuffer",
                       &mfxExtCodingOptionSPSPPS::PPSBuffer,
                       "Pointer to a valid bitstream that contains the PPS buffer.")
        .def_readwrite("SPSBufSize",
                       &mfxExtCodingOptionSPSPPS::SPSBufSize,
                       "Size of the SPS in bytes.")
        .def_readwrite("PPSBufSize",
                       &mfxExtCodingOptionSPSPPS::PPSBufSize,
                       "Size of the PPS in bytes.")
        .def_readwrite("SPSId",
                       &mfxExtCodingOptionSPSPPS::SPSId,
                       "SPS identifier. The value is reserved and must be zero.")
        .def_readwrite("PPSId",
                       &mfxExtCodingOptionSPSPPS::PPSId,
                       "PPS identifier. The value is reserved and must be zero.");

    py::class_<mfxExtCodingOptionVPS, std::shared_ptr<mfxExtCodingOptionVPS>>(
        m,
        "mfxExtCodingOptionVPS")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtCodingOptionVPS::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_CODING_OPTION_VPS.")
        .def_readwrite("VPSBuffer",
                       &mfxExtCodingOptionVPS::VPSBuffer,
                       "Pointer to a valid bitstream that contains the VPS buffer.")
        .def_readwrite("VPSBufSize",
                       &mfxExtCodingOptionVPS::VPSBufSize,
                       "Size of the VPS in bytes.")
        .def_readwrite("VPSId",
                       &mfxExtCodingOptionVPS::VPSId,
                       "VPS identifier; the value is reserved and must be zero.");

    py::class_<mfxExtVPPComposite, std::shared_ptr<mfxExtVPPComposite>>(m, "mfxExtVPPComposite")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPComposite::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_COMPOSITE.")
        .def_readwrite("Y", &mfxExtVPPComposite::Y, " Y value of the background color.")
        .def_readwrite("R", &mfxExtVPPComposite::R, " R value of the background color.")
        .def_readwrite("U", &mfxExtVPPComposite::U, " U value of the background color.")
        .def_readwrite("G", &mfxExtVPPComposite::G, " G value of the background color.")
        .def_readwrite("V", &mfxExtVPPComposite::V, " V value of the background color.")
        .def_readwrite("B", &mfxExtVPPComposite::B, " B value of the background color.")
        .def_readwrite(
            "NumTiles",
            &mfxExtVPPComposite::NumTiles,
            "Number of input surface clusters grouped together to enable fast compositing.")
        .def_readwrite("NumInputStream",
                       &mfxExtVPPComposite::NumInputStream,
                       "Number of input surfaces to compose one output.")
        // .def_readwrite("InputStream", &mfxExtVPPComposite::InputStream)
        ;

    py::class_<mfxExtVideoSignalInfo, std::shared_ptr<mfxExtVideoSignalInfo>>(
        m,
        "mfxExtVideoSignalInfo")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVideoSignalInfo::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VIDEO_SIGNAL_INFO.")
        .def_readwrite("VideoFormat", &mfxExtVideoSignalInfo::VideoFormat)
        .def_readwrite("VideoFullRange", &mfxExtVideoSignalInfo::VideoFullRange)
        .def_readwrite("ColourDescriptionPresent", &mfxExtVideoSignalInfo::ColourDescriptionPresent)
        .def_readwrite("ColourPrimaries", &mfxExtVideoSignalInfo::ColourPrimaries)
        .def_readwrite("TransferCharacteristics", &mfxExtVideoSignalInfo::TransferCharacteristics)
        .def_readwrite("MatrixCoefficients", &mfxExtVideoSignalInfo::MatrixCoefficients);

    py::class_<mfxExtVPPDoUse, std::shared_ptr<mfxExtVPPDoUse>>(m, "mfxExtVPPDoUse")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPDoUse::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_DOUSE.")
        .def_readwrite("NumAlg", &mfxExtVPPDoUse::NumAlg, "Number of filters (algorithms) to use")
        .def_property_readonly(
            "AlgList",
            [](mfxExtVPPDoUse *self) {
                return py::array_t<uint32_t>({ self->NumAlg }, { sizeof(uint32_t) }, self->AlgList);
            },
            "Pointer to a list of filters (algorithms) to use");

    using mfxExtAVCRefListCtrl_ref =
        std::remove_reference<decltype(mfxExtAVCRefListCtrl::PreferredRefList[0])>::type;
    py::class_<mfxExtAVCRefListCtrl_ref, std::shared_ptr<mfxExtAVCRefListCtrl_ref>>(
        m,
        "mfxExtAVCRefListCtrl_ref")
        .def(py::init<>())
        .def_readwrite(
            "FrameOrder",
            &mfxExtAVCRefListCtrl_ref::FrameOrder,
            "Together FrameOrder and PicStruct fields are used to identify reference picture.")
        .def_readwrite(
            "PicStruct",
            &mfxExtAVCRefListCtrl_ref::PicStruct,
            "Together FrameOrder and PicStruct fields are used to identify reference picture.")
        .def_readwrite("ViewId", &mfxExtAVCRefListCtrl_ref::ViewId, "Reserved and must be zero.")
        .def_readwrite(
            "LongTermIdx",
            &mfxExtAVCRefListCtrl_ref::LongTermIdx,
            "Index that should be used by the encoder to mark long-term reference frame.");

    CArrayWrapper<mfxExtAVCRefListCtrl_ref>(m, "mfxExtAVCRefListCtrl_ref_list");
    using mfxExtAVCRefListCtrl_ref_list = CArrayWrapper<mfxExtAVCRefListCtrl_ref>::List;

    py::class_<mfxExtAVCRefListCtrl, std::shared_ptr<mfxExtAVCRefListCtrl>>(m,
                                                                            "mfxExtAVCRefListCtrl")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtAVCRefListCtrl::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_AVC_REFLIST_CTRL.")
        .def_readwrite("NumRefIdxL0Active",
                       &mfxExtAVCRefListCtrl::NumRefIdxL0Active,
                       "Specify the number of reference frames in the active reference list L0.")
        .def_readwrite("NumRefIdxL1Active",
                       &mfxExtAVCRefListCtrl::NumRefIdxL1Active,
                       "Specify the number of reference frames in the active reference list L1.")
        .def_property_readonly(
            "PreferredRefList",
            [](mfxExtAVCRefListCtrl *self) {
                return mfxExtAVCRefListCtrl_ref_list(32, self->PreferredRefList);
            },
            "Reference list that specifies the list of frames that should be used to predict the current frame.")
        .def_property_readonly(
            "RejectedRefList",
            [](mfxExtAVCRefListCtrl *self) {
                return mfxExtAVCRefListCtrl_ref_list(16, self->RejectedRefList);
            },
            "Reference list that specifies the list of frames that should not be used for prediction.")
        .def_property_readonly(
            "LongTermRefList",
            [](mfxExtAVCRefListCtrl *self) {
                return mfxExtAVCRefListCtrl_ref_list(16, self->LongTermRefList);
            },
            "Reference list that specifies the list of frames that should be marked as long-term reference frame.")
        .def_readwrite(
            "ApplyLongTermIdx",
            &mfxExtAVCRefListCtrl::ApplyLongTermIdx,
            "If it is equal to zero, the encoder assigns long-term index according to internal algorithm. If it is equal to one, the encoder uses LongTermIdx value as long-term index.");

    py::class_<mfxExtVPPFrameRateConversion, std::shared_ptr<mfxExtVPPFrameRateConversion>>(
        m,
        "mfxExtVPPFrameRateConversion")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPFrameRateConversion::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION.")
        .def_readwrite(
            "Algorithm",
            &mfxExtVPPFrameRateConversion::Algorithm,
            "See the FrcAlgm enumerator for a list of frame rate conversion algorithms.");

    py::class_<mfxExtVPPImageStab, std::shared_ptr<mfxExtVPPImageStab>>(m, "mfxExtVPPImageStab")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPImageStab::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_IMAGE_STABILIZATION.")
        .def_readwrite("Mode",
                       &mfxExtVPPImageStab::Mode,
                       "Image stabilization mode. See ImageStabMode enumerator for values.");

    py::class_<mfxExtMasteringDisplayColourVolume,
               std::shared_ptr<mfxExtMasteringDisplayColourVolume>>(
        m,
        "mfxExtMasteringDisplayColourVolume")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtMasteringDisplayColourVolume::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME.")
        .def_readwrite("InsertPayloadToggle",
                       &mfxExtMasteringDisplayColourVolume::InsertPayloadToggle,
                       "InsertHDRPayload enumerator value.")
        .DEF_ARRAY_PROP("DisplayPrimariesX",
                        mfxExtMasteringDisplayColourVolume,
                        DisplayPrimariesX,
                        3)
        .DEF_ARRAY_PROP("DisplayPrimariesY",
                        mfxExtMasteringDisplayColourVolume,
                        DisplayPrimariesY,
                        3)
        .def_readwrite("WhitePointX",
                       &mfxExtMasteringDisplayColourVolume::WhitePointX,
                       "White point X coordinate.")
        .def_readwrite("WhitePointY",
                       &mfxExtMasteringDisplayColourVolume::WhitePointY,
                       "White point Y coordinate.")
        .def_readwrite(
            "MaxDisplayMasteringLuminance",
            &mfxExtMasteringDisplayColourVolume::MaxDisplayMasteringLuminance,
            "Specify maximum luminance of the display on which the content was authored in units of 0.00001 candelas per square meter.")
        .def_readwrite(
            "MinDisplayMasteringLuminance",
            &mfxExtMasteringDisplayColourVolume::MinDisplayMasteringLuminance,
            "Specify minimum luminance of the display on which the content was authored in units of 0.00001 candelas per square meter.");

    py::class_<mfxExtContentLightLevelInfo, std::shared_ptr<mfxExtContentLightLevelInfo>>(
        m,
        "mfxExtContentLightLevelInfo")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtContentLightLevelInfo::Header,
            "Extension buffer header. Header.BufferId must be equal to EXTBUFF_CONTENT_LIGHT_LEVEL_INFO.")
        .def_readwrite("InsertPayloadToggle",
                       &mfxExtContentLightLevelInfo::InsertPayloadToggle,
                       "InsertHDRPayload enumerator value.")
        .def_readwrite("MaxContentLightLevel",
                       &mfxExtContentLightLevelInfo::MaxContentLightLevel,
                       "Maximum luminance level of the content.")
        .def_readwrite("MaxPicAverageLightLevel",
                       &mfxExtContentLightLevelInfo::MaxPicAverageLightLevel,
                       "Maximum average per-frame luminance level of the content.");

    using mfxExtPictureTimingSEI_timestamp =
        std::remove_reference<decltype(mfxExtPictureTimingSEI::TimeStamp[0])>::type;
    py::class_<mfxExtPictureTimingSEI_timestamp, std::shared_ptr<mfxExtPictureTimingSEI_timestamp>>(
        m,
        "mfxExtPictureTimingSEI_timestamp")
        .def(py::init<>())
        .def_readwrite("ClockTimestampFlag", &mfxExtPictureTimingSEI_timestamp::ClockTimestampFlag)
        .def_readwrite("CtType", &mfxExtPictureTimingSEI_timestamp::CtType)
        .def_readwrite("NuitFieldBasedFlag", &mfxExtPictureTimingSEI_timestamp::NuitFieldBasedFlag)
        .def_readwrite("CountingType", &mfxExtPictureTimingSEI_timestamp::CountingType)
        .def_readwrite("FullTimestampFlag", &mfxExtPictureTimingSEI_timestamp::FullTimestampFlag)
        .def_readwrite("DiscontinuityFlag", &mfxExtPictureTimingSEI_timestamp::DiscontinuityFlag)
        .def_readwrite("CntDroppedFlag", &mfxExtPictureTimingSEI_timestamp::CntDroppedFlag)
        .def_readwrite("NFrames", &mfxExtPictureTimingSEI_timestamp::NFrames)
        .def_readwrite("SecondsFlag", &mfxExtPictureTimingSEI_timestamp::SecondsFlag)
        .def_readwrite("MinutesFlag", &mfxExtPictureTimingSEI_timestamp::MinutesFlag)
        .def_readwrite("HoursFlag", &mfxExtPictureTimingSEI_timestamp::HoursFlag)
        .def_readwrite("SecondsValue", &mfxExtPictureTimingSEI_timestamp::SecondsValue)
        .def_readwrite("MinutesValue", &mfxExtPictureTimingSEI_timestamp::MinutesValue)
        .def_readwrite("HoursValue", &mfxExtPictureTimingSEI_timestamp::HoursValue)
        .def_readwrite("TimeOffset", &mfxExtPictureTimingSEI_timestamp::TimeOffset);

    CArrayWrapper<mfxExtPictureTimingSEI_timestamp>(m, "mfxExtPictureTimingSEI_timestamp_list");
    using mfxExtPictureTimingSEI_timestamp_list =
        CArrayWrapper<mfxExtPictureTimingSEI_timestamp>::List;

    py::class_<mfxExtPictureTimingSEI, std::shared_ptr<mfxExtPictureTimingSEI>>(
        m,
        "mfxExtPictureTimingSEI")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtPictureTimingSEI::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_PICTURE_TIMING_SEI.")
        .def_property_readonly("TimeStamp", [](mfxExtPictureTimingSEI *self) {
            return mfxExtPictureTimingSEI_timestamp_list(3, self->TimeStamp);
        });

    using mfxExtAvcTemporalLayers_layer =
        std::remove_reference<decltype(mfxExtAvcTemporalLayers::Layer[0])>::type;
    py::class_<mfxExtAvcTemporalLayers_layer, std::shared_ptr<mfxExtAvcTemporalLayers_layer>>(
        m,
        "mfxExtAvcTemporalLayers_layer")
        .def(py::init<>())
        .def_readwrite(
            "Scale",
            &mfxExtAvcTemporalLayers_layer::Scale,
            "The ratio between the frame rates of the current temporal layer and the base layer.");

    CArrayWrapper<mfxExtAvcTemporalLayers_layer>(m, "mfxExtAvcTemporalLayers_layer_list");
    using mfxExtAvcTemporalLayers_layer_list = CArrayWrapper<mfxExtAvcTemporalLayers_layer>::List;

    py::class_<mfxExtAvcTemporalLayers, std::shared_ptr<mfxExtAvcTemporalLayers>>(
        m,
        "mfxExtAvcTemporalLayers")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtAvcTemporalLayers::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_AVC_TEMPORAL_LAYERS.")
        .def_readwrite("BaseLayerPID",
                       &mfxExtAvcTemporalLayers::BaseLayerPID,
                       "The priority ID of the base layer.")
        .def_property_readonly("Layer", [](mfxExtAvcTemporalLayers *self) {
            return mfxExtAvcTemporalLayers_layer_list(8, self->Layer);
        });

    py::class_<mfxExtEncoderCapability, std::shared_ptr<mfxExtEncoderCapability>>(
        m,
        "mfxExtEncoderCapability")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtEncoderCapability::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_ENCODER_CAPABILITY.")
        .def_readwrite("MBPerSec",
                       &mfxExtEncoderCapability::MBPerSec,
                       "Specify the maximum processing rate in macro blocks per second.");

    py::class_<mfxExtEncoderResetOption, std::shared_ptr<mfxExtEncoderResetOption>>(
        m,
        "mfxExtEncoderResetOption")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtEncoderResetOption::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_ENCODER_RESET_OPTION.")
        .def_readwrite("StartNewSequence",
                       &mfxExtEncoderResetOption::StartNewSequence,
                       "Instructs encoder to start new sequence after reset.");

    using mfxExtAVCEncodedFrameInfo_UsedRefList =
        std::remove_reference<decltype(mfxExtAVCEncodedFrameInfo::UsedRefListL0[0])>::type;
    py::class_<mfxExtAVCEncodedFrameInfo_UsedRefList,
               std::shared_ptr<mfxExtAVCEncodedFrameInfo_UsedRefList>>(
        m,
        "mfxExtAVCEncodedFrameInfo_UsedRefList")
        .def(py::init<>())
        .def_readwrite("FrameOrder",
                       &mfxExtAVCEncodedFrameInfo_UsedRefList::FrameOrder,
                       "Frame order of reference picture.")
        .def_readwrite("PicStruct",
                       &mfxExtAVCEncodedFrameInfo_UsedRefList::PicStruct,
                       "Picture structure of reference picture.")
        .def_readwrite("LongTermIdx",
                       &mfxExtAVCEncodedFrameInfo_UsedRefList::LongTermIdx,
                       "Long term index of reference picture if applicable.");

    CArrayWrapper<mfxExtAVCEncodedFrameInfo_UsedRefList>(
        m,
        "mfxExtAVCEncodedFrameInfo_UsedRefList_list");
    using mfxExtAVCEncodedFrameInfo_UsedRefList_list =
        CArrayWrapper<mfxExtAVCEncodedFrameInfo_UsedRefList>::List;

    py::class_<mfxExtAVCEncodedFrameInfo, std::shared_ptr<mfxExtAVCEncodedFrameInfo>>(
        m,
        "mfxExtAVCEncodedFrameInfo")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtAVCEncodedFrameInfo::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_ENCODED_FRAME_INFO.")
        .def_readwrite("FrameOrder",
                       &mfxExtAVCEncodedFrameInfo::FrameOrder,
                       "Frame order of encoded picture.")
        .def_readwrite("PicStruct",
                       &mfxExtAVCEncodedFrameInfo::PicStruct,
                       "Picture structure of encoded picture.")
        .def_readwrite("LongTermIdx",
                       &mfxExtAVCEncodedFrameInfo::LongTermIdx,
                       "Long term index of encoded picture if applicable.")
        .def_readwrite(
            "MAD",
            &mfxExtAVCEncodedFrameInfo::MAD,
            "Mean Absolute Difference between original pixels of the frame and motion compensated (for inter macroblocks) or spatially predicted (for intra macroblocks) pixels.")
        .def_readwrite(
            "BRCPanicMode",
            &mfxExtAVCEncodedFrameInfo::BRCPanicMode,
            "Bitrate control was not able to allocate enough bits for this frame. Frame quality may be unacceptably low.")
        .def_readwrite("QP", &mfxExtAVCEncodedFrameInfo::QP, "Luma QP.")
        .def_readwrite("SecondFieldOffset",
                       &mfxExtAVCEncodedFrameInfo::SecondFieldOffset,
                       "Offset to second field.")
        .def_property_readonly(
            "UsedRefListL0",
            [](mfxExtAVCEncodedFrameInfo *self) {
                return mfxExtAVCEncodedFrameInfo_UsedRefList_list(8, self->UsedRefListL0);
            },
            "Reference list that has been used to encode picture.")
        .def_property_readonly(
            "UsedRefListL1",
            [](mfxExtAVCEncodedFrameInfo *self) {
                return mfxExtAVCEncodedFrameInfo_UsedRefList_list(8, self->UsedRefListL1);
            },
            "Reference list that has been used to encode picture.");

    using mfxExtVPPVideoSignalInfo_io = decltype(mfxExtVPPVideoSignalInfo::In);
    py::class_<mfxExtVPPVideoSignalInfo_io, std::shared_ptr<mfxExtVPPVideoSignalInfo_io>>(
        m,
        "mfxExtVPPVideoSignalInfo_io")
        .def(py::init<>())
        .def_readwrite("TransferMatrix",
                       &mfxExtVPPVideoSignalInfo_io::TransferMatrix,
                       "Transfer matrix.")
        .def_readwrite("NominalRange",
                       &mfxExtVPPVideoSignalInfo_io::NominalRange,
                       "Nominal Range.");

    py::class_<mfxExtVPPVideoSignalInfo, std::shared_ptr<mfxExtVPPVideoSignalInfo>>(
        m,
        "mfxExtVPPVideoSignalInfo")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPVideoSignalInfo::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO.")
        .def_readwrite("In", &mfxExtVPPVideoSignalInfo::In)
        .def_readwrite("Out", &mfxExtVPPVideoSignalInfo::Out)
        .def_readwrite("TransferMatrix",
                       &mfxExtVPPVideoSignalInfo::TransferMatrix,
                       "Transfer matrix.")
        .def_readwrite("NominalRange", &mfxExtVPPVideoSignalInfo::NominalRange, "Nominal Range.");

    using mfxExtEncoderROI_roi = std::remove_reference<decltype(mfxExtEncoderROI::ROI[0])>::type;
    py::class_<mfxExtEncoderROI_roi, std::shared_ptr<mfxExtEncoderROI_roi>>(m,
                                                                            "mfxExtEncoderROI_roi")
        .def(py::init<>())
        .def_readwrite("Left", &mfxExtEncoderROI_roi::Left, "Left ROI's coordinate.")
        .def_readwrite("Top", &mfxExtEncoderROI_roi::Top, "Top ROI's coordinate.")
        .def_readwrite("Right", &mfxExtEncoderROI_roi::Right, "Right ROI's coordinate.")
        .def_readwrite("Bottom", &mfxExtEncoderROI_roi::Bottom, "Bottom ROI's coordinate.")
        .def_readwrite("Priority", &mfxExtEncoderROI_roi::Priority, "Priority of ROI")
        .def_readwrite("DeltaQP", &mfxExtEncoderROI_roi::DeltaQP, "Delta QP of ROI.");

    CArrayWrapper<mfxExtEncoderROI_roi>(m, "mfxExtEncoderROI_roi_list");
    using mfxExtEncoderROI_roi_list = CArrayWrapper<mfxExtEncoderROI_roi>::List;

    py::class_<mfxExtEncoderROI, std::shared_ptr<mfxExtEncoderROI>>(m, "mfxExtEncoderROI")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtEncoderROI::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_ENCODER_ROI.")
        .def_readwrite("NumROI", &mfxExtEncoderROI::NumROI, "Number of ROI descriptions in array.")
        .def_readwrite("ROIMode", &mfxExtEncoderROI::ROIMode, "QP adjustment mode for ROIs.")
        .def_property_readonly(
            "ROI",
            [](mfxExtEncoderROI *self) {
                return mfxExtEncoderROI_roi_list(256, self->ROI);
            },
            "Array of ROIs. Different ROI may overlap each other.");

    py::class_<mfxExtVPPDeinterlacing, std::shared_ptr<mfxExtVPPDeinterlacing>>(
        m,
        "mfxExtVPPDeinterlacing")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPDeinterlacing::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_DEINTERLACING.")
        .def_readwrite("Mode", &mfxExtVPPDeinterlacing::Mode, "Deinterlacing algorithm.")
        .def_readwrite(
            "TelecinePattern",
            &mfxExtVPPDeinterlacing::TelecinePattern,
            "Specifies telecine pattern when Mode = MFX_DEINTERLACING_FIXED_TELECINE_PATTERN.")
        .def_readwrite(
            "TelecineLocation",
            &mfxExtVPPDeinterlacing::TelecineLocation,
            "Specifies position inside a sequence of 5 frames where the artifacts start when TelecinePattern = MFX_TELECINE_POSITION_PROVIDED");

    using mfxExtRefPic = std::remove_reference<decltype(mfxExtAVCRefLists::RefPicList0[0])>::type;
    py::class_<mfxExtRefPic, std::shared_ptr<mfxExtRefPic>>(m, "mfxExtRefPic")
        .def(py::init<>())
        .def_readwrite("FrameOrder",
                       &mfxExtRefPic::FrameOrder,
                       "Use FrameOrder = MFX_FRAMEORDER_UNKNOWN to mark unused entry.")
        .def_readwrite(
            "PicStruct",
            &mfxExtRefPic::PicStruct,
            "Use PicStruct = MFX_PICSTRUCT_FIELD_TFF for TOP field, PicStruct = MFX_PICSTRUCT_FIELD_BFF for BOTTOM field.");

    CArrayWrapper<mfxExtRefPic>(m, "mfxExtRefPic_list");
    using mfxExtRefPic_list = CArrayWrapper<mfxExtRefPic>::List;

    py::class_<mfxExtAVCRefLists, std::shared_ptr<mfxExtAVCRefLists>>(m, "mfxExtAVCRefLists")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtAVCRefLists::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_AVC_REFLISTS.")
        .def_readwrite("NumRefIdxL0Active",
                       &mfxExtAVCRefLists::NumRefIdxL0Active,
                       "Specify the number of reference frames in the active reference list L0.")
        .def_readwrite("NumRefIdxL1Active",
                       &mfxExtAVCRefLists::NumRefIdxL1Active,
                       "Specify the number of reference frames in the active reference list L1.")
        .def_property_readonly(
            "RefPicList0",
            [](mfxExtAVCRefLists *self) {
                return mfxExtRefPic_list(32, self->RefPicList0);
            },
            "Specify L0 reference list.")
        .def_property_readonly(
            "RefPicList1",
            [](mfxExtAVCRefLists *self) {
                return mfxExtRefPic_list(32, self->RefPicList1);
            },
            "Specify L1 reference list.");

    py::class_<mfxExtVPPFieldProcessing, std::shared_ptr<mfxExtVPPFieldProcessing>>(
        m,
        "mfxExtVPPFieldProcessing")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPFieldProcessing::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_FIELD_PROCESSING.")
        .def_readwrite("Mode",
                       &mfxExtVPPFieldProcessing::Mode,
                       "Specifies the mode of the field processing algorithm.")
        .def_readwrite("InField",
                       &mfxExtVPPFieldProcessing::InField,
                       "When Mode is MFX_VPP_COPY_FIELD, specifies input field.")
        .def_readwrite("OutField",
                       &mfxExtVPPFieldProcessing::OutField,
                       "When Mode is MFX_VPP_COPY_FIELD, specifies output field.");

    using mfxExtDecVideoProcessing_In = decltype(mfxExtDecVideoProcessing::In);
    py::class_<mfxExtDecVideoProcessing_In, std::shared_ptr<mfxExtDecVideoProcessing_In>>(
        m,
        "mfxExtDecVideoProcessing_In")
        .def(py::init<>())
        .def_readwrite("CropX",
                       &mfxExtDecVideoProcessing_In::CropX,
                       "X coordinate of region of interest of the output surface.")
        .def_readwrite("CropY",
                       &mfxExtDecVideoProcessing_In::CropY,
                       "Y coordinate of region of interest of the output surface.")
        .def_readwrite("CropW",
                       &mfxExtDecVideoProcessing_In::CropW,
                       "Width coordinate of region of interest of the output surface.")
        .def_readwrite("CropH",
                       &mfxExtDecVideoProcessing_In::CropH,
                       "Height coordinate of region of interest of the output surface.");

    using mfxExtDecVideoProcessing_Out = decltype(mfxExtDecVideoProcessing::Out);
    py::class_<mfxExtDecVideoProcessing_Out, std::shared_ptr<mfxExtDecVideoProcessing_Out>>(
        m,
        "mfxExtDecVideoProcessing_Out")
        .def(py::init<>())
        .def_readwrite("FourCC",
                       &mfxExtDecVideoProcessing_Out::FourCC,
                       "FourCC of output surface Note: Should be MFX_FOURCC_NV12.")
        .def_readwrite("ChromaFormat",
                       &mfxExtDecVideoProcessing_Out::ChromaFormat,
                       "Chroma Format of output surface.")
        .def_readwrite("Width", &mfxExtDecVideoProcessing_Out::Width, "Width of output surface.")
        .def_readwrite("Height", &mfxExtDecVideoProcessing_Out::Height, "Height of output surface.")
        .def_readwrite("CropX",
                       &mfxExtDecVideoProcessing_Out::CropX,
                       "X coordinate of region of interest of the output surface.")
        .def_readwrite("CropY",
                       &mfxExtDecVideoProcessing_Out::CropY,
                       "Y coordinate of region of interest of the output surface.")
        .def_readwrite("CropW",
                       &mfxExtDecVideoProcessing_Out::CropW,
                       "Width coordinate of region of interest of the output surface.")
        .def_readwrite("CropH",
                       &mfxExtDecVideoProcessing_Out::CropH,
                       "Height coordinate of region of interest of the output surface.");

    py::class_<mfxExtDecVideoProcessing, std::shared_ptr<mfxExtDecVideoProcessing>>(
        m,
        "mfxExtDecVideoProcessing")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtDecVideoProcessing::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_DEC_VIDEO_PROCESSING.")
        .def_readwrite("In", &mfxExtDecVideoProcessing::In, "Input surface description.")
        .def_readwrite("Out", &mfxExtDecVideoProcessing::Out, "Output surface description.");

    py::class_<mfxExtChromaLocInfo, std::shared_ptr<mfxExtChromaLocInfo>>(m, "mfxExtChromaLocInfo")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtChromaLocInfo::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_CHROMA_LOC_INFO.")
        .def_readwrite("ChromaLocInfoPresentFlag", &mfxExtChromaLocInfo::ChromaLocInfoPresentFlag)
        .def_readwrite("ChromaSampleLocTypeTopField",
                       &mfxExtChromaLocInfo::ChromaSampleLocTypeTopField)
        .def_readwrite("ChromaSampleLocTypeBottomField",
                       &mfxExtChromaLocInfo::ChromaSampleLocTypeBottomField);

    py::class_<mfxQPandMode, std::shared_ptr<mfxQPandMode>>(m, "mfxQPandMode")
        .def(py::init<>())
        .def_readwrite("QP", &mfxQPandMode::QP, "QP for MB or CU.")
        .def_readwrite("DeltaQP", &mfxQPandMode::DeltaQP, "Per-macroblock QP delta.")
        .def_readwrite("Mode", &mfxQPandMode::Mode, "Defines QP update mode.");

    py::class_<mfxExtMBQP, std::shared_ptr<mfxExtMBQP>>(m, "mfxExtMBQP")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtMBQP::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_MBQP.")
        .def_readwrite("Mode",
                       &mfxExtMBQP::Mode,
                       "Defines QP update mode. See MBQPMode enumerator for more details.")
        .def_readwrite("BlockSize",
                       &mfxExtMBQP::BlockSize,
                       "QP block size, valid for HEVC only during Init and Runtime.")
        .def_readwrite("NumQPAlloc",
                       &mfxExtMBQP::NumQPAlloc,
                       "Size of allocated by application QP or DeltaQP array.")
        .def_readwrite("QP",
                       &mfxExtMBQP::QP,
                       "Pointer to a list of per-macroblock QP in raster scan order.")
        .def_readwrite("DeltaQP",
                       &mfxExtMBQP::DeltaQP,
                       "Pointer to a list of per-macroblock QP deltas in raster scan order.")
        .def_readwrite("QPmode",
                       &mfxExtMBQP::QPmode,
                       "Block-granularity modes when MFX_MBQP_MODE_QP_ADAPTIVE is set.");

    py::class_<mfxExtInsertHeaders, std::shared_ptr<mfxExtInsertHeaders>>(m, "mfxExtInsertHeaders")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtInsertHeaders::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_INSERT_HEADERS.")
        .def_readwrite("SPS", &mfxExtInsertHeaders::SPS, "Tri-state option to insert SPS.")
        .def_readwrite("PPS", &mfxExtInsertHeaders::PPS, "Tri-state option to insert PPS.");

    using mfxExtEncoderIPCMArea_Area =
        std::remove_reference<decltype(mfxExtEncoderIPCMArea::Areas[0])>::type;
    py::class_<mfxExtEncoderIPCMArea_Area, std::shared_ptr<mfxExtEncoderIPCMArea_Area>>(
        m,
        "mfxExtEncoderIPCMArea_Area")
        .def(py::init<>())
        .def_readwrite("Left", &mfxExtEncoderIPCMArea_Area::Left, "Left area coordinate.")
        .def_readwrite("Top", &mfxExtEncoderIPCMArea_Area::Top, "Top area coordinate.")
        .def_readwrite("Right", &mfxExtEncoderIPCMArea_Area::Right, "Right area coordinate.")
        .def_readwrite("Bottom", &mfxExtEncoderIPCMArea_Area::Bottom, "Bottom area coordinate.");

    CArrayWrapper<mfxExtEncoderIPCMArea_Area>(m, "mfxExtEncoderIPCMArea_Area_list");
    using mfxExtEncoderIPCMArea_Area_list = CArrayWrapper<mfxExtEncoderIPCMArea_Area>::List;

    py::class_<mfxExtEncoderIPCMArea, std::shared_ptr<mfxExtEncoderIPCMArea>>(
        m,
        "mfxExtEncoderIPCMArea")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtEncoderIPCMArea::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_ENCODER_IPCM_AREA.")
        .def_readwrite("NumArea", &mfxExtEncoderIPCMArea::NumArea, "Number of areas")
        .def_property_readonly(
            "Areas",
            [](mfxExtEncoderIPCMArea *self) {
                return mfxExtEncoderIPCMArea_Area_list(0, self->Areas);
            },
            "Array of areas.");

    py::class_<mfxExtMBForceIntra, std::shared_ptr<mfxExtMBForceIntra>>(m, "mfxExtMBForceIntra")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtMBForceIntra::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_MB_FORCE_INTRA.")
        .def_readwrite("MapSize", &mfxExtMBForceIntra::MapSize, "Macroblock map size.")
        .def_readwrite("Map",
                       &mfxExtMBForceIntra::Map,
                       "Pointer to a list of force intra macroblock flags in raster scan order.");

    py::class_<mfxExtHEVCTiles, std::shared_ptr<mfxExtHEVCTiles>>(m, "mfxExtHEVCTiles")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtHEVCTiles::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_HEVC_TILES.")
        .def_readwrite("NumTileRows", &mfxExtHEVCTiles::NumTileRows, "Number of tile rows.")
        .def_readwrite("NumTileColumns",
                       &mfxExtHEVCTiles::NumTileColumns,
                       "Number of tile columns.");

    py::class_<mfxExtMBDisableSkipMap, std::shared_ptr<mfxExtMBDisableSkipMap>>(
        m,
        "mfxExtMBDisableSkipMap")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtMBDisableSkipMap::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_MB_DISABLE_SKIP_MAP.")
        .def_readwrite("MapSize", &mfxExtMBDisableSkipMap::MapSize, "Macroblock map size.")
        .def_readwrite("Map",
                       &mfxExtMBDisableSkipMap::Map,
                       "Pointer to a list of non-skip macroblock flags in raster scan order.");

    py::class_<mfxExtHEVCParam, std::shared_ptr<mfxExtHEVCParam>>(m, "mfxExtHEVCParam")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtHEVCParam::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_HEVC_PARAM.")
        .def_readwrite("PicWidthInLumaSamples",
                       &mfxExtHEVCParam::PicWidthInLumaSamples,
                       "Specifies the width of each coded picture in units of luma samples.")
        .def_readwrite("PicHeightInLumaSamples",
                       &mfxExtHEVCParam::PicHeightInLumaSamples,
                       "Specifies the height of each coded picture in units of luma samples.")
        .def_readwrite("GeneralConstraintFlags",
                       &mfxExtHEVCParam::GeneralConstraintFlags,
                       "Additional flags to specify exact profile and constraints.")
        .def_readwrite("SampleAdaptiveOffset",
                       &mfxExtHEVCParam::SampleAdaptiveOffset,
                       "Controls SampleAdaptiveOffset encoding feature.")
        .def_readwrite("LCUSize",
                       &mfxExtHEVCParam::LCUSize,
                       "Specifies largest coding unit size (max luma coding block).");

    py::class_<mfxExtDecodeErrorReport, std::shared_ptr<mfxExtDecodeErrorReport>>(
        m,
        "mfxExtDecodeErrorReport")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtDecodeErrorReport::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_DECODE_ERROR_REPORT.")
        .def_readwrite("ErrorTypes",
                       &mfxExtDecodeErrorReport::ErrorTypes,
                       "Bitstream error types (bit-ORed values).");

    py::class_<mfxExtDecodedFrameInfo, std::shared_ptr<mfxExtDecodedFrameInfo>>(
        m,
        "mfxExtDecodedFrameInfo")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtDecodedFrameInfo::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_DECODED_FRAME_INFO.")
        .def_readwrite("FrameType", &mfxExtDecodedFrameInfo::FrameType, "Frame type.");

    py::class_<mfxExtTimeCode, std::shared_ptr<mfxExtTimeCode>>(m, "mfxExtTimeCode")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtTimeCode::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_TIME_CODE.")
        .def_readwrite("DropFrameFlag", &mfxExtTimeCode::DropFrameFlag, "Indicated dropped frame.")
        .def_readwrite("TimeCodeHours", &mfxExtTimeCode::TimeCodeHours, "Hours.")
        .def_readwrite("TimeCodeMinutes", &mfxExtTimeCode::TimeCodeMinutes, "Minutes.")
        .def_readwrite("TimeCodeSeconds", &mfxExtTimeCode::TimeCodeSeconds, "Seconds.")
        .def_readwrite("TimeCodePictures", &mfxExtTimeCode::TimeCodePictures, "Pictures.");

    py::class_<mfxExtHEVCRegion, std::shared_ptr<mfxExtHEVCRegion>>(m, "mfxExtHEVCRegion")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtHEVCRegion::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_HEVC_REGION.")
        .def_readwrite("RegionId", &mfxExtHEVCRegion::RegionId, "ID of region.")
        .def_readwrite("RegionType", &mfxExtHEVCRegion::RegionType, "Type of region.")
        .def_readwrite("RegionEncoding",
                       &mfxExtHEVCRegion::RegionEncoding,
                       "Set to MFX_HEVC_REGION_ENCODING_ON to encode only specified region.");

    py::class_<mfxExtPredWeightTable, std::shared_ptr<mfxExtPredWeightTable>>(
        m,
        "mfxExtPredWeightTable")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtPredWeightTable::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_PRED_WEIGHT_TABLE.")
        .def_readwrite("LumaLog2WeightDenom",
                       &mfxExtPredWeightTable::LumaLog2WeightDenom,
                       "Base 2 logarithm of the denominator for all luma weighting factors.")
        .def_readwrite("ChromaLog2WeightDenom",
                       &mfxExtPredWeightTable::ChromaLog2WeightDenom,
                       "Base 2 logarithm of the denominator for all chroma weighting factors.")
        .def("get_LumaWeightFlag",
             [](mfxExtPredWeightTable *self, int L, int R) {
                 return self->LumaWeightFlag[L][R];
             })
        .def("set_LumaWeightFlag",
             [](mfxExtPredWeightTable *self, int L, int R, uint16_t value) {
                 self->LumaWeightFlag[L][R] = value;
             })
        .def("get_ChromaWeightFlag",
             [](mfxExtPredWeightTable *self, int L, int R) {
                 return self->ChromaWeightFlag[L][R];
             })
        .def("set_ChromaWeightFlag",
             [](mfxExtPredWeightTable *self, int L, int R, uint16_t value) {
                 self->ChromaWeightFlag[L][R] = value;
             })
        .def("get_Weight",
             [](mfxExtPredWeightTable *self, int i, int j, int k, int m) {
                 return self->Weights[i][j][k][m];
             })
        .def("set_Weight",
             [](mfxExtPredWeightTable *self, int i, int j, int k, int m, uint16_t value) {
                 self->Weights[i][j][k][m] = value;
             });

    py::class_<mfxExtAVCRoundingOffset, std::shared_ptr<mfxExtAVCRoundingOffset>>(
        m,
        "mfxExtAVCRoundingOffset")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtAVCRoundingOffset::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_AVC_ROUNDING_OFFSET.")
        .def_readwrite("EnableRoundingIntra",
                       &mfxExtAVCRoundingOffset::EnableRoundingIntra,
                       "Enable rounding offset for intra blocks.")
        .def_readwrite("RoundingOffsetIntra",
                       &mfxExtAVCRoundingOffset::RoundingOffsetIntra,
                       "Intra rounding offset.")
        .def_readwrite("EnableRoundingInter",
                       &mfxExtAVCRoundingOffset::EnableRoundingInter,
                       "Enable rounding offset for inter blocks.")
        .def_readwrite("RoundingOffsetInter",
                       &mfxExtAVCRoundingOffset::RoundingOffsetInter,
                       "Inter rounding offset.");

    using mfxExtDirtyRect_rect = std::remove_reference<decltype(mfxExtDirtyRect::Rect[0])>::type;
    py::class_<mfxExtDirtyRect_rect, std::shared_ptr<mfxExtDirtyRect_rect>>(m,
                                                                            "mfxExtDirtyRect_rect")
        .def(py::init<>())
        .def_readwrite("Left", &mfxExtDirtyRect_rect::Left, "Dirty region left coordinate.")
        .def_readwrite("Top", &mfxExtDirtyRect_rect::Top, "Dirty region top coordinate.")
        .def_readwrite("Right", &mfxExtDirtyRect_rect::Right, "Dirty region right coordinate.")
        .def_readwrite("Bottom", &mfxExtDirtyRect_rect::Bottom, "Dirty region bottom coordinate.");

    CArrayWrapper<mfxExtDirtyRect_rect>(m, "mfxExtDirtyRect_rect_list");
    using mfxExtDirtyRect_rect_list = CArrayWrapper<mfxExtDirtyRect_rect>::List;

    py::class_<mfxExtDirtyRect, std::shared_ptr<mfxExtDirtyRect>>(m, "mfxExtDirtyRect")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtDirtyRect::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_DIRTY_RECTANGLES.")
        .def_readwrite("NumRect", &mfxExtDirtyRect::NumRect, "Number of dirty rectangles.")
        .def_property_readonly(
            "Rect",
            [](mfxExtDirtyRect *self) {
                return mfxExtDirtyRect_rect_list(256, self->Rect);
            },
            "Array of dirty rectangles.");

    using mfxExtMoveRect_rect = std::remove_reference<decltype(mfxExtMoveRect::Rect[0])>::type;
    py::class_<mfxExtMoveRect_rect, std::shared_ptr<mfxExtMoveRect_rect>>(m, "mfxExtMoveRect_rect")
        .def(py::init<>())
        .def_readwrite("DestLeft",
                       &mfxExtMoveRect_rect::DestLeft,
                       "Destination rectangle location.")
        .def_readwrite("DestTop", &mfxExtMoveRect_rect::DestTop, "Destination rectangle location.")
        .def_readwrite("DestRight",
                       &mfxExtMoveRect_rect::DestRight,
                       "Destination rectangle location.")
        .def_readwrite("DestBottom",
                       &mfxExtMoveRect_rect::DestBottom,
                       "Destination rectangle location.")
        .def_readwrite("SourceLeft", &mfxExtMoveRect_rect::SourceLeft, "Source rectangle location.")
        .def_readwrite("SourceTop", &mfxExtMoveRect_rect::SourceTop, "Source rectangle location.");

    CArrayWrapper<mfxExtMoveRect_rect>(m, "mfxExtMoveRect_rect_list");
    using mfxExtMoveRect_rect_list = CArrayWrapper<mfxExtMoveRect_rect>::List;

    py::class_<mfxExtMoveRect, std::shared_ptr<mfxExtMoveRect>>(m, "mfxExtMoveRect")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtMoveRect::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_MOVING_RECTANGLE.")
        .def_readwrite("NumRect", &mfxExtMoveRect::NumRect, "Number of moving rectangles.")
        .def_property_readonly(
            "Rect",
            [](mfxExtMoveRect *self) {
                return mfxExtMoveRect_rect_list(256, self->Rect);
            },
            "Array of moving rectangles.");

    py::class_<mfxExtVPPRotation, std::shared_ptr<mfxExtVPPRotation>>(m, "mfxExtVPPRotation")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPRotation::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_ROTATION.")
        .def_readwrite("Angle", &mfxExtVPPRotation::Angle, "Rotation angle.");

    py::class_<mfxExtEncodedSlicesInfo, std::shared_ptr<mfxExtEncodedSlicesInfo>>(
        m,
        "mfxExtEncodedSlicesInfo")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtEncodedSlicesInfo::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_ENCODED_SLICES_INFO.")
        .def_readwrite(
            "SliceSizeOverflow",
            &mfxExtEncodedSlicesInfo::SliceSizeOverflow,
            "When mfxExtCodingOption2::MaxSliceSize is used, indicates the requested slice size was not met for one or more generated slices.")
        .def_readwrite(
            "NumSliceNonCopliant",
            &mfxExtEncodedSlicesInfo::NumSliceNonCopliant,
            "When mfxExtCodingOption2::MaxSliceSize is used, indicates the number of generated slices exceeds specification limits.")
        .def_readwrite("NumEncodedSlice",
                       &mfxExtEncodedSlicesInfo::NumEncodedSlice,
                       "Number of encoded slices.")
        .def_readwrite("NumSliceSizeAlloc",
                       &mfxExtEncodedSlicesInfo::NumSliceSizeAlloc,
                       "SliceSize array allocation size. Must be specified by application.")
        .def_readwrite("SliceSize",
                       &mfxExtEncodedSlicesInfo::SliceSize,
                       "Slice size in bytes. Array must be allocated by application.");

    py::class_<mfxExtVPPScaling, std::shared_ptr<mfxExtVPPScaling>>(m, "mfxExtVPPScaling")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPScaling::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_SCALING.")
        .def_readwrite("ScalingMode", &mfxExtVPPScaling::ScalingMode, "Scaling mode.")
        .def_readwrite("InterpolationMethod",
                       &mfxExtVPPScaling::InterpolationMethod,
                       "Interpolation mode for scaling algorithm.");

    py::class_<mfxExtVPPMirroring, std::shared_ptr<mfxExtVPPMirroring>>(m, "mfxExtVPPMirroring")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPMirroring::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_MIRRORING.")
        .def_readwrite("Type", &mfxExtVPPMirroring::Type, "Mirroring type.");

    py::class_<mfxExtMVOverPicBoundaries, std::shared_ptr<mfxExtMVOverPicBoundaries>>(
        m,
        "mfxExtMVOverPicBoundaries")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtMVOverPicBoundaries::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_MV_OVER_PIC_BOUNDARIES.")
        .def_readwrite(
            "StickTop",
            &mfxExtMVOverPicBoundaries::StickTop,
            "When set to OFF, one or more samples outside corresponding picture boundary may be used in inter prediction.")
        .def_readwrite(
            "StickBottom",
            &mfxExtMVOverPicBoundaries::StickBottom,
            "When set to OFF, one or more samples outside corresponding picture boundary may be used in inter prediction.")
        .def_readwrite(
            "StickLeft",
            &mfxExtMVOverPicBoundaries::StickLeft,
            "When set to OFF, one or more samples outside corresponding picture boundary may be used in inter prediction.")
        .def_readwrite(
            "StickRight",
            &mfxExtMVOverPicBoundaries::StickRight,
            "When set to OFF, one or more samples outside corresponding picture boundary may be used in inter prediction.");

    py::class_<mfxExtVPPColorFill, std::shared_ptr<mfxExtVPPColorFill>>(m, "mfxExtVPPColorFill")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVPPColorFill::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_COLORFILL.")
        .def_readwrite("Enable",
                       &mfxExtVPPColorFill::Enable,
                       "Set to ON makes VPP fill the area between Width/Height and Crop borders.");

    py::class_<mfxExtColorConversion, std::shared_ptr<mfxExtColorConversion>>(
        m,
        "mfxExtColorConversion")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtColorConversion::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_COLOR_CONVERSION.")
        .def_readwrite("ChromaSiting",
                       &mfxExtColorConversion::ChromaSiting,
                       "See ChromaSiting enumerator for details.");

    py::class_<mfxVP9SegmentParam, std::shared_ptr<mfxVP9SegmentParam>>(m, "mfxVP9SegmentParam")
        .def(py::init<>())
        .def_readwrite("FeatureEnabled",
                       &mfxVP9SegmentParam::FeatureEnabled,
                       "Indicates which features are enabled for the segment.")
        .def_readwrite("QIndexDelta",
                       &mfxVP9SegmentParam::QIndexDelta,
                       "Quantization index delta for the segment.")
        .def_readwrite("LoopFilterLevelDelta",
                       &mfxVP9SegmentParam::LoopFilterLevelDelta,
                       "Loop filter level delta for the segment.")
        .def_readwrite("ReferenceFrame",
                       &mfxVP9SegmentParam::ReferenceFrame,
                       "Reference frame for the segment.");

    CArrayWrapper<mfxVP9SegmentParam>(m, "mfxVP9SegmentParam_list");
    using mfxVP9SegmentParam_list = CArrayWrapper<mfxVP9SegmentParam>::List;

    py::class_<mfxExtVP9Segmentation, std::shared_ptr<mfxExtVP9Segmentation>>(
        m,
        "mfxExtVP9Segmentation")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVP9Segmentation::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VP9_SEGMENTATION.")
        .def_readwrite("NumSegments",
                       &mfxExtVP9Segmentation::NumSegments,
                       "Number of segments for frame.")
        .def_property_readonly(
            "Segment",
            [](mfxExtVP9Segmentation *self) {
                return mfxVP9SegmentParam_list(8, self->Segment);
            },
            "Array of mfxVP9SegmentParam structures containing features and parameters for every segment.")
        .def_readwrite("SegmentIdBlockSize",
                       &mfxExtVP9Segmentation::SegmentIdBlockSize,
                       "Size of block (NxN) for segmentation map.")
        .def_readwrite("NumSegmentIdAlloc",
                       &mfxExtVP9Segmentation::NumSegmentIdAlloc,
                       "Size of buffer allocated for segmentation map (in bytes).")
        .def_property_readonly(
            "SegmentId",
            [](mfxExtVP9Segmentation *self) {
                return nullptr;
            },
            "Pointer to the segmentation map buffer which holds the array of segment_ids in raster scan order.");

    py::class_<mfxVP9TemporalLayer, std::shared_ptr<mfxVP9TemporalLayer>>(m, "mfxVP9TemporalLayer")
        .def(py::init<>())
        .def_readwrite(
            "FrameRateScale",
            &mfxVP9TemporalLayer::FrameRateScale,
            "The ratio between the frame rates of the current temporal layer and the base layer.")
        .def_readwrite("TargetKbps",
                       &mfxVP9TemporalLayer::TargetKbps,
                       "Target bitrate for the current temporal layer.");

    CArrayWrapper<mfxVP9TemporalLayer>(m, "mfxVP9TemporalLayer_list");
    using mfxVP9TemporalLayer_list = CArrayWrapper<mfxVP9TemporalLayer>::List;

    py::class_<mfxExtVP9TemporalLayers, std::shared_ptr<mfxExtVP9TemporalLayers>>(
        m,
        "mfxExtVP9TemporalLayers")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVP9TemporalLayers::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VP9_TEMPORAL_LAYERS.")
        .def_property_readonly(
            "Layer",
            [](mfxExtVP9TemporalLayers *self) {
                return mfxVP9TemporalLayer_list(8, self->Layer);
            },
            "The array of temporal layers.");

    py::class_<mfxExtVP9Param, std::shared_ptr<mfxExtVP9Param>>(m, "mfxExtVP9Param")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVP9Param::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VP9_PARAM.")
        .def_readwrite("FrameWidth",
                       &mfxExtVP9Param::FrameWidth,
                       "Width of the coded frame in pixels.")
        .def_readwrite("FrameHeight",
                       &mfxExtVP9Param::FrameHeight,
                       "Height of the coded frame in pixels.")
        .def_readwrite(
            "WriteIVFHeaders",
            &mfxExtVP9Param::WriteIVFHeaders,
            "Set this option to ON to make the encoder insert IVF container headers to the output stream.")
        .def_readwrite("QIndexDeltaLumaDC",
                       &mfxExtVP9Param::QIndexDeltaLumaDC,
                       "Specifies an offset for a particular quantization parameter.")
        .def_readwrite("QIndexDeltaChromaAC",
                       &mfxExtVP9Param::QIndexDeltaChromaAC,
                       "Specifies an offset for a particular quantization parameter.")
        .def_readwrite("QIndexDeltaChromaDC",
                       &mfxExtVP9Param::QIndexDeltaChromaDC,
                       "Specifies an offset for a particular quantization parameter.")
        .def_readwrite("NumTileRows", &mfxExtVP9Param::NumTileRows, "Number of tile rows.")
        .def_readwrite("NumTileColumns",
                       &mfxExtVP9Param::NumTileColumns,
                       "Number of tile columns.");

    py::class_<mfxEncodedUnitInfo, std::shared_ptr<mfxEncodedUnitInfo>>(m, "mfxEncodedUnitInfo")
        .def(py::init<>())
        .def_readwrite("Type", &mfxEncodedUnitInfo::Type, "Codec-dependent coding unit type")
        .def_readwrite("Offset",
                       &mfxEncodedUnitInfo::Offset,
                       "Offset relative to the associated mfxBitstream::DataOffset.")
        .def_readwrite("Size", &mfxEncodedUnitInfo::Size, "Unit size, including delimiter.");

    py::class_<mfxExtEncodedUnitsInfo, std::shared_ptr<mfxExtEncodedUnitsInfo>>(
        m,
        "mfxExtEncodedUnitsInfo")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtEncodedUnitsInfo::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_ENCODED_UNITS_INFO.")
        .def_readwrite(
            "UnitInfo",
            &mfxExtEncodedUnitsInfo::UnitInfo,
            "Pointer to an array of mfxEncodedUnitsInfo structures whose size is equal to or greater than NumUnitsAlloc.")
        .def_readwrite("NumUnitsAlloc",
                       &mfxExtEncodedUnitsInfo::NumUnitsAlloc,
                       "UnitInfo array size.")
        .def_readwrite("NumUnitsEncoded",
                       &mfxExtEncodedUnitsInfo::NumUnitsEncoded,
                       "Output field. Number of coding units to report.");

    py::class_<mfxExtVppMctf, std::shared_ptr<mfxExtVppMctf>>(m, "mfxExtVppMctf")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtVppMctf::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_VPP_MCTF.")
        .def_readwrite(
            "FilterStrength",
            &mfxExtVppMctf::FilterStrength,
            "Value in range of 0 to 20 (inclusive) to indicate the filter strength of MCTF.");

    py::class_<mfxExtPartialBitstreamParam, std::shared_ptr<mfxExtPartialBitstreamParam>>(
        m,
        "mfxExtPartialBitstreamParam")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtPartialBitstreamParam::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_PARTIAL_BITSTREAM_PARAM.")
        .def_readwrite("BlockSize",
                       &mfxExtPartialBitstreamParam::BlockSize,
                       "Output block granularity for PartialBitstreamGranularity.")
        .def_readwrite("Granularity",
                       &mfxExtPartialBitstreamParam::Granularity,
                       "Granularity of the partial bitstream.");

    py::class_<mfxExtDeviceAffinityMask, std::shared_ptr<mfxExtDeviceAffinityMask>>(
        m,
        "mfxExtDeviceAffinityMask")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtDeviceAffinityMask::Header,
            "Extension buffer header. Header.BufferId must be equal to MFX_EXTBUFF_DEVICE_AFFINITY_MASK.")
        .def_property(
            "DeviceID",
            [](mfxExtDeviceAffinityMask *self) {
                return std::string(self->DeviceID);
            },
            [](mfxExtDeviceAffinityMask *self, std::string &value) {
                strncpy_s(self->DeviceID, MFX_STRFIELD_LEN, value.c_str(), value.size() + 1);
            },
            "Null terminated string with device ID.")
        .def_readwrite("NumSubDevices",
                       &mfxExtDeviceAffinityMask::NumSubDevices,
                       "Number of sub devices or threads in case of CPU in the mask.")
        // .def_readwrite("Mask", &mfxExtDeviceAffinityMask::Mask)
        ;

    py::class_<mfxAV1FilmGrainPoint, std::shared_ptr<mfxAV1FilmGrainPoint>>(m,
                                                                            "mfxAV1FilmGrainPoint")
        .def(py::init<>())
        .def_readwrite(
            "Value",
            &mfxAV1FilmGrainPoint::Value,
            "The x coordinate for the i-th point of the piece-wise linear scaling function for luma/Cb/Cr component.")
        .def_readwrite(
            "Scaling",
            &mfxAV1FilmGrainPoint::Scaling,
            "The scaling (output) value for the i-th point of the piecewise linear scaling function for luma/Cb/Cr component.");

    CArrayWrapper<mfxAV1FilmGrainPoint>(m, "mfxAV1FilmGrainPoint_list");
    using mfxAV1FilmGrainPoint_list = CArrayWrapper<mfxAV1FilmGrainPoint>::List;

    py::class_<mfxExtAV1FilmGrainParam, std::shared_ptr<mfxExtAV1FilmGrainParam>>(
        m,
        "mfxExtAV1FilmGrainParam")
        .def(py::init<>())
        .def_readwrite("Header", &mfxExtAV1FilmGrainParam::Header)
        .def_readwrite("FilmGrainFlags",
                       &mfxExtAV1FilmGrainParam::FilmGrainFlags,
                       "Bit map with bit-ORed flags from FilmGrainFlags enum.")
        .def_readwrite("GrainSeed",
                       &mfxExtAV1FilmGrainParam::GrainSeed,
                       "Starting value for pseudo-random numbers used during film grain synthesis.")
        .def_readwrite(
            "RefIdx",
            &mfxExtAV1FilmGrainParam::RefIdx,
            "Indicate which reference frame contains the film grain parameters to be used for this frame.")
        .def_readwrite(
            "NumYPoints",
            &mfxExtAV1FilmGrainParam::NumYPoints,
            "The number of points for the piece-wise linear scaling function of the luma component.")
        .def_readwrite(
            "NumCbPoints",
            &mfxExtAV1FilmGrainParam::NumCbPoints,
            "The number of points for the piece-wise linear scaling function of the Cb component.")
        .def_readwrite(
            "NumCrPoints",
            &mfxExtAV1FilmGrainParam::NumCrPoints,
            "The number of points for the piece-wise linear scaling function of the Cr component.")
        .def_property_readonly(
            "PointY",
            [](mfxExtAV1FilmGrainParam *self) {
                return mfxAV1FilmGrainPoint_list(14, self->PointY);
            },
            "The array of points for luma component.")
        .def_property_readonly(
            "PointCb",
            [](mfxExtAV1FilmGrainParam *self) {
                return mfxAV1FilmGrainPoint_list(10, self->PointCb);
            },
            "The array of points for Cb component.")
        .def_property_readonly(
            "PointCr",
            [](mfxExtAV1FilmGrainParam *self) {
                return mfxAV1FilmGrainPoint_list(10, self->PointCr);
            },
            "The array of points for Cr component.")
        .def_readwrite("GrainScalingMinus8",
                       &mfxExtAV1FilmGrainParam::GrainScalingMinus8,
                       "The shift – 8 applied to the values of the chroma component.")
        .def_readwrite("ArCoeffLag",
                       &mfxExtAV1FilmGrainParam::ArCoeffLag,
                       "The number of auto-regressive coefficients for luma and chroma.")
        .DEF_ARRAY_PROP("ArCoeffsYPlus128", mfxExtAV1FilmGrainParam, ArCoeffsYPlus128, 24)
        .DEF_ARRAY_PROP("ArCoeffsCbPlus128", mfxExtAV1FilmGrainParam, ArCoeffsCbPlus128, 25)
        .DEF_ARRAY_PROP("ArCoeffsCrPlus128", mfxExtAV1FilmGrainParam, ArCoeffsCrPlus128, 25)
        .def_readwrite("ArCoeffShiftMinus6",
                       &mfxExtAV1FilmGrainParam::ArCoeffShiftMinus6,
                       "The range of the auto-regressive coefficients.")
        .def_readwrite(
            "GrainScaleShift",
            &mfxExtAV1FilmGrainParam::GrainScaleShift,
            "Downscaling factor of the grain synthesis process for the Gaussian random numbers.")
        .def_readwrite(
            "CbMult",
            &mfxExtAV1FilmGrainParam::CbMult,
            "The multiplier for the Cb component used in derivation of the input index to the Cb component scaling function.")
        .def_readwrite(
            "CbLumaMult",
            &mfxExtAV1FilmGrainParam::CbLumaMult,
            "The multiplier for the average luma component used in derivation of the input index to the Cb component scaling function.")
        .def_readwrite(
            "CbOffset",
            &mfxExtAV1FilmGrainParam::CbOffset,
            "The offset used in derivation of the input index to the Cb component scaling function.")
        .def_readwrite(
            "CrMult",
            &mfxExtAV1FilmGrainParam::CrMult,
            "The multiplier for the Cr component used in derivation of the input index to the Cr component scaling function.")
        .def_readwrite(
            "CrLumaMult",
            &mfxExtAV1FilmGrainParam::CrLumaMult,
            "The multiplier for the average luma component used in derivation of the input index to the Cr component scaling function.")
        .def_readwrite(
            "CrOffset",
            &mfxExtAV1FilmGrainParam::CrOffset,
            "The offset used in derivation of the input index to the Cr component scaling function.");

    py::class_<mfxSurfaceArray, std::shared_ptr<mfxSurfaceArray>>(m, "mfxSurfaceArray")
        .def(py::init<>([]() {
            mfxSurfaceArray self = {};
            self.Version.Version = MFX_SURFACEARRAY_VERSION;
            return self;
        }))
        .def_readwrite("Context", &mfxSurfaceArray::Context, "The context of the memory interface.")
        .def_readwrite("Version", &mfxSurfaceArray::Version, "The version of the structure.")
        // .def_readwrite("Surfaces", &mfxSurfaceArray::Surfaces)
        .def_readwrite("NumSurfaces",
                       &mfxSurfaceArray::NumSurfaces,
                       "The size of array of pointers to mfxFrameSurface1.")
        .def(
            "AddRef",
            [](struct mfxSurfaceArray *self) {
                return self->AddRef(self);
            },
            "Increments the internal reference counter of the surface.")
        .def(
            "Release",
            [](struct mfxSurfaceArray *self) {
                return self->Release(self);
            },
            "Decrements the internal reference counter of the surface.")
        .def(
            "GetRefCounter",
            [](struct mfxSurfaceArray *self) {
                uint32_t counter = 0;
                mfxStatus status = self->GetRefCounter(self, &counter);
                return std::tuple(status, counter);
            },
            "Returns current reference counter of mfxSurfaceArray structure.");

    py::class_<mfxRect, std::shared_ptr<mfxRect>>(m, "mfxRect")
        .def(py::init<>())
        .def_readwrite("Left",
                       &mfxRect::Left,
                       "X coordinate of region of top-left corner of rectangle.")
        .def_readwrite("Top",
                       &mfxRect::Top,
                       "Y coordinate of region of top-left corner of rectangle.")
        .def_readwrite("Right",
                       &mfxRect::Right,
                       "X coordinate of region of bottom-right corner of rectangle.")
        .def_readwrite("Bottom",
                       &mfxRect::Bottom,
                       "Y coordinate of region of bottom-right corner of rectangle.");

    py::class_<mfxExtInCrops, std::shared_ptr<mfxExtInCrops>>(m, "mfxExtInCrops")
        .def(py::init<>())
        .def_readwrite("Header",
                       &mfxExtInCrops::Header,
                       "Extension buffer header. BufferId must be equal to MFX_EXTBUFF_CROPS.")
        .def_readwrite("Crops",
                       &mfxExtInCrops::Crops,
                       "Crops parameters for letterboxing operations.");

    py::class_<mfxExtHyperModeParam, std::shared_ptr<mfxExtHyperModeParam>>(m,
                                                                            "mfxExtHyperModeParam")
        .def(py::init<>())
        .def_readwrite(
            "Header",
            &mfxExtHyperModeParam::Header,
            "Extension buffer header. BufferId must be equal to MFX_EXTBUFF_HYPER_MODE_PARAM.")
        .def_readwrite("Mode", &mfxExtHyperModeParam::Mode, "HyperMode implementation behavior.");
}