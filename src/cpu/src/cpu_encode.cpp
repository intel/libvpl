/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_encode.h"
#include <memory>
#include <sstream>
#include "src/cpu_workstream.h"

#define X264_DEFAULT_QUALITY_VALUE 23

CpuEncode::CpuEncode(CpuWorkstream *session)
        : m_session(session),
          m_avEncCodec(nullptr),
          m_avEncContext(nullptr),
          m_avEncPacket(nullptr),
          m_param({}),
          m_encSurfaces() {}

mfxStatus CpuEncode::ValidateEncodeParams(mfxVideoParam *par) {
    if (par->mfx.FrameInfo.FourCC) {
        if (par->mfx.FrameInfo.FourCC != MFX_FOURCC_I420 &&
            par->mfx.FrameInfo.FourCC != MFX_FOURCC_I010)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else {
        par->mfx.FrameInfo.FourCC = MFX_FOURCC_I420;
    }

    // validate fields in the input param struct
    if (par->mfx.CodecId != MFX_CODEC_HEVC &&
        par->mfx.CodecId != MFX_CODEC_AVC &&
        par->mfx.CodecId != MFX_CODEC_JPEG && par->mfx.CodecId != MFX_CODEC_AV1)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    // check codec id and the values
    switch (par->mfx.CodecId) {
        case MFX_CODEC_AVC: // leave this for later
        {
            if (par->mfx.FrameInfo.Width < 64 ||
                par->mfx.FrameInfo.Width > 4096)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.FrameInfo.Height < 64 ||
                par->mfx.FrameInfo.Height > 2304)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.RateControlMethod) {
                if (par->mfx.RateControlMethod != MFX_RATECONTROL_CQP &&
                    par->mfx.RateControlMethod != MFX_RATECONTROL_CBR &&
                    par->mfx.RateControlMethod != MFX_RATECONTROL_VBR)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.RateControlMethod = MFX_RATECONTROL_VBR;
            }

            if (par->mfx.RateControlMethod == MFX_RATECONTROL_CQP)
                if (par->mfx.QPI > 51 || par->mfx.QPP > 51 || par->mfx.QPB > 51)
                    return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.CodecProfile) {
                if (par->mfx.FrameInfo.FourCC == MFX_FOURCC_I010) {
                    if (par->mfx.CodecProfile != MFX_PROFILE_AVC_HIGH10 &&
                        par->mfx.CodecProfile != MFX_PROFILE_AVC_HIGH_422)
                        return MFX_ERR_INVALID_VIDEO_PARAM;
                }
                else {
                    if (par->mfx.CodecProfile != MFX_PROFILE_AVC_BASELINE &&
                        par->mfx.CodecProfile != MFX_PROFILE_AVC_MAIN &&
                        par->mfx.CodecProfile != MFX_PROFILE_AVC_HIGH)
                        return MFX_ERR_INVALID_VIDEO_PARAM;
                }
            }
            else {
                if (par->mfx.FrameInfo.FourCC == MFX_FOURCC_I010)
                    par->mfx.CodecProfile = MFX_PROFILE_AVC_HIGH10;
                else
                    par->mfx.CodecProfile = MFX_PROFILE_AVC_HIGH;
            }

            if (par->mfx.CodecLevel) {
                if (par->mfx.CodecLevel != MFX_LEVEL_AVC_1 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_1b &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_11 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_12 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_13 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_2 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_21 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_22 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_3 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_31 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_32 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_4 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_41 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_42 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_5 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_51 &&
                    par->mfx.CodecLevel != MFX_LEVEL_AVC_52)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }

            //Update AVC level to match resolution
            int MBsTotal =
                par->mfx.FrameInfo.Width * par->mfx.FrameInfo.Height / 256;
            int recommendedLevel;
            if (MBsTotal <= 99)
                recommendedLevel = MFX_LEVEL_AVC_1b;
            else if (MBsTotal <= 396)
                recommendedLevel = MFX_LEVEL_AVC_2;
            else if (MBsTotal <= 792)
                recommendedLevel = MFX_LEVEL_AVC_21;
            else if (MBsTotal <= 1620)
                recommendedLevel = MFX_LEVEL_AVC_3;
            else if (MBsTotal <= 3600)
                recommendedLevel = MFX_LEVEL_AVC_31;
            else if (MBsTotal <= 5120)
                recommendedLevel = MFX_LEVEL_AVC_32;
            else if (MBsTotal <= 8192)
                recommendedLevel = MFX_LEVEL_AVC_41;
            else if (MBsTotal <= 8704)
                recommendedLevel = MFX_LEVEL_AVC_42;
            else if (MBsTotal <= 22080)
                recommendedLevel = MFX_LEVEL_AVC_5;
            else if (MBsTotal <= 36864)
                recommendedLevel = MFX_LEVEL_AVC_52;
            else
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.CodecLevel != recommendedLevel) {
                par->mfx.CodecLevel = recommendedLevel;
                return MFX_WRN_INCOMPATIBLE_VIDEO_PARAM;
            }
        }

        break;
        case MFX_CODEC_HEVC:
            // default: vbr, profile main, level 3.1
            if (par->mfx.FrameInfo.Width < 64 ||
                par->mfx.FrameInfo.Width > 8192)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.FrameInfo.Height < 64 ||
                par->mfx.FrameInfo.Height > 4320)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.RateControlMethod) {
                if (par->mfx.RateControlMethod != MFX_RATECONTROL_CQP &&
                    par->mfx.RateControlMethod != MFX_RATECONTROL_VBR)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.RateControlMethod = MFX_RATECONTROL_VBR;
            }

            if (par->mfx.RateControlMethod == MFX_RATECONTROL_CQP)
                if (par->mfx.QPI > 51 || par->mfx.QPP > 51 || par->mfx.QPB > 51)
                    return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.CodecProfile) {
                if (par->mfx.CodecProfile != MFX_PROFILE_HEVC_MAIN &&
                    par->mfx.CodecProfile != MFX_PROFILE_HEVC_MAIN10)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN;
            }

            if (par->mfx.CodecLevel) {
                if (par->mfx.CodecLevel != MFX_LEVEL_HEVC_1 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_2 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_21 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_3 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_31 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_4 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_41 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_5 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_51 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_52 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_6 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_61 &&
                    par->mfx.CodecLevel != MFX_LEVEL_HEVC_62)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.CodecLevel = MFX_LEVEL_HEVC_31;
            }

            break;
        case MFX_CODEC_JPEG:
            // default: baseline
            if (par->mfx.FrameInfo.Width < 64 ||
                par->mfx.FrameInfo.Width > 8192)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.FrameInfo.Height < 64 ||
                par->mfx.FrameInfo.Height > 8192)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.Quality) {
                if (par->mfx.Quality < 1 || par->mfx.Quality > 100)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.Quality = 80;
            }

            if (par->mfx.CodecProfile) {
                if (par->mfx.CodecProfile != MFX_PROFILE_JPEG_BASELINE)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.CodecProfile = MFX_PROFILE_JPEG_BASELINE;
            }

            break;
        case MFX_CODEC_AV1:
            // default: VBR
            if (par->mfx.FrameInfo.Width < 64 ||
                par->mfx.FrameInfo.Width > 4096)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.FrameInfo.Height < 64 ||
                par->mfx.FrameInfo.Height > 2304)
                return MFX_ERR_INVALID_VIDEO_PARAM;

            if (par->mfx.RateControlMethod) {
                if (par->mfx.RateControlMethod != MFX_RATECONTROL_CQP &&
                    par->mfx.RateControlMethod != MFX_RATECONTROL_CBR &&
                    par->mfx.RateControlMethod != MFX_RATECONTROL_VBR)
                    return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            else {
                par->mfx.RateControlMethod = MFX_RATECONTROL_VBR;
            }

            if (par->mfx.RateControlMethod == MFX_RATECONTROL_CQP)
                if (par->mfx.QPI > 63 || par->mfx.QPP > 63 || par->mfx.QPB > 63)
                    return MFX_ERR_INVALID_VIDEO_PARAM;

            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (par->mfx.TargetKbps == 0)
        par->mfx.TargetKbps = 4000;

    if (par->mfx.FrameInfo.FrameRateExtN == 0)
        par->mfx.FrameInfo.FrameRateExtN = 30;

    if (par->mfx.FrameInfo.FrameRateExtN > 65535)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->mfx.FrameInfo.FrameRateExtD == 0)
        par->mfx.FrameInfo.FrameRateExtD = 1;

    if (par->mfx.FrameInfo.FrameRateExtD > 65535)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->mfx.FrameInfo.AspectRatioW == 0)
        par->mfx.FrameInfo.AspectRatioW = 1;

    if (par->mfx.FrameInfo.AspectRatioH == 0)
        par->mfx.FrameInfo.AspectRatioH = 1;

    if (par->mfx.GopOptFlag != 0 && par->mfx.GopOptFlag != MFX_GOP_CLOSED)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->mfx.FrameInfo.BitDepthChroma) {
        if (par->mfx.FrameInfo.BitDepthChroma != 8 &&
            par->mfx.FrameInfo.BitDepthChroma != 10)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else {
        par->mfx.FrameInfo.BitDepthChroma = 8;
    }

    if (par->mfx.FrameInfo.BitDepthLuma) {
        if (par->mfx.FrameInfo.BitDepthLuma != 8 &&
            par->mfx.FrameInfo.BitDepthLuma != 10)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else {
        par->mfx.FrameInfo.BitDepthLuma = 8;
    }

    if (par->mfx.TargetUsage) {
        if (par->mfx.TargetUsage < MFX_TARGETUSAGE_1 ||
            par->mfx.TargetUsage > MFX_TARGETUSAGE_7)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else {
        par->mfx.TargetUsage = MFX_TARGETUSAGE_BALANCED;
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuEncode::InitEncode(mfxVideoParam *par) {
    m_param = *par;
    par     = &m_param;

    RET_ERROR(ValidateEncodeParams(par));

    AVCodecID cid = MFXCodecId_to_AVCodecID(m_param.mfx.CodecId);
    RET_IF_FALSE(cid, MFX_ERR_INVALID_VIDEO_PARAM);

    m_avEncCodec = avcodec_find_encoder(cid);
    RET_IF_FALSE(m_avEncCodec, MFX_ERR_INVALID_VIDEO_PARAM);
    VPL_DEBUG_MESSAGE("AVCodec encoder name=" +
                      std::string(m_avEncCodec->name));

    m_avEncContext = avcodec_alloc_context3(m_avEncCodec);
    RET_IF_FALSE(m_avEncContext, MFX_ERR_MEMORY_ALLOC);

    m_avEncPacket = av_packet_alloc();
    RET_IF_FALSE(m_avEncPacket, MFX_ERR_MEMORY_ALLOC);

    //------------------------------
    // Set general libav parameters
    // values not set in mfxVideoParam should keep defaults
    //------------------------------
    m_avEncContext->width  = par->mfx.FrameInfo.Width;
    m_avEncContext->height = par->mfx.FrameInfo.Height;

    m_avEncContext->gop_size     = par->mfx.GopPicSize;
    m_avEncContext->max_b_frames = par->mfx.GopRefDist;
    m_avEncContext->bit_rate = par->mfx.TargetKbps * 1000; // prop is in kbps;

    m_avEncContext->framerate.num = par->mfx.FrameInfo.FrameRateExtN;
    m_avEncContext->framerate.den = par->mfx.FrameInfo.FrameRateExtD;
    m_avEncContext->time_base.num = par->mfx.FrameInfo.FrameRateExtD;
    m_avEncContext->time_base.den = par->mfx.FrameInfo.FrameRateExtN;

    m_avEncContext->sample_aspect_ratio.num = par->mfx.FrameInfo.AspectRatioH;
    m_avEncContext->sample_aspect_ratio.den = par->mfx.FrameInfo.AspectRatioW;

    m_avEncContext->slices = par->mfx.NumSlice;
    m_avEncContext->refs   = par->mfx.NumRefFrame;

    if (par->mfx.GopOptFlag == MFX_GOP_CLOSED)
        m_avEncContext->flags &= AV_CODEC_FLAG_CLOSED_GOP;

    if (par->mfx.FrameInfo.BitDepthChroma == 10) {
        // Main10: 10-bit 420
        m_avEncContext->pix_fmt = AV_PIX_FMT_YUV420P10;
    }
    else {
        // default: 8-bit 420
        if (par->mfx.CodecId == MFX_CODEC_JPEG)
            m_avEncContext->pix_fmt = AV_PIX_FMT_YUVJ420P;
        else
            m_avEncContext->pix_fmt = AV_PIX_FMT_YUV420P;
    }

    if (m_avEncContext->pix_fmt == AV_PIX_FMT_YUV420P10LE)
        m_param.mfx.FrameInfo.FourCC = MFX_FOURCC_I010;
    else if (m_avEncContext->pix_fmt == AV_PIX_FMT_YUV420P)
        m_param.mfx.FrameInfo.FourCC = MFX_FOURCC_I420;
    else
        m_param.mfx.FrameInfo.FourCC = MFX_FOURCC_I420;

    // set defaults for anything not passed in
    if (!m_avEncContext->gop_size)
        m_avEncContext->gop_size =
            2 *
            static_cast<int>(static_cast<float>(m_avEncContext->framerate.num) /
                             m_avEncContext->framerate.den);

    switch (m_param.mfx.CodecId) {
        case MFX_CODEC_HEVC:
            if (m_avEncCodec->name != std::string("libx265")) {
                RET_ERROR(InitHEVCParams(par)); // SVT specific params
            }
            break;
        case MFX_CODEC_AV1:
            RET_ERROR(InitAV1Params(par));
            break;
        case MFX_CODEC_AVC:
            RET_ERROR(InitAVCParams(par));
            break;
        case MFX_CODEC_JPEG:
            RET_ERROR(InitJPEGParams(par));
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

#ifdef ENABLE_LIBAV_AUTO_THREADS
    m_avEncContext->thread_count = 0;
#endif

    int err = 0;
    err     = avcodec_open2(m_avEncContext, m_avEncCodec, NULL);
    RET_IF_FALSE(err == 0, MFX_ERR_INVALID_VIDEO_PARAM);

    if (!m_param.mfx.BufferSizeInKB) {
        // TODO(estimate better based on RateControlMethod)
        m_param.mfx.BufferSizeInKB = m_param.mfx.TargetKbps;
    }

    return MFX_ERR_NONE;
}

//utility function to convert between TargetUsage/Encode Mode
int CpuEncode::convertTargetUsageVal(int val,
                                     int minIn,
                                     int maxIn,
                                     int minOut,
                                     int maxOut) {
    int rangeIn  = maxIn - minIn;
    int rangeOut = maxOut - minOut;
    double ratio = (double)rangeOut / (double)rangeIn;
    double val1  = (double)val - minIn;
    int outval   = (int)(val1 * ratio + .49) + minOut;
    if (outval > maxOut)
        outval = maxOut;
    if (outval < minOut)
        outval = minOut;
    return outval;
}

mfxStatus CpuEncode::InitHEVCParams(mfxVideoParam *par) {
    int ret;

    // set rate control
    if (par->mfx.RateControlMethod == MFX_RATECONTROL_CQP) {
        // SVT-HEVC rc 0=CBR
        ret = av_opt_set(m_avEncContext->priv_data,
                         "rc",
                         "0",
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        // since SVT-HEVC does not distinguish between QPI/P/B, use QPP value
        std::stringstream qpss;
        qpss << par->mfx.QPP;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "qp",
                         qpss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else {
        //SVT-HEVC rc 1=VBR
        ret = av_opt_set(m_avEncContext->priv_data,
                         "rc",
                         "1",
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        m_avEncContext->bit_rate =
            par->mfx.TargetKbps * 1000; // prop is in kbps;

        m_avEncContext->rc_initial_buffer_occupancy =
            par->mfx.InitialDelayInKB * 8000;
        m_avEncContext->rc_buffer_size = par->mfx.BufferSizeInKB * 1000;
        m_avEncContext->rc_max_rate    = par->mfx.MaxKbps * 1000;
    }

    if (par->mfx.TargetUsage) {
        // set targetUsage
        // note, HEVC encode can be 0-9 for <=1080p
        // 0-10 for 1082-4k
        // and 0-11 for >=4K
        // however, in the most common cases we don't know the resolution yet
        int encMode = 0;
        switch (par->mfx.TargetUsage) {
            case 1:
                encMode = 0;
                break;
            case 2:
                encMode = 1;
                break;
            case 3:
                encMode = 3;
                break;
            case 4:
                encMode = 5;
                break;
            case 5:
                encMode = 7;
                break;
            case 6:
                encMode = 8;
                break;
            case 7:
            default:
                encMode = 9;
                break;
        }
        std::stringstream tuss;
        tuss << encMode;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "preset",
                         tuss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (par->mfx.CodecProfile) {
        //       **Profile** | -profile | [1,2] | 2 | 1: Main, 2: Main 10 |
        ret = av_opt_set_int(
            m_avEncContext->priv_data,
            "profile",
            (par->mfx.CodecProfile == MFX_PROFILE_HEVC_MAIN10) ? 2 : 1,
            AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (par->mfx.CodecLevel) {
        //   int tier; | **Tier** | -tier | [0, 1] | 0 | 0: Main, 1: High |
        //   int level; | **Level** | -level | [1, 2, 2.1,3, 3.1, 4, 4.1, 5, 5.1, 5.2, 6, 6.1, 6.2] | 0 | 0 to 6.2 [0 for auto determine Level] |

        //   In MSDK, tier is combined with level.  In SVT-HEVC it is set
        //   separately.
        std::stringstream tierss;
        tierss << (par->mfx.CodecLevel & MFX_TIER_HEVC_HIGH) ? 1 : 0;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "tier",
                         tierss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        std::string levelstr;
        uint8_t level = par->mfx.CodecLevel & 0xFF;
        switch (level) {
            case MFX_LEVEL_HEVC_1:
                levelstr = "1";
                break;
            case MFX_LEVEL_HEVC_2:
                levelstr = "2";
                break;
            case MFX_LEVEL_HEVC_21:
                levelstr = "2.1";
                break;
            case MFX_LEVEL_HEVC_3:
                levelstr = "3";
                break;
            case MFX_LEVEL_HEVC_31:
                levelstr = "3.1";
                break;
            case MFX_LEVEL_HEVC_4:
                levelstr = "4";
                break;
            case MFX_LEVEL_HEVC_41:
                levelstr = "4.1";
                break;
            case MFX_LEVEL_HEVC_5:
                levelstr = "5";
                break;
            case MFX_LEVEL_HEVC_51:
                levelstr = "5.1";
                break;
            case MFX_LEVEL_HEVC_52:
                levelstr = "5.2";
                break;
            case MFX_LEVEL_HEVC_6:
                levelstr = "6";
                break;
            case MFX_LEVEL_HEVC_61:
                levelstr = "6.1";
                break;
            case MFX_LEVEL_HEVC_62:
                levelstr = "6.2";
                break;
            default:
                return MFX_ERR_INVALID_VIDEO_PARAM;
                break;
        }
        ret = av_opt_set(m_avEncContext->priv_data,
                         "level",
                         tierss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuEncode::GetHEVCParams(mfxVideoParam *par) {
    int ret;
    int64_t optval;
    ret = av_opt_get_int(m_avEncContext->priv_data,
                         "rc",
                         AV_OPT_SEARCH_CHILDREN,
                         &optval);
    if (optval == 0) {
        par->mfx.RateControlMethod = MFX_RATECONTROL_CQP;
        int64_t qpval;
        ret          = av_opt_get_int(m_avEncContext->priv_data,
                             "qp",
                             AV_OPT_SEARCH_CHILDREN,
                             &qpval);
        par->mfx.QPP = qpval;
    }
    else {
        par->mfx.RateControlMethod = MFX_RATECONTROL_VBR;
        if (m_avEncContext->bit_rate) {
            par->mfx.TargetKbps = m_avEncContext->bit_rate / 1000;
        }
        if (m_avEncContext->rc_initial_buffer_occupancy) {
            par->mfx.InitialDelayInKB =
                m_avEncContext->rc_initial_buffer_occupancy / 8000;
        }
        if (m_avEncContext->rc_buffer_size) {
            par->mfx.BufferSizeInKB = m_avEncContext->rc_buffer_size / 1000;
        }
        if (m_avEncContext->rc_max_rate) {
            par->mfx.MaxKbps = m_avEncContext->rc_max_rate / 1000;
        }
    }

    ret = av_opt_get_int(m_avEncContext->priv_data,
                         "preset",
                         AV_OPT_SEARCH_CHILDREN,
                         &optval);
    if (ret == 0) {
        par->mfx.TargetUsage = convertTargetUsageVal((int)optval, 0, 9, 1, 7);
    }

    ret = av_opt_get_int(m_avEncContext->priv_data,
                         "profile",
                         AV_OPT_SEARCH_CHILDREN,
                         &optval);
    if (ret == 0) {
        switch (optval) {
            case 2:
                par->mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN10;
                break;
            case 1:
            default:
                par->mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN;
        }
    }

    int64_t tierval = MFX_TIER_HEVC_MAIN;
    ret             = av_opt_get_int(m_avEncContext->priv_data,
                         "tier",
                         AV_OPT_SEARCH_CHILDREN,
                         &tierval);

    ret = av_opt_get_int(m_avEncContext->priv_data,
                         "level",
                         AV_OPT_SEARCH_CHILDREN,
                         &optval);

    if (ret == 0) {
        par->mfx.CodecLevel = optval | tierval;
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuEncode::InitAVCParams(mfxVideoParam *par) {
    int ret;
    if (par->mfx.RateControlMethod == MFX_RATECONTROL_CQP) {
        std::stringstream qpss;
        qpss << par->mfx.QPI;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "qp",
                         qpss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else {
        // default to VBR
        std::stringstream crfss;
        crfss << X264_DEFAULT_QUALITY_VALUE;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "crf",
                         crfss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (par->mfx.TargetUsage) {
        std::string encMode;
        switch (par->mfx.TargetUsage) {
            case 1:
                encMode = "veryslow";
                break;
            case 2:
                encMode = "slower";
                break;
            case 3:
                encMode = "slow";
                break;
            case 5:
                encMode = "veryfast";
                break;
            case 6:
                encMode = "superfast";
                ;
                break;
            case 7:
                encMode = "ultrafast";
                break;
            case 4:
            default:
                encMode = "medium";
                break;
        }
        std::stringstream tuss;
        tuss << encMode;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "preset",
                         tuss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (par->mfx.CodecProfile) {
        std::string profValue;
        switch (par->mfx.CodecProfile) {
            case MFX_PROFILE_AVC_HIGH10:
                profValue = "high10";
                break;
            case MFX_PROFILE_AVC_HIGH_422:
                profValue = "high422";
                break;
            case MFX_PROFILE_AVC_BASELINE:
                profValue = "baseline";
                break;
            case MFX_PROFILE_AVC_MAIN:
                profValue = "main";
                break;
            case MFX_PROFILE_AVC_HIGH:
            default:
                profValue = "high";
                break;
        }
        std::stringstream profss;
        profss << profValue;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "profile",
                         profss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (par->mfx.CodecLevel) {
        std::string levelstr;
        uint8_t level = par->mfx.CodecLevel & 0xFF;
        switch (level) {
            case MFX_LEVEL_AVC_1:
                levelstr = "1";
                break;
            case MFX_LEVEL_AVC_1b:
                levelstr = "9";
                break;
            case MFX_LEVEL_AVC_11:
                levelstr = "1.1";
                break;
            case MFX_LEVEL_AVC_12:
                levelstr = "1.2";
                break;
            case MFX_LEVEL_AVC_13:
                levelstr = "1.3";
                break;
            case MFX_LEVEL_AVC_2:
                levelstr = "2";
                break;
            case MFX_LEVEL_AVC_21:
                levelstr = "2.1";
                break;
            case MFX_LEVEL_AVC_22:
                levelstr = "2.2";
                break;
            case MFX_LEVEL_AVC_3:
                levelstr = "3";
                break;
            case MFX_LEVEL_AVC_31:
                levelstr = "3.1";
            case MFX_LEVEL_AVC_32:
                levelstr = "3.2";
                break;
            case MFX_LEVEL_AVC_4:
                levelstr = "4";
                break;
            case MFX_LEVEL_AVC_41:
                levelstr = "4.1";
                break;
            case MFX_LEVEL_AVC_42:
                levelstr = "4.2";
                break;
            case MFX_LEVEL_AVC_5:
                levelstr = "5";
                break;
            case MFX_LEVEL_AVC_51:
                levelstr = "5.1";
                break;
            case MFX_LEVEL_AVC_52:
                levelstr = "5.2";
                break;
            default:
                return MFX_ERR_INVALID_VIDEO_PARAM;
                break;
        }
        std::stringstream lvls;
        lvls << levelstr;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "level",
                         lvls.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuEncode::GetAVCParams(mfxVideoParam *par) {
    int ret;
    int64_t optval;
    ret = av_opt_get_int(m_avEncContext->priv_data,
                         "rc",
                         AV_OPT_SEARCH_CHILDREN,
                         &optval);
    if (optval == 0) {
        par->mfx.RateControlMethod = MFX_RATECONTROL_CQP;
        int64_t qpval;
        ret          = av_opt_get_int(m_avEncContext->priv_data,
                             "qp",
                             AV_OPT_SEARCH_CHILDREN,
                             &qpval);
        par->mfx.QPP = qpval;
    }
    else {
        par->mfx.RateControlMethod = MFX_RATECONTROL_VBR;
        if (m_avEncContext->bit_rate) {
            par->mfx.TargetKbps = m_avEncContext->bit_rate / 1000;
        }
        if (m_avEncContext->rc_initial_buffer_occupancy) {
            par->mfx.InitialDelayInKB =
                m_avEncContext->rc_initial_buffer_occupancy / 8000;
        }
        if (m_avEncContext->rc_buffer_size) {
            par->mfx.BufferSizeInKB = m_avEncContext->rc_buffer_size / 1000;
        }
        if (m_avEncContext->rc_max_rate) {
            par->mfx.MaxKbps = m_avEncContext->rc_max_rate / 1000;
        }
    }

    uint8_t *presetval = new uint8_t[2048];
    ret                = av_opt_get(m_avEncContext->priv_data,
                     "preset",
                     AV_OPT_SEARCH_CHILDREN,
                     &presetval);
    std::string presetstr((char *)presetval);
    int tu = 4;
    if (presetstr == "veryslow")
        tu = 1;
    if (presetstr == "slower")
        tu = 2;
    if (presetstr == "slow")
        tu = 3;
    if (presetstr == "medium")
        tu = 4;
    if (presetstr == "veryfast")
        tu = 5;
    if (presetstr == "superfast")
        tu = 6;
    if (presetstr == "ultrafast")
        tu = 7;
    par->mfx.TargetUsage = tu;
    delete[] presetval;

    return MFX_ERR_NONE;
}

mfxStatus CpuEncode::InitJPEGParams(mfxVideoParam *par) {
    if (par->mfx.Quality) {
        uint32_t jpegQuality;

        // convert scale from 1 - 100 (VPL, worst to best) to
        //   2 - 31 (ffmpeg, best to worst)
        float q = (float)(par->mfx.Quality);
        q       = 31 - ((31 - 2) * (q - 1) / (100 - 1));

        jpegQuality = (int)(q + 0.5f);
        if (jpegQuality < 2)
            jpegQuality = 2;
        if (jpegQuality > 31)
            jpegQuality = 31;

        // enable CQP for MJPEG
        m_avEncContext->flags |= AV_CODEC_FLAG_QSCALE;
        m_avEncContext->global_quality = jpegQuality * FF_QP2LAMBDA;
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuEncode::GetJPEGParams(mfxVideoParam *par) {
    if (m_avEncContext->global_quality) {
        par->mfx.Quality = m_avEncContext->global_quality / FF_QP2LAMBDA;
    }
    return MFX_ERR_NONE;
}

mfxStatus CpuEncode::InitAV1Params(mfxVideoParam *par) {
    int ret;

    // set AV1 rate control (0=CQP, 1=VBR, 2 = CVBR)
    if (par->mfx.RateControlMethod == MFX_RATECONTROL_CQP) {
        //SVT-AV1 rc 0=CBR
        ret = av_opt_set(m_avEncContext->priv_data,
                         "rc",
                         "0",
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        // since SVT-AV1 does not distinguish between QPI/P/B, use the QPP value
        std::stringstream qpss;
        qpss << par->mfx.QPP;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "qp",
                         qpss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else if (par->mfx.RateControlMethod == MFX_RATECONTROL_CBR) {
        // since CVBR is available using this as a slightly better map than just
        // defaulting to VBR

        // SVT-AV1 rc 2=CVBR
        ret = av_opt_set(m_avEncContext->priv_data,
                         "rc",
                         "2",
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        m_avEncContext->bit_rate =
            par->mfx.TargetKbps * 1000; // prop is in kbps;

        m_avEncContext->rc_initial_buffer_occupancy =
            par->mfx.InitialDelayInKB * 8000;
        m_avEncContext->rc_buffer_size = par->mfx.BufferSizeInKB * 1000;
        m_avEncContext->rc_max_rate    = par->mfx.MaxKbps * 1000;
    }
    else {
        // default to VBR
        // SVT-HEVC rc 1=VBR
        ret = av_opt_set(m_avEncContext->priv_data,
                         "rc",
                         "1",
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;

        m_avEncContext->bit_rate =
            par->mfx.TargetKbps * 1000; // prop is in kbps;

        m_avEncContext->rc_initial_buffer_occupancy =
            par->mfx.InitialDelayInKB * 8000;
        m_avEncContext->rc_buffer_size = par->mfx.BufferSizeInKB * 1000;
        m_avEncContext->rc_max_rate    = par->mfx.MaxKbps * 1000;
    }

    // set targetUsage
    // note, AV1 encode can be 0-8
    if (par->mfx.TargetUsage) {
        int encMode = 0;
        switch (par->mfx.TargetUsage) {
            case 1:
                encMode = 0;
                break;
            case 2:
                encMode = 1;
                break;
            case 3:
                encMode = 3;
                break;
            case 4:
                encMode = 5;
                break;
            case 5:
                encMode = 6;
                break;
            case 6:
                encMode = 7;
                break;
            case 7:
            default:
                encMode = 8;
                break;
        }
        std::stringstream tuss;
        tuss << encMode;
        ret = av_opt_set(m_avEncContext->priv_data,
                         "preset",
                         tuss.str().c_str(),
                         AV_OPT_SEARCH_CHILDREN);
        if (ret)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuEncode::GetAV1Params(mfxVideoParam *par) {
    int ret;
    int64_t optval;
    ret = av_opt_get_int(m_avEncContext->priv_data,
                         "rc",
                         AV_OPT_SEARCH_CHILDREN,
                         &optval);
    if (optval == 0) {
        par->mfx.RateControlMethod = MFX_RATECONTROL_CQP;
        int64_t qpval;
        ret          = av_opt_get_int(m_avEncContext->priv_data,
                             "qp",
                             AV_OPT_SEARCH_CHILDREN,
                             &qpval);
        par->mfx.QPP = qpval;
    }
    else {
        if (optval == 2) {
            par->mfx.RateControlMethod = MFX_RATECONTROL_CBR;
        }
        else {
            par->mfx.RateControlMethod = MFX_RATECONTROL_VBR;
        }
        if (m_avEncContext->bit_rate) {
            par->mfx.TargetKbps = m_avEncContext->bit_rate / 1000;
        }
        if (m_avEncContext->rc_initial_buffer_occupancy) {
            par->mfx.InitialDelayInKB =
                m_avEncContext->rc_initial_buffer_occupancy / 8000;
        }
        if (m_avEncContext->rc_buffer_size) {
            par->mfx.BufferSizeInKB = m_avEncContext->rc_buffer_size / 1000;
        }
        if (m_avEncContext->rc_max_rate) {
            par->mfx.MaxKbps = m_avEncContext->rc_max_rate / 1000;
        }
    }

    ret = av_opt_get_int(m_avEncContext->priv_data,
                         "preset",
                         AV_OPT_SEARCH_CHILDREN,
                         &optval);
    if (ret == 0) {
        par->mfx.TargetUsage = convertTargetUsageVal((int)optval, 0, 8, 1, 7);
    }

    return MFX_ERR_NONE;
}

CpuEncode::~CpuEncode() {
    // drain encoder - workaround for SVT encoder hang on avcodec_close
    mfxBitstream bs{};
    mfxStatus sts;
    do {
        sts = EncodeFrame(nullptr, &bs);
    } while (sts == MFX_ERR_NOT_ENOUGH_BUFFER || sts == MFX_ERR_NONE);

    if (m_avEncContext) {
        avcodec_close(m_avEncContext);
        avcodec_free_context(&m_avEncContext);
        m_avEncContext = nullptr;
    }

    if (m_avEncPacket) {
        av_packet_free(&m_avEncPacket);
        m_avEncPacket = nullptr;
    }
}

mfxStatus CpuEncode::EncodeFrame(mfxFrameSurface1 *surface, mfxBitstream *bs) {
    RET_IF_FALSE(m_avEncContext, MFX_ERR_NOT_INITIALIZED);
    int err;

    // encode one frame
    if (surface) {
        AVFrame *av_frame =
            m_input_locker.GetAVFrame(surface,
                                      MFX_MAP_READ,
                                      m_session->GetFrameAllocator());
        RET_IF_FALSE(av_frame, MFX_ERR_ABORTED);

        if (m_param.mfx.CodecId == MFX_CODEC_JPEG) {
            // must be set for every frame
            av_frame->quality = m_avEncContext->global_quality;
        }

        err = avcodec_send_frame(m_avEncContext, av_frame);
        m_input_locker.Unlock();
        RET_IF_FALSE(err >= 0, MFX_ERR_ABORTED);
    }
    else {
        // send NULL packet to drain frames
        err = avcodec_send_frame(m_avEncContext, NULL);
        RET_IF_FALSE(err == 0 || err == AVERROR_EOF, MFX_ERR_UNKNOWN);
    }

    // get encoded packet, if available
    mfxU32 nBytesOut = 0, nBytesAvail = 0;

    err = avcodec_receive_packet(m_avEncContext, m_avEncPacket);
    if (err == AVERROR(EAGAIN)) {
        // need more data - nothing to do
        RET_ERROR(MFX_ERR_MORE_DATA);
    }
    else if (err == AVERROR_EOF) {
        RET_ERROR(MFX_ERR_MORE_DATA);
    }
    else if (err != 0) {
        // other error
        RET_ERROR(MFX_ERR_UNDEFINED_BEHAVIOR);
    }
    else {
        // copy encoded data to output buffer
        nBytesOut   = m_avEncPacket->size;
        nBytesAvail = bs->MaxLength - (bs->DataLength + bs->DataOffset);

        if (nBytesOut > nBytesAvail) {
            //error if encoded bytes out is larger than provided output buffer size
            return MFX_ERR_NOT_ENOUGH_BUFFER;
        }
        memcpy_s(bs->Data + bs->DataOffset,
                 nBytesAvail,
                 m_avEncPacket->data,
                 nBytesOut);
        bs->DataLength += nBytesOut;
    }

    av_packet_unref(m_avEncPacket);

    return MFX_ERR_NONE;
}

mfxStatus CpuEncode::EncodeQueryIOSurf(mfxVideoParam *par,
                                       mfxFrameAllocRequest *request) {
    // may be null for internal use
    if (par)
        request->Info = par->mfx.FrameInfo;
    else
        memset(&request->Info, 0, sizeof(mfxFrameInfo));

    request->NumFrameMin       = 3; // TO DO - calculate correctly from libav
    request->NumFrameSuggested = 3;
    request->Type = MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_FROM_ENCODE;

    return MFX_ERR_NONE;
}

mfxStatus CpuEncode::EncodeQuery(mfxVideoParam *in, mfxVideoParam *out) {
    mfxStatus sts = MFX_ERR_NONE;

    if (in) {
        // save a local copy of in, since user may set out == in
        mfxVideoParam inCopy = *in;
        in                   = &inCopy;

        // start with out = copy of in (does not deep copy extBufs)
        *out = *in;

        // validate fields in the input param struct
        sts            = ValidateEncodeParams(out);
        out->IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
    }
    else {
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

mfxStatus CpuEncode::GetEncodeSurface(mfxFrameSurface1 **surface) {
    if (!m_encSurfaces) {
        mfxFrameAllocRequest EncRequest = { 0 };
        RET_ERROR(EncodeQueryIOSurf(nullptr, &EncRequest));

        auto pool = std::make_unique<CpuFramePool>();
        RET_ERROR(pool->Init(m_param.mfx.FrameInfo.FourCC,
                             m_param.mfx.FrameInfo.Width,
                             m_param.mfx.FrameInfo.Height,
                             EncRequest.NumFrameSuggested));
        m_encSurfaces = std::move(pool);
    }

    return m_encSurfaces->GetFreeSurface(surface);
}

mfxStatus CpuEncode::GetVideoParam(mfxVideoParam *par) {
    *par = m_param;
    //*par = { 0 };

    par->IOPattern  = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
    par->AsyncDepth = 1;

    switch (m_avEncCodec->id) {
        case AV_CODEC_ID_H264:
            par->mfx.CodecId = MFX_CODEC_AVC;
            break;
        case AV_CODEC_ID_HEVC:
            par->mfx.CodecId = MFX_CODEC_HEVC;
            break;
        case AV_CODEC_ID_MJPEG:
            par->mfx.CodecId = MFX_CODEC_JPEG;
            break;
        case AV_CODEC_ID_AV1:
            par->mfx.CodecId = MFX_CODEC_AV1;
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    // resolution
    par->mfx.FrameInfo.Width  = (uint16_t)m_avEncContext->width;
    par->mfx.FrameInfo.Height = (uint16_t)m_avEncContext->height;
    par->mfx.FrameInfo.CropW  = (uint16_t)m_avEncContext->width;
    par->mfx.FrameInfo.CropH  = (uint16_t)m_avEncContext->height;

    par->mfx.BufferSizeInKB =
        par->mfx.FrameInfo.Width * par->mfx.FrameInfo.Height;

    // FourCC and chroma format
    switch (m_avEncContext->pix_fmt) {
        case AV_PIX_FMT_YUV420P10LE:
            par->mfx.FrameInfo.FourCC         = MFX_FOURCC_I010;
            par->mfx.FrameInfo.BitDepthLuma   = 10;
            par->mfx.FrameInfo.BitDepthChroma = 10;
            par->mfx.FrameInfo.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
            break;
        case AV_PIX_FMT_YUV420P:
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
    par->mfx.FrameInfo.FrameRateExtD = (uint16_t)m_avEncContext->framerate.num;
    par->mfx.FrameInfo.FrameRateExtN = (uint16_t)m_avEncContext->framerate.den;

    // Aspect ratio
    par->mfx.FrameInfo.AspectRatioW =
        (uint16_t)m_avEncContext->sample_aspect_ratio.num;
    par->mfx.FrameInfo.AspectRatioH =
        (uint16_t)m_avEncContext->sample_aspect_ratio.den;

    // Codec params
    switch (par->mfx.CodecId) {
        case MFX_CODEC_HEVC:
            GetHEVCParams(par);
            break;
        case MFX_CODEC_AV1:
            GetAV1Params(par);
            break;
        case MFX_CODEC_AVC:
            GetAVCParams(par);
            break;
        case MFX_CODEC_JPEG:
            GetJPEGParams(par);
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
    }

    return MFX_ERR_NONE;
}
