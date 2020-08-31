/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_frame_pool.h"
#include <memory>
#include <utility>

mfxStatus CpuFramePool::Init(mfxU32 nPoolSize) {
    for (mfxU32 i = 0; i < nPoolSize; i++) {
        auto cpu_frame = std::make_unique<CpuFrame>();
        RET_IF_FALSE(cpu_frame->GetAVFrame(), MFX_ERR_MEMORY_ALLOC);
        m_surfaces.push_back(std::move(cpu_frame));
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuFramePool::Init(mfxU32 FourCC,
                             mfxU32 width,
                             mfxU32 height,
                             mfxU32 nPoolSize) {
    for (mfxU32 i = 0; i < nPoolSize; i++) {
        auto cpu_frame = std::make_unique<CpuFrame>();
        RET_ERROR(cpu_frame->Allocate(FourCC, width, height));
        m_surfaces.push_back(std::move(cpu_frame));
    }

    m_info.FourCC = FourCC;
    m_info.Width  = width;
    m_info.Height = height;

    return MFX_ERR_NONE;
}

// return free surface and set refCount to 1
mfxStatus CpuFramePool::GetFreeSurface(mfxFrameSurface1** surface) {
    RET_IF_FALSE(surface, MFX_ERR_NULL_PTR);
    *surface = nullptr;

    for (std::unique_ptr<CpuFrame>& surf : m_surfaces) {
        mfxU32 counter = 0xFFFFFFFF;
        surf->FrameInterface->GetRefCounter(surf.get(), &counter);

        if (!counter && !surf->Data.Locked) {
            *surface = surf.get();
            (*surface)->FrameInterface->AddRef(*surface);
            return MFX_ERR_NONE;
        }
    }

    // no free surface found in pool, create new one
    auto cpu_frame = std::make_unique<CpuFrame>();
    RET_IF_FALSE(cpu_frame && cpu_frame->GetAVFrame(), MFX_ERR_MEMORY_ALLOC);
    if (m_info.FourCC) {
        RET_ERROR(
            cpu_frame->Allocate(m_info.FourCC, m_info.Width, m_info.Height));
    }
    *surface = cpu_frame.get();
    (*surface)->FrameInterface->AddRef(*surface);
    m_surfaces.push_back(std::move(cpu_frame));

    return MFX_ERR_NONE;
}
