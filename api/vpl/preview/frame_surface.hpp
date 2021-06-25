/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <chrono>
#include <string>
#include <utility>
#include <memory>

#include "vpl/mfxstructures.h"

#include "vpl/preview/defs.hpp"
#include "vpl/preview/exception.hpp"
#include "vpl/preview/video_param.hpp"

#include "vpl/preview/detail/sdk_callable.hpp"

namespace oneapi {
namespace vpl {

/// @brief Manages lifecycle of the surface with the frame data. This class works on top of the mfxFrameSurface1 object,
/// which provides interface to access the data and has an internal reference counting mechanism.
class frame_surface : public std::enable_shared_from_this<frame_surface> {
public:
    /// @brief Default dtor
    frame_surface() : surface_(nullptr), lazy_sync_(false) {}

    /// @brief Creates object on top of mfxFrameSurface1 object.
    /// Increments mfxFrameSurface1 reference counter value.
    /// @param[in] surface Pointer to the mfxFrameSurface1 object
    /// @param[in] lazy_sync Temporal flag indicating that lazy sync technique must be used.
    /// @todo Remove flag with API 2.1 support
    explicit frame_surface(mfxFrameSurface1* surface, bool lazy_sync = false)
            : surface_(surface),
              lazy_sync_(lazy_sync) {
        detail::c_api_invoker(detail::default_checker,
                                surface_->FrameInterface->AddRef,
                                surface_);
    }

    /// @brief Copy ctor.
    /// Increments mfxFrameSurface1 reference counter value.
    /// @param[in] other another object to use as data source
    frame_surface(const frame_surface& other) {
        surface_   = other.surface_;
        lazy_sync_ = other.lazy_sync_;
        detail::c_api_invoker(detail::default_checker,
                                surface_->FrameInterface->AddRef,
                                surface_);
    }

    /// @brief Move ctor.
    /// mfxFrameSurface1 reference counter value isn't incremented
    /// @param[in] other another object to use as data source
    frame_surface(frame_surface&& other) {
        lazy_sync_ = other.lazy_sync_;
        surface_   = std::move(other.surface_);
    }

    /// @brief Copy operator.
    /// Increments mfxFrameSurface1 reference counter value.
    /// @param[in] other another object to use as data source
    /// @returns Reference to this object
    frame_surface& operator=(const frame_surface& other) {
        surface_   = other.surface_;
        lazy_sync_ = other.lazy_sync_;
        detail::c_api_invoker(detail::default_checker,
                                surface_->FrameInterface->AddRef,
                                surface_);
        return *this;
    }

    /// @brief Dtor.
    /// Decrements mfxFrameSurface1 reference counter value.
    virtual ~frame_surface() {
        if (surface_) {
            if (surface_->FrameInterface) {
                if (surface_->FrameInterface->Synchronize) {
                }
            }
            else {
            }
        }
        else {
        }

        if (surface_) {
            detail::c_api_invoker(detail::default_checker,
                                    surface_->FrameInterface->Release,
                                    surface_);
        }
        else {
        }
    }

    /// @brief Inject mfxFrameSurface1 object to take care of it. This is temporal method until VPL RT will support all
    /// functions for the internal memory allocation
    /// @todo Check reference counter correctness.
    /// @param[in] surface mfxFrameSurface1 surface to use.
    /// @param[in] n_times Reference counter increment.
    /// @param[in] lazy_sync Do lazy sync or not.
    void inject(mfxFrameSurface1* surface, unsigned int n_times, bool lazy_sync = false) {
        if (!surface_) {
            uint32_t c;
            surface_   = surface;
            lazy_sync_ = lazy_sync;
            surface_->FrameInterface->GetRefCounter(surface_, &c);
            for (unsigned int i = 0; i < n_times; i++) {
                detail::c_api_invoker(detail::default_checker,
                                        surface_->FrameInterface->AddRef,
                                        surface_);
            }
            surface_->FrameInterface->GetRefCounter(surface_, &c);
        }
    }

    /// @brief Indefinetely wait for operation completion.
    void wait() {
        detail::c_api_invoker(detail::default_checker,
                                surface_->FrameInterface->Synchronize,
                                surface_,
                                MFX_INFINITE);
    }

    /// @brief Waits for the operation completion. Waits for the result to become available. Blocks until specified
    /// timeout_duration has elapsed or the result becomes available, whichever comes first. Returns value identifying
    /// the state of the result.
    /// @param timeout_duration Maximum duration to block for.
    /// @return Wait ststus.
    template <class Rep, class Period>
    async_op_status wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const {
        async_op_status surf_sts;
        auto wait_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeout_duration);

