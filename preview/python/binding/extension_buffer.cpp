//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/extension_buffer.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

template <typename T, uint32_t ID>
class extension_buffer_template {
public:
    using BaseClass = vpl::extension_buffer_base;
    using Class     = vpl::extension_buffer<T, ID>;
    using PyClass   = py::class_<Class, BaseClass, std::shared_ptr<Class>>;
    PyClass pyclass;
    extension_buffer_template(const py::module &m, const std::string &typestr, bool is_base = false)
            : pyclass(m, (std::string("extension_buffer_") + typestr).c_str()) {
        (void)is_base;
        pyclass
            .def_property_readonly("ID",
                                   &Class::get_ID,
                                   "ID of the extension buffer in a form of FourCC code.")
            .def_property_readonly("size",
                                   &Class::get_size,
                                   "Size of the extension buffer in bytes.")
            .def(
                "ref",
                [](Class *self) {
                    return self->get_ref();
                },
                py::return_value_policy::reference_internal,
                "Returns reference to underlying C structure with the extension buffer.")
            .def("get",
                 &Class::get,
                 "Returns instance of underlying C structure with the extension buffer.")
            .def("__eq__", [](Class *self, Class &other) {
                return memcmp(&self->get_ref(), &other.get_ref(), sizeof(self->get_ref())) == 0;
            });
    }
};

template <typename T, uint32_t ID>
class extension_buffer_trival_template {
public:
    using BaseClass = vpl::extension_buffer<T, ID>;
    using Class     = vpl::extension_buffer_trival<T, ID>;
    using PyClass   = py::class_<Class, BaseClass, std::shared_ptr<Class>>;
    extension_buffer_template<T, ID> base;
    PyClass pyclass;
    extension_buffer_trival_template(const py::module &m,
                                     const std::string &typestr,
                                     bool is_base = false)
            : base(m, typestr),
              pyclass(m, (std::string("extension_buffer_trival_") + typestr).c_str()) {
        pyclass
            .def_property_readonly("ID",
                                   &Class::get_ID,
                                   "ID of the extension buffer in a form of FourCC code.")
            .def_property_readonly("size",
                                   &Class::get_size,
                                   "Size of the extension buffer in bytes.")
            .def(
                "ref",
                [](Class *self) {
                    return &(self->get_ref());
                },
                py::return_value_policy::reference_internal,
                "Returns reference to underlying C structure with the extension buffer.")
            .def("get",
                 &Class::get,
                 "Returns instance of underlying C structure with the extension buffer.")
            .def("__eq__", [](Class *self, Class &other) {
                return memcmp(&self->get_ref(), &other.get_ref(), sizeof(self->get_ref())) == 0;
            });
        if (is_base) {
            pyclass.def(py::init<>()).def(py::init<const Class &>());
        }
    }
};

template <typename T, uint32_t ID>
class extension_buffer_with_ptrs_template {
public:
    using Class     = vpl::extension_buffer_with_ptrs<T, ID>;
    using BaseClass = vpl::extension_buffer<T, ID>;
    using PyClass   = py::class_<Class, BaseClass, std::shared_ptr<Class>>;
    extension_buffer_template<T, ID> base;
    PyClass pyclass;
    extension_buffer_with_ptrs_template(const py::module &m,
                                        const std::string &typestr,
                                        bool is_base = false)
            : base(m, typestr),
              pyclass(m, (std::string("extension_buffer_with_ptrs_") + typestr).c_str()) {}
};

