/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_frame.h"

// increase refCount on surface (+1)
mfxStatus CpuFrame::AddRef(mfxFrameSurface1* surface) {
    CpuFrame* cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);

    cpu_frame->m_refCount++;

    return MFX_ERR_NONE;
}

// decrease refCount on surface (-1)
mfxStatus CpuFrame::Release(mfxFrameSurface1* surface) {
    CpuFrame* cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);

    cpu_frame->m_refCount--;

    return MFX_ERR_NONE;
}

// return current refCount on surface
mfxStatus CpuFrame::GetRefCounter(mfxFrameSurface1* surface, mfxU32* counter) {
    CpuFrame* cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(counter, MFX_ERR_NULL_PTR);

    *counter = cpu_frame->m_refCount;

    // Add 1 to ref counter if m_avframe is locked (ref_count > 1)
    if (cpu_frame->m_avframe && cpu_frame->m_avframe->data[0] &&
        !av_frame_is_writable(cpu_frame->m_avframe)) {
        (*counter)++;
    }

    return MFX_ERR_NONE;
}

// map surface to system memory according to "flags"
// currently does nothing for system memory
mfxStatus CpuFrame::Map(mfxFrameSurface1* surface, mfxU32 flags) {
    CpuFrame* cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);

    //cpu_frame->Update();

    return MFX_ERR_NONE;
}

// unmap surface - no longer accessible to application
// currently does nothing for system memory
mfxStatus CpuFrame::Unmap(mfxFrameSurface1* surface) {
    CpuFrame* cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);

    return MFX_ERR_NONE;
}

// return native handle and type
mfxStatus CpuFrame::GetNativeHandle(mfxFrameSurface1* surface,
                                    mfxHDL* resource,
                                    mfxResourceType* resource_type) {
    CpuFrame* cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(resource && resource_type, MFX_ERR_NULL_PTR);

#if 0
    if (cpu_frame->m_avframe) {
        *resource_type = static_cast<mfxResourceType>(
            MFX_MAKEFOURCC('A', 'V', 'F', 'R')); // TODO(API)
        *resource = (mfxHDL)cpu_frame->m_avframe;
        return MFX_ERR_NONE;
    }
#endif

    return MFX_ERR_NOT_FOUND;
}

// return device handle and type
// not relevant for system memory
mfxStatus CpuFrame::GetDeviceHandle(mfxFrameSurface1* surface,
                                    mfxHDL* device_handle,
                                    mfxHandleType* device_type) {
    CpuFrame* cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);
    RET_IF_FALSE(device_handle && device_type, MFX_ERR_NULL_PTR);

    return MFX_ERR_NOT_FOUND;
}

// synchronize on surface after calling DecodeFrameAsync or VPP
// alternative to calling MFXCore_SyncOperation
mfxStatus CpuFrame::Synchronize(mfxFrameSurface1* surface, mfxU32 wait) {
    CpuFrame* cpu_frame = TryCast(surface);
    RET_IF_FALSE(cpu_frame, MFX_ERR_INVALID_HANDLE);

    // TO DO - VPL CPU ref is currently synchronous (Sync does nothing)
    // need to align sync behavior between legacy SyncOperation()
    //   and new sync on surfaces
    // probably need to track mapping of unique sync points to associated
    //   mfxFrameSurface1 surfaces, and sync here

    return MFX_ERR_NONE;
}
