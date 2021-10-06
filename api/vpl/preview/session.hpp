/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <functional>
#include <iostream>
#include <limits>
#include <memory>

#include "vpl/preview/defs.hpp"
#include "vpl/preview/exception.hpp"
#include "vpl/preview/extension_buffer_list.hpp"
#include "vpl/preview/frame_surface.hpp"
#include "vpl/preview/future.hpp"
#include "vpl/preview/impl_selector.hpp"
#include "vpl/preview/source_reader.hpp"
#include "vpl/preview/stat.hpp"
#include "vpl/preview/video_param.hpp"

#include "vpl/mfxvideo.h"

#include "vpl/preview/detail/sdk_callable.hpp"

#ifdef LIBVA_SUPPORT
  #include "va/va.h"
  #include "va/va_drm.h"
  #include <fcntl.h>
  #include <unistd.h>
#endif

namespace oneapi {
namespace vpl {

/// @brief Base class to manage any session. User doesn't have ability to create instanse of this class.
/// only specialized classes for decoder or encoder or VPP sessions must be used.
/// General note: negative status is sent thought exceptions, warnings are delivered as a status codes.
/// @tparam VideoParams Class to manage Sesson paramers
/// @tparam InitList Class to manage extensio buffers for Init stage
/// @tparam ResetList Class to manage extensio buffers for Reset stage
template <typename VideoParams, typename InitList, typename ResetList>
class session {
protected:
    /// @brief Session's data processing state.
    enum class state : unsigned int {
        Processing = 0, ///< Session process the input data.
        Draining   = 1, ///< Session's source reported EOS, so session is draining.
        Done       = 2, ///< All processing is done. No cached data remains in the session.
    };
    /// @brief Protected ctor. Creates session by using supplyed implementation selector.
    /// @param[in] sel Implementation selector
    /// @param[in] callable C API functions table
    session(const implemetation_selector &sel, detail::sdk_c_api callable)
            : c_api_callable_(callable),
              state_(state::Processing),
              component_(component::unknown),
              accelerator_handle(nullptr) {
        auto [l_, s_]  = sel.session();
        this->loader_  = l_;
        this->session_ = s_;        

        mfxStatus sts = MFXQueryIMPL(this->session_, &this->selected_impl_);
        if (sts != MFX_ERR_NONE) {
            this->selected_impl_ = 0;
        }
        this->version_ = { { 0, 0 } };
        sts            = MFXQueryVersion(this->session_, &this->version_);
        if (sts != MFX_ERR_NONE) {
            this->version_ = { { 0, 0 } };
        }
        init_accelerator_handle();
    }

public:
    /// @brief Dtor. Additionaly it closes loader.
    virtual ~session() {
        c_api_callable_.close(session_);
        MFXClose(session_);
        MFXUnload(loader_);
        free_accelerator_handle();
    }

    /// @brief Returns implementation capabilities.
    /// @return Implementation capabilities.
    std::shared_ptr<VideoParams> Caps() {
        std::shared_ptr<VideoParams> caps = std::make_shared<VideoParams>();
        [[maybe_unused]] detail::c_api_invoker e(detail::default_checker,
                                c_api_callable_.query,
                                session_,
                                nullptr,
                                caps->getMfx());
        return caps;
    }

    /// @brief Verifyes that implementation supports such capabilities. On output, corrected capabilities are returned
    /// @param[in] param Pointer to the imaplementation capabiliies.
    /// @return Corrected implementation capabilities.
    std::shared_ptr<VideoParams> Verify(VideoParams *param) {
        std::shared_ptr<VideoParams> out = std::make_shared<VideoParams>();
        [[maybe_unused]] detail::c_api_invoker e(
            detail::default_checker,
            std::bind(c_api_callable_.query, session_, param->getMfx(), out->getMfx()));
        return out;
    }

