/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

#include "vpl/mfxstructures.h"

namespace oneapi {
namespace vpl {

#define ENABLE_BIT_OPERATORS_WITH_ENUM(Enum) \
inline Enum operator|(Enum lhs, Enum rhs) { \
    using underlying = typename std::underlying_type<Enum>::type; \
    return static_cast<Enum>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs)); \
} \
inline Enum operator&(Enum lhs, Enum rhs) { \
    using underlying = typename std::underlying_type<Enum>::type; \
    return static_cast<Enum>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs)); \
}

/// @brief Asyncroniouse operation status
enum class component {
    decoder, ///! Decoder component.
    encoder, ///! Encoder component.
    vpp, ///! VPP component.
    decoder_vpp, ///! decode+vvpp component.
    unknown, ///! decode+vvpp component.
};

/// @brief Asyncroniouse operation status
enum class async_op_status {
    ready, ///! Asyncroniouse operation complited and data is ready.
    timeout, ///! Asyncroniouse operation is in the progress and data is not yet ready.
    aborted, ///! Asyncroniouse operation aborted.
    unknown, ///! Asyncroniouse operation status unknown.
    cancelled ///! Asyncroniouse operation cancelled due to issues raised during schedulling.
};

enum class status : int {
    Ok                      = 0,
    ExecutionInProgress     = 1,
    DeviceBusy              = 2,
    VideoParamChanged       = 3,
    PartialAcceleration     = 4,
    IncompartibleVideoParam = 5,
    ValueNotChanged         = 6,
    OutOfRange              = 7,
    TaskWorking             = 8,
    TaskBusy                = 9,
    FilterSkipped           = 10,
    PartialOutput           = 12,
    NotEnoughData           = -10,
    NotEnoughSurface        = -11,
    NotEnoughBuffer         = -5,
    EndOfStreamReached      = -100,
    Unknown                 = (std::numeric_limits<int>::min)()
};

// C API enums

enum class codec_format_fourcc : uint32_t {
    avc     = MFX_CODEC_AVC,
    hevc    = MFX_CODEC_HEVC,
    mpeg2   = MFX_CODEC_MPEG2,
    vc1     = MFX_CODEC_VC1,
    capture = MFX_CODEC_CAPTURE,
    vp9     = MFX_CODEC_VP9,
    av1     = MFX_CODEC_AV1
};

enum class color_format_fourcc : uint32_t {
    nv12       = MFX_FOURCC_NV12,
    yv12       = MFX_FOURCC_YV12,
    nv16       = MFX_FOURCC_NV16,
    yuy2       = MFX_FOURCC_YUY2,
    rgb465     = MFX_FOURCC_RGB565,
    rgbp       = MFX_FOURCC_RGBP,
    rgb3       = MFX_FOURCC_RGB3,
    bgra       = MFX_FOURCC_RGB4, // not a bug
    p8         = MFX_FOURCC_P8,
    p8_texture = MFX_FOURCC_P8_TEXTURE,
    p010       = MFX_FOURCC_P010,
    p016       = MFX_FOURCC_P016,
    p210       = MFX_FOURCC_P210,
    bgr4       = MFX_FOURCC_BGR4,
    a2rgb10    = MFX_FOURCC_A2RGB10,
    argb16     = MFX_FOURCC_ARGB16,
    abgr16     = MFX_FOURCC_ABGR16,
    r16        = MFX_FOURCC_R16,
    ayuv       = MFX_FOURCC_AYUV,
    ayuv_rgb4  = MFX_FOURCC_AYUV_RGB4,
    uyvy       = MFX_FOURCC_UYVY,
    y210       = MFX_FOURCC_Y210,
    y410       = MFX_FOURCC_Y410,
    y216       = MFX_FOURCC_Y216,
    y416       = MFX_FOURCC_Y416,
    nv21       = MFX_FOURCC_NV21,
    i420       = MFX_FOURCC_IYUV,
    i010       = MFX_FOURCC_I010,
    bgrp       = MFX_FOURCC_BGRP,
};

