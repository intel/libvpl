/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <cstring>

#include "vpl/mfxvideo.h"

namespace oneapi {
namespace vpl {
namespace detail {

/// @brief This strucure holds context used in the mfxFrameInterface to map/unpam/sync the data
/// when surface is allocated by the application. This hack is needed to unite DX when user works with
/// internally and externally allocated surfaces.
typedef struct _surfCtx__ {
    mfxSession s; ///< MFX Session handle
    /// Syncronization point provided by the processing functio. Later this sync point is ingested
    /// into the context by Session class by using rude hack.
    mfxSyncPoint sp;
    mfxU32 cnt; ///< References counter
    void (*deleter)(mfxFrameSurface1* surface); ///< Allocator provided Data buffer deleter.
} surfCtx__;

inline void add_data_deleter(mfxFrameSurface1* surface,
                             void (*deleter)(mfxFrameSurface1* surface)) {
    surfCtx__* cnt = reinterpret_cast<surfCtx__*>(surface->FrameInterface->Context);
    cnt->deleter   = deleter;
}

inline mfxStatus AddRef__(mfxFrameSurface1* surface) {
    surfCtx__* cnt = reinterpret_cast<surfCtx__*>(surface->FrameInterface->Context);
    cnt->cnt++;
    return MFX_ERR_NONE;
}

inline mfxStatus Release__(mfxFrameSurface1* surface) {
    surfCtx__* cnt = reinterpret_cast<surfCtx__*>(surface->FrameInterface->Context);
    cnt->cnt--;

    // std::cout << "i have " << cnt->cnt << std::endl;
    if (0 == cnt->cnt) {
        // std::cout << "Patched Real destroy of the surface" << std::endl;
        if (cnt->deleter) {
            cnt->deleter(surface);
        }
        delete cnt;
        delete surface->FrameInterface;
        std::memset(surface, 0, sizeof(mfxFrameSurface1));
        delete surface;
    }
    return MFX_ERR_NONE;
}

inline mfxStatus GetRefCounter__(mfxFrameSurface1* surface, mfxU32* counter) {
    surfCtx__* cnt = reinterpret_cast<surfCtx__*>(surface->FrameInterface->Context);
    *counter       = cnt->cnt;
    return MFX_ERR_NONE;
}

inline mfxStatus Map__(mfxFrameSurface1* surface, mfxU32 flags) {
    surfCtx__* cnt = reinterpret_cast<surfCtx__*>(surface->FrameInterface->Context);
    if (flags & MFX_MAP_WRITE && cnt->cnt > 1) {
        return MFX_ERR_LOCK_MEMORY;
    }
    return MFX_ERR_NONE;
}

inline mfxStatus Unmap__(mfxFrameSurface1* surface) {
    return MFX_ERR_NONE;
}

inline mfxStatus GetNativeHandle__(mfxFrameSurface1* surface,
                                   mfxHDL* resource,
                                   mfxResourceType* resource_type) {
    return MFX_ERR_UNSUPPORTED;
}

inline mfxStatus GetDeviceHandle__(mfxFrameSurface1* surface,
                                   mfxHDL* device_handle,
                                   mfxHandleType* device_type) {
    return MFX_ERR_UNSUPPORTED;
}

inline mfxStatus Synchronize__(mfxFrameSurface1* surface, mfxU32 wait) {
    surfCtx__* cnt = reinterpret_cast<surfCtx__*>(surface->FrameInterface->Context);
    if (cnt->sp && cnt->s) {
        detail::c_api_invoker e(detail::default_checker,
                                MFXVideoCORE_SyncOperation,
                                cnt->s,
                                cnt->sp,
                                wait);
    }
    return MFX_ERR_NONE;
}

inline void OnComplete__(mfxStatus sts) {
    // not supported
}

inline mfxStatus QueryInterface__(mfxFrameSurface1* surface, mfxGUID guid, mfxHDL* iface) {
    return MFX_ERR_UNSUPPORTED;
}


inline void deleter_Y(mfxFrameSurface1* data) {
    delete[] data->Data.Y;
}

inline void deleter_B(mfxFrameSurface1* data) {
    delete[] data->Data.B;
}

inline auto alloc_i420(const frame_info& info, mfxFrameSurface1* data) {
    auto [width, height] = info.get_frame_size();

    mfxU32 bytes     = width * height + (width >> 1) * (height >> 1) + (width >> 1) * (height >> 1);
    mfxU8* rawbuffer = new mfxU8[bytes];

    data->Info       = info();
    data->Data.Y     = rawbuffer;
    data->Data.U     = data->Data.Y + width * height;
    data->Data.V     = data->Data.U + ((width / 2) * (height / 2));
    data->Data.Pitch = width;

    return deleter_Y;
}

inline auto alloc_bgra(const frame_info& info, mfxFrameSurface1* data) {
    auto [width, height] = info.get_frame_size();

    mfxU32 bytes     = width * height * 4;
    mfxU8* rawbuffer = new mfxU8[bytes];

    data->Info       = info();
    data->Data.B     = rawbuffer;
    data->Data.G     = data->Data.B + 1;
    data->Data.R     = data->Data.G + 1;
    data->Data.A     = data->Data.R + 1;
    data->Data.Pitch = width * 4;

    return deleter_B;
}

} // namespace detail
} // namespace vpl
} // namespace oneapi
