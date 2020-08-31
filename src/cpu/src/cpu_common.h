/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_CPU_SRC_CPU_COMMON_H_
#define SRC_CPU_SRC_CPU_COMMON_H_

#include <chrono>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "vpl/mfxjpeg.h"
#include "vpl/mfxstructures.h"
#include "vpl/mfxvideo.h"

#define ENABLE_LIBAV_AUTO_THREADS

// TODO(m) do we need this?
#if !defined(WIN32) && !defined(memcpy_s)
    #define memcpy_s(dest, destsz, src, count) memcpy(dest, src, count)
#endif

// FFMPEG header
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libswscale/swscale.h"
}

// Debug messages
#if defined(DEBUG) || defined(_DEBUG)
    #include <iostream>

    #define VPL_DEBUG_MESSAGE(_MESSAGE)                                  \
        std::cout << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION__ \
                  << ": " << (_MESSAGE) << std::endl;
#else
    #define VPL_DEBUG_MESSAGE(_MESSAGE)
#endif

// ITT and console tracing
#ifdef VPL_ENABLE_CONSOLE_TRACING
    #include <iostream>
class TraceObject {
public:
    explicit TraceObject(const char* name) : m_name(name) {
        std::cout << "### entering " << name << std::endl;
    }
    ~TraceObject() {
        std::cout << "### leaving " << m_name << std::endl;
    }

protected:
    const char* m_name;
};

    #define VPL_TRACE(_NAME) TraceObject trace_object(_NAME)
#else
    #define VPL_TRACE(_NAME)
#endif
#define VPL_TRACE_FUNC VPL_TRACE(__FUNCTION__)

// Returns from current function if (value<0)
#define RET_ERROR(_VAR)                                                      \
    {                                                                        \
        auto _sts = _VAR;                                                    \
        /*VPL_DEBUG_MESSAGE("#info: " #_VAR " = " + std::to_string(_sts));*/ \
        if (_sts < 0) {                                                      \
            VPL_DEBUG_MESSAGE("Error " + std::to_string(_sts) +              \
                              " calling " #_VAR);                            \
            return _sts;                                                     \
        }                                                                    \
    }

// Returns from current function if (!value)
#define RET_IF_FALSE(_VAR, _ERR_CODE)             \
    {                                             \
        if (!(_VAR)) {                            \
            VPL_DEBUG_MESSAGE(#_VAR " is false"); \
            return _ERR_CODE;                     \
        }                                         \
    }

AVPixelFormat MFXFourCC2AVPixelFormat(uint32_t fourcc);
uint32_t AVPixelFormat2MFXFourCC(int format);

AVCodecID MFXCodecId_to_AVCodecID(mfxU32 CodecId);
mfxU32 AVCodecID_to_MFXCodecId(AVCodecID CodecId);

std::shared_ptr<AVFrame> GetAVFrameFromMfxSurface(mfxFrameSurface1* surface,
                                                  mfxFrameAllocator* allocator);

// copy image data from AVFrame to mfxFrameSurface1
mfxStatus AVFrame2mfxFrameSurface(mfxFrameSurface1* surface,
                                  AVFrame* frame,
                                  mfxFrameAllocator* allocator);

mfxStatus CheckFrameInfoCommon(mfxFrameInfo* info, mfxU32 codecId);
mfxStatus CheckFrameInfoCodecs(mfxFrameInfo* info, mfxU32 codecId);
mfxStatus CheckVideoParamCommon(mfxVideoParam* in);
#endif // SRC_CPU_SRC_CPU_COMMON_H_