enum class chroma_format_idc : uint32_t {
    monochrome = MFX_CHROMAFORMAT_MONOCHROME,
    yuv420     = MFX_CHROMAFORMAT_YUV420,
    yuv422     = MFX_CHROMAFORMAT_YUV422,
    yuv444     = MFX_CHROMAFORMAT_YUV444,
    yuv400     = MFX_CHROMAFORMAT_YUV400,
    yuv411     = MFX_CHROMAFORMAT_YUV411,
    yuv422h    = MFX_CHROMAFORMAT_YUV422H,
    yuv422v    = MFX_CHROMAFORMAT_YUV422V
};

enum class target_usage : uint32_t {
    t1           = MFX_TARGETUSAGE_1,
    t2           = MFX_TARGETUSAGE_2,
    t3           = MFX_TARGETUSAGE_3,
    t4           = MFX_TARGETUSAGE_4,
    t5           = MFX_TARGETUSAGE_5,
    t6           = MFX_TARGETUSAGE_6,
    t7           = MFX_TARGETUSAGE_7,
    unknown      = MFX_TARGETUSAGE_UNKNOWN,
    best_quality = t1,
    balanced     = t4,
    best_speed   = t7,
};

enum class rate_control_method : uint32_t {
    cbr    = MFX_RATECONTROL_CBR,
    vbr    = MFX_RATECONTROL_VBR,
    cqp    = MFX_RATECONTROL_CQP,
    avbr   = MFX_RATECONTROL_AVBR,
    la     = MFX_RATECONTROL_LA,
    icq    = MFX_RATECONTROL_ICQ,
    vcm    = MFX_RATECONTROL_VCM,
    la_icq = MFX_RATECONTROL_LA_ICQ,
    la_hrd = MFX_RATECONTROL_LA_HRD,
    qvbr   = MFX_RATECONTROL_QVBR
};

enum class io_pattern : uint32_t {
    in_device_memory  = MFX_IOPATTERN_IN_VIDEO_MEMORY,
    in_system_memory  = MFX_IOPATTERN_IN_SYSTEM_MEMORY,
    out_device_memory = MFX_IOPATTERN_OUT_VIDEO_MEMORY,
    out_system_memory = MFX_IOPATTERN_OUT_SYSTEM_MEMORY,
    io_system_memory  = in_system_memory | out_system_memory,
};
ENABLE_BIT_OPERATORS_WITH_ENUM(io_pattern);

enum class pic_struct : uint16_t {
    unknown     = MFX_PICSTRUCT_UNKNOWN,
    progressive = MFX_PICSTRUCT_PROGRESSIVE,
    field_tff   = MFX_PICSTRUCT_FIELD_TFF,
    field_bff   = MFX_PICSTRUCT_FIELD_BFF,

    field_repeated = MFX_PICSTRUCT_FIELD_REPEATED,
    frame_doubling = MFX_PICSTRUCT_FRAME_DOUBLING,
    frame_tripling = MFX_PICSTRUCT_FRAME_TRIPLING,

    field_single      = MFX_PICSTRUCT_FIELD_SINGLE,
    field_top         = MFX_PICSTRUCT_FIELD_TOP,
    field_bottom      = MFX_PICSTRUCT_FIELD_BOTTOM,
    field_paired_prev = MFX_PICSTRUCT_FIELD_PAIRED_PREV,
    field_paired_next = MFX_PICSTRUCT_FIELD_PAIRED_NEXT,
};
ENABLE_BIT_OPERATORS_WITH_ENUM(pic_struct);

enum class memory_access : uint32_t {
    read       = MFX_MAP_READ,
    write      = MFX_MAP_WRITE,
    read_write = MFX_MAP_READ_WRITE,
    nowait     = MFX_MAP_NOWAIT
};

enum class hevc_nal_unit_type : uint16_t {
    unknown    = MFX_HEVC_NALU_TYPE_UNKNOWN,
    trail_n    = MFX_HEVC_NALU_TYPE_TRAIL_N,
    trail_r    = MFX_HEVC_NALU_TYPE_TRAIL_R,
    radl_n     = MFX_HEVC_NALU_TYPE_RADL_N,
    radl_r     = MFX_HEVC_NALU_TYPE_RADL_R,
    rasl_n     = MFX_HEVC_NALU_TYPE_RASL_N,
    rasl_r     = MFX_HEVC_NALU_TYPE_RASL_R,
    idr_w_radl = MFX_HEVC_NALU_TYPE_IDR_W_RADL,
    idr_n_lp   = MFX_HEVC_NALU_TYPE_IDR_N_LP,
    cra_nut    = MFX_HEVC_NALU_TYPE_CRA_NUT,
};

