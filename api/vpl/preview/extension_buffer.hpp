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
    virtual mfxExtBuffer* get_ptr() = 0;
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

    /// @brief Returns raw pointer to underlying C structure with the extension buffer.
    /// @return Raw pointer to underlying C structure.
    mfxExtBuffer* get_ptr() {
        return reinterpret_cast<mfxExtBuffer*>(&buffer_);
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

REGISTER_TRIVIAL_EXT_BUFFER(ExtCodingOption, mfxExtCodingOption, MFX_EXTBUFF_CODING_OPTION)
REGISTER_TRIVIAL_EXT_BUFFER(ExtCodingOption2, mfxExtCodingOption2, MFX_EXTBUFF_CODING_OPTION2)
REGISTER_TRIVIAL_EXT_BUFFER(ExtCodingOption3, mfxExtCodingOption3, MFX_EXTBUFF_CODING_OPTION3)

REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPDenoise, mfxExtVPPDenoise, MFX_EXTBUFF_VPP_DENOISE)
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
REGISTER_TRIVIAL_EXT_BUFFER(ExtEncoderResetOption,
                            mfxExtEncoderResetOption,
                            MFX_EXTBUFF_ENCODER_RESET_OPTION)
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
REGISTER_TRIVIAL_EXT_BUFFER(ExtAVCRoundingOffset,
                            mfxExtAVCRoundingOffset,
                            MFX_EXTBUFF_AVC_ROUNDING_OFFSET)
REGISTER_TRIVIAL_EXT_BUFFER(ExtDirtyRect, mfxExtDirtyRect, MFX_EXTBUFF_DIRTY_RECTANGLES)
REGISTER_TRIVIAL_EXT_BUFFER(ExtMoveRect, mfxExtMoveRect, MFX_EXTBUFF_MOVING_RECTANGLES)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPRotation, mfxExtVPPRotation, MFX_EXTBUFF_VPP_ROTATION)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPScaling, mfxExtVPPScaling, MFX_EXTBUFF_VPP_SCALING)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPMirroring, mfxExtVPPMirroring, MFX_EXTBUFF_VPP_MIRRORING)
REGISTER_TRIVIAL_EXT_BUFFER(ExtMVOverPicBoundaries,
                            mfxExtMVOverPicBoundaries,
                            MFX_EXTBUFF_MV_OVER_PIC_BOUNDARIES)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVPPColorFill, mfxExtVPPColorFill, MFX_EXTBUFF_VPP_COLORFILL)
REGISTER_TRIVIAL_EXT_BUFFER(ExtColorConversion,
                            mfxExtColorConversion,
                            MFX_EXTBUFF_VPP_COLOR_CONVERSION)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVP9TemporalLayers,
                            mfxExtVP9TemporalLayers,
                            MFX_EXTBUFF_VP9_TEMPORAL_LAYERS)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVP9Param, mfxExtVP9Param, MFX_EXTBUFF_VP9_PARAM)
REGISTER_TRIVIAL_EXT_BUFFER(ExtVppMctf, mfxExtVppMctf, MFX_EXTBUFF_VPP_MCTF)
REGISTER_TRIVIAL_EXT_BUFFER(ExtPartialBitstreamParam,
                            mfxExtPartialBitstreamParam,
                            MFX_EXTBUFF_PARTIAL_BITSTREAM_PARAM)

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
            if (this->buffer_.ptr)                                                                  \
                delete[] this->buffer_.ptr;                                                         \
                this->buffer_.ptr = NULL;                                                           \
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
    explicit EncodeCtrl(const std::vector<payload> &userPayload)
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
