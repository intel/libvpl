/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <functional>
#include <utility>

#include "vpl/mfxvideo.h"
#include "vpl/preview/exception.hpp"

namespace oneapi {
namespace vpl {
namespace detail {

/// @brief Underlying C API Functions table. It is initialized either for encoder or foe decoder or forVPP.
struct sdk_c_api {
    /// @brief MFXVideo***_Query function
    std::function<mfxStatus(mfxSession, mfxVideoParam*, mfxVideoParam*)> query;
    /// @brief MFXVideo***_Init function
    std::function<mfxStatus(mfxSession, mfxVideoParam*)> init;
    /// @brief MFXVideo***_Reset function
    std::function<mfxStatus(mfxSession, mfxVideoParam*)> reset;
    /// @brief MFXVideo***_GetVideoParams function
    std::function<mfxStatus(mfxSession, mfxVideoParam*)> params;
    /// @brief MFXVideo***_Close function
    std::function<mfxStatus(mfxSession)> close;
};

template <class T = void>
struct CAPI {
    static inline sdk_c_api Decoder = { MFXVideoDECODE_Query,
                                        MFXVideoDECODE_Init,
                                        MFXVideoDECODE_Reset,
                                        MFXVideoDECODE_GetVideoParam,
                                        MFXVideoDECODE_Close };

    static inline sdk_c_api Encoder = { MFXVideoENCODE_Query,
                                        MFXVideoENCODE_Init,
                                        MFXVideoENCODE_Reset,
                                        MFXVideoENCODE_GetVideoParam,
                                        MFXVideoENCODE_Close };

    static inline sdk_c_api VPP = { MFXVideoVPP_Query,
                                    MFXVideoVPP_Init,
                                    MFXVideoVPP_Reset,
                                    MFXVideoVPP_GetVideoParam,
                                    MFXVideoVPP_Close };
};

/// @brief Safely calls C functions and throw exception in case of negative error code. User can provide own
/// function to verify error code.
class c_api_invoker {
public:
    /// @brief Construct executer and call the C function. Throws exception if error code verification permits it.
    /// @tparam Function C type.
    /// @tparam ... C function arguments.
    /// @param  bad_status_checker Lambda function to check error code. If True is returned, exception is thrown.
    /// @param f C function name
    /// @param args C function arguments
    template <class Function, class... Args>
    explicit c_api_invoker(std::function<bool(mfxStatus)> bad_status_checker,
                           Function&& f,
                           Args&&... args) {
        sts_ = std::invoke(std::forward<Function>(f), std::forward<Args>(args)...);
        if (bad_status_checker(sts_)) {
            throw base_exception(sts_);
        }
    }
    /// @brief C function call status
    mfxStatus sts_;
};

/// @brief Verifies error code and returns true if it negative
inline bool default_checker(mfxStatus s) {
    bool ret = (s < 0) ? true : false;
    return ret;
}

} // namespace detail
} // namespace vpl
} // namespace oneapi
