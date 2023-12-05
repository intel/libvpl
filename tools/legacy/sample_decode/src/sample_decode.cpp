/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "mfx_samples_config.h"

#include <regex>
#include <sstream>
#include "pipeline_decode.h"
#include "version.h"

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

// Intel® Video Processing Library (Intel® VPL)

void PrintHelp(char* strAppName, const char* strErrorMessage) {
    printf("Decoding Sample Version %s\n\n", GetMSDKSampleVersion().c_str());

    if (strErrorMessage) {
        printf("Error: %s\n", strErrorMessage);
    }

    printf("Usage: %s <codecid> [<options>] -i InputBitstream\n", strAppName);
    printf("   or: %s <codecid> [<options>] -i InputBitstream -r\n", strAppName);
    printf("   or: %s <codecid> [<options>] -i InputBitstream -o OutputYUVFile\n", strAppName);
    printf("\n");
    printf("Supported codecs (<codecid>):\n");
    printf("   <codecid>=h264|mpeg2|vc1|mvc|jpeg|vp9|av1 - built-in Media SDK codecs\n");
    printf(
        "   <codecid>=h265|vp9|capture            - in-box Media SDK plugins (may require separate downloading and installation)\n");
    printf("\n");
    printf("Work models:\n");
    printf(
        "  1. Performance model: decoding on MAX speed, no screen rendering, no YUV dumping (no -r or -o option)\n");
    printf("  2. Rendering model: decoding with rendering on the screen (-r option)\n");
    printf("  3. Dump model: decoding with YUV dumping (-o option)\n");
    printf("\n");
    printf("Options:\n");
    printf("   [-?]                      - print help\n");
    printf("   [-hw]                     - use platform specific SDK implementation (default)\n");
    printf(
        "   [-sw]                     - use software implementation, if not specified platform specific SDK implementation is used\n");
    printf(
        "                               (optional for Media SDK in-box plugins, required for user-decoder ones)\n");
    printf(
        "   [-p plugin]               - DEPRECATED: decoder plugin. Supported values: hevcd_sw, hevcd_hw, vp8d_hw, vp9d_hw, camera_hw, capture_hw\n");
    printf("   [-fps]                    - limits overall fps of pipeline\n");
    printf("   [-w]                      - output width\n");
    printf("   [-h]                      - output height\n");
    printf("   [-di bob/adi]             - enable deinterlacing BOB/ADI\n");
    printf("   [-scaling_mode value]     - specify scaling mode (lowpower/quality) for VPP\n");
    printf("   [-d]                      - enable decode error report\n");
#if defined(_WIN32) || defined(_WIN64)
    printf(
        "   [-dxgiFs]                 - enable fullscreen via IDXGISwapChain::SetFullScreenState method\n");
#endif
    printf(
        "   [-dGfx]                   - prefer processing on dGfx (by default system decides), also can be set with index, for example: '-dGfx 1'\n");
    printf(
        "   [-iGfx]                   - prefer processing on iGfx (by default system decides)\n");
    printf(
        "   [-AdapterNum]             - specifies adapter number for processing, starts from 0\n");
    printf(
        "   [-dispatcher:fullSearch]  - enable search for all available implementations in Intel® VPL dispatcher\n");
    printf(
        "   [-dispatcher:lowLatency]  - enable limited implementation search and query in Intel® VPL dispatcher\n");
#if defined(LINUX32) || defined(LINUX64)
    printf("   [-device /path/to/device] - set graphics device for processing\n");
    printf("                                 For example: '-device /dev/dri/card0'\n");
    printf("                                              '-device /dev/dri/renderD128'\n");
    printf(
        "                                 If not specified, defaults to the first Intel device found on the system\n");
#endif
#if (defined(_WIN64) || defined(_WIN32))
    printf("   [-luid HighPart:LowPart] - setup adapter by LUID  \n");
    printf("                                 For example: \"0x0:0x8a46\"  \n");
#endif
    printf("   [-pci domain:bus:device.function] - setup device with PCI \n");
    printf("                                 For example: \"0:3:0.0\"  \n");

    printf("   [-ignore_level_constrain] - ignore level constrain\n");
    printf("   [-disable_film_grain]     - disable film grain application(valid only for av1)\n");
    printf(
        "   [-api_ver_init::<1x,2x>]  - select the api version for the session initialization\n");
    printf("\n");
    printf("JPEG Chroma Type:\n");
    printf("   [-jpeg_rgb] - RGB Chroma Type\n");
    printf("Output format parameters:\n");
    printf("   [-i420] - pipeline output format: NV12, output file format: I420\n");
    printf("   [-nv12] - pipeline output format: NV12, output file format: NV12\n");
    printf("   [-rgb4] - pipeline output format: RGB4, output file format: RGB4\n");
    printf(
        "   [-rgb4_fcr] - pipeline output format: RGB4 in full color range, output file format: RGB4 in full color range\n");
    printf("   [-p010] - pipeline output format: P010, output file format: P010\n");
    printf("   [-a2rgb10] - pipeline output format: A2RGB10, output file format: A2RGB10\n");
    printf("   [-p016] - pipeline output format: P010, output file format: P016\n");
    printf("   [-y216] - pipeline output format: Y216, output file format: Y216\n");
    printf("   [-y416] - pipeline output format: Y416, output file format: Y416\n");
    printf("\n");
#if D3D_SURFACES_SUPPORT
    printf("   [-d3d]                    - work with d3d9 surfaces\n");
    printf("   [-d3d11]                  - work with d3d11 surfaces\n");
    printf("   [-r]                      - render decoded data in a separate window \n");
    printf(
        "   [-wall w h n m t tmo]     - same as -r, and positioned rendering window in a particular cell on specific monitor \n");
    printf(
        "       w                     - number of columns of video windows on selected monitor\n");
    printf("       h                     - number of rows of video windows on selected monitor\n");
    printf("       n(0,.,w*h-1)          - order of video window in table that will be rendered\n");
    printf("       m(0,1..)              - monitor id \n");
    printf("       t(0/1)                - enable/disable window's title\n");
    printf("       tmo                   - timeout for -wall option\n");
    printf("\n");
#endif
#if defined(LIBVA_SUPPORT)
    printf("   [-vaapi]                  - work with vaapi surfaces\n");
#endif
#if defined(LIBVA_X11_SUPPORT)
    printf("   [-r]                      - render decoded data in a separate X11 window \n");
#endif
#if defined(LIBVA_WAYLAND_SUPPORT)
    printf("   [-rwld]                   - render decoded data in a Wayland window \n");
    printf("   [-perf]                   - turn on asynchronous flipping for Wayland rendering \n");
#endif
#if defined(LIBVA_DRM_SUPPORT)
    printf("   [-rdrm]                   - render decoded data in a thru DRM frame buffer\n");
    printf("   [-window x y w h]         - set render window position and size\n");
#endif
    printf(
        "   [-low_latency]            - configures decoder for low latency mode (supported only for H.264 and JPEG codec)\n");
    printf(
        "   [-calc_latency]           - calculates latency during decoding and prints log (supported only for H.264 and JPEG codec)\n");
    printf(
        "   [-async]                  - depth of asynchronous pipeline. default value is 4. must be between 1 and 20\n");
    printf("   [-gpucopy::<on,off>] Enable or disable GPU copy mode\n");
    printf("   [-robust:soft]            - GPU hang recovery by inserting an IDR frame\n");
    printf("   [-timeout]                - timeout in seconds\n");
    printf("   [-dec_postproc force/auto] - resize after decoder using direct pipe\n");
    printf("                  force: instruct to use decoder-based post processing\n");
    printf("                         or fail if the decoded stream is unsupported\n");
    printf(
        "                  auto: instruct to use decoder-based post processing for supported streams \n");
    printf(
        "                        or perform VPP operation through separate pipeline component for unsupported streams\n");

#if !defined(_WIN32) && !defined(_WIN64)
    printf("   [-threads_num]            - number of mediasdk task threads\n");
    printf("   [-threads_schedtype]      - scheduling type of mediasdk task threads\n");
    printf("   [-threads_priority]       - priority of mediasdk task threads\n");
    printf("\n");
    msdk_thread_printf_scheduling_help();
#endif
#if defined(_WIN32) || defined(_WIN64)
    printf("   [-jpeg_rotate n]          - rotate jpeg frame n degrees \n");
    printf("       n(90,180,270)         - number of degrees \n");
#endif
#ifdef ONEVPL_EXPERIMENTAL
    printf("   [-cfg::dec config]    - Set decoder options via string-api\n");
    printf("   [-cfg::vpp config]    - Set VPP options via string-api\n");
#endif
    printf(
        "   [-dump fileName]         - dump MSDK components configuration to the file in text form\n");

#if defined(_WIN32) || defined(_WIN64)
    printf("\nFeatures: \n");
    printf("   Press 1 to toggle fullscreen rendering on/off\n");
#endif
    printf("\n");
    printf("Example:\n");
    printf("  %s h265 -i in.bit -o out.yuv -p 15dd936825ad475ea34e35f3f54217a6\n", strAppName);
}

