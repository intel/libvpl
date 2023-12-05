/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include "sample_utils.h"
#include "sample_vpp_utils.h"
#include "version.h"

#define VAL_CHECK(val)              \
    {                               \
        if (val)                    \
            return MFX_ERR_UNKNOWN; \
    }

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

// Intel® Video Processing Library (Intel® VPL)

void vppPrintHelp(const char* strAppName, const char* strErrorMessage) {
    printf("Intel(R) Media SDK VPP Sample version %s\n", GetMSDKSampleVersion().c_str());
    if (strErrorMessage) {
        printf("Error: %s\n", strErrorMessage);
    }

    printf("Usage: %s [Options] -i InputFile -o OutputFile\n", strAppName);

    printf("Options: \n");
    printf("   [-lib  type]                - type of used library. sw, hw (def: sw)\n\n");
#if defined(LINUX32) || defined(LINUX64)
    printf("   [-device /path/to/device]   - set graphics device for processing\n");
    printf("                                  For example: '-device /dev/dri/card0'\n");
    printf("                                               '-device /dev/dri/renderD128'\n");
    printf(
        "                                  If not specified, defaults to the first Intel device found on the system\n\n");
#endif
#if (defined(_WIN64) || defined(_WIN32))
    printf("   [-luid HighPart:LowPart] - setup adapter by LUID  \n");
    printf("                                 For example: \"0x0:0x8a46\"  \n");
#endif
    printf("   [-pci domain:bus:device.function] - setup device with PCI \n");
    printf("                                 For example: \"0:3:0.0\"  \n");
    printf(
        "   [-dGfx]                     - prefer processing on dGfx (by default system decides)\n");
    printf(
        "   [-iGfx]                     - prefer processing on iGfx (by default system decides)\n");
    printf(
        "   [-AdapterNum]               - specifies adapter number for processing, starts from 0\n");
    printf(
        "   [-dispatcher:fullSearch]    - enable search for all available implementations in Intel® VPL dispatcher\n");
    printf(
        "   [-dispatcher:lowLatency]    - enable limited implementation search and query in Intel® VPL dispatcher\n");
#if defined(D3D_SURFACES_SUPPORT)
    printf("   [-d3d]                      - use d3d9 surfaces\n\n");
#endif
#if MFX_D3D11_SUPPORT
    printf("   [-d3d11]                    - use d3d11 surfaces\n\n");
#endif
#ifdef LIBVA_SUPPORT
    printf("   [-vaapi]                    - work with vaapi surfaces\n\n");
#endif

    printf("   [-sw   width]               - width  of src video (def: 352)\n");
    printf("   [-sh   height]              - height of src video (def: 288)\n");
    printf("   [-scrX  x]                  - cropX  of src video (def: 0)\n");
    printf("   [-scrY  y]                  - cropY  of src video (def: 0)\n");
    printf("   [-scrW  w]                  - cropW  of src video (def: width)\n");
    printf("   [-scrH  h]                  - cropH  of src video (def: height)\n");
    printf("   [-sf   frameRate]           - frame rate of src video (def: 30.0)\n");
    printf(
        "   [-scc  format]              - format (FourCC) of src video (def: nv12. support i420|nv12|yv12|yuy2|rgb565|rgb3|rgb4|imc3|yuv400|yuv411|yuv422h|yuv422v|yuv444|uyvy|ayuv|i010|p010|y210|y410|p016|y216|y416)\n");
    printf(
        "   [-sbitshift 0|1]            - shift data to right or keep it the same way as in Microsoft's P010\n");
    printf(
        "   [-sbitdepthluma value]      - shift luma channel to right to \"16 - value\" bytes\n");
    printf(
        "   [-sbitdepthchroma value]    - shift chroma channel to right to \"16 - value\" bytes\n");

    printf("   [-spic value]               - picture structure of src video\n");
    printf("                                 0 - interlaced top    field first\n");
    printf("                                 2 - interlaced bottom field first\n");
    printf("                                 3 - single field\n");
    printf("                                 1 - progressive (default)\n");
    printf("                                -1 - unknown\n\n");

    printf("   [-dw  width]                - width  of dst video (def: 352)\n");
    printf("   [-dh  height]               - height of dst video (def: 288)\n");
    printf("   [-dcrX  x]                  - cropX  of dst video (def: 0)\n");
    printf("   [-dcrY  y]                  - cropY  of dst video (def: 0)\n");
    printf("   [-dcrW  w]                  - cropW  of dst video (def: width)\n");
    printf("   [-dcrH  h]                  - cropH  of dst video (def: height)\n");
    printf("   [-df  frameRate]            - frame rate of dst video (def: 30.0)\n");
    printf(
        "   [-dcc format]               - format (FourCC) of dst video (def: nv12. support i420|nv12|yuy2|rgb4|rgbp|yv12|ayuv|a2rgb10|y210|y410|p016|y216|y416)\n");
    printf(
        "   [-dbitshift 0|1]            - shift data to right or keep it the same way as in Microsoft's P010\n");
    printf("   [-dbitdepthluma value]      - shift luma channel to left to \"16 - value\" bytes\n");
    printf(
        "   [-dbitdepthchroma value]    - shift chroma channel to left to \"16 - value\" bytes\n");

    printf("   [-dpic value]               - picture structure of dst video\n");
    printf("                                 0 - interlaced top    field first\n");
    printf("                                 2 - interlaced bottom field first\n");
    printf("                                 3 - single field\n");
    printf("                                 1 - progressive (default)\n");
    printf("                                -1 - unknown\n\n");

    printf("   Video Composition\n");
    printf(
        "   [-composite parameters_file] - composite several input files in one output. The syntax of the parameters file is:\n");
    printf("                                  stream=<video file name>\n");
    printf("                                  width=<input video width>\n");
    printf("                                  height=<input video height>\n");
    printf("                                  cropx=<input cropX (def: 0)>\n");
    printf("                                  cropy=<input cropY (def: 0)>\n");
    printf("                                  cropw=<input cropW (def: width)>\n");
    printf("                                  croph=<input cropH (def: height)>\n");
    printf("                                  framerate=<input frame rate (def: 30.0)>\n");
    printf(
        "                                  fourcc=<format (FourCC) of input video (def: nv12. support nv12|rgb4)>\n");
    printf("                                  picstruct=<picture structure of input video,\n");
    printf("                                             0 = interlaced top    field first\n");
    printf("                                             2 = interlaced bottom field first\n");
    printf("                                             3 = single field\n");
    printf("                                             1 = progressive (default)>\n");
    printf(
        "                                  dstx=<X coordinate of input video located in the output (def: 0)>\n");
    printf(
        "                                  dsty=<Y coordinate of input video located in the output (def: 0)>\n");
    printf(
        "                                  dstw=<width of input video located in the output (def: width)>\n");
    printf(
        "                                  dsth=<height of input video located in the output (def: height)>\n\n");
    printf("                                  stream=<video file name>\n");
    printf("                                  width=<input video width>\n");
    printf("                                  GlobalAlphaEnable=1\n");
    printf("                                  GlobalAlpha=<Alpha value>\n");
    printf("                                  LumaKeyEnable=1\n");
    printf("                                  LumaKeyMin=<Luma key min value>\n");
    printf("                                  LumaKeyMax=<Luma key max value>\n");
    printf("                                  ...\n");
    printf(
        "                                  The parameters file may contain up to 64 streams.\n\n");
    printf("                                  For example:\n");
    printf("                                  stream=input_720x480.yuv\n");
    printf("                                  width=720\n");
    printf("                                  height=480\n");
    printf("                                  cropx=0\n");
    printf("                                  cropy=0\n");
    printf("                                  cropw=720\n");
    printf("                                  croph=480\n");
    printf("                                  dstx=0\n");
    printf("                                  dsty=0\n");
    printf("                                  dstw=720\n");
    printf("                                  dsth=480\n\n");
    printf("                                  stream=input_480x320.yuv\n");
    printf("                                  width=480\n");
    printf("                                  height=320\n");
    printf("                                  cropx=0\n");
    printf("                                  cropy=0\n");
    printf("                                  cropw=480\n");
    printf("                                  croph=320\n");
    printf("                                  dstx=100\n");
    printf("                                  dsty=100\n");
    printf("                                  dstw=320\n");
    printf("                                  dsth=240\n");
    printf("                                  GlobalAlphaEnable=1\n");
    printf("                                  GlobalAlpha=128\n");
    printf("                                  LumaKeyEnable=1\n");
    printf("                                  LumaKeyMin=250\n");
    printf("                                  LumaKeyMax=255\n");
    printf("   [-cf_disable]                  disable colorfill\n");

    printf("   Video Enhancement Algorithms\n");

    printf("   [-di_mode (mode)] - set type of deinterlace algorithm\n");
    printf("                        12 - advanced with Scene Change Detection (SCD) \n");
    printf(
        "                        8 - reverse telecine for a selected telecine pattern (use -tc_pattern). For PTIR plug-in\n");
    printf("                        2 - advanced or motion adaptive (default)\n");
    printf("                        1 - simple or BOB\n\n");

    printf(
        "   [-deinterlace (type)] - enable deinterlace algorithm (alternative way: -spic 0 -dpic 1) \n");
    printf("                         type is tff (default) or bff \n");

    printf("   [-rotate (angle)]   - enable rotation. Supported angles: 0, 90, 180, 270.\n");

    printf("   [-scaling_mode (mode)] - specify type of scaling to be used for resize\n");
    printf("                            0 - default\n");
    printf("                            1 - low power mode\n");
    printf("                            2 - quality mode\n\n");

    printf(
        "   [-interpolation_method (method)] - specify interpolation method to be used for resize\n");
    printf("                                      0 - default\n");
    printf("                                      1 - nearest neighbor\n");
    printf("                                      2 - bilinear\n");
    printf("                                      3 - advanced\n\n");

    printf("   [-denoise (level) (mode)]  - enable denoise.\n");
    printf("           mode  - denoise mode\n");
    printf("               0    - default\n");
    printf("               1001 - auto BD rate\n");
    printf("               1002 - auto subjective\n");
    printf("               1003 - auto adjust\n");
    printf("               1004 - manual mode for pre-processing, need level\n");
    printf("               1005 - manual mode for post-processing, need level\n");
    printf("           level - range of noise level is [0, 100]\n");
    printf(
        "   [-chroma_siting (vmode hmode)] - specify chroma siting mode for VPP color conversion, allowed values: vtop|vcen|vbot hleft|hcen\n");
#ifdef ENABLE_MCTF
    printf("  -mctf [Strength]\n");
    printf("        Strength is an optional value;  it is in range [0...20]\n");
    printf("        value 0 makes MCTF operates in auto mode;\n");
    printf("        values [1...20] makes MCTF operates with fixed-strength mode;\n");
    printf("        In fixed-strength mode, MCTF strength can be adjusted at framelevel;\n");
    printf("        If no Strength is given, MCTF operates in auto mode.\n");
#endif //ENABLE_MCTF
    printf("   [-detail  (level)]  - enable detail enhancement algorithm. Level is optional \n");
    printf("                         range of detail level is [0, 100]\n\n");
    printf(
        "   [-pa_hue  hue]        - procamp hue property.         range [-180.0, 180.0] (def: 0.0)\n");
    printf(
        "   [-pa_sat  saturation] - procamp satursation property. range [   0.0,  10.0] (def: 1.0)\n");
    printf(
        "   [-pa_con  contrast]   - procamp contrast property.    range [   0.0,  10.0] (def: 1.0)\n");
    printf(
        "   [-pa_bri  brightness] - procamp brightness property.  range [-100.0, 100.0] (def: 0.0)\n\n");
#ifdef ENABLE_VPP_RUNTIME_HSBC
    printf(
        "   [-rt_hue  num_frames hue1 hue2] - enable per-frame hue adjustment in run-time without the whole video processing pipeline reinitialization.\n");
    printf(
        "             num_frames - a number of frames after which hue is changed either from hue1 to hue2 or from hue2 to hue1. \n");
    printf("                          The first num_frames frames are initialized to hue1.\n");
    printf("             hue1 - the first hue value in range [-180.0, 180.0] (def: 0.0)\n");
    printf("             hue2 - the second hue value in range [-180.0, 180.0] (def: 0.0)\n\n");
    printf(
        "   [-rt_sat  num_frames sat1 sat2] - enable per-frame saturation adjustment in run-time without the whole video processing pipeline reinitialization.\n");
    printf(
        "             num_frames - a number of frames after which saturation is changed either from sat1 to sat2 or from sat2 to sat1. \n");
    printf("                          The first num_frames frames are initialized to sat1.\n");
    printf("             sat1 - the first saturation value in range [0.0, 10.0] (def: 1.0)\n");
    printf("             sat2 - the second saturation value in range [0.0, 10.0] (def: 1.0)\n\n");
    printf(
        "   [-rt_con  num_frames con1 con2] - enable per-frame contrast adjustment in run-time without the whole video processing pipeline reinitialization.\n");
    printf(
        "             num_frames - a number of frames after which contrast is changed either from con1 to con2 or from con2 to con1. \n");
    printf("                          The first num_frames frames are initialized to con1.\n");
    printf("             con1 - the first contrast value in range [0.0, 10.0] (def: 1.0)\n");
    printf("             con2 - the second contrast value in range [0.0, 10.0] (def: 1.0)\n\n");
    printf(
        "   [-rt_bri  num_frames bri1 bri2] - enable per-frame brightness adjustment in run-time without the whole video processing pipeline reinitialization.\n");
    printf(
        "             num_frames - a number of frames after which brightness is changed either from bri1 to bri2 or from bri2 to bri1. \n");
    printf("                          The first num_frames frames are initialized to bri1.\n");
    printf("             bri1 - the first brightness value in range [-100.0, 100.0] (def: 0.0)\n");
    printf(
        "             bri2 - the second brightness value in range [-100.0, 100.0] (def: 0.0)\n\n");
#endif
    printf("   [-gamut:compression]  - enable gamut compression algorithm (xvYCC->sRGB) \n");
    printf(
        "   [-gamut:bt709]        - enable BT.709 matrix transform (RGB->YUV conversion)(def: BT.601)\n\n");
    printf("   [-frc:advanced]       - enable advanced FRC algorithm (based on PTS) \n");
    printf("   [-frc:interp]         - enable FRC based on frame interpolation algorithm\n\n");

    printf("   [-tcc:red]            - enable color saturation algorithm (R component) \n");
    printf("   [-tcc:green]          - enable color saturation algorithm (G component)\n");
    printf("   [-tcc:blue]           - enable color saturation algorithm (B component)\n");
    printf("   [-tcc:cyan]           - enable color saturation algorithm (C component)\n");
    printf("   [-tcc:magenta]        - enable color saturation algorithm (M component)\n");
    printf("   [-tcc:yellow]         - enable color saturation algorithm (Y component)\n\n");

    printf("   [-ace]                - enable auto contrast enhancement algorithm \n\n");
    printf(
        "   [-ste (level)]        - enable Skin Tone Enhancement algorithm.  Level is optional \n");
    printf("                           range of ste level is [0, 9] (def: 4)\n\n");
    printf("   [-istab (mode)]       - enable Image Stabilization algorithm.  Mode is optional \n");
    printf("                           mode of istab can be [1, 2] (def: 2)\n");
    printf("                           where: 1 means upscale mode, 2 means croppping mode\n");
    printf(
        "   [-view:count value]   - enable Multi View preprocessing. range of views [1, 1024] (def: 1)\n\n");
    printf("                           id-layerId, width/height-resolution \n\n");
    printf("   [-ssitm (id)]         - specify YUV<->RGB transfer matrix for input surface.\n");
    printf("   [-dsitm (id)]         - specify YUV<->RGB transfer matrix for output surface.\n");
    printf("   [-ssinr (id)]         - specify YUV nominal range for input surface.\n");
    printf("   [-dsinr (id)]         - specify YUV nominal range for output surface.\n\n");
    printf("   [-mirror (mode)]      - mirror image using specified mode.\n");

    printf("   [-n frames] - number of frames to VPP process\n\n");

    printf(
        "   [-iopattern IN/OUT surface type] -  IN/OUT surface type: sys_to_sys, sys_to_d3d, d3d_to_sys, d3d_to_d3d    (def: sys_to_sys)\n");
    printf("   [-async n] - maximum number of asynchronious tasks. def: -async 1 \n");
    printf(
        "   [-perf_opt n m] - n: number of prefetech frames. m : number of passes. In performance mode app preallocates bufer and load first n frames,  def: no performace 1 \n");
    printf("   [-pts_check] - checking of time stampls. Default is OFF \n");
    printf(
        "   [-pts_jump ] - checking of time stamps jumps. Jump for random value since 13-th frame. Also, you can change input frame rate (via pts). Default frame_rate = sf \n");
    printf("   [-pts_fr ]   - input frame rate which used for pts. Default frame_rate = sf \n");
    printf("   [-pts_advanced]   - enable FRC checking mode based on PTS \n");
    printf(
        "   [-pf file for performance data] -  file to save performance data. Default is off \n\n\n");

    printf("   [-roi_check mode seed1 seed2] - checking of ROI processing. Default is OFF \n");
    printf("               mode - usage model of cropping\n");
    printf("                      var_to_fix - variable input ROI and fixed output ROI\n");
    printf("                      fix_to_var - fixed input ROI and variable output ROI\n");
    printf("                      var_to_var - variable input ROI and variable output ROI\n");
    printf("               seed1 - seed for init of rand generator for src\n");
    printf("               seed2 - seed for init of rand generator for dst\n");
    printf("                       range of seed [1, 65535]. 0 reserved for random init\n\n");

    printf("   [-tc_pattern (pattern)] - set telecine pattern\n");
    printf(
        "                        4 - provide a position inside a sequence of 5 frames where the artifacts starts. Use to -tc_pos to provide position\n");
    printf("                        3 - 4:1 pattern\n");
    printf("                        2 - frame repeat pattern\n");
    printf("                        1 - 2:3:3:2 pattern\n");
    printf("                        0 - 3:2 pattern\n\n");

    printf(
        "   [-tc_pos (position)] - Position inside a telecine sequence of 5 frames where the artifacts starts - Value [0 - 4]\n\n");

    printf(
        "   [-reset_start (frame number)] - after reaching this frame, encoder will be reset with new parameters, followed after this command and before -reset_end \n");
    printf("   [-reset_end]                  - specifies end of reset related options \n");
    printf(
        "   [-api_ver_init::<1x,2x>]  - select the api version for the session initialization\n");
    printf("   [-rbf] - read frame-by-frame from the input (sw lib only)\n\n");

    printf("   [-3dlut] path to 3dlut table file\n");
    printf("   [-3dlutMemType] specify 3dlut memory type, 0: video, 1: sys. Default value is 0\n");
    printf("   [-3dlutMode] specify 3dlut mode for HDR 3Dlut, allowwed values:17|33|65\n");

    printf(
        "   [-SignalInfoIn fullrange colorprimary transfer_characteristic]    - set input video signal info\n");
    printf("             fullrange - 1 is full, 0 is limited\n");
    printf(
        "             colorprimary - BT709 value 1, BT2020 value 9, refer to video spec(e.g. HEVC Table E.3) for more\n");
    printf(
        "             transfer_characteristic - BT709 value 1, ST2084 value 16, refer to video spec(e.g. HEVC Table E.4) for more\n");
    printf(
        "   [-SignalInfoOut fullrange colorprimary transfer_characteristic]   - set output video signal info\n");
    printf("             fullrange - 1 is full, 0 is limited\n");
    printf(
        "             colorprimary - BT709 value 1, BT2020 value 9, refer to video spec(e.g. HEVC Table E.3) for more\n");
    printf(
        "             transfer_characteristic - BT709 value 1, ST2084 value 16, refer to video spec(e.g. HEVC Table E.4) for more\n");

#ifdef ONEVPL_EXPERIMENTAL
    printf("   [-cfg::vpp config]    - Set VPP options via string-api\n");
#endif
    printf(
        "   [-dump fileName]         - dump MSDK components configuration to the file in text form\n");
    printf("\n");

    printf(
        "Usage2: %s -sw 352 -sh 144 -scc rgb3 -dw 320 -dh 240 -dcc nv12 -denoise 32 -iopattern d3d_to_d3d -i in.rgb -o out.yuv -roi_check var_to_fix 7 7\n",
        strAppName);

    printf("\n");

} // void vppPrintHelp(char *strAppName, char *strErrorMessage)

