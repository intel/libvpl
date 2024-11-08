/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/mfx_config_interface/mfx_config_interface.h"

#include <cctype>
#include <cinttypes>
#include <cstring>
#include <limits>
#include <type_traits>
#include <vector>
#include "vpl/mfxcommon.h"

namespace MFX_CONFIG_INTERFACE {

static const char ebPrefix[] = "mfxExt";

struct ExtBufType {
    mfxU32 BufferId;
    mfxU32 BufferSz;
    std::string ParamStr;
};

static const ExtBufType extBufTypeTab[] = {
    { MFX_EXTBUFF_HEVC_PARAM, sizeof(mfxExtHEVCParam), "HEVCParam" },
    { MFX_EXTBUFF_CODING_OPTION2, sizeof(mfxExtCodingOption2), "CodingOption2" },
    { MFX_EXTBUFF_CODING_OPTION, sizeof(mfxExtCodingOption), "CodingOption" },
    { MFX_EXTBUFF_CODING_OPTION3, sizeof(mfxExtCodingOption3), "CodingOption3" },
    { MFX_EXTBUFF_VPP_DONOTUSE, sizeof(mfxExtVPPDoNotUse), "VPPDoNotUse" },
    { MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION, sizeof(mfxExtVPPFrameRateConversion), "VPPFrameRateConversion" },
    { MFX_EXTBUFF_VPP_IMAGE_STABILIZATION, sizeof(mfxExtVPPImageStab), "VPPImageStab" },
    { MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME, sizeof(mfxExtMasteringDisplayColourVolume), "MasteringDisplayColourVolume" },
    { MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO, sizeof(mfxExtContentLightLevelInfo), "ContentLightLevelInfo" },
    { MFX_EXTBUFF_AVC_TEMPORAL_LAYERS, sizeof(mfxExtAvcTemporalLayers), "AvcTemporalLayers" },
    { MFX_EXTBUFF_VPP_COMPOSITE, sizeof(mfxExtVPPComposite), "VPPComposite" },
    { MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO, sizeof(mfxExtVPPVideoSignalInfo), "VPPVideoSignalInfo" },
    { MFX_EXTBUFF_VPP_DEINTERLACING, sizeof(mfxExtVPPDeinterlacing), "VPPDeinterlacing" },
    { MFX_EXTBUFF_AVC_REFLISTS, sizeof(mfxExtAVCRefLists), "AVCRefLists" },
    { MFX_EXTBUFF_VPP_FIELD_PROCESSING, sizeof(mfxExtVPPFieldProcessing), "VPPFieldProcessing" },
    { MFX_EXTBUFF_DEC_VIDEO_PROCESSING, sizeof(mfxExtDecVideoProcessing), "DecVideoProcessing" },
    { MFX_EXTBUFF_CHROMA_LOC_INFO, sizeof(mfxExtChromaLocInfo), "ChromaLocInfo" },
    { MFX_EXTBUFF_HEVC_TILES, sizeof(mfxExtHEVCTiles), "HEVCTiles" },
    { MFX_EXTBUFF_VPP_ROTATION, sizeof(mfxExtVPPRotation), "VPPRotation" },
    { MFX_EXTBUFF_VPP_SCALING, sizeof(mfxExtVPPScaling), "VPPScaling" },
    { MFX_EXTBUFF_VPP_MIRRORING, sizeof(mfxExtVPPMirroring), "VPPMirroring" },
    { MFX_EXTBUFF_VPP_COLORFILL, sizeof(mfxExtVPPColorFill), "VPPColorFill" },
    { MFX_EXTBUFF_VPP_COLOR_CONVERSION, sizeof(mfxExtColorConversion), "ColorConversion" },
    { MFX_EXTBUFF_VP9_SEGMENTATION, sizeof(mfxExtVP9Segmentation), "VP9Segmentation" },
    { MFX_EXTBUFF_VP9_TEMPORAL_LAYERS, sizeof(mfxExtVP9TemporalLayers), "VP9TemporalLayers" },
    { MFX_EXTBUFF_AV1_FILM_GRAIN_PARAM, sizeof(mfxExtAV1FilmGrainParam), "AV1FilmGrainParam" },
    { MFX_EXTBUFF_AV1_RESOLUTION_PARAM, sizeof(mfxExtAV1ResolutionParam), "AV1ResolutionParam" },
    { MFX_EXTBUFF_AV1_SEGMENTATION, sizeof(mfxExtAV1Segmentation), "AV1Segmentation" },
    { MFX_EXTBUFF_AV1_TILE_PARAM, sizeof(mfxExtAV1TileParam), "AV1TileParam" },
    { MFX_EXTBUFF_ENCODED_FRAME_INFO, sizeof(mfxExtAVCEncodedFrameInfo), "AVCEncodedFrameInfo" },
    { MFX_EXTBUFF_HEVC_REFLIST_CTRL, sizeof(mfxExtAVCRefListCtrl), "AVCRefListCtrl" },
    { MFX_EXTBUFF_AVC_ROUNDING_OFFSET, sizeof(mfxExtAVCRoundingOffset), "AVCRoundingOffset" },
    { MFX_EXTBUFF_ENCODED_SLICES_INFO, sizeof(mfxExtEncodedSlicesInfo), "EncodedSlicesInfo" },
    { MFX_HEVC_REGION_SLICE, sizeof(mfxExtHEVCRegion), "HEVCRegion" },
    { MFX_EXTBUFF_CROPS, sizeof(mfxExtInCrops), "InCrops" },
    { MFX_EXTBUFF_INSERT_HEADERS, sizeof(mfxExtInsertHeaders), "InsertHeaders" },
    { MFX_EXTBUFF_MV_OVER_PIC_BOUNDARIES, sizeof(mfxExtMVOverPicBoundaries), "MVOverPicBoundaries" },
    { MFX_EXTBUFF_VP9_PARAM, sizeof(mfxExtVP9Param), "VP9Param" },
    { MFX_EXTBUFF_TIME_CODE, sizeof(mfxExtTimeCode), "TimeCode" },
    { MFX_EXTBUFF_MBQP, sizeof(mfxExtMBQP), "MBQP" },
    { MFX_EXTBUFF_CODING_OPTION_SPSPPS, sizeof(mfxExtCodingOptionSPSPPS), "CodingOptionSPSPPS" },
    { MFX_EXTBUFF_CODING_OPTION_VPS, sizeof(mfxExtCodingOptionVPS), "CodingOptionVPS" },
    { MFX_EXTBUFF_VIDEO_SIGNAL_INFO, sizeof(mfxExtVideoSignalInfo), "VideoSignalInfo" },
    { MFX_EXTBUFF_VPP_AUXDATA, sizeof(mfxExtVppAuxData), "VppAuxData" },
    { MFX_EXTBUFF_VPP_MCTF, sizeof(mfxExtVppMctf), "VppMctf" },
    { MFX_EXTBUFF_UNIVERSAL_TEMPORAL_LAYERS, sizeof(mfxExtTemporalLayers), "TemporalLayers" },
    { MFX_EXTBUFF_PARTIAL_BITSTREAM_PARAM, sizeof(mfxExtPartialBitstreamParam), "PartialBitstreamParam" },
    { MFX_EXTBUFF_PRED_WEIGHT_TABLE, sizeof(mfxExtPredWeightTable), "PredWeightTable" },
    { MFX_EXTBUFF_ENCODED_UNITS_INFO, sizeof(mfxExtEncodedUnitsInfo), "EncodedUnitsInfo" },
    { MFX_EXTBUFF_AV1_BITSTREAM_PARAM, sizeof(mfxExtAV1BitstreamParam), "AV1BitstreamParam" },
    { MFX_EXTBUFF_ENCODER_ROI, sizeof(mfxExtEncoderROI), "EncoderROI" },
    { MFX_EXTBUFF_DECODE_ERROR_REPORT, sizeof(mfxExtDecodeErrorReport), "DecodeErrorReport" },
    { MFX_EXTBUFF_DECODED_FRAME_INFO, sizeof(mfxExtDecodedFrameInfo), "DecodedFrameInfo" },
    { MFX_EXTBUFF_ENCODER_CAPABILITY, sizeof(mfxExtEncoderCapability), "EncoderCapability" },
    { MFX_EXTBUFF_DEVICE_AFFINITY_MASK, sizeof(mfxExtDeviceAffinityMask), "DeviceAffinityMask" },
    { MFX_EXTBUFF_DIRTY_RECTANGLES, sizeof(mfxExtDirtyRect), "DirtyRect" },
    { MFX_EXTBUFF_ENCODER_IPCM_AREA, sizeof(mfxExtEncoderIPCMArea), "EncoderIPCMArea" },
    { MFX_EXTBUFF_ENCODER_RESET_OPTION, sizeof(mfxExtEncoderResetOption), "EncoderResetOption" },
    { MFX_EXTBUFF_MB_DISABLE_SKIP_MAP, sizeof(mfxExtMBDisableSkipMap), "MBDisableSkipMap" },
    { MFX_EXTBUFF_MB_FORCE_INTRA, sizeof(mfxExtMBForceIntra), "MBForceIntra" },
    { MFX_EXTBUFF_MOVING_RECTANGLES, sizeof(mfxExtMoveRect), "MoveRect" },
    { MFX_EXTBUFF_VPP_PROCAMP, sizeof(mfxExtVPPProcAmp), "VPPProcAmp" },
    { MFX_EXTBUFF_HYPER_MODE_PARAM, sizeof(mfxExtHyperModeParam), "HyperModeParam" },
    { MFX_EXTBUFF_THREADS_PARAM, sizeof(mfxExtThreadsParam), "ThreadsParam" },
    { MFX_EXTBUFF_VPP_3DLUT, sizeof(mfxExtVPP3DLut), "VPP3DLut" },
    { MFX_EXTBUFF_VPP_DENOISE, sizeof(mfxExtVPPDenoise), "VPPDenoise" },
    { MFX_EXTBUFF_VPP_DENOISE2, sizeof(mfxExtVPPDenoise2), "VPPDenoise2" },
    { MFX_EXTBUFF_VPP_DETAIL, sizeof(mfxExtVPPDetail), "VPPDetail" },
    { MFX_EXTBUFF_VPP_DOUSE, sizeof(mfxExtVPPDoUse), "VPPDoUse" },
    { MFX_EXTBUFF_PICTURE_TIMING_SEI, sizeof(mfxExtPictureTimingSEI), "PictureTimingSEI" },
    { MFX_EXTBUFF_VPP_AI_SUPER_RESOLUTION, sizeof(mfxExtVPPAISuperResolution), "VPPAISuperResolution" },
    { MFX_EXTBUFF_VPP_AI_FRAME_INTERPOLATION, sizeof(mfxExtVPPAIFrameInterpolation), "VPPAIFrameInterpolation" },
};

// Example: extBuf = mfxExtHEVCParam, fourCC = MFX_EXTBUFF_HEVC_PARAM, element = PicWidthInLumaSamples
// "mfxExtHEVCParam.PicWidthInLumaSamples=1280"
//
// This simple prefix detection can be changed - just a placeholder for now.
// We should define some consistent syntax for parameters, value types, extension buffer mapping, etc.
bool IsExtBuf(const KVPair &kvStr) {
    // check if this is an extBuf
    if (kvStr.first.rfind(ebPrefix) == 0) {
        return true;
    }

    return false;
}

// determine extBuf type based on key string - see comment above about need to decide on some patterns
// need to add implementation for each supported mfxExt*** type
mfxStatus GetExtBufType(const KVPair &kvStr, mfxExtBuffer *extBufRequired, KVPair &kvStrParsed) {
    kvStrParsed.first.clear();
    kvStrParsed.second.clear();

    std::string ebPrefixStr(ebPrefix);
    std::string extString = kvStr.first;
    if (extString.rfind(ebPrefixStr) != 0)
        return MFX_ERR_UNSUPPORTED;
    extString.erase(0, ebPrefixStr.length());

    for (ExtBufType eb : extBufTypeTab) {
        std::string extTypeStr = eb.ParamStr;
        extTypeStr += ".";
        if (extString.rfind(extTypeStr) == 0) {
            // set buffer type and erase the leading "EB_ParamStr_" portion
            extBufRequired->BufferId = eb.BufferId;
            extBufRequired->BufferSz = eb.BufferSz;
            extString.erase(0, extTypeStr.length());

            // save new key, value is unchanged
            kvStrParsed.first  = extString;
            kvStrParsed.second = kvStr.second;

            return MFX_ERR_NONE;
        }
    }

    return MFX_ERR_NOT_FOUND;
}

mfxStatus UpdateExtBufParam(const KVPair &kvStr, mfxVideoParam *videoParam, mfxExtBuffer *extBufRequired) {
    mfxStatus sts = MFX_ERR_NONE;

    // Upon return from GetExtBufType, kvStrParsed has "param=value" with the extBuf identifying prefixes removed.
    // e.g. "EB_HEVC_PARAM_PicWidthInLumaSamples=1280" --> "PicWidthInLumaSamples=1280"
    KVPair kvStrParsed = {};

    // Fill in extBuffer with with BufferId and BufferSz based on parameter name.
    sts = GetExtBufType(kvStr, extBufRequired, kvStrParsed);
    if (sts != MFX_ERR_NONE)
        return sts;

    // If no extBuf array attached, return MFX_ERR_MORE_EXTBUFFER to indicate that app needs to allocate buffer.
    // extBufRequired contains the BufferId and BufferSz for the app to use in allocating the buffer
    if (!videoParam->NumExtParam)
        return MFX_ERR_MORE_EXTBUFFER;

    if (!videoParam->ExtParam)
        return MFX_ERR_NULL_PTR; // error - NumExtParam > 0, but array pointer is null

    // Check whether an extbuf of the appropriate type has been attached.
    mfxExtBuffer *extBufFound = nullptr;
    mfxU32 idx;
    for (idx = 0; idx < videoParam->NumExtParam; idx++) {
        extBufFound = videoParam->ExtParam[idx];
        if (!extBufFound)
            return MFX_ERR_NULL_PTR;

        if ((extBufFound->BufferId == extBufRequired->BufferId) && (extBufFound->BufferSz == extBufRequired->BufferSz))
            break;
    }

    // Required extBuf not attached - return MFX_ERR_MORE_EXTBUFFER to indicate that app must allocate it.
    if (idx == videoParam->NumExtParam)
        return MFX_ERR_MORE_EXTBUFFER;

    // Update the specific field in this extBuf corresponding to the string param.
    sts = SetExtBufParam(extBufFound, kvStrParsed);
    if (sts != MFX_ERR_NONE)
        return sts;

    return MFX_ERR_NONE;
}

static inline void trim(std::string &s) {
    // trim leading whitespace
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
    // trim trailing whitespace
    s.erase(std::find_if(s.rbegin(),
                         s.rend(),
                         [](unsigned char ch) {
                             return !std::isspace(ch);
                         })
                .base(),
            s.end());
}

template <typename VType, typename Enable = void>
struct value_converter {
    static mfxStatus str_to_value(std::string value, VType &t) {
        return MFX_ERR_UNSUPPORTED;
    }
};

template <typename VType>
struct value_converter<VType, typename std::enable_if<std::is_unsigned<VType>::value && std::is_integral<VType>::value>::type> {
    static mfxStatus str_to_value(std::string value, VType &t) {
        trim(value);
        if (value.find_first_of('-', 0) != std::string::npos)
            return MFX_ERR_UNSUPPORTED;
        if (value.find_first_of('+', 0) == 0)
            value = value.substr(1, value.size() - 1);

        uint64_t converted_value = 0;
        try {
            converted_value = std::stoull(value, 0);
        }
        catch (...) {
            return MFX_ERR_UNSUPPORTED;
        }

        // error if input was out of range
        if (converted_value > std::numeric_limits<VType>::max()) {
            return MFX_ERR_UNSUPPORTED;
        }
        if (converted_value < std::numeric_limits<VType>::lowest()) {
            return MFX_ERR_UNSUPPORTED;
        }
        t = static_cast<VType>(converted_value);
        return MFX_ERR_NONE;
    }
};

template <typename VType>
struct value_converter<VType, typename std::enable_if<std::is_signed<VType>::value && std::is_integral<VType>::value>::type> {
    static mfxStatus str_to_value(std::string value, VType &t) {
        trim(value);
        int64_t converted_value = 0;
        if (value.find_first_of('+', 0) == 0)
            value = value.substr(1, value.size() - 1);
        try {
            converted_value = std::stoll(value, 0);
        }
        catch (...) {
            return MFX_ERR_UNSUPPORTED;
        }

        // error if input was out of range
        if (converted_value > std::numeric_limits<VType>::max()) {
            return MFX_ERR_UNSUPPORTED;
        }
        if (converted_value < std::numeric_limits<VType>::lowest()) {
            return MFX_ERR_UNSUPPORTED;
        }
        t = static_cast<VType>(converted_value);
        return MFX_ERR_NONE;
    }
};

template <typename VType>
struct value_converter<VType, typename std::enable_if<std::is_floating_point<VType>::value>::type> {
    static mfxStatus str_to_value(std::string value, VType &t) {
        trim(value);
        long double converted_value = 0;
        if (value.find_first_of('+', 0) == 0)
            value = value.substr(1, value.size() - 1);
        try {
            converted_value = std::stold(value);
        }
        catch (...) {
            return MFX_ERR_UNSUPPORTED;
        }

        // error if input was out of range
        if (converted_value > std::numeric_limits<VType>::max()) {
            return MFX_ERR_UNSUPPORTED;
        }
        if (converted_value < std::numeric_limits<VType>::lowest()) {
            return MFX_ERR_UNSUPPORTED;
        }
        t = static_cast<VType>(converted_value);
        return MFX_ERR_NONE;
    }
};

template <typename VType>
struct value_converter<VType, typename std::enable_if<std::is_enum<VType>::value>::type> {
    static mfxStatus str_to_value(std::string value, VType &t) {
        int v            = 0;
        mfxStatus result = value_converter<int>::str_to_value(value, v);
        if (result != MFX_ERR_NONE) {
            return result;
        }
        t = static_cast<VType>(v);
        return MFX_ERR_NONE;
    }
};

// If trimmed input is 4 characters long it is treated as a
// fourcc string otherwise it is treated as an integer
// representation of a fourcc
mfxStatus ConvertStrToFourCC(std::string value, mfxU32 &t) {
    uint32_t converted_value = 0;
    if (value.size() == 4) {
        converted_value = static_cast<uint32_t>((((uint32_t)value[0]) << 0) + (((uint32_t)value[1]) << 8) + (((uint32_t)value[2]) << 16) +
                                                (((uint32_t)value[3]) << 24));
    }
    else {
        trim(value);
        if (value.find_first_of('-', 0) != std::string::npos)
            return MFX_ERR_UNSUPPORTED;
        if (value.find_first_of('+', 0) == 0)
            value = value.substr(1, value.size() - 1);
        try {
            converted_value = std::stoul(value);
        }
        catch (...) {
            return MFX_ERR_UNSUPPORTED;
        }
    }
    // error if input was out of range
    if (converted_value > std::numeric_limits<mfxU32>::max())
        return MFX_ERR_UNSUPPORTED;
    if (converted_value < std::numeric_limits<mfxU32>::lowest())
        return MFX_ERR_UNSUPPORTED;
    t = static_cast<mfxU32>(converted_value);
    return MFX_ERR_NONE;
}

static mfxStatus ConvertStrToStr(std::string value, char *dest, size_t size) {
    size_t len = 0;
    if (value.size() >= size) {
        len = size - 1;
    }
    else {
        len = value.size();
    }
    memset(dest, 0, size);
    for (size_t i = 0; i < len; i++) {
        dest[i] = value[i];
    }
    return MFX_ERR_NONE;
}

// convert a string of format "X, Y, Z" into array of scalars [X, Y, Z]
// type is defined by arrayDataType (mfxU32, mfxI16, etc.)
// arrSize is the number of elements in the array
template <typename EType, typename FType>
static mfxStatus ConvertStrToArray(std::string value, EType *arr, mfxU32 arrSize, FType &(*field)(EType &)) {
    trim(value);
    mfxU32 idx = 0;
    std::string s;

    // parse value string into array elements, separated by ','
    // stoi (and friends) ignore leading and trailing whitespaces so no need to trim here
    std::stringstream valueSS(value);
    while (getline(valueSS, s, ',')) {
        mfxStatus sts = MFX_ERR_NONE;

        if (idx >= arrSize)
            return (mfxStatus)(MFX_ERR_UNSUPPORTED + 2000);

        sts = value_converter<FType>::str_to_value(s, field(arr[idx]));

        if (sts != MFX_ERR_NONE)
            return sts;

        idx++;
    }

    if (idx != arrSize)
        return (mfxStatus)(MFX_ERR_UNSUPPORTED + 3000); // too few elements in string

    return MFX_ERR_NONE;
}

// Set numeric field
//  p1: parameter struct
//  s1: input name
//  v1: string encoded value
//  s2: expected name
//  d1: field name in struct
#define UPDATE_VIDEO_PARAM_VALUE(p1, s1, v1, s2, d1)                        \
    if (s1 == #s2) {                                                        \
        return value_converter<decltype(p1->d1)>::str_to_value(v1, p1->d1); \
    }

// Set fourcc field
//  p1: parameter struct
//  s1: input name
//  v1: string encoded value
//  s2: expected name
//  d1: field name in struct
#define UPDATE_VIDEO_PARAM_FOURCC(p1, s1, v1, s2, d1) \
    if (s1 == #s2) {                                  \
        return ConvertStrToFourCC(v1, p1->d1);        \
    }

// Set fixed width string field
//  p1: parameter struct
//  s1: input name
//  v1: string encoded value
//  s2: expected name
//  d1: field name in struct
//  sz: field size in struct
#define UPDATE_VIDEO_PARAM_STRING(p1, s1, v1, s2, d1, sz) \
    if (s1 == #s2) {                                      \
        return ConvertStrToStr(v1, p1->d1, sz);           \
    }

// Set array field
//  p1: parameter struct
//  s1: input name
//  v1: string encoded value
//  s2: expected name
//  d1: field name in struct
//  ty: type of array elements
//  sz: array size in struct
#define UPDATE_VIDEO_PARAM_FLAT_ARRAY(p1, s1, v1, s2, d1, ty, sz)                    \
    if (s1 == #s2) {                                                                 \
        return ConvertStrToArray<ty, ty>(v1, (ty *)p1->d1, sz, [](ty &par) -> ty & { \
            return par;                                                              \
        });                                                                          \
    }

// Set struct field in array field
//  p1: parameter struct
//  s1: input name
//  v1: string encoded value
//  s2: expected name
//  d1: field name of array in struct
//  sz: array size in struct
//  f1: field to set
#define UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(p1, s1, v1, s2, d1, sz, f1)                                             \
    if (s1 == #s2) {                                                                                               \
        typedef std::remove_reference<decltype((p1->d1[0]))>::type element_type;                                   \
        typedef std::remove_reference<decltype((p1->d1[0].f1))>::type field_type;                                  \
        return ConvertStrToArray<element_type, field_type>(v1, p1->d1, sz, [](element_type &par) -> field_type & { \
            return par.f1;                                                                                         \
        });                                                                                                        \
    }

// clang-format off
mfxStatus UpdateVideoParam(const KVPair &kvStr, mfxVideoParam* videoParam) {
    mfxStatus sts = MFX_ERR_NOT_FOUND; // default if param is unknown

    const std::string param = kvStr.first;
    const std::string value = kvStr.second;

    // in below, first string is for the API (can be anything), second string is part of the mfxVideoParam definition
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, AllocId,                       AllocId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, AsyncDepth,                    AsyncDepth);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, Protected,                     Protected);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, IOPattern,                     IOPattern);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, NumExtParam,                   NumExtParam);

    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, LowPower,                      mfx.LowPower);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, BRCParamMultiplier,            mfx.BRCParamMultiplier);
    UPDATE_VIDEO_PARAM_FOURCC(videoParam, param, value, CodecId,                      mfx.CodecId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, CodecProfile,                  mfx.CodecProfile);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, CodecLevel,                    mfx.CodecLevel);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, NumThread,                     mfx.NumThread);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, TargetUsage,                   mfx.TargetUsage);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, GopPicSize,                    mfx.GopPicSize);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, GopRefDist,                    mfx.GopRefDist);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, GopOptFlag,                    mfx.GopOptFlag);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, IdrInterval,                   mfx.IdrInterval);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, RateControlMethod,             mfx.RateControlMethod);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, InitialDelayInKB,              mfx.InitialDelayInKB);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, QPI,                           mfx.QPI);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, Accuracy,                      mfx.Accuracy);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, BufferSizeInKB,                mfx.BufferSizeInKB);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, TargetKbps,                    mfx.TargetKbps);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, QPP,                           mfx.QPP);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, ICQQuality,                    mfx.ICQQuality);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, MaxKbps,                       mfx.MaxKbps);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, QPB,                           mfx.QPB);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, Convergence,                   mfx.Convergence);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, NumSlice,                      mfx.NumSlice);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, NumRefFrame,                   mfx.NumRefFrame);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, EncodedOrder,                  mfx.EncodedOrder);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, DecodedOrder,                  mfx.DecodedOrder);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, ExtendedPicStruct,             mfx.ExtendedPicStruct);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, TimeStampCalc,                 mfx.TimeStampCalc);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, SliceGroupsPresent,            mfx.SliceGroupsPresent);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, MaxDecFrameBuffering,          mfx.MaxDecFrameBuffering);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, EnableReallocRequest,          mfx.EnableReallocRequest);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, FilmGrain,                     mfx.FilmGrain);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, IgnoreLevelConstrain,          mfx.IgnoreLevelConstrain);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, SkipOutput,                    mfx.SkipOutput);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, JPEGChromaFormat,              mfx.JPEGChromaFormat);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, Rotation,                      mfx.Rotation);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, JPEGColorFormat,               mfx.JPEGColorFormat);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, InterleavedDec,                mfx.InterleavedDec);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, Interleaved,                   mfx.Interleaved);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, Quality,                       mfx.Quality);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, RestartInterval,               mfx.RestartInterval);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, ChannelId,                     mfx.FrameInfo.ChannelId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, BitDepthLuma,                  mfx.FrameInfo.BitDepthLuma);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, BitDepthChroma,                mfx.FrameInfo.BitDepthChroma);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, Shift,                         mfx.FrameInfo.Shift);
    UPDATE_VIDEO_PARAM_FOURCC(videoParam, param, value, FourCC,                       mfx.FrameInfo.FourCC);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, Width,                         mfx.FrameInfo.Width);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, Height,                        mfx.FrameInfo.Height);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, CropX,                         mfx.FrameInfo.CropX);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, CropY,                         mfx.FrameInfo.CropY);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, CropW,                         mfx.FrameInfo.CropW);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, CropH,                         mfx.FrameInfo.CropH);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, BufferSize,                    mfx.FrameInfo.BufferSize);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, FrameRateExtN,                 mfx.FrameInfo.FrameRateExtN);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, FrameRateExtD,                 mfx.FrameInfo.FrameRateExtD);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, AspectRatioW,                  mfx.FrameInfo.AspectRatioW);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, AspectRatioH,                  mfx.FrameInfo.AspectRatioH);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, PicStruct,                     mfx.FrameInfo.PicStruct);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, ChromaFormat,                  mfx.FrameInfo.ChromaFormat);

    // special handling for array types
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(videoParam, param, value, SamplingFactorH[],        mfx.SamplingFactorH, mfxU8, 4);
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(videoParam, param, value, SamplingFactorV[],        mfx.SamplingFactorV, mfxU8, 4);

    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, FrameId.TemporalId,            mfx.FrameInfo.FrameId.TemporalId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, FrameId.PriorityId,            mfx.FrameInfo.FrameId.PriorityId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, FrameId.DependencyId,          mfx.FrameInfo.FrameId.DependencyId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, FrameId.QualityId,             mfx.FrameInfo.FrameId.QualityId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, FrameId.ViewId,                mfx.FrameInfo.FrameId.ViewId);

    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.ChannelId,              vpp.In.ChannelId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.BitDepthLuma,           vpp.In.BitDepthLuma);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.BitDepthChroma,         vpp.In.BitDepthChroma);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.Shift,                  vpp.In.Shift);
    UPDATE_VIDEO_PARAM_FOURCC(videoParam, param, value, vpp.In.FourCC,                vpp.In.FourCC);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.Width,                  vpp.In.Width);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.Height,                 vpp.In.Height);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.CropX,                  vpp.In.CropX);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.CropY,                  vpp.In.CropY);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.CropW,                  vpp.In.CropW);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.CropH,                  vpp.In.CropH);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.BufferSize,             vpp.In.BufferSize);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.FrameRateExtN,          vpp.In.FrameRateExtN);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.FrameRateExtD,          vpp.In.FrameRateExtD);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.AspectRatioW,           vpp.In.AspectRatioW);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.AspectRatioH,           vpp.In.AspectRatioH);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.PicStruct,              vpp.In.PicStruct);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.ChromaFormat,           vpp.In.ChromaFormat);

    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.FrameId.TemporalId,     vpp.In.FrameId.TemporalId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.FrameId.PriorityId,     vpp.In.FrameId.PriorityId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.FrameId.DependencyId,   vpp.In.FrameId.DependencyId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.FrameId.QualityId,      vpp.In.FrameId.QualityId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.In.FrameId.ViewId,         vpp.In.FrameId.ViewId);

    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.ChannelId,             vpp.Out.ChannelId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.BitDepthLuma,          vpp.Out.BitDepthLuma);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.BitDepthChroma,        vpp.Out.BitDepthChroma);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.Shift,                 vpp.Out.Shift);
    UPDATE_VIDEO_PARAM_FOURCC(videoParam, param, value, vpp.Out.FourCC,               vpp.Out.FourCC);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.Width,                 vpp.Out.Width);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.Height,                vpp.Out.Height);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.CropX,                 vpp.Out.CropX);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.CropY,                 vpp.Out.CropY);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.CropW,                 vpp.Out.CropW);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.CropH,                 vpp.Out.CropH);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.BufferSize,            vpp.Out.BufferSize);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.FrameRateExtN,         vpp.Out.FrameRateExtN);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.FrameRateExtD,         vpp.Out.FrameRateExtD);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.AspectRatioW,          vpp.Out.AspectRatioW);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.AspectRatioH,          vpp.Out.AspectRatioH);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.PicStruct,             vpp.Out.PicStruct);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.ChromaFormat,          vpp.Out.ChromaFormat);

    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.FrameId.TemporalId,    vpp.Out.FrameId.TemporalId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.FrameId.PriorityId,    vpp.Out.FrameId.PriorityId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.FrameId.DependencyId,  vpp.Out.FrameId.DependencyId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.FrameId.QualityId,     vpp.Out.FrameId.QualityId);
    UPDATE_VIDEO_PARAM_VALUE(videoParam, param, value, vpp.Out.FrameId.ViewId,        vpp.Out.FrameId.ViewId);

    return sts;
}