mfxStatus ParseInputString(char* strInput[], mfxU32 nArgNum, sInputParams* pParams) {
    if (1 == nArgNum) {
        PrintHelp(strInput[0], NULL);
        return MFX_ERR_UNSUPPORTED;
    }

    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);

    // set default implementation
    pParams->bUseHWLib          = true;
    pParams->bUseFullColorRange = false;
    pParams->adapterType        = mfxMediaAdapterType::MFX_MEDIA_UNKNOWN;
    pParams->dGfxIdx            = -1;
    pParams->adapterNum         = -1;
    pParams->dispFullSearch     = DEF_DISP_FULLSEARCH;

#if defined(LIBVA_SUPPORT)
    pParams->libvaBackend = MFX_LIBVA_DRM;
#endif

    for (mfxU32 i = 1; i < nArgNum; i++) {
        if ('-' != strInput[i][0]) {
            mfxStatus sts = StrFormatToCodecFormatFourCC(strInput[i], pParams->videoType);
            if (sts != MFX_ERR_NONE) {
                PrintHelp(strInput[0], "Unknown codec");
                return MFX_ERR_UNSUPPORTED;
            }
            if (!IsDecodeCodecSupported(pParams->videoType)) {
                PrintHelp(strInput[0], "Unsupported codec");
                return MFX_ERR_UNSUPPORTED;
            }
            if (pParams->videoType == CODEC_MVC) {
                pParams->videoType = MFX_CODEC_AVC;
                pParams->bIsMVC    = true;
            }
            continue;
        }

        if (msdk_match(strInput[i], "-?")) {
            PrintHelp(strInput[0], "");
            return MFX_ERR_ABORTED;
        }
        else if (msdk_match(strInput[i], "-sw")) {
            pParams->bUseHWLib = false;
        }
        else if (msdk_match(strInput[i], "-hw")) {
            pParams->bUseHWLib = true;
        }
#if D3D_SURFACES_SUPPORT
        else if (msdk_match(strInput[i], "-d3d")) {
            pParams->memType          = D3D9_MEMORY;
            pParams->accelerationMode = MFX_ACCEL_MODE_VIA_D3D9;
        }
        else if (msdk_match(strInput[i], "-d3d11")) {
            pParams->memType          = D3D11_MEMORY;
            pParams->accelerationMode = MFX_ACCEL_MODE_VIA_D3D11;
        }
        else if (msdk_match(strInput[i], "-r")) {
            pParams->mode = MODE_RENDERING;
            // use d3d9 rendering by default
            if (SYSTEM_MEMORY == pParams->memType) {
                pParams->memType          = D3D9_MEMORY;
                pParams->accelerationMode = MFX_ACCEL_MODE_VIA_D3D9;
            }
        }
        else if (msdk_match(strInput[i], "-wall")) {
            if (i + 6 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -wall key");
                return MFX_ERR_UNSUPPORTED;
            }
            // use d3d9 rendering by default
            if (SYSTEM_MEMORY == pParams->memType) {
                pParams->memType          = D3D9_MEMORY;
                pParams->accelerationMode = MFX_ACCEL_MODE_VIA_D3D9;
            }

            pParams->mode = MODE_RENDERING;

            msdk_opt_read(strInput[++i], pParams->nWallW);
            msdk_opt_read(strInput[++i], pParams->nWallH);
            msdk_opt_read(strInput[++i], pParams->nWallCell);
            msdk_opt_read(strInput[++i], pParams->nWallMonitor);

            mfxU32 nTitle;
            msdk_opt_read(strInput[++i], nTitle);

            pParams->bWallNoTitle = 0 == nTitle;

            msdk_opt_read(strInput[++i], pParams->nTimeout);
        }
#endif
#if defined(LIBVA_SUPPORT)
        else if (msdk_match(strInput[i], "-vaapi")) {
            pParams->memType          = D3D9_MEMORY;
            pParams->accelerationMode = MFX_ACCEL_MODE_VIA_VAAPI;
        }
        else if (msdk_match(strInput[i], "-r")) {
            pParams->memType          = D3D9_MEMORY;
            pParams->accelerationMode = MFX_ACCEL_MODE_VIA_VAAPI;
            pParams->mode             = MODE_RENDERING;
            pParams->libvaBackend     = MFX_LIBVA_X11;
        }
        else if (msdk_match(strInput[i], "-rwld")) {
            pParams->memType          = D3D9_MEMORY;
            pParams->accelerationMode = MFX_ACCEL_MODE_VIA_VAAPI;
            pParams->mode             = MODE_RENDERING;
            pParams->libvaBackend     = MFX_LIBVA_WAYLAND;
        }
        else if (msdk_match(strInput[i], "-perf")) {
            pParams->bPerfMode = true;
        }
        else if (0 == strncmp(strInput[i], "-rdrm", 5)) {
            pParams->memType          = D3D9_MEMORY;
            pParams->accelerationMode = MFX_ACCEL_MODE_VIA_VAAPI;
            pParams->mode             = MODE_RENDERING;
            pParams->libvaBackend     = MFX_LIBVA_DRM_MODESET;
            if (strInput[i][5]) {
                if (strInput[i][5] != '-') {
                    PrintHelp(strInput[0], "unsupported monitor type");
                    return MFX_ERR_UNSUPPORTED;
                }
                pParams->monitorType = getMonitorType(&strInput[i][6]);
                if (pParams->monitorType >= MFX_MONITOR_MAXNUMBER) {
                    PrintHelp(strInput[0], "unsupported monitor type");
                    return MFX_ERR_UNSUPPORTED;
                }
            }
            else {
                pParams->monitorType = MFX_MONITOR_AUTO; // that's case of "-rdrm" pure option
            }
        }
        else if (msdk_match(strInput[i], "-window")) {
            if (i + 4 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -window key");
                return MFX_ERR_UNSUPPORTED;
            }
            msdk_opt_read(strInput[++i], pParams->nRenderWinX);
            msdk_opt_read(strInput[++i], pParams->nRenderWinY);
            msdk_opt_read(strInput[++i], pParams->Width);
            msdk_opt_read(strInput[++i], pParams->Height);

            if (0 == pParams->Width)
                pParams->Width = 320;
            if (0 == pParams->Height)
                pParams->Height = 240;

            pParams->bRenderWin = true;
        }
#endif
        else if (msdk_match(strInput[i], "-low_latency")) {
            switch (pParams->videoType) {
                case MFX_CODEC_HEVC:
                case MFX_CODEC_AVC:
                case MFX_CODEC_JPEG: {
                    pParams->bLowLat = true;
                    if (!pParams->bIsMVC)
                        break;
                }
                default: {
                    PrintHelp(strInput[0],
                              "-low_latency mode is suppoted only for H.264 and JPEG codecs");
                    return MFX_ERR_UNSUPPORTED;
                }
            }
        }
        else if (msdk_match(strInput[i], "-jpeg_rotate")) {
            if (MFX_CODEC_JPEG != pParams->videoType)
                return MFX_ERR_UNSUPPORTED;

            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -jpeg_rotate key");
                return MFX_ERR_UNSUPPORTED;
            }

            msdk_opt_read(strInput[++i], pParams->nRotation);
            if ((pParams->nRotation != 90) && (pParams->nRotation != 180) &&
                (pParams->nRotation != 270)) {
                PrintHelp(strInput[0], "-jpeg_rotate is supported only for 90, 180 and 270 angles");
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (msdk_match(strInput[i], "-calc_latency")) {
            switch (pParams->videoType) {
                case MFX_CODEC_HEVC:
                case MFX_CODEC_AVC:
                case MFX_CODEC_JPEG: {
                    pParams->bCalLat = true;
                    if (!pParams->bIsMVC)
                        break;
                }
                default: {
                    PrintHelp(strInput[0],
                              "-calc_latency mode is suppoted only for H.264 and JPEG codecs");
                    return MFX_ERR_UNSUPPORTED;
                }
            }
        }
        else if (msdk_match(strInput[i], "-async")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -async key");
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nAsyncDepth)) {
                PrintHelp(strInput[0], "async is invalid");
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (msdk_match(strInput[i], "-timeout")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -timeout key");
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nTimeout)) {
                PrintHelp(strInput[0], "timeout is invalid");
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (msdk_match(strInput[i], "-di")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -di key");
                return MFX_ERR_UNSUPPORTED;
            }
            char diMode[32] = {};
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], diMode)) {
                PrintHelp(strInput[0], "deinterlace value is not set");
                return MFX_ERR_UNSUPPORTED;
            }

            if (msdk_match(diMode, "bob")) {
                pParams->eDeinterlace = MFX_DEINTERLACING_BOB;
            }
            else if (msdk_match(diMode, "adi")) {
                pParams->eDeinterlace = MFX_DEINTERLACING_ADVANCED;
            }
            else {
                PrintHelp(strInput[0], "deinterlace value is invalid");
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (msdk_match(strInput[i], "-scaling_mode")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -scaling_mode key");
                return MFX_ERR_UNSUPPORTED;
            }
            char diMode[32] = {};
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], diMode)) {
                PrintHelp(strInput[0], "mode type is not set");
                return MFX_ERR_UNSUPPORTED;
            }
            if (msdk_match(diMode, "lowpower")) {
                pParams->ScalingMode = MFX_SCALING_MODE_LOWPOWER;
            }
            else if (msdk_match(diMode, "quality")) {
                pParams->ScalingMode = MFX_SCALING_MODE_QUALITY;
            }
            else {
                PrintHelp(strInput[0], "deinterlace value is invalid");
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (msdk_match(strInput[i], "-gpucopy::on")) {
            pParams->gpuCopy = MFX_GPUCOPY_ON;
        }
        else if (msdk_match(strInput[i], "-gpucopy::off")) {
            pParams->gpuCopy = MFX_GPUCOPY_OFF;
        }
        else if (msdk_match(strInput[i], "-robust:soft")) {
            pParams->bSoftRobustFlag = true;
        }
#if defined(LINUX32) || defined(LINUX64)
        else if (msdk_match(strInput[i], "-device")) {
            if (!pParams->strDevicePath.empty()) {
                printf("error: you can specify only one device\n");
                return MFX_ERR_UNSUPPORTED;
            }
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -device key");
                return MFX_ERR_UNSUPPORTED;
            }
            pParams->strDevicePath = strInput[++i];

            size_t pos = pParams->strDevicePath.find("renderD");
            if (pos != std::string::npos) {
                pParams->DRMRenderNodeNum = std::stoi(pParams->strDevicePath.substr(pos + 7, 3));
            }
        }