mfxU16 GetPicStruct(mfxI8 picStruct) {
    if (0 == picStruct) {
        return MFX_PICSTRUCT_FIELD_TFF;
    }
    else if (2 == picStruct) {
        return MFX_PICSTRUCT_FIELD_BFF;
    }
    else if (3 == picStruct) {
        return MFX_PICSTRUCT_FIELD_SINGLE;
    }
    else if (-1 == picStruct) {
        return MFX_PICSTRUCT_UNKNOWN;
    }
    else {
        return MFX_PICSTRUCT_PROGRESSIVE;
    }

} // mfxU16 GetPicStruct( mfxI8 picStruct )

mfxU32 Str2FourCC(char* strInput) {
    mfxU32 fourcc = 0; //default

    if (msdk_match_i(strInput, "yv12")) {
        fourcc = MFX_FOURCC_YV12;
    }
    else if (msdk_match_i(strInput, "rgb565")) {
        fourcc = MFX_FOURCC_RGB565;
    }
    else if (msdk_match_i(strInput, "rgb3")) {
        fourcc = MFX_FOURCC_RGB3;
    }
    else if (msdk_match_i(strInput, "rgb4")) {
        fourcc = MFX_FOURCC_RGB4;
    }
#if !(defined(_WIN32) || defined(_WIN64))
    else if (msdk_match_i(strInput, "rgbp")) {
        fourcc = MFX_FOURCC_RGBP;
    }
#endif
    else if (msdk_match_i(strInput, "yuy2")) {
        fourcc = MFX_FOURCC_YUY2;
    }
    else if (msdk_match_i(strInput, "nv12")) {
        fourcc = MFX_FOURCC_NV12;
    }
    else if (msdk_match_i(strInput, "imc3")) {
        fourcc = MFX_FOURCC_IMC3;
    }
    else if (msdk_match_i(strInput, "yuv400")) {
        fourcc = MFX_FOURCC_YUV400;
    }
    else if (msdk_match_i(strInput, "yuv411")) {
        fourcc = MFX_FOURCC_YUV411;
    }
    else if (msdk_match_i(strInput, "yuv422h")) {
        fourcc = MFX_FOURCC_YUV422H;
    }
    else if (msdk_match_i(strInput, "yuv422v")) {
        fourcc = MFX_FOURCC_YUV422V;
    }
    else if (msdk_match_i(strInput, "yuv444")) {
        fourcc = MFX_FOURCC_YUV444;
    }
    else if (msdk_match_i(strInput, "i010")) {
        fourcc = MFX_FOURCC_I010;
    }
    else if (msdk_match_i(strInput, "p010")) {
        fourcc = MFX_FOURCC_P010;
    }
    else if (msdk_match_i(strInput, "p210")) {
        fourcc = MFX_FOURCC_P210;
    }
    else if (msdk_match_i(strInput, "nv16")) {
        fourcc = MFX_FOURCC_NV16;
    }
    else if (msdk_match_i(strInput, "a2rgb10")) {
        fourcc = MFX_FOURCC_A2RGB10;
    }
    else if (msdk_match_i(strInput, "uyvy")) {
        fourcc = MFX_FOURCC_UYVY;
    }
    else if (msdk_match_i(strInput, "y210")) {
        fourcc = MFX_FOURCC_Y210;
    }
    else if (msdk_match_i(strInput, "y410")) {
        fourcc = MFX_FOURCC_Y410;
    }
    else if (msdk_match_i(strInput, "p016")) {
        fourcc = MFX_FOURCC_P016;
    }
    else if (msdk_match_i(strInput, "y216")) {
        fourcc = MFX_FOURCC_Y216;
    }
    else if (msdk_match_i(strInput, "y416")) {
        fourcc = MFX_FOURCC_Y416;
    }
    else if (msdk_match_i(strInput, "i420")) {
        fourcc = MFX_FOURCC_I420;
    }
    else if (msdk_match_i(strInput, "ayuv")) {
        fourcc = MFX_FOURCC_AYUV;
    }

    return fourcc;

} // mfxU32 Str2FourCC( char* strInput )