// implementations of each extBuf update function

mfxStatus UpdateSingleExtBuf(const std::string &param, const std::string &value, mfxExtHEVCParam *eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, PicWidthInLumaSamples,     PicWidthInLumaSamples);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, PicHeightInLumaSamples,    PicHeightInLumaSamples);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, GeneralConstraintFlags,    GeneralConstraintFlags);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, SampleAdaptiveOffset,      SampleAdaptiveOffset);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, LCUSize,                   LCUSize);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string &param, const std::string &value, mfxExtCodingOption2 *eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, IntRefType,           IntRefType);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, IntRefCycleSize,      IntRefCycleSize);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, IntRefQPDelta,        IntRefQPDelta);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MaxFrameSize,         MaxFrameSize);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MaxSliceSize,         MaxSliceSize);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, BitrateLimit,         BitrateLimit);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MBBRC,                MBBRC);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ExtBRC,               ExtBRC);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, LookAheadDepth,       LookAheadDepth);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Trellis,              Trellis);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, RepeatPPS,            RepeatPPS);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, BRefType,             BRefType);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, AdaptiveI,            AdaptiveI);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, AdaptiveB,            AdaptiveB);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, LookAheadDS,          LookAheadDS);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumMbPerSlice,        NumMbPerSlice);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, SkipFrame,            SkipFrame);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MaxQPI,               MaxQPI);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MinQPI,               MinQPI);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MinQPP,               MinQPP);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MaxQPP,               MaxQPP);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MinQPB,               MinQPB);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MaxQPB,               MaxQPB);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, FixedFrameRate,       FixedFrameRate);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, DisableDeblockingIdc, DisableDeblockingIdc);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, DisableVUI,           DisableVUI);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, BufferingPeriodSEI,   BufferingPeriodSEI);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, EnableMAD,            EnableMAD);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, UseRawRef,            UseRawRef);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string &param, const std::string &value, mfxExtCodingOption *eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, RateDistortionOpt,    RateDistortionOpt);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MECostType,           MECostType);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MESearchType,         MESearchType);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, FramePicture,         FramePicture);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, CAVLC,                CAVLC);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, RecoveryPointSEI,     RecoveryPointSEI);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ViewOutput,           ViewOutput);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NalHrdConformance,    NalHrdConformance);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, SingleSeiNalUnit,     SingleSeiNalUnit);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, VuiVclHrdParameters,  VuiVclHrdParameters);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, RefPicListReordering, RefPicListReordering);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ResetRefList,         ResetRefList);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, RefPicMarkRep,        RefPicMarkRep);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, FieldOutput,          FieldOutput);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, IntraPredBlockSize,   IntraPredBlockSize);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, InterPredBlockSize,   InterPredBlockSize);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MVPrecision,          MVPrecision);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MaxDecFrameBuffering, MaxDecFrameBuffering);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, AUDelimiter,          AUDelimiter);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, PicTimingSEI,         PicTimingSEI);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, VuiNalHrdParameters,  VuiNalHrdParameters);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MVSearchWindow.x,    MVSearchWindow.x);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MVSearchWindow.y,    MVSearchWindow.y);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, EndOfStream,    EndOfStream);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, EndOfSequence,    EndOfSequence);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string &param, const std::string &value, mfxExtCodingOption3 *eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumSliceI,                      NumSliceI);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumSliceP,                      NumSliceP);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumSliceB,                      NumSliceB);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, WinBRCMaxAvgKbps,               WinBRCMaxAvgKbps);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, WinBRCSize,                     WinBRCSize);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, QVBRQuality,                    QVBRQuality);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, EnableMBQP,                     EnableMBQP);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, IntRefCycleDist,                IntRefCycleDist);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, DirectBiasAdjustment,           DirectBiasAdjustment);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, GlobalMotionBiasAdjustment,     GlobalMotionBiasAdjustment);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MVCostScalingFactor,            MVCostScalingFactor);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MBDisableSkipMap,               MBDisableSkipMap);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, WeightedPred,                   WeightedPred);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, WeightedBiPred,                 WeightedBiPred);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, AspectRatioInfoPresent,         AspectRatioInfoPresent);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, OverscanInfoPresent,            OverscanInfoPresent);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, OverscanAppropriate,            OverscanAppropriate);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TimingInfoPresent,              TimingInfoPresent);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, BitstreamRestriction,           BitstreamRestriction);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, LowDelayHrd,                    LowDelayHrd);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MotionVectorsOverPicBoundaries, MotionVectorsOverPicBoundaries);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ScenarioInfo,                   ScenarioInfo);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ContentInfo,                    ContentInfo);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, PRefType,                       PRefType);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, FadeDetection,                  FadeDetection);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, GPB,                            GPB);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MaxFrameSizeI,                  MaxFrameSizeI);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MaxFrameSizeP,                  MaxFrameSizeP);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, EnableQPOffset,                 EnableQPOffset);
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(eb, param, value, QPOffset[],                     QPOffset, mfxI16, 8);
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(eb, param, value, NumRefActiveP[],                NumRefActiveP, mfxI16, 8);
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(eb, param, value, NumRefActiveBL0[],              NumRefActiveBL0, mfxI16, 8);
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(eb, param, value, NumRefActiveBL1[],              NumRefActiveBL1, mfxI16, 8);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TransformSkip,                  TransformSkip);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TargetChromaFormatPlus1,        TargetChromaFormatPlus1);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TargetBitDepthLuma,             TargetBitDepthLuma);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TargetBitDepthChroma,           TargetBitDepthChroma);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, BRCPanicMode,                   BRCPanicMode);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, LowDelayBRC,                    LowDelayBRC);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, EnableMBForceIntra,             EnableMBForceIntra);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, AdaptiveMaxFrameSize,           AdaptiveMaxFrameSize);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, RepartitionCheckEnable,         RepartitionCheckEnable);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, EncodedUnitsInfo,               EncodedUnitsInfo);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, EnableNalUnitType,              EnableNalUnitType);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, AdaptiveLTR,                    AdaptiveLTR);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, AdaptiveCQM,                    AdaptiveCQM);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, AdaptiveRef,                    AdaptiveRef);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ExtBrcAdaptiveLTR,                    ExtBrcAdaptiveLTR);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string &param, const std::string &value, mfxExtVPPDoNotUse *eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumAlg, NumAlg);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string &param, const std::string &value, mfxExtVPPFrameRateConversion *eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Algorithm, Algorithm);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string &param, const std::string &value, mfxExtVPPImageStab *eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Mode, Mode);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string &param, const std::string &value, mfxExtMasteringDisplayColourVolume *eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, InsertPayloadToggle,               InsertPayloadToggle);
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(eb, param, value, DisplayPrimariesX[],               DisplayPrimariesX, mfxU16, 3);
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(eb, param, value, DisplayPrimariesY[],               DisplayPrimariesY, mfxU16, 3);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, WhitePointX,                       WhitePointX);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, WhitePointY,                       WhitePointY);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MaxDisplayMasteringLuminance,      MaxDisplayMasteringLuminance);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MinDisplayMasteringLuminance,      MinDisplayMasteringLuminance);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtContentLightLevelInfo* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, InsertPayloadToggle,          InsertPayloadToggle);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MaxContentLightLevel,         MaxContentLightLevel);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MaxPicAverageLightLevel,      MaxPicAverageLightLevel);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtAvcTemporalLayers* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, BaseLayerPID, BaseLayerPID);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Layer[].Scale, Layer, 8, Scale);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPPComposite* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Y,              Y);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, U,              U);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, V,              V);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumTiles,       NumTiles);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumInputStream, NumInputStream);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, R,              R);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, G,              G);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, B,              B);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPPVideoSignalInfo* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, In.TransferMatrix,  In.TransferMatrix);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, In.NominalRange,    In.NominalRange);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Out.TransferMatrix, Out.TransferMatrix);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Out.NominalRange,   Out.NominalRange);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TransferMatrix,     TransferMatrix);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NominalRange,       NominalRange);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPPDeinterlacing* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Mode,             Mode);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TelecinePattern,  TelecinePattern);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TelecineLocation, TelecineLocation);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtAVCRefLists* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumRefIdxL0Active, NumRefIdxL0Active);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumRefIdxL1Active, NumRefIdxL1Active);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, RefPicList0[].FrameOrder, RefPicList0, 32, FrameOrder);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, RefPicList0[].PicStruct, RefPicList0, 32, PicStruct);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, RefPicList1[].FrameOrder, RefPicList1, 32, FrameOrder);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, RefPicList1[].PicStruct, RefPicList1, 32, PicStruct);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPPFieldProcessing* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Mode,     Mode);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, InField,  InField);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, OutField, OutField);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtDecVideoProcessing* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, In.CropX,         In.CropX);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, In.CropY,         In.CropY);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, In.CropW,         In.CropW);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, In.CropH,         In.CropH);
    UPDATE_VIDEO_PARAM_FOURCC(eb, param, value, Out.FourCC,      Out.FourCC);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Out.ChromaFormat, Out.ChromaFormat);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Out.Width,        Out.Width);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Out.Height,       Out.Height);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Out.CropX,        Out.CropX);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Out.CropY,        Out.CropY);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Out.CropW,        Out.CropW);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Out.CropH,        Out.CropH);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtChromaLocInfo* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ChromaLocInfoPresentFlag,       ChromaLocInfoPresentFlag);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ChromaSampleLocTypeTopField,    ChromaSampleLocTypeTopField);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ChromaSampleLocTypeBottomField, ChromaSampleLocTypeBottomField);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtHEVCTiles* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumTileRows,    NumTileRows);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumTileColumns, NumTileColumns);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPPRotation* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Angle, Angle);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPPScaling* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ScalingMode, ScalingMode);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, InterpolationMethod, InterpolationMethod);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPPMirroring* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Type, Type);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPPColorFill* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Enable, Enable);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtColorConversion* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ChromaSiting, ChromaSiting);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVP9Segmentation* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumSegments,                NumSegments);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, SegmentIdBlockSize,         SegmentIdBlockSize);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumSegmentIdAlloc,          NumSegmentIdAlloc);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Segment[].FeatureEnabled, Segment, 8, FeatureEnabled);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Segment[].QIndexDelta, Segment, 8, QIndexDelta);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Segment[].LoopFilterLevelDelta, Segment, 8, LoopFilterLevelDelta);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Segment[].ReferenceFrame, Segment, 8, ReferenceFrame);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVP9TemporalLayers* eb) {
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Layer[].FrameRateScale, Layer, 8, FrameRateScale);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Layer[].TargetKbps, Layer, 8, TargetKbps);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string &param, const std::string &value, mfxExtAV1FilmGrainParam *eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, FilmGrainFlags,     FilmGrainFlags);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, GrainSeed,    GrainSeed);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, RefIdx,    RefIdx);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumYPoints,      NumYPoints);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumCbPoints,     NumCbPoints);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumCrPoints,     NumCrPoints);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, GrainScalingMinus8,     GrainScalingMinus8);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ArCoeffLag,     ArCoeffLag);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ArCoeffShiftMinus6,     ArCoeffShiftMinus6);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, GrainScaleShift,     GrainScaleShift);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, CbMult,     CbMult);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, CbLumaMult,     CbLumaMult);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, CbOffset,     CbOffset);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, CrMult,     CrMult);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, CrLumaMult,     CrLumaMult);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, CrOffset,     CrOffset);
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(eb, param, value, ArCoeffsYPlus128[],     ArCoeffsYPlus128, mfxU8, 24);
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(eb, param, value, ArCoeffsCbPlus128[],     ArCoeffsCbPlus128, mfxU8, 25);
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(eb, param, value, ArCoeffsCrPlus128[],     ArCoeffsCrPlus128, mfxU8, 25);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, PointY[].Value, PointY, 14, Value);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, PointY[].Scaling, PointY, 14, Scaling);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, PointCb[].Value, PointCb, 10, Value);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, PointCb[].Scaling, PointCb, 10, Scaling);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, PointCr[].Value, PointCr, 10, Value);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, PointCr[].Scaling, PointCr, 10, Scaling);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtAV1ResolutionParam* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, FrameWidth, FrameWidth);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, FrameHeight, FrameHeight);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtAV1Segmentation* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, SegmentIdBlockSize, SegmentIdBlockSize);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumSegmentIdAlloc, NumSegmentIdAlloc);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumSegments, NumSegments);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Segment[].FeatureEnabled, Segment, 8, FeatureEnabled);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Segment[].AltQIndex, Segment, 8, AltQIndex);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtAV1TileParam* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumTileRows, NumTileRows);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumTileColumns, NumTileColumns);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumTileGroups, NumTileGroups);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtAVCEncodedFrameInfo* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, FrameOrder, FrameOrder);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, PicStruct, PicStruct);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, LongTermIdx, LongTermIdx);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MAD, MAD);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, BRCPanicMode, BRCPanicMode);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, QP, QP);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, SecondFieldOffset, SecondFieldOffset);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, UsedRefListL0[].FrameOrder, UsedRefListL0, 32, FrameOrder);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, UsedRefListL0[].PicStruct, UsedRefListL0, 32, PicStruct);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, UsedRefListL0[].LongTermIdx, UsedRefListL0, 32, LongTermIdx);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, UsedRefListL1[].FrameOrder, UsedRefListL1, 32, FrameOrder);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, UsedRefListL1[].PicStruct, UsedRefListL1, 32, PicStruct);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, UsedRefListL1[].LongTermIdx, UsedRefListL1, 32, LongTermIdx);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtAVCRefListCtrl* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumRefIdxL0Active, NumRefIdxL0Active);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumRefIdxL1Active, NumRefIdxL1Active);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ApplyLongTermIdx, ApplyLongTermIdx);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, PreferredRefList[].FrameOrder, PreferredRefList, 32, FrameOrder);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, PreferredRefList[].PicStruct, PreferredRefList, 32, PicStruct);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, PreferredRefList[].ViewId, PreferredRefList, 32, ViewId);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, PreferredRefList[].LongTermIdx, PreferredRefList, 32, LongTermIdx);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, RejectedRefList[].FrameOrder, RejectedRefList, 16, FrameOrder);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, RejectedRefList[].PicStruct, RejectedRefList, 16, PicStruct);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, RejectedRefList[].ViewId, RejectedRefList, 16, ViewId);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, RejectedRefList[].LongTermIdx, RejectedRefList, 16, LongTermIdx);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, LongTermRefList[].FrameOrder, LongTermRefList, 16, FrameOrder);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, LongTermRefList[].PicStruct, LongTermRefList, 16, PicStruct);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, LongTermRefList[].ViewId, LongTermRefList, 16, ViewId);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, LongTermRefList[].LongTermIdx, LongTermRefList, 16, LongTermIdx);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtAVCRoundingOffset* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, EnableRoundingIntra, EnableRoundingIntra);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, RoundingOffsetIntra, RoundingOffsetIntra);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, EnableRoundingInter, EnableRoundingInter);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, RoundingOffsetInter, RoundingOffsetInter);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtEncodedSlicesInfo* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, SliceSizeOverflow, SliceSizeOverflow);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumSliceNonCopliant, NumSliceNonCopliant);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumEncodedSlice, NumEncodedSlice);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumSliceSizeAlloc, NumSliceSizeAlloc);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtHEVCRegion* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, RegionId, RegionId);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, RegionType, RegionType);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, RegionEncoding, RegionEncoding);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtInCrops* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Crops.Left, Crops.Left);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Crops.Top, Crops.Top);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Crops.Right, Crops.Right);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Crops.Bottom, Crops.Bottom);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtInsertHeaders* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, SPS, SPS);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, PPS, PPS);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtMVOverPicBoundaries* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, StickTop, StickTop);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, StickBottom, StickBottom);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, StickLeft, StickLeft);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, StickRight, StickRight);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVP9Param* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, FrameWidth, FrameWidth);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, FrameHeight, FrameHeight);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, WriteIVFHeaders, WriteIVFHeaders);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, QIndexDeltaLumaDC, QIndexDeltaLumaDC);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, QIndexDeltaChromaAC, QIndexDeltaChromaAC);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, QIndexDeltaChromaDC, QIndexDeltaChromaDC);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumTileRows, NumTileRows);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumTileColumns, NumTileColumns);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtTimeCode* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, DropFrameFlag, DropFrameFlag);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TimeCodeHours, TimeCodeHours);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TimeCodeMinutes, TimeCodeMinutes);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TimeCodeSeconds, TimeCodeSeconds);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TimeCodePictures, TimeCodePictures);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtMBQP* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Mode, Mode);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, BlockSize, BlockSize);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumQPAlloc, NumQPAlloc);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtCodingOptionSPSPPS* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, SPSBufSize, SPSBufSize);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, PPSBufSize, PPSBufSize);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, SPSId, SPSId);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, PPSId, PPSId);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtCodingOptionVPS* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, VPSId, VPSId);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, VPSBufSize, VPSBufSize);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVideoSignalInfo* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, VideoFormat, VideoFormat);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, VideoFullRange, VideoFullRange);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ColourDescriptionPresent, ColourDescriptionPresent);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ColourPrimaries, ColourPrimaries);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TransferCharacteristics, TransferCharacteristics);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MatrixCoefficients, MatrixCoefficients);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVppAuxData* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, SpatialComplexity, SpatialComplexity);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, TemporalComplexity, TemporalComplexity);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, PicStruct, PicStruct);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, SceneChangeRate, SceneChangeRate);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, RepeatedFrame, RepeatedFrame);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVppMctf* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, FilterStrength, FilterStrength);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtTemporalLayers* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumLayers, NumLayers);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, BaseLayerPID, BaseLayerPID);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtPartialBitstreamParam* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, BlockSize, BlockSize);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Granularity, Granularity);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtPredWeightTable* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, LumaLog2WeightDenom, LumaLog2WeightDenom);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ChromaLog2WeightDenom, ChromaLog2WeightDenom);
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(eb, param, value, LumaWeightFlag[], LumaWeightFlag, mfxU16, 2*32);
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(eb, param, value, ChromaWeightFlag[], ChromaWeightFlag, mfxU16, 2*32);
    UPDATE_VIDEO_PARAM_FLAT_ARRAY(eb, param, value, Weights[], Weights, mfxI16, 2*32*3*2);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtEncodedUnitsInfo* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumUnitsAlloc, NumUnitsAlloc);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumUnitsEncoded, NumUnitsEncoded);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtAV1BitstreamParam* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, WriteIVFHeaders, WriteIVFHeaders);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtEncoderROI* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumROI, NumROI);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ROIMode, ROIMode);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, ROI[].Left, ROI, 256, Left);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, ROI[].Top, ROI, 256, Top);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, ROI[].Right, ROI, 256, Right);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, ROI[].Bottom, ROI, 256, Bottom);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, ROI[].Priority, ROI, 256, Priority);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, ROI[].DeltaQP, ROI, 256, DeltaQP);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtDecodeErrorReport* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ErrorTypes, ErrorTypes);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtDecodedFrameInfo* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, FrameType, FrameType);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtEncoderCapability* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MBPerSec, MBPerSec);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtDeviceAffinityMask* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumSubDevices, NumSubDevices);
    UPDATE_VIDEO_PARAM_STRING(eb, param, value, DeviceID[], DeviceID, 128);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtDirtyRect* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumRect, NumRect);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Rect[].Left, Rect, 256, Left);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Rect[].Top, Rect, 256, Top);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Rect[].Right, Rect, 256, Right);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Rect[].Bottom, Rect, 256, Bottom);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtEncoderIPCMArea* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumArea, NumArea);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtEncoderResetOption* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, StartNewSequence, StartNewSequence);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtMBDisableSkipMap* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MapSize, MapSize);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtMBForceIntra* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, MapSize, MapSize);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtMoveRect* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumRect, NumRect);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Rect[].DestLeft, Rect, 256, DestLeft);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Rect[].DestTop, Rect, 256, DestTop);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Rect[].DestRight, Rect, 256, DestRight);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Rect[].DestBottom, Rect, 256, DestBottom);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Rect[].SourceLeft, Rect, 256, SourceLeft);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, Rect[].SourceTop, Rect, 256, SourceTop);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPPProcAmp* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Brightness, Brightness);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Contrast, Contrast);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Hue, Hue);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Saturation, Saturation);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtThreadsParam* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumThread, NumThread);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, SchedulingType, SchedulingType);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Priority, Priority);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPPDenoise* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, DenoiseFactor, DenoiseFactor);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPPDetail* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, DetailFactor, DetailFactor);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPPDoUse* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, NumAlg, NumAlg);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtHyperModeParam* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Mode, Mode);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPPDenoise2* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Mode, Mode);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, Strength, Strength);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtVPP3DLut* eb) {
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, ChannelMapping, ChannelMapping);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, BufferType, BufferType);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, SystemBuffer.Channel[].DataType, SystemBuffer.Channel, 3, DataType);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, SystemBuffer.Channel[].Size, SystemBuffer.Channel, 3, Size);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, VideoBuffer.DataType, VideoBuffer.DataType);
    UPDATE_VIDEO_PARAM_VALUE(eb, param, value, VideoBuffer.MemLayout, VideoBuffer.MemLayout);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

