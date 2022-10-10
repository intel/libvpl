/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "mfx_samples_config.h"
#include "vpl_implementation_loader.h"

#include <stdarg.h>
#include <memory>
#include <regex>
#include <string>
#include "pipeline_encode.h"
#include "pipeline_region_encode.h"
#include "pipeline_user.h"
#include "version.h"

#define VAL_CHECK(val, argIdx, argName)                                                       \
    {                                                                                         \
        if (val) {                                                                            \
            PrintHelp(NULL,                                                                   \
                      MSDK_STRING("Input argument number %d \"%s\" require more parameters"), \
                      argIdx,                                                                 \
                      argName);                                                               \
            return MFX_ERR_UNSUPPORTED;                                                       \
        }                                                                                     \
    }

#ifdef MOD_ENC
    // Extensions for internal use, normally these macros are blank
    #include "extension_macros.h"
#else
    #define MOD_ENC_CREATE_PIPELINE
    #define MOD_ENC_PRINT_HELP
    #define MOD_ENC_PARSE_INPUT
#endif

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

void PrintHelp(msdk_char* strAppName, const msdk_char* strErrorMessage, ...) {
    msdk_printf(MSDK_STRING("Encoding Sample Version %s\n\n"), GetMSDKSampleVersion().c_str());

    if (strErrorMessage) {
        va_list args;
        msdk_printf(MSDK_STRING("ERROR: "));
        va_start(args, strErrorMessage);
        msdk_vprintf(strErrorMessage, args);
        va_end(args);
        msdk_printf(MSDK_STRING("\n\n"));
    }

    msdk_printf(
        MSDK_STRING(
            "Usage: %s <msdk-codecid> [<options>] -i InputYUVFile -o OutputEncodedFile -w width -h height\n"),
        strAppName);
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("Supported codecs, <msdk-codecid>:\n"));
    msdk_printf(
        MSDK_STRING("   <codecid>=h264|mpeg2|vc1|mvc|jpeg|av1 - built-in Media SDK codecs\n"));
    msdk_printf(MSDK_STRING(
        "   <codecid>=h265|vp9                    - in-box Media SDK plugins (may require separate downloading and installation)\n"));
    msdk_printf(MSDK_STRING("   If codecid is jpeg, -q option is mandatory.)\n"));
    msdk_printf(MSDK_STRING("Options: \n"));
#if defined(LINUX32) || defined(LINUX64)
    msdk_printf(MSDK_STRING("   [-device /path/to/device] - set graphics device for processing\n"));
    msdk_printf(
        MSDK_STRING("                                 For example: '-device /dev/dri/card0'\n"));
    msdk_printf(MSDK_STRING(
        "                                              '-device /dev/dri/renderD128'\n"));
    msdk_printf(MSDK_STRING(
        "                                 If not specified, defaults to the first Intel device found on the system\n"));
#endif
#if (defined(_WIN64) || defined(_WIN32))
    msdk_printf(MSDK_STRING(
        "   [-dual_gfx::<on,off,adaptive>] - prefer processing on both iGfx and dGfx simultaneously\n"));
#endif
#ifdef ONEVPL_EXPERIMENTAL
    #if (defined(_WIN64) || defined(_WIN32))
    msdk_printf(MSDK_STRING("   [-luid HighPart:LowPart] - setup adapter by LUID  \n"));
    msdk_printf(MSDK_STRING("                                 For example: \"0x0:0x8a46\"  \n"));
    #endif
    msdk_printf(MSDK_STRING("   [-pci domain:bus:device.function] - setup device with PCI \n"));
    msdk_printf(MSDK_STRING("                                 For example: \"0:3:0.0\"  \n"));
#endif
    msdk_printf(MSDK_STRING(
        "   [-dGfx] - prefer processing on dGfx (by default system decides), also can be set with index, for example: '-dGfx 1' \n"));
    msdk_printf(
        MSDK_STRING("   [-iGfx] - prefer processing on iGfx (by default system decides)\n"));
    msdk_printf(
        MSDK_STRING("   [-AdapterNum] - specifies adapter number for processing, starts from 0\n"));
    msdk_printf(MSDK_STRING(
        "   [-dispatcher:fullSearch]  - enable search for all available implementations in oneVPL dispatcher\n"));
    msdk_printf(MSDK_STRING(
        "   [-dispatcher:lowLatency]  - enable limited implementation search and query in oneVPL dispatcher\n"));
#ifdef MOD_ENC
    MOD_ENC_PRINT_HELP;