eROIMode Str2ROIMode(char* strInput) {
    eROIMode mode;

    if (msdk_match(strInput, "var_to_fix")) {
        mode = ROI_VAR_TO_FIX;
    }
    else if (msdk_match(strInput, "var_to_var")) {
        mode = ROI_VAR_TO_VAR;
    }
    else if (msdk_match(strInput, "fix_to_var")) {
        mode = ROI_FIX_TO_VAR;
    }
    else {
        mode = ROI_FIX_TO_FIX; // default mode
    }

    return mode;

} // eROIMode Str2ROIMode( char* strInput )

static mfxU16 Str2IOpattern(char* strInput) {
    mfxU16 IOPattern = 0;

    if (msdk_match(strInput, "d3d_to_d3d")) {
        IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;
    }
    else if (msdk_match(strInput, "d3d_to_sys")) {
        IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    }
    else if (msdk_match(strInput, "sys_to_d3d")) {
        IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;
    }
    else if (msdk_match(strInput, "sys_to_sys")) {
        IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    }
    return IOPattern;

} // static mfxU16 Str2IOpattern( char* strInput )

#ifdef ENABLE_MCTF
// returns a pointer to start of argument with a number argn;
// if failes to find argn, returns NULL
char* ParseArgn(char* pIn, mfxU32 argn, char separator) {
    char* pstr = pIn;
    if (!argn)
        return pIn;
    else {
        for (mfxU32 n = 0; n != argn; ++n) {
            while (separator != *pstr && char('\0') != *pstr)
                ++pstr;
            if (char('\0') == *pstr)
                return NULL;
            else
                ++pstr;
        }
        return pstr;
    }
};

size_t split(const std::string& source, std::vector<std::string>& dest, char delim = char(' ')) {
    size_t items = 0;
    std::string item;
    std::stringstream source_stream(source);
    while (getline(source_stream, item, delim)) {
        items += 1;
        dest.push_back(item);
    }
    return items;
}

void ParseMCTFParamsByValue(const std::string& argument, sInputParams* pParams, mfxU32 paramID);

void ParseMCTFParams(char* strInput[],
                     mfxU32 nArgNum,
                     mfxU32& curArg,
                     sInputParams* pParams,
                     mfxU32 paramID) {
    if (!msdk_match(strInput[curArg], "-mctf")) {
        printf("MCTF options should start with -mcft.\n");
        return;
    }

    pParams->mctfParam[paramID].mode                  = VPP_FILTER_ENABLED_DEFAULT;
    pParams->mctfParam[paramID].params.FilterStrength = 0;

    if (curArg + 1 >= nArgNum) {
        printf("MCTF option should have at least one value. Enabling default settings\n");
        return;
    }
    curArg++;

    std::string argument = std::string(strInput[curArg]);

    if (argument.empty()) {
        printf("MCTF option should have at least one non-empty value. Enabling default settings\n");
        return;
    }

    ParseMCTFParamsByValue(argument, pParams, paramID);
}

void ParseMCTFParamsByValue(const std::string& argument, sInputParams* pParams, mfxU32 paramID) {
    std::vector<std::string> mctf_param_parts;
    std::stringstream mctf_param_stream(argument);
    std::string temp_str;
    split(argument, mctf_param_parts, char(':'));

    mfxU16 _strength = 0;
    try {
        _strength = std::stoi(mctf_param_parts[0]);
    }
    catch (const std::invalid_argument&) {
        printf("Error reading MCTF strength setting.\n");
    }
    catch (const std::out_of_range&) {
        printf("MCTF strength out of bounds.\n");
    }
    pParams->mctfParam[paramID].mode                  = VPP_FILTER_ENABLED_CONFIGURED;
    pParams->mctfParam[paramID].params.FilterStrength = _strength;
}
#endif

mfxStatus vppParseResetPar(char* strInput[],
                           mfxU32 nArgNum,
                           mfxU32& curArg,
                           sInputParams* pParams,
                           mfxU32 paramID,
                           sFiltersParam* pDefaultFiltersParam) {
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(strInput, MFX_ERR_NULL_PTR);

    sOwnFrameInfo info = pParams->frameInfoIn.back();
    pParams->frameInfoIn.push_back(info);
    info = pParams->frameInfoOut.back();
    pParams->frameInfoOut.push_back(info);

    pParams->deinterlaceParam.push_back(*pDefaultFiltersParam->pDIParam);
    pParams->denoiseParam.push_back(*pDefaultFiltersParam->pDenoiseParam);
#ifdef ENABLE_MCTF
    pParams->mctfParam.push_back(*pDefaultFiltersParam->pMctfParam);
#endif
    pParams->detailParam.push_back(*pDefaultFiltersParam->pDetailParam);
    pParams->procampParam.push_back(*pDefaultFiltersParam->pProcAmpParam);
    pParams->frcParam.push_back(*pDefaultFiltersParam->pFRCParam);
    pParams->multiViewParam.push_back(*pDefaultFiltersParam->pMultiViewParam);
    pParams->gamutParam.push_back(*pDefaultFiltersParam->pGamutParam);
    pParams->tccParam.push_back(*pDefaultFiltersParam->pClrSaturationParam);
    pParams->aceParam.push_back(*pDefaultFiltersParam->pContrastParam);
    pParams->steParam.push_back(*pDefaultFiltersParam->pSkinParam);
    pParams->istabParam.push_back(*pDefaultFiltersParam->pImgStabParam);
    pParams->videoSignalInfoParam.push_back(*pDefaultFiltersParam->pVideoSignalInfo);
    pParams->mirroringParam.push_back(*pDefaultFiltersParam->pMirroringParam);
    pParams->rotate.push_back(0);
    pParams->colorfillParam.push_back(*pDefaultFiltersParam->pColorfillParam);

    mfxU32 readData;

    for (mfxU32& i = curArg; i < nArgNum; i++) {
        MSDK_CHECK_POINTER(strInput[i], MFX_ERR_NULL_PTR);
        {
            if (msdk_match(strInput[i], "-o")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;

                pParams->strDstFiles.push_back(strInput[i]);
                pParams->isOutput = true;
            }
            //-----------------------------------------------------------------------------------
            //                   Video Enhancement Algorithms
            //-----------------------------------------------------------------------------------
            else if (msdk_match(strInput[i], "-ssinr")) {
                VAL_CHECK(1 + i == nArgNum);

                pParams->videoSignalInfoParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;

                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoParam[paramID].In.NominalRange);
            }
            else if (msdk_match(strInput[i], "-dsinr")) {
                VAL_CHECK(1 + i == nArgNum);

                pParams->videoSignalInfoParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;

                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoParam[paramID].Out.NominalRange);
            }
            else if (msdk_match(strInput[i], "-ssitm")) {
                VAL_CHECK(1 + i == nArgNum);

                pParams->videoSignalInfoParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;

                i++;
                msdk_opt_read(strInput[i],
                              pParams->videoSignalInfoParam[paramID].In.TransferMatrix);
            }
            else if (msdk_match(strInput[i], "-dsitm")) {
                VAL_CHECK(1 + i == nArgNum);

                pParams->videoSignalInfoParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;

                i++;
                msdk_opt_read(strInput[i],
                              pParams->videoSignalInfoParam[paramID].Out.TransferMatrix);
            }
            else if (msdk_match(strInput[i], "-mirror")) {
                VAL_CHECK(1 + i == nArgNum);

                pParams->mirroringParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;

                i++;
                msdk_opt_read(strInput[i], pParams->mirroringParam[paramID].Type);
            }
            else if (msdk_match(strInput[i], "-sw")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoIn.back().nWidth);
            }
            else if (msdk_match_i(strInput[i], "-dw")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoOut.back().nWidth);
            }
            else if (msdk_match(strInput[i], "-sh")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoIn.back().nHeight);
            }
            else if (msdk_match_i(strInput[i], "-dh")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoOut.back().nHeight);
            }
            else if (msdk_match(strInput[i], "-denoise")) {
                pParams->denoiseParam[paramID].mode = VPP_FILTER_ENABLED_DEFAULT;

                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->denoiseParam[paramID].factor = (mfxU16)readData;
                        pParams->denoiseParam[paramID].mode   = VPP_FILTER_ENABLED_CONFIGURED;
                        i++;
                    }
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->denoiseParam[paramID].config = (mfxU16)readData;
                        pParams->denoiseParam[paramID].mode   = VPP_FILTER_ENABLED_CONFIGURED;
                        i++;
                    }
                }
            }