    /// @brief Initializes the session by using provided parameters
    /// @param[in] par Init parameters
    /// @param[in] list List of extension buffers.
    /// @return Status of the initialization.
    status Init(VideoParams *par, InitList list = {}) {
        if (list.get_size()) {
            if (auto [buffers, size] = list.get_raw_ext_buffers(); size) {
                par->set_extension_buffers(buffers, static_cast<uint16_t>(size));
            }
        }
        detail::c_api_invoker e(detail::default_checker,
                                std::bind(c_api_callable_.init, session_, par->getMfx()));
        par->clear_extension_buffers();

        return mfxstatus_to_onevplstatus(e.sts_);
    }

    /// @brief Resets the session by using provided parameters
    /// @param[in] par Reset parameters
    /// @param[in] list List of extension buffers.
    /// @return Status of the reset.
    status Reset(VideoParams *par, ResetList list) {
        if (list.get_size()) {
            if (auto [buffers, size] = list.get_raw_ext_buffers(); size) {
                par->set_extension_buffers(buffers, static_cast<uint16_t>(size));
            }
        }

        detail::c_api_invoker e(detail::default_checker,
                                std::bind(c_api_callable_.reset, session_, par->getMfx()),
                                std::bind(c_api_callable_.init, session_, par->getMfx()));
        state_ = state::Processing;
        par->clear_extension_buffers();
        return mfxstatus_to_onevplstatus(e.sts_);
    }

    /// @brief Retrieves current session parameters.
    /// @return Session parameters.
    std::shared_ptr<VideoParams> working_params() {
        std::shared_ptr<VideoParams> out = std::make_shared<VideoParams>();
        [[maybe_unused]] detail::c_api_invoker e(detail::default_checker,
                                c_api_callable_.params,
                                session_,
                                out->getMfx());
        return out;
    }

    /// @brief Returns session's domain
    /// @return Session's domain
    component get_component_domain() const {
        return component_;
    }

    /// @brief Returns implementation
    /// @return implementation
    mfxIMPL get_implementation() {
        return selected_impl_;
    }

    /// @brief Returns version
    /// @return version
    mfxVersion get_version() {
        return version_;
    }

protected:
    /// @brief Session handle.
    mfxSession session_;
    /// @brief Functions table.
    detail::sdk_c_api c_api_callable_;
    /// @brief Processing state of the session.
    state state_;
    /// @brief Session's type identifier. Domain in other words
    component component_;

    /// @brief Selected actual implementation
    mfxIMPL selected_impl_;

    /// @brief Version of implementation
    mfxVersion version_;

    /// @brief accelorator file handle
    int fd_;

    void init_accelerator_handle() {
        mfxIMPL impl;
        mfxStatus sts = MFXQueryIMPL(session_, &impl);
        if (sts != MFX_ERR_NONE)
            return;

        #ifdef LIBVA_SUPPORT
        if ((impl & MFX_IMPL_VIA_VAAPI) == MFX_IMPL_VIA_VAAPI) {
            VADisplay va_dpy = NULL;
            // initialize VAAPI context and set session handle (req in Linux)
            fd_ = open("/dev/dri/renderD128", O_RDWR);
            if (fd_ >= 0) {
                va_dpy = vaGetDisplayDRM(fd_);
                if (va_dpy) {
                    int major_version = 0, minor_version = 0;
                    if (VA_STATUS_SUCCESS == vaInitialize(va_dpy, &major_version, &minor_version)) {
                        MFXVideoCORE_SetHandle(session_,
                                            static_cast<mfxHandleType>(MFX_HANDLE_VA_DISPLAY),
                                            va_dpy);
                    }
                }
            }
            accelerator_handle = va_dpy;
        }
        #endif
    }

    void free_accelerator_handle() {
        #ifdef LIBVA_SUPPORT
            vaTerminate((VADisplay)accelerator_handle);
            accelerator_handle = nullptr;
            close(fd_);
        #endif
    }


    /// @brief Accelerator handle
    void *accelerator_handle;

