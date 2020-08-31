/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_vpp.h"
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include "src/cpu_workstream.h"

#define MFX_MAX(a, b) (((a) > (b)) ? (a) : (b))

#define VPP_GET_REAL_WIDTH(info, width) \
    { width = info->Width; }
#define VPP_GET_REAL_HEIGHT(info, height) \
    { height = info->Height; }

#define MAX_NUM_VPP_FILTERS (15)

#define PAR_NRF_STRENGTH_MAX 100

#define MFX_STS_TRACE(sts) sts
#define MFX_SUCCEEDED(sts) (MFX_STS_TRACE(sts) == MFX_ERR_NONE)
#define MFX_FAILED(sts)    (MFX_STS_TRACE(sts) != MFX_ERR_NONE)
#define MFX_RETURN(sts) \
    { return MFX_STS_TRACE(sts); }

#define MAX_NUM_OF_VPP_CONFIG_FILTERS (6)
#define MAX_NUM_OF_VPP_EXT_PARAM      (2 + MAX_NUM_OF_VPP_CONFIG_FILTERS)
#define PAR_NRF_STRENGTH_MAX          100

#define VPP_DETAIL_GAIN_MIN            (0)
#define VPP_DETAIL_GAIN_MAX            (63)
#define VPP_DETAIL_GAIN_MAX_REAL       (63)
#define VPP_DETAIL_GAIN_MAX_USER_LEVEL (100)
#define VPP_DETAIL_GAIN_DEFAULT        VPP_DETAIL_GAIN_MIN

#define VPP_PROCAMP_BRIGHTNESS_MAX     100.0
#define VPP_PROCAMP_BRIGHTNESS_MIN     -100.0
#define VPP_PROCAMP_BRIGHTNESS_DEFAULT 0.0

#define VPP_PROCAMP_CONTRAST_MAX     10.0
#define VPP_PROCAMP_CONTRAST_MIN     0.0
#define VPP_PROCAMP_CONTRAST_DEFAULT 1.0

#define VPP_PROCAMP_HUE_MAX     180.0
#define VPP_PROCAMP_HUE_MIN     -180.0
#define VPP_PROCAMP_HUE_DEFAULT 0.0

#define VPP_PROCAMP_SATURATION_MAX     10.0
#define VPP_PROCAMP_SATURATION_MIN     0.0
#define VPP_PROCAMP_SATURATION_DEFAULT 1.0

// names of VPP smart filters. internal using only
enum {
    MFX_EXTBUFF_VPP_CSC =
        MFX_MAKEFOURCC('C', 'S', 'C', 'F'), //COLOR SPACE CONVERSION FILTER
    MFX_EXTBUFF_VPP_RESIZE = MFX_MAKEFOURCC('R', 'S', 'Z', 'F'),
    MFX_EXTBUFF_VPP_DI =
        MFX_MAKEFOURCC('S',
                       'D',
                       'I',
                       'F'), //STANDARD DEINTERLACE FILTER (60i->30p)
    MFX_EXTBUFF_VPP_DI_30i60p =
        MFX_MAKEFOURCC('F',
                       'D',
                       'I',
                       'F'), //FULL PTS DEINTERLACE FILTER (60i->60p)
    MFX_EXTBUFF_VPP_DI_WEAVE =
        MFX_MAKEFOURCC('F',
                       'D',
                       'I',
                       'W'), //WEAVE DEINTERLACE FILTER (60i->30p)
    MFX_EXTBUFF_VPP_ITC =
        MFX_MAKEFOURCC('I', 'T', 'C', 'F'), //INV TELECINE FILTER
    MFX_EXTBUFF_VPP_FIELD_WEAVING   = MFX_MAKEFOURCC('F', 'I', 'W', 'F'),
    MFX_EXTBUFF_VPP_FIELD_SPLITTING = MFX_MAKEFOURCC('F', 'I', 'S', 'F'),

    MFX_EXTBUFF_VPP_CSC_OUT_RGB4 =
        MFX_MAKEFOURCC('C', 'S', 'R', '4'), //COLOR SPACE CONVERSION FILTER
    MFX_EXTBUFF_VPP_CSC_OUT_A2RGB10 =
        MFX_MAKEFOURCC('C', 'S', '1', '0'), //COLOR SPACE CONVERSION FILTER
    MFX_EXTBUFF_VPP_RSHIFT_IN  = MFX_MAKEFOURCC('R', 'S', 'F', 'I'),
    MFX_EXTBUFF_VPP_LSHIFT_IN  = MFX_MAKEFOURCC('L', 'S', 'F', 'I'),
    MFX_EXTBUFF_VPP_RSHIFT_OUT = MFX_MAKEFOURCC('R', 'S', 'F', 'O'),
    MFX_EXTBUFF_VPP_LSHIFT_OUT = MFX_MAKEFOURCC('L', 'S', 'F', 'O'),
};

// request type
typedef enum {
    MFX_REQUEST_FROM_VPP_CHECK   = 0x0001,
    MFX_REQUEST_FROM_VPP_PROCESS = 0x0002

} mfxRequestType;

// picture structure mode
typedef enum {
    PASS_THROUGH_PICSTRUCT_MODE = 0x0001,
    DYNAMIC_DI_PICSTRUCT_MODE   = 0x0004,
    ERR_PICSTRUCT_MODE          = 0x0000

} PicStructMode;

// vpp in/out type
enum { VPP_IN = 0x00, VPP_OUT = 0x01 };

// extended buffer ids
enum {
    MFX_EXTBUFF_MVC_SEQ_DESC     = MFX_MAKEFOURCC('M', 'V', 'C', 'D'),
    MFX_EXTBUFF_MVC_TARGET_VIEWS = MFX_MAKEFOURCC('M', 'V', 'C', 'T')
};

const mfxU32 g_TABLE_DO_USE[] = { MFX_EXTBUFF_VPP_DENOISE,
                                  MFX_EXTBUFF_VPP_SCENE_ANALYSIS,
                                  MFX_EXTBUFF_VPP_PROCAMP,
                                  MFX_EXTBUFF_VPP_DETAIL,
                                  MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION,
                                  MFX_EXTBUFF_VPP_IMAGE_STABILIZATION,
                                  MFX_EXTBUFF_VPP_COMPOSITE,
                                  MFX_EXTBUFF_VPP_ROTATION,
                                  MFX_EXTBUFF_VPP_SCALING,
#if (MFX_VERSION >= 1025)
                                  MFX_EXTBUFF_VPP_COLOR_CONVERSION,
#endif
#ifdef MFX_UNDOCUMENTED_VPP_VARIANCE_REPORT
                                  MFX_EXTBUFF_VPP_VARIANCE_REPORT,
#endif
                                  MFX_EXTBUFF_VPP_DEINTERLACING,
                                  MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO,
                                  MFX_EXTBUFF_VPP_FIELD_PROCESSING,
                                  MFX_EXTBUFF_VPP_MIRRORING };

const mfxU32 g_TABLE_DO_NOT_USE[] = { MFX_EXTBUFF_VPP_DENOISE,
#ifdef MFX_ENABLE_MCTF
                                      MFX_EXTBUFF_VPP_MCTF,
#endif
                                      MFX_EXTBUFF_VPP_SCENE_ANALYSIS,
                                      MFX_EXTBUFF_VPP_PROCAMP,
                                      MFX_EXTBUFF_VPP_DETAIL,
                                      MFX_EXTBUFF_VPP_IMAGE_STABILIZATION,
                                      MFX_EXTBUFF_VPP_COMPOSITE,
                                      MFX_EXTBUFF_VPP_ROTATION,
                                      MFX_EXTBUFF_VPP_SCALING,
#if (MFX_VERSION >= 1025)
                                      MFX_EXTBUFF_VPP_COLOR_CONVERSION,
#endif
                                      MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO,
                                      MFX_EXTBUFF_VPP_FIELD_PROCESSING,
                                      MFX_EXTBUFF_VPP_MIRRORING };

const mfxU32 g_TABLE_CONFIG[] = { MFX_EXTBUFF_VPP_DENOISE,
                                  MFX_EXTBUFF_VPP_SCENE_ANALYSIS,
                                  MFX_EXTBUFF_VPP_PROCAMP,
                                  MFX_EXTBUFF_VPP_DETAIL,
                                  MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION,
                                  MFX_EXTBUFF_VPP_IMAGE_STABILIZATION,
                                  MFX_EXTBUFF_VPP_COMPOSITE,
                                  MFX_EXTBUFF_VPP_ROTATION,
                                  MFX_EXTBUFF_VPP_DEINTERLACING,
                                  MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO,
                                  MFX_EXTBUFF_VPP_FIELD_PROCESSING,
                                  MFX_EXTBUFF_VPP_SCALING,
#if (MFX_VERSION >= 1025)
                                  MFX_EXTBUFF_VPP_COLOR_CONVERSION,
#endif
                                  MFX_EXTBUFF_VPP_MIRRORING };

const mfxU32 g_TABLE_EXT_PARAM[] = {
//MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION,
#ifdef MFX_ENABLE_VPP_SVC
    MFX_EXTBUFF_SVC_SEQ_DESC,
#endif
    MFX_EXTBUFF_MVC_SEQ_DESC,

    MFX_EXTBUFF_VPP_DONOTUSE,
    MFX_EXTBUFF_VPP_DOUSE,

    // should be the same as g_TABLE_CONFIG
    MFX_EXTBUFF_VPP_DENOISE,
    MFX_EXTBUFF_VPP_SCENE_ANALYSIS,
    MFX_EXTBUFF_VPP_PROCAMP,
    MFX_EXTBUFF_VPP_DETAIL,
    MFX_EXTBUFF_VPP_ROTATION,
    MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION,
    MFX_EXTBUFF_VPP_IMAGE_STABILIZATION,
    MFX_EXTBUFF_VPP_COMPOSITE,
    MFX_EXTBUFF_VPP_DEINTERLACING,
    MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO,
    MFX_EXTBUFF_VPP_FIELD_PROCESSING,
    MFX_EXTBUFF_VPP_SCALING,
#if (MFX_VERSION >= 1025)
    MFX_EXTBUFF_VPP_COLOR_CONVERSION,
#endif
    MFX_EXTBUFF_VPP_MIRRORING
};

