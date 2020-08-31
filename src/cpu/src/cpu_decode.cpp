/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_decode.h"
#include <memory>
#include <utility>
#include "src/cpu_workstream.h"

CpuDecode::CpuDecode(CpuWorkstream *session)
        : m_session(session),
          m_avDecCodec(nullptr),
          m_avDecContext(nullptr),
          m_avDecParser(nullptr),
          m_avDecPacket(nullptr),
          m_avDecFrameOut(nullptr),
          m_swsContext(nullptr),
          m_param(),
          m_decSurfaces(),
          m_bFrameBuffered(false) {}

mfxStatus CpuDecode::ValidateDecodeParams(mfxVideoParam *par, bool canCorrect) {
    switch (par->mfx.CodecId) {
        case MFX_CODEC_HEVC:
        case MFX_CODEC_AVC:
        case MFX_CODEC_JPEG:
        case MFX_CODEC_AV1:
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (!par->IOPattern && canCorrect)
        par->IOPattern = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    if (!par->AsyncDepth && canCorrect)
        par->AsyncDepth = 1;

    //only system memory allowed
    if (par->IOPattern != MFX_IOPATTERN_OUT_SYSTEM_MEMORY)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (!par->mfx.FrameInfo.FourCC && canCorrect)
        par->mfx.FrameInfo.FourCC = MFX_FOURCC_I420;

    //only I420 and I010 colorspaces allowed
    switch (par->mfx.FrameInfo.FourCC) {
        case MFX_FOURCC_I420:
            if (canCorrect) {
                par->mfx.FrameInfo.BitDepthLuma = 8;
                par->mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
            }
            break;
        case MFX_FOURCC_I010:
            if (canCorrect) {
                par->mfx.FrameInfo.BitDepthLuma = 10;
                par->mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
            }
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    //Must have width and height
    if (par->mfx.FrameInfo.Width == 0 || par->mfx.FrameInfo.Height == 0) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    mfxU32 MAX_WIDTH  = 3840;
    mfxU32 MAX_HEIGHT = 2160;

    //width and height must be <= max
    if (par->mfx.FrameInfo.Width > MAX_WIDTH ||
        par->mfx.FrameInfo.Height > MAX_HEIGHT ||
        par->mfx.FrameInfo.CropW > MAX_WIDTH ||
        par->mfx.FrameInfo.CropH > MAX_HEIGHT) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    //BitDepthLuma can only be 8 or 10
    switch (par->mfx.FrameInfo.BitDepthLuma) {
        case 8:
        case 10:
        case 0:
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    //BitDepthChroma can only be 8 or 10
    switch (par->mfx.FrameInfo.BitDepthChroma) {
        case 8:
        case 10:
        case 0:
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (par->mfx.CodecProfile > 0x1FF)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->mfx.CodecLevel > 0x1FF)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    switch (par->mfx.FrameInfo.ChromaFormat) {
        case MFX_CHROMAFORMAT_YUV420:
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (par->mfx.FrameInfo.AspectRatioW == 0 && canCorrect)
        par->mfx.FrameInfo.AspectRatioW = 1;

    if (par->mfx.FrameInfo.AspectRatioH == 0 && canCorrect)
        par->mfx.FrameInfo.AspectRatioH = 1;

    if (par->mfx.FrameInfo.FrameRateExtN == 0 && canCorrect)
        par->mfx.FrameInfo.FrameRateExtN = 30;

    if (par->mfx.FrameInfo.FrameRateExtN > 65535)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->mfx.FrameInfo.FrameRateExtD == 0 && canCorrect)
        par->mfx.FrameInfo.FrameRateExtD = 1;

    if (par->mfx.FrameInfo.FrameRateExtD > 65535)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    return MFX_ERR_NONE;
}

//InitDecode can operate in two modes:
// With no bitstream: assumes header decoded elsewhere, validates params given
// With bitstream
//  1. Attempts to decode a frame
//  2. Gets parameters
mfxStatus CpuDecode::InitDecode(mfxVideoParam *par, mfxBitstream *bs) {
    AVCodecID cid = MFXCodecId_to_AVCodecID(par->mfx.CodecId);
    RET_IF_FALSE(cid, MFX_ERR_INVALID_VIDEO_PARAM);

    if (!bs) {
        mfxStatus sts = ValidateDecodeParams(par, false);
        if (sts != MFX_ERR_NONE)
            return sts;
    }

    m_avDecCodec = avcodec_find_decoder(cid);
    if (!m_avDecCodec) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    m_avDecContext = avcodec_alloc_context3(m_avDecCodec);
    if (!m_avDecContext) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    m_avDecParser = av_parser_init(m_avDecCodec->id);
    if (!m_avDecParser) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

#ifdef ENABLE_LIBAV_AUTO_THREADS
    m_avDecContext->thread_count = 0;
#endif

    if (avcodec_open2(m_avDecContext, m_avDecCodec, NULL) < 0) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    m_avDecPacket = av_packet_alloc();
    if (!m_avDecPacket) {
        return MFX_ERR_MEMORY_ALLOC;
    }

    m_avDecFrameOut = av_frame_alloc();
    if (!m_avDecFrameOut) {
        return MFX_ERR_MEMORY_ALLOC;
    }

    m_param = *par;

    if (bs) {
        // create copy to not modify caller's mfxBitstream
        // todo: this only works if input is large enough to
        // decode a frame
        mfxBitstream bs2 = *bs;
        bs2.DataFlag     = MFX_BITSTREAM_EOS;
        DecodeFrame(&bs2, nullptr, nullptr);
        GetVideoParam(par);
    }

    return MFX_ERR_NONE;
}

CpuDecode::~CpuDecode() {
    if (m_swsContext) {
        sws_freeContext(m_swsContext);
    }

    if (m_avDecFrameOut) {
        av_frame_free(&m_avDecFrameOut);
        m_avDecFrameOut = nullptr;
    }

    if (m_avDecParser) {
        av_parser_close(m_avDecParser);
        m_avDecParser = nullptr;
    }

    if (m_avDecPacket) {
        av_packet_free(&m_avDecPacket);
        m_avDecPacket = nullptr;
    }

    if (m_avDecContext) {
        avcodec_close(m_avDecContext);
        avcodec_free_context(&m_avDecContext);
        m_avDecContext = nullptr;
    }
}

// bs == 0 is a signal to drain
mfxStatus CpuDecode::DecodeFrame(mfxBitstream *bs,
                                 mfxFrameSurface1 *surface_work,
                                 mfxFrameSurface1 **surface_out) {
    if (m_bFrameBuffered) {
        if (surface_work && surface_out) {
            RET_ERROR(AVFrame2mfxFrameSurface(surface_work,
                                              m_avDecFrameOut,
                                              m_session->GetFrameAllocator()));

            *surface_out     = surface_work;
            m_bFrameBuffered = false;
            return MFX_ERR_NONE;
        }
        else {
            return MFX_ERR_MORE_SURFACE;
        }
    }

    // Try get AVFrame from surface_work
    AVFrame *avframe    = nullptr;
    CpuFrame *cpu_frame = CpuFrame::TryCast(surface_work);
    if (cpu_frame) {
        avframe = cpu_frame->GetAVFrame();
    }
    if (!avframe) { // Otherwise use AVFrame allocated in this class
        avframe = m_avDecFrameOut;
    }

    bool complete_frame_mode = false;
    if (bs && ((bs->DataFlag & MFX_BITSTREAM_COMPLETE_FRAME) ==
               MFX_BITSTREAM_COMPLETE_FRAME)) {
        complete_frame_mode = true;
    }

    for (;;) {
        int bytes_parsed = 0;

        if (complete_frame_mode) {
            m_avDecPacket->data = bs->Data + bs->DataOffset;
            m_avDecPacket->size = bs->DataLength;
            bytes_parsed        = bs->DataLength;
            bs->DataOffset += bytes_parsed;
            bs->DataLength -= bytes_parsed;
        }
        else {
            // parse
            auto data_ptr = bs ? (bs->Data + bs->DataOffset) : nullptr;
            int data_size = bs ? bs->DataLength : 0;
            bytes_parsed += av_parser_parse2(m_avDecParser,
                                             m_avDecContext,
                                             &m_avDecPacket->data,
                                             &m_avDecPacket->size,
                                             data_ptr,
                                             data_size,
                                             AV_NOPTS_VALUE,
                                             AV_NOPTS_VALUE,
                                             0);

            if (bs && bytes_parsed) {
                bs->DataOffset += bytes_parsed;
                bs->DataLength -= bytes_parsed;
            }
        }

        // send packet
        if (m_avDecPacket->size) {
            auto av_ret = avcodec_send_packet(m_avDecContext, m_avDecPacket);
            if (av_ret < 0) {
                return MFX_ERR_ABORTED;
            }
        }

        if (!bs) {
            // null bitstream indicates drain, send EOF packet
            avcodec_send_packet(m_avDecContext, nullptr);
        }
        else {
            // send EOF packet if EOS flag is set
            if ((bs->DataFlag & MFX_BITSTREAM_EOS) == MFX_BITSTREAM_EOS) {
                avcodec_send_packet(m_avDecContext, nullptr);
            }
        }

        // receive frame
        auto av_ret = avcodec_receive_frame(m_avDecContext, avframe);
        if (av_ret == 0) {
            // in case mjpeg, convert yuvj420p -> yuv420p
            if (m_avDecContext->codec_id == AV_CODEC_ID_MJPEG) {
                if (m_avDecContext->pix_fmt != AV_PIX_FMT_YUV420P) {
                    avframe = ConvertJPEGOutputColorSpace(avframe,
                                                          AV_PIX_FMT_YUV420P);
                    if (avframe == nullptr)
                        return MFX_ERR_ABORTED;
                }
            }
            if (m_param.mfx.FrameInfo.Width != m_avDecContext->width ||
                m_param.mfx.FrameInfo.Height != m_avDecContext->height) {
                m_param.mfx.FrameInfo.Width  = m_avDecContext->width;
                m_param.mfx.FrameInfo.Height = m_avDecContext->height;

                switch (m_avDecContext->pix_fmt) {
                    case AV_PIX_FMT_YUV420P10LE:
                        m_param.mfx.FrameInfo.FourCC = MFX_FOURCC_I010;
                        break;
                    case AV_PIX_FMT_YUV420P:
                    case AV_PIX_FMT_YUVJ420P:
                    default:
                        m_param.mfx.FrameInfo.FourCC = MFX_FOURCC_I420;
                        break;
                }
            }
            if (surface_out) {
                if (avframe == m_avDecFrameOut) { // copy image data
                    m_bFrameBuffered = true;
                    RET_ERROR(AVFrame2mfxFrameSurface(
                        surface_work,
                        m_avDecFrameOut,
                        m_session->GetFrameAllocator()));
                    m_bFrameBuffered = false;
                }
                else {
                    if (cpu_frame) { // update MFXFrameSurface from AVFrame
                        cpu_frame->Update();
                    }
                }
                *surface_out = surface_work;
            }
            return MFX_ERR_NONE;
        }
        if (av_ret == AVERROR(EAGAIN)) {
            if (bs && bs->DataLength) {
                continue; // we have more input data
            }
            else {
                if (bs &&
                    ((bs->DataFlag & MFX_BITSTREAM_EOS) == MFX_BITSTREAM_EOS)) {
                    //send a null packet and continue
                    avcodec_send_packet(m_avDecContext, nullptr);
                    continue;
                }
                else {
                    return MFX_ERR_MORE_DATA;
                }
            }
        }
        if (av_ret == AVERROR_EOF) {
            return MFX_ERR_MORE_DATA;
        }
        return MFX_ERR_ABORTED;
    }
}

AVFrame *CpuDecode::ConvertJPEGOutputColorSpace(AVFrame *avframe,
                                                AVPixelFormat target_pixfmt) {
    static int prev_w, prev_h;

    if (!m_swsContext ||
        (prev_w != avframe->width || prev_h != avframe->height)) {
        if (m_swsContext)
            sws_freeContext(m_swsContext);
        m_swsContext = sws_getContext(m_avDecContext->width,
                                      m_avDecContext->height,
                                      m_avDecContext->pix_fmt,
                                      m_avDecContext->width,
                                      m_avDecContext->height,
                                      target_pixfmt,
                                      SWS_BILINEAR,
                                      NULL,
                                      NULL,
                                      NULL);
        if (!m_swsContext) {
            return nullptr;
        }
    }

    int ret = sws_scale(m_swsContext,
                        avframe->data,
                        avframe->linesize,
                        0,
                        avframe->height,
                        avframe->data,
                        avframe->linesize);
    if (ret != avframe->height)
        return nullptr;
    else
        avframe->format = target_pixfmt;

    prev_w = avframe->width;
    prev_h = avframe->height;

    return avframe;
}

mfxStatus CpuDecode::DecodeQueryIOSurf(mfxVideoParam *par,
                                       mfxFrameAllocRequest *request) {
    // may be null for internal use
    if (par)
        request->Info = par->mfx.FrameInfo;
    else
        request->Info = { 0 };

    request->NumFrameMin       = 1;
    request->NumFrameSuggested = 3;
    request->Type = MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_FROM_DECODE;

    return MFX_ERR_NONE;
}

mfxStatus CpuDecode::DecodeQuery(mfxVideoParam *in, mfxVideoParam *out) {
    mfxStatus sts = MFX_ERR_NONE;

    if (in) {
        // save a local copy of in, since user may set out == in
        mfxVideoParam inCopy = *in;
        in                   = &inCopy;

        // start with out = copy of in (does not deep copy extBufs)
        *out = *in;

        // validate fields in the input param struct
        sts = ValidateDecodeParams(out, true);
        if (sts != MFX_ERR_NONE)
            return sts;
    }
    else {
        // set output struct to zero for unsupported params, non-zero for supported params
        *out                              = { 0 };
        out->mfx.CodecId                  = 0xFFFFFFFF;
        out->mfx.FrameInfo.BitDepthChroma = 0xFFFF;
        out->mfx.FrameInfo.Width          = 0xFFFF;
        out->mfx.FrameInfo.Height         = 0xFFFF;
        out->mfx.FrameInfo.CropW          = 0xFFFF;
        out->mfx.FrameInfo.CropH          = 0xFFFF;
        out->mfx.FrameInfo.FourCC         = 0xFFFFFFFF;
        out->mfx.CodecProfile             = 0xFFFF;
        out->mfx.CodecLevel               = 0xFFFF;
        out->IOPattern                    = 0xFFFF;
    }

    return sts;
}

// return free surface and set refCount to 1
mfxStatus CpuDecode::GetDecodeSurface(mfxFrameSurface1 **surface) {
    if (!m_decSurfaces) {
        mfxFrameAllocRequest DecRequest = { 0 };
        RET_ERROR(DecodeQueryIOSurf(&m_param, &DecRequest));

        auto pool = std::make_unique<CpuFramePool>();
        RET_ERROR(pool->Init(DecRequest.NumFrameSuggested));
        m_decSurfaces = std::move(pool);
    }

    return m_decSurfaces->GetFreeSurface(surface);
}

mfxStatus CpuDecode::GetVideoParam(mfxVideoParam *par) {
    par->mfx       = m_param.mfx;
    par->IOPattern = m_param.IOPattern;

    //If DecodeFrame() is not executed at all, we can't update params from m_avDecContext
    //but return current params
    if (!m_avDecContext->width && !m_avDecContext->height &&
        m_avDecContext->pix_fmt == AV_PIX_FMT_NONE) {
        if (!par->mfx.FrameInfo.FrameRateExtD &&
            !par->mfx.FrameInfo.FrameRateExtN) {
            par->mfx.FrameInfo.FrameRateExtN = 30;
            par->mfx.FrameInfo.FrameRateExtD = 1;
        }

        if (!par->mfx.FrameInfo.AspectRatioH &&
            !par->mfx.FrameInfo.AspectRatioW) {
            par->mfx.FrameInfo.AspectRatioH = 1;
            par->mfx.FrameInfo.AspectRatioW = 1;
        }
        return MFX_ERR_NONE;
    }

    //Get parameters from the decode context
    //This allows checking if parameters have
    //been effectively set
    par->mfx.CodecId = AVCodecID_to_MFXCodecId(m_avDecCodec->id);

    // resolution
    par->mfx.FrameInfo.Width  = (uint16_t)m_avDecContext->width;
    par->mfx.FrameInfo.Height = (uint16_t)m_avDecContext->height;
    par->mfx.FrameInfo.CropW  = (uint16_t)m_avDecContext->width;
    par->mfx.FrameInfo.CropH  = (uint16_t)m_avDecContext->height;

    // FourCC and chroma format
    switch (m_avDecContext->pix_fmt) {
        case AV_PIX_FMT_YUV420P10LE:
            par->mfx.FrameInfo.FourCC         = MFX_FOURCC_I010;
            par->mfx.FrameInfo.BitDepthLuma   = 10;
            par->mfx.FrameInfo.BitDepthChroma = 10;
            par->mfx.FrameInfo.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
            break;
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_YUVJ420P:
            par->mfx.FrameInfo.FourCC         = MFX_FOURCC_IYUV;
            par->mfx.FrameInfo.BitDepthLuma   = 8;
            par->mfx.FrameInfo.BitDepthChroma = 8;
            par->mfx.FrameInfo.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
            break;
        default:
            //zero value after decodeheader indicates that
            //a supported decode fourcc could not be found
            par->mfx.FrameInfo.FourCC = 0;
    }

    // Frame rate
    par->mfx.FrameInfo.FrameRateExtD = (uint16_t)m_avDecContext->framerate.num;
    par->mfx.FrameInfo.FrameRateExtN = (uint16_t)m_avDecContext->framerate.den;

    // Aspect ratio
    par->mfx.FrameInfo.AspectRatioW =
        (uint16_t)m_avDecContext->sample_aspect_ratio.num;
    par->mfx.FrameInfo.AspectRatioH =
        (uint16_t)m_avDecContext->sample_aspect_ratio.den;

    // Profile/Level
    int profile = m_avDecContext->profile;
    int level   = m_avDecContext->level;

    switch (par->mfx.CodecId) {
        case MFX_CODEC_AV1:
            //if (profile==FF_PROFILE_AV1_MAIN)
            //if (profile==FF_PROFILE_AV1_HIGH)
            break;

        case MFX_CODEC_HEVC:
            if (profile == FF_PROFILE_HEVC_MAIN)
                par->mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN;
            else if (profile == FF_PROFILE_HEVC_MAIN_10)
                par->mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN10;

            //TODO(jeff) check if true for all levels
            par->mfx.CodecLevel = level;
            break;

        case MFX_CODEC_AVC:
            if (profile == FF_PROFILE_H264_BASELINE)
                par->mfx.CodecProfile = MFX_PROFILE_AVC_BASELINE;
            if (profile == FF_PROFILE_H264_MAIN)
                par->mfx.CodecProfile = MFX_PROFILE_AVC_MAIN;
            if (profile == FF_PROFILE_H264_HIGH)
                par->mfx.CodecProfile = MFX_PROFILE_AVC_HIGH;

            //TODO(jeff) check if true for all levels
            par->mfx.CodecLevel = level;

            break;
        case MFX_CODEC_JPEG:
            if (profile == FF_PROFILE_MJPEG_HUFFMAN_BASELINE_DCT)
                par->mfx.CodecProfile = MFX_PROFILE_JPEG_BASELINE;
            break;
        default:
            par->mfx.CodecProfile = 0;
            par->mfx.CodecLevel   = 0;
            return MFX_ERR_NONE;
    }

    par->IOPattern = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    return MFX_ERR_NONE;
}

mfxStatus CpuDecode::CheckVideoParamDecoders(mfxVideoParam *in) {
    mfxStatus sts = CheckVideoParamCommon(in);
    RET_ERROR(sts);

    if (in->IOPattern != MFX_IOPATTERN_OUT_SYSTEM_MEMORY) {
        if (in->IOPattern == 0x40) { //MFX_IOPATTERN_OUT_OPAQUE_MEMORY
            return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
        }
        else {
            return MFX_ERR_INVALID_VIDEO_PARAM;
        }
    }

    if (in->mfx.DecodedOrder)
        return MFX_ERR_UNSUPPORTED;

    if (in->NumExtParam)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    return MFX_ERR_NONE;
}

mfxStatus CpuDecode::IsSameVideoParam(mfxVideoParam *newPar,
                                      mfxVideoParam *oldPar) {
    if ((newPar->IOPattern & MFX_IOPATTERN_OUT_SYSTEM_MEMORY) !=
        (oldPar->IOPattern & MFX_IOPATTERN_OUT_SYSTEM_MEMORY)) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (newPar->AsyncDepth != oldPar->AsyncDepth) {
        return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
    }

    if (newPar->mfx.FrameInfo.Width > oldPar->mfx.FrameInfo.Width) {
        return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
    }
    else if (newPar->mfx.FrameInfo.Width < oldPar->mfx.FrameInfo.Width) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (newPar->mfx.FrameInfo.Height > oldPar->mfx.FrameInfo.Height) {
        return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
    }
    else if (newPar->mfx.FrameInfo.Height < oldPar->mfx.FrameInfo.Height) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (newPar->mfx.FrameInfo.FourCC != oldPar->mfx.FrameInfo.FourCC) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (newPar->mfx.FrameInfo.ChromaFormat !=
        oldPar->mfx.FrameInfo.ChromaFormat) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    mfxFrameAllocRequest requestOld = { 0 };
    mfxFrameAllocRequest requestNew = { 0 };

    mfxStatus mfxSts = DecodeQueryIOSurf(oldPar, &requestOld);
    if (mfxSts != MFX_ERR_NONE)
        return mfxSts;

    mfxSts = DecodeQueryIOSurf(newPar, &requestNew);
    if (mfxSts != MFX_ERR_NONE)
        return mfxSts;

    if (requestNew.NumFrameMin > requestOld.NumFrameMin ||
        requestNew.Type != requestOld.Type) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    return MFX_ERR_NONE;
}