    /// @brief Convert MFX_ return codes to oneVPL status
    /// @return oneVPL status code
    static status mfxstatus_to_onevplstatus(mfxStatus s) {
        switch (s) {
            case MFX_ERR_NONE:
                return status::Ok;
            case MFX_WRN_IN_EXECUTION:
                return status::ExecutionInProgress;
            case MFX_WRN_DEVICE_BUSY:
                return status::DeviceBusy;
            case MFX_WRN_VIDEO_PARAM_CHANGED:
                return status::VideoParamChanged;
            case MFX_WRN_PARTIAL_ACCELERATION:
                return status::PartialAcceleration;
            case MFX_WRN_INCOMPATIBLE_VIDEO_PARAM:
                return status::IncompartibleVideoParam;
            case MFX_WRN_VALUE_NOT_CHANGED:
                return status::ValueNotChanged;
            case MFX_WRN_OUT_OF_RANGE:
                return status::OutOfRange;
            case MFX_TASK_WORKING:
                return status::TaskWorking;
            case MFX_TASK_BUSY:
                return status::TaskBusy;
            case MFX_WRN_FILTER_SKIPPED:
                return status::FilterSkipped;
            case MFX_ERR_NONE_PARTIAL_OUTPUT:
                return status::PartialOutput;
            //// Errors but they need to be treat as positive status in some cases.
            case MFX_ERR_NOT_ENOUGH_BUFFER:
                return status::NotEnoughBuffer;
            case MFX_ERR_MORE_DATA:
                return status::NotEnoughData;
            case MFX_ERR_MORE_SURFACE:
                return status::NotEnoughSurface;
            default:
                return status::Unknown;
        }
    }

private:
    mfxLoader loader_;
};

/// @brief Manages decoder's sessions.
/// @tparam Reader Bitstream reader class
/// @todo SFINAE it
template <typename Reader>
class decode_session
        : public session<decoder_video_param, decoder_init_reset_list, decoder_init_reset_list> {
public:
    /// @brief Constructs decoder session
    /// @param[in] sel Implementation selector
    /// @param[in] codecID Codec ID
    /// @param[in] rdr Bitstream reader
    decode_session(const implemetation_selector &sel, codec_format_fourcc codecID, Reader *rdr)
            : session(sel, detail::CAPI<>::Decoder),
              bits_(codecID),
              rdr_(rdr),
              params_() {
        component_ = component::decoder;
        params_.set_CodecId(codecID);
        params_.clear_extension_buffers();
    }

    /// @brief Constructs decoder session
    /// @param[in] sel Implementation selector
    /// @param[in] params Video params
    /// @param[in] rdr Bitstream reader
    decode_session(const implemetation_selector &sel,
                   const decoder_video_param &params,
                   Reader *rdr)
            : session(sel, detail::CAPI<>::Decoder),
              bits_((codec_format_fourcc)params.get_CodecId()),
              rdr_(rdr),
              params_(params) {
        component_ = component::decoder;
        params_.clear_extension_buffers();
    }

    /// @brief Dtor
    ~decode_session() {}

    /// @brief Initialize the session by using bitream portion. This step can be omitted if the codec ID is known or
    /// we don't need to get SSP or PPS data from the bitstream.
    /// @param[in] decHeaderList List of extension buffers for InitHeader stage. Can be NULL.
    /// @param[in] initList List of extension buffers for Init stage. Can be NULL.
    /// @return Ok or warnings
    status init_by_header(decoder_init_header_list decHeaderList = {},
                          decoder_init_reset_list initList       = {}) {
        mfxStatus sts = MFX_ERR_MORE_DATA;

        if (decHeaderList.get_size()) {
            if (auto [buffers, size] = decHeaderList.get_raw_ext_buffers(); size) {
                params_.set_extension_buffers(buffers, static_cast<uint16_t>(size));
            }
        }

        do {
            rdr_->get_data(&bits_);

            detail::c_api_invoker e({ [](mfxStatus s) {
                                        switch (s) {
                                            case MFX_ERR_MORE_DATA:
                                                return false;
                                            default:
                                                break;
                                        }

                                        bool ret = (s < 0) ? true : false;
                                        return ret;
                                    } },
                                    MFXVideoDECODE_DecodeHeader,
                                    session_,
                                    bits_(),
                                    params_.getMfx());
            sts = e.sts_;
        } while (sts == MFX_ERR_MORE_DATA && !rdr_->is_EOS());

        if (sts != MFX_ERR_NONE && rdr_->is_EOS())
            return status::EndOfStreamReached;
        if (sts != MFX_ERR_NONE)
            mfxstatus_to_onevplstatus(sts);
        auto result = Init(&params_, initList);
        params_.clear_extension_buffers();
        return result;
    }

    /// @brief Decodes frame
    /// @param[out] out_surface Future object with decoded data.
    /// @param[in] list List of extension buffers to attach to bitstream.
    /// @return Ok or warning
    status decode_frame(std::shared_ptr<frame_surface> out_surface,
                        decoder_process_list list = {}) {
        mfxSyncPoint syncp;
        mfxFrameSurface1 *surf = NULL;

        rdr_->get_data(&bits_);

        mfxBitstream *bts;
        if (bits_.get_DataLength() == 0 && rdr_->is_EOS()) {
            bts    = nullptr;
            state_ = state::Draining;
        }
        else {
            bts = bits_();
            if (auto [buffers, size] = list.get_raw_ext_buffers(); size) {
                bts->NumExtParam = static_cast<uint16_t>(size);
                bts->ExtParam    = buffers;
            }
            else {
                bts->NumExtParam = 0;
                bts->ExtParam    = nullptr;
            }
        }

        detail::c_api_invoker e({ [](mfxStatus s) {
                                    switch (s) {
                                        case MFX_ERR_MORE_DATA:
                                            return false;
                                        case MFX_ERR_MORE_SURFACE:
                                            return false;
                                        default:
                                            break;
                                    }

                                    bool ret = (s < 0) ? true : false;
                                    return ret;
                                } },
                                MFXVideoDECODE_DecodeFrameAsync,
                                session_,
                                bts,
                                nullptr,
                                &surf,
                                &syncp);

        if (surf) {
            // out_surface = std::make_shared<frame_surface>(surf);
            // out_surface.reset(new frame_surface(surf));
            // std::shared_ptr<frame_surface> tmp = std::make_shared<frame_surface>(surf);
            // out_surface.swap(tmp);
            out_surface->inject(surf, out_surface.use_count() + 1);
        }

        if (e.sts_ == MFX_ERR_MORE_DATA && state_ == state::Draining) {
            state_ = state::Done;
            return status::EndOfStreamReached;
        }
        return mfxstatus_to_onevplstatus(e.sts_);
    }

    /// @brief Decodes frame
    /// @param[in] list List of extension buffers to attach to bitstream
    /// @return Future object with decoded data
    std::shared_ptr<future<std::shared_ptr<frame_surface>>> process(
        decoder_process_list list = {}) {
        std::shared_ptr<frame_surface> surface = std::make_shared<frame_surface>();
        std::shared_ptr<future_surface_t> f;

        operation_status op(component_, this);

        if (state_ != state::Done) {
            try {
                status schedule_status;
                schedule_status     = decode_frame(surface, list);
                f                   = std::make_shared<future_surface_t>(surface);
                op.schedule_status_ = schedule_status;
            }
            catch (base_exception &e) {
                f                   = std::make_shared<future_surface_t>(nullptr);
                op.schedule_status_ = mfxstatus_to_onevplstatus(e.get_status());
                op.fatal_           = true;
            }
        }
        else {
            f                   = std::make_shared<future_surface_t>(nullptr);
            op.schedule_status_ = status::EndOfStreamReached;
        }

        f->add_operation(op);
        return f;
    }
    /// @brief Retrieve decoder statistic
    /// @return Decoder statistic
    std::shared_ptr<decode_stat> getStat() {
        std::shared_ptr<decode_stat> out = std::make_shared<decode_stat>();
        decode_stat *dec_stat            = out.get();
        [[maybe_unused]] detail::c_api_invoker e(detail::default_checker,
                                MFXVideoDECODE_GetDecodeStat,
                                session_,
                                dec_stat->get_raw());
        return out;
    }

    /// @brief Get video params
    /// @return params
    decoder_video_param getParams() {
        return params_;
    }

protected:
    /// @brief Bitstream keeper
    bitstream_as_src bits_;
    /// @brief Bitstream reader
    Reader *rdr_;
    /// @brief Video params
    decoder_video_param params_;
};

/// @brief Manages encoder's sessions.
/// @todo SFINAE it
class encode_session : public session<encoder_video_param, encoder_init_list, encoder_reset_list> {
public:
    /// @brief Constructs encoder session
    /// @param[in] sel Implementation selector
    explicit encode_session(const implemetation_selector &sel)
            : session(sel, detail::CAPI<>::Encoder),
              rdr_(nullptr) {
        component_ = component::encoder;
    }