CpuVPP::CpuVPP(CpuWorkstream* session)
        : m_session(session),
          m_avVppFrameOut(nullptr),
          m_vpp_graph(nullptr),
          m_buffersrc_ctx(nullptr),
          m_buffersink_ctx(nullptr),
          m_vppInFormat(MFX_FOURCC_I420),
          m_vppWidth(0),
          m_vppHeight(0),
          m_vppSurfaces() {
    memset(&m_vpp_base, 0, sizeof(m_vpp_base));
    memset(m_vpp_filter_desc, 0, sizeof(m_vpp_filter_desc));
}

// buffersrc --> execution filters from filter description --> buffersink
bool CpuVPP::InitFilters(void) {
    int ret                          = 0;
    char buffersrc_fmt[512]          = { 0 };
    const AVFilter* buffersrc        = avfilter_get_by_name("buffer");
    const AVFilter* buffersink       = avfilter_get_by_name("buffersink");
    AVFilterInOut* buffersrc_out_pad = avfilter_inout_alloc();
    AVFilterInOut* buffersink_in_pad = avfilter_inout_alloc();

    m_vpp_graph = avfilter_graph_alloc();

    if (!buffersrc_out_pad || !buffersink_in_pad || !m_vpp_graph) {
        printf("cannot alloc filter graph\n");
        CloseFilterPads(buffersrc_out_pad, buffersink_in_pad);
        return false;
    }

    snprintf(
        buffersrc_fmt,
        sizeof(buffersrc_fmt),
        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d", //:pixel_aspect=1/1",
        m_vpp_base.src_width,
        m_vpp_base.src_height,
        m_vpp_base.src_pixel_format,
        m_vpp_base.src_fr_num,
        m_vpp_base.src_fr_den);

    ret = avfilter_graph_create_filter(&m_buffersrc_ctx,
                                       buffersrc,
                                       "video-in",
                                       buffersrc_fmt,
                                       NULL,
                                       m_vpp_graph);
    if (ret < 0) {
        printf("cannot create buffer source\n");
        CloseFilterPads(buffersrc_out_pad, buffersink_in_pad);
        return false;
    }

    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&m_buffersink_ctx,
                                       buffersink,
                                       "video-out",
                                       NULL,
                                       NULL,
                                       m_vpp_graph);
    if (ret < 0) {
        printf("cannot create buffer sink\n");
        CloseFilterPads(buffersrc_out_pad, buffersink_in_pad);
        return false;
    }

    // scale
    if (m_vpp_base.vpp_func & VPL_VPP_SCALE) {
        snprintf(m_vpp_filter_desc,
                 sizeof(m_vpp_filter_desc),
                 "scale=%d:%d",
                 m_vpp_base.dst_width,
                 m_vpp_base.dst_height);
    }

    // crop - do crop and scale to match msdk feature
    if (m_vpp_base.vpp_func & VPL_VPP_CROP) {
        // no need background
        if (m_vpp_base.dst_width == m_vpp_base.dst_rc.w &&
            m_vpp_base.dst_height == m_vpp_base.dst_rc.h) {
            snprintf(m_vpp_filter_desc,
                     sizeof(m_vpp_filter_desc),
                     "crop=%d:%d:%d:%d,scale=%d:%d",
                     m_vpp_base.src_rc.w,
                     m_vpp_base.src_rc.h,
                     m_vpp_base.src_rc.x,
                     m_vpp_base.src_rc.y,
                     m_vpp_base.dst_rc.w,
                     m_vpp_base.dst_rc.h);
        }
        else {
            std::string f_split = "split=2[bg][main];";
            std::string f_scale_dst =
                "[bg]scale=" + std::to_string(m_vpp_base.dst_width) + ":" +
                std::to_string(m_vpp_base.dst_height) + ",";
            std::string f_bg =
                "drawbox=x=0:y=0:w=" + std::to_string(m_vpp_base.dst_width) +
                ":h=" + std::to_string(m_vpp_base.dst_height) + ":t=fill[bg2];";
            std::string f_crop_src =
                "[main]crop=" + std::to_string(m_vpp_base.src_rc.w) + ":" +
                std::to_string(m_vpp_base.src_rc.h) + ":" +
                std::to_string(m_vpp_base.src_rc.x) + ":" +
                std::to_string(m_vpp_base.src_rc.y) +
                ",scale=" + std::to_string(m_vpp_base.dst_rc.w) + ":" +
                std::to_string(m_vpp_base.dst_rc.h) + "[ovr];";
            std::string f_ovr =
                "[bg2][ovr]overlay=" + std::to_string(m_vpp_base.dst_rc.x) +
                ":" + std::to_string(m_vpp_base.dst_rc.y);

            snprintf(m_vpp_filter_desc,
                     sizeof(m_vpp_filter_desc),
                     "%s%s%s%s%s",
                     f_split.c_str(),
                     f_scale_dst.c_str(),
                     f_bg.c_str(),
                     f_crop_src.c_str(),
                     f_ovr.c_str());
        }

        m_vpp_base.vpp_func |= VPL_VPP_CSC;
    }

    // csc - set pixel format of buffersink
    if (m_vpp_base.vpp_func & VPL_VPP_CSC) {
        AVPixelFormat csc_dst_fmt     = m_vpp_base.dst_pixel_format;
        enum AVPixelFormat pix_fmts[] = { csc_dst_fmt, AV_PIX_FMT_NONE };

        ret = av_opt_set_int_list(m_buffersink_ctx,
                                  "pix_fmts",
                                  pix_fmts,
                                  AV_PIX_FMT_NONE,
                                  AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            printf("cannot set output pixel format\n");
            CloseFilterPads(buffersrc_out_pad, buffersink_in_pad);
            return false;
        }

        char pixel_format[50] = { 0 };
        if (csc_dst_fmt == AV_PIX_FMT_YUV420P)
            snprintf(pixel_format,
                     sizeof(pixel_format),
                     "format=pix_fmts=yuv420p");
        else if (csc_dst_fmt == AV_PIX_FMT_YUV420P10LE)
            snprintf(pixel_format,
                     sizeof(pixel_format),
                     "format=pix_fmts=yuv420p10le");
        else if (csc_dst_fmt == AV_PIX_FMT_BGRA)
            snprintf(pixel_format,
                     sizeof(pixel_format),
                     "format=pix_fmts=bgra");

        if (m_vpp_base.vpp_func == VPL_VPP_CSC) // there's no filter assigned
            snprintf(m_vpp_filter_desc,
                     sizeof(m_vpp_filter_desc),
                     "%s",
                     pixel_format);
        else {
            std::string curr_desc = m_vpp_filter_desc;
            snprintf(m_vpp_filter_desc,
                     sizeof(m_vpp_filter_desc),
                     "%s,%s",
                     curr_desc.c_str(),
                     pixel_format);
        }
    }

    /*
    printf("vpp src width, hegiht: %d x %d\n",
           m_vpp_base.src_width,
           m_vpp_base.src_height);
    printf("vpp src rc: %d, %d, %d, %d\n",
           m_vpp_base.src_rc.x,
           m_vpp_base.src_rc.y,
           m_vpp_base.src_rc.w,
           m_vpp_base.src_rc.h);
    printf("vpp dst width, hegiht: %d x %d\n",
           m_vpp_base.dst_width,
           m_vpp_base.dst_height);
    printf("vpp dst rc: %d, %d, %d, %d\n",
           m_vpp_base.dst_rc.x,
           m_vpp_base.dst_rc.y,
           m_vpp_base.dst_rc.w,
           m_vpp_base.dst_rc.h);
    printf(
        "-----------------------------------------------------------------\n");
    printf("filter desc: %s\n", m_vpp_filter_desc);
    */

    buffersrc_out_pad->name       = av_strdup("in");
    buffersrc_out_pad->filter_ctx = m_buffersrc_ctx;
    buffersrc_out_pad->pad_idx    = 0;
    buffersrc_out_pad->next       = NULL;

    buffersink_in_pad->name       = av_strdup("out");
    buffersink_in_pad->filter_ctx = m_buffersink_ctx;
    buffersink_in_pad->pad_idx    = 0;
    buffersink_in_pad->next       = NULL;

    // this prevents from failing by non filter description
    if (m_vpp_filter_desc[0] == '\0') {
        snprintf(m_vpp_filter_desc, sizeof(m_vpp_filter_desc), "null");
    }

    ret = avfilter_graph_parse_ptr(m_vpp_graph,
                                   (const char*)m_vpp_filter_desc,
                                   &buffersink_in_pad,
                                   &buffersrc_out_pad,
                                   NULL);
    if (ret < 0) {
        printf("cannot setup graph with filter description\n");
    }
    else {
        ret = avfilter_graph_config(m_vpp_graph, NULL);
    }

    CloseFilterPads(buffersrc_out_pad, buffersink_in_pad);

    if (ret < 0) {
        printf("vpp filter initialization fail\n");
        return false;
    }
    else {
        return true;
    }
}

void CpuVPP::CloseFilterPads(AVFilterInOut* src_out, AVFilterInOut* sink_in) {
    if (src_out)
        avfilter_inout_free(&src_out);
    if (sink_in)
        avfilter_inout_free(&sink_in);
    return;
}