mfxStatus UpdateSingleExtBuf(const std::string& param, const std::string& value, mfxExtPictureTimingSEI* eb) {
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].ClockTimestampFlag, TimeStamp, 3, ClockTimestampFlag);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].CtType, TimeStamp, 3, CtType);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].NuitFieldBasedFlag, TimeStamp, 3, NuitFieldBasedFlag);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].CountingType, TimeStamp, 3, CountingType);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].FullTimestampFlag, TimeStamp, 3, FullTimestampFlag);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].DiscontinuityFlag, TimeStamp, 3, DiscontinuityFlag);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].CntDroppedFlag, TimeStamp, 3, CntDroppedFlag);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].NFrames, TimeStamp, 3, NFrames);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].SecondsFlag, TimeStamp, 3, SecondsFlag);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].MinutesFlag, TimeStamp, 3, MinutesFlag);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].HoursFlag, TimeStamp, 3, HoursFlag);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].SecondsValue, TimeStamp, 3, SecondsValue);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].MinutesValue, TimeStamp, 3, MinutesValue);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].HoursValue, TimeStamp, 3, HoursValue);
    UPDATE_VIDEO_PARAM_ARRAY_OF_STRUCT(eb, param, value, TimeStamp[].TimeOffset, TimeStamp, 3, TimeOffset);

    return MFX_ERR_INVALID_VIDEO_PARAM;
}