    /// @brief Constructs encoder session
    /// @param[in] sel Implementation selector
    /// @param[in] rdr Pointer to the raw frame reader
    encode_session(const implemetation_selector &sel, frame_source_reader *rdr)
            : session(sel, detail::CAPI<>::Encoder),
              rdr_(rdr) {
        component_ = component::encoder;
    }

    /// @brief Dtor
    ~encode_session() {}

    /// @brief Allocate and return shared pointer to the surface
    /// @todo temporary method
    /// @return Shared pointer to the allocated surface
    auto alloc_input() {
        mfxFrameSurface1 *surface = nullptr;
        detail::c_api_invoker e(detail::default_checker,
                                MFXMemory_GetSurfaceForEncode,
                                session_,
                                &surface);

        return std::make_shared<frame_surface>(surface);
    }

    /// @brief Temporal method to sync the surface's data.
    /// @todo remove during migration to 2.1
    /// @param[in] sp Synchronization point handle.
    /// @param[in] to Time to wait.
    void sync(mfxSyncPoint sp, int to) {
        if (sp) {
            detail::c_api_invoker e(detail::default_checker,
                                    MFXVideoCORE_SyncOperation,
                                    session_,
                                    sp,
                                    to);
        }
    }
    /// @brief Encodes frame
    /// @param[in] in_surface Object with the data to encode.
    /// @param[out] bs Future object with bitstream portion.
    /// @param[in] list List of extension buffers to use
    /// @return Ok or warning
    status encode_frame(std::shared_ptr<frame_surface> in_surface,
                        std::shared_ptr<bitstream_as_dst> bs,
                        encoder_process_list list = {}) {
        mfxSyncPoint sp;
        mfxFrameSurface1 *surf = in_surface.get() ? in_surface.get()->get_raw_ptr() : nullptr;
        std::shared_ptr<mfxEncodeCtrl> ctrl = nullptr;
        bool alocated_ctrl                  = false;

        if (nullptr == surf) {
            state_ = state::Draining;
        }
        if(list.get_size() && list.has_buffer<0>()) {
            ctrl.reset(list.get_buffer<mfxEncodeCtrl, 0>(), [](mfxEncodeCtrl *p) { p = nullptr; });
            std::cout << "enc ctrl from the list" << std::endl;
        } else {
            ctrl = std::make_shared<mfxEncodeCtrl>();
            *ctrl = {0};
            alocated_ctrl = true;
        }
        
        // Asumption: Encoder will copy-in all extension buffers.
        if (auto [buffers, size] = list.get_raw_ext_buffers(); size) {
            ctrl->ExtParam = buffers;
            ctrl->NumExtParam = (mfxU16)size;
        } else {
            if (alocated_ctrl) {
                ctrl.reset();
            } else {
                ctrl->ExtParam = 0;
                ctrl->NumExtParam = 0;
            }
        }
        detail::c_api_invoker e({ [](mfxStatus s) {
                                    switch (s) {
                                        case MFX_ERR_MORE_DATA:
                                            return false;
                                        case MFX_ERR_NOT_ENOUGH_BUFFER:
                                            return false;
                                        default:
                                            break;
                                    }

                                    bool ret = (s < 0) ? true : false;
                                    return ret;
                                } },
                                MFXVideoENCODE_EncodeFrameAsync,
                                session_,
                                ctrl.get(),
                                surf,
                                (*bs.get())(),
                                &sp);
        bs->associate_context({ session_, sp });

        if (e.sts_ == MFX_ERR_MORE_DATA && state_ == state::Draining) {
            state_ = state::Done;
            return status::EndOfStreamReached;
        }

        return mfxstatus_to_onevplstatus(e.sts_);
    }

