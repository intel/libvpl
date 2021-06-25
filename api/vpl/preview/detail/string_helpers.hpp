/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <iostream>
#include <string>
#include <utility>

#include "vpl/preview/defs.hpp"
#include "vpl/mfxstructures.h"

namespace oneapi {
namespace vpl {
namespace detail {

constexpr unsigned int INTENT = 4;

template <typename T>
const T& space(int space, std::ostream& out, const T& arg) {
    for (int i = 0; i < space; i++) {
        out << ' ';
    }
    return arg;
}

inline std::string Boolean2String(uint16_t code) {
    if (0 == code) {
        return ("False");
    }
    return (std::string("True"));
}

inline std::string FourCC2String(uint32_t code) {
    if (0 != code) {
        const char* b = (const char*)&code;
        return (std::string(b, 4));
    }
    return (std::string("Unset"));
}

inline std::string NotSpecifyed0(uint16_t code) {
    if (0 == code) {
        return (std::string("Not Specifyed"));
    }
    return (std::to_string(code));
}

inline std::string IOPattern2String(uint16_t code) {
    std::string iop;
    bool carry = false;
    bool isset = false;
    if (0 == code) {
        return (std::string("Unset"));
    }
    if (code & MFX_IOPATTERN_IN_VIDEO_MEMORY) {
        iop += "In Video";
        carry = true;
        isset = true;
    }
    if (code & MFX_IOPATTERN_IN_SYSTEM_MEMORY) {
        if (carry) {
            iop += " & ";
        }
        iop += "In System";
        carry = true;
        isset = true;
    }
    if (code & MFX_IOPATTERN_OUT_VIDEO_MEMORY) {
        if (carry) {
            iop += " & ";
        }
        iop += "Out Video";
        carry = true;
        isset = true;
    }
    if (code & MFX_IOPATTERN_OUT_SYSTEM_MEMORY) {
        if (carry) {
            iop += " & ";
        }
        iop += "Out Syatem";
        isset = true;
    }
    if (isset) {
        iop += " Memory";
    }
    return (iop);
}

inline std::string TriState2String(uint16_t code) {
    switch (code) {
        case MFX_CODINGOPTION_ON:
            return (std::string("ON"));
        case MFX_CODINGOPTION_OFF:
            return (std::string("OFF"));
        case MFX_CODINGOPTION_ADAPTIVE:
            return (std::string("Adaptive"));
        default:
            return (std::string("Unset"));
    }
    return (std::string("Unset"));
}

inline std::string PicStruct2String(uint16_t code) {
    switch (code) {
        case MFX_PICSTRUCT_PROGRESSIVE:
            return (std::string("Progressive Picture"));
        case MFX_PICSTRUCT_FIELD_TFF:
            return (std::string("Top field"));
        case MFX_PICSTRUCT_FIELD_BFF:
            return (std::string("Bottom field"));
        case MFX_PICSTRUCT_FIELD_REPEATED:
            return (std::string("First field repeated"));
        case MFX_PICSTRUCT_FRAME_DOUBLING:
            return (std::string("Double the frame"));
        case MFX_PICSTRUCT_FRAME_TRIPLING:
            return (std::string("Triple the frame"));
        case MFX_PICSTRUCT_FIELD_SINGLE:
            return (std::string("Single field"));
        case MFX_PICSTRUCT_FIELD_TOP:
            return (std::string("Top field in a picture"));
        case MFX_PICSTRUCT_FIELD_BOTTOM:
            return (std::string("Bottom field in a picture"));
        case MFX_PICSTRUCT_FIELD_PAIRED_PREV:
            return (std::string("Paired with previouse field"));
        case MFX_PICSTRUCT_FIELD_PAIRED_NEXT:
            return (std::string("Paired with nect field"));
        case MFX_PICSTRUCT_UNKNOWN:
        default:
            return (std::string("Unset"));
    }
    return (std::string("Unset"));
}

inline std::string ChromaFormat2String(uint16_t code) {
    switch (code) {
        case MFX_CHROMAFORMAT_MONOCHROME:
            return (std::string("Monochrome"));
        case MFX_CHROMAFORMAT_YUV420:
            return (std::string("4:2:0"));
        case MFX_CHROMAFORMAT_YUV422:
            return (std::string("4:2:2"));
        case MFX_CHROMAFORMAT_YUV444:
            return (std::string("4:4:4"));
        case MFX_CHROMAFORMAT_YUV411:
            return (std::string("4:1:1"));
        case MFX_CHROMAFORMAT_YUV422V:
            return (std::string("4:2:2 vertical"));
        default:
            return (std::string("Unset"));
    }
    return (std::string("Unset"));
}

inline std::string TimeStampCalc2String(uint16_t code) {
    switch (code) {
        case MFX_TIMESTAMPCALC_TELECINE:
            return (std::string("Telecine"));
        default:
            return (std::string("Not specifyed"));
    }
    return (std::string("Not specifyed"));
}

inline std::string GopOptFlag2String(uint16_t code) {
    switch (code) {
        case MFX_GOP_CLOSED:
            return (std::string("Closed"));
        case MFX_GOP_STRICT:
            return (std::string("Strict"));
    }
    return (std::string("Value is out of possible values"));
}

inline std::string RateControlMethod2String(uint16_t code) {
    switch (code) {
        case MFX_RATECONTROL_CBR:
            return (std::string("CBR"));
        case MFX_RATECONTROL_VBR:
            return (std::string("VBR"));
        case MFX_RATECONTROL_CQP:
            return (std::string("CQP"));
        case MFX_RATECONTROL_AVBR:
            return (std::string("AVBR"));
        case MFX_RATECONTROL_LA:
            return (std::string("LA"));
        case MFX_RATECONTROL_ICQ:
            return (std::string("ICQ"));
        case MFX_RATECONTROL_VCM:
            return (std::string("VCM"));
        case MFX_RATECONTROL_LA_ICQ:
            return (std::string("LA ICQ"));
        case MFX_RATECONTROL_LA_HRD:
            return (std::string("LA HRD"));
        case MFX_RATECONTROL_QVBR:
            return (std::string("QVBR"));
        default:
            return (std::string("Unknown"));
    }
    return (std::string("Not specifyed"));
}

inline std::string MemType2String(uint16_t code) {
    std::string res;
    bool carry = false;
    if (code & MFX_MEMTYPE_INTERNAL_FRAME) {
        res += "Internal frame";
        carry = true;
    }
    else if (code & MFX_MEMTYPE_EXTERNAL_FRAME) {
        res += "External frame";
        carry = true;
    }
    else if (code & MFX_MEMTYPE_EXPORT_FRAME) {
        res += "Export frame";
        carry = true;
    }

    if (code & MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET) {
        res += carry ? " " : "";
        res += "in dec video memory target";
    }
    else if (code & MFX_MEMTYPE_VIDEO_MEMORY_PROCESSOR_TARGET) {
        res += carry ? " " : "";
        res += "in VPP video memory target";
    }
    else if (code & MFX_MEMTYPE_SYSTEM_MEMORY) {
        res += carry ? " " : "";
        res += "in system memory";
    }
    else if (code & MFX_MEMTYPE_VIDEO_MEMORY_ENCODER_TARGET) {
        res += carry ? " " : "";
        res += "in enc video memory target";
    }
    else {
        carry = false;
    }

    if (code & MFX_MEMTYPE_FROM_ENCODE) {
        res += carry ? " " : "";
        res += "alocated by enc";
    }
    else if (code & MFX_MEMTYPE_FROM_DECODE) {
        res += carry ? " " : "";
        res += "alocated by dec";
    }
    else if (code & MFX_MEMTYPE_FROM_VPPIN) {
        res += carry ? " " : "";
        res += "alocated by vpp for in";
    }
    else if (code & MFX_MEMTYPE_FROM_VPPOUT) {
        res += carry ? " " : "";
        res += "alocated by vpp for out";
    }
    if (res.size() == 0)
        return (std::string("Invalid"));
    return (res);
}

inline std::string TimeStamp2String(uint64_t tms) {
    if (tms == (uint64_t)MFX_TIMESTAMP_UNKNOWN)
        return (std::string("Unknown"));
    return (std::to_string(tms));
}

inline std::string TimeStamp2String(int64_t tms) {
    if (tms == (int64_t)MFX_TIMESTAMP_UNKNOWN)
        return (std::string("Unknown"));
    return (std::to_string(tms));
}

inline std::string TimeStamp2String(uint16_t code) {
    if (code == MFX_FRAMEDATA_ORIGINAL_TIMESTAMP)
        return (std::string("Original timestamp."));
    return ("Calculated timestamp");
}

inline std::string Corruption2String(uint16_t code) {
    switch (code) {
        case MFX_CORRUPTION_MINOR:
            return (std::string("Minor"));
        case MFX_CORRUPTION_MAJOR:
            return (std::string("Major"));
        case MFX_CORRUPTION_ABSENT_TOP_FIELD:
            return (std::string("Top field corrupted"));
        case MFX_CORRUPTION_ABSENT_BOTTOM_FIELD:
            return (std::string("Bottom field corrupted"));
        case MFX_CORRUPTION_REFERENCE_FRAME:
            return (std::string("Corrupted reference frame"));
        case MFX_CORRUPTION_REFERENCE_LIST:
            return (std::string("corrupted reference list"));
        default:
            return (std::string("No corruption"));
    }
    return (std::string("Not specifyed"));
}

inline std::string ResourceType2String(mfxResourceType code) {
    switch (code) {
        case MFX_RESOURCE_SYSTEM_SURFACE:
            return (std::string("System buffer"));
        case MFX_RESOURCE_VA_SURFACE:
            return (std::string("VA Surface"));
        case MFX_RESOURCE_VA_BUFFER:
            return (std::string("VA buffer"));
        case MFX_RESOURCE_DX9_SURFACE:
            return (std::string("DX9 Surface"));
        case MFX_RESOURCE_DX11_TEXTURE:
            return (std::string("DX11 Texture"));
        case MFX_RESOURCE_DX12_RESOURCE:
            return (std::string("DX12 Resource"));
        case MFX_RESOURCE_DMA_RESOURCE:
            return (std::string("DMA buffer"));
        default:
            return (std::string("Unknown resource type"));
    }
    return (std::string("Not specifyed"));
}

inline std::string ImplType2String(mfxImplType code) {
    switch (code) {
        case MFX_IMPL_TYPE_SOFTWARE:
            return (std::string("Software"));
        case MFX_IMPL_TYPE_HARDWARE:
            return (std::string("Hardware"));
        default:
            return (std::string("Unknown type"));
    }
    return (std::string("Not specifyed"));
}

inline std::string AccelerationMode2String(mfxAccelerationMode code) {
    switch (code) {
        case MFX_ACCEL_MODE_NA:
            return (std::string("No HW acceleration"));
        case MFX_ACCEL_MODE_VIA_D3D9:
            return (std::string("HW acceleration via Microsoft* Direct3D9*"));
        case MFX_ACCEL_MODE_VIA_D3D11:
            return (std::string("HW acceleration via Microsoft* Direct3D11*"));
        case MFX_ACCEL_MODE_VIA_VAAPI:
            return (std::string("HW acceleration via Linux* VA-API"));
        default:
            return (std::string("Unknown resource type"));
    }
    return (std::string("Not specifyed"));
}

inline std::string component2String(component c) {
    switch (c) {
        case component::decoder:
            return (std::string("Decoder"));
        case component::encoder:
            return (std::string("Encoder"));
        case component::vpp:
            return (std::string("VPP"));
        case component::decoder_vpp:
            return (std::string("Decoder+VPP"));
        default:
            return (std::string("Unknown component"));
    }
    return (std::string("Unknown component"));
}

inline std::string status2Str(status s) {
    switch (s) {
        case status::Ok:
            return (std::string("Ok"));
            break;
        case status::ExecutionInProgress:
            return (std::string("ExecutionInProgress"));
            break;
        case status::DeviceBusy:
            return (std::string("DeviceBusy"));
            break;
        case status::VideoParamChanged:
            return (std::string("VideoParamChanged"));
            break;
        case status::PartialAcceleration:
            return (std::string("PartialAcceleration"));
            break;
        case status::IncompartibleVideoParam:
            return (std::string("IncompartibleVideoParam"));
            break;
        case status::ValueNotChanged:
            return (std::string("ValueNotChanged"));
            break;
        case status::OutOfRange:
            return (std::string("OutOfRange"));
            break;
        case status::TaskWorking:
            return (std::string("TaskWorking"));
            break;
        case status::TaskBusy:
            return (std::string("TaskBusy"));
            break;
        case status::FilterSkipped:
            return (std::string("FilterSkipped"));
            break;
        case status::PartialOutput:
            return (std::string("PartialOutput"));
            break;
        case status::NotEnoughData:
            return (std::string("NotEnoughData"));
            break;
        case status::NotEnoughSurface:
            return (std::string("NotEnoughSurface"));
            break;
        case status::NotEnoughBuffer:
            return (std::string("NotEnoughBuffer"));
            break;
        case status::EndOfStreamReached:
            return (std::string("EndOfStreamReached"));
            break;
        case status::Unknown:
        default:
            return (std::string("Unknown"));
            break;
    }
}

} // namespace detail
} // namespace vpl
} // namespace oneapi