#endif
    msdk_printf(MSDK_STRING(
        "   [-nv12|nv16|yuy2|uyvy|ayuv|rgb4|p010|y210|y410|a2rgb10|p016|p210|y216|i010|i420] - input color format (by default YUV420 is expected).\n"));
    msdk_printf(MSDK_STRING(
        "   [-msb10] - 10-bit color format is expected to have data in Most Significant Bits of words.\n                 (LSB data placement is expected by default).\n                 This option also disables data shifting during file reading.\n"));
    msdk_printf(MSDK_STRING(
        "   [-ec::p010|yuy2|nv12|nv16|rgb4|ayuv|uyvy|y210|y410|p016|y216|i010|i420] - force output color format for encoder (conversion will be made if necessary). Default value: input color format\n"));
    msdk_printf(MSDK_STRING(
        "   [-tff|bff] - input stream is interlaced, top|bottom fielf first, if not specified progressive is expected\n"));
    msdk_printf(MSDK_STRING("   [-bref] - arrange B frames in B pyramid reference structure\n"));
    msdk_printf(MSDK_STRING(
        "   [-nobref] -  do not use B-pyramid (by default the decision is made by library)\n"));
    msdk_printf(MSDK_STRING(
        "   [-idr_interval size] - idr interval, default 0 means every I is an IDR, 1 means every other I frame is an IDR etc\n"));
    msdk_printf(MSDK_STRING("   [-f frameRate] - video frame rate (frames per second)\n"));
    msdk_printf(MSDK_STRING("   [-n number] - number of frames to process\n"));
    msdk_printf(MSDK_STRING(
        "   [-b bitRate] - encoded bit rate (Kbits per second), valid for H.264, H.265, MPEG2 and MVC encoders \n"));
    msdk_printf(MSDK_STRING(
        "   [-u usage] - target usage, valid for H.265, H.264, H.265, MPEG2 and MVC encoders. Expected values:\n"));
    msdk_printf(MSDK_STRING(
        "                veryslow(quality), slower, slow, medium(balanced), fast, faster, veryfast(speed)\n"));
    msdk_printf(MSDK_STRING(
        "   [-q quality] - mandatory quality parameter for JPEG encoder. In range [1,100]. 100 is the best quality. \n"));
    msdk_printf(MSDK_STRING(
        "   [-r distance] - Distance between I- or P- key frames (1 means no B-frames) \n"));
    msdk_printf(MSDK_STRING("   [-g size] - GOP size (default 256)\n"));
    msdk_printf(MSDK_STRING("   [-x numRefs]   - number of reference frames\n"));
    msdk_printf(MSDK_STRING(
        "   [-num_active_P numRefs]   - number of maximum allowed references for P frames (for HEVC only)\n"));
    msdk_printf(MSDK_STRING(
        "   [-num_active_BL0 numRefs] - number of maximum allowed references for B frames in L0 (for HEVC only)\n"));
    msdk_printf(MSDK_STRING(
        "   [-num_active_BL1 numRefs] - number of maximum allowed references for B frames in L1 (for HEVC only)\n"));
    msdk_printf(MSDK_STRING(
        "   [-TargetBitDepthLuma] - Encoding target bit depth for luma samples, by default same as source one \n"));
    msdk_printf(MSDK_STRING(
        "   [-TargetBitDepthChroma] - Encoding target bit depth for chroma samples, by default same as source one \n"));
    msdk_printf(MSDK_STRING(
        "   [-la] - use the look ahead bitrate control algorithm (LA BRC) (by default constant bitrate control method is used)\n"));
    msdk_printf(MSDK_STRING(
        "           for H.264, H.265 encoder. Supported only with -hw option on 4th Generation Intel Core processors. \n"));
    msdk_printf(MSDK_STRING(
        "   [-lad depth] - depth parameter for the LA BRC, the number of frames to be analyzed before encoding. In range [0,100] (0 - default: auto-select by mediasdk library).\n"));
    msdk_printf(MSDK_STRING("            may be 1 in the case when -mss option is specified \n"));
    msdk_printf(
        MSDK_STRING("   [-dstw width] - destination picture width, invokes VPP resizing\n"));
    msdk_printf(
        MSDK_STRING("   [-dsth height] - destination picture height, invokes VPP resizing\n"));
    msdk_printf(MSDK_STRING("   [-hw] - use platform specific SDK implementation (default)\n"));
    msdk_printf(MSDK_STRING(
        "   [-sw] - use software implementation, if not specified platform specific SDK implementation is used\n"));
    msdk_printf(MSDK_STRING(
        "   [-p plugin] - encoder plugin. Supported values: hevce_sw, hevce_gacc, hevce_hw, vp8e_hw, vp9e_hw, hevce_fei_hw\n"));
    msdk_printf(MSDK_STRING(
        "                              (optional for Media SDK in-box plugins, required for user-encoder ones)\n"));
    msdk_printf(
        MSDK_STRING("   [-path path] - path to plugin (valid only in pair with -p option)\n"));
    msdk_printf(MSDK_STRING(
        "   [-async]                 - depth of asynchronous pipeline. default value is 4. must be between 1 and 20.\n"));
    msdk_printf(MSDK_STRING("   [-gpucopy::<on,off>] Enable or disable GPU copy mode\n"));
    msdk_printf(
        MSDK_STRING("   [-robust:soft]           - Recovery from GPU hang by inserting an IDR\n"));
    msdk_printf(MSDK_STRING("   [-vbr]                   - variable bitrate control\n"));
    msdk_printf(MSDK_STRING("   [-cbr]                   - constant bitrate control\n"));
    msdk_printf(MSDK_STRING(
        "   [-vcm]                   - Video Conferencing Mode (VCM) bitrate control method\n"));
    msdk_printf(MSDK_STRING(
        "   [-qvbr quality]          - variable bitrate control algorithm with constant quality. Quality in range [1,51]. 1 is the best quality.\n"));
    msdk_printf(MSDK_STRING(
        "   [-icq quality]           - Intelligent Constant Quality (ICQ) bitrate control method. In range [1,51]. 1 is the best quality.\n"));
    msdk_printf(
        MSDK_STRING("   [-avbr]                  - average variable bitrate control algorithm \n"));
    msdk_printf(MSDK_STRING(
        "   [-convergence]           - bitrate convergence period for avbr, in the unit of frame \n"));
    msdk_printf(MSDK_STRING(
        "   [-accuracy]              - bitrate accuracy for avbr, in the range of [1, 100] \n"));
    msdk_printf(MSDK_STRING(
        "   [-cqp]                   - constant quantization parameter (CQP BRC) bitrate control method\n"));
    msdk_printf(MSDK_STRING(
        "                              (by default constant bitrate control method is used), should be used along with -qpi, -qpp, -qpb.\n"));
    msdk_printf(MSDK_STRING(
        "   [-qpi]                   - constant quantizer for I frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.\n"));
    msdk_printf(MSDK_STRING(
        "   [-qpp]                   - constant quantizer for P frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.\n"));
    msdk_printf(MSDK_STRING(
        "   [-qpb]                   - constant quantizer for B frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.\n"));
    msdk_printf(MSDK_STRING(
        "   [-round_offset_in file]  - use this file to set per frame inter/intra rounding offset(for AVC only)\n"));
    msdk_printf(MSDK_STRING(
        "   [-lowpower:<on,off>]     - Turn this option ON to enable QuickSync Fixed Function (low-power HW) encoding mode\n"));
    msdk_printf(MSDK_STRING(
        "   [-ir_type]               - Intra refresh type. 0 - no refresh, 1 - vertical refresh, 2 - horizontal refresh, 3 - slice refresh\n"));
    msdk_printf(MSDK_STRING(
        "   [-ir_cycle_size]         - Number of pictures within refresh cycle starting from 2\n"));
    msdk_printf(MSDK_STRING(
        "   [-ir_qp_delta]           - QP difference for inserted intra MBs. This is signed value in [-51, 51] range\n"));
    msdk_printf(MSDK_STRING(
        "   [-ir_cycle_dist]         - Distance between the beginnings of the intra-refresh cycles in frames\n"));
    msdk_printf(MSDK_STRING(
        "   [-gpb:<on,off>]          - Turn this option OFF to make HEVC encoder use regular P-frames instead of GPB\n"));
    msdk_printf(
        MSDK_STRING("   [-TransformSkip:<on,off>]- Turn this option ON to enable transformskip\n"));
    msdk_printf(MSDK_STRING(
        "   [-AvcTemporalLayers <array:Layer.Scale>]    Configures the temporal layers hierarchy"
        "                                            example: \"3 1 2 4 0 0 0 0 0\" - # 3 temporal layers(temporal_id = 0,1,2)\n"));
    msdk_printf(MSDK_STRING(
        "   [-BaseLayerPID <pid>]                       Sets priority ID for the base layer\n"));
    msdk_printf(MSDK_STRING(
        "   [-SPSId <pid>]                              Sets sequence parameter set ID\n"));
    msdk_printf(MSDK_STRING(
        "   [-PPSId <pid>]                              Sets picture parameter set ID\n"));
    msdk_printf(MSDK_STRING(
        "   [-PicTimingSEI:<on,off>]                    Enables or disables picture timing SEI\n"));
    msdk_printf(MSDK_STRING(
        "   [-NalHrdConformance:<on,off>]               Enables or disables picture HRD conformance\n"));
    msdk_printf(MSDK_STRING(
        "   [-VuiNalHrdParameters:<on,off>]             Enables or disables NAL HRD parameters in VUI header\n"));
    msdk_printf(MSDK_STRING(
        "   [-ppyr:<on,off>]         - Turn this option ON to enable P-pyramid (by default the decision is made by library)\n"));
    msdk_printf(MSDK_STRING(
        "   [-num_slice]             - number of slices in each video frame. 0 by default.\n"));
    msdk_printf(MSDK_STRING(
        "                              If num_slice equals zero, the encoder may choose any slice partitioning allowed by the codec standard.\n"));
    msdk_printf(MSDK_STRING(
        "   [-mss]                   - maximum slice size in bytes. Supported only with -hw and h264 codec. This option is not compatible with -num_slice option.\n"));
    msdk_printf(MSDK_STRING(
        "   [-mfs]                   - maximum frame size in bytes. Supported only with h264 and hevc codec for VBR mode.\n"));
    msdk_printf(MSDK_STRING(
        "   [-BitrateLimit:<on,off>] - Turn this flag ON to set bitrate limitations imposed by the SDK encoder. Off by default.\n"));
    msdk_printf(MSDK_STRING(
        "   [-re]                    - enable region encode mode. Works only with h265 encoder\n"));
    msdk_printf(MSDK_STRING("   [-trows rows]            - Number of rows for tiled encoding\n"));
    msdk_printf(
        MSDK_STRING("   [-tcols cols]            - Number of columns for tiled encoding\n"));
    msdk_printf(MSDK_STRING("   [-CodecProfile]          - specifies codec profile\n"));
    msdk_printf(MSDK_STRING("   [-CodecLevel]            - specifies codec level\n"));
    msdk_printf(MSDK_STRING("   [-GopOptFlag:closed]     - closed gop\n"));
    msdk_printf(MSDK_STRING("   [-GopOptFlag:strict]     - strict gop\n"));
    msdk_printf(MSDK_STRING("   [-AdaptiveI:<on,off>]    - Turn Adaptive I frames on/off\n"));
    msdk_printf(MSDK_STRING("   [-AdaptiveB:<on,off>]    - Turn Adaptive B frames on/off\n"));
    msdk_printf(MSDK_STRING(
        "   [-InitialDelayInKB]      - the decoder starts decoding after the buffer reaches the initial size InitialDelayInKB, \
                            which is equivalent to reaching an initial delay of InitialDelayInKB*8000/TargetKbps ms\n"));
    msdk_printf(MSDK_STRING(
        "   [-BufferSizeInKB ]       - represents the maximum possible size of any compressed frames\n"));
    msdk_printf(MSDK_STRING(
        "   [-MaxKbps ]              - for variable bitrate control, specifies the maximum bitrate at which \
                            the encoded data enters the Video Buffering Verifier buffer\n"));
    msdk_printf(
        MSDK_STRING("   [-ws]                    - specifies sliding window size in frames\n"));
    msdk_printf(MSDK_STRING(
        "   [-wb]                    - specifies the maximum bitrate averaged over a sliding window in Kbps\n"));
    msdk_printf(MSDK_STRING(
        "   [-amfs:<on,off>]         - adaptive max frame size. If set on, P or B frame size can exceed MaxFrameSize when the scene change is detected.\
                            It can benefit the video quality \n"));
    msdk_printf(MSDK_STRING(
        "   [-LowDelayBRC]           - strictly obey average frame size set by MaxKbps\n"));

#if defined(_WIN64) || defined(_WIN32)
    msdk_printf(MSDK_STRING(
        "   [-PartialOutput <mode> <<block_size>>]         - specify partial output mode [0 - slice, 1 - block <blocksize>B, 2 - any]\n"));
#endif

    msdk_printf(MSDK_STRING(
        "   [-signal:tm ]            - represents transfer matrix coefficients for mfxExtVideoSignalInfo. 0 - unknown, 1 - BT709, 2 - BT601\n"));
    msdk_printf(
        MSDK_STRING("   [-WeightedPred:default|implicit ]   - enables weighted prediction mode\n"));
    msdk_printf(MSDK_STRING(
        "   [-WeightedBiPred:default|implicit ] - enables weighted bi-prediction mode\n"));
    msdk_printf(MSDK_STRING(
        "   [-timeout]               - encoding in cycle not less than specific time in seconds\n"));
    msdk_printf(
        MSDK_STRING("   [-syncop_timeout]        - SyncOperation timeout in milliseconds\n"));
    msdk_printf(MSDK_STRING(
        "   [-perf_opt n]            - sets number of prefetched frames. In performance mode app preallocates buffer and loads first n frames\n"));
    msdk_printf(MSDK_STRING("   [-fps]                   - limits overall fps of pipeline\n"));
    msdk_printf(MSDK_STRING(
        "   [-uncut]                 - do not cut output file in looped mode (in case of -timeout option)\n"));
    msdk_printf(MSDK_STRING(
        "   [-dump fileName]         - dump MSDK components configuration to the file in text form\n"));
    msdk_printf(MSDK_STRING(
        "   [-qpfile <filepath>]     - if specified, the encoder will take frame parameters (frame number, QP, frame type) from text file\n"));
    msdk_printf(MSDK_STRING(
        "   [-tcbrctestfile <filepath>] - if specified, the encoder will take targetFrameSize parameters for TCBRC test from text file. \
                            The parameters for TCBRC should be calculated based on the channel width conditions, sample doesn't have this functionality. \
                            Therefore the file data from <filepath> is used for TCBRC test. This is a test model\n"));
    msdk_printf(MSDK_STRING(
        "   [-usei]                  - insert user data unregistered SEI. eg: 7fc92488825d11e7bb31be2e44b06b34:0:MSDK (uuid:type<0-preifx/1-suffix>:message)\n"));
    msdk_printf(MSDK_STRING(
        "                              the suffix SEI for HEVCe can be inserted when CQP used or HRD disabled\n"));