#ifdef ENABLE_MCTF
            else if (msdk_match(strInput[i], "-mctf")) {
                ParseMCTFParams(strInput, nArgNum, i, pParams, paramID);
            }

#endif
            else if (msdk_match(strInput[i], "-di_mode")) {
                pParams->deinterlaceParam[paramID].mode = VPP_FILTER_ENABLED_DEFAULT;

                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->deinterlaceParam[paramID].algorithm = (mfxU16)readData;
                        pParams->deinterlaceParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
                        i++;
                    }
                }
            }
            else if (msdk_match(strInput[i], "-tc_pattern")) {
                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->deinterlaceParam[paramID].tc_pattern = (mfxU16)readData;
                        i++;
                    }
                }
            }
            else if (msdk_match(strInput[i], "-tc_pos")) {
                //pParams->deinterlaceParam.mode = VPP_FILTER_ENABLED_DEFAULT;

                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->deinterlaceParam[paramID].tc_pos = (mfxU16)readData;
                        i++;
                    }
                }
            }
            else if (msdk_match(strInput[i], "-detail")) {
                pParams->detailParam[paramID].mode = VPP_FILTER_ENABLED_DEFAULT;

                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->detailParam[paramID].factor = (mfxU16)readData;
                        pParams->detailParam[paramID].mode   = VPP_FILTER_ENABLED_CONFIGURED;
                        i++;
                    }
                }
            }
            else if (msdk_match(strInput[i], "-rotate")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rotate[paramID]);
            }
            // different modes of MFX FRC
            else if (msdk_match(strInput[i], "-frc:advanced")) {
                pParams->frcParam[paramID].mode      = VPP_FILTER_ENABLED_CONFIGURED;
                pParams->frcParam[paramID].algorithm = MFX_FRCALGM_DISTRIBUTED_TIMESTAMP;
            }
            else if (msdk_match(strInput[i], "-frc:interp")) {
                pParams->frcParam[paramID].mode      = VPP_FILTER_ENABLED_CONFIGURED;
                pParams->frcParam[paramID].algorithm = MFX_FRCALGM_FRAME_INTERPOLATION;
            }
            //---------------------------------------------
            else if (msdk_match(strInput[i], "-pa_hue")) {
                pParams->procampParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->procampParam[paramID].hue);
            }
            else if (msdk_match(strInput[i], "-pa_bri")) {
                pParams->procampParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->procampParam[paramID].brightness);
            }
            else if (msdk_match(strInput[i], "-pa_con")) {
                pParams->procampParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->procampParam[paramID].contrast);
            }
            else if (msdk_match(strInput[i], "-pa_sat")) {
                pParams->procampParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->procampParam[paramID].saturation);
            }
#ifdef ENABLE_VPP_RUNTIME_HSBC
            else if (msdk_match(strInput[i], "-rt_hue")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtHue.interval);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtHue.value1);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtHue.value2);
                pParams->rtHue.isEnabled = true;
            }
            else if (msdk_match(strInput[i], "-rt_bri")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtBrightness.interval);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtBrightness.value1);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtBrightness.value2);
                pParams->rtBrightness.isEnabled = true;
            }
            else if (msdk_match(strInput[i], "-rt_con")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtContrast.interval);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtContrast.value1);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtContrast.value2);
                pParams->rtContrast.isEnabled = true;
            }
            else if (msdk_match(strInput[i], "-rt_sat")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtSaturation.interval);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtSaturation.value1);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtSaturation.value2);
                pParams->rtSaturation.isEnabled = true;
            }
#endif
            //MSDK 3.0
            else if (msdk_match(strInput[i], "-gamut:compression")) {
                pParams->gamutParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
            }
            else if (msdk_match(strInput[i], "-gamut:bt709")) {
                pParams->gamutParam[paramID].bBT709 = true;
            }
            else if (msdk_match(strInput[i], "-view:count")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;

                mfxU16 viewCount;
                msdk_opt_read(strInput[i], viewCount);
                if (viewCount > 1) {
                    pParams->multiViewParam[paramID].viewCount = (mfxU16)viewCount;
                    pParams->multiViewParam[paramID].mode      = VPP_FILTER_ENABLED_CONFIGURED;
                }
            }
            //---------------------------------------------
            // MSDK API 1.5
            else if (msdk_match(strInput[i], "-istab")) {
                pParams->istabParam[paramID].mode = VPP_FILTER_ENABLED_DEFAULT;

                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->istabParam[paramID].istabMode = (mfxU8)readData;
                        pParams->istabParam[paramID].mode      = VPP_FILTER_ENABLED_CONFIGURED;
                        i++;

                        if (pParams->istabParam[paramID].istabMode != 1 &&
                            pParams->istabParam[paramID].istabMode != 2) {
                            vppPrintHelp(strInput[0], "Invalid IStab configuration");
                            return MFX_ERR_UNSUPPORTED;
                        }
                    }
                }
            }
            //---------------------------------------------
            // IECP
            else if (msdk_match(strInput[i], "-ace")) {
                pParams->aceParam[paramID].mode = VPP_FILTER_ENABLED_DEFAULT;
            }
            else if (msdk_match(strInput[i], "-ste")) {
                pParams->steParam[paramID].mode = VPP_FILTER_ENABLED_DEFAULT;

                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->steParam[paramID].SkinToneFactor = (mfxU8)readData;
                        pParams->steParam[paramID].mode           = VPP_FILTER_ENABLED_CONFIGURED;
                        i++;
                    }
                }
            }
            else if (msdk_match(strInput[i], "-tcc:red")) {
                pParams->tccParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->tccParam[paramID].Red);
            }
            else if (msdk_match(strInput[i], "-tcc:green")) {
                pParams->tccParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->tccParam[paramID].Green);
            }
            else if (msdk_match(strInput[i], "-tcc:blue")) {
                pParams->tccParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->tccParam[paramID].Blue);
            }
            else if (msdk_match(strInput[i], "-tcc:magenta")) {
                pParams->tccParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->tccParam[paramID].Magenta);
            }
            else if (msdk_match(strInput[i], "-tcc:yellow")) {
                pParams->tccParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->tccParam[paramID].Yellow);
            }
            else if (msdk_match(strInput[i], "-tcc:cyan")) {
                pParams->tccParam[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->tccParam[paramID].Cyan);
            }
            else if (msdk_match(strInput[i], "-reset_end")) {
                break;
            }
            else if (msdk_match(strInput[i], "-cf_disable")) {
                pParams->colorfillParam[paramID].mode   = VPP_FILTER_ENABLED_CONFIGURED;
                pParams->colorfillParam[paramID].Enable = MFX_CODINGOPTION_OFF;
            }
            else if (msdk_match(strInput[i], "-SignalInfoIn")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoIn[paramID].VideoFullRange);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoIn[paramID].ColourPrimaries);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i],
                              pParams->videoSignalInfoIn[paramID].TransferCharacteristics);
                pParams->videoSignalInfoIn[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
            }
            else if (msdk_match(strInput[i], "-SignalInfoOut")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoOut[paramID].VideoFullRange);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoOut[paramID].ColourPrimaries);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i],
                              pParams->videoSignalInfoOut[paramID].TransferCharacteristics);
                pParams->videoSignalInfoOut[paramID].mode = VPP_FILTER_ENABLED_CONFIGURED;
            }
            else {
                printf("Unknow reset option: %s\n", strInput[i]);

                return MFX_ERR_UNKNOWN;
            }
        }
    }

    return MFX_ERR_NONE;

} // mfxStatus vppParseResetPar( ... )

void AdjustBitDepth(sInputParams& params) {
    if (params.frameInfoIn[0].BitDepthLuma != 0 || params.frameInfoIn[0].BitDepthChroma != 0) {
        if (params.frameInfoIn[0].BitDepthLuma == 0) {
            params.frameInfoIn[0].BitDepthLuma = params.frameInfoIn[0].BitDepthChroma;
            printf(
                "Warning: input BitDepthLuma was defaulted to value which was set to BitDepthChroma (%d).",
                params.frameInfoIn[0].BitDepthLuma);
        }

        if (params.frameInfoIn[0].BitDepthChroma == 0) {
            params.frameInfoIn[0].BitDepthChroma = params.frameInfoIn[0].BitDepthLuma;
            printf(
                "Warning: input BitDepthChroma was defaulted to value which was set to BitDepthLuma (%d).",
                params.frameInfoIn[0].BitDepthChroma);
        }
    }

    if (params.frameInfoOut[0].BitDepthLuma != 0 || params.frameInfoOut[0].BitDepthChroma != 0) {
        if (params.frameInfoOut[0].BitDepthLuma == 0) {
            params.frameInfoOut[0].BitDepthLuma = params.frameInfoOut[0].BitDepthChroma;
            printf(
                "Warning: output BitDepthLuma was defaulted to value which was set to BitDepthChroma (%d).",
                params.frameInfoOut[0].BitDepthLuma);
        }

        if (params.frameInfoOut[0].BitDepthChroma == 0) {
            params.frameInfoOut[0].BitDepthChroma = params.frameInfoOut[0].BitDepthLuma;
            printf(
                "Warning: output BitDepthChroma was defaulted to value which was set to BitDepthLuma (%d).",
                params.frameInfoOut[0].BitDepthChroma);
        }
    }
}