mfxStatus CpuVPP::InitVPP(mfxVideoParam* par) {
    int ret           = 0;
    mfxStatus sts     = MFX_ERR_INVALID_VIDEO_PARAM;
    mfxStatus sts_wrn = MFX_ERR_NONE;

    if (0 == par->IOPattern) // IOPattern is mandatory parameter
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (!(par->IOPattern & MFX_IOPATTERN_IN_SYSTEM_MEMORY) ||
        !(par->IOPattern & MFX_IOPATTERN_OUT_SYSTEM_MEMORY))
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->Protected)
        return MFX_ERR_UNSUPPORTED;

    sts = CheckFrameInfo(&(par->vpp.In), VPP_IN);
    RET_ERROR(sts);

    sts = CheckFrameInfo(&(par->vpp.Out), VPP_OUT);
    RET_ERROR(sts);

    sts = CheckExtParam(par->ExtParam, par->NumExtParam);
    if (MFX_WRN_INCOMPATIBLE_VIDEO_PARAM == sts ||
        MFX_WRN_FILTER_SKIPPED == sts) {
        sts_wrn = sts;
        sts     = MFX_ERR_NONE;
    }
    RET_ERROR(sts);

    std::vector<mfxU32> pipelineList;
    sts = GetPipelineList(par, pipelineList, true);
    RET_ERROR(sts);

    m_vpp_base.src_pixel_format = MFXFourCC2AVPixelFormat(par->vpp.In.FourCC);
    m_vpp_base.src_shift        = par->vpp.In.Shift;
    m_vpp_base.src_fr_num       = par->vpp.In.FrameRateExtN;
    m_vpp_base.src_fr_den       = par->vpp.In.FrameRateExtD;
    m_vpp_base.src_rc.x         = par->vpp.In.CropX;
    m_vpp_base.src_rc.y         = par->vpp.In.CropY;
    m_vpp_base.src_rc.w         = par->vpp.In.CropW;
    m_vpp_base.src_rc.h         = par->vpp.In.CropH;
    m_vpp_base.src_width        = par->vpp.In.Width;
    m_vpp_base.src_height       = par->vpp.In.Height;

    m_vpp_base.dst_rc.x         = par->vpp.Out.CropX;
    m_vpp_base.dst_rc.y         = par->vpp.Out.CropY;
    m_vpp_base.dst_rc.w         = par->vpp.Out.CropW;
    m_vpp_base.dst_rc.h         = par->vpp.Out.CropH;
    m_vpp_base.dst_pixel_format = MFXFourCC2AVPixelFormat(par->vpp.Out.FourCC);
    m_vpp_base.dst_shift        = par->vpp.Out.Shift;
    m_vpp_base.dst_fr_num       = par->vpp.Out.FrameRateExtN;
    m_vpp_base.dst_fr_den       = par->vpp.Out.FrameRateExtD;
    m_vpp_base.dst_width        = par->vpp.Out.Width;
    m_vpp_base.dst_height       = par->vpp.Out.Height;

    if (m_vpp_base.src_pixel_format != m_vpp_base.dst_pixel_format) {
        m_vpp_base.vpp_func |= VPL_VPP_CSC;
    }

    if (m_vpp_base.src_rc.x != 0 || m_vpp_base.src_rc.y != 0 ||
        m_vpp_base.dst_rc.x != 0 || m_vpp_base.dst_rc.y != 0) {
        m_vpp_base.vpp_func |= VPL_VPP_CROP;
    }

    if (!(m_vpp_base.vpp_func & VPL_VPP_CROP) &&
        (m_vpp_base.src_rc.w != m_vpp_base.src_width ||
         m_vpp_base.src_rc.h != m_vpp_base.src_height ||
         m_vpp_base.dst_rc.w != m_vpp_base.dst_width ||
         m_vpp_base.dst_rc.h != m_vpp_base.dst_height)) {
        m_vpp_base.vpp_func |= VPL_VPP_CROP;
    }

    if (!(m_vpp_base.vpp_func & VPL_VPP_CROP) &&
        (m_vpp_base.src_width != m_vpp_base.dst_width ||
         m_vpp_base.src_height != m_vpp_base.dst_height)) {
        m_vpp_base.vpp_func |= VPL_VPP_SCALE;
    }

    if (InitFilters() == false)
        return MFX_ERR_NOT_INITIALIZED;

    m_avVppFrameOut = av_frame_alloc();
    if (!m_avVppFrameOut)
        return MFX_ERR_NOT_INITIALIZED;

    ret = av_image_alloc(m_avVppFrameOut->data,
                         m_avVppFrameOut->linesize,
                         m_vpp_base.dst_width,
                         m_vpp_base.dst_height,
                         m_vpp_base.dst_pixel_format,
                         16);
    if (ret < 0)
        return MFX_ERR_NOT_INITIALIZED;

    m_vppInFormat = par->vpp.In.FourCC;
    m_vppWidth    = par->vpp.In.Width;
    m_vppHeight   = par->vpp.In.Height;

    return MFX_ERR_NONE;
}

CpuVPP::~CpuVPP() {
    if (m_vpp_graph) {
        avfilter_graph_free(&m_vpp_graph);
        m_vpp_graph = nullptr;
    }
}

