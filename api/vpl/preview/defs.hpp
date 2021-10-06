/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <iostream>
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
    Ok                      = MFX_ERR_NONE,
    ExecutionInProgress     = MFX_WRN_IN_EXECUTION,
    DeviceBusy              = MFX_WRN_DEVICE_BUSY,
    VideoParamChanged       = MFX_WRN_VIDEO_PARAM_CHANGED,
    PartialAcceleration     = MFX_WRN_PARTIAL_ACCELERATION,
    IncompartibleVideoParam = MFX_WRN_INCOMPATIBLE_VIDEO_PARAM,
    ValueNotChanged         = MFX_WRN_VALUE_NOT_CHANGED,
    OutOfRange              = MFX_WRN_OUT_OF_RANGE,
    TaskWorking             = MFX_TASK_WORKING,
    TaskBusy                = MFX_TASK_BUSY,
    FilterSkipped           = MFX_WRN_FILTER_SKIPPED,
    PartialOutput           = MFX_ERR_NONE_PARTIAL_OUTPUT,
    AllocTimeoutExpired     = MFX_WRN_ALLOC_TIMEOUT_EXPIRED,
    NotEnoughData           = MFX_ERR_MORE_DATA,
    NotEnoughSurface        = MFX_ERR_MORE_SURFACE,
    NotEnoughBuffer         = MFX_ERR_NOT_ENOUGH_BUFFER,
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

inline std::ostream& operator<<(std::ostream& out, const codec_format_fourcc& p) {
    out << char(((uint32_t)p >> 0) & 0x00FF);
    out << char(((uint32_t)p >> 8) & 0x00FF);
    out << char(((uint32_t)p >> 16) & 0x00FF);
    out << char(((uint32_t)p >> 24) & 0x00FF);
    return out;
}

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
    i210       = MFX_FOURCC_I210,
    i422       = MFX_FOURCC_I422,
    bgrp       = MFX_FOURCC_BGRP,
};

inline std::ostream& operator<<(std::ostream& out, const color_format_fourcc& p) {
    out << char(((uint32_t)p >> 0) & 0x00FF);
    out << char(((uint32_t)p >> 8) & 0x00FF);
    out << char(((uint32_t)p >> 16) & 0x00FF);
    out << char(((uint32_t)p >> 24) & 0x00FF);
    return out;
}


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
    io_device_memory  = in_device_memory | out_device_memory,
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

inline std::ostream &operator<<(std::ostream &out, const implementation_via &r) {
    switch(r) {
        case implementation_via::any:
            out << "any";
            break;
        case implementation_via::d3d9:
            out << "d3d9";
            break;
        case implementation_via::d3d11:
            out << "d3d11";
            break;
        case implementation_via::vaapi:
            out << "vaapi";
            break;
        case implementation_via::hddlunite:
            out << "hddlunite";
            break;
        case implementation_via::none:
            out << "none";
            break;
    }
    return out;
}


enum class resource_type : uint32_t {
    system_surface = MFX_RESOURCE_SYSTEM_SURFACE,
    va_surface_ptr = MFX_RESOURCE_VA_SURFACE_PTR,
    va_buffer_ptr  = MFX_RESOURCE_VA_BUFFER_PTR,
    dx9_surface    = MFX_RESOURCE_DX9_SURFACE,
    dx11_texture   = MFX_RESOURCE_DX11_TEXTURE,
    dx12_resource  = MFX_RESOURCE_DX12_RESOURCE,
    dma_resource   = MFX_RESOURCE_DMA_RESOURCE,
};

inline std::ostream &operator<<(std::ostream &out, const resource_type &r) {
    switch(r) {
        case resource_type::system_surface:
            out << "System surface";
            break;
        case resource_type::va_surface_ptr:
            out << "VASurfaceID pointer";
            break;
        case resource_type::va_buffer_ptr:
            out << "VASurfaceID pointer";
            break;
        case resource_type::dx9_surface:
            out << "DX9 Texture";
            break;
        case resource_type::dx11_texture:
            out << "DX11 Texture";
            break;
        case resource_type::dx12_resource:
            out << "DX12 Resource";
            break;
        case resource_type::dma_resource:
            out << "DMA buffer file descriptor";
            break;
        default:
            out << "Unknown";
            break;
    }
    return out;
}

enum class handle_type : uint32_t {
    d3d9_device_manager       = MFX_HANDLE_D3D9_DEVICE_MANAGER,
    d3d11_device_manager      = MFX_HANDLE_D3D11_DEVICE,
    va_display                = MFX_HANDLE_VA_DISPLAY,
    va_config_id              = MFX_HANDLE_VA_CONFIG_ID,
    va_context_id             = MFX_HANDLE_VA_CONTEXT_ID,
    cm_device                 = MFX_HANDLE_CM_DEVICE,
    hddlunite_workloadcontext = MFX_HANDLE_HDDLUNITE_WORKLOADCONTEXT,
};