#if (MFX_VERSION >= MFX_VERSION_NEXT)
    msdk_printf(MSDK_STRING(
        "   [-dblk_alpha]            - alpha deblocking parameter. In range[-12,12]. 0 by default.\n"));
    msdk_printf(MSDK_STRING(
        "   [-dblk_beta]             - beta deblocking parameter. In range[-12,12]. 0 by default.\n"));
#endif
    msdk_printf(
        MSDK_STRING("   [-extbrc:<on,off,implicit>] - External BRC for AVC and HEVC encoders\n"));
    msdk_printf(MSDK_STRING("   [-encTools]     - enables enctools for AVC encoder\n"));
    msdk_printf(MSDK_STRING(
        "   [-et:adaptiveI:<on,off>] - flag for configuring “Frame type calculation” feature.\n"));
    msdk_printf(MSDK_STRING(
        "                      Distance between Intra frames depends on the content.\n"));
    msdk_printf(MSDK_STRING(
        "   [-et:adaptiveB:<on,off>] - flag for configuring “Frame type calculation” feature.\n"));
    msdk_printf(MSDK_STRING(
        "                      Distance between nearest P (or I) frames depends on the content.\n"));
    msdk_printf(MSDK_STRING(
        "   [-et:arefP:<on,off>]     - flag for configuring “Reference frame list calculation” feature. \n"));
    msdk_printf(MSDK_STRING(
        "                      The most useful reference frames are calculated for P frames.\n"));
    msdk_printf(MSDK_STRING(
        "   [-et:arefB:<on,off>]     - flag for configuring “Reference frame list calculation” feature. \n"));
    msdk_printf(MSDK_STRING(
        "                      The most useful reference frames are calculated for B frames.\n"));
    msdk_printf(MSDK_STRING(
        "   [-et:sc:<on,off>]        - flag for enabling “Scene change analysis” feature\n"));
    msdk_printf(MSDK_STRING(
        "   [-et:aLTR:<on,off>]      - flag for configuring “Reference frame list calculation” feature.\n"));
    msdk_printf(MSDK_STRING(
        "                      The most useful reference frames are calculated as LTR.\n"));
    msdk_printf(MSDK_STRING(
        "   [-et:apyrQP:<on,off>]    - flag for configuring “Delta QP hints” feature. Delta QP is calculated for P frames.\n"));
    msdk_printf(MSDK_STRING(
        "   [-et:apyrQB:<on,off>]    - flag for configuring “Delta QP hints” feature. Delta QP is calculated for B frames.\n"));
    msdk_printf(MSDK_STRING(
        "   [-et:brchints:<on,off>]  - flag for enabling “BRC buffer hints” feature: calculation of optimal frame size, HRD buffer fullness, etc.\n"));
    msdk_printf(MSDK_STRING(
        "   [-et:brc:<on,off>]       - flag for enabling functionality: QP calculation for frame encoding, encoding status calculation after frame encoding\n"));
    msdk_printf(MSDK_STRING(
        "   [-ScenarioInfo n]        - Sets scenario info. 0=unknown, 7=MFX_SCENARIO_GAME_STREAMING, 8=MFX_SCENARIO_REMOTE_GAMING \n"));
    msdk_printf(
        MSDK_STRING("   [-ExtBrcAdaptiveLTR:<on,off>] - Set AdaptiveLTR for implicit extbrc\n"));
    msdk_printf(MSDK_STRING(
        "   [-preset <default,dss,conference,gaming>] - Use particular preset for encoding parameters\n"));
    msdk_printf(MSDK_STRING("   [-pp] - Print preset parameters\n"));
    msdk_printf(MSDK_STRING("   [-ivf:<on,off>] - Turn IVF header on/off\n"));
    msdk_printf(MSDK_STRING(
        "   [-api_ver_init::<1x,2x>]  - select the api version for the session initialization\n"));
    msdk_printf(MSDK_STRING("   [-rbf] - read frame-by-frame from the input (sw lib only)\n"));

#if D3D_SURFACES_SUPPORT
    msdk_printf(MSDK_STRING("   [-d3d] - work with d3d surfaces\n"));
    msdk_printf(MSDK_STRING("   [-d3d11] - work with d3d11 surfaces\n"));
    msdk_printf(
        MSDK_STRING("   [-single_texture_d3d11 ] - single texture mode for d3d11 allocator \n"));
    msdk_printf(
        MSDK_STRING(
            "Example: %s h264|h265|mpeg2|jpeg -i InputYUVFile -o OutputEncodedFile -w width -h height -d3d -hw \n"),
        strAppName);
    msdk_printf(
        MSDK_STRING(
            "Example for MVC: %s mvc -i InputYUVFile_1 -i InputYUVFile_2 -o OutputEncodedFile -w width -h height \n"),
        strAppName);
#endif
#ifdef LIBVA_SUPPORT
    msdk_printf(MSDK_STRING("   [-vaapi] - work with vaapi surfaces\n"));
    msdk_printf(
        MSDK_STRING(
            "Example: %s h264|mpeg2|mvc -i InputYUVFile -o OutputEncodedFile -w width -h height -angle 180 -g 300 -r 1 \n"),
        strAppName);
#endif
#if defined(ENABLE_V4L2_SUPPORT)
    msdk_printf(
        MSDK_STRING("   [-d]                            - Device video node (eg: /dev/video0)\n"));
    msdk_printf(
        MSDK_STRING("   [-p]                            - Mipi Port number (eg: Port 0)\n"));
    msdk_printf(MSDK_STRING(
        "   [-m]                            - Mipi Mode Configuration [PREVIEW/CONTINUOUS/STILL/VIDEO]\n"));
    msdk_printf(
        MSDK_STRING("   [-uyvy]                        - Input Raw format types V4L2 Encode\n"));
    msdk_printf(
        MSDK_STRING("   [-YUY2]                        - Input Raw format types V4L2 Encode\n"));
    msdk_printf(MSDK_STRING("   [-i::v4l2]                        - To enable v4l2 option\n"));
    msdk_printf(
        MSDK_STRING(
            "Example: %s h264|mpeg2|mvc -i::v4l2 -o OutputEncodedFile -w width -h height -d /dev/video0 -uyvy -m preview -p 0\n"),
        strAppName);
#endif
    msdk_printf(MSDK_STRING(
        "   [-viewoutput] - instruct the MVC encoder to output each view in separate bitstream buffer. Depending on the number of -o options behaves as follows:\n"));
    msdk_printf(MSDK_STRING("                   1: two views are encoded in single file\n"));
    msdk_printf(MSDK_STRING("                   2: two views are encoded in separate files\n"));
    msdk_printf(MSDK_STRING(
        "                   3: behaves like 2 -o opitons was used and then one -o\n\n"));
    msdk_printf(
        MSDK_STRING(
            "Example: %s mvc -i InputYUVFile_1 -i InputYUVFile_2 -o OutputEncodedFile_1 -o OutputEncodedFile_2 -viewoutput -w width -h height \n"),
        strAppName);
    // user module options
    msdk_printf(MSDK_STRING("User module options: \n"));
    msdk_printf(MSDK_STRING(
        "   [-angle 180] - enables 180 degrees picture rotation before encoding, CPU implementation by default. Rotation requires NV12 input. Options -tff|bff, -dstw, -dsth, -d3d are not effective together with this one, -nv12 is required.\n"));
    msdk_printf(MSDK_STRING("   [-opencl] - rotation implementation through OPENCL\n\n"));
    msdk_printf(
        MSDK_STRING(
            "Example: %s h264|h265|mpeg2|mvc|jpeg -i InputYUVFile -o OutputEncodedFile -w width -h height -angle 180 -opencl \n"),
        strAppName);

    msdk_printf(MSDK_STRING("\n"));
}