void init_extension_buffer(const py::module &m) {
    auto extension_buffer_base =
        py::class_<vpl::extension_buffer_base, std::shared_ptr<vpl::extension_buffer_base>>(
            m,
            "extension_buffer_base")
            .def_property_readonly(
                "ID",
                &vpl::extension_buffer_base::get_ID,
                "Interface to retrieve extension buffer ID in a form of FourCC code.")
            .def_property_readonly(
                "ptr",
                [](vpl::extension_buffer_base *self, std::pair<uint32_t, uint32_t>) {
                    mfxExtBuffer *ptr = self->get_base_ptr();
                    return std::pair(ptr->BufferId, ptr->BufferSz);
                },
                "Raw pointer to underlying C structure with the extension buffer.");

#define BIND_TRIVIAL_EXT_BUFFER(className, extBuffer, bufferID)                          \
    [&m]() {                                                                             \
        using Base = ::extension_buffer_trival_template<extBuffer, bufferID>;            \
        Base(m, #className);                                                             \
        return py::class_<vpl::className, Base::Class, std::shared_ptr<vpl::className>>( \
                   m,                                                                    \
                   #className)                                                           \
            .def(py::init<>());                                                          \
    }()

#define BIND_TRIVIAL_EXT_BUFFER_NOINIT(className, extBuffer, bufferID)                   \
    [&m]() {                                                                             \
        using Base = ::extension_buffer_trival_template<extBuffer, bufferID>;            \
        Base(m, #className);                                                             \
        return py::class_<vpl::className, Base::Class, std::shared_ptr<vpl::className>>( \
            m,                                                                           \
            #className);                                                                 \
    }()

#define BIND_SINGLE_POINTER_EXT_BUFFER(className, extBuffer, bufferID, ptrType)          \
    [&m]() {                                                                             \
        using Base = ::extension_buffer_with_ptrs_template<extBuffer, bufferID>;         \
        Base(m, #className);                                                             \
        return py::class_<vpl::className, Base::Class, std::shared_ptr<vpl::className>>( \
                   m,                                                                    \
                   #className)                                                           \
            .def(py::init<std::vector<ptrType> &>())                                     \
            .def(py::init<>([]() {                                                       \
                std::vector<ptrType> vec;                                                \
                return new vpl::className(vec);                                          \
            }));                                                                         \
    }()

#define BIND_POINTER_EXT_BUFFER(className, extBuffer, bufferID)                          \
    [&m]() {                                                                             \
        using Base = ::extension_buffer_with_ptrs_template<extBuffer, bufferID>;         \
        Base(m, #className);                                                             \
        return py::class_<vpl::className, Base::Class, std::shared_ptr<vpl::className>>( \
            m,                                                                           \
            #className);                                                                 \
    }()

    auto ExtCodingOption =
        BIND_TRIVIAL_EXT_BUFFER(ExtCodingOption, mfxExtCodingOption, MFX_EXTBUFF_CODING_OPTION);
    auto ExtCodingOption2 =
        BIND_TRIVIAL_EXT_BUFFER(ExtCodingOption2, mfxExtCodingOption2, MFX_EXTBUFF_CODING_OPTION2);
    auto ExtCodingOption3 =
        BIND_TRIVIAL_EXT_BUFFER(ExtCodingOption3, mfxExtCodingOption3, MFX_EXTBUFF_CODING_OPTION3);
    auto ExtVPPDenoise2 =
        BIND_TRIVIAL_EXT_BUFFER(ExtVPPDenoise2, mfxExtVPPDenoise2, MFX_EXTBUFF_VPP_DENOISE2);
    auto ExtVPPDetail =
        BIND_TRIVIAL_EXT_BUFFER(ExtVPPDetail, mfxExtVPPDetail, MFX_EXTBUFF_VPP_DETAIL);
    auto ExtVPPProcAmp =
        BIND_TRIVIAL_EXT_BUFFER(ExtVPPProcAmp, mfxExtVPPProcAmp, MFX_EXTBUFF_VPP_PROCAMP);
    auto ExtVppAuxData =
        BIND_TRIVIAL_EXT_BUFFER_NOINIT(ExtVppAuxData, mfxExtVppAuxData, MFX_EXTBUFF_VPP_AUXDATA)
            .def(py::init<vpl::pic_struct>())
            .def(py::init<>([]() {
                vpl::pic_struct pic = {};
                return new vpl::ExtVppAuxData(pic);
            }));
    ;
    auto ExtVideoSignalInfo        = BIND_TRIVIAL_EXT_BUFFER(ExtVideoSignalInfo,
                                                      mfxExtVideoSignalInfo,
                                                      MFX_EXTBUFF_VIDEO_SIGNAL_INFO);
    auto ExtAVCRefListCtrl         = BIND_TRIVIAL_EXT_BUFFER(ExtAVCRefListCtrl,
                                                     mfxExtAVCRefListCtrl,
                                                     MFX_EXTBUFF_AVC_REFLIST_CTRL);
    auto ExtVPPFrameRateConversion = BIND_TRIVIAL_EXT_BUFFER(ExtVPPFrameRateConversion,
                                                             mfxExtVPPFrameRateConversion,
                                                             MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION);
    auto ExtVPPImageStab           = BIND_TRIVIAL_EXT_BUFFER(ExtVPPImageStab,
                                                   mfxExtVPPImageStab,
                                                   MFX_EXTBUFF_VPP_IMAGE_STABILIZATION);
    auto ExtMasteringDisplayColourVolume =
        BIND_TRIVIAL_EXT_BUFFER(ExtMasteringDisplayColourVolume,
                                mfxExtMasteringDisplayColourVolume,
                                MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME);
    auto ExtContentLightLevelInfo = BIND_TRIVIAL_EXT_BUFFER(ExtContentLightLevelInfo,
                                                            mfxExtContentLightLevelInfo,
                                                            MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO);
    auto ExtPictureTimingSEI      = BIND_TRIVIAL_EXT_BUFFER(ExtPictureTimingSEI,
                                                       mfxExtPictureTimingSEI,
                                                       MFX_EXTBUFF_PICTURE_TIMING_SEI);
    auto ExtAvcTemporalLayers     = BIND_TRIVIAL_EXT_BUFFER(ExtAvcTemporalLayers,
                                                        mfxExtAvcTemporalLayers,
                                                        MFX_EXTBUFF_AVC_TEMPORAL_LAYERS);
    auto ExtEncoderCapability     = BIND_TRIVIAL_EXT_BUFFER(ExtEncoderCapability,
                                                        mfxExtEncoderCapability,
                                                        MFX_EXTBUFF_ENCODER_CAPABILITY);
    auto ExtEncoderResetOption    = BIND_TRIVIAL_EXT_BUFFER_NOINIT(ExtEncoderResetOption,
                                                                mfxExtEncoderResetOption,
                                                                MFX_EXTBUFF_ENCODER_RESET_OPTION)
                                     .def(py::init<vpl::coding_option>())
                                     .def(py::init<>([]() {
                                         vpl::coding_option opt = {};
                                         return new vpl::ExtEncoderResetOption(opt);
                                     }));
    auto ExtAVCEncodedFrameInfo = BIND_TRIVIAL_EXT_BUFFER(ExtAVCEncodedFrameInfo,
                                                          mfxExtAVCEncodedFrameInfo,
                                                          MFX_EXTBUFF_ENCODED_FRAME_INFO);
    auto ExtVPPVideoSignalInfo  = BIND_TRIVIAL_EXT_BUFFER(ExtVPPVideoSignalInfo,
                                                         mfxExtVPPVideoSignalInfo,
                                                         MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO);
    auto ExtEncoderROI =
        BIND_TRIVIAL_EXT_BUFFER(ExtEncoderROI, mfxExtEncoderROI, MFX_EXTBUFF_ENCODER_ROI);
    auto ExtAVCRefLists =
        BIND_TRIVIAL_EXT_BUFFER(ExtAVCRefLists, mfxExtAVCRefLists, MFX_EXTBUFF_AVC_REFLISTS);
    auto ExtVPPDeinterlacing   = BIND_TRIVIAL_EXT_BUFFER(ExtVPPDeinterlacing,
                                                       mfxExtVPPDeinterlacing,
                                                       MFX_EXTBUFF_VPP_DEINTERLACING);
    auto ExtVPPFieldProcessing = BIND_TRIVIAL_EXT_BUFFER(ExtVPPFieldProcessing,
                                                         mfxExtVPPFieldProcessing,
                                                         MFX_EXTBUFF_VPP_FIELD_PROCESSING);
    auto ExtDecVideoProcessing = BIND_TRIVIAL_EXT_BUFFER(ExtDecVideoProcessing,
                                                         mfxExtDecVideoProcessing,
                                                         MFX_EXTBUFF_DEC_VIDEO_PROCESSING);
    auto ExtChromaLocInfo =
        BIND_TRIVIAL_EXT_BUFFER(ExtChromaLocInfo, mfxExtChromaLocInfo, MFX_EXTBUFF_CHROMA_LOC_INFO);
    auto ExtInsertHeaders =
        BIND_TRIVIAL_EXT_BUFFER(ExtInsertHeaders, mfxExtInsertHeaders, MFX_EXTBUFF_INSERT_HEADERS);
    auto ExtHEVCTiles =
        BIND_TRIVIAL_EXT_BUFFER(ExtHEVCTiles, mfxExtHEVCTiles, MFX_EXTBUFF_HEVC_TILES);
    auto ExtHEVCParam =
        BIND_TRIVIAL_EXT_BUFFER(ExtHEVCParam, mfxExtHEVCParam, MFX_EXTBUFF_HEVC_PARAM);
    auto ExtDecodeErrorReport = BIND_TRIVIAL_EXT_BUFFER(ExtDecodeErrorReport,
                                                        mfxExtDecodeErrorReport,
                                                        MFX_EXTBUFF_DECODE_ERROR_REPORT);
    auto ExtDecodedFrameInfo  = BIND_TRIVIAL_EXT_BUFFER(ExtDecodedFrameInfo,
                                                       mfxExtDecodedFrameInfo,
                                                       MFX_EXTBUFF_DECODED_FRAME_INFO)
                                   .def(py::init<vpl::frame_type>());
    ;
    auto ExtTimeCode = BIND_TRIVIAL_EXT_BUFFER(ExtTimeCode, mfxExtTimeCode, MFX_EXTBUFF_TIME_CODE);
    auto ExtHEVCRegion =
        BIND_TRIVIAL_EXT_BUFFER(ExtHEVCRegion, mfxExtHEVCRegion, MFX_EXTBUFF_HEVC_REGION);
    auto ExtPredWeightTable   = BIND_TRIVIAL_EXT_BUFFER(ExtPredWeightTable,
                                                      mfxExtPredWeightTable,
                                                      MFX_EXTBUFF_PRED_WEIGHT_TABLE);
    auto ExtAVCRoundingOffset = BIND_TRIVIAL_EXT_BUFFER(ExtAVCRoundingOffset,
                                                        mfxExtAVCRoundingOffset,
                                                        MFX_EXTBUFF_AVC_ROUNDING_OFFSET)
                                    .def("set_rounding_offset_intra",
                                         &vpl::ExtAVCRoundingOffset::set_rounding_offset_intra,
                                         "Set rounding offset intra")
                                    .def("set_rounding_offset_inter",
                                         &vpl::ExtAVCRoundingOffset::set_rounding_offset_inter,
                                         "Set rounding offset inter");
    auto ExtDirtyRect =
        BIND_TRIVIAL_EXT_BUFFER(ExtDirtyRect, mfxExtDirtyRect, MFX_EXTBUFF_DIRTY_RECTANGLES);
    auto ExtMoveRect =
        BIND_TRIVIAL_EXT_BUFFER(ExtMoveRect, mfxExtMoveRect, MFX_EXTBUFF_MOVING_RECTANGLES);
    auto ExtVPPRotation =
        BIND_TRIVIAL_EXT_BUFFER(ExtVPPRotation, mfxExtVPPRotation, MFX_EXTBUFF_VPP_ROTATION);
    auto ExtVPPScaling =
        BIND_TRIVIAL_EXT_BUFFER(ExtVPPScaling, mfxExtVPPScaling, MFX_EXTBUFF_VPP_SCALING);
    auto ExtVPPMirroring =
        BIND_TRIVIAL_EXT_BUFFER(ExtVPPMirroring, mfxExtVPPMirroring, MFX_EXTBUFF_VPP_MIRRORING);
    auto ExtMVOverPicBoundaries = BIND_TRIVIAL_EXT_BUFFER(ExtMVOverPicBoundaries,
                                                          mfxExtMVOverPicBoundaries,
                                                          MFX_EXTBUFF_MV_OVER_PIC_BOUNDARIES)
                                      .def("stick_top",
                                           &vpl::ExtMVOverPicBoundaries::stick_top,
                                           "Set stick to the top boundary mode")
                                      .def("stick_bottom",
                                           &vpl::ExtMVOverPicBoundaries::stick_bottom,
                                           "Set stick to the bottom boundary mode")
                                      .def("stick_left",
                                           &vpl::ExtMVOverPicBoundaries::stick_left,
                                           "Set stick to the left boundary mode")
                                      .def("stick_right",
                                           &vpl::ExtMVOverPicBoundaries::stick_right,
                                           "Set stick to the right boundary mode");
    auto ExtVPPColorFill =
        BIND_TRIVIAL_EXT_BUFFER(ExtVPPColorFill, mfxExtVPPColorFill, MFX_EXTBUFF_VPP_COLORFILL)
            .def("enable_fill_outside",
                 &vpl::ExtVPPColorFill::enable_fill_outside,
                 "Fill the area between Width/Height and Crop borders");
    auto ExtColorConversion   = BIND_TRIVIAL_EXT_BUFFER(ExtColorConversion,
                                                      mfxExtColorConversion,
                                                      MFX_EXTBUFF_VPP_COLOR_CONVERSION);
    auto ExtVP9TemporalLayers = BIND_TRIVIAL_EXT_BUFFER(ExtVP9TemporalLayers,
                                                        mfxExtVP9TemporalLayers,
                                                        MFX_EXTBUFF_VP9_TEMPORAL_LAYERS);
    auto ExtVP9Param = BIND_TRIVIAL_EXT_BUFFER(ExtVP9Param, mfxExtVP9Param, MFX_EXTBUFF_VP9_PARAM);
    auto ExtVppMctf  = BIND_TRIVIAL_EXT_BUFFER(ExtVppMctf, mfxExtVppMctf, MFX_EXTBUFF_VPP_MCTF);
    auto ExtPartialBitstreamParam = BIND_TRIVIAL_EXT_BUFFER(ExtPartialBitstreamParam,
                                                            mfxExtPartialBitstreamParam,
                                                            MFX_EXTBUFF_PARTIAL_BITSTREAM_PARAM);

    auto ExtAV1BitstreamParam  = BIND_TRIVIAL_EXT_BUFFER(ExtAV1BitstreamParam,
                                                        mfxExtAV1BitstreamParam,
                                                        MFX_EXTBUFF_AV1_BITSTREAM_PARAM);
    auto ExtAV1ResolutionParam = BIND_TRIVIAL_EXT_BUFFER(ExtAV1ResolutionParam,
                                                         mfxExtAV1ResolutionParam,
                                                         MFX_EXTBUFF_AV1_RESOLUTION_PARAM);
    auto ExtAV1TileParam =
        BIND_TRIVIAL_EXT_BUFFER(ExtAV1TileParam, mfxExtAV1TileParam, MFX_EXTBUFF_AV1_TILE_PARAM);
    auto ExtAV1FilmGrainParam = BIND_TRIVIAL_EXT_BUFFER(ExtAV1FilmGrainParam,
                                                        mfxExtAV1FilmGrainParam,
                                                        MFX_EXTBUFF_AV1_FILM_GRAIN_PARAM);

    auto ExtAV1Segmentation =
        BIND_SINGLE_POINTER_EXT_BUFFER(ExtAV1Segmentation,
                                       mfxExtAV1Segmentation,
                                       MFX_EXTBUFF_AV1_SEGMENTATION,
                                       uint8_t)
            .def("set_NumSegments", &vpl::ExtAV1Segmentation::set_NumSegments, "Set Num Segments")
            .def("set_SegmentIdBlockSize",
                 &vpl::ExtAV1Segmentation::set_SegmentIdBlockSize,
                 "Set Segment ID Block Size")
            .def(
                "set_Segment",
                [](vpl::ExtAV1Segmentation &e, const std::vector<mfxAV1SegmentParam> &value) {
                    if (value.size() <= 8) {
                        throw vpl::base_exception("Insufficant data", MFX_ERR_UNKNOWN);
                    }
                    e.set_Segment(&value[0]);
                },
                "Set Segment");

    auto ExtTemporalLayers = BIND_SINGLE_POINTER_EXT_BUFFER(ExtTemporalLayers,
                                                            mfxExtTemporalLayers,
                                                            MFX_EXTBUFF_UNIVERSAL_TEMPORAL_LAYERS,
                                                            mfxTemporalLayer)
                                 .def("set_BaseLayerPID",
                                      &vpl::ExtTemporalLayers::set_BaseLayerPID,
                                      "Set Base Layer PID");

    auto ExtCodingOptionVPS =
        BIND_SINGLE_POINTER_EXT_BUFFER(ExtCodingOptionVPS,
                                       mfxExtCodingOptionVPS,
                                       MFX_EXTBUFF_CODING_OPTION_VPS,
                                       uint8_t)
            .def("set_VPSId", &vpl::ExtCodingOptionVPS::set_VPSId, "Set VPS ID");

    auto ExtVPPComposite =
        BIND_SINGLE_POINTER_EXT_BUFFER(ExtVPPComposite,
                                       mfxExtVPPComposite,
                                       MFX_EXTBUFF_CODING_OPTION_VPS,
                                       mfxVPPCompInputStream)
            .def("set_Y", &vpl::ExtVPPComposite::set_Y, "Set Y")
            .def("set_R", &vpl::ExtVPPComposite::set_R, "Set R")
            .def("set_U", &vpl::ExtVPPComposite::set_U, "Set U")
            .def("set_G", &vpl::ExtVPPComposite::set_G, "Set G")
            .def("set_V", &vpl::ExtVPPComposite::set_V, "Set V")
            .def("set_B", &vpl::ExtVPPComposite::set_B, "Set B")
            .def("set_NumTiles", &vpl::ExtVPPComposite::set_NumTiles, "Set Number of Tiles");

    auto ExtEncoderIPCMArea   = BIND_SINGLE_POINTER_EXT_BUFFER(ExtEncoderIPCMArea,
                                                             mfxExtEncoderIPCMArea,
                                                             MFX_EXTBUFF_ENCODER_IPCM_AREA,
                                                             mfxExtEncoderIPCMArea::area);
    auto ExtMBForceIntra      = BIND_SINGLE_POINTER_EXT_BUFFER(ExtMBForceIntra,
                                                          mfxExtMBForceIntra,
                                                          MFX_EXTBUFF_MB_FORCE_INTRA,
                                                          uint8_t);
    auto ExtMBDisableSkipMap  = BIND_SINGLE_POINTER_EXT_BUFFER(ExtMBDisableSkipMap,
                                                              mfxExtMBDisableSkipMap,
                                                              MFX_EXTBUFF_MB_DISABLE_SKIP_MAP,
                                                              uint8_t);
    auto ExtEncodedSlicesInfo = BIND_SINGLE_POINTER_EXT_BUFFER(ExtEncodedSlicesInfo,
                                                               mfxExtEncodedSlicesInfo,
                                                               MFX_EXTBUFF_ENCODED_SLICES_INFO,
                                                               uint16_t)
                                    .def("set_SliceSizeOverflow",
                                         &vpl::ExtEncodedSlicesInfo::set_SliceSizeOverflow,
                                         "Set Slice Size Overflow")
                                    .def("set_NumSliceNonCopliant",
                                         &vpl::ExtEncodedSlicesInfo::set_NumSliceNonCopliant,
                                         "Set Num Slice Non-copliant")
                                    .def("set_NumEncodedSlice",
                                         &vpl::ExtEncodedSlicesInfo::set_NumEncodedSlice,
                                         "Set Num Encoded Slice");

    auto ExtVP9Segmentation =
        BIND_SINGLE_POINTER_EXT_BUFFER(ExtVP9Segmentation,
                                       mfxExtVP9Segmentation,
                                       MFX_EXTBUFF_VP9_SEGMENTATION,
                                       uint8_t)
            .def("set_NumSegments", &vpl::ExtVP9Segmentation::set_NumSegments, "Set Num Segments")
            .def("set_SegmentIdBlockSize",
                 &vpl::ExtVP9Segmentation::set_SegmentIdBlockSize,
                 "Set Segment ID Block Size")
            .def(
                "set_Segment",
                [](vpl::ExtVP9Segmentation &e, const std::vector<mfxVP9SegmentParam> &value) {
                    if (value.size() <= 8) {
                        throw vpl::base_exception("Insufficant data", MFX_ERR_UNKNOWN);
                    }
                    e.set_Segment(&value[0]);
                },
                "Set Segment");
    auto ExtEncodedUnitsInfo = BIND_SINGLE_POINTER_EXT_BUFFER(ExtEncodedUnitsInfo,
                                                              mfxExtEncodedUnitsInfo,
                                                              MFX_EXTBUFF_ENCODED_UNITS_INFO,
                                                              mfxEncodedUnitInfo)
                                   .def("set_NumUnitsEncoded",
                                        &vpl::ExtEncodedUnitsInfo::set_NumUnitsEncoded,
                                        "Set Num Units Encoded");

    auto ExtVPPDoNotUse =
        BIND_POINTER_EXT_BUFFER(ExtVPPDoNotUse, mfxExtVPPDoNotUse, MFX_EXTBUFF_VPP_DONOTUSE)
            .def(py::init<std::vector<uint32_t>>())
            .def(py::init<>([]() {
                return new vpl::ExtVPPDoNotUse({});
            }));

    auto ExtVPPDoUse = BIND_POINTER_EXT_BUFFER(ExtVPPDoUse, mfxExtVPPDoUse, MFX_EXTBUFF_VPP_DOUSE)
                           .def(py::init<std::vector<uint32_t>>())
                           .def(py::init<>([]() {
                               return new vpl::ExtVPPDoUse({});
                           }));

    auto ExtCodingOptionSPSPPS = BIND_POINTER_EXT_BUFFER(ExtCodingOptionSPSPPS,
                                                         mfxExtCodingOptionSPSPPS,
                                                         MFX_EXTBUFF_CODING_OPTION_SPSPPS)
                                     .def(py::init<>([]() {
                                         std::vector<uint8_t> v1;
                                         std::vector<uint8_t> v2;
                                         return new vpl::ExtCodingOptionSPSPPS(v1, v2);
                                     }))
                                     .def(py::init<std::vector<uint8_t>, std::vector<uint8_t>>());

    auto ExtMBQP = BIND_POINTER_EXT_BUFFER(ExtMBQP, mfxExtMBQP, MFX_EXTBUFF_MBQP)
                       .def(py::init<>([](std::vector<uint8_t> vec) {
                           return new vpl::ExtMBQP(vec);
                       }))
                       .def(py::init<>([]() {
                           std::vector<uint8_t> vec;
                           return new vpl::ExtMBQP(vec);
                       }));

    auto ExtDeviceAffinityMask = BIND_POINTER_EXT_BUFFER(ExtDeviceAffinityMask,
                                                         mfxExtDeviceAffinityMask,
                                                         MFX_EXTBUFF_DEVICE_AFFINITY_MASK)
                                     .def(py::init<std::string, uint32_t, std::vector<uint8_t>>())
                                     .def(py::init<>([]() {
                                         std::vector<uint8_t> vec;
                                         return new vpl::ExtDeviceAffinityMask("", 0, vec);
                                     }));

    auto EncodeCtrl =
        BIND_POINTER_EXT_BUFFER(EncodeCtrl, mfxEncodeCtrl, 0)
            .def(py::init<std::vector<vpl::payload>>())
            .def(py::init<vpl::frame_type, std::vector<vpl::payload>>())
            .def(py::init<vpl::frame_type, uint16_t, uint16_t, std::vector<vpl::payload>>())
            .def(py::init<uint16_t,
                          vpl::frame_type,
                          uint16_t,
                          uint16_t,
                          std::vector<vpl::payload>>());
}