#endif

#if (defined(_WIN64) || defined(_WIN32))
        else if (msdk_match(strInput[i], "-luid")) {
            // <HighPart:LowPart>
            char luid[MSDK_MAX_FILENAME_LEN];
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -luid key");
                return MFX_ERR_UNSUPPORTED;
            }
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(strInput[i], luid)) {
                PrintHelp(strInput[0], "error: '-luid' arguments is invalid\n");
                return MFX_ERR_UNSUPPORTED;
            }

            std::string temp = std::string(luid);
            const std::regex pieces_regex("(0[xX][0-9a-fA-F]+):(0[xX][0-9a-fA-F]+)");
            std::smatch pieces_match;

            if (std::regex_match(temp, pieces_match, pieces_regex) && pieces_match.size() == 3) {
                pParams->luid.HighPart = std::strtol(pieces_match[1].str().c_str(), 0, 16);
                pParams->luid.LowPart  = std::strtol(pieces_match[2].str().c_str(), 0, 16);
            }
            else {
                PrintHelp(
                    strInput[0],
                    "error: '-luid' arguments is invalid, please, use format: HighPart:LowPart\n");
                return MFX_ERR_UNSUPPORTED;
            }
        }
#endif
        else if (msdk_match(strInput[i], "-pci")) {
            char deviceInfo[MSDK_MAX_FILENAME_LEN];
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -pci key");
                return MFX_ERR_UNSUPPORTED;
            }
            i++;
            if ((strlen(strInput[i]) + 1) > MSDK_ARRAY_LEN(deviceInfo)) {
                PrintHelp(strInput[0], "error: '-pci' arguments is too long\n");
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[i], deviceInfo)) {
                PrintHelp(strInput[0], "error: '-pci' arguments is invalid\n");
                return MFX_ERR_UNSUPPORTED;
            }

            // template: <domain:bus:device.function>
            std::string temp = std::string(deviceInfo);
            const std::regex pieces_regex("([0-9]+):([0-9]+):([0-9]+).([0-9]+)");
            std::smatch pieces_match;

            if (std::regex_match(temp, pieces_match, pieces_regex) && pieces_match.size() == 5) {
                pParams->PCIDomain      = std::atoi(pieces_match[1].str().c_str());
                pParams->PCIBus         = std::atoi(pieces_match[2].str().c_str());
                pParams->PCIDevice      = std::atoi(pieces_match[3].str().c_str());
                pParams->PCIFunction    = std::atoi(pieces_match[4].str().c_str());
                pParams->PCIDeviceSetup = true;
            }
            else {
                PrintHelp(strInput[0],
                          "format of -pci is invalid, please, use: domain:bus:device.function");
                return MFX_ERR_UNSUPPORTED;
            }
        }

        else if (msdk_match(strInput[i], "-dGfx")) {
            pParams->adapterType = mfxMediaAdapterType::MFX_MEDIA_DISCRETE;
            if (i + 1 < nArgNum && isdigit(*strInput[1 + i])) {
                if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->dGfxIdx)) {
                    PrintHelp(strInput[0], "value of -dGfx is invalid");
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
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -AdapterNum key");
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->adapterNum)) {
                PrintHelp(strInput[0], "AdapterNum is invalid");
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (msdk_match(strInput[i], "-dispatcher:fullSearch")) {
            pParams->dispFullSearch = true;
        }
        else if (msdk_match(strInput[i], "-dispatcher:lowLatency")) {
            pParams->dispFullSearch = false;
        }
        else if (msdk_match(strInput[i], "-dxgiFs")) {
#if defined(_WIN32) || defined(_WIN64)
            pParams->bDxgiFs = true;
#endif
        }
#if !defined(_WIN32) && !defined(_WIN64)
        else if (msdk_match(strInput[i], "-d")) {
            pParams->bErrorReport = true;
        }
        else if (msdk_match(strInput[i], "-threads_num")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -threads_num key");
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nThreadsNum)) {
                PrintHelp(strInput[0], "threads_num is invalid");
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (msdk_match(strInput[i], "-threads_schedtype")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -threads_schedtype key");
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_thread_get_schedtype(strInput[++i], pParams->SchedulingType)) {
                PrintHelp(strInput[0], "threads_schedtype is invalid");
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (msdk_match(strInput[i], "-threads_priority")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -threads_priority key");
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->Priority)) {
                PrintHelp(strInput[0], "threads_priority is invalid");
                return MFX_ERR_UNSUPPORTED;
            }
        }