    /// @brief Encodes frame by using provided source reader to get data to encode
    /// @param[out] bs Future object with bitstream portion.
    /// @param[in] list List of extension buffers to use
    /// @return Ok or warning
    status encode_frame(std::shared_ptr<bitstream_as_dst> bs, encoder_process_list list = {}) {
        status sts;
        if (!rdr_)
            throw base_exception("NULL reader ptr", MFX_ERR_NULL_PTR);

        while (1) {
            std::shared_ptr<frame_surface> input_surface(nullptr);

            if (!rdr_->is_EOS()) {
                input_surface = alloc_input();
                rdr_->get_data(input_surface);
                if (rdr_->is_EOS())
                    input_surface.reset();
            }

            sts = encode_frame(input_surface, bs, list);

            switch (sts) {
                case status::Ok:
                    return status::Ok;
                case status::EndOfStreamReached:
                    return status::EndOfStreamReached;
                case status::NotEnoughData:
                    break;
                case status::NotEnoughBuffer: {
                    bs->realloc();
                    // Assume that frame was cached so we need to increase buffer size only and send new frame to enc
                    break;
                }
                default:
                    return sts;
            }
        }

        return sts;
    }

    /// @brief Encode frame. Function returns the future object with the bitstream which will hold processed data. User
    /// needs to sync up the future object before accessing.
    /// This function expected to work in the chain and uses provided future object to get the data to process.
    /// @param[in] in_future Future object with the surface from the previous operation.
    /// @param[in] list List of extension buffers to use
    /// @return Future object with the bitstream.
    std::shared_ptr<future_bitstream_t> process(std::shared_ptr<future_surface_t> in_future,
                                                encoder_process_list list = {}) {
        std::shared_ptr<bitstream_as_dst> bits;
        std::shared_ptr<future_bitstream_t> f_out = std::make_shared<future_bitstream_t>(nullptr);
        operation_status op(component_, this);

        /// @todo add smart wait with status propagation
        std::shared_ptr<frame_surface> in_surface = in_future->get();

        if (state_ == state::Done) {
            op.schedule_status_ = status::EndOfStreamReached;
        }
        else {
            switch (in_future->get_last_schedule_status()) {
                case status::Ok:
                case status::EndOfStreamReached: {
                    try {
                        status schedule_status;

                        bits            = std::make_shared<bitstream_as_dst>();
                        schedule_status = encode_frame(in_surface, bits, list);
                        f_out           = std::make_shared<future_bitstream_t>(bits);
                        f_out.reset(new future_bitstream_t(bits));
                        op.schedule_status_ = schedule_status;
                    }
                    catch (base_exception &e) {
                        std::cout << "encoder gonna die" << std::endl << std::flush;
                        op.schedule_status_ = mfxstatus_to_onevplstatus(e.get_status());
                        op.fatal_           = true;
                    }
                } break;
                default: {
                    op.schedule_status_ = in_future->get_last_schedule_status();
                } break;
            }
        }

        f_out->add_operation(op);
        f_out->propagate_history(*(in_future.get()));
        return f_out;
    }