        detail::c_api_invoker e({ [](mfxStatus s) {
                                    switch (s) {
                                        case MFX_ERR_NONE:
                                            return false;
                                        case MFX_WRN_IN_EXECUTION:
                                            return false;
                                        case MFX_ERR_ABORTED:
                                            return false;
                                        case MFX_ERR_UNKNOWN:
                                            return false;
                                        default:
                                            break;
                                    }

                                    bool ret = (s < 0) ? true : false;
                                    return ret;
                                } },
                                surface_->FrameInterface->Synchronize,
                                surface_,
                                (uint32_t)wait_ms.count());

        switch (e.sts_) {
            case MFX_ERR_NONE:
                surf_sts = async_op_status::ready;
                break;
            case MFX_WRN_IN_EXECUTION:
                surf_sts = async_op_status::timeout;
                break;
            case MFX_ERR_ABORTED:
                surf_sts = async_op_status::aborted;
                break;
            case MFX_ERR_UNKNOWN:
                surf_sts = async_op_status::unknown;
                break;
            default:
                throw base_exception(e.sts_);
        }
        return surf_sts;
    }

    /// @brief Provide frame information.
    /// @return Return instance of frame_info class
    frame_info get_frame_info() {
        return frame_info(surface_->Info);
    }

    /// @brief Maps data to the system memory.
    /// @param flags Data access flag: read or write.
    /// @return Pair of pointers to the surface info structure and surface data strucuture in the system memory
    auto map(memory_access flags) {
        wait();
        detail::c_api_invoker(detail::default_checker,
                                surface_->FrameInterface->Map,
                                surface_,
                                (mfxMemoryFlags)flags);
        return std::pair(frame_info(surface_->Info), frame_data(surface_->Data));
    }

    /// @brief Unmaps data to the system memory.
    void unmap() {
        detail::c_api_invoker(detail::default_checker, surface_->FrameInterface->Unmap, surface_);
    }

    /// @brief Provides native surface handle of the surface.
    /// @return Pair of native surface handle and its type
    auto get_native_handle() {
        void* resource;
        mfxResourceType resource_type;
        detail::c_api_invoker(detail::default_checker,
                                surface_->FrameInterface->GetNativeHandle,
                                surface_,
                                &resource,
                                &resource_type);

        return std::pair(resource, resource_type);
    }

    /// @brief Provides native device handle of the surface.
    /// @return Pair of native device handle and its type
    auto get_device_handle() {
        void* device_handle;
        mfxHandleType device_type;
        detail::c_api_invoker(detail::default_checker,
                                surface_->FrameInterface->GetDeviceHandle,
                                surface_,
                                &device_handle,
                                &device_type);
        return std::pair(device_handle, device_type);
    }

    /// @brief Provides current reference counter value.
    /// @return reference counter value.
    uint32_t get_ref_counter() {
        uint32_t counter;
        detail::c_api_invoker(detail::default_checker,
                                surface_->FrameInterface->GetRefCounter,
                                surface_,
                                &counter);
        return counter;
    }

    /// @brief Provides pointer to the raw data.
    /// @return Pointer to the raw data.
    mfxFrameSurface1* get_raw_ptr() {
        return surface_;
    }

    /// @brief Temporal method to assotiate externally allocated surface with sync point generated
    /// by the processing function.
    /// @param[in] session Session handle.
    /// @param[in] sp sync point.
    void associate_context(mfxSession session, mfxSyncPoint sp) {
        if (lazy_sync_) {
            void* d       = reinterpret_cast<void*>(surface_->FrameInterface->Context);
            mfxSession* s = reinterpret_cast<mfxSession*>(d);
            s[0]          = session;

            mfxSyncPoint* sps = reinterpret_cast<mfxSyncPoint*>(d);
            sps[1]            = sp;
        }
        return;
    }

    /// @brief Friend operator to print out state of the class in human readable form.
    /// @param[inout] out Reference to the stream to write.
    /// @param[in] f Referebce to the frame_surface instance to dump the state.
    /// @return Reference to the stream.
    friend std::ostream& operator<<(std::ostream& out, const frame_surface& f);

protected:
    /// @brief Pointer to the mfxFrameSurface1 object.
    mfxFrameSurface1* surface_;
    /// @brief Flag indicating that lazy sync technique must be used.
    bool lazy_sync_;
};

inline std::ostream& operator<<(std::ostream& out, const frame_surface& f) {
    out << "frame_surface class" << std::endl;
    out << detail::space(detail::INTENT, out, "Lazy sync    = ")
        << detail::Boolean2String(f.lazy_sync_) << std::endl;
    out << "frame_info" << std::endl;
    frame_info i(f.surface_->Info);
    out << i << std::endl;
    out << "frame_data" << std::endl;
    frame_data d(f.surface_->Data);
    out << d << std::endl;

    return out;
}

} // namespace vpl
} // namespace oneapi