mfxStatus vppParseInputString(char* strInput[],
                              mfxU32 nArgNum,
                              sInputParams* pParams,
                              sFiltersParam* pDefaultFiltersParam) {
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(strInput, MFX_ERR_NULL_PTR);

    mfxU32 readData;
    if (nArgNum < 4) {
        vppPrintHelp(strInput[0], "Not enough parameters");

        return MFX_ERR_MORE_DATA;
    }

    pParams->frameInfoIn.back().CropX = 0;
    pParams->frameInfoIn.back().CropY = 0;
    pParams->frameInfoIn.back().CropW = NOT_INIT_VALUE;
    pParams->frameInfoIn.back().CropH = NOT_INIT_VALUE;
    // zeroize destination crops
    pParams->frameInfoOut.back().CropX = 0;
    pParams->frameInfoOut.back().CropY = 0;
    pParams->frameInfoOut.back().CropW = NOT_INIT_VALUE;
    pParams->frameInfoOut.back().CropH = NOT_INIT_VALUE;
    pParams->numStreams                = 1;

    for (mfxU32 i = 1; i < nArgNum; i++) {
        MSDK_CHECK_POINTER(strInput[i], MFX_ERR_NULL_PTR);
        {
            if (msdk_match(strInput[i], "-ssinr")) {
                VAL_CHECK(1 + i == nArgNum);

                pParams->videoSignalInfoParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;

                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoParam[0].In.NominalRange);
            }
            else if (msdk_match(strInput[i], "-dsinr")) {
                VAL_CHECK(1 + i == nArgNum);

                pParams->videoSignalInfoParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;

                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoParam[0].Out.NominalRange);
            }
            else if (msdk_match(strInput[i], "-ssitm")) {
                VAL_CHECK(1 + i == nArgNum);

                pParams->videoSignalInfoParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;

                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoParam[0].In.TransferMatrix);
            }
            else if (msdk_match(strInput[i], "-dsitm")) {
                VAL_CHECK(1 + i == nArgNum);

                pParams->videoSignalInfoParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;

                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoParam[0].Out.TransferMatrix);
            }
            else if (msdk_match(strInput[i], "-mirror")) {
                VAL_CHECK(1 + i == nArgNum);

                pParams->mirroringParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;

                i++;
                msdk_opt_read(strInput[i], pParams->mirroringParam[0].Type);
            }
            else if (msdk_match(strInput[i], "-sw")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoIn[0].nWidth);
            }
            else if (msdk_match(strInput[i], "-sh")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoIn[0].nHeight);
            }
            else if (msdk_match(strInput[i], "-scrX")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoIn[0].CropX);
            }
            else if (msdk_match(strInput[i], "-scrY")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoIn[0].CropY);
            }
            else if (msdk_match(strInput[i], "-scrW")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoIn[0].CropW);
            }
            else if (msdk_match(strInput[i], "-scrH")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoIn[0].CropH);
            }
            else if (msdk_match(strInput[i], "-spic")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                mfxI16 tmp;
                msdk_opt_read(strInput[i], tmp);
                pParams->frameInfoIn[0].PicStruct = GetPicStruct(static_cast<mfxI8>(tmp));
            }
            else if (msdk_match(strInput[i], "-sf")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoIn[0].dFrameRate);
            }
            else if (msdk_match_i(strInput[i], "-dw")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoOut[0].nWidth);
            }
            else if (msdk_match_i(strInput[i], "-dh")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoOut[0].nHeight);
            }
            else if (msdk_match(strInput[i], "-dcrX")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoOut[0].CropX);
            }
            else if (msdk_match(strInput[i], "-dcrY")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoOut[0].CropY);
            }
            else if (msdk_match(strInput[i], "-dcrW")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoOut[0].CropW);
            }
            else if (msdk_match(strInput[i], "-dcrH")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoOut[0].CropH);
            }
            else if (msdk_match(strInput[i], "-dpic")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                mfxI16 tmp;
                msdk_opt_read(strInput[i], tmp);
                pParams->frameInfoOut[0].PicStruct = GetPicStruct(static_cast<mfxI8>(tmp));
            }
            else if (msdk_match(strInput[i], "-df")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoOut[0].dFrameRate);
            }
            //-----------------------------------------------------------------------------------
            //                   Video Enhancement Algorithms
            //-----------------------------------------------------------------------------------
            else if (msdk_match(strInput[i], "-denoise")) {
                pParams->denoiseParam[0].mode = VPP_FILTER_ENABLED_DEFAULT;

                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->denoiseParam[0].factor = (mfxU16)readData;
                        pParams->denoiseParam[0].mode   = VPP_FILTER_ENABLED_CONFIGURED;
                        i++;
                    }
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->denoiseParam[0].config = (mfxU16)readData;
                        pParams->denoiseParam[0].mode   = VPP_FILTER_ENABLED_CONFIGURED;
                        i++;
                    }
                }
            }
#ifdef ENABLE_MCTF
            else if (msdk_match(strInput[i], "-mctf")) {
                ParseMCTFParams(strInput, nArgNum, i, pParams, 0);
            }
#endif

            // aya: altenative and simple way to enable deinterlace
            else if (msdk_match(strInput[i], "-deinterlace")) {
                pParams->frameInfoOut[0].PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
                pParams->frameInfoIn[0].PicStruct  = MFX_PICSTRUCT_FIELD_TFF;

                if (i + 1 < nArgNum) {
                    if (msdk_match(strInput[i + 1], "bff")) {
                        pParams->frameInfoOut[0].PicStruct = MFX_PICSTRUCT_FIELD_BFF;
                        i++;
                    }
                }
            }
            else if (msdk_match(strInput[i], "-di_mode")) {
                pParams->deinterlaceParam[0].mode = VPP_FILTER_ENABLED_DEFAULT;

                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->deinterlaceParam[0].algorithm = (mfxU16)readData;
                        pParams->deinterlaceParam[0].mode      = VPP_FILTER_ENABLED_CONFIGURED;
                        i++;
                    }
                }
            }
            else if (msdk_match(strInput[i], "-tc_pattern")) {
                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->deinterlaceParam[0].tc_pattern = (mfxU16)readData;
                        i++;
                    }
                }
            }
            else if (msdk_match(strInput[i], "-tc_pos")) {
                //pParams->deinterlaceParam.mode = VPP_FILTER_ENABLED_DEFAULT;

                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->deinterlaceParam[0].tc_pos = (mfxU16)readData;
                        i++;
                    }
                }
            }
            else if (msdk_match(strInput[i], "-detail")) {
                pParams->detailParam[0].mode = VPP_FILTER_ENABLED_DEFAULT;

                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->detailParam[0].factor = (mfxU16)readData;
                        pParams->detailParam[0].mode   = VPP_FILTER_ENABLED_CONFIGURED;
                        i++;
                    }
                }
            }
            else if (msdk_match(strInput[i], "-rotate")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rotate[0]);
            }
            else if (msdk_match(strInput[i], "-scaling_mode")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                pParams->bScaling = true;
                msdk_opt_read(strInput[i], pParams->scalingMode);
            }
            else if (msdk_match(strInput[i], "-interpolation_method")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                pParams->bScaling = true;
                msdk_opt_read(strInput[i], pParams->interpolationMethod);
            }
            else if (msdk_match(strInput[i], "-chroma_siting")) {
                VAL_CHECK(2 + i == nArgNum);
                bool bVfound = false;
                bool bHfound = false;
                i++;
                for (int ii = 0; ii < 2; ii++) {
                    /* ChromaSiting */
                    if (msdk_match(strInput[i + ii], "vtop")) {
                        pParams->uChromaSiting |= MFX_CHROMA_SITING_VERTICAL_TOP;
                        bVfound = true;
                    }
                    else if (msdk_match(strInput[i + ii], "vcen")) {
                        pParams->uChromaSiting |= MFX_CHROMA_SITING_VERTICAL_CENTER;
                        bVfound = true;
                    }
                    else if (msdk_match(strInput[i + ii], "vbot")) {
                        pParams->uChromaSiting |= MFX_CHROMA_SITING_VERTICAL_BOTTOM;
                        bVfound = true;
                    }
                    else if (msdk_match(strInput[i + ii], "hleft")) {
                        pParams->uChromaSiting |= MFX_CHROMA_SITING_HORIZONTAL_LEFT;
                        bHfound = true;
                    }
                    else if (msdk_match(strInput[i + ii], "hcen")) {
                        pParams->uChromaSiting |= MFX_CHROMA_SITING_HORIZONTAL_CENTER;
                        bHfound = true;
                    }
                    else
                        printf("Unknown Chroma siting flag %s", strInput[i + ii]);
                }
                pParams->bChromaSiting = bVfound && bHfound;
                if (!pParams->bChromaSiting) {
                    vppPrintHelp(strInput[0], "Invalid chroma siting flags\n");
                    return MFX_ERR_UNSUPPORTED;
                }
                i++;
            }
            else if (msdk_match(strInput[i], "-SignalInfoIn")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoIn[0].VideoFullRange);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoIn[0].ColourPrimaries);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoIn[0].TransferCharacteristics);
                pParams->videoSignalInfoIn[0].mode = VPP_FILTER_ENABLED_CONFIGURED;
            }
            else if (msdk_match(strInput[i], "-SignalInfoOut")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoOut[0].VideoFullRange);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoOut[0].ColourPrimaries);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->videoSignalInfoOut[0].TransferCharacteristics);
                pParams->videoSignalInfoOut[0].mode = VPP_FILTER_ENABLED_CONFIGURED;
            }
            else if (msdk_match(strInput[i], "-composite")) {
                if (i + 1 < nArgNum) {
                    if (ParseCompositionParfile(strInput[i + 1], pParams) != MFX_ERR_NONE) {
                        vppPrintHelp(
                            strInput[0],
                            "Parfile for -composite has invalid data or cannot be opened\n");
                        return MFX_ERR_UNSUPPORTED;
                    }
                    pParams->compositionParam.mode = VPP_FILTER_ENABLED_CONFIGURED;
                    i++;
                }
            }
            // different modes of MFX FRC
            else if (msdk_match(strInput[i], "-frc:advanced")) {
                pParams->frcParam[0].mode      = VPP_FILTER_ENABLED_CONFIGURED;
                pParams->frcParam[0].algorithm = MFX_FRCALGM_DISTRIBUTED_TIMESTAMP;
            }
            else if (msdk_match(strInput[i], "-frc:interp")) {
                pParams->frcParam[0].mode      = VPP_FILTER_ENABLED_CONFIGURED;
                pParams->frcParam[0].algorithm = MFX_FRCALGM_FRAME_INTERPOLATION;
            }
            //---------------------------------------------
            else if (msdk_match(strInput[i], "-pa_hue")) {
                pParams->procampParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->procampParam[0].hue);
            }
            else if (msdk_match(strInput[i], "-pa_bri")) {
                pParams->procampParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->procampParam[0].brightness);
            }
            else if (msdk_match(strInput[i], "-pa_con")) {
                pParams->procampParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->procampParam[0].contrast);
            }
            else if (msdk_match(strInput[i], "-pa_sat")) {
                pParams->procampParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->procampParam[0].saturation);
            }