    /// @brief Retrieve encoder statistic
    /// @return Encoder statistic
    std::shared_ptr<encode_stat> getStat() {
        std::shared_ptr<encode_stat> out = std::make_shared<encode_stat>();
        encode_stat *enc_stat            = out.get();
        detail::c_api_invoker e(detail::default_checker,
                                MFXVideoENCODE_GetEncodeStat,
                                session_,
                                enc_stat->get_raw());
        return out;
    }

protected:
    /// @brief Raw freames reader
    frame_source_reader *rdr_;
};

/// @brief Manages VPP's sessions.
/// @todo SFINAE it
class vpp_session : public session<vpp_video_param, vpp_init_reset_list, vpp_init_reset_list> {
public:
    /// @brief Constructs encoder session
    /// @param[in] sel Implementation selector
    explicit vpp_session(const implemetation_selector &sel)
            : session(sel, detail::CAPI<>::VPP),
              rdr_(nullptr) {
        component_ = component::vpp;
    }

    /// @brief Constructs encoder session
    /// @param[in] sel Implementation selector
    /// @param[in] rdr Pointer to the raw frame reader
    vpp_session(const implemetation_selector &sel, frame_source_reader *rdr)
            : session(sel, detail::CAPI<>::VPP),
              rdr_(rdr) {
        component_ = component::vpp;
    }

