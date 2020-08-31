/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_CPU_SRC_CPU_FRAME_H_
#define SRC_CPU_SRC_CPU_FRAME_H_

#include "src/cpu_common.h"

// Implemented via AVFrame
class CpuFrame : public mfxFrameSurface1 {
public:
    CpuFrame() : m_refCount(0) {
        m_avframe = av_frame_alloc();

        *(mfxFrameSurface1*)this    = {};
        Version.Version             = MFX_FRAMESURFACE1_VERSION;
        FrameInterface              = &m_interface;
        m_interface.Context         = (mfxHDL*)this;
        m_interface.Version.Version = MFX_FRAMESURFACEINTERFACE_VERSION;
        m_interface.AddRef          = AddRef;
        m_interface.Release         = Release;
        m_interface.GetRefCounter   = GetRefCounter;
        m_interface.Map             = Map;
        m_interface.Unmap           = Unmap;
        m_interface.GetNativeHandle = GetNativeHandle;
        m_interface.GetDeviceHandle = GetDeviceHandle;
        m_interface.Synchronize     = Synchronize;
    }

    ~CpuFrame() {
        if (m_avframe) {
            av_frame_unref(m_avframe);
        }
    }

    static CpuFrame* TryCast(mfxFrameSurface1* surface) {
        if (surface && surface->FrameInterface &&
            surface->FrameInterface->Context &&
            surface->Version.Version >= MFX_FRAMESURFACE1_VERSION &&
            surface->FrameInterface->Map == Map) {
            return (CpuFrame*)surface->FrameInterface->Context;
        }
        else {
            return nullptr;
        }
    }

    AVFrame* GetAVFrame() {
        return m_avframe;
    }

    mfxStatus Allocate(mfxU32 FourCC, mfxU32 width, mfxU32 height) {
        m_avframe->width  = width;
        m_avframe->height = height;
        m_avframe->format = MFXFourCC2AVPixelFormat(FourCC);
        RET_IF_FALSE(m_avframe->format != AV_PIX_FMT_NONE,
                     MFX_ERR_INVALID_VIDEO_PARAM);
        RET_IF_FALSE(av_frame_get_buffer(m_avframe, 0) == 0,
                     MFX_ERR_MEMORY_ALLOC);
        return Update();
    }

    mfxStatus ImportAVFrame(AVFrame* avframe) {
        RET_IF_FALSE(avframe, MFX_ERR_NULL_PTR);
        RET_IF_FALSE(m_avframe == nullptr || avframe == m_avframe,
                     MFX_ERR_UNDEFINED_BEHAVIOR);
        m_avframe   = avframe;
        Info.Width  = avframe->width;
        Info.Height = avframe->height;
        Info.FourCC = AVPixelFormat2MFXFourCC(avframe->format);
        if (Info.FourCC == MFX_FOURCC_RGB4) {
            Data.B = avframe->data[0] + 0;
            Data.G = avframe->data[0] + 1;
            Data.R = avframe->data[0] + 2;
            Data.A = avframe->data[0] + 3;
        }
        else {
            Data.Y = avframe->data[0];
            Data.U = avframe->data[1];
            Data.V = avframe->data[2];
            Data.A = avframe->data[3];
        }
        Data.Pitch     = avframe->linesize[0];
        Data.TimeStamp = avframe->pts; // TODO(check units)
        // TODO(fill more fields)
        return MFX_ERR_NONE;
    }

    mfxStatus Update() {
        return ImportAVFrame(m_avframe);
    }

private:
    std::atomic<mfxU32> m_refCount; // TODO(we have C++11, correct?)
    AVFrame* m_avframe;
    mfxFrameSurfaceInterface m_interface;

    static mfxStatus AddRef(mfxFrameSurface1* surface);
    static mfxStatus Release(mfxFrameSurface1* surface);
    static mfxStatus GetRefCounter(mfxFrameSurface1* surface, mfxU32* counter);
    static mfxStatus Map(mfxFrameSurface1* surface, mfxU32 flags);
    static mfxStatus Unmap(mfxFrameSurface1* surface);
    static mfxStatus GetNativeHandle(mfxFrameSurface1* surface,
                                     mfxHDL* resource,
                                     mfxResourceType* resource_type);
    static mfxStatus GetDeviceHandle(mfxFrameSurface1* surface,
                                     mfxHDL* device_handle,
                                     mfxHandleType* device_type);
    static mfxStatus Synchronize(mfxFrameSurface1* surface, mfxU32 wait);
};

#endif // SRC_CPU_SRC_CPU_FRAME_H_