#ifdef ENABLE_VPP_RUNTIME_HSBC
            else if (msdk_match(strInput[i], "-rt_hue")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtHue.interval);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtHue.value1);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtHue.value2);
                pParams->rtHue.isEnabled = true;
            }
            else if (msdk_match(strInput[i], "-rt_bri")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtBrightness.interval);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtBrightness.value1);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtBrightness.value2);
                pParams->rtBrightness.isEnabled = true;
            }
            else if (msdk_match(strInput[i], "-rt_con")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtContrast.interval);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtContrast.value1);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtContrast.value2);
                pParams->rtContrast.isEnabled = true;
            }
            else if (msdk_match(strInput[i], "-rt_sat")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtSaturation.interval);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtSaturation.value1);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->rtSaturation.value2);
                pParams->rtSaturation.isEnabled = true;
            }
#endif

            //MSDK 3.0
            else if (msdk_match(strInput[i], "-gamut:compression")) {
                pParams->gamutParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;
            }
            else if (msdk_match(strInput[i], "-gamut:bt709")) {
                pParams->gamutParam[0].bBT709 = true;
            }
            else if (msdk_match(strInput[i], "-view:count")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;

                mfxU16 viewCount;
                msdk_opt_read(strInput[i], viewCount);
                if (viewCount > 1) {
                    pParams->multiViewParam[0].viewCount = (mfxU16)viewCount;
                    pParams->multiViewParam[0].mode      = VPP_FILTER_ENABLED_CONFIGURED;
                }
            }
            //---------------------------------------------
            // MSDK API 1.5
            else if (msdk_match(strInput[i], "-istab")) {
                pParams->istabParam[0].mode = VPP_FILTER_ENABLED_DEFAULT;

                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->istabParam[0].istabMode = (mfxU8)readData;
                        pParams->istabParam[0].mode      = VPP_FILTER_ENABLED_CONFIGURED;
                        i++;

                        if (pParams->istabParam[0].istabMode != 1 &&
                            pParams->istabParam[0].istabMode != 2) {
                            vppPrintHelp(strInput[0], "Invalid IStab configuration");
                            return MFX_ERR_UNSUPPORTED;
                        }
                    }
                }
            }
            //---------------------------------------------
            // IECP
            else if (msdk_match(strInput[i], "-ace")) {
                pParams->aceParam[0].mode = VPP_FILTER_ENABLED_DEFAULT;
            }
            else if (msdk_match(strInput[i], "-ste")) {
                pParams->steParam[0].mode = VPP_FILTER_ENABLED_DEFAULT;

                if (i + 1 < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i + 1], readData)) {
                        return MFX_ERR_UNSUPPORTED;
                    }
                    else {
                        pParams->steParam[0].SkinToneFactor = (mfxU8)readData;
                        pParams->steParam[0].mode           = VPP_FILTER_ENABLED_CONFIGURED;
                        i++;
                    }
                }
            }
            else if (msdk_match(strInput[i], "-tcc:red")) {
                pParams->tccParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->tccParam[0].Red);
            }
            else if (msdk_match(strInput[i], "-tcc:green")) {
                pParams->tccParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->tccParam[0].Green);
            }
            else if (msdk_match(strInput[i], "-tcc:blue")) {
                pParams->tccParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->tccParam[0].Blue);
            }
            else if (msdk_match(strInput[i], "-tcc:magenta")) {
                pParams->tccParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->tccParam[0].Magenta);
            }
            else if (msdk_match(strInput[i], "-tcc:yellow")) {
                pParams->tccParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->tccParam[0].Yellow);
            }
            else if (msdk_match(strInput[i], "-tcc:cyan")) {
                pParams->tccParam[0].mode = VPP_FILTER_ENABLED_CONFIGURED;
                i++;
                msdk_opt_read(strInput[i], pParams->tccParam[0].Cyan);
            }
            //-----------------------------------------------------------------------------------
            //                   Region of Interest Testing
            //-----------------------------------------------------------------------------------
            else if (msdk_match(strInput[i], "-roi_check")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                pParams->roiCheckParam.mode = Str2ROIMode(strInput[i]);

                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->roiCheckParam.srcSeed);

                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->roiCheckParam.dstSeed);
            }
            //-----------------------------------------------------------------------------------
            else if (msdk_match(strInput[i], "-i")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_strncopy_s(pParams->strSrcFile,
                                MSDK_MAX_FILENAME_LEN,
                                strInput[i],
                                MSDK_MAX_FILENAME_LEN - 1);
                pParams->strSrcFile[MSDK_MAX_FILENAME_LEN - 1] = 0;
            }
            else if (msdk_match(strInput[i], "-o")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;

                pParams->strDstFiles.push_back(strInput[i]);
                pParams->isOutput = true;
            }
            else if (msdk_match(strInput[i], "-pf")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_strncopy_s(pParams->strPerfFile,
                                MSDK_MAX_FILENAME_LEN,
                                strInput[i],
                                MSDK_MAX_FILENAME_LEN - 1);
                pParams->strPerfFile[MSDK_MAX_FILENAME_LEN - 1] = 0;
            }
            else if (msdk_match(strInput[i], "-scc")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                pParams->fccSource = pParams->frameInfoIn[0].FourCC = Str2FourCC(strInput[i]);

                //if (MFX_FOURCC_I420 == pParams->frameInfoIn[0].FourCC)
                //{
                //    pParams->frameInfoIn[0].FourCC = MFX_FOURCC_YV12; // I420 input is implemented using YV12 internally
                //}

                if (!pParams->frameInfoIn[0].FourCC) {
                    vppPrintHelp(strInput[0], "Invalid -scc format\n");
                    return MFX_ERR_UNSUPPORTED;
                }
            }
            else if (msdk_match(strInput[i], "-dcc")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                pParams->frameInfoOut[0].FourCC = Str2FourCC(strInput[i]);
                pParams->forcedOutputFourcc     = 0;
                if (MFX_FOURCC_I420 == pParams->frameInfoOut[0].FourCC ||
                    MFX_FOURCC_YV12 == pParams->frameInfoOut[0].FourCC) {
                    pParams->forcedOutputFourcc = pParams->frameInfoOut[0].FourCC;
                    pParams->frameInfoOut[0].FourCC =
                        MFX_FOURCC_NV12; // I420 output is implemented using NV12 internally
                }

                if (!pParams->frameInfoOut[0].FourCC) {
                    vppPrintHelp(strInput[0], "Invalid -dcc format\n");
                    return MFX_ERR_UNSUPPORTED;
                }
            }
            else if (msdk_match(strInput[i], "-dbitshift")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoOut[0].Shift);
            }
            else if (msdk_match(strInput[i], "-dbitdepthluma")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoOut[0].BitDepthLuma);
            }
            else if (msdk_match(strInput[i], "-dbitdepthchroma")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoOut[0].BitDepthChroma);
            }
            else if (msdk_match(strInput[i], "-sbitshift")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoIn[0].Shift);
            }
            else if (msdk_match(strInput[i], "-sbitdepthluma")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoIn[0].BitDepthLuma);
            }
            else if (msdk_match(strInput[i], "-sbitdepthchroma")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->frameInfoIn[0].BitDepthChroma);
            }
            else if (msdk_match(strInput[i], "-iopattern")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                pParams->IOPattern = Str2IOpattern(strInput[i]);
            }
            else if (msdk_match(strInput[i], "-lib")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                if (msdk_match(strInput[i], "sw"))
                    pParams->ImpLib = MFX_IMPL_SOFTWARE;
                else if (msdk_match(strInput[i], "hw")) {
                    pParams->ImpLib = MFX_IMPL_HARDWARE;
                }
            }

#if defined(_WIN32) || defined(_WIN64)
            else if (msdk_match(strInput[i], "-3dlut")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_strncopy_s(pParams->lutTableFile,
                                MSDK_MAX_FILENAME_LEN,
                                strInput[i],
                                MSDK_MAX_FILENAME_LEN - 1);
                pParams->lutTableFile[MSDK_MAX_FILENAME_LEN - 1] = 0;
                pParams->b3dLut                                  = true;
            }
            else if (msdk_match(strInput[i], "-3dlutMode")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->lutSize);
            }

            else if (msdk_match(strInput[i], "-3dlutMemType")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                mfxU16 memType;
                msdk_opt_read(strInput[i], memType);
                if (memType == 0) {
                    pParams->bIs3dLutVideoMem = true;
                }
                else {
                    pParams->bIs3dLutVideoMem = false;
                }
            }