#endif // #if !defined(_WIN32) && !defined(_WIN64)
        else if (msdk_match(strInput[i], "-dec_postproc")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(
                    strInput[0],
                    "Not enough parameters for \"-dec_postproc\", right is  \"--dec_postproc force//auto\"");
                return MFX_ERR_UNSUPPORTED;
            }
            char postProcMode[32] = {};
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], postProcMode)) {
                PrintHelp(strInput[0], "dec_postproc value is not set");
                return MFX_ERR_UNSUPPORTED;
            }
            if (msdk_match(postProcMode, "auto")) {
                pParams->nDecoderPostProcessing = MODE_DECODER_POSTPROC_AUTO;
            }
            else if (msdk_match(postProcMode, "force")) {
                pParams->nDecoderPostProcessing = MODE_DECODER_POSTPROC_FORCE;
            }
            else {
                PrintHelp(strInput[0], "dec_postproc is invalid");
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (msdk_match(strInput[i], "-fps")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -fps key");
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nMaxFPS)) {
                PrintHelp(strInput[0], "overall fps is invalid");
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (msdk_match(strInput[i], "-w")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -w key");
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->Width)) {
                PrintHelp(strInput[0], "width is invalid");
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (msdk_match(strInput[i], "-h")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -h key");
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->Height)) {
                PrintHelp(strInput[0], "height is invalid");
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (msdk_match(strInput[i], "-n")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -n key");
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nFrames)) {
                PrintHelp(strInput[0], "rendering frame rate is invalid");
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (msdk_match(strInput[i], "-jpeg_rgb")) {
            if (MFX_CODEC_JPEG == pParams->videoType) {
                pParams->chromaType = MFX_JPEG_COLORFORMAT_RGB;
            }
        }
        else if (msdk_match(strInput[i], "-i420")) {
            pParams->fourcc  = MFX_FOURCC_NV12;
            pParams->outI420 = true;
        }
        else if (msdk_match(strInput[i], "-nv12")) {
            pParams->fourcc = MFX_FOURCC_NV12;
        }
        else if (msdk_match(strInput[i], "-rgb4")) {
            pParams->fourcc = MFX_FOURCC_RGB4;
        }
        else if (msdk_match(strInput[i], "-ayuv")) {
            pParams->fourcc = MFX_FOURCC_AYUV;
        }
        else if (msdk_match(strInput[i], "-yuy2")) {
            pParams->fourcc = MFX_FOURCC_YUY2;
        }
        else if (msdk_match(strInput[i], "-uyvy")) {
            pParams->fourcc = MFX_FOURCC_UYVY;
        }
        else if (msdk_match(strInput[i], "-rgb4_fcr")) {
            pParams->fourcc             = MFX_FOURCC_RGB4;
            pParams->bUseFullColorRange = true;
        }
        else if (msdk_match(strInput[i], "-i010")) {
            pParams->fourcc = MFX_FOURCC_I010;
        }
        else if (msdk_match(strInput[i], "-p010")) {
            pParams->fourcc = MFX_FOURCC_P010;
        }
        else if (msdk_match(strInput[i], "-a2rgb10")) {
            pParams->fourcc = MFX_FOURCC_A2RGB10;
        }
        else if (msdk_match(strInput[i], "-p016")) {
            pParams->fourcc = MFX_FOURCC_P016;
        }
        else if (msdk_match(strInput[i], "-y216")) {
            pParams->fourcc = MFX_FOURCC_Y216;
        }
        else if (msdk_match(strInput[i], "-y416")) {
            pParams->fourcc = MFX_FOURCC_Y416;
        }
        else if (msdk_match(strInput[i], "-i:null")) {
            ;
        }
        else if (msdk_match(strInput[i], "-ignore_level_constrain")) {
            pParams->bIgnoreLevelConstrain = true;
        }
        else if (msdk_match(strInput[i], "-disable_film_grain")) {
            pParams->bDisableFilmGrain = true;
        }
        else if (msdk_match(strInput[i], "-api_ver_init::1x")) {
            pParams->verSessionInit = API_1X;
        }
        else if (msdk_match(strInput[i], "-api_ver_init::2x")) {
            pParams->verSessionInit = API_2X;
        }
#ifdef ONEVPL_EXPERIMENTAL
        else if (msdk_match(strInput[i], "-cfg::dec")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -cfg::dec");
                return MFX_ERR_UNSUPPORTED;
            }
            i++;
            pParams->m_decode_cfg = strInput[i];
        }
        else if (msdk_match(strInput[i], "-cfg::vpp")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0], "Not enough parameters for -cfg::vpp");
                return MFX_ERR_UNSUPPORTED;
            }
            i++;
            pParams->m_vpp_cfg = strInput[i];
        }