mfxStatus CpuVPP::ProcessFrame(mfxFrameSurface1* surface_in,
                               mfxFrameSurface1* surface_out,
                               mfxExtVppAuxData* aux) {
    // Try get AVFrame from surface_out
    AVFrame* dst_avframe = nullptr;
    CpuFrame* dst_frame  = CpuFrame::TryCast(surface_out);
    if (dst_frame) {
        dst_avframe = dst_frame->GetAVFrame();
    }
    if (!dst_avframe) { // Otherwise use AVFrame allocated in this class
        dst_avframe = m_avVppFrameOut;
    }

    if (surface_in) {
        AVFrame* av_frame =
            m_input_locker.GetAVFrame(surface_in,
                                      MFX_MAP_READ,
                                      m_session->GetFrameAllocator());
        RET_IF_FALSE(av_frame, MFX_ERR_ABORTED);

        int ret = av_buffersrc_add_frame_flags(m_buffersrc_ctx,
                                               av_frame,
                                               AV_BUFFERSRC_FLAG_KEEP_REF);
        m_input_locker.Unlock();
        RET_IF_FALSE(ret >= 0, MFX_ERR_ABORTED);
    }

    // av_buffersink_get_frame
    int ret = av_buffersink_get_frame(m_buffersink_ctx, dst_avframe);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return MFX_ERR_MORE_DATA;
    }
    RET_IF_FALSE(ret >= 0, MFX_ERR_ABORTED);

    if (dst_avframe == m_avVppFrameOut) { // copy image data
        RET_ERROR(AVFrame2mfxFrameSurface(surface_out,
                                          m_avVppFrameOut,
                                          m_session->GetFrameAllocator()));
    }
    else if (dst_frame) { // update MFXFrameSurface from AVFrame
        dst_frame->Update();
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuVPP::VPPQuery(mfxVideoParam* in, mfxVideoParam* out) {
    if (out == 0)
        return MFX_ERR_NULL_PTR;

    if (in != 0 && in->Protected != 0)
        return MFX_ERR_UNSUPPORTED;

    if (in == NULL) {
        out->mfx = { 0 };
        out->vpp = { 0 };

        // We have to set FourCC and FrameRate below to
        // pass requirements of CheckPlatformLimitation for frame interpolation

        /* vppIn */
        out->vpp.In.FourCC        = 0xFFFFFFFF;
        out->vpp.In.Height        = 0xFFFF;
        out->vpp.In.Width         = 0xFFFF;
        out->vpp.In.CropH         = 0xFFFF;
        out->vpp.In.CropW         = 0xFFFF;
        out->vpp.In.PicStruct     = 0xFFFF;
        out->vpp.In.FrameRateExtN = 0xFFFFFFFF;
        out->vpp.In.FrameRateExtD = 0xFFFFFFFF;

        /* vppOut */
        out->vpp.Out.FourCC        = 0xFFFFFFFF;
        out->vpp.Out.Height        = 0xFFFF;
        out->vpp.Out.Width         = 0xFFFF;
        out->vpp.Out.CropH         = 0xFFFF;
        out->vpp.Out.CropW         = 0xFFFF;
        out->vpp.Out.PicStruct     = 0xFFFF;
        out->vpp.Out.FrameRateExtN = 0xFFFFFFFF;
        out->vpp.Out.FrameRateExtD = 0xFFFFFFFF;

        out->IOPattern = 0xFFFF;

        return MFX_ERR_NONE;
    }
    else {
        *out = *in;

        if (!out->vpp.Out.Width)
            out->vpp.Out.Width = out->vpp.In.Width;

        if (!out->vpp.Out.Height)
            out->vpp.Out.Height = out->vpp.In.Height;

        if (!out->vpp.Out.FourCC)
            out->vpp.Out.FourCC = out->vpp.In.FourCC;

        if (out->Protected) {
            return MFX_ERR_INVALID_VIDEO_PARAM;
        }

        //query, always correct
        out->IOPattern =
            MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    }

    return MFX_ERR_NONE;
}

mfxStatus CpuVPP::VPPQueryIOSurf(mfxVideoParam* par,
                                 mfxFrameAllocRequest request[2]) {
    mfxStatus sts;

    // VPP_IN
    request[VPP_IN].NumFrameMin       = 1;
    request[VPP_IN].NumFrameSuggested = 1;

    //VPP_OUT
    request[VPP_OUT].NumFrameMin       = 1;
    request[VPP_OUT].NumFrameSuggested = 1;

    // may be null for internal use
    if (par) {
        request[VPP_IN].Info  = par->vpp.In;
        request[VPP_OUT].Info = par->vpp.Out;

        sts = CheckIOPattern_AndSetIOMemTypes(par->IOPattern,
                                              &request[VPP_IN].Type,
                                              &request[VPP_OUT].Type);
    }
    else {
        request[VPP_IN].Info  = { 0 };
        request[VPP_OUT].Info = { 0 };

        sts = MFX_ERR_NONE;
    }

    return sts;
}

mfxStatus CpuVPP::CheckIOPattern_AndSetIOMemTypes(mfxU16 IOPattern,
                                                  mfxU16* pInMemType,
                                                  mfxU16* pOutMemType) {
    if (IOPattern & MFX_IOPATTERN_IN_VIDEO_MEMORY ||
        IOPattern & MFX_IOPATTERN_OUT_VIDEO_MEMORY)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (IOPattern & MFX_IOPATTERN_IN_SYSTEM_MEMORY)
        *pInMemType = MFX_MEMTYPE_FROM_VPPIN | MFX_MEMTYPE_EXTERNAL_FRAME |
                      MFX_MEMTYPE_SYSTEM_MEMORY;
    else
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (IOPattern & MFX_IOPATTERN_OUT_SYSTEM_MEMORY)
        *pOutMemType = MFX_MEMTYPE_FROM_VPPOUT | MFX_MEMTYPE_EXTERNAL_FRAME |
                       MFX_MEMTYPE_SYSTEM_MEMORY;
    else
        return MFX_ERR_INVALID_VIDEO_PARAM;

    return MFX_ERR_NONE;
}

// check is buffer or filter are configurable
bool CpuVPP::IsConfigurable(mfxU32 filterId) {
    mfxU32 searchCount = sizeof(g_TABLE_CONFIG) / sizeof(*g_TABLE_CONFIG);
    return IsFilterFound(g_TABLE_CONFIG, searchCount, filterId) ? true : false;
}

size_t CpuVPP::GetConfigSize(mfxU32 filterId) {
    switch (filterId) {
        case MFX_EXTBUFF_VPP_DENOISE: {
            return sizeof(mfxExtVPPDenoise);
        }
        case MFX_EXTBUFF_VPP_PROCAMP: {
            return sizeof(mfxExtVPPProcAmp);
        }
        case MFX_EXTBUFF_VPP_DETAIL: {
            return sizeof(mfxExtVPPDetail);
        }
        case MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION: {
            return sizeof(mfxExtVPPFrameRateConversion);
        }
#if defined(MFX_ENABLE_IMAGE_STABILIZATION_VPP)
        case MFX_EXTBUFF_VPP_IMAGE_STABILIZATION: {
            return sizeof(mfxExtVPPImageStab);
        }
#endif
        case MFX_EXTBUFF_VPP_DEINTERLACING: {
            return sizeof(mfxExtVPPDeinterlacing);
        }
            /*case MFX_EXTBUFF_VPP_COMPOSITE:
		{
			return sizeof(mfxExtVPPDeinterlacing);
		}???*/

        default:
            return 0;
    }
}

mfxStatus CpuVPP::ExtendedQuery(mfxU32 filterName, mfxExtBuffer* pHint) {
    mfxStatus sts = MFX_ERR_NONE;

    if (MFX_EXTBUFF_VPP_DENOISE == filterName) {
        mfxExtVPPDenoise* pParam = (mfxExtVPPDenoise*)pHint;

        if (pParam->DenoiseFactor > PAR_NRF_STRENGTH_MAX) {
            pParam->DenoiseFactor = PAR_NRF_STRENGTH_MAX;
            sts                   = MFX_WRN_INCOMPATIBLE_VIDEO_PARAM;
        }
    }
    else if (MFX_EXTBUFF_VPP_DETAIL == filterName) {
        mfxExtVPPDetail* pParam = (mfxExtVPPDetail*)pHint;

        if (pParam->DetailFactor > VPP_DETAIL_GAIN_MAX_USER_LEVEL) {
            pParam->DetailFactor = VPP_DETAIL_GAIN_MAX_USER_LEVEL;
            sts                  = MFX_WRN_INCOMPATIBLE_VIDEO_PARAM;
        }
    }
    else if (MFX_EXTBUFF_VPP_PROCAMP == filterName) {
        mfxExtVPPProcAmp* pParam = (mfxExtVPPProcAmp*)pHint;
        /* Brightness */
        if (pParam->Brightness < VPP_PROCAMP_BRIGHTNESS_MIN ||
            pParam->Brightness > VPP_PROCAMP_BRIGHTNESS_MAX) {
            pParam->Brightness = std::min(
                VPP_PROCAMP_BRIGHTNESS_MAX,
                std::max(pParam->Brightness, VPP_PROCAMP_BRIGHTNESS_MIN));
            sts = MFX_WRN_INCOMPATIBLE_VIDEO_PARAM;
        }
        /* Contrast */
        if (pParam->Contrast < VPP_PROCAMP_CONTRAST_MIN ||
            pParam->Contrast > VPP_PROCAMP_CONTRAST_MAX) {
            pParam->Contrast =
                std::min(VPP_PROCAMP_CONTRAST_MAX,
                         std::max(pParam->Contrast, VPP_PROCAMP_CONTRAST_MIN));
            sts = MFX_WRN_INCOMPATIBLE_VIDEO_PARAM;
        }
        /* Hue */
        if (pParam->Hue < VPP_PROCAMP_HUE_MIN ||
            pParam->Hue > VPP_PROCAMP_HUE_MAX) {
            pParam->Hue = std::min(VPP_PROCAMP_HUE_MAX,
                                   std::max(pParam->Hue, VPP_PROCAMP_HUE_MIN));
            sts         = MFX_WRN_INCOMPATIBLE_VIDEO_PARAM;
        }
        /* Saturation */
        if (pParam->Saturation < VPP_PROCAMP_SATURATION_MIN ||
            pParam->Saturation > VPP_PROCAMP_SATURATION_MAX) {
            pParam->Saturation = std::min(
                VPP_PROCAMP_SATURATION_MAX,
                std::max(pParam->Saturation, VPP_PROCAMP_SATURATION_MIN));
            sts = MFX_WRN_INCOMPATIBLE_VIDEO_PARAM;
        }
    }
    else if (MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION == filterName) {
        mfxExtVPPFrameRateConversion* pParam =
            (mfxExtVPPFrameRateConversion*)pHint;
        if (MFX_FRCALGM_PRESERVE_TIMESTAMP == pParam->Algorithm ||
            MFX_FRCALGM_DISTRIBUTED_TIMESTAMP == pParam->Algorithm ||
            MFX_FRCALGM_FRAME_INTERPOLATION == pParam->Algorithm) {
            sts = MFX_ERR_NONE;
        }
        else {
            sts = MFX_ERR_UNSUPPORTED;
        }
    }
#if defined(MFX_ENABLE_IMAGE_STABILIZATION_VPP)
    else if (MFX_EXTBUFF_VPP_IMAGE_STABILIZATION == filterName) {
        if (false == bLinuxAndIVB_HSW_BDW)
            sts = MFXVideoVPPImgStab::Query(pHint);
        else {
            // This filter is not supported in Linux
            sts = MFX_WRN_FILTER_SKIPPED;
        }
    }
#endif
#if 0
	else if (MFX_EXTBUFF_VPP_GAMUT_MAPPING == filterName)
	{
		sts = MFXVideoVPPGamutCompression::Query(pHint);
	}
#endif
    else if (MFX_EXTBUFF_VPP_SCENE_ANALYSIS == filterName) {
        sts = MFX_ERR_UNSUPPORTED;
    }
    else if (MFX_EXTBUFF_VPP_COMPOSITE == filterName) {
        sts = MFX_ERR_NONE;
    }
    else if (MFX_EXTBUFF_VPP_FIELD_PROCESSING == filterName) {
        sts = MFX_ERR_UNSUPPORTED;
    }
    else {
        sts = MFX_ERR_NONE;
    }

    return sts;
}

mfxU32 CpuVPP::GetFilterIndex(mfxU32* pList, mfxU32 len, mfxU32 filterName) {
    mfxU32 filterIndex;
    for (filterIndex = 0; filterIndex < len; filterIndex++) {
        if (filterName == pList[filterIndex]) {
            return filterIndex;
        }
    }

    return 0;
}

bool CpuVPP::CheckDoUseCompatibility(mfxU32 filterName) {
    bool bResult = false;

    mfxU32 douseCount = sizeof(g_TABLE_DO_USE) / sizeof(*g_TABLE_DO_USE);

    bResult = IsFilterFound(g_TABLE_DO_USE, douseCount, filterName);

    return bResult;

} // bool CheckDoUseCompatibility( mfxU32 filterName )

bool CpuVPP::IsFilterFound(const mfxU32* pList, mfxU32 len, mfxU32 filterName) {
    if (0 == len)
        return false;

    for (mfxU32 i = 0; i < len; i++) {
        if (filterName == pList[i])
            return true;
    }

    return false;
}

void CpuVPP::GetDoUseFilterList(mfxVideoParam* par,
                                mfxU32** ppList,
                                mfxU32* pLen) {
    mfxU32 i                 = 0;
    mfxExtVPPDoUse* pVPPHint = NULL;

    /* robustness */
    *ppList = NULL;
    *pLen   = 0;

    for (i = 0; i < par->NumExtParam; i++) {
        if (MFX_EXTBUFF_VPP_DOUSE == par->ExtParam[i]->BufferId) {
            pVPPHint = (mfxExtVPPDoUse*)(par->ExtParam[i]);
            *ppList  = pVPPHint->AlgList;
            *pLen    = pVPPHint->NumAlg;

            return;
        }
    }

    return;
}

void CpuVPP::GetConfigurableFilterList(mfxVideoParam* par,
                                       mfxU32* pList,
                                       mfxU32* pLen) {
    mfxU32 fIdx = 0;

    /* robustness */
    *pLen = 0;

    mfxU32 fCount      = par->NumExtParam;
    mfxU32 searchCount = sizeof(g_TABLE_CONFIG) / sizeof(*g_TABLE_CONFIG);

    for (fIdx = 0; fIdx < fCount; fIdx++) {
        mfxU32 curId = par->ExtParam[fIdx]->BufferId;
        if (IsFilterFound(g_TABLE_CONFIG, searchCount, curId) &&
            !IsFilterFound(pList, *pLen, curId)) {
            pList[(*pLen)++] = curId;
        }
    }

    return;
}

double CpuVPP::CalculateUMCFramerate(mfxU32 FrameRateExtN,
                                     mfxU32 FrameRateExtD) {
    if (FrameRateExtN && FrameRateExtD)
        return (double)FrameRateExtN / FrameRateExtD;
    else
        return 0;
}

// vpp best quality is |CSC| + |DN| + |DI| + |IS| + |RS| + |Detail| + |ProcAmp| + |FRC| + |SA| */
// sw_vpp reorder |FRC| to meet best speed                                                        */
void CpuVPP::ReorderPipelineListForQuality(std::vector<mfxU32>& pipelineList) {
    //mfxU32 newList[MAX_NUM_VPP_FILTERS] = {0};
    std::vector<mfxU32> newList;
    newList.resize(pipelineList.size());
    mfxU32 index = 0;

    // [-1] Shift is very first, since shifted content is not supported by VPP
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_RSHIFT_IN)) {
        newList[index] = MFX_EXTBUFF_VPP_RSHIFT_IN;
        index++;
    }

    // [0] canonical order
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_CSC)) {
        newList[index] = MFX_EXTBUFF_VPP_CSC;
        index++;
    }
    // Resize for Best Speed
    /*if( IsFilterFound( pList, len, MFX_EXTBUFF_VPP_RESIZE ) )
	{
		newList[index] = MFX_EXTBUFF_VPP_RESIZE;
		index++;
	}*/
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_DENOISE)) {
        newList[index] = MFX_EXTBUFF_VPP_DENOISE;
        index++;
    }
    // DI, advDI, ITC has the same priority
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_DI)) {
        newList[index] = MFX_EXTBUFF_VPP_DI;
        index++;
    }
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_DI_WEAVE)) {
        newList[index] = MFX_EXTBUFF_VPP_DI_WEAVE;
        index++;
    }
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_DI_30i60p)) {
        newList[index] = MFX_EXTBUFF_VPP_DI_30i60p;
        index++;
    }
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_ITC)) {
        newList[index] = MFX_EXTBUFF_VPP_ITC;
        index++;
    }
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_DEINTERLACING) &&
        !IsFilterFound(&pipelineList[0],
                       (mfxU32)pipelineList.size(),
                       MFX_EXTBUFF_VPP_DI_30i60p) &&
        !IsFilterFound(&pipelineList[0],
                       (mfxU32)pipelineList.size(),
                       MFX_EXTBUFF_VPP_DI_WEAVE) &&
        !IsFilterFound(&pipelineList[0],
                       (mfxU32)pipelineList.size(),
                       MFX_EXTBUFF_VPP_DI)) {
        newList[index] = MFX_EXTBUFF_VPP_DEINTERLACING;
        index++;
    }
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO)) {
        newList[index] = MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO;
        index++;
    }

    /* [IStab] FILTER */