inline std::ostream &operator<<(std::ostream &out, const handle_type &r) {
    switch(r) {
        case handle_type::d3d9_device_manager:
            out << "d3d9_device_manager";
            break;
        case handle_type::d3d11_device_manager:
            out << "d3d11_device_manager";
            break;
        case handle_type::va_display:
            out << "va_display";
            break;
        case handle_type::va_config_id:
            out << "va_config_id";
            break;
        case handle_type::cm_device:
            out << "cm_device";
            break;
        case handle_type::hddlunite_workloadcontext:
            out << "hddlunite_workloadcontext";
            break;
        default:
            out << "Unknown";
            break;
    }
    return out;
}

enum class memory_type : uint16_t {
    video_memory_decoder_target   = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET,
    video_memory_processor_target = MFX_MEMTYPE_VIDEO_MEMORY_PROCESSOR_TARGET,
    system_memory                 = MFX_MEMTYPE_SYSTEM_MEMORY,
    from_encode                   = MFX_MEMTYPE_FROM_ENCODE,
    from_decode                   = MFX_MEMTYPE_FROM_DECODE,
    from_vppin                    = MFX_MEMTYPE_FROM_VPPIN,
    from_vppout                   = MFX_MEMTYPE_FROM_VPPOUT,
    internal_frame                = MFX_MEMTYPE_INTERNAL_FRAME,
    external_frame                = MFX_MEMTYPE_EXTERNAL_FRAME,
    export_frame                  = MFX_MEMTYPE_EXPORT_FRAME,
    video_memory_encoder_target   = MFX_MEMTYPE_VIDEO_MEMORY_ENCODER_TARGET,
};

ENABLE_BIT_OPERATORS_WITH_ENUM(memory_type);

inline bool isVideoMemory(memory_type mem_type) {
    if ((mem_type & memory_type::video_memory_decoder_target) ==
                   memory_type::video_memory_decoder_target) {
                       return true;
    } else if ((mem_type & memory_type::video_memory_encoder_target) ==
                   memory_type::video_memory_encoder_target) {
                       return true;
    } else if ((mem_type & memory_type::video_memory_processor_target) ==
                   memory_type::video_memory_processor_target) {
                       return true;
    }
    return false;
}

inline bool isSystemMemory(memory_type mem_type) {
    return (memory_type::system_memory == (mem_type & memory_type::system_memory));
}

enum class pool_alloction_policy : uint32_t {
    optimal       = MFX_ALLOCATION_OPTIMAL,
    unlimited     = MFX_ALLOCATION_UNLIMITED,
    limited       = MFX_ALLOCATION_LIMITED,
};

inline std::ostream &operator<<(std::ostream &out, const pool_alloction_policy &r) {
    switch(r) {
        case pool_alloction_policy::optimal:
            out << "Optimal";
            break;
        case pool_alloction_policy::unlimited:
            out << "Unlimited";
            break;
        case pool_alloction_policy::limited:
            out << "Limited";
            break;
    }
    return out;
}

enum class coding_option : uint16_t {
    unknown  = MFX_CODINGOPTION_UNKNOWN,
    on       = MFX_CODINGOPTION_ON,
    off      = MFX_CODINGOPTION_OFF,
    adaptive = MFX_CODINGOPTION_ADAPTIVE,
};

enum class skip_frame : uint16_t {
    no_skip        = MFX_SKIPFRAME_NO_SKIP,
    insert_dummy   = MFX_SKIPFRAME_INSERT_DUMMY,
    insert_nothing = MFX_SKIPFRAME_INSERT_NOTHING,
    brc_only       = MFX_SKIPFRAME_BRC_ONLY,
};

enum class media_adapter_type : uint16_t {
    unknown    = MFX_MEDIA_UNKNOWN,
    integrated = MFX_MEDIA_INTEGRATED,
    discrete   = MFX_MEDIA_DISCRETE,
};

inline std::ostream &operator<<(std::ostream &out, const media_adapter_type &r) {
    switch(r) {
        case media_adapter_type::unknown:
            out << "Unknown";
            break;
        case media_adapter_type::integrated:
            out << "Integrated";
            break;
        case media_adapter_type::discrete:
            out << "Discrete";
            break;
    }
    return out;
}

enum class implementation_type : uint32_t {
    sw = MFX_IMPL_TYPE_SOFTWARE,
    hw = MFX_IMPL_TYPE_HARDWARE,
};

inline std::ostream &operator<<(std::ostream &out, const implementation_type &r) {
    switch(r) {
        case implementation_type::sw:
            out << "SW";
            break;
        case implementation_type::hw:
            out << "HW";
            break;
    }
    return out;
}

} // namespace vpl
} // namespace oneapi
