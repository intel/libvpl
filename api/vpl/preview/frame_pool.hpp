/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <memory>
#include <stack>
#include <utility>
#include <vector>

#include "vpl/preview/frame_surface.hpp"
#include "vpl/preview/video_param.hpp"

#include "vpl/preview/detail/frame_interface.hpp"

namespace oneapi {
namespace vpl {

/// @brief Temporal class to allocate and manage pool of the surfaces.
/// @todo remove during migration to the API 2.1
class temporal_frame_allocator {
public:
    /// @brief Type of the allocated surfaces.
    using ptr_type = std::unique_ptr<frame_surface>;

    /// @brief Default ctor
    temporal_frame_allocator() : info_() {}

    /// @brief Update class instance with the frame information which is used to allocate proper
    /// buffer for surface data.
    /// Use this method only once. This class has no protection to handle changes of the frame info.
    /// @todo Handle frame info change during reset operation.
    /// @param[in] info Frame info
    void attach_frame_info(frame_info info) {
        info_ = info;
    }

    /// @brief Returns available surface.
    /// @return Pointer to the allocated surface.
    ptr_type acquire() {
        mfxFrameSurface1 *surf_data = new mfxFrameSurface1;
        std::memset(surf_data, 0, sizeof(mfxFrameSurface1));

        surf_data->FrameInterface          = new mfxFrameSurfaceInterface;
        surf_data->FrameInterface->Context = new detail::surfCtx__;
        std::memset(surf_data->FrameInterface->Context, 0, sizeof(detail::surfCtx__));

        switch (info_.get_FourCC()) {
            case color_format_fourcc::i420:
                detail::add_data_deleter(surf_data, detail::alloc_i420(info_, surf_data));
                break;
            case color_format_fourcc::bgra:
                detail::add_data_deleter(surf_data, detail::alloc_bgra(info_, surf_data));
                break;
            default:
                throw base_exception(MFX_ERR_NOT_IMPLEMENTED);
        }

        surf_data->FrameInterface->AddRef          = detail::AddRef__;
        surf_data->FrameInterface->Release         = detail::Release__;
        surf_data->FrameInterface->GetRefCounter   = detail::GetRefCounter__;
        surf_data->FrameInterface->Map             = detail::Map__;
        surf_data->FrameInterface->Unmap           = detail::Unmap__;
        surf_data->FrameInterface->GetNativeHandle = detail::GetNativeHandle__;
        surf_data->FrameInterface->GetDeviceHandle = detail::GetDeviceHandle__;
        surf_data->FrameInterface->Synchronize     = detail::Synchronize__;
        surf_data->FrameInterface->OnComplete      = detail::OnComplete__;
        surf_data->FrameInterface->QueryInterface  = detail::QueryInterface__;

        return std::make_unique<frame_surface>(surf_data, true);
    }

protected:
    /// @brief Frame information for the allocation
    frame_info info_;
};

} // namespace vpl
} // namespace oneapi