mfxStatus ParseAdditionalParams(msdk_char* strInput[],
                                mfxU8 nArgNum,
                                mfxU8& i,
                                sInputParams* pParams) {
    if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-AvcTemporalLayers"))) {
        pParams->nTemp = 1;
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
        mfxU16 arr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        int j;
        msdk_sscanf(strInput[i + 1],
                    MSDK_STRING("%hu %hu %hu %hu %hu %hu %hu %hu"),
                    &arr[0],
                    &arr[1],
                    &arr[2],
                    &arr[3],
                    &arr[4],
                    &arr[5],
                    &arr[6],
                    &arr[7]);
        for (j = 0; j < 8; j++) {
            pParams->nTemporalLayers[j] = arr[j];
        }
        i += 1;
    }

    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-tcbrctestfile"))) {
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
        pParams->TCBRCFileMode    = true;
        pParams->strTCBRCFilePath = strInput[++i];
        i += 1;
    }

    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-BaseLayerPID"))) {
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
        if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nBaseLayerPID)) {
            PrintHelp(strInput[0], MSDK_STRING("BaseLayerPID is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }

    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-SPSId"))) {
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
        if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nSPSId)) {
            PrintHelp(strInput[0], MSDK_STRING("SPSId is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-PPSId"))) {
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
        if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nPPSId)) {
            PrintHelp(strInput[0], MSDK_STRING("PPSId is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-PicTimingSEI:on"))) {
        pParams->nPicTimingSEI = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-PicTimingSEI:off"))) {
        pParams->nPicTimingSEI = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-NalHrdConformance:on"))) {
        pParams->nNalHrdConformance = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-NalHrdConformance:off"))) {
        pParams->nNalHrdConformance = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-VuiNalHrdParameters:on"))) {
        pParams->nVuiNalHrdParameters = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-VuiNalHrdParameters:off"))) {
        pParams->nVuiNalHrdParameters = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-encTools"))) {
        pParams->bEncTools = true;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:adaptiveI:on"))) {
        pParams->etAdaptiveI = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:adaptiveB:on"))) {
        pParams->etAdaptiveB = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:arefP:on"))) {
        pParams->etArefP = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:arefB:on"))) {
        pParams->etArefB = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:sc:on"))) {
        pParams->etSceneChange = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:aLTR:on"))) {
        pParams->etALTR = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:apyrQP:on"))) {
        pParams->etApyrQP = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:apyrQB:on"))) {
        pParams->etApyrQB = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:brchints:on"))) {
        pParams->etBRCHints = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:brc:on"))) {
        pParams->etBRC = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:adaptiveI:off"))) {
        pParams->etAdaptiveI = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:adaptiveB:off"))) {
        pParams->etAdaptiveB = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:arefP:off"))) {
        pParams->etArefP = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:arefB:off"))) {
        pParams->etArefB = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:sc:off"))) {
        pParams->etSceneChange = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:aLTR:off"))) {
        pParams->etALTR = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:apyrQP:off"))) {
        pParams->etApyrQP = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:apyrQB:off"))) {
        pParams->etApyrQB = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:brchints:off"))) {
        pParams->etBRCHints = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-et:brc:off"))) {
        pParams->etBRC = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ScenarioInfo"))) {
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
        if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->ScenarioInfo)) {
            PrintHelp(strInput[0], MSDK_STRING("Scenario info is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-p210"))) {
        pParams->FileInputFourCC = MFX_FOURCC_P210;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-nv16"))) {
        pParams->FileInputFourCC = MFX_FOURCC_NV16;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-p016"))) {
        pParams->FileInputFourCC = MFX_FOURCC_P016;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-y216"))) {
        pParams->FileInputFourCC = MFX_FOURCC_Y216;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::yuy2"))) {
        pParams->EncodeFourCC = MFX_FOURCC_YUY2;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::nv12"))) {
        pParams->EncodeFourCC = MFX_FOURCC_NV12;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::rgb4"))) {
        pParams->EncodeFourCC = MFX_FOURCC_RGB4;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::ayuv"))) {
        pParams->EncodeFourCC = MFX_FOURCC_AYUV;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::uyvy"))) {
        pParams->EncodeFourCC = MFX_FOURCC_UYVY;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::nv16"))) {
        pParams->EncodeFourCC = MFX_FOURCC_NV16;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::y210"))) {
        pParams->EncodeFourCC = MFX_FOURCC_Y210;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::y410"))) {
        pParams->EncodeFourCC = MFX_FOURCC_Y410;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::p016"))) {
        pParams->EncodeFourCC = MFX_FOURCC_P016;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::y216"))) {
        pParams->EncodeFourCC = MFX_FOURCC_Y216;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-fps"))) {
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);

        if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nMaxFPS)) {
            PrintHelp(strInput[0], MSDK_STRING("overall fps is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-TargetBitDepthLuma"))) {
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);

        if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->TargetBitDepthLuma)) {
            PrintHelp(strInput[0], MSDK_STRING("TargetBitDepthLuma is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-TargetBitDepthChroma"))) {
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);

        if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->TargetBitDepthChroma)) {
            PrintHelp(strInput[0], MSDK_STRING("TargetBitDepthChroma is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
#if (defined(_WIN64) || defined(_WIN32))
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dual_gfx::on"))) {
        pParams->isDualMode = true;
        pParams->hyperMode  = MFX_HYPERMODE_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dual_gfx::off"))) {
        pParams->isDualMode = true;
        pParams->hyperMode  = MFX_HYPERMODE_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dual_gfx::adaptive"))) {
        pParams->isDualMode = true;
        pParams->hyperMode  = MFX_HYPERMODE_ADAPTIVE;
    }
#endif
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-icq"))) {
        pParams->nRateControlMethod = MFX_RATECONTROL_ICQ;

        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
        if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->ICQQuality)) {
            PrintHelp(strInput[0], MSDK_STRING("ICQQuality is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-cqp"))) {
        pParams->nRateControlMethod = MFX_RATECONTROL_CQP;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qpi"))) {
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
        if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nQPI)) {
            PrintHelp(strInput[0], MSDK_STRING("Quantizer for I frames is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qpp"))) {
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
        if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nQPP)) {
            PrintHelp(strInput[0], MSDK_STRING("Quantizer for P frames is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qpb"))) {
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
        if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nQPB)) {
            PrintHelp(strInput[0], MSDK_STRING("Quantizer for B frames is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-round_offset_in"))) {
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
        pParams->RoundingOffsetFile = strInput[++i];
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpb:on"))) {
        pParams->nGPB = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpb:off"))) {
        pParams->nGPB = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-TransformSkip:on"))) {
        pParams->nTransformSkip = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-TransformSkip:off"))) {
        pParams->nTransformSkip = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ivf:on"))) {
        pParams->nIVFHeader = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ivf:off"))) {
        pParams->nIVFHeader = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dispatcher:fullSearch"))) {
        pParams->dispFullSearch = true;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dispatcher:lowLatency"))) {
        pParams->dispFullSearch = false;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-api_ver_init::1x"))) {
        pParams->verSessionInit = API_1X;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-api_ver_init::2x"))) {
        pParams->verSessionInit = API_2X;
    }
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-rbf"))) {
        pParams->bReadByFrame = true;
    }