    /// @brief Dtor
    ~vpp_session() {}

    /// @brief Allocate and return shared pointer to the input surface
    /// @todo temporary method
    /// @return Shared pointer to the allocated surface
    auto alloc_input() {
        mfxFrameSurface1 *surface = nullptr;
        detail::c_api_invoker e(detail::default_checker,
                                MFXMemory_GetSurfaceForVPP,
                                session_,
                                &surface);

        return std::make_shared<frame_surface>(surface);
    }

    /// @brief Allocate internal raw surface and attach it to the output surface
    /// @param[inout] out_surface Reference to the frame_surface.
    /// @todo temporary method
    void alloc_output(std::shared_ptr<frame_surface> &out_surface) {
        mfxFrameSurface1 *surface = nullptr;
        detail::c_api_invoker e(detail::default_checker,
                                MFXMemory_GetSurfaceForVPPOut,
                                session_,
                                &surface);

        out_surface->inject(surface, out_surface.use_count() + 1);

        return;
    }

    /// @brief Initializes session with given parameters and extention buffers.
    /// @param[in] par Pointer to the parameters.
    /// @param[in] list List of extention buffers.
    /// @return Initialization status
    /// @todo This method brakes RAII concept. Need to move to ctor.
    status Init(vpp_video_param *par, vpp_init_reset_list list = {}) {
        status init_sts                 = session::Init(par, list);
        return init_sts;
    }

    /// @brief Temporal method to sync rhe surface's data.
    /// @todo remove during migration to 2.1
    /// @param[in] sp Synchronization point handle.
    /// @param[in] to Time to wait.
    void sync(mfxSyncPoint sp, int to) {
        if (sp) {
            detail::c_api_invoker e(detail::default_checker,
                                    MFXVideoCORE_SyncOperation,
                                    session_,
                                    sp,
                                    to);
        }
    }
    /// @brief Process frame. Function returns the surface which will hold processed data. User need to sync up the
    /// surface data before accessing.
    /// @param[in] in_surface Pointer to the input surface.
    /// @param[out] out_surface Placeholder for the allocated output surface.
    /// @return Ok or warning
    status process_frame(std::shared_ptr<frame_surface> in_surface,
                         std::shared_ptr<frame_surface>& out_surface) {
        mfxSyncPoint sp;
        mfxFrameSurface1 *surf = in_surface.get() ? in_surface->get_raw_ptr() : nullptr;

        if (nullptr == surf) {
            state_ = state::Draining;
        }
        alloc_output(out_surface);
        detail::c_api_invoker e({ [](mfxStatus s) {
                                    switch (s) {
                                        case MFX_ERR_MORE_DATA:
                                            return false;
                                        case MFX_ERR_MORE_SURFACE:
                                            return false;
                                        default:
                                            break;
                                    }

                                    bool ret = (s < 0) ? true : false;
                                    return ret;
                                } },
                                MFXVideoVPP_RunFrameVPPAsync,
                                session_,
                                surf,
                                out_surface->get_raw_ptr(),
                                nullptr,
                                &sp);

        if (e.sts_ == MFX_ERR_MORE_DATA && state_ == state::Draining) {
            state_ = state::Done;
            out_surface.reset();
            return status::EndOfStreamReached;
        }

        return mfxstatus_to_onevplstatus(e.sts_);
    }