#endif
#if (defined(LINUX32) || defined(LINUX64))
            else if (msdk_match(strInput[i], "-device")) {
                if (!pParams->strDevicePath.empty()) {
                    printf("error: you can specify only one device\n");
                    return MFX_ERR_UNSUPPORTED;
                }
                VAL_CHECK(i + 1 == nArgNum);
                pParams->strDevicePath = strInput[++i];
                size_t pos             = pParams->strDevicePath.find("renderD");
                if (pos != std::string::npos) {
                    pParams->DRMRenderNodeNum =
                        std::stoi(pParams->strDevicePath.substr(pos + 7, 3));
                }
            }
#endif
#if defined(_WIN32)
            else if (msdk_match(strInput[i], "-luid")) {
                // <HighPart:LowPart>
                char luid[MSDK_MAX_FILENAME_LEN];
                if (i + 1 >= nArgNum) {
                    printf("error: Not enough parameters for -luid key\n");
                    return MFX_ERR_UNSUPPORTED;
                }
                i++;
                if (MFX_ERR_NONE != msdk_opt_read(strInput[i], luid)) {
                    printf("error: '-luid' arguments is invalid\n");
                    return MFX_ERR_UNSUPPORTED;
                }

                std::string temp = std::string(luid);
                const std::regex pieces_regex("(0[xX][0-9a-fA-F]+):(0[xX][0-9a-fA-F]+)");
                std::smatch pieces_match;

                // pieces_match = [full match, HighPart, LowPart]
                if (std::regex_match(temp, pieces_match, pieces_regex) &&
                    pieces_match.size() == 3) {
                    pParams->luid.HighPart = std::strtol(pieces_match[1].str().c_str(), 0, 16);
                    pParams->luid.LowPart  = std::strtol(pieces_match[2].str().c_str(), 0, 16);
                }
                else {
                    printf("error: format of -LUID is invalid, please, use: HighPart:LowPart\n");
                    return MFX_ERR_UNSUPPORTED;
                }
            }
#endif
            else if (msdk_match(strInput[i], "-pci")) {
                char deviceInfo[MSDK_MAX_FILENAME_LEN];
                if (i + 1 >= nArgNum) {
                    printf("error: Not enough parameters for -pci key\n");
                    return MFX_ERR_UNSUPPORTED;
                }
                i++;
                if ((strlen(strInput[i]) + 1) > MSDK_ARRAY_LEN(deviceInfo)) {
                    printf("error: '-pci' arguments is too long\n");
                    return MFX_ERR_UNSUPPORTED;
                }
                msdk_opt_read(strInput[i], deviceInfo);

                // template: <domain:bus:device.function>
                std::string temp = std::string(deviceInfo);
                const std::regex pieces_regex("([0-9]+):([0-9]+):([0-9]+).([0-9]+)");
                std::smatch pieces_match;

                // pieces_match = [full match, PCIDomain, PCIBus, PCIDevice, PCIFunction]
                if (std::regex_match(temp, pieces_match, pieces_regex) &&
                    pieces_match.size() == 5) {
                    pParams->PCIDomain      = std::atoi(pieces_match[1].str().c_str());
                    pParams->PCIBus         = std::atoi(pieces_match[2].str().c_str());
                    pParams->PCIDevice      = std::atoi(pieces_match[3].str().c_str());
                    pParams->PCIFunction    = std::atoi(pieces_match[4].str().c_str());
                    pParams->PCIDeviceSetup = true;
                }
                else {
                    printf("format of -pci is invalid, please, use: domain:bus:device.function");
                    return MFX_ERR_UNSUPPORTED;
                }
            }
            else if (msdk_match(strInput[i], "-dGfx")) {
                pParams->adapterType = mfxMediaAdapterType::MFX_MEDIA_DISCRETE;
                if (i + 1 < nArgNum && isdigit(*strInput[1 + i])) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->dGfxIdx)) {
                        printf("value of -dGfx is invalid");
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
#if (defined(_WIN64) || defined(_WIN32)) && (MFX_VERSION >= 1031)
                pParams->bPreferdGfx = true;
#endif
            }
            else if (msdk_match(strInput[i], "-iGfx")) {
                pParams->adapterType = mfxMediaAdapterType::MFX_MEDIA_INTEGRATED;
#if (defined(_WIN64) || defined(_WIN32)) && (MFX_VERSION >= 1031)
                pParams->bPreferiGfx = true;
#endif
            }
            else if (msdk_match(strInput[i], "-AdapterNum")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->adapterNum);
            }
            else if (msdk_match(strInput[i], "-dispatcher:fullSearch")) {
                pParams->dispFullSearch = true;
            }
            else if (msdk_match(strInput[i], "-dispatcher:lowLatency")) {
                pParams->dispFullSearch = false;
            }
#if defined(D3D_SURFACES_SUPPORT)
            else if (msdk_match(strInput[i], "-d3d")) {
                pParams->IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;
                pParams->ImpLib |= MFX_IMPL_VIA_D3D9;
                pParams->accelerationMode = MFX_ACCEL_MODE_VIA_D3D9;
            }
#endif
#if MFX_D3D11_SUPPORT
            else if (msdk_match(strInput[i], "-d3d11")) {
                pParams->IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;
                pParams->ImpLib |= MFX_IMPL_VIA_D3D11;
                pParams->accelerationMode = MFX_ACCEL_MODE_VIA_D3D11;
            }
#endif
#ifdef LIBVA_SUPPORT
            else if (msdk_match(strInput[i], "-vaapi") || msdk_match(strInput[i], "-d3d")) {
                pParams->IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;
                pParams->ImpLib |= MFX_IMPL_VIA_VAAPI;
                pParams->accelerationMode = MFX_ACCEL_MODE_VIA_VAAPI;
            }
#endif
            else if (msdk_match(strInput[i], "-async")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->asyncNum);
            }
            else if (msdk_match(strInput[i], "-perf_opt")) {
                if (pParams->numFrames)
                    return MFX_ERR_UNKNOWN;

                VAL_CHECK(1 + i == nArgNum);
                pParams->bPerf = true;
                i++;
                msdk_opt_read(strInput[i], pParams->numFrames);
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->numRepeat);
            }
            else if (msdk_match(strInput[i], "-pts_check")) {
                pParams->ptsCheck = true;
            }
            else if (msdk_match(strInput[i], "-pts_jump")) {
                pParams->ptsJump = true;
            }
            else if (msdk_match(strInput[i], "-pts_fr")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->ptsFR);
            }
            else if (msdk_match(strInput[i], "-pts_advanced")) {
                pParams->ptsAdvanced = true;
            }
            else if (msdk_match(strInput[i], "-n")) {
                if (pParams->bPerf)
                    return MFX_ERR_UNKNOWN;

                VAL_CHECK(1 + i == nArgNum);
                i++;
                msdk_opt_read(strInput[i], pParams->numFrames);
            }
            else if (msdk_match(strInput[i], "-reset_start")) {
                VAL_CHECK(1 + i == nArgNum);
                msdk_opt_read(strInput[i + 1], readData);
                i += 2;

                pParams->resetFrmNums.push_back((mfxU16)readData);

                if (MFX_ERR_NONE != vppParseResetPar(strInput,
                                                     nArgNum,
                                                     i,
                                                     pParams,
                                                     (mfxU32)pParams->resetFrmNums.size(),
                                                     pDefaultFiltersParam))
                    return MFX_ERR_UNKNOWN;
            }
            else if (msdk_match(strInput[i], "-cf_disable")) {
                pParams->colorfillParam[0].mode   = VPP_FILTER_ENABLED_CONFIGURED;
                pParams->colorfillParam[0].Enable = MFX_CODINGOPTION_OFF;
            }
            else if (msdk_match(strInput[i], "-api_ver_init::1x")) {
                pParams->verSessionInit = API_1X;
            }
            else if (msdk_match(strInput[i], "-api_ver_init::2x")) {
                pParams->verSessionInit = API_2X;
            }
            else if (msdk_match(strInput[i], "-rbf")) {
                pParams->bReadByFrame = true;
            }
#ifdef ONEVPL_EXPERIMENTAL
            else if (msdk_match(strInput[i], "-cfg::vpp")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                pParams->m_vpp_cfg = strInput[i];
            }
#endif
            else if (msdk_match(strInput[i], "-dump")) {
                VAL_CHECK(1 + i == nArgNum);
                i++;
                pParams->dump_file = strInput[i];
            }
            else {
                printf("Unknown option: %s\n", strInput[i]);

                return MFX_ERR_UNKNOWN;
            }
        }
    }

    if ((pParams->ImpLib & MFX_IMPL_HARDWARE) && !(pParams->ImpLib & MFX_IMPL_VIA_D3D9)) {
        pParams->ImpLib = MFX_IMPL_HARDWARE;
#ifdef LIBVA_SUPPORT
        pParams->ImpLib |= MFX_IMPL_VIA_VAAPI;
        pParams->accelerationMode = MFX_ACCEL_MODE_VIA_VAAPI;
#else
        pParams->ImpLib |= MFX_IMPL_VIA_D3D11;
        pParams->accelerationMode = MFX_ACCEL_MODE_VIA_D3D11;
#endif
    }

    std::vector<sOwnFrameInfo>::iterator it = pParams->frameInfoIn.begin();
    while (it != pParams->frameInfoIn.end()) {
        if (NOT_INIT_VALUE == it->CropW) {
            it->CropW = it->nWidth;
        }

        if (NOT_INIT_VALUE == it->CropH) {
            it->CropH = it->nHeight;
        }
        it++;
    }

    it = pParams->frameInfoOut.begin();
    while (it != pParams->frameInfoOut.end()) {
        if (NOT_INIT_VALUE == it->CropW) {
            it->CropW = it->nWidth;
        }

        if (NOT_INIT_VALUE == it->CropH) {
            it->CropH = it->nHeight;
        }
        it++;
    }

    if (0 == strlen(pParams->strSrcFile) &&
        pParams->compositionParam.mode != VPP_FILTER_ENABLED_CONFIGURED) {
        vppPrintHelp(strInput[0], "Source file name not found");
        return MFX_ERR_UNSUPPORTED;
    };

    if (1 != pParams->strDstFiles.size() &&
        (pParams->resetFrmNums.size() + 1) != pParams->strDstFiles.size()) {
        printf("File output is disabled as -o option isn't specified\n");
    };

    if (0 == pParams->IOPattern) {
        vppPrintHelp(strInput[0], "Incorrect IOPattern");
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->ImpLib & MFX_IMPL_SOFTWARE) &&
        (pParams->IOPattern & (MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY))) {
        printf(
            "Warning: IOPattern has been reset to 'sys_to_sys' mode because software library implementation is selected.");
        pParams->IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    }