enum class frame_type : uint16_t {
    unknown = MFX_FRAMETYPE_UNKNOWN,
    i       = MFX_FRAMETYPE_I,
    p       = MFX_FRAMETYPE_P,
    b       = MFX_FRAMETYPE_B,
    s       = MFX_FRAMETYPE_S,
    ref     = MFX_FRAMETYPE_REF,
    idr     = MFX_FRAMETYPE_IDR,
    xi      = MFX_FRAMETYPE_xI,
    xp      = MFX_FRAMETYPE_xP,
    xb      = MFX_FRAMETYPE_xB,
    xs      = MFX_FRAMETYPE_xS,
    xref    = MFX_FRAMETYPE_xREF,
    xidr    = MFX_FRAMETYPE_xIDR,
};

ENABLE_BIT_OPERATORS_WITH_ENUM(frame_type);

static status mfxstatus_to_onevplstatus(mfxStatus s) {
    switch (s) {
        case MFX_ERR_NONE:
            return status::Ok;
        case MFX_WRN_IN_EXECUTION:
            return status::ExecutionInProgress;
        case MFX_WRN_DEVICE_BUSY:
            return status::DeviceBusy;
        case MFX_WRN_VIDEO_PARAM_CHANGED:
            return status::VideoParamChanged;
        case MFX_WRN_PARTIAL_ACCELERATION:
            return status::PartialAcceleration;
        case MFX_WRN_INCOMPATIBLE_VIDEO_PARAM:
            return status::IncompartibleVideoParam;
        case MFX_WRN_VALUE_NOT_CHANGED:
            return status::ValueNotChanged;
        case MFX_WRN_OUT_OF_RANGE:
            return status::OutOfRange;
        case MFX_TASK_WORKING:
            return status::TaskWorking;
        case MFX_TASK_BUSY:
            return status::TaskBusy;
        case MFX_WRN_FILTER_SKIPPED:
            return status::FilterSkipped;
        case MFX_ERR_NONE_PARTIAL_OUTPUT:
            return status::PartialOutput;
        //// Errors but they need to be treat as positive status in some cases.
        case MFX_ERR_NOT_ENOUGH_BUFFER:
            return status::NotEnoughBuffer;
        case MFX_ERR_MORE_DATA:
            return status::NotEnoughData;
        case MFX_ERR_MORE_SURFACE:
            return status::NotEnoughSurface;
        default:
            return status::Unknown;
    }
}

enum class implementation : uint32_t {
    automatic        = MFX_IMPL_AUTO,
    unsupported      = MFX_IMPL_UNSUPPORTED,
    software         = MFX_IMPL_SOFTWARE,
    hardware         = MFX_IMPL_HARDWARE,
    auto_any         = MFX_IMPL_AUTO_ANY,
    hardware_any     = MFX_IMPL_HARDWARE_ANY,
    hardware_2       = MFX_IMPL_HARDWARE2,
    hardware_3       = MFX_IMPL_HARDWARE3,
    hardware_4       = MFX_IMPL_HARDWARE4,
    hardware_runtime = MFX_IMPL_RUNTIME,
};

enum class implementation_via : uint32_t {
    any       = MFX_IMPL_VIA_ANY,
    d3d9      = MFX_IMPL_VIA_D3D9,
    d3d11     = MFX_IMPL_VIA_D3D11,
    vaapi     = MFX_IMPL_VIA_VAAPI,
    hddlunite = MFX_IMPL_VIA_HDDLUNITE,
    none      = 0
};

enum class handle_type : uint32_t {
    d3d9_device_manager       = MFX_HANDLE_D3D9_DEVICE_MANAGER,
    d3d11_device_manager      = MFX_HANDLE_D3D11_DEVICE,
    va_display                = MFX_HANDLE_VA_DISPLAY,
    va_config_id              = MFX_HANDLE_VA_CONFIG_ID,
    va_context_id             = MFX_HANDLE_VA_CONTEXT_ID,
    cm_device                 = MFX_HANDLE_CM_DEVICE,
    hddlunite_workloadcontext = MFX_HANDLE_HDDLUNITE_WORKLOADCONTEXT,
};

} // namespace vpl
} // namespace oneapi