#endif
        else if (msdk_match(strInput[i], "-dump")) {
            if (i + 1 >= nArgNum) {
                PrintHelp(strInput[0],
                          "File Name for dumping MSDK library configuration should be provided");
                return MFX_ERR_UNSUPPORTED;
            }
            i++;
            pParams->dump_file = strInput[i];
        }
        else // 1-character options
        {
            switch (strInput[i][1]) {
                case 'p':
                    ++i;
                    printf("WARNING: plugins are deprecated and not supported by Intel® VPL RT \n");
                    break;
                case 'i':
                    if (++i < nArgNum) {
                        msdk_opt_read(strInput[i], pParams->strSrcFile);
                    }
                    else {
                        printf("error: option '-i' expects an argument\n");
                    }
                    break;
                case 'o':
                    if (++i < nArgNum) {
                        pParams->mode = MODE_FILE_DUMP;
                        msdk_opt_read(strInput[i], pParams->strDstFile);
                    }
                    else {
                        printf("error: option '-o' expects an argument\n");
                    }
                    break;
                case '?':
                    PrintHelp(strInput[0], NULL);
                    return MFX_ERR_UNSUPPORTED;
                default: {
                    std::stringstream stream;
                    stream << "Unknown option: " << strInput[i];
                    PrintHelp(strInput[0], stream.str().c_str());
                    return MFX_ERR_UNSUPPORTED;
                }
            }
        }
    }

    if (0 == strlen(pParams->strSrcFile)) {
        printf("error: source file name not found");
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->mode == MODE_FILE_DUMP) && (0 == strlen(pParams->strDstFile))) {
        printf("error: destination file name not found");
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_CODEC_MPEG2 != pParams->videoType && MFX_CODEC_AVC != pParams->videoType &&
        MFX_CODEC_HEVC != pParams->videoType && MFX_CODEC_VC1 != pParams->videoType &&
        MFX_CODEC_JPEG != pParams->videoType && MFX_CODEC_VP8 != pParams->videoType &&
        MFX_CODEC_VP9 != pParams->videoType && MFX_CODEC_AV1 != pParams->videoType) {
        PrintHelp(strInput[0], "Unknown codec");
        return MFX_ERR_UNSUPPORTED;
    }

    if (pParams->nAsyncDepth == 0) {
        pParams->nAsyncDepth = 4; //set by default;
    }

    if (!pParams->bUseHWLib) { // Intel® VPL cpu plugin
        pParams->nAsyncDepth = 1;
    }