#if (defined(_WIN64) || defined(_WIN32)) && (MFX_VERSION >= 1031)
    if (pParams->bPreferdGfx && pParams->bPreferiGfx) {
        printf("Warning: both dGfx and iGfx flags set. iGfx will be preferred");
        pParams->bPreferdGfx = false;
    }
#endif

    // Align values of luma and chroma bit depth if only one of them set by user
    AdjustBitDepth(*pParams);

    return MFX_ERR_NONE;

} // mfxStatus vppParseInputString( ... )

bool CheckInputParams(char* strInput[], sInputParams* pParams) {
    // Setting  default width and height if it was omitted. For composition case parameters should be define explicitly
    if (pParams->frameInfoOut[0].nWidth == 0) {
        if (pParams->compositionParam.mode == VPP_FILTER_ENABLED_CONFIGURED) {
            vppPrintHelp(
                strInput[0],
                "ERROR: Destination width should be set explicitely in case of composition mode.\n");
            return false;
        }
        pParams->frameInfoOut[0].nWidth = pParams->frameInfoIn[0].nWidth;
        pParams->frameInfoOut[0].CropW  = pParams->frameInfoIn[0].CropW;
        pParams->frameInfoOut[0].CropX  = 0;
    }

    if (pParams->frameInfoOut[0].nHeight == 0) {
        if (pParams->compositionParam.mode == VPP_FILTER_ENABLED_CONFIGURED) {
            vppPrintHelp(
                strInput[0],
                "ERROR: Destination height should be set explicitely in case of composition mode.\n");
            return false;
        }
        pParams->frameInfoOut[0].nHeight = pParams->frameInfoIn[0].nHeight;
        pParams->frameInfoOut[0].CropH   = pParams->frameInfoIn[0].CropH;
        pParams->frameInfoOut[0].CropY   = 0;
    }

    // Checking other parameters
    if (0 == pParams->asyncNum) {
        vppPrintHelp(strInput[0], "Incompatible parameters: [ayncronous number must exceed 0]\n");
        return false;
    }

    for (mfxU32 i = 0; i < pParams->rotate.size(); i++) {
        if (pParams->rotate[i] != 0 && pParams->rotate[i] != 90 && pParams->rotate[i] != 180 &&
            pParams->rotate[i] != 270) {
            vppPrintHelp(strInput[0],
                         "Invalid -rotate parameter: supported values 0, 90, 180, 270\n");
            return false;
        }
    }

    for (mfxU32 i = 0; i < pParams->numStreams; i++) {
        const mfxVPPCompInputStream& is = pParams->compositionParam.streamInfo[i].compStream;

        if ((pParams->frameInfoOut[0].nWidth < is.DstW + is.DstX) ||
            (pParams->frameInfoOut[0].nHeight < is.DstH + is.DstY)) {
            vppPrintHelp(strInput[0],
                         "One of composing frames cannot fit into destination frame.\n");
            return false;
        }
    }

    if ((pParams->ImpLib & MFX_IMPL_HARDWARE) && pParams->bReadByFrame) {
        vppPrintHelp(strInput[0], "-rbf (Read by frame) is not supported in hw lib.\n");
        return false;
    }

    return true;
} // bool CheckInputParams(char* strInput[], sInputVppParams* pParams )

// trim from start
static inline std::string& ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c) {
                return !std::isspace(c);
            }));
    return s;
}

// trim from end
static inline std::string& rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(),
                         s.rend(),
                         [](unsigned char c) {
                             return !std::isspace(c);
                         })
                .base(),
            s.end());
    return s;
}

// trim from both ends
static inline std::string& trim(std::string& s) {
    return ltrim(rtrim(s));
}

void getPair(std::string line, std::string& key, std::string& value) {
    std::istringstream iss(line);
    getline(iss, key, '=');
    getline(iss, value, '=');
    trim(key);
    trim(value);
}

mfxStatus ParseCompositionParfile(const char* parFileName, sInputParams* pParams) {
    mfxStatus sts = MFX_ERR_NONE;
    if (strlen(parFileName) == 0)
        return MFX_ERR_UNKNOWN;

    MSDK_ZERO_MEMORY(pParams->inFrameInfo);

    std::string line;
    std::string key, value;
    mfxU8 nStreamInd       = 0;
    mfxU8 firstStreamFound = 0;
    std::ifstream stream(parFileName);
    if (stream.fail())
        return MFX_ERR_UNKNOWN;

    while (getline(stream, line) && nStreamInd < MAX_INPUT_STREAMS) {
        getPair(line, key, value);
        if (key.compare("width") == 0) {
            pParams->inFrameInfo[nStreamInd].nWidth = (mfxU16)MSDK_ALIGN16(atoi(value.c_str()));
        }
        else if (key.compare("height") == 0) {
            pParams->inFrameInfo[nStreamInd].nHeight =
                (MFX_PICSTRUCT_PROGRESSIVE == pParams->inFrameInfo[nStreamInd].PicStruct)
                    ? (mfxU16)MSDK_ALIGN16(atoi(value.c_str()))
                    : (mfxU16)MSDK_ALIGN32(atoi(value.c_str()));
        }
        else if (key.compare("cropx") == 0) {
            pParams->inFrameInfo[nStreamInd].CropX = (mfxU16)atoi(value.c_str());
        }
        else if (key.compare("cropy") == 0) {
            pParams->inFrameInfo[nStreamInd].CropY = (mfxU16)atoi(value.c_str());
        }
        else if (key.compare("cropw") == 0) {
            pParams->inFrameInfo[nStreamInd].CropW = (mfxU16)atoi(value.c_str());
        }
        else if (key.compare("croph") == 0) {
            pParams->inFrameInfo[nStreamInd].CropH = (mfxU16)atoi(value.c_str());
        }
        else if (key.compare("framerate") == 0) {
            pParams->inFrameInfo[nStreamInd].dFrameRate = (mfxF64)atof(value.c_str());
        }
        else if (key.compare("fourcc") == 0) {
            const mfxU16 len_size = 5;
            char fourcc[len_size];
            for (mfxU16 i = 0; i < (value.size() > len_size ? len_size : value.size()); i++)
                fourcc[i] = value.at(i);
            fourcc[len_size - 1]                    = 0;
            pParams->inFrameInfo[nStreamInd].FourCC = Str2FourCC(fourcc);

            if (!pParams->inFrameInfo[nStreamInd].FourCC) {
                printf("Invalid fourcc parameter in par file: %s\n", fourcc);
                return MFX_ERR_INVALID_VIDEO_PARAM;
            }
        }
        else if (key.compare("picstruct") == 0) {
            pParams->inFrameInfo[nStreamInd].PicStruct = GetPicStruct((mfxI8)atoi(value.c_str()));
        }
        else if (key.compare("dstx") == 0) {
            pParams->compositionParam.streamInfo[nStreamInd].compStream.DstX =
                (mfxU16)atoi(value.c_str());
        }
        else if (key.compare("dsty") == 0) {
            pParams->compositionParam.streamInfo[nStreamInd].compStream.DstY =
                (mfxU16)atoi(value.c_str());
        }
        else if (key.compare("dstw") == 0) {
            pParams->compositionParam.streamInfo[nStreamInd].compStream.DstW =
                (mfxU16)atoi(value.c_str());
        }
        else if (key.compare("dsth") == 0) {
            pParams->compositionParam.streamInfo[nStreamInd].compStream.DstH =
                (mfxU16)atoi(value.c_str());
        }
        else if (key.compare("GlobalAlphaEnable") == 0) {
            pParams->compositionParam.streamInfo[nStreamInd].compStream.GlobalAlphaEnable =
                (mfxU16)atoi(value.c_str());
        }
        else if (key.compare("GlobalAlpha") == 0) {
            pParams->compositionParam.streamInfo[nStreamInd].compStream.GlobalAlpha =
                (mfxU16)atoi(value.c_str());
        }
        else if (key.compare("PixelAlphaEnable") == 0) {
            pParams->compositionParam.streamInfo[nStreamInd].compStream.PixelAlphaEnable =
                (mfxU16)atoi(value.c_str());
        }
        else if (key.compare("LumaKeyEnable") == 0) {
            pParams->compositionParam.streamInfo[nStreamInd].compStream.LumaKeyEnable =
                (mfxU16)atoi(value.c_str());
        }
        else if (key.compare("LumaKeyMin") == 0) {
            pParams->compositionParam.streamInfo[nStreamInd].compStream.LumaKeyMin =
                (mfxU16)atoi(value.c_str());
        }
        else if (key.compare("LumaKeyMax") == 0) {
            pParams->compositionParam.streamInfo[nStreamInd].compStream.LumaKeyMax =
                (mfxU16)atoi(value.c_str());
        }
        else if ((key.compare("stream") == 0 || key.compare("primarystream") == 0) &&
                 nStreamInd < (MAX_INPUT_STREAMS - 1)) {
            const mfxU16 len_size = MSDK_MAX_FILENAME_LEN - 1;

            if (firstStreamFound == 1) {
                nStreamInd++;
            }
            else {
                nStreamInd       = 0;
                firstStreamFound = 1;
            }
            pParams->inFrameInfo[nStreamInd].CropX = 0;
            pParams->inFrameInfo[nStreamInd].CropY = 0;
            pParams->inFrameInfo[nStreamInd].CropW = NOT_INIT_VALUE;
            pParams->inFrameInfo[nStreamInd].CropH = NOT_INIT_VALUE;
            mfxU16 i                               = 0;
            for (; i < (value.size() > len_size ? len_size : value.size()); i++)
                pParams->compositionParam.streamInfo[nStreamInd].streamName[i] = value.at(i);
            pParams->compositionParam.streamInfo[nStreamInd].streamName[i] = 0;
            pParams->inFrameInfo[nStreamInd].dFrameRate                    = 30;
            pParams->inFrameInfo[nStreamInd].PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
        }
    }

    pParams->numStreams = nStreamInd + 1;

    for (int i = 0; i < pParams->numStreams; i++) {
        if (!pParams->inFrameInfo[i].FourCC) {
            printf("Fourcc parameter of stream %d in par file is invalid or missing.\n", i);
            return MFX_ERR_INVALID_VIDEO_PARAM;
        }
    }
    return sts;
}

/* EOF */