#ifdef ONEVPL_EXPERIMENTAL
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-pci"))) {
        msdk_char deviceInfo[MSDK_MAX_FILENAME_LEN];
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
        i++;
        if ((msdk_strlen(strInput[i]) + 1) > MSDK_ARRAY_LEN(deviceInfo)) {
            PrintHelp(strInput[0], MSDK_STRING("error: '-pci' arguments is too long\n"));
            return MFX_ERR_UNSUPPORTED;
        }
        if (MFX_ERR_NONE != msdk_opt_read(strInput[i], deviceInfo)) {
            PrintHelp(strInput[0], MSDK_STRING("error: '-pci' arguments is invalid\n"));
            return MFX_ERR_UNSUPPORTED;
        }

        // template: <domain:bus:device.function>
        std::string temp = std::string(deviceInfo);
        const std::regex pieces_regex("([0-9]+):([0-9]+):([0-9]+).([0-9]+)");
        std::smatch pieces_match;

        // pieces_match = [full match, PCIDomain, PCIBus, PCIDevice, PCIFunction]
        if (std::regex_match(temp, pieces_match, pieces_regex) && pieces_match.size() == 5) {
            pParams->PCIDomain      = std::atoi(pieces_match[1].str().c_str());
            pParams->PCIBus         = std::atoi(pieces_match[2].str().c_str());
            pParams->PCIDevice      = std::atoi(pieces_match[3].str().c_str());
            pParams->PCIFunction    = std::atoi(pieces_match[4].str().c_str());
            pParams->PCIDeviceSetup = true;
        }
        else {
            PrintHelp(
                strInput[0],
                MSDK_STRING(
                    "error: format of '-pci' arguments is invalid, please, use: domain:bus:device.function \n"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    #if defined(_WIN32)
    else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-luid"))) {
        // <HighPart:LowPart>
        msdk_char luid[MSDK_MAX_FILENAME_LEN];
        VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
        i++;
        if (MFX_ERR_NONE != msdk_opt_read(strInput[i], luid)) {
            PrintHelp(strInput[0], MSDK_STRING("error: '-luid' arguments is invalid\n"));
            return MFX_ERR_UNSUPPORTED;
        }

        std::string temp = std::string(luid);
        const std::regex pieces_regex("(0[xX][0-9a-fA-F]+):(0[xX][0-9a-fA-F]+)");
        std::smatch pieces_match;

        // pieces_match = [full match, HighPart, LowPart]
        if (std::regex_match(temp, pieces_match, pieces_regex) && pieces_match.size() == 3) {
            pParams->luid.HighPart = std::strtol(pieces_match[1].str().c_str(), 0, 16);
            pParams->luid.LowPart  = std::strtol(pieces_match[2].str().c_str(), 0, 16);
        }
        else {
            PrintHelp(
                strInput[0],
                MSDK_STRING(
                    "error: format of '-luid' arguments is invalid, please, use: HighPart:LowPart \n"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    #endif
#endif
    else {
        return MFX_ERR_NOT_FOUND;
    }
    return MFX_ERR_NONE;
}

mfxStatus ParseInputString(msdk_char* strInput[], mfxU8 nArgNum, sInputParams* pParams) {
    if (1 == nArgNum) {
        PrintHelp(strInput[0], NULL);
        return MFX_ERR_UNSUPPORTED;
    }

    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);
    msdk_opt_read(MSDK_CPU_ROTATE_PLUGIN, pParams->strPluginDLLPath);

    // default implementation
    pParams->bUseHWLib          = true;
    pParams->libType            = MFX_IMPL_HARDWARE_ANY;
    pParams->isV4L2InputEnabled = false;
    pParams->nNumFrames         = 0;
    pParams->FileInputFourCC    = MFX_FOURCC_I420;
    pParams->EncodeFourCC       = 0;
    pParams->nPRefType          = MFX_P_REF_DEFAULT;
    pParams->QPFileMode         = false;
    pParams->TCBRCFileMode      = false;
    pParams->BitrateLimit       = MFX_CODINGOPTION_OFF;
    pParams->adapterType        = mfxMediaAdapterType::MFX_MEDIA_UNKNOWN;
    pParams->dGfxIdx            = -1;
    pParams->adapterNum         = -1;
    pParams->dispFullSearch     = DEF_DISP_FULLSEARCH;
    pParams->RoundingOffsetFile = NULL;
#if defined(ENABLE_V4L2_SUPPORT)
    pParams->MipiPort   = -1;
    pParams->MipiMode   = NONE;
    pParams->v4l2Format = NO_FORMAT;
#endif

    // parse command line parameters
    for (mfxU8 i = 1; i < nArgNum; i++) {
        MSDK_CHECK_POINTER(strInput[i], MFX_ERR_NULL_PTR);

        if (MSDK_CHAR('-') != strInput[i][0]) {
            mfxStatus sts = StrFormatToCodecFormatFourCC(strInput[i], pParams->CodecId);
            if (sts != MFX_ERR_NONE) {
                PrintHelp(strInput[0], MSDK_STRING("Unknown codec"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (!IsEncodeCodecSupported(pParams->CodecId)) {
                PrintHelp(strInput[0], MSDK_STRING("Unsupported codec"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (pParams->CodecId == CODEC_MVC) {
                pParams->CodecId = MFX_CODEC_AVC;
                pParams->MVC_flags |= MVC_ENABLED;
            }
            continue;
        }

        // process multi-character options
        if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dstw"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nDstWidth)) {
                PrintHelp(strInput[0], MSDK_STRING("Destination picture Width is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dsth"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nDstHeight)) {
                PrintHelp(strInput[0], MSDK_STRING("Destination picture Height is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
#if (defined(LINUX32) || defined(LINUX64))
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-device"))) {
            if (!pParams->strDevicePath.empty()) {
                msdk_printf(MSDK_STRING("error: you can specify only one device\n"));
                return MFX_ERR_UNSUPPORTED;
            }
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            pParams->strDevicePath = strInput[++i];

            size_t pos = pParams->strDevicePath.find("renderD");
            if (pos != std::string::npos) {
                pParams->DRMRenderNodeNum = std::stoi(pParams->strDevicePath.substr(pos + 7, 3));
            }
        }
#endif
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dGfx"))) {
            pParams->adapterType = mfxMediaAdapterType::MFX_MEDIA_DISCRETE;
            if (i + 1 < nArgNum && isdigit(*strInput[1 + i])) {
                if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->dGfxIdx)) {
                    msdk_printf(MSDK_STRING("value of -dGfx is invalid\n"));
                    return MFX_ERR_UNSUPPORTED;
                }
            }
#if (defined(_WIN64) || defined(_WIN32)) && (MFX_VERSION >= 1031)
            pParams->bPreferdGfx = true;
#endif
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-iGfx"))) {
            pParams->adapterType = mfxMediaAdapterType::MFX_MEDIA_INTEGRATED;
#if (defined(_WIN64) || defined(_WIN32)) && (MFX_VERSION >= 1031)
            pParams->bPreferiGfx = true;
#endif
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-AdapterNum"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->adapterNum)) {
                PrintHelp(strInput[0], MSDK_STRING("Value of -AdapterNum is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-trows"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nEncTileRows)) {
                PrintHelp(strInput[0], MSDK_STRING("Encoding tile row count is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-tcols"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nEncTileCols)) {
                PrintHelp(strInput[0], MSDK_STRING("Encoding tile column count is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-sw"))) {
            pParams->bUseHWLib = false;
            pParams->libType   = MFX_IMPL_SOFTWARE;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-hw"))) {
            pParams->bUseHWLib = true;
            pParams->libType   = MFX_IMPL_HARDWARE;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-yuy2"))) {
#if defined(ENABLE_V4L2_SUPPORT)
            pParams->v4l2Format = YUY2;
#endif
            pParams->FileInputFourCC = MFX_FOURCC_YUY2;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-nv12"))) {
            pParams->FileInputFourCC = MFX_FOURCC_NV12;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-i420"))) {
            pParams->FileInputFourCC = MFX_FOURCC_I420;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-i010"))) {
            pParams->FileInputFourCC = MFX_FOURCC_I010;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-rgb4"))) {
            pParams->FileInputFourCC = MFX_FOURCC_RGB4;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-p010"))) {
            pParams->FileInputFourCC = MFX_FOURCC_P010;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ayuv"))) {
            pParams->FileInputFourCC = MFX_FOURCC_AYUV;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-uyvy"))) {
            pParams->FileInputFourCC = MFX_FOURCC_UYVY;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-y210"))) {
            pParams->FileInputFourCC = MFX_FOURCC_Y210;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-y410"))) {
            pParams->FileInputFourCC = MFX_FOURCC_Y410;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-a2rgb10"))) {
            pParams->FileInputFourCC = MFX_FOURCC_A2RGB10;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::p010"))) {
            pParams->EncodeFourCC = MFX_FOURCC_P010;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::i420"))) {
            pParams->FileInputFourCC = MFX_FOURCC_I420;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::i010"))) {
            pParams->FileInputFourCC = MFX_FOURCC_I010;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-tff"))) {
            pParams->nPicStruct = MFX_PICSTRUCT_FIELD_TFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-bff"))) {
            pParams->nPicStruct = MFX_PICSTRUCT_FIELD_BFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-bref"))) {
            pParams->nBRefType = MFX_B_REF_PYRAMID;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-nobref"))) {
            pParams->nBRefType = MFX_B_REF_OFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-idr_interval"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nIdrInterval)) {
                PrintHelp(strInput[0], MSDK_STRING("IdrInterval is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-msb10"))) {
            pParams->IsSourceMSB = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-angle"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nRotationAngle)) {
                PrintHelp(strInput[0], MSDK_STRING("Rotation Angle is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-opencl"))) {
            msdk_opt_read(MSDK_OCL_ROTATE_PLUGIN, pParams->strPluginDLLPath);
            pParams->nRotationAngle = 180;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-viewoutput"))) {
            if (!(MVC_ENABLED & pParams->MVC_flags)) {
                PrintHelp(
                    strInput[0],
                    MSDK_STRING("-viewoutput option is supported only when mvc codec specified"));
                return MFX_ERR_UNSUPPORTED;
            }
            pParams->MVC_flags |= MVC_VIEWOUTPUT;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-la"))) {
            if (!pParams->nRateControlMethod) {
                pParams->nRateControlMethod = MFX_RATECONTROL_LA;
            }
            else if (pParams->nRateControlMethod == MFX_RATECONTROL_ICQ) {
                pParams->nRateControlMethod = MFX_RATECONTROL_LA_ICQ;
            }
            else if (pParams->nRateControlMethod != MFX_RATECONTROL_LA &&
                     pParams->nRateControlMethod != MFX_RATECONTROL_LA_ICQ) {
                PrintHelp(
                    strInput[0],
                    MSDK_STRING(
                        "More than one BRC modes assigned, and another BRC mode isn't compatible with LA."));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-lad"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (!pParams->nRateControlMethod) {
                pParams->nRateControlMethod = MFX_RATECONTROL_LA;
            }
            else if (pParams->nRateControlMethod == MFX_RATECONTROL_ICQ) {
                pParams->nRateControlMethod = MFX_RATECONTROL_LA_ICQ;
            }
            else if (pParams->nRateControlMethod != MFX_RATECONTROL_LA &&
                     pParams->nRateControlMethod != MFX_RATECONTROL_LA_ICQ && !pParams->bEncTools) {
                PrintHelp(
                    strInput[0],
                    MSDK_STRING(
                        "More than one BRC modes assigned, and another BRC mode isn't compatible with LA."));
                return MFX_ERR_UNSUPPORTED;
            }

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nLADepth)) {
                PrintHelp(strInput[0], MSDK_STRING("Look Ahead Depth is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-vbr"))) {
            pParams->nRateControlMethod = MFX_RATECONTROL_VBR;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-cbr"))) {
            pParams->nRateControlMethod = MFX_RATECONTROL_CBR;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-vcm"))) {
            pParams->nRateControlMethod = MFX_RATECONTROL_VCM;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-single_texture_d3d11"))) {
            pParams->bSingleTexture = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qvbr"))) {
            pParams->nRateControlMethod = MFX_RATECONTROL_QVBR;
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->QVBRQuality)) {
                PrintHelp(strInput[0], MSDK_STRING("QVBRQuality param is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-avbr"))) {
            pParams->nRateControlMethod = MFX_RATECONTROL_AVBR;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-convergence"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->Convergence)) {
                PrintHelp(strInput[0], MSDK_STRING("convergence is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-accuracy"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->Accuracy)) {
                PrintHelp(strInput[0], MSDK_STRING("accuracy is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-mss"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nMaxSliceSize)) {
                PrintHelp(strInput[0], MSDK_STRING("MaxSliceSize is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-mfs"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nMaxFrameSize)) {
                PrintHelp(strInput[0], MSDK_STRING("MaxFrameSize is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-BitrateLimit:on"))) {
            pParams->BitrateLimit = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-BitrateLimit:off"))) {
            pParams->BitrateLimit = MFX_CODINGOPTION_OFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qpfile"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            MSDK_CHECK_ERROR(msdk_strlen(strInput[i + 1]), 0, MFX_ERR_NOT_INITIALIZED);
            pParams->QPFileMode    = true;
            pParams->strQPFilePath = strInput[++i];
        }
#if D3D_SURFACES_SUPPORT
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-d3d"))) {
            pParams->memType          = D3D9_MEMORY;
            pParams->accelerationMode = MFX_ACCEL_MODE_VIA_D3D9;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-d3d11"))) {
            pParams->memType          = D3D11_MEMORY;
            pParams->accelerationMode = MFX_ACCEL_MODE_VIA_D3D11;
        }
#endif
#ifdef LIBVA_SUPPORT
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-vaapi"))) {
            pParams->memType          = D3D9_MEMORY;
            pParams->accelerationMode = MFX_ACCEL_MODE_VIA_VAAPI;
        }
#endif
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-async"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nAsyncDepth)) {
                PrintHelp(strInput[0], MSDK_STRING("Async Depth is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-CodecLevel"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->CodecLevel)) {
                PrintHelp(strInput[0], MSDK_STRING("CodecLevel is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-CodecProfile"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->CodecProfile)) {
                PrintHelp(strInput[0], MSDK_STRING("CodecProfile is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-GopOptFlag:closed"))) {
            pParams->GopOptFlag = MFX_GOP_CLOSED;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-GopOptFlag:strict"))) {
            pParams->GopOptFlag = MFX_GOP_STRICT;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-AdaptiveI:on"))) {
            pParams->AdaptiveI = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-AdaptiveI:off"))) {
            pParams->AdaptiveI = MFX_CODINGOPTION_OFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-AdaptiveB:on"))) {
            pParams->AdaptiveB = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-AdaptiveB:off"))) {
            pParams->AdaptiveB = MFX_CODINGOPTION_OFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-InitialDelayInKB"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->InitialDelayInKB)) {
                PrintHelp(strInput[0], MSDK_STRING("InitialDelayInKB is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-MaxKbps"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->MaxKbps)) {
                PrintHelp(strInput[0], MSDK_STRING("MaxKbps is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-LowDelayBRC"))) {
            pParams->LowDelayBRC = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-BufferSizeInKB"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->BufferSizeInKB)) {
                PrintHelp(strInput[0], MSDK_STRING("BufferSizeInKB is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ws"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->WinBRCSize)) {
                PrintHelp(strInput[0], MSDK_STRING("Sliding window size is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-wb"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->WinBRCMaxAvgKbps)) {
                PrintHelp(strInput[0], MSDK_STRING("Sliding window bitrate is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-timeout"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nTimeout)) {
                PrintHelp(strInput[0], MSDK_STRING("Timeout is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-syncop_timeout"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nSyncOpTimeout)) {
                PrintHelp(strInput[0], MSDK_STRING("syncop_timeout is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-signal:tm"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->TransferMatrix)) {
                PrintHelp(strInput[0], MSDK_STRING("Transfer matrix param is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ir_type"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->IntRefType)) {
                PrintHelp(strInput[0], MSDK_STRING("Intra refresh type is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ir_cycle_size"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->IntRefCycleSize)) {
                PrintHelp(strInput[0], MSDK_STRING("IR refresh cycle size param is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ir_qp_delta"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->IntRefQPDelta)) {
                PrintHelp(strInput[0], MSDK_STRING("IR QP delta param is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ir_cycle_dist"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->IntRefCycleDist)) {
                PrintHelp(strInput[0], MSDK_STRING("IR cycle distance param is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-perf_opt"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nPerfOpt)) {
                PrintHelp(strInput[0], MSDK_STRING("perf_opt is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-WeightedPred:default"))) {
            pParams->WeightedPred = MFX_WEIGHTED_PRED_DEFAULT;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-WeightedPred:implicit"))) {
            pParams->WeightedPred = MFX_WEIGHTED_PRED_IMPLICIT;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-WeightedBiPred:default"))) {
            pParams->WeightedBiPred = MFX_WEIGHTED_PRED_DEFAULT;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-WeightedBiPred:implicit"))) {
            pParams->WeightedBiPred = MFX_WEIGHTED_PRED_IMPLICIT;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-uncut"))) {
            pParams->bUncut = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpucopy::on"))) {
            pParams->gpuCopy = MFX_GPUCOPY_ON;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpucopy::off"))) {
            pParams->gpuCopy = MFX_GPUCOPY_OFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dump"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->DumpFileName)) {
                PrintHelp(
                    strInput[0],
                    MSDK_STRING(
                        "File Name for dumping MSDK library configuration should be provided"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-usei"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (msdk_strlen(strInput[i + 1]) > MSDK_MAX_USER_DATA_UNREG_SEI_LEN) {
                PrintHelp(strInput[0], MSDK_STRING("error: '-usei' arguments is too long\n"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->uSEI)) {
                PrintHelp(strInput[0], MSDK_STRING("error: option '-usei' expects arguments\n"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (msdk_strlen(pParams->uSEI) < 32) {
                PrintHelp(strInput[0],
                          MSDK_STRING("error: option '-usei' expects at least 32 char uuid\n"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ppyr:on"))) {
            pParams->nPRefType = MFX_P_REF_PYRAMID;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ppyr:off"))) {
            pParams->nPRefType = MFX_P_REF_SIMPLE;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-num_active_P"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nNumRefActiveP)) {
                PrintHelp(strInput[0],
                          MSDK_STRING("Number of active reference for P frames is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-num_active_BL0"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nNumRefActiveBL0)) {
                PrintHelp(strInput[0],
                          MSDK_STRING("Number of active reference for B frames (L0) is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-num_active_BL1"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nNumRefActiveBL1)) {
                PrintHelp(strInput[0],
                          MSDK_STRING("Number of active reference for B frames (L1) is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-extbrc:on"))) {
            pParams->nExtBRC = EXTBRC_ON;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-extbrc:off"))) {
            pParams->nExtBRC = EXTBRC_OFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-extbrc:implicit"))) {
            pParams->nExtBRC = EXTBRC_IMPLICIT;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ExtBrcAdaptiveLTR:on"))) {
            pParams->ExtBrcAdaptiveLTR = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ExtBrcAdaptiveLTR:off"))) {
            pParams->ExtBrcAdaptiveLTR = MFX_CODINGOPTION_OFF;
            ;
        }
#if (MFX_VERSION >= MFX_VERSION_NEXT)
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dblk_alpha"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->DeblockingAlphaTcOffset)) {
                PrintHelp(strInput[0], MSDK_STRING("Alpha deblocking parameter is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dblk_beta"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->DeblockingBetaOffset)) {
                PrintHelp(strInput[0], MSDK_STRING("Beta deblocking parameter is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
#endif
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-pp"))) {
            pParams->shouldPrintPresets = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-preset"))) {
            msdk_char presetName[MSDK_MAX_FILENAME_LEN];
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], presetName)) {
                PrintHelp(strInput[0], MSDK_STRING("Preset Name is not defined"));
                return MFX_ERR_UNSUPPORTED;
            }

            pParams->PresetMode = CPresetManager::PresetNameToMode(presetName);
            if (pParams->PresetMode == PRESET_MAX_MODES) {
                PrintHelp(strInput[0], MSDK_STRING("Preset Name is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-amfs:on"))) {
            pParams->nAdaptiveMaxFrameSize = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-amfs:off"))) {
            pParams->nAdaptiveMaxFrameSize = MFX_CODINGOPTION_OFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qsv-ff"))) {
            pParams->enableQSVFF = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-lowpower:on"))) {
            pParams->enableQSVFF = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-lowpower:off"))) {
            pParams->enableQSVFF = false;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-robust:soft"))) {
            pParams->bSoftRobustFlag = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-num_slice"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nNumSlice)) {
                PrintHelp(strInput[0], MSDK_STRING("Number of slices is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-n"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nNumFrames)) {
                PrintHelp(strInput[0], MSDK_STRING("Number of frames is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-w"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nWidth)) {
                PrintHelp(strInput[0], MSDK_STRING("Width is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-h"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nHeight)) {
                PrintHelp(strInput[0], MSDK_STRING("Height of frames is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-b"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nBitRate)) {
                PrintHelp(strInput[0], MSDK_STRING("Bitrate is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-f"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->dFrameRate)) {
                PrintHelp(strInput[0], MSDK_STRING("Framerate is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-x"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nNumRefFrame)) {
                PrintHelp(strInput[0], MSDK_STRING("NumRefFrames is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-g"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nGopPicSize)) {
                PrintHelp(strInput[0], MSDK_STRING("GopSize is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-r"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nGopRefDist)) {
                PrintHelp(strInput[0], MSDK_STRING("GopRefDist is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-i"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            pParams->InputFiles.push_back(strInput[++i]);
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-o"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            pParams->dstFileBuff.push_back(strInput[++i]);
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-p"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            pParams->pluginParams = ParsePluginGuid(strInput[++i]);
            if (AreGuidsEqual(pParams->pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL)) {
                msdk_printf(MSDK_STRING("error:  invalid encoder guid\n"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-q"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nQuality)) {
                PrintHelp(strInput[0], MSDK_STRING("Quality is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-?"))) {
            PrintHelp(strInput[0], NULL);
            return MFX_ERR_UNSUPPORTED;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-path"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            i++;
            pParams->pluginParams = ParsePluginPath(strInput[i]);
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-re"))) {
            pParams->UseRegionEncode = true;
        }
#if defined(_WIN64) || defined(_WIN32)
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-PartialOutput"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->PartialOutputMode)) {
                PrintHelp(strInput[0], MSDK_STRING("PartialOuput mode is not specified"));
                return MFX_ERR_UNSUPPORTED;
            }
            pParams->PartialOutputMode++; //need to increment it because zero means parameter unset

            if (pParams->PartialOutputMode == MFX_PARTIAL_BITSTREAM_BLOCK) {
                VAL_CHECK(i + 1 >= nArgNum, i - 1, strInput[i - 1]);
                if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->PartialOutputBlockSize)) {
                    PrintHelp(
                        strInput[0],
                        MSDK_STRING(
                            "Block size for MFX_PARTIAL_BITSTREAM_BLOCK mode is not specified"));
                    return MFX_ERR_UNSUPPORTED;
                }
            }
        }
#endif
#ifdef MOD_ENC
        MOD_ENC_PARSE_INPUT
#endif
#if defined(ENABLE_V4L2_SUPPORT)
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-d"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->DeviceName)) {
                PrintHelp(strInput[0], MSDK_STRING("Device name is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-uyvy"))) {
            pParams->v4l2Format = UYVY;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-p"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->MipiPort)) {
                PrintHelp(strInput[0], MSDK_STRING("Mipi-port is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-m"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->MipiModeName)) {
                PrintHelp(strInput[0], MSDK_STRING("Device name is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }

            if (strcasecmp(pParams->MipiModeName, "STILL") == 0)
                pParams->MipiMode = STILL;
            else if (strcasecmp(pParams->MipiModeName, "VIDEO") == 0)
                pParams->MipiMode = VIDEO;
            else if (strcasecmp(pParams->MipiModeName, "PREVIEW") == 0)
                pParams->MipiMode = PREVIEW;
            else if (strcasecmp(pParams->MipiModeName, "CONTINUOUS") == 0)
                pParams->MipiMode = CONTINUOUS;
            else
                pParams->MipiMode = NONE;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-i::v4l2"))) {
            pParams->isV4L2InputEnabled = true;
        }
#endif
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-u"))) {
            VAL_CHECK(i + 1 >= nArgNum, i, strInput[i]);
            pParams->nTargetUsage = StrToTargetUsage(strInput[++i]);
            if (!pParams->nTargetUsage) {
                msdk_printf(MSDK_STRING("error: wrong '-u' parameter. Balanced will be used.\n"));
                pParams->nTargetUsage = MFX_TARGETUSAGE_BALANCED;
            }
        }
        else {
            mfxStatus sts = ParseAdditionalParams(strInput, nArgNum, i, pParams);
            if (sts < MFX_ERR_NONE) {
                msdk_printf(MSDK_STRING("Unknown option: %s\n"), strInput[i]);
                PrintHelp(strInput[0], NULL);
                return MFX_ERR_UNSUPPORTED;
            }
        }
    }

    if (pParams->etAdaptiveI || pParams->etAdaptiveB || pParams->etArefP || pParams->etArefB ||
        pParams->etSceneChange || pParams->etALTR || pParams->etApyrQB || pParams->etApyrQP ||
        pParams->etBRCHints || pParams->etBRC) {
        pParams->bEncTools = true;
    }

#if defined(ENABLE_V4L2_SUPPORT)
    if (pParams->isV4L2InputEnabled) {
        if (0 == msdk_strlen(pParams->DeviceName)) {
            PrintHelp(strInput[0], MSDK_STRING("Device Name not found"));
            return MFX_ERR_UNSUPPORTED;
        }

        if ((pParams->MipiPort > -1 && pParams->MipiMode == NONE) ||
            (pParams->MipiPort < 0 && pParams->MipiMode != NONE)) {
            PrintHelp(strInput[0], MSDK_STRING("Invalid Mipi Configuration\n"));
            return MFX_ERR_UNSUPPORTED;
        }

        if (pParams->v4l2Format == NO_FORMAT) {
            PrintHelp(strInput[0], MSDK_STRING("NO input v4l2 format\n"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
#endif

    // check if all mandatory parameters were set
    if (!pParams->InputFiles.size() && !pParams->isV4L2InputEnabled) {
        PrintHelp(strInput[0], MSDK_STRING("Source file name not found"));
        return MFX_ERR_UNSUPPORTED;
    };

    if (0 == pParams->nWidth || 0 == pParams->nHeight) {
        PrintHelp(strInput[0], MSDK_STRING("-w, -h must be specified"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_CODEC_MPEG2 != pParams->CodecId && MFX_CODEC_AVC != pParams->CodecId &&
        MFX_CODEC_JPEG != pParams->CodecId && MFX_CODEC_HEVC != pParams->CodecId &&
        MFX_CODEC_VP9 != pParams->CodecId && MFX_CODEC_AV1 != pParams->CodecId) {
        PrintHelp(strInput[0], MSDK_STRING("Unknown codec"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (!pParams->EncodeFourCC) {
        if (pParams->FileInputFourCC == MFX_FOURCC_UYVY)
            // use YUY2 to get chroma subsampling 4:2:2 in encoded image
            pParams->EncodeFourCC = MFX_FOURCC_YUY2;
        else if (pParams->FileInputFourCC == MFX_FOURCC_I420 && pParams->bUseHWLib)
            pParams->EncodeFourCC = MFX_FOURCC_NV12;
        else
            pParams->EncodeFourCC = pParams->FileInputFourCC;
    }

    if (MFX_CODEC_JPEG != pParams->CodecId && MFX_CODEC_HEVC != pParams->CodecId &&
        pParams->FileInputFourCC == MFX_FOURCC_YUY2 && !pParams->isV4L2InputEnabled) {
        PrintHelp(strInput[0],
                  MSDK_STRING("-yuy2 option is supported only for JPEG or HEVC encoder"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_CODEC_HEVC != pParams->CodecId && MFX_CODEC_VP9 != pParams->CodecId &&
        MFX_CODEC_AV1 != pParams->CodecId && (pParams->EncodeFourCC == MFX_FOURCC_P010)) {
        PrintHelp(strInput[0],
                  MSDK_STRING("P010 surfaces are supported only for HEVC, VP9 and AV1 encoder"));
        return MFX_ERR_UNSUPPORTED;
    }

    // check parameters validity
    if (pParams->nRotationAngle != 0 && pParams->nRotationAngle != 180) {
        PrintHelp(strInput[0], MSDK_STRING("Angles other than 180 degrees are not supported."));
        return MFX_ERR_UNSUPPORTED; // other than 180 are not supported
    }

    if (pParams->nQuality && (MFX_CODEC_JPEG != pParams->CodecId)) {
        PrintHelp(strInput[0], MSDK_STRING("-q option is supported only for JPEG encoder"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->nTargetUsage || pParams->nBitRate) && (MFX_CODEC_JPEG == pParams->CodecId)) {
        PrintHelp(
            strInput[0],
            MSDK_STRING(
                "-u and -b options are supported only for H.264, MPEG2 and MVC encoders. For JPEG encoder use -q"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (!pParams->nQuality && (MFX_CODEC_JPEG == pParams->CodecId)) {
        PrintHelp(strInput[0], MSDK_STRING("-q must be specified for JPEG encoder"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_TRANSFERMATRIX_UNKNOWN != pParams->TransferMatrix &&
        MFX_TRANSFERMATRIX_BT601 != pParams->TransferMatrix &&
        MFX_TRANSFERMATRIX_BT709 != pParams->TransferMatrix) {
        PrintHelp(strInput[0], MSDK_STRING("Invalid transfer matrix type"));
        return MFX_ERR_UNSUPPORTED;
    }

    // set default values for optional parameters that were not set or were set incorrectly
    mfxU32 nviews = (mfxU32)pParams->InputFiles.size();
    if ((nviews <= 1) || (nviews > 2)) {
        if (!(MVC_ENABLED & pParams->MVC_flags)) {
            pParams->numViews = 1;
        }
        else {
            PrintHelp(strInput[0],
                      MSDK_STRING("Only 2 views are supported right now in this sample."));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else {
        pParams->numViews = nviews;
    }

    if (pParams->dFrameRate <= 0) {
        pParams->dFrameRate = 30;
    }

    // if no destination picture width or height wasn't specified set it to the source picture size
    if (pParams->nDstWidth == 0) {
        pParams->nDstWidth = pParams->nWidth;
    }

    if (pParams->nDstHeight == 0) {
        pParams->nDstHeight = pParams->nHeight;
    }

    if (!pParams->nPicStruct) {
        pParams->nPicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    }

    if ((pParams->nRateControlMethod == MFX_RATECONTROL_LA) && (!pParams->bUseHWLib)) {
        PrintHelp(strInput[0], MSDK_STRING("Look ahead BRC is supported only with -hw option!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (pParams->nRateControlMethod == MFX_RATECONTROL_AVBR) {
        if (pParams->Accuracy > 100) {
            msdk_printf(MSDK_STRING(
                "For AVBR BRC, the assigned accuracy exceeds 100, now set it to 100\n"));
            pParams->Accuracy = 100;
        }
    }

    if ((pParams->nMaxSliceSize) && (!pParams->bUseHWLib)) {
        PrintHelp(strInput[0],
                  MSDK_STRING("MaxSliceSize option is supported only with -hw option!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->nMaxSliceSize) && (pParams->nNumSlice)) {
        PrintHelp(strInput[0], MSDK_STRING("-mss and -num_slice options are not compatible!"));
        return MFX_ERR_UNSUPPORTED;
    }
    if ((pParams->nMaxSliceSize) && (pParams->CodecId != MFX_CODEC_AVC) &&
        (pParams->CodecId != MFX_CODEC_HEVC)) {
        PrintHelp(strInput[0],
                  MSDK_STRING(
                      "MaxSliceSize option is supported only with H.264 and H.265(HEVC) encoder!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->nRateControlMethod == MFX_RATECONTROL_LA ||
         pParams->nRateControlMethod == MFX_RATECONTROL_LA_ICQ ||
         pParams->nRateControlMethod == MFX_RATECONTROL_LA_HRD) &&
        (pParams->nLADepth > 100)) {
        PrintHelp(
            strInput[0],
            MSDK_STRING(
                "Unsupported value of -lad parameter, must be in range [1,100] or 0 for automatic selection"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (pParams->nNumRefActiveP || pParams->nNumRefActiveBL0 || pParams->nNumRefActiveBL1) {
        if (pParams->CodecId != MFX_CODEC_HEVC) {
            PrintHelp(strInput[0],
                      MSDK_STRING("NumRefActiveP/BL0/BL1 are supported only with HEVC encoder"));
            return MFX_ERR_UNSUPPORTED;
        }
    }

    // not all options are supported if rotate plugin is enabled
    if (pParams->nRotationAngle == 180 &&
        (MFX_PICSTRUCT_PROGRESSIVE != pParams->nPicStruct ||
         pParams->nDstWidth != pParams->nWidth || pParams->nDstHeight != pParams->nHeight ||
         MVC_ENABLED & pParams->MVC_flags || pParams->nRateControlMethod == MFX_RATECONTROL_LA)) {
        PrintHelp(strInput[0],
                  MSDK_STRING(
                      "Some of the command line options are not supported with rotation plugin!"));
        return MFX_ERR_UNSUPPORTED;
    }

    // Ignoring user-defined Async Depth for LA
    if (pParams->nAsyncDepth == 0 || pParams->nMaxSliceSize) {
        pParams->nAsyncDepth = 4;
    }

    if (pParams->UseRegionEncode) {
        if (pParams->CodecId != MFX_CODEC_HEVC) {
            msdk_printf(MSDK_STRING(
                "Region encode option is compatible with h265(HEVC) encoder only.\nRegion encoding is disabled\n"));
            pParams->UseRegionEncode = false;
        }
        if (pParams->nWidth != pParams->nDstWidth || pParams->nHeight != pParams->nDstHeight ||
            pParams->nRotationAngle != 0) {
            msdk_printf(MSDK_STRING(
                "Region encode option is not compatible with VPP processing and rotation plugin.\nRegion encoding is disabled\n"));
            pParams->UseRegionEncode = false;
        }
    }

    if (pParams->dstFileBuff.size() == 0) {
        msdk_printf(MSDK_STRING("File output is disabled as -o option isn't specified\n"));
    }

    if (pParams->PartialOutputMode) {
        pParams->nAsyncDepth = 1;
        msdk_printf(MSDK_STRING("Warning: async depth changed to 1 for partial output"));
    }

#if (defined(_WIN64) || defined(_WIN32)) && (MFX_VERSION >= 1031)
    if (pParams->bPreferdGfx && pParams->bPreferiGfx) {
        msdk_printf(MSDK_STRING("Warning: both dGfx and iGfx flags set. iGfx will be preferred"));
        pParams->bPreferdGfx = false;
    }
#endif

    mfxU16 mfxU16Limit = std::numeric_limits<mfxU16>::max();
    if (pParams->MaxKbps > mfxU16Limit || pParams->nBitRate > mfxU16Limit ||
        pParams->InitialDelayInKB > mfxU16Limit || pParams->BufferSizeInKB > mfxU16Limit) {
        mfxU32 maxVal               = std::max<mfxU32>({ pParams->MaxKbps,
                                                         pParams->nBitRate,
                                                         pParams->InitialDelayInKB,
                                                         pParams->BufferSizeInKB });
        pParams->nBitRateMultiplier = (mfxU16)std::ceil(static_cast<double>(maxVal) / mfxU16Limit);
        msdk_printf(MSDK_STRING("WARNING: BitRateMultiplier(-bm) was updated, new value - %d. \n"),
                    pParams->nBitRateMultiplier);

        auto recalculate = [mfxU16Limit, pParams](mfxU32& param, std::string paramName) {
            if (param) {
                if (param > mfxU16Limit) {
                    msdk_printf(MSDK_STRING("WARNING: %s (%d) > allow limit (%d). \n"),
                                paramName.c_str(),
                                param,
                                mfxU16Limit);
                }
                param = param / pParams->nBitRateMultiplier;
                msdk_printf(MSDK_STRING("WARNING: %s was updated, new value: %d. \n"),
                            paramName.c_str(),
                            param);
            }
        };

        recalculate(pParams->MaxKbps, "MaxKbps");
        recalculate(pParams->nBitRate, "nBitRate(-b)");
        recalculate(pParams->InitialDelayInKB, "InitialDelayInKB");
        recalculate(pParams->BufferSizeInKB, "BufferSizeInKB");
    }

    if (pParams->bUseHWLib && pParams->bReadByFrame) {
        PrintHelp(strInput[0], MSDK_STRING("-rbf (Read by frame) is not supported in hw lib"));
        return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_NONE;
}

void ModifyParamsUsingPresets(sInputParams& params) {
    if (!params.bUseHWLib)
        return;

    COutputPresetParameters presetParams = CPresetManager::Inst.GetPreset(params.PresetMode,
                                                                          params.CodecId,
                                                                          params.dFrameRate,
                                                                          params.nWidth,
                                                                          params.nHeight,
                                                                          params.bUseHWLib);

    if (params.shouldPrintPresets) {
        msdk_printf(MSDK_STRING("Preset-controlled parameters (%s):\n"),
                    presetParams.PresetName.c_str());
    }
    MODIFY_AND_PRINT_PARAM(params.nAdaptiveMaxFrameSize,
                           AdaptiveMaxFrameSize,
                           params.shouldPrintPresets);
    MODIFY_AND_PRINT_PARAM(params.nAsyncDepth, AsyncDepth, params.shouldPrintPresets);
    MODIFY_AND_PRINT_PARAM(params.nBRefType, BRefType, params.shouldPrintPresets);
    MODIFY_AND_PRINT_PARAM(params.bEncTools, EncTools, params.shouldPrintPresets);
    //    MODIFY_AND_PRINT_PARAM(params., EnableBPyramid);
    //    MODIFY_AND_PRINT_PARAM(params., EnablePPyramid);
    MODIFY_AND_PRINT_PARAM(params.nGopPicSize, GopPicSize, params.shouldPrintPresets);
    MODIFY_AND_PRINT_PARAM(params.nGopRefDist, GopRefDist, params.shouldPrintPresets);
    MODIFY_AND_PRINT_PARAM(params.IntRefCycleDist, IntRefCycleDist, params.shouldPrintPresets);
    MODIFY_AND_PRINT_PARAM(params.IntRefCycleSize, IntRefCycleSize, params.shouldPrintPresets);
    MODIFY_AND_PRINT_PARAM(params.IntRefQPDelta, IntRefQPDelta, params.shouldPrintPresets);
    MODIFY_AND_PRINT_PARAM(params.IntRefType, IntRefType, params.shouldPrintPresets);
    MODIFY_AND_PRINT_PARAM(params.nLADepth, LookAheadDepth, params.shouldPrintPresets);
    MODIFY_AND_PRINT_PARAM(params.LowDelayBRC, LowDelayBRC, params.shouldPrintPresets);
    MODIFY_AND_PRINT_PARAM(params.nMaxFrameSize, MaxFrameSize, params.shouldPrintPresets);
    if (!params.nRateControlMethod) {
        // Use preset ExtBRC parameter only if rate control mode was not set manually
        MODIFY_AND_PRINT_PARAM_EXT(params.nExtBRC,
                                   ExtBRCUsage,
                                   (ExtBRCType)presetParams.ExtBRCUsage,
                                   params.shouldPrintPresets);
    }
    MODIFY_AND_PRINT_PARAM(params.nRateControlMethod, RateControlMethod, params.shouldPrintPresets);

    if (params.nRateControlMethod != MFX_RATECONTROL_CQP) {
        MODIFY_AND_PRINT_PARAM(params.nBitRate, TargetKbps, params.shouldPrintPresets);
        MODIFY_AND_PRINT_PARAM(params.MaxKbps, MaxKbps, params.shouldPrintPresets);
        presetParams.BufferSizeInKB =
            params.nBitRate /
            8; // Update bitrate to reflect manually set bitrate. BufferSize should be enough for 1 second of video
        MODIFY_AND_PRINT_PARAM(params.BufferSizeInKB, BufferSizeInKB, params.shouldPrintPresets);
    }

    MODIFY_AND_PRINT_PARAM(params.nTargetUsage, TargetUsage, params.shouldPrintPresets);
    MODIFY_AND_PRINT_PARAM(params.WeightedBiPred, WeightedBiPred, params.shouldPrintPresets);
    MODIFY_AND_PRINT_PARAM(params.WeightedPred, WeightedPred, params.shouldPrintPresets);
    if (params.shouldPrintPresets) {
        msdk_printf(MSDK_STRING("\n"));
    }
}

CEncodingPipeline* CreatePipeline(const sInputParams& params) {
#ifdef MOD_ENC
    MOD_ENC_CREATE_PIPELINE;
#endif
    if (params.UseRegionEncode) {
        return new CRegionEncodingPipeline;
    }
    else if (params.nRotationAngle) {
        return new CUserPipeline;
    }
    else {
        return new CEncodingPipeline;
    }
}

#if defined(_WIN32) || defined(_WIN64)
int _tmain(int argc, msdk_char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    sInputParams Params = {}; // input parameters from command line
    std::unique_ptr<CEncodingPipeline> pPipeline;

    mfxStatus sts = MFX_ERR_NONE; // return value check

    // Parsing Input stream workign with presets
    sts = ParseInputString(argv, (mfxU8)argc, &Params);

    ModifyParamsUsingPresets(Params);

    MSDK_CHECK_PARSE_RESULT(sts, MFX_ERR_NONE, 1);

    // Choosing which pipeline to use
    pPipeline.reset(CreatePipeline(Params));

    MSDK_CHECK_POINTER(pPipeline.get(), MFX_ERR_MEMORY_ALLOC);

    if (MVC_ENABLED & Params.MVC_flags) {
        pPipeline->SetNumView(Params.numViews);
    }

    sts = pPipeline->Init(&Params);
    MSDK_CHECK_STATUS(sts, "pPipeline->Init failed");

    pPipeline->PrintInfo();

    msdk_printf(MSDK_STRING("Processing started\n"));

    if (pPipeline->CaptureStartV4L2Pipeline() != MFX_ERR_NONE) {
        msdk_printf(MSDK_STRING("V4l2 failure terminating the program\n"));
        return 0;
    }

    for (;;) {
        sts = pPipeline->Run();

        if (MFX_ERR_DEVICE_LOST == sts || MFX_ERR_DEVICE_FAILED == sts) {
            msdk_printf(MSDK_STRING(
                "\nERROR: Hardware device was lost or returned an unexpected error. Recovering...\n"));
            sts = pPipeline->ResetDevice();
            MSDK_CHECK_STATUS(sts, "pPipeline->ResetDevice failed");

            sts = pPipeline->ResetMFXComponents(&Params);
            MSDK_CHECK_STATUS(sts, "pPipeline->ResetMFXComponents failed");
            continue;
        }
        else {
            MSDK_CHECK_STATUS(sts, "pPipeline->Run failed");
            break;
        }
    }

    pPipeline->CaptureStopV4L2Pipeline();

    pPipeline->Close();

    msdk_printf(MSDK_STRING("\nProcessing finished\n"));

    return 0;
}