    /// @brief Process frame. Function returns the surface which will hold processed data. User need to sync up the
    /// surface data before accessing.
    /// @param[out] out_surface Placeholder for the allocated output surface.
    /// @return Ok or warning
    status process_frame(std::shared_ptr<frame_surface>& out_surface) {
        status sts;
        if (!rdr_)
            throw base_exception("NULL reader ptr", MFX_ERR_NULL_PTR);

        while (1) {
            std::shared_ptr<frame_surface> input_surface(nullptr);

            if (!rdr_->is_EOS()) {
                input_surface = alloc_input();
                rdr_->get_data(input_surface);
                if (rdr_->is_EOS())
                    input_surface.reset();
            }

            sts = process_frame(input_surface, out_surface);

            switch (sts) {
                case status::Ok:
                    return status::Ok;
                case status::EndOfStreamReached:
                    return status::EndOfStreamReached;
                case status::NotEnoughData:
                    break;
                default:
                    return sts;
            }
        }

        return sts;
    }

    /// @brief Process frame. Function returns the future object with the surface which will hold processed data. User
    /// need to sync up the future object before accessing.
    /// This function expected to work in the chain and uses provided future object to get the data to process.
    /// @param[in] in_future Future object with the surface from the previouse operation.
    /// @return Future object with the surface.
    std::shared_ptr<future_surface_t> process(std::shared_ptr<future_surface_t> in_future) {
        std::shared_ptr<frame_surface> surface;
        std::shared_ptr<future_surface_t> f_out = std::make_shared<future_surface_t>(nullptr);
        operation_status op(component_, this);

        /// @todo add smart wait with status propagation
        std::shared_ptr<frame_surface> in_surface = in_future->get();

        if (state_ == state::Done) {
            op.schedule_status_ = status::EndOfStreamReached;
        }
        else {
            switch (in_future->get_last_schedule_status()) {
                case status::Ok:
                case status::EndOfStreamReached: {
                    try {
                        status schedule_status;
                        schedule_status = process_frame(in_surface, surface);
                        f_out.reset(new future_surface_t(surface));
                        op.schedule_status_ = schedule_status;
                    }
                    catch (base_exception &e) {
                        op.schedule_status_ = mfxstatus_to_onevplstatus(e.get_status());
                        op.fatal_           = true;
                    }
                } break;
                default: {
                    op.schedule_status_ = in_future->get_last_schedule_status();
                } break;
            }
        }

        f_out->add_operation(op);
        f_out->propagate_history(*(in_future.get()));

        return f_out;
    }

    /// @brief Retrieve encoder statistic
    /// @return VPP statistic
    std::shared_ptr<vpp_stat> getStat() {
        std::shared_ptr<vpp_stat> out = std::make_shared<vpp_stat>();
        vpp_stat *vpp_stat            = out.get();
        detail::c_api_invoker e(detail::default_checker,
                                MFXVideoVPP_GetVPPStat,
                                session_,
                                vpp_stat->get_raw());
        return out;
    }

protected:
    /// @brief Raw freames reader
    frame_source_reader *rdr_;
};

} // namespace vpl
} // namespace oneapi