#if defined(MFX_ENABLE_IMAGE_STABILIZATION_VPP)
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_IMAGE_STABILIZATION)) {
        newList[index] = MFX_EXTBUFF_VPP_IMAGE_STABILIZATION;
        index++;
    }
#endif

    // Resize for Best Quality
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_RESIZE)) {
        newList[index] = MFX_EXTBUFF_VPP_RESIZE;
        index++;
    }

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_DETAIL)) {
        newList[index] = MFX_EXTBUFF_VPP_DETAIL;
        index++;
    }

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_PROCAMP)) {
        newList[index] = MFX_EXTBUFF_VPP_PROCAMP;
        index++;
    }

#ifdef MFX_UNDOCUMENTED_VPP_VARIANCE_REPORT
    /* [VarianceRep] FILTER */
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_VARIANCE_REPORT)) {
        newList[index] = MFX_EXTBUFF_VPP_VARIANCE_REPORT;
        index++;
    }
#endif

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION)) {
        newList[index] = MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION;
        index++;
    }

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_SCENE_ANALYSIS)) {
        newList[index] = MFX_EXTBUFF_VPP_SCENE_ANALYSIS;
        index++;
    }

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_CSC_OUT_RGB4)) {
        newList[index] = MFX_EXTBUFF_VPP_CSC_OUT_RGB4;
        index++;
    }

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_CSC_OUT_A2RGB10)) {
        newList[index] = MFX_EXTBUFF_VPP_CSC_OUT_A2RGB10;
        index++;
    }

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_COMPOSITE)) {
        newList[index] = MFX_EXTBUFF_VPP_COMPOSITE;
        index++;
    }

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_FIELD_PROCESSING)) {
        newList[index] = MFX_EXTBUFF_VPP_FIELD_PROCESSING;
        index++;
    }

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_FIELD_WEAVING)) {
        newList[index] = MFX_EXTBUFF_VPP_FIELD_WEAVING;
        index++;
    }

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_FIELD_SPLITTING)) {
        newList[index] = MFX_EXTBUFF_VPP_FIELD_SPLITTING;
        index++;
    }

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_LSHIFT_OUT)) {
        newList[index] = MFX_EXTBUFF_VPP_LSHIFT_OUT;
        index++;
    }

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_ROTATION)) {
        newList[index] = MFX_EXTBUFF_VPP_ROTATION;
        index++;
    }

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_SCALING)) {
        newList[index] = MFX_EXTBUFF_VPP_SCALING;
        index++;
    }

#if (MFX_VERSION >= 1025)
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_COLOR_CONVERSION)) {
        newList[index] = MFX_EXTBUFF_VPP_COLOR_CONVERSION;
        index++;
    }
#endif

    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_MIRRORING)) {
        newList[index] = MFX_EXTBUFF_VPP_MIRRORING;
        index++;
    }

    // [1] update
    pipelineList.resize(index);
    for (index = 0; index < (mfxU32)pipelineList.size(); index++) {
        pipelineList[index] = newList[index];
    }
}

void CpuVPP::ReorderPipelineListForSpeed(mfxVideoParam* videoParam,
                                         std::vector<mfxU32>& pipelineList) {
    // optimization in case of FRC
    if (IsFilterFound(&pipelineList[0],
                      (mfxU32)pipelineList.size(),
                      MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION)) {
        mfxFrameInfo* in  = &(videoParam->vpp.In);
        mfxFrameInfo* out = &(videoParam->vpp.Out);

        mfxF64 inFrameRate =
            CalculateUMCFramerate(in->FrameRateExtN, in->FrameRateExtD);
        mfxF64 outFrameRate =
            CalculateUMCFramerate(out->FrameRateExtN, out->FrameRateExtD);

        mfxU32 filterIndex = 0;
        mfxU32 filterIndexFRC =
            GetFilterIndex(&pipelineList[0],
                           (mfxU32)pipelineList.size(),
                           MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION);

        if (inFrameRate > outFrameRate) {
            // FRC_DOWN must be first filter in pipeline
            for (filterIndex = filterIndexFRC; filterIndex > 0; filterIndex--) {
                std::swap(pipelineList[filterIndex],
                          pipelineList[filterIndex - 1]);
            }
            //exclude CSC
            if (IsFilterFound(&pipelineList[0],
                              (mfxU32)pipelineList.size(),
                              MFX_EXTBUFF_VPP_CSC)) {
                std::swap(pipelineList[1], pipelineList[0]);
            }
        }
    }
}

