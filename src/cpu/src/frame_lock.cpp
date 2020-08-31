/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/frame_lock.h"
#include "src/cpu_frame.h"

FrameLock::FrameLock()
        : m_data(nullptr),
          m_surface(nullptr),
          m_allocator(nullptr),
          m_newapi(false),
          m_avframe(nullptr),
          mem_id(0) {}

FrameLock::~FrameLock() {
    Unlock();
    if (m_avframe) {
        av_frame_free(&m_avframe);
    }
}

mfxStatus FrameLock::Lock(mfxFrameSurface1 *surface,
                          mfxU32 flags,
                          mfxFrameAllocator *allocator) {
    Unlock();

    m_surface   = surface;
    m_allocator = allocator;
    m_newapi    = surface->Version.Version >= MFX_FRAMESURFACE1_VERSION &&
               surface->FrameInterface;
    if (allocator && allocator->pthis) {
        mem_id = surface->Data.MemId;
        RET_ERROR(allocator->Lock(allocator->pthis, mem_id, &m_locked_data));
        m_data = &m_locked_data;
    }
    else {
        if (m_newapi) {
            RET_ERROR(surface->FrameInterface->Map(surface, flags));
            surface->FrameInterface->AddRef(surface);
        }
        else {
            surface->Data.Locked++; // TODO(make atomic)
        }
        m_data = &surface->Data;
    }

    return MFX_ERR_NONE;
}

void FrameLock::Unlock() {
    VPL_TRACE_FUNC;
    if (m_data) {
        if (m_allocator && m_allocator->pthis) {
            m_allocator->Unlock(m_allocator->pthis, mem_id, m_data);
        }
        else if (m_surface) {
            if (m_newapi) {
                m_surface->FrameInterface->Unmap(m_surface);
                m_surface->FrameInterface->Release(m_surface);
            }
            else {
                m_surface->Data.Locked--; // TODO(make atomic)
            }
        }
        m_data = nullptr;
    }
}

mfxFrameData *FrameLock::GetData() {
    return m_data;
}

AVFrame *FrameLock::GetAVFrame(mfxFrameSurface1 *surface,
                               mfxU32 flags,
                               mfxFrameAllocator *allocator) {
    VPL_TRACE_FUNC;
    RET_IF_FALSE(surface, nullptr);

    // Try cast to CpuFrame and get existent AVFrame
    CpuFrame *dst_frame = CpuFrame::TryCast(surface);
    if (dst_frame) {
        AVFrame *avframe = dst_frame->GetAVFrame();
        if (avframe) {
            return avframe;
        }
    }

    // Lock and convert to AVFrame
    RET_IF_FALSE(Lock(surface, flags, allocator) >= 0, nullptr);

    if (!m_avframe) {
        m_avframe = av_frame_alloc();
    }
    RET_IF_FALSE(m_avframe, nullptr);

    mfxFrameInfo *info = &m_surface->Info;
    m_avframe->format  = MFXFourCC2AVPixelFormat(info->FourCC);
    m_avframe->width   = info->Width;
    m_avframe->height  = info->Height;
    if (info->FourCC == MFX_FOURCC_RGB4) {
        m_avframe->data[0] = m_data->B;
    }
    else {
        m_avframe->data[0] = m_data->Y;
        m_avframe->data[1] = m_data->U;
        m_avframe->data[2] = m_data->V;
        m_avframe->data[3] = m_data->A;
    }
    m_avframe->linesize[0] = m_data->Pitch;
    switch (info->FourCC) {
        case MFX_FOURCC_I420:
        case MFX_FOURCC_I010:
            m_avframe->linesize[1] = m_data->Pitch / 2;
            m_avframe->linesize[2] = m_data->Pitch / 2;
            break;
        case MFX_FOURCC_NV12:
            m_avframe->linesize[1] = m_data->Pitch;
            break;
        case MFX_FOURCC_YUY2:
        case MFX_FOURCC_RGB4:
            break;
        default:
            RET_IF_FALSE(!"Unsupported format", nullptr);
    }

    return m_avframe;
}