#if (defined(_WIN64) || defined(_WIN32)) && (MFX_VERSION >= 1031)
    if (pParams->bPreferdGfx && pParams->bPreferiGfx) {
        printf("Warning: both dGfx and iGfx flags set. iGfx will be preferred");
        pParams->bPreferdGfx = false;
    }
#endif

    return MFX_ERR_NONE;
}

int main(int argc, char* argv[]) {
    sInputParams Params = {}; // input parameters from command line
    CDecodingPipeline
        Pipeline; // pipeline for decoding, includes input file reader, decoder and output file writer

    mfxStatus sts = MFX_ERR_NONE; // return value check

    sts = ParseInputString(argv, (mfxU8)argc, &Params);
    if (sts == MFX_ERR_ABORTED) {
        // No error, just need to close app normally
        return MFX_ERR_NONE;
    }
    MSDK_CHECK_PARSE_RESULT(sts, MFX_ERR_NONE, 1);

    // if version is >= 2000, sw lib is Intel® VPL
    // if outI420 is true, it means sample will convert decode output to I420, which is useless in Intel® VPL.
    // we set foucc to I420 back and set outI420 to false
    if (Params.bUseHWLib == false && Params.outI420 == true) {
        Params.fourcc  = MFX_FOURCC_I420;
        Params.outI420 = false;
    }
    sts = Pipeline.Init(&Params);
    MSDK_CHECK_STATUS(sts, "Pipeline.Init failed");

    // print library info
    if (Params.verSessionInit != API_1X) {
        Pipeline.PrintLibInfo();
    }

    // print stream info
    Pipeline.PrintStreamInfo();

    printf("Decoding started\n");

    mfxU64 prevResetBytesCount = 0xFFFFFFFFFFFFFFFF;
    for (;;) {
        sts = Pipeline.RunDecoding();

        if (MFX_ERR_INCOMPATIBLE_VIDEO_PARAM == sts || MFX_ERR_DEVICE_LOST == sts ||
            MFX_ERR_DEVICE_FAILED == sts) {
            if (prevResetBytesCount == Pipeline.GetTotalBytesProcessed()) {
                printf(
                    "\nERROR: No input data was consumed since last reset. Quitting to avoid looping forever.\n");
                break;
            }
            prevResetBytesCount = Pipeline.GetTotalBytesProcessed();

            if (MFX_ERR_INCOMPATIBLE_VIDEO_PARAM == sts) {
                printf("\nERROR: Incompatible video parameters detected. Recovering...\n");
            }
            else {
                printf(
                    "\nERROR: Hardware device was lost or returned unexpected error. Recovering...\n");
                sts = Pipeline.ResetDevice();
                MSDK_CHECK_STATUS(sts, "Pipeline.ResetDevice failed");
            }

            sts = Pipeline.ResetDecoder(&Params);
            MSDK_CHECK_STATUS(sts, "Pipeline.ResetDecoder failed");
            continue;
        }
        else {
            MSDK_CHECK_STATUS(sts, "Pipeline.RunDecoding failed");
            break;
        }
    }

    printf("\nDecoding finished\n");

    return 0;
}