void CpuVPP::ShowPipeline(std::vector<mfxU32> pipelineList) {
#if !defined(_DEBUG) && !defined(_WIN32) && !defined(_WIN64) || \
    !defined(LINUX) && !defined(LINUX32) && !defined(LINUX64)

    (void)pipelineList;
#endif

#ifdef _DEBUG
    #if defined(_WIN32) || defined(_WIN64)
    mfxU32 filterIndx;
    char cStr[256];

    sprintf_s(cStr, sizeof(cStr), "\nVPP PIPELINE: \n");
    printf("%s\n", cStr);

    for (filterIndx = 0; filterIndx < pipelineList.size(); filterIndx++) {
        switch (pipelineList[filterIndx]) {
            case (mfxU32)MFX_EXTBUFF_VPP_DENOISE: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "DENOISE");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_RSHIFT_IN: {
                sprintf_s(cStr,
                          sizeof(cStr),
                          "%s \n",
                          "MFX_EXTBUFF_VPP_RSHIFT_IN");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_RSHIFT_OUT: {
                sprintf_s(cStr,
                          sizeof(cStr),
                          "%s \n",
                          "MFX_EXTBUFF_VPP_RSHIFT_OUT");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_LSHIFT_IN: {
                sprintf_s(cStr,
                          sizeof(cStr),
                          "%s \n",
                          "MFX_EXTBUFF_VPP_LSHIFT_IN");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_LSHIFT_OUT: {
                sprintf_s(cStr,
                          sizeof(cStr),
                          "%s \n",
                          "MFX_EXTBUFF_VPP_LSHIFT_OUT");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_RESIZE: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "RESIZE");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "FRC");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_DI_30i60p: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "ADV DI");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_DI_WEAVE: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "WEAVE DI");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_ITC: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "ITC");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_DI: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "DI");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_CSC: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "CSC");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_CSC_OUT_RGB4: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "CSC_RGB4");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_CSC_OUT_A2RGB10: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "CSC_A2RGB10");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_SCENE_ANALYSIS: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "SA");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_PROCAMP: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "PROCAMP");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_DETAIL: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "DETAIL");
                printf("%s\n", cStr);
                break;
            }

        #if defined(MFX_ENABLE_IMAGE_STABILIZATION_VPP)
            case (mfxU32)MFX_EXTBUFF_VPP_IMAGE_STABILIZATION: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "IMAGE_STAB");
                printf("%s\n", cStr);
                break;
            }
        #endif

        #ifdef MFX_UNDOCUMENTED_VPP_VARIANCE_REPORT
            case (mfxU32)MFX_EXTBUFF_VPP_VARIANCE_REPORT: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "VARIANCE_REP");
                printf("%s\n", cStr);
                break;
            }
        #endif

            case (mfxU32)MFX_EXTBUFF_VPP_COMPOSITE: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "COMPOSITE");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_FIELD_PROCESSING: {
                sprintf_s(cStr, sizeof(cStr), "%s \n", "VPP_FIELD_PROCESSIN");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_ROTATION: {
                sprintf_s(cStr,
                          sizeof(cStr),
                          "%s \n",
                          "MFX_EXTBUFF_VPP_ROTATION");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_SCALING: {
                sprintf_s(cStr,
                          sizeof(cStr),
                          "%s \n",
                          "MFX_EXTBUFF_VPP_SCALING");
                printf("%s\n", cStr);
                break;
            }

        #ifndef MFX_FUTURE_FEATURE_DISABLE
            case (mfxU32)MFX_EXTBUFF_VPP_COLOR_CONVERSION: {
                sprintf_s(cStr,
                          sizeof(cStr),
                          "%s \n",
                          "MFX_EXTBUFF_VPP_COLOR_CONVERSION");
                printf("%s\n", cStr);
                break;
            }
        #endif
            case (mfxU32)MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO: {
                sprintf_s(cStr,
                          sizeof(cStr),
                          "%s \n",
                          "MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO");
                printf("%s\n", cStr);
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_MIRRORING: {
                sprintf_s(cStr,
                          sizeof(cStr),
                          "%s \n",
                          "MFX_EXTBUFF_VPP_MIRRORING");
                printf("%s\n", cStr);
                break;
            }

            default: {
            }

        } // CASE
    } //end of filter search

    sprintf_s(cStr, sizeof(cStr), "\n");
    printf("%s\n", cStr);
    #endif // #if defined(_WIN32) || defined(_WIN64)

    #if defined(LINUX) || defined(LINUX32) || defined(LINUX64)
    mfxU32 filterIndx;
    fprintf(stderr, "VPP PIPELINE: \n");

    for (filterIndx = 0; filterIndx < pipelineList.size(); filterIndx++) {
        switch (pipelineList[filterIndx]) {
            case (mfxU32)MFX_EXTBUFF_VPP_DENOISE: {
                fprintf(stderr, "DENOISE \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_RSHIFT_IN: {
                fprintf(stderr, "MFX_EXTBUFF_VPP_RSHIFT_IN \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_RSHIFT_OUT: {
                fprintf(stderr, "MFX_EXTBUFF_VPP_RSHIFT_OUT \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_LSHIFT_IN: {
                fprintf(stderr, "MFX_EXTBUFF_VPP_LSHIFT_IN \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_LSHIFT_OUT: {
                fprintf(stderr, "MFX_EXTBUFF_VPP_LSHIFT_OUT \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_RESIZE: {
                fprintf(stderr, "RESIZE \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION: {
                fprintf(stderr, "FRC \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_DI_30i60p: {
                fprintf(stderr, "ADV DI 30i60p \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_DI_WEAVE: {
                fprintf(stderr, "WEAVE DI\n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_ITC: {
                fprintf(stderr, "ITC \n");

                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_DI: {
                fprintf(stderr, "DI \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_DEINTERLACING: {
                fprintf(stderr, "DI EXT BUF\n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_CSC: {
                fprintf(stderr, "CSC_NV12 \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_CSC_OUT_RGB4: {
                fprintf(stderr, "%s \n", "CSC_RGB4");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_CSC_OUT_A2RGB10: {
                fprintf(stderr, "CSC_A2RGB10 \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_SCENE_ANALYSIS: {
                fprintf(stderr, "SA \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_PROCAMP: {
                fprintf(stderr, "PROCAMP \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_DETAIL: {
                fprintf(stderr, "DETAIL \n");
                break;
            }

        #if defined(MFX_ENABLE_IMAGE_STABILIZATION_VPP)
            case (mfxU32)MFX_EXTBUFF_VPP_IMAGE_STABILIZATION: {
                fprintf(stderr, "IMAGE_STAB \n");
                break;
            }
        #endif

        #ifdef MFX_UNDOCUMENTED_VPP_VARIANCE_REPORT
            case (mfxU32)MFX_EXTBUFF_VPP_VARIANCE_REPORT: {
                fprintf(stderr, "VARIANCE_REP \n");
                break;
            }
        #endif

            case (mfxU32)MFX_EXTBUFF_VPP_COMPOSITE: {
                fprintf(stderr, "COMPOSITE \n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_FIELD_PROCESSING: {
                fprintf(stderr, "VPP_FIELD_PROCESSING \n");
                break;
            }
            case (mfxU32)MFX_EXTBUFF_VPP_ROTATION: {
                fprintf(stderr, "VPP_ROTATION\n");
                break;
            }
            case (mfxU32)MFX_EXTBUFF_VPP_SCALING: {
                fprintf(stderr, "MFX_EXTBUFF_VPP_SCALING\n");
                break;
            }
        #ifndef MFX_FUTURE_FEATURE_DISABLE
            case (mfxU32)MFX_EXTBUFF_VPP_COLOR_CONVERSION: {
                fprintf(stderr, "MFX_EXTBUFF_VPP_COLOR_CONVERSION\n");
                break;
            }
        #endif
            case (mfxU32)MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO: {
                fprintf(stderr, "MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO\n");
                break;
            }

            case (mfxU32)MFX_EXTBUFF_VPP_MIRRORING: {
                fprintf(stderr, "MFX_EXTBUFF_VPP_MIRRORING\n");
                break;
            }
            case (mfxU32)MFX_EXTBUFF_VPP_FIELD_WEAVING: {
                fprintf(stderr, "VPP_FIELD_WEAVING\n");
                break;
            }
            case (mfxU32)MFX_EXTBUFF_VPP_FIELD_SPLITTING: {
                fprintf(stderr, "VPP_FIELD_SPLITTING\n");
                break;
            }

            default: {
                fprintf(stderr, "UNKNOWN Filter ID!!! \n");
                break;
            }

        } // CASE
    } //end of filter search

        //fprintf(stderr,"\n");
    #endif // #if defined(LINUX) || defined(LINUX32) || defined(LINUX64)

#endif //#ifdef _DEBUG
    return;
} // void ShowPipeline( std::vector<mfxU32> pipelineList )

mfxStatus CpuVPP::GetPipelineList(mfxVideoParam* videoParam,
                                  std::vector<mfxU32>& pipelineList,
                                  bool bExtended) {
    mfxInfoVPP* par            = NULL;
    mfxFrameInfo* srcFrameInfo = NULL;
    mfxFrameInfo* dstFrameInfo = NULL;
    mfxU16 srcW = 0, dstW = 0;
    mfxU16 srcH = 0, dstH = 0;
    //mfxU32  lenList = 0;

    RET_IF_FALSE(videoParam, MFX_ERR_NULL_PTR);

    //MFX_CHECK_NULL_PTR2( pList, pLen );

    par          = &(videoParam->vpp);
    srcFrameInfo = &(par->In);
    dstFrameInfo = &(par->Out);
    /* ************************************************************************** */
    /* [1] the filter chain first based on input and output mismatch formats only */
    /* ************************************************************************** */
    if ((MFX_FOURCC_RGB4 != par->In.FourCC) ||
        (MFX_FOURCC_RGB4 != par->Out.FourCC)) {
        switch (par->In.FourCC) {
            case MFX_FOURCC_P010:
                switch (par->Out.FourCC) {
                    case MFX_FOURCC_NV12:
                        pipelineList.push_back(MFX_EXTBUFF_VPP_CSC);
                        break;
                }
                break;

            case MFX_FOURCC_NV12:
                switch (par->Out.FourCC) {
                    case MFX_FOURCC_NV16:
                    case MFX_FOURCC_P010:
                        pipelineList.push_back(MFX_EXTBUFF_VPP_CSC);
                        break;
                    case MFX_FOURCC_RGB4:
                        pipelineList.push_back(MFX_EXTBUFF_VPP_CSC_OUT_RGB4);
                        break;
                }
                break;
            default:
                switch (par->Out.FourCC) {
                    case MFX_FOURCC_RGB4:
                        pipelineList.push_back(MFX_EXTBUFF_VPP_CSC_OUT_RGB4);
                        break;
                }
                break;
        }

        if (MFX_FOURCC_NV12 != par->In.FourCC &&
            MFX_FOURCC_P010 != par->In.FourCC &&
            MFX_FOURCC_P210 != par->In.FourCC) {
            /* [Color Space Conversion] FILTER */
            pipelineList.push_back(MFX_EXTBUFF_VPP_CSC);
        }
    }
    else if (!bExtended) {
        /* ********************************************************************** */
        /* RGB32->RGB32 (resize only)                                             */
        /* ********************************************************************** */
        pipelineList.push_back(MFX_EXTBUFF_VPP_RESIZE);

        return MFX_ERR_NONE;
    }

    /* VPP natively supports P010 and P210 formats w/o shift. If input is shifted,
	 * need get it back to normal position.
	 */
    if ((MFX_FOURCC_P010 == srcFrameInfo->FourCC ||
         MFX_FOURCC_P210 == srcFrameInfo->FourCC) &&
        srcFrameInfo->Shift) {
        pipelineList.push_back(MFX_EXTBUFF_VPP_RSHIFT_IN);
    }

    /*
	 * VPP produces P010 and P210 formats w/o shift. If output is requested to be shifted, need to do so
	 */
    if ((MFX_FOURCC_P010 == dstFrameInfo->FourCC ||
         MFX_FOURCC_P210 == dstFrameInfo->FourCC) &&
        dstFrameInfo->Shift) {
        pipelineList.push_back(MFX_EXTBUFF_VPP_LSHIFT_OUT);
    }

    /* [Resize] FILTER */
    VPP_GET_REAL_WIDTH(srcFrameInfo, srcW);
    VPP_GET_REAL_HEIGHT(srcFrameInfo, srcH);

    /* OUT */
    VPP_GET_REAL_WIDTH(dstFrameInfo, dstW);
    VPP_GET_REAL_HEIGHT(dstFrameInfo, dstH);

    { //resize or cropping
        pipelineList.push_back(MFX_EXTBUFF_VPP_RESIZE);
    }

    /* [Deinterlace] FILTER */
#if 0
	mfxU32 extParamCount = MFX_MAX(sizeof(g_TABLE_CONFIG) / sizeof(*g_TABLE_CONFIG), videoParam->NumExtParam);
	std::vector<mfxU32> extParamList(extParamCount);

	GetConfigurableFilterList(videoParam, &extParamList[0], &extParamCount);

	mfxU32*   pExtBufList = NULL;
	mfxU32    extBufCount = 0;
#endif
    if (0 != videoParam->NumExtParam && NULL == videoParam->ExtParam) {
        return MFX_ERR_NULL_PTR;
    }
#if 0
	GetDoUseFilterList(videoParam, &pExtBufList, &extBufCount);

	extParamList.insert(extParamList.end(), &pExtBufList[0], &pExtBufList[extBufCount]);
	extParamCount = (mfxU32)extParamList.size();
#endif
    PicStructMode picStructMode;

    if ((par->In.PicStruct &
         (MFX_PICSTRUCT_FIELD_BFF | MFX_PICSTRUCT_FIELD_TFF)) &&
        MFX_PICSTRUCT_PROGRESSIVE == par->Out.PicStruct) {
        picStructMode = DYNAMIC_DI_PICSTRUCT_MODE;
    }
    else if (MFX_PICSTRUCT_UNKNOWN == par->In.PicStruct &&
             MFX_PICSTRUCT_PROGRESSIVE == par->Out.PicStruct) {
        picStructMode = DYNAMIC_DI_PICSTRUCT_MODE;
    }
    else {
        picStructMode = PASS_THROUGH_PICSTRUCT_MODE;
    }

    mfxI32 deinterlacingMode = 0;
    // look for user defined deinterlacing mode
    for (mfxU32 i = 0; i < videoParam->NumExtParam; i++) {
        if (videoParam->ExtParam[i] && videoParam->ExtParam[i]->BufferId ==
                                           MFX_EXTBUFF_VPP_DEINTERLACING) {
            mfxExtVPPDeinterlacing* extDI =
                (mfxExtVPPDeinterlacing*)videoParam->ExtParam[i];
            /* MSDK ignored all any DI modes values except two defined:
			 * MFX_DEINTERLACING_ADVANCED && MFX_DEINTERLACING_BOB
			 * If DI mode in Ext Buffer is not related BOB or ADVANCED Ext buffer ignored
			 * */
            if (extDI->Mode == MFX_DEINTERLACING_ADVANCED ||
#if defined(MFX_ENABLE_SCENE_CHANGE_DETECTION_VPP)
                extDI->Mode == MFX_DEINTERLACING_ADVANCED_SCD ||
#endif
                extDI->Mode == MFX_DEINTERLACING_BOB ||
                extDI->Mode == MFX_DEINTERLACING_ADVANCED_NOREF ||
                extDI->Mode == MFX_DEINTERLACING_FIELD_WEAVING) {
                /* DI Ext buffer present
				 * and DI type is correct
				 * */
                deinterlacingMode = extDI->Mode;
            }
            break;
        }
    }

    /* ********************************************************************** */
    /* 2. optional filters, enabled by default, disabled by DO_NOT_USE        */
    /* ********************************************************************** */

    // DO_NOT_USE structure is ignored by VPP since MSDK 3.0

    /* *************************************************************************** */
    /* 3. optional filters, disabled by default, enabled by DO_USE                 */
    /* *************************************************************************** */
    mfxU32* pExtList = NULL;
    mfxU32 extCount  = 0;

    GetDoUseFilterList(videoParam, &pExtList, &extCount);

    /* [Core Frame Rate Conversion] FILTER */
    /* must be used AFTER [Deinterlace] FILTER !!! due to SW performance specific */
    if (!IsFilterFound(&pipelineList[0],
                       (mfxU32)pipelineList.size(),
                       MFX_EXTBUFF_VPP_DI_30i60p) &&
        !IsFilterFound(&pipelineList[0],
                       (mfxU32)pipelineList.size(),
                       MFX_EXTBUFF_VPP_DI_WEAVE) &&
        !IsFilterFound(&pipelineList[0],
                       (mfxU32)pipelineList.size(),
                       MFX_EXTBUFF_VPP_ITC)) {
        if (IsFilterFound(pExtList,
                          extCount,
                          MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION) ||
            (par->In.FrameRateExtN * par->Out.FrameRateExtD !=
             par->Out.FrameRateExtN * par->In.FrameRateExtD)) {
            pipelineList.push_back(MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION);
        }
    }

    mfxU32 searchCount = sizeof(g_TABLE_DO_USE) / sizeof(*g_TABLE_DO_USE);
    mfxU32 fCount      = extCount;
    mfxU32 fIdx        = 0;
    for (fIdx = 0; fIdx < fCount; fIdx++) {
        if (IsFilterFound(&g_TABLE_DO_USE[0], searchCount, pExtList[fIdx]) &&
            !IsFilterFound(&pipelineList[0],
                           (mfxU32)pipelineList.size(),
                           pExtList[fIdx])) {
            pipelineList.push_back(pExtList[fIdx]);
        }
    }

    /* *************************************************************************** */
    /* 4. optional filters, disabled by default, enabled by EXT_BUFFER             */
    /* *************************************************************************** */
    mfxU32 configCount =
        MFX_MAX(sizeof(g_TABLE_CONFIG) / sizeof(*g_TABLE_CONFIG),
                videoParam->NumExtParam);
    if (configCount) {
        std::vector<mfxU32> configList(configCount);

        GetConfigurableFilterList(videoParam, &configList[0], &configCount);

        /* [FrameRateConversion] FILTER */
        if (IsFilterFound(&configList[0],
                          configCount,
                          MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION) &&
            !IsFilterFound(&pipelineList[0],
                           (mfxU32)pipelineList.size(),
                           MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION)) {
            if (!IsFilterFound(&pipelineList[0],
                               (mfxU32)pipelineList.size(),
                               MFX_EXTBUFF_VPP_DI_30i60p) &&
                !IsFilterFound(&pipelineList[0],
                               (mfxU32)pipelineList.size(),
                               MFX_EXTBUFF_VPP_ITC)) {
                pipelineList.push_back(MFX_EXTBUFF_VPP_FRAME_RATE_CONVERSION);
            }
        }

        /* ROTATION FILTER */
        if (IsFilterFound(&configList[0],
                          configCount,
                          MFX_EXTBUFF_VPP_ROTATION) &&
            !IsFilterFound(&pipelineList[0],
                           (mfxU32)pipelineList.size(),
                           MFX_EXTBUFF_VPP_ROTATION)) {
            if (!IsFilterFound(&pipelineList[0],
                               (mfxU32)pipelineList.size(),
                               MFX_EXTBUFF_VPP_ROTATION)) {
                pipelineList.push_back(MFX_EXTBUFF_VPP_ROTATION);
            }
        }

        if (IsFilterFound(&configList[0],
                          configCount,
                          MFX_EXTBUFF_VPP_SCALING) &&
            !IsFilterFound(&pipelineList[0],
                           (mfxU32)pipelineList.size(),
                           MFX_EXTBUFF_VPP_SCALING)) {
            if (!IsFilterFound(&pipelineList[0],
                               (mfxU32)pipelineList.size(),
                               MFX_EXTBUFF_VPP_SCALING)) {
                pipelineList.push_back(MFX_EXTBUFF_VPP_SCALING);
            }
        }

#if (MFX_VERSION >= 1025)
        if (IsFilterFound(&configList[0],
                          configCount,
                          MFX_EXTBUFF_VPP_COLOR_CONVERSION) &&
            !IsFilterFound(&pipelineList[0],
                           (mfxU32)pipelineList.size(),
                           MFX_EXTBUFF_VPP_COLOR_CONVERSION)) {
            if (!IsFilterFound(&pipelineList[0],
                               (mfxU32)pipelineList.size(),
                               MFX_EXTBUFF_VPP_COLOR_CONVERSION)) {
                pipelineList.push_back(MFX_EXTBUFF_VPP_COLOR_CONVERSION);
            }
        }
#endif

        if (IsFilterFound(&configList[0],
                          configCount,
                          MFX_EXTBUFF_VPP_MIRRORING) &&
            !IsFilterFound(&pipelineList[0],
                           (mfxU32)pipelineList.size(),
                           MFX_EXTBUFF_VPP_MIRRORING)) {
            if (!IsFilterFound(&pipelineList[0],
                               (mfxU32)pipelineList.size(),
                               MFX_EXTBUFF_VPP_MIRRORING)) {
                pipelineList.push_back(MFX_EXTBUFF_VPP_MIRRORING);
            }
        }

        if (IsFilterFound(&configList[0],
                          configCount,
                          MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO) &&
            !IsFilterFound(&pipelineList[0],
                           (mfxU32)pipelineList.size(),
                           MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO)) {
            if (!IsFilterFound(&pipelineList[0],
                               (mfxU32)pipelineList.size(),
                               MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO)) {
                pipelineList.push_back(MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO);
            }
        }

        searchCount = sizeof(g_TABLE_CONFIG) / sizeof(*g_TABLE_CONFIG);
        fCount      = configCount;
        for (fIdx = 0; fIdx < fCount; fIdx++) {
            if (IsFilterFound(g_TABLE_CONFIG, searchCount, configList[fIdx]) &&
                !IsFilterFound(&pipelineList[0],
                               (mfxU32)pipelineList.size(),
                               configList[fIdx])) {
                /* Add filter to the list.
                * Don't care about duplicates, they will be eliminated by Reorder... calls below
                */
                pipelineList.push_back(configList[fIdx]);
            } /* if( IsFilterFound( g_TABLE_CONFIG */
        } /*for(fIdx = 0; fIdx < fCount; fIdx++)*/
    }

    /* *************************************************************************** */
    /* 5. reordering for speed/quality                                             */
    /* *************************************************************************** */
    if (pipelineList.size() > 1) {
        ReorderPipelineListForQuality(pipelineList);
        ReorderPipelineListForSpeed(videoParam, pipelineList);
    }

    //if (pipelineList.size() > 0)
    //ShowPipeline(pipelineList);

    return ((pipelineList.size() > 0) ? MFX_ERR_NONE
                                      : MFX_ERR_INVALID_VIDEO_PARAM);
}

mfxStatus CpuVPP::CheckIOPattern(mfxVideoParam* par) {
    if (0 == par->IOPattern) // IOPattern is mandatory parameter
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (par->IOPattern & MFX_IOPATTERN_IN_VIDEO_MEMORY ||
        par->IOPattern & MFX_IOPATTERN_OUT_VIDEO_MEMORY)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    return MFX_ERR_NONE;
}

// check each field of FrameInfo excluding PicStruct
mfxStatus CpuVPP::CheckFrameInfo(mfxFrameInfo* info, mfxU32 request) {
    mfxStatus mfxSts = MFX_ERR_NONE;

    /* FourCC */
    switch (info->FourCC) {
        case MFX_FOURCC_NV12:
        case MFX_FOURCC_RGB4:
        case MFX_FOURCC_P210:
        case MFX_FOURCC_NV16:
        case MFX_FOURCC_YUY2:
        case MFX_FOURCC_I420:
        case MFX_FOURCC_I010:
            break;
        case MFX_FOURCC_P010:
            if (info->Shift == 0)
                return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    /* Picture Size */
    if (0 == info->Width || 0 == info->Height) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    // no need to limit in cpu plugin
    //if ((info->Width & 15) != 0) {
    //	return MFX_ERR_INVALID_VIDEO_PARAM;
    //}

#if 0
	/* AspectRatio */
	if ((info->AspectRatioW || info->AspectRatioH) && !(info->AspectRatioW && info->AspectRatioH))
		return MFX_ERR_INVALID_VIDEO_PARAM;
#endif

    /* Frame Rate */
    if (0 == info->FrameRateExtN || 0 == info->FrameRateExtD) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    // no need to limit in cpu plugin
    /*
	// checking Height based on PicStruct filed
	if (MFX_PICSTRUCT_PROGRESSIVE & info->PicStruct ||
		MFX_PICSTRUCT_FIELD_SINGLE & info->PicStruct) {
		if ((info->Height & 15) != 0) {
			return MFX_ERR_INVALID_VIDEO_PARAM)
		}
	}
	else if (MFX_PICSTRUCT_FIELD_BFF & info->PicStruct ||
		MFX_PICSTRUCT_FIELD_TFF & info->PicStruct ||
		(MFX_PICSTRUCT_UNKNOWN == info->PicStruct)) {
		if ((info->Height & 15) != 0) {
			return MFX_ERR_INVALID_VIDEO_PARAM)
		}
	}
	else {  //error protection
		return MFX_ERR_INVALID_VIDEO_PARAM;
	}
	*/
    return mfxSts;
}

bool CpuVPP::GetExtParamList(mfxVideoParam* par, mfxU32* pList, mfxU32* pLen) {
    mfxU32 fIdx = 0;

    /* robustness */
    *pLen = 0;

    mfxU32 searchCount = sizeof(g_TABLE_EXT_PARAM) / sizeof(*g_TABLE_EXT_PARAM);
    mfxU32 fCount      = par->NumExtParam;
    bool bResOK        = true;

    for (fIdx = 0; fIdx < fCount; fIdx++) {
        mfxU32 curId = par->ExtParam[fIdx]->BufferId;
        if (IsFilterFound(g_TABLE_EXT_PARAM, searchCount, curId)) {
            if (!IsFilterFound(pList, *pLen, curId)) {
                pList[(*pLen)++] = curId;
            }
            else {
                bResOK = false; // duplicate ID
            }
        }
        else {
            bResOK = false; //invalid ID
        }
    }

    return bResOK;
}

mfxStatus CpuVPP::GetFilterParam(mfxVideoParam* par,
                                 mfxU32 filterName,
                                 mfxExtBuffer** ppHint) {
    RET_IF_FALSE(par && ppHint, MFX_ERR_NULL_PTR);

    *ppHint = NULL;

    if (par->ExtParam && par->NumExtParam > 0) {
        mfxU32 paramIndex;

        for (paramIndex = 0; paramIndex < par->NumExtParam; paramIndex++) {
            if (filterName == par->ExtParam[paramIndex]->BufferId) {
                *ppHint = par->ExtParam[paramIndex];
                break;
            }
        }
    }

    return MFX_ERR_NONE;
}

void CpuVPP::GetDoNotUseFilterList(mfxVideoParam* par,
                                   mfxU32** ppList,
                                   mfxU32* pLen) {
    mfxU32 i                    = 0;
    mfxExtVPPDoNotUse* pVPPHint = NULL;

    /* robustness */
    *ppList = NULL;
    *pLen   = 0;

    for (i = 0; i < par->NumExtParam; i++) {
        if (MFX_EXTBUFF_VPP_DONOTUSE == par->ExtParam[i]->BufferId) {
            pVPPHint = (mfxExtVPPDoNotUse*)(par->ExtParam[i]);
            *ppList  = pVPPHint->AlgList;
            *pLen    = pVPPHint->NumAlg;

            return;
        }
    }

    return;
}

bool CpuVPP::CheckFilterList(mfxU32* pList, mfxU32 count, bool bDoUseTable) {
    bool bResOK = true;
    // strong check
    if ((NULL == pList && count > 0) || (NULL != pList && count == 0)) {
        bResOK = false;
        return bResOK;
    }

    mfxU32 searchCount = sizeof(g_TABLE_DO_USE) / sizeof(*g_TABLE_DO_USE);
    mfxU32* pSearchTab = (mfxU32*)&g_TABLE_DO_USE[0];
    if (!bDoUseTable) {
        searchCount = sizeof(g_TABLE_DO_NOT_USE) / sizeof(*g_TABLE_DO_NOT_USE);
        pSearchTab  = (mfxU32*)&g_TABLE_DO_NOT_USE[0];
    }

    mfxU32 fIdx = 0;
    for (fIdx = 0; fIdx < count; fIdx++) {
        mfxU32 curId = pList[fIdx];

        if (!IsFilterFound(pSearchTab, searchCount, curId)) {
            bResOK = false; //invalid ID
        }
        else if (fIdx == count - 1) {
            continue;
        }
        else if (IsFilterFound(pList + 1 + fIdx, count - 1 - fIdx, curId)) {
            bResOK = false; //duplicate ID
        }
    }

    return bResOK;
}

mfxStatus CpuVPP::CheckExtParam(mfxExtBuffer** ppExtParam, mfxU16 count) {
    if ((NULL == ppExtParam && count > 0)) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    bool bError = false;

    // [1] ExtParam
    mfxVideoParam tmpParam;
    tmpParam.ExtParam    = ppExtParam;
    tmpParam.NumExtParam = count;

    mfxU32 extParamCount =
        sizeof(g_TABLE_EXT_PARAM) / sizeof(*g_TABLE_EXT_PARAM);
    std::vector<mfxU32> extParamList(extParamCount);
    if (!GetExtParamList(&tmpParam, &extParamList[0], &extParamCount)) {
        bError = true;
    }

    // [2] configurable
    mfxU32 configCount = sizeof(g_TABLE_CONFIG) / sizeof(*g_TABLE_CONFIG);
    std::vector<mfxU32> configList(configCount);

    GetConfigurableFilterList(&tmpParam, &configList[0], &configCount);

    //-----------------------------------------------------
    mfxStatus sts = MFX_ERR_NONE, sts_wrn = MFX_ERR_NONE;
    mfxU32 fIdx = 0;
    for (fIdx = 0; fIdx < configCount; fIdx++) {
        mfxU32 curId = configList[fIdx];

        mfxExtBuffer* pHint = NULL;
        GetFilterParam(&tmpParam, curId, &pHint);

        //3 status's could be returned only: MFX_ERR_NONE, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM, MFX_ERR_UNSUPPORTED
        // AL update: now 4 status, added MFX_WRN_FILTER_SKIPPED
        sts = ExtendedQuery(curId, pHint);

        if (MFX_WRN_INCOMPATIBLE_VIDEO_PARAM == sts ||
            MFX_WRN_FILTER_SKIPPED == sts) {
            sts_wrn = sts;
            sts     = MFX_ERR_NONE;
        }
        else if (MFX_ERR_UNSUPPORTED == sts) {
            bError = true;
            sts    = MFX_ERR_NONE;
        }
        RET_ERROR(sts); // for double check only
    }
    //-----------------------------------------------------

    // [3] Do NOT USE
    mfxU32* pDnuList = NULL;
    mfxU32 dnuCount  = 0;
    GetDoNotUseFilterList(&tmpParam, &pDnuList, &dnuCount);

    if (!CheckFilterList(pDnuList, dnuCount, false)) {
        bError = true;
    }

    for (mfxU32 extParIdx = 0; extParIdx < count; extParIdx++) {
        // configured via extended parameters filter should not be disabled
        if (IsFilterFound(pDnuList,
                          dnuCount,
                          ppExtParam[extParIdx]->BufferId)) {
            sts = MFX_ERR_INVALID_VIDEO_PARAM;
        }
        RET_ERROR(sts);
    }

    // [4] Do USE
    mfxU32* pDO_USE_List = NULL;
    mfxU32 douseCount    = 0;
    GetDoUseFilterList(&tmpParam, &pDO_USE_List, &douseCount);

    if (!CheckFilterList(pDO_USE_List, douseCount, true)) {
        bError = true;
    }

    // [5] cmp DO_USE vs DO_NOT_USE
    for (fIdx = 0; fIdx < dnuCount; fIdx++) {
        if (IsFilterFound(pDO_USE_List, douseCount, pDnuList[fIdx])) {
            bError = true;
        }
    }

    if (bError) {
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else if (MFX_ERR_NONE != sts_wrn) {
        return sts_wrn;
    }
    else {
        return MFX_ERR_NONE;
    }
}

mfxStatus CpuVPP::GetVPPSurface(mfxFrameSurface1** surface) {
    if (!m_vppSurfaces) {
        mfxFrameAllocRequest VPPRequest[2] = { 0 };
        VPPQueryIOSurf(nullptr, VPPRequest);

        auto pool = std::make_unique<CpuFramePool>();
        RET_ERROR(pool->Init(m_vppInFormat,
                             m_vppWidth,
                             m_vppHeight,
                             VPPRequest[0].NumFrameSuggested));
        m_vppSurfaces = std::move(pool);
    }

    return m_vppSurfaces->GetFreeSurface(surface);
}