// check extBuf type and call the appropriate function
// need to add implementation for each supported mfxExt*** type
mfxStatus SetExtBufParam(mfxExtBuffer *extBufActual, KVPair &kvStrParsed) {
    switch (extBufActual->BufferId) {
        case MFX_EXTBUFF_CODING_OPTION2:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtCodingOption2 *>(extBufActual));
        case MFX_EXTBUFF_CODING_OPTION:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtCodingOption *>(extBufActual));
        case MFX_EXTBUFF_HEVC_PARAM:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtHEVCParam *>(extBufActual));
        case MFX_EXTBUFF_CODING_OPTION3:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtCodingOption3 *>(extBufActual));
        case MFX_EXTBUFF_VPP_DONOTUSE:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPDoNotUse *>(extBufActual));
        case MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPFrameRateConversion *>(extBufActual));
        case MFX_EXTBUFF_VPP_IMAGE_STABILIZATION:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPImageStab *>(extBufActual));
        case MFX_EXTBUFF_MASTERING_DISPLAY_COLOUR_VOLUME:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtMasteringDisplayColourVolume *>(extBufActual));
        case MFX_EXTBUFF_CONTENT_LIGHT_LEVEL_INFO:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtContentLightLevelInfo *>(extBufActual));
        case MFX_EXTBUFF_AVC_TEMPORAL_LAYERS:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtAvcTemporalLayers *>(extBufActual));
        case MFX_EXTBUFF_VPP_COMPOSITE:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPComposite *>(extBufActual));
        case MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPVideoSignalInfo *>(extBufActual));
        case MFX_EXTBUFF_VPP_DEINTERLACING:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPDeinterlacing *>(extBufActual));
        case MFX_EXTBUFF_AVC_REFLISTS:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtAVCRefLists *>(extBufActual));
        case MFX_EXTBUFF_VPP_FIELD_PROCESSING:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPFieldProcessing *>(extBufActual));
        case MFX_EXTBUFF_DEC_VIDEO_PROCESSING:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtDecVideoProcessing *>(extBufActual));
        case MFX_EXTBUFF_CHROMA_LOC_INFO:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtChromaLocInfo *>(extBufActual));
        case MFX_EXTBUFF_HEVC_TILES:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtHEVCTiles *>(extBufActual));
        case MFX_EXTBUFF_VPP_ROTATION:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPRotation *>(extBufActual));
        case MFX_EXTBUFF_VPP_SCALING:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPScaling *>(extBufActual));
        case MFX_EXTBUFF_VPP_MIRRORING:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPMirroring *>(extBufActual));
        case MFX_EXTBUFF_VPP_COLORFILL:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPColorFill *>(extBufActual));
        case MFX_EXTBUFF_VPP_COLOR_CONVERSION:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtColorConversion *>(extBufActual));
        case MFX_EXTBUFF_VP9_SEGMENTATION:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVP9Segmentation *>(extBufActual));
        case MFX_EXTBUFF_VP9_TEMPORAL_LAYERS:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVP9TemporalLayers *>(extBufActual));
        case MFX_EXTBUFF_AV1_FILM_GRAIN_PARAM:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtAV1FilmGrainParam *>(extBufActual));
        case MFX_EXTBUFF_AV1_RESOLUTION_PARAM:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtAV1ResolutionParam *>(extBufActual));
        case MFX_EXTBUFF_AV1_SEGMENTATION:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtAV1Segmentation *>(extBufActual));
        case MFX_EXTBUFF_AV1_TILE_PARAM:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtAV1TileParam *>(extBufActual));
        case MFX_EXTBUFF_ENCODED_FRAME_INFO:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtAVCEncodedFrameInfo *>(extBufActual));
        case MFX_EXTBUFF_HEVC_REFLIST_CTRL:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtAVCRefListCtrl *>(extBufActual));
        case MFX_EXTBUFF_AVC_ROUNDING_OFFSET:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtAVCRoundingOffset *>(extBufActual));
        case MFX_EXTBUFF_ENCODED_SLICES_INFO:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtEncodedSlicesInfo *>(extBufActual));
        case MFX_HEVC_REGION_SLICE:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtHEVCRegion *>(extBufActual));
        case MFX_EXTBUFF_CROPS:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtInCrops *>(extBufActual));
        case MFX_EXTBUFF_INSERT_HEADERS:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtInsertHeaders *>(extBufActual));
        case MFX_EXTBUFF_MV_OVER_PIC_BOUNDARIES:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtMVOverPicBoundaries *>(extBufActual));
        case MFX_EXTBUFF_VP9_PARAM:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVP9Param *>(extBufActual));
        case MFX_EXTBUFF_TIME_CODE:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtTimeCode *>(extBufActual));
        case MFX_EXTBUFF_MBQP:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtMBQP *>(extBufActual));
        case MFX_EXTBUFF_CODING_OPTION_SPSPPS:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtCodingOptionSPSPPS *>(extBufActual));
        case MFX_EXTBUFF_CODING_OPTION_VPS:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtCodingOptionVPS *>(extBufActual));
        case MFX_EXTBUFF_VIDEO_SIGNAL_INFO:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVideoSignalInfo *>(extBufActual));
        case MFX_EXTBUFF_VPP_AUXDATA:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVppAuxData *>(extBufActual));
        case MFX_EXTBUFF_VPP_MCTF:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVppMctf *>(extBufActual));
        case MFX_EXTBUFF_UNIVERSAL_TEMPORAL_LAYERS:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtTemporalLayers *>(extBufActual));
        case MFX_EXTBUFF_PARTIAL_BITSTREAM_PARAM:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtPartialBitstreamParam *>(extBufActual));
        case MFX_EXTBUFF_PRED_WEIGHT_TABLE:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtPredWeightTable *>(extBufActual));
        case MFX_EXTBUFF_ENCODED_UNITS_INFO:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtEncodedUnitsInfo *>(extBufActual));
        case MFX_EXTBUFF_AV1_BITSTREAM_PARAM:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtAV1BitstreamParam *>(extBufActual));
        case MFX_EXTBUFF_ENCODER_ROI:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtEncoderROI *>(extBufActual));
        case MFX_EXTBUFF_DECODE_ERROR_REPORT:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtDecodeErrorReport *>(extBufActual));
        case MFX_EXTBUFF_DECODED_FRAME_INFO:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtDecodedFrameInfo *>(extBufActual));
        case MFX_EXTBUFF_ENCODER_CAPABILITY:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtEncoderCapability *>(extBufActual));
        case MFX_EXTBUFF_DEVICE_AFFINITY_MASK:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtDeviceAffinityMask *>(extBufActual));
        case MFX_EXTBUFF_DIRTY_RECTANGLES:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtDirtyRect *>(extBufActual));
        case MFX_EXTBUFF_ENCODER_IPCM_AREA:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtEncoderIPCMArea *>(extBufActual));
        case MFX_EXTBUFF_ENCODER_RESET_OPTION:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtEncoderResetOption *>(extBufActual));
        case MFX_EXTBUFF_MB_DISABLE_SKIP_MAP:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtMBDisableSkipMap *>(extBufActual));
        case MFX_EXTBUFF_MB_FORCE_INTRA:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtMBForceIntra *>(extBufActual));
        case MFX_EXTBUFF_MOVING_RECTANGLES:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtMoveRect *>(extBufActual));
        case MFX_EXTBUFF_VPP_PROCAMP:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPProcAmp *>(extBufActual));
        case MFX_EXTBUFF_HYPER_MODE_PARAM:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtHyperModeParam *>(extBufActual));
        case MFX_EXTBUFF_THREADS_PARAM:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtThreadsParam *>(extBufActual));
        case MFX_EXTBUFF_VPP_3DLUT:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPP3DLut *>(extBufActual));
        case MFX_EXTBUFF_VPP_DENOISE:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPDenoise *>(extBufActual));
        case MFX_EXTBUFF_VPP_DENOISE2:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPDenoise2 *>(extBufActual));
        case MFX_EXTBUFF_VPP_DETAIL:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPDetail *>(extBufActual));
        case MFX_EXTBUFF_VPP_DOUSE:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtVPPDoUse *>(extBufActual));
        case MFX_EXTBUFF_PICTURE_TIMING_SEI:
            return UpdateSingleExtBuf(kvStrParsed.first, kvStrParsed.second, reinterpret_cast<mfxExtPictureTimingSEI *>(extBufActual));
        default:
            return MFX_ERR_NOT_FOUND;
    }

    return MFX_ERR_NOT_FOUND;
}

// clang-format on

} // namespace MFX_CONFIG_INTERFACE
