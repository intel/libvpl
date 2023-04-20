/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <regex>
#include "mfx_samples_config.h"
#include "plugin_utils.h"
#include "sample_defs.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <d3d9.h>
    #include <psapi.h>
    #include <windows.h>
    #include "d3d11_allocator.h"
    #include "d3d_allocator.h"

#else
    #include <stdarg.h>
    #include "vaapi_allocator.h"
#endif

#include "smt_cli.h"
#include "sysmem_allocator.h"

#include "version.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace TranscodingSample;

// parsing defines
#define IS_SEPARATOR(ch) ((ch) <= ' ' || (ch) == '=')
#define VAL_CHECK(val, argIdx, argName)                                                        \
    {                                                                                          \
        if (val) {                                                                             \
            PrintError(MSDK_STRING("Input argument number %d \"%s\" require more parameters"), \
                       argIdx,                                                                 \
                       argName);                                                               \
            return MFX_ERR_UNSUPPORTED;                                                        \
        }                                                                                      \
    }

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

msdk_tick TranscodingSample::GetTick() {
    return msdk_time_get_tick();
}

mfxF64 TranscodingSample::GetTime(msdk_tick start) {
    static msdk_tick frequency = msdk_time_get_frequency();

    return MSDK_GET_TIME(msdk_time_get_tick(), start, frequency);
}

#define HELP_LINE(TEXT) msdk_printf(MSDK_STRING("%s\n"), MSDK_STRING(TEXT))

size_t split(const msdk_string& source,
             std::vector<msdk_string>& dest,
             msdk_char delim = msdk_char(' ')) {
    size_t items = 0;
    msdk_string item;
    msdk_stringstream source_stream(source);
    while (getline(source_stream, item, delim)) {
        items += 1;
        dest.push_back(item);
    }
    return items;
}

void TranscodingSample::PrintError(const msdk_char* strErrorMessage, ...) {
    if (strErrorMessage) {
        msdk_printf(MSDK_STRING("ERROR: "));
        va_list args;
        va_start(args, strErrorMessage);
        msdk_vprintf(strErrorMessage, args);
        va_end(args);
        HELP_LINE("");
        HELP_LINE("Usage: sample_multi_transcode [options] [--] pipeline-description");
        HELP_LINE("   or: sample_multi_transcode [options] -par ParFile");
        HELP_LINE("");
        HELP_LINE("Run application with -? option to get full help text.");
        HELP_LINE("");
    }
}

void TranscodingSample::PrintHelp() {
    msdk_printf(MSDK_STRING("Multi Transcoding Sample Version %s\n"),
                GetMSDKSampleVersion().c_str());
    HELP_LINE("");
    HELP_LINE("Command line parameters");
    HELP_LINE("");
    HELP_LINE("Usage: sample_multi_transcode [options] [--] pipeline-description");
    HELP_LINE("   or: sample_multi_transcode [options] -par ParFile");
    HELP_LINE("");
    HELP_LINE("");
    HELP_LINE("  -stat <N>");
    HELP_LINE("                Output statistic every N transcoding cycles");
    HELP_LINE("");
    HELP_LINE("  -stat-log <name>");
    HELP_LINE("                Output statistic to the specified file (opened in append mode)");
    HELP_LINE("");
    HELP_LINE("  -stat-per-frame <name>");
    HELP_LINE("                Output per-frame latency values to a file (opened in append mode).");
    HELP_LINE("                The file name will be for an input session:");
    HELP_LINE("                <name>_input_ID_<N>.log");
    HELP_LINE("                or, for output session: <name>_output_ID_<N>.log;");
    HELP_LINE("                <N> - a number of a session.");
    HELP_LINE("Options:");
    HELP_LINE("");
    HELP_LINE("  -?            Print this help and exit");
    HELP_LINE("");
    HELP_LINE("  -p <file-name>");
    HELP_LINE("                Collect performance statistics in specified file");
    HELP_LINE("");
    HELP_LINE("  -timeout <seconds>");
    HELP_LINE("                Set time to run transcoding in seconds");
    HELP_LINE("");
    HELP_LINE("  -greedy");
    HELP_LINE("                Use greedy formula to calculate number of surfaces");
    HELP_LINE("");
    HELP_LINE("Pipeline description (general options):");
    HELP_LINE("");
    HELP_LINE("  -i::<h265|h264|mpeg2|vc1|mvc|jpeg|vp9|av1> <file-name>");
    HELP_LINE("                Set input file and decoder type");
    HELP_LINE("");
    HELP_LINE("  -i::<i420|nv12|p010> <file-name>");
    HELP_LINE("                Set raw input file and color format");
    HELP_LINE("");
    HELP_LINE("  -i::rgb4_frame Set input rgb4 file for composition.");
    HELP_LINE("                File should contain just one single frame");
    HELP_LINE("                (-vpp_comp_src_h and -vpp_comp_src_w should");
    HELP_LINE("                be specified as well).");
    HELP_LINE("");
    HELP_LINE("  -msb10 - 10-bit color format is expected to have data in Most");
    HELP_LINE("                Significant Bits of words.");
    HELP_LINE("                LSB data placement is expected by default.");
    HELP_LINE("");
    HELP_LINE("  -o::<h265|h264|mpeg2|mvc|jpeg|vp9|av1|raw> <file-name>|null");
    HELP_LINE("                Set output file and encoder type");
    HELP_LINE("                'null' keyword as file-name disables output file writing");
    HELP_LINE("");
    HELP_LINE("  -sw|-hw|-hw_d3d11|-hw_d3d9");
    HELP_LINE("                SDK implementation to use:");
    HELP_LINE("                    -hw - platform-specific on default display adapter (default)");
    HELP_LINE("                    -hw_d3d11 - platform-specific via d3d11");
    HELP_LINE("                                (d3d11 is default for win)");
    HELP_LINE("                    -hw_d3d9 - platform-specific via d3d9");
    HELP_LINE("                    -sw - software");
#if defined(LINUX32) || defined(LINUX64)
    HELP_LINE("");
    HELP_LINE("  -device <path to device>");
    HELP_LINE("                set graphics device for processing");
    HELP_LINE("                For example: '-device /dev/dri/card0'");
    HELP_LINE("                             '-device /dev/dri/renderD128'");
    HELP_LINE("                If not specified, defaults to the first Intel");
    HELP_LINE("                device found on the system");
#endif
#if (defined(_WIN64) || defined(_WIN32))
    HELP_LINE("");
    HELP_LINE("  -dual_gfx::<on,off,adaptive>");
    HELP_LINE("                prefer encode processing on both iGfx and dGfx simultaneously");
#endif
#ifdef ONEVPL_EXPERIMENTAL
    #if (defined(_WIN64) || defined(_WIN32))
    HELP_LINE("");
    HELP_LINE("  -luid <HighPart:LowPart>");
    HELP_LINE("                setup adapter by LUID");
    HELP_LINE("                For example: \"0x0:0x8a46\"");
    #endif
    HELP_LINE("");
    HELP_LINE("  -pci <domain:bus:device.function>");
    HELP_LINE("                setup device with PCI");
    HELP_LINE("                For example: \"0:3:0.0\"");
#endif
    HELP_LINE("");
    HELP_LINE("  -dGfx         prefer processing on dGfx (by default system decides),");
    HELP_LINE("                also can be set with index,");
    HELP_LINE("                For example: '-dGfx 1'");
    HELP_LINE("");
    HELP_LINE("  -iGfx         prefer processing on iGfx (by default system decides)");
    HELP_LINE("");
    HELP_LINE("  -AdapterNum   specifies adapter number for processing, starts from 0");
    HELP_LINE("");
    HELP_LINE("  -dispatcher:fullSearch");
    HELP_LINE("                enable search for all available implementations in");
    HELP_LINE("                oneVPL dispatcher");
    HELP_LINE("");
    HELP_LINE("  -dispatcher:lowLatency");
    HELP_LINE("                enable limited implementation search and query in");
    HELP_LINE("                oneVPL dispatcher");
    HELP_LINE("");
    HELP_LINE("  -mfe_frames <N>");
    HELP_LINE("                maximum number of frames to be combined in multi-frame");
    HELP_LINE("                encode pipeline");
    HELP_LINE("                0 - default for platform will be used");
    HELP_LINE("");
    HELP_LINE("  -mfe_mode <0|1|2|3>");
    HELP_LINE("                multi-frame encode operation mode - should be the same");
    HELP_LINE("                for all sessions");
    HELP_LINE("                0 - MFE operates as DEFAULT mode, decided by SDK if MFE enabled");
    HELP_LINE("                1 - MFE is disabled");
    HELP_LINE("                2 - MFE operates as AUTO mode");
    HELP_LINE("                3 - MFE operates as MANUAL mode");
    HELP_LINE("");
    HELP_LINE("  -mfe_timeout <N>");
    HELP_LINE("                multi-frame encode timeout in milliseconds");
    HELP_LINE("                set per sessions control");
    HELP_LINE("");
    HELP_LINE("  -HdrSEI:mdcv <mdcv array>");
    HELP_LINE("                set mastering display colour volume for HDR SEI metadata.");
    HELP_LINE("                totally 10 values (DPX[0],DPX[1],DPX[2],DPY[0],DPY[1],DPY[2],");
    HELP_LINE("                                   WPX,WPY,MaxDML,MinDML)");
    HELP_LINE("                for mdcv parameter to set, separated by comma.");
    HELP_LINE("                For example: -HdrSEI:mdcv 13250,7500,34000,34500,3000,16000,");
    HELP_LINE("                                          15635,16450,12000000,200");
    HELP_LINE("");
    HELP_LINE("  -HdrSEI:clli <clli array>");
    HELP_LINE("                set content light level for HDR SEI metadata.");
    HELP_LINE("                totally 2 values (MaxCLL, MaxPALL) to set, separated by comma.");
    HELP_LINE("                for example: -HdrSEI:clli 1000,40");
    HELP_LINE("");
    HELP_LINE("  -SignalInfoIn <signal_info array>");
    HELP_LINE("                set input video signal info.");
    HELP_LINE("                totally 2 values (VideoFullRange, ColourPrimaries) to set,");
    HELP_LINE("                separated by comma.");
    HELP_LINE("                for example: -SignalInfoIn 0,9");
    HELP_LINE("");
    HELP_LINE("  -SignalInfoOut <signal_info array>");
    HELP_LINE("                set output video signal info.");
    HELP_LINE("                totally 2 values (VideoFullRange, ColourPrimaries) to set,");
    HELP_LINE("                separated by comma.");
    HELP_LINE("                for example: -SignalInfoOut 0,9");
#ifdef ENABLE_MCTF
    #if !defined ENABLE_MCTF_EXT
    HELP_LINE("");
    HELP_LINE("  -mctf [Strength]");
    HELP_LINE("                Strength is an optional value;  it is in range [0...20]");
    HELP_LINE("                value 0 makes MCTF operates in auto mode;");
    HELP_LINE("                Strength: integer, [0...20]. Default value is 0.");
    HELP_LINE("                Might be a CSV filename (upto 15 symbols);");
    HELP_LINE("                if a string is convertible to an integer, integer has a");
    HELP_LINE("                priority over filename");
    HELP_LINE("                In fixed-strength mode, MCTF strength can be adjusted");
    HELP_LINE("                at framelevel;");
    HELP_LINE("                If no Strength is given, MCTF operates in auto mode.");
    #else
    HELP_LINE("");
    HELP_LINE("  -mctf <MctfMode>:<BitsPerPixel>:<Strength>:<ME>:<Overlap>:<DB>");
    HELP_LINE("                every parameter may be missed; in this case default value is used.");
    HELP_LINE("                MctfMode:");
    HELP_LINE("                    0 - spatial filter");
    HELP_LINE("                    1 - temporal filtering, 1 backward reference");
    HELP_LINE("                    2 - temporal filtering, 1 backward & 1 forward reference");
    HELP_LINE("                    3 - temporal filtering, 2 backward & 2 forward references");
    HELP_LINE("                    other values: force default mode to be used");
    HELP_LINE("                BitsPerPixel: float, valid range [0...12.0]; if exists, is used");
    HELP_LINE("                              for automatic filter strength adaptation.");
    HELP_LINE("                              Default is 0.0");
    HELP_LINE("                Strength: integer, [0...20]. Default value is 0.Might be a CSV");
    HELP_LINE("                          filename (upto 15 symbols);");
    HELP_LINE("                          if a string is convertible to an integer, integer");
    HELP_LINE("                          has a priority over filename");
    HELP_LINE("                ME: Motion Estimation precision;");
    HELP_LINE("                    0 - integer ME (default)");
    HELP_LINE("                    1 - quarter-pel ME");
    HELP_LINE("                Overlap:");
    HELP_LINE("                    0 - do not apply overlap ME (default)");
    HELP_LINE("                    1 - to apply overlap ME");
    HELP_LINE("                DB:");
    HELP_LINE("                    0 - do not apply deblock Filter (default);");
    HELP_LINE("                    1 - to apply Deblock Filter");
    #endif //ENABLE_MCTF_EXT
#endif //ENABLE_MCTF

    HELP_LINE("");
    HELP_LINE("  -robust       Recover from gpu hang errors as they come");
    HELP_LINE("                (by resetting components)");
    HELP_LINE("");
    HELP_LINE("  -robust:soft  Recover from gpu hang errors by inserting an IDR");
    HELP_LINE("");
    HELP_LINE("  -async        Depth of asynchronous pipeline. default value 1");
    HELP_LINE("");
    HELP_LINE("  -join         Join session with other session(s),");
    HELP_LINE("                by default sessions are not joined");
    HELP_LINE("");
    HELP_LINE("  -priority     Use priority for join sessions.");
    HELP_LINE("                  0 - Low");
    HELP_LINE("                  1 - Normal");
    HELP_LINE("                  2 - High.");
    HELP_LINE("                Normal by default");
    HELP_LINE("");
    HELP_LINE("  -threads num  Number of session internal threads to create");
    HELP_LINE("");
    HELP_LINE("  -n            Number of frames to transcode");
    HELP_LINE("                (session ends after this number of frames is reached).");
    HELP_LINE("                In decoding sessions (-o::sink) this parameter limits");
    HELP_LINE("                number of frames acquired from decoder.");
    HELP_LINE("                In encoding sessions (-o::source) and transcoding sessions");
    HELP_LINE("                this parameter limits number of frames sent to encoder.");
    HELP_LINE("");
    HELP_LINE("  -MemType::video");
    HELP_LINE("                Force usage of external video allocator (default)");
    HELP_LINE("");
    HELP_LINE("  -ext_allocator");
    HELP_LINE("                Force usage of external video allocator (default)");
    HELP_LINE("");
    HELP_LINE("  -MemType::system");
    HELP_LINE("                Force usage of external system allocator");
    HELP_LINE("");
    HELP_LINE("  -sys          Force usage of external system allocator");
    HELP_LINE("");
    HELP_LINE("  -MemType::opaque");
    HELP_LINE("                Force usage of internal allocator");
    HELP_LINE("");
    HELP_LINE("  -MemModel::GeneralAlloc (default)");
    HELP_LINE("                Force usage of:");
    HELP_LINE("                    External allocator in the case of video/system memory type");
    HELP_LINE("                    Internal allocator in the case of opaque memory type");
    HELP_LINE("");
    HELP_LINE("  -MemModel::VisibleIntAlloc");
    HELP_LINE("                Force usage of internal allocation with manual surfaces control");
    HELP_LINE("");
    HELP_LINE("  -MemModel::HiddenIntAlloc");
    HELP_LINE("                Force usage of internal allocation without manual surfaces control");
    HELP_LINE("");
    HELP_LINE("  -memory <1|2|3>");
    HELP_LINE("                1 - GeneralAlloc(default)");
    HELP_LINE("                2 - VisibleIntAlloc");
    HELP_LINE("                3 - HiddenIntAlloc");
    HELP_LINE("");
    HELP_LINE("  -AllocPolicy::optimal");
    HELP_LINE("                Force optimal allocation policy for surface pool");
    HELP_LINE("");
    HELP_LINE("  -AllocPolicy::limited");
    HELP_LINE("                Force limited allocation policy for surface pool");
    HELP_LINE("");
    HELP_LINE("  -AllocPolicy::unlimited");
    HELP_LINE("                Force unlimited allocation policy for surface pool");
    HELP_LINE("");
    HELP_LINE("  -preallocate <number of surfaces>");
    HELP_LINE("                Set numebr of surfaces to preallocate for -AllocPolicy::limited");
    HELP_LINE("");
    HELP_LINE("  -dec::sys     Set dec output to system memory");
    HELP_LINE("");
    HELP_LINE("  -vpp::sys     Set vpp output to system memory");
    HELP_LINE("");
    HELP_LINE("  -vpp::vid     Set vpp output to video memory");
    HELP_LINE("");
    HELP_LINE("  -fps <frames per second>");
    HELP_LINE("                Transcoding frame rate limit");
    HELP_LINE("");
    HELP_LINE("  -pe           Set encoding plugin for this particular session.");
    HELP_LINE("                This setting overrides plugin settings defined by SET clause.");
    HELP_LINE("");
    HELP_LINE("  -pd           Set decoding plugin for this particular session.");
    HELP_LINE("                This setting overrides plugin settings defined by SET clause.");
    HELP_LINE("                Supported values: hevcd_sw, hevcd_hw, hevce_sw, hevce_gacc,");
    HELP_LINE("                hevce_hw, vp8d_hw,vp8e_hw, vp9d_hw, vp9e_hw, camera_hw,");
    HELP_LINE("                capture_hw, h264_la_hw, ptir_hw, hevce_fei_hw");
    HELP_LINE("                Direct GUID number can be used as well");
    HELP_LINE("");
    HELP_LINE("  -api_ver_init::<1x|2x>");
    HELP_LINE("                Select the api version for the session initialization");
    HELP_LINE("");
    HELP_LINE("Pipeline description (encoding options):");
    MOD_SMT_PRINT_HELP;
    HELP_LINE("");
    HELP_LINE("  -b <Kbits per second>");
    HELP_LINE("                Encoded bit rate, valid for H.264, MPEG2 and MVC encoders");
    HELP_LINE("");
    HELP_LINE("  -bm           Bitrate multiplier. Use it when required bitrate isn't");
    HELP_LINE("                fit into 16-bit. Affects following parameters:");
    HELP_LINE("                    InitialDelayInKB, BufferSizeInKB, TargetKbps, MaxKbps");
    HELP_LINE("");
    HELP_LINE("  -f <frames per second>");
    HELP_LINE("                Video frame rate for the FRC and deinterlace options");
    HELP_LINE("");
    HELP_LINE("  -fe <frames per second>");
    HELP_LINE("                Video frame rate for the FRC and deinterlace options");
    HELP_LINE("                (deprecated, will be removed in next versions).");
    HELP_LINE("");
    HELP_LINE("  -override_decoder_framerate <framerate>");
    HELP_LINE("                Forces decoder output framerate to be set to");
    HELP_LINE("                provided value (overwriting actual framerate from");
    HELP_LINE("                decoder)");
    HELP_LINE("");
    HELP_LINE("  -override_encoder_framerate <framerate>");
    HELP_LINE("                Overwrites framerate of stream going into encoder input with");
    HELP_LINE("                provided value (this option does not enable FRC, it just");
    HELP_LINE("                overwrites framerate value)");
    HELP_LINE("");
    HELP_LINE("  -override_encoder_picstruct <picstruct>");
    HELP_LINE("                Overwrites encoder picstruct with specific value");
    HELP_LINE("");
    HELP_LINE("  -u <usage>    Target usage. Valid for H.265, H.264, MPEG2 and MVC encoders.");
    HELP_LINE("                Expected values:");
    HELP_LINE("                    veryslow(quality)");
    HELP_LINE("                    slower");
    HELP_LINE("                    slow");
    HELP_LINE("                    medium(balanced)");
    HELP_LINE("                    fast");
    HELP_LINE("                    faster");
    HELP_LINE("                    veryfast(speed)");
    HELP_LINE("");
    HELP_LINE("  -q <quality>  Quality parameter for JPEG encoder;");
    HELP_LINE("                in range [1,100], 100 is the best quality");
    HELP_LINE("");
    HELP_LINE("  -l numSlices  Number of slices for encoder; default value 0");
    HELP_LINE("");
    HELP_LINE("  -mss maxSliceSize");
    HELP_LINE("                Maximum slice size in bytes.");
    HELP_LINE("                Supported only with -hw and h264 codec.");
    HELP_LINE("                This option is not compatible with -l option.");
    HELP_LINE("");
    HELP_LINE("  -BitrateLimit:<on|off>");
    HELP_LINE("                Turn this flag ON to set bitrate limitations imposed");
    HELP_LINE("                by the SDK encoder. Off by default.");
    HELP_LINE("");
    HELP_LINE("  -la           Use the look ahead bitrate control algorithm (LA BRC)");
    HELP_LINE("                for H.264 encoder. Supported only with");
    HELP_LINE("                -hw option on 4th Generation Intel Core processors.");
    HELP_LINE("");
    HELP_LINE("  -lad depth    Depth parameter for the LA BRC, the number of frames to be");
    HELP_LINE("                analyzed before encoding.");
    HELP_LINE("                In range [0,100] (0 - default: auto-select by mediasdk library).");
    HELP_LINE("                May be 1 in the case when -mss option is specified");
    HELP_LINE("");
    HELP_LINE("  -la_ext       Use external LA plugin (compatible with h264 & hevc encoders)");
    HELP_LINE("");
    HELP_LINE("  -vbr          Variable bitrate control");
    HELP_LINE("");
    HELP_LINE("  -cbr          Constant bitrate control");
    HELP_LINE("");
    HELP_LINE("  -vcm          Video Conferencing Mode (VCM) bitrate control");
    HELP_LINE("");
    HELP_LINE("  -hrd <KBytes> Maximum possible size of any compressed frames");
    HELP_LINE("");
    HELP_LINE("  -wb <Kbits per second>");
    HELP_LINE("                Maximum bitrate for sliding window");
    HELP_LINE("");
    HELP_LINE("  -ws           Sliding window size in frames");
    HELP_LINE("");
    HELP_LINE("  -gop_size     Size of GOP structure in frames");
    HELP_LINE("");
    HELP_LINE("  -dist         Distance between I- or P- key frames");
    HELP_LINE("");
    HELP_LINE("  -num_ref      Number of reference frames");
    HELP_LINE("");
    HELP_LINE("  -bref         Arrange B frames in B pyramid reference structure");
    HELP_LINE("");
    HELP_LINE("  -nobref       Do not use B-pyramid (by default the decision is made by library)");
    HELP_LINE("");
    HELP_LINE("  -bpyr         Enable B pyramid");
    HELP_LINE("");
    HELP_LINE("  -NumActiveRefP");
    HELP_LINE("                Number of active reference frames for P Frames");
    HELP_LINE("");
    HELP_LINE("  -gpb:<on|off>");
    HELP_LINE("                Enable or disable Generalized P/B frames");
    HELP_LINE("");
    HELP_LINE("  -TransformSkip:<on|off>");
    HELP_LINE("                Enable or disable TransformSkip");
    HELP_LINE("");
    HELP_LINE("  -trows <rows> Number of rows for tiled encoding");
    HELP_LINE("");
    HELP_LINE("  -tcols <cols> Number of columns for tiled encoding");
    HELP_LINE("");
    HELP_LINE("  -CodecProfile Specifies codec profile");
    HELP_LINE("");
    HELP_LINE("  -CodecLevel   Specifies codec level");
    HELP_LINE("");
    HELP_LINE("  -GopOptFlag:closed");
    HELP_LINE("                Closed gop");
    HELP_LINE("");
    HELP_LINE("  -GopOptFlag:strict");
    HELP_LINE("                Strict gop");
    HELP_LINE("");
    HELP_LINE("  -AdaptiveI:<on|off>");
    HELP_LINE("                Turn Adaptive I frames on/off");
    HELP_LINE("");
    HELP_LINE("  -AdaptiveB:<on|off>");
    HELP_LINE("                Turn Adaptive B frames on/off");
    HELP_LINE("");
    HELP_LINE("  -InitialDelayInKB");
    HELP_LINE("                The decoder starts decoding after the buffer reaches the");
    HELP_LINE("                initial size InitialDelayInKB, which is equivalent to reaching");
    HELP_LINE("                an initial delay of InitialDelayInKB*8000/TargetKbps ms");
    HELP_LINE("");
    HELP_LINE("  -MaxKbps      For variable bitrate control, specifies the maximum bitrate");
    HELP_LINE("                at which the encoded data enters the Video Buffering Verifier");
    HELP_LINE("                buffer");
    HELP_LINE("");
    HELP_LINE("  -gpucopy::<on|off>");
    HELP_LINE("                Enable or disable GPU copy mode");
    HELP_LINE("");
    HELP_LINE("  -repartitioncheck::<on|off>");
    HELP_LINE("                Enable or disable RepartitionCheckEnable mode");
    HELP_LINE("");
    HELP_LINE("  -cqp          Constant quantization parameter (CQP BRC) bitrate control method");
    HELP_LINE("                (by default constant bitrate control method is used)");
    HELP_LINE("                should be used along with -qpi, -qpp, -qpb.");
    HELP_LINE("");
    HELP_LINE("  -qpi          Constant quantizer for I frames (if bitrace control");
    HELP_LINE("                method is CQP).");
    HELP_LINE("                In range [1,51]. 0 by default, i.e.no limitations on QP.");
    HELP_LINE("");
    HELP_LINE("  -qpp          Constant quantizer for P frames (if bitrace control");
    HELP_LINE("                method is CQP).");
    HELP_LINE("                In range [1,51]. 0 by default, i.e.no limitations on QP.");
    HELP_LINE("");
    HELP_LINE("  -qpb          Constant quantizer for B frames (if bitrace control");
    HELP_LINE("                method is CQP).");
    HELP_LINE("                In range [1,51]. 0 by default, i.e.no limitations on QP.");
    HELP_LINE("");
    HELP_LINE("  -DisableQPOffset");
    HELP_LINE("                Disable QP adjustment for GOP pyramid-level frames");
    HELP_LINE("");
    HELP_LINE("  -MinQPI <QP>  min QP for I frames. In range [1,51]. 0 by default i.e. no limits");
    HELP_LINE("");
    HELP_LINE("  -MaxQPI <QP>  max QP for I frames. In range [1,51]. 0 by default i.e. no limits");
    HELP_LINE("");
    HELP_LINE("  -MinQPP <QP>  min QP for P frames. In range [1,51]. 0 by default i.e. no limits");
    HELP_LINE("");
    HELP_LINE("  -MaxQPP <QP>  max QP for P frames. In range [1,51]. 0 by default i.e. no limits");
    HELP_LINE("");
    HELP_LINE("  -MinQPB <QP>  min QP for B frames. In range [1,51]. 0 by default i.e. no limits");
    HELP_LINE("");
    HELP_LINE("  -MaxQPB <QP>  max QP for B frames. In range [1,51]. 0 by default i.e. no limits");
    HELP_LINE("");
    HELP_LINE("  -lowpower:<on|off>");
    HELP_LINE("                Turn this option ON to enable QuickSync Fixed Function");
    HELP_LINE("                (low-power HW) encoding mode");
    HELP_LINE("");
    HELP_LINE("  -qsv-ff       Turn option lowpower ON to enable QuickSync Fixed Function");
    HELP_LINE("                (low-power HW) encoding mode");
    HELP_LINE("");
    HELP_LINE("  -TargetBitDepthLuma");
    HELP_LINE("                Encoding target bit depth for luma samples,");
    HELP_LINE("                by default same as source one.");
    HELP_LINE("");
    HELP_LINE("  -TargetBitDepthChroma");
    HELP_LINE("                Encoding target bit depth for chroma samples,");
    HELP_LINE("                by default same as source one.");
    HELP_LINE("");
    HELP_LINE("  -roi_file <roi-file-name>");
    HELP_LINE("                Set Regions of Interest for each frame from <roi-file-name>");
    HELP_LINE("");
    HELP_LINE("  -roi_qpmap    Use QP map to emulate ROI for CQP mode");
    HELP_LINE("");
    HELP_LINE("  -extmbqp      Use external MBQP map");
    HELP_LINE("");
    HELP_LINE("  -AvcTemporalLayers <array:Layer.Scale>");
    HELP_LINE("                Configures the AVC temporal layers hierarchy");
#if defined(_WIN32) || defined(_WIN64)
    HELP_LINE("");
    HELP_LINE("  -TemporalLayers <array:Layer.Scale>");
    HELP_LINE("                Configures the AV1 + future codec temporal layers hierarchy");
#endif
    HELP_LINE("");
    HELP_LINE("  -BaseLayerPID <pid>");
    HELP_LINE("                Sets priority ID for the base layer");
    HELP_LINE("");
    HELP_LINE("  -SPSId <pid>  Sets sequence parameter set ID");
    HELP_LINE("");
    HELP_LINE("  -PPSId <pid>  Sets picture parameter set ID");
    HELP_LINE("");
    HELP_LINE("  -PicTimingSEI:<on|off>");
    HELP_LINE("                Enables or disables picture timing SEI");
    HELP_LINE("");
    HELP_LINE("  -NalHrdConformance:<on|off>");
    HELP_LINE("                Enables or disables picture HRD conformance");
    HELP_LINE("");
    HELP_LINE("  -VuiNalHrdParameters:<on|off>");
    HELP_LINE("                Enables or disables NAL HRD parameters in VUI header");
    HELP_LINE("");
    HELP_LINE("  -VuiTC        Sets transfer_characteristics for VUI.");
    HELP_LINE("                1 - BT.709");
    HELP_LINE("                18 - HLG(BT.2020)");
    HELP_LINE("");
    HELP_LINE("  -EmbeddedDenoise <mode> <level>");
    HELP_LINE("                Enables Denoiser in encoder");
    HELP_LINE("                    mode - mode of deniose");
    HELP_LINE("                    0    - default");
    HELP_LINE("                    1001 - auto BD rate");
    HELP_LINE("                    1002 - auto subjective");
    HELP_LINE("                    1003 - auto adjust");
    HELP_LINE("                    1004 - manual mode for pre-processing, need level");
    HELP_LINE("                    1005 - manual mode for post-processing, need level");
    HELP_LINE("                level - range of noise level is [0, 100]");
    HELP_LINE("");
    HELP_LINE("  -idr_interval size");
    HELP_LINE("                idr interval:");
    HELP_LINE("                0 (default) means every I is an IDR");
    HELP_LINE("                1 means every other I frame is an IDR etc");
    HELP_LINE("");
    HELP_LINE("  -ivf:<on|off> Turn IVF header on/off");
    HELP_LINE("");
    HELP_LINE("Pipeline description (vpp options):");
    HELP_LINE("");
    HELP_LINE("  -deinterlace  Forces VPP to deinterlace input stream");
    HELP_LINE("");
    HELP_LINE("  -deinterlace::ADI");
    HELP_LINE("                Forces VPP to deinterlace input stream using ADI algorithm");
    HELP_LINE("");
    HELP_LINE("  -deinterlace::ADI_SCD");
    HELP_LINE("                Forces VPP to deinterlace input stream using ADI_SCD algorithm");
    HELP_LINE("");
    HELP_LINE("  -deinterlace::ADI_NO_REF");
    HELP_LINE("                Forces VPP to deinterlace input stream using ADI no ref algorithm");
    HELP_LINE("");
    HELP_LINE("  -deinterlace::BOB");
    HELP_LINE("                Forces VPP to deinterlace input stream using BOB algorithm");
    HELP_LINE("");
    HELP_LINE("  -detail <level>");
    HELP_LINE("                Enables detail (edge enhancement) filter with");
    HELP_LINE("                provided level(0..100)");
    HELP_LINE("");
    HELP_LINE("  -denoise <level>");
    HELP_LINE("                Enables denoise filter with provided level (0..100)");
    HELP_LINE("");
    HELP_LINE("  -VppHvsDenoise <mode> <level>");
    HELP_LINE("                Enables vpp hvsdenoise filter)");
    HELP_LINE("                    mode - mode of deniose");
    HELP_LINE("                        0    - default");
    HELP_LINE("                        1001 - auto BD rate");
    HELP_LINE("                        1002 - auto subjective");
    HELP_LINE("                        1003 - auto adjust");
    HELP_LINE("                        1004 - manual mode for pre-processing, need level");
    HELP_LINE("                        1005 - manual mode for post-processing, need level");
    HELP_LINE("                level - range of noise level is [0, 100]");
    HELP_LINE("");
    HELP_LINE("  -FRC::PT      Enables FRC filter with Preserve Timestamp algorithm");
    HELP_LINE("");
    HELP_LINE("  -FRC::DT      Enables FRC filter with Distributed Timestamp algorithm");
    HELP_LINE("");
    HELP_LINE("  -FRC::INTERP  Enables FRC filter with Frame Interpolation algorithm");
    HELP_LINE("");
    HELP_LINE("  -scaling_mode <mode>");
    HELP_LINE("                Specifies scaling mode (lowpower/quality/EU)");
    HELP_LINE("");
    HELP_LINE("  -ec::<nv12|rgb4|yuy2|nv16|p010|p210|y210|y410|p016|y216>");
    HELP_LINE("                Forces encoder input to use provided chroma mode");
    HELP_LINE("");
    HELP_LINE("  -dc::<nv12|rgb4|yuy2|p010|y210|y410|p016|y216|y416>");
    HELP_LINE("                Forces decoder output to use provided chroma mode");
    HELP_LINE("                NOTE: chroma transform VPP may be automatically enabled");
    HELP_LINE("                if -ec/-dc parameters are provided");
    HELP_LINE("");
    HELP_LINE("  -angle 180    Enables 180 degrees picture rotation user module before encoding");
    HELP_LINE("");
    HELP_LINE("  -opencl       Uses implementation of rotation plugin");
    HELP_LINE("                (enabled with -angle option) through Intel(R) OpenCL");
    HELP_LINE("");
    HELP_LINE("  -w            Destination picture width, invokes VPP resize or decoder");
    HELP_LINE("                fixed function resize engine (if -dec_postproc specified)");
    HELP_LINE("");
    HELP_LINE("  -h            Destination picture height, invokes VPP resize or decoder fixed");
    HELP_LINE("                function resize engine (if -dec_postproc specified)");
    HELP_LINE("");
    HELP_LINE("  -field_processing <t2t|t2b|b2t|b2b|fr2fr>");
    HELP_LINE("                Field Copy feature");
    HELP_LINE("");
    HELP_LINE("  -WeightedPred::<default|implicit>");
    HELP_LINE("                Enables weighted prediction usage");
    HELP_LINE("");
    HELP_LINE("  -WeightedBiPred::<default|implicit>");
    HELP_LINE("                Enables weighted bi-prediction usage");
    HELP_LINE("");
    HELP_LINE("  -ir_type      Intra refresh type.");
    HELP_LINE("                    0 - no refresh");
    HELP_LINE("                    1 - vertical refresh");
    HELP_LINE("                    2 - horizontal refresh");
    HELP_LINE("                    3 - slice refresh");
    HELP_LINE("");
    HELP_LINE("  -ir_cycle_size");
    HELP_LINE("                Number of pictures within refresh cycle starting from 2");
    HELP_LINE("");
    HELP_LINE("  -ir_qp_delta  QP difference for inserted intra MBs.");
    HELP_LINE("                This is signed value in [-51, 51] range");
    HELP_LINE("");
    HELP_LINE("  -ir_cycle_dist");
    HELP_LINE("                Distance between the beginnings of the intra-refresh");
    HELP_LINE("                cycles in frames");
    HELP_LINE("");
    HELP_LINE("  -LowDelayBRC  strictly obey average frame size set by MaxKbps");
    HELP_LINE("");
    HELP_LINE("  -amfs:<on|off>");
    HELP_LINE("                adaptive max frame size. If set on, P or B frame size");
    HELP_LINE("                can exceed MaxFrameSize when the scene change is detected.");
    HELP_LINE("                It can benefit the video quality");
    HELP_LINE("");
    HELP_LINE("  -mfs          maximum frame size in bytes. Supported only with h264 and");
    HELP_LINE("                hevc codec for VBR mode.");
    HELP_LINE("");
    HELP_LINE("  -extbrc::<on|off|implicit>");
    HELP_LINE("                Enables external BRC for AVC and HEVC encoders");
    HELP_LINE("");
    HELP_LINE("  -ExtBrcAdaptiveLTR:<on|off>");
    HELP_LINE("                Set AdaptiveLTR for implicit extbrc");
    HELP_LINE("");
    HELP_LINE("  -vpp_comp <sourcesNum>");
    HELP_LINE("                Enables composition from several decoding sessions.");
    HELP_LINE("                Result is written to the file");
    HELP_LINE("");
    HELP_LINE("  -vpp_comp_only <sourcesNum>");
    HELP_LINE("                Enables composition from several decoding sessions.");
    HELP_LINE("                Result is shown on screen");
    HELP_LINE("");
    HELP_LINE("  -vpp_comp_num_tiles <Num>");
    HELP_LINE("                Quantity of tiles for composition.");
    HELP_LINE("                if equal to 0 tiles processing ignored");
    HELP_LINE("");
    HELP_LINE("  -vpp_comp_render");
    HELP_LINE("                Set pipeline mode when pipeline makes vpp composition + encode");
    HELP_LINE("                and get data from shared buffer");
    HELP_LINE("");
    HELP_LINE("  -vpp_comp_dst_x");
    HELP_LINE("                X position of this stream in composed stream");
    HELP_LINE("                (should be used in decoder session)");
    HELP_LINE("");
    HELP_LINE("  -vpp_comp_dst_y");
    HELP_LINE("                Y position of this stream in composed stream");
    HELP_LINE("                (should be used in decoder session)");
    HELP_LINE("");
    HELP_LINE("  -vpp_comp_dst_h");
    HELP_LINE("                Height of this stream in composed stream");
    HELP_LINE("                (should be used in decoder session)");
    HELP_LINE("");
    HELP_LINE("  -vpp_comp_dst_w");
    HELP_LINE("                Width of this stream in composed stream");
    HELP_LINE("                (should be used in decoder session)");
    HELP_LINE("");
    HELP_LINE("  -vpp_comp_src_h");
    HELP_LINE("                Height of this stream in composed stream");
    HELP_LINE("                (should be used in decoder session)");
    HELP_LINE("");
    HELP_LINE("  -vpp_comp_src_w");
    HELP_LINE("                Width of this stream in composed stream");
    HELP_LINE("                (should be used in decoder session)");
    HELP_LINE("");
    HELP_LINE("  -vpp_comp_tile_id");
    HELP_LINE("                Tile_id for current channel of composition");
    HELP_LINE("                (should be used in decoder session)");
    HELP_LINE("");
    HELP_LINE("  -vpp_comp_dump <file-name>");
    HELP_LINE("                Dump of VPP Composition's output into file.");
    HELP_LINE("                Valid if with -vpp_comp* options");
    HELP_LINE("");
    HELP_LINE("  -vpp_comp_dump null_render");
    HELP_LINE("                Disabling rendering after VPP Composition.");
    HELP_LINE("                This is for performance measurements");
    HELP_LINE("");
    HELP_LINE("  -dec_postproc Resize after decoder using direct pipe");
    HELP_LINE("                (should be used in decoder session)");
    HELP_LINE("");
    HELP_LINE("  -single_texture_d3d11");
    HELP_LINE("                single texture mode for d3d11 allocator");
    HELP_LINE("");
    HELP_LINE("  -preset <default|dss|conference|gaming>");
    HELP_LINE("                Use particular preset for encoding parameters");
    HELP_LINE("");
    HELP_LINE("  -pp           Print preset parameters");
    HELP_LINE("");
    HELP_LINE("  -forceSyncAllSession:<on|off>");
    HELP_LINE("                Enable across-session synchronization");
    HELP_LINE("                How it works: the decoder takes a required number of");
    HELP_LINE("                surfaces for work, and when it ready to take more,");
    HELP_LINE("                does not take a new one, but waits until one of them");
    HELP_LINE("                is available for work");
    HELP_LINE("                Option is ON by default for");
    HELP_LINE("                -MemModel::VisibleIntAlloc/HiddenIntAlloc(-memory 2/3)");
    HELP_LINE("                and 1->N scenarios");
    HELP_LINE("                Option is not intended for -memory 1 and for transcoding scenarios");
    HELP_LINE("");
    HELP_LINE("  -dump <fileName>");
    HELP_LINE("                dump MSDK components configuration to the file in text form");
    HELP_LINE("");
    HELP_LINE("  -tcbrctestfile <filepath>");
    HELP_LINE("                if specified, the encoder will take targetFrameSize parameters for");
    HELP_LINE("                TCBRC test from text file. The parameters for TCBRC should be");
    HELP_LINE("                calculated based on the channel width conditions, sample doesn't");
    HELP_LINE("                have this functionality. Therefore the file data from <filepath>");
    HELP_LINE("                is used for TCBRC test. This is a test model");
    HELP_LINE("");
    HELP_LINE("  -cs           turn on cascade scaling");
    HELP_LINE("");
    HELP_LINE("  -trace        turn on tracing");
    HELP_LINE("");
    HELP_LINE("  -trace::ENC   turn on tracing, tune pipeline for ENC latency");
    HELP_LINE("");
    HELP_LINE("  -trace::E2E   turn on tracing, tune pipeline for E2E latency");
    HELP_LINE("");
    HELP_LINE("  -trace_buffer_size <x>");
    HELP_LINE("                trace buffer size in MBytes");
    HELP_LINE("");
    HELP_LINE("  -parallel_encoding");
    HELP_LINE("                use several encoders to encode single bitstream,");
    HELP_LINE("                see readme for more details");
#if defined(LIBVA_X11_SUPPORT)
    HELP_LINE("");
    HELP_LINE("  -rx11        use libva X11 backend");
#endif
#if defined(LIBVA_WAYLAND_SUPPORT)
    HELP_LINE("");
    HELP_LINE("  -rwld        use libva WAYLAND backend");
#endif
#if defined(LIBVA_DRM_SUPPORT)
    HELP_LINE("");
    HELP_LINE("  -rdrm        use libva DRM backend");
#endif
    HELP_LINE("");
    HELP_LINE("  -syncop_timeout");
    HELP_LINE("                SyncOperation timeout in milliseconds");
#if defined(LIBVA_SUPPORT)
    HELP_LINE("");
    HELP_LINE("  -3dlut <file-name>");
    HELP_LINE("                Enable 3DLUT VPP filter");
#endif
#ifdef ONEVPL_EXPERIMENTAL
    HELP_LINE("");
    HELP_LINE("  -perc_enc_filter            ");
    HELP_LINE("                Enable perceptual encoding prefilter in VPP");
#endif
    HELP_LINE("");
    HELP_LINE("ParFile format:");
    HELP_LINE("  ParFile is extension of what can be achieved by setting pipeline in the command");
    HELP_LINE("  line. For more information on ParFile format see readme-multi-transcode.pdf");
    HELP_LINE("");
    HELP_LINE("Examples:");
    HELP_LINE("  sample_multi_transcode -i::mpeg2 in.mpeg2 -o::h264 out.h264");
    HELP_LINE("  sample_multi_transcode -i::mvc in.mvc -o::mvc out.mvc -w 320 -h 240");
}

void TranscodingSample::PrintStreamInfo(mfxU32 session_number,
                                        sInputParams* pParams,
                                        mfxVersion* pVer) {
    MSDK_CHECK_POINTER_NO_RET(pVer);

    if ((MFX_IMPL_AUTO <= pParams->libType) && (MFX_IMPL_HARDWARE4 >= pParams->libType)) {
        msdk_printf(MSDK_STRING("MFX %s Session %d API ver %d.%d parameters: \n"),
                    (MFX_IMPL_SOFTWARE == pParams->libType) ? MSDK_STRING("SOFTWARE")
                                                            : MSDK_STRING("HARDWARE"),
                    (int)session_number,
                    (int)pVer->Major,
                    (int)pVer->Minor);
    }

    if (0 == pParams->DecodeId)
        msdk_printf(MSDK_STRING("Input  video: From parent session\n"));
    else
        msdk_printf(MSDK_STRING("Input  video: %s\n"), CodecIdToStr(pParams->DecodeId).c_str());

    // means that source is parent session
    if (0 == pParams->EncodeId)
        msdk_printf(MSDK_STRING("Output video: To child session\n"));
    else
        msdk_printf(MSDK_STRING("Output video: %s\n"), CodecIdToStr(pParams->EncodeId).c_str());
    msdk_string dll_info = GetDllInfo(pParams);
    if (!dll_info.empty())
        msdk_printf(MSDK_STRING("MFX dll: %s\n"), dll_info.c_str());
    msdk_printf(MSDK_STRING("\n"));
}

msdk_string TranscodingSample::GetDllInfo(sInputParams* pParams) {
#if defined(_WIN32) || defined(_WIN64)
    std::vector<msdk_char> buf;
    buf.resize(2048);
    HANDLE hCurrent = GetCurrentProcess();
    std::vector<HMODULE> modules;
    DWORD cbNeeded;
    int nModules;
    if (NULL == EnumProcessModules(hCurrent, NULL, 0, &cbNeeded))
        return "";
    nModules = cbNeeded / sizeof(HMODULE);
    modules.resize(nModules);
    if (NULL == EnumProcessModules(hCurrent, &modules[0], (DWORD)modules.size(), &cbNeeded)) {
        return "";
    }
    for (auto module : modules) {
        DWORD chars_read = GetModuleFileName(module, &buf[0], (DWORD)buf.size());
        while (buf.size() <= chars_read) {
            buf.resize(buf.size() + 1024);
            chars_read = GetModuleFileName(module, &buf[0], (DWORD)buf.size());
        }
        if (msdk_strstr(&buf[0], MSDK_STRING("libmfxhw")) &&
            (MFX_IMPL_SOFTWARE != pParams->libType)) {
            return msdk_string(buf.begin(), buf.end());
        }
        else if (msdk_strstr(&buf[0], MSDK_STRING("libmfxsw")) &&
                 (MFX_IMPL_SOFTWARE == pParams->libType)) {
            return msdk_string(buf.begin(), buf.end());
        }
    }
#endif
    return "";
}

CmdProcessor::CmdProcessor()
        : m_SessionParamId(0),
          m_SessionArray(),
          m_decoderPlugins(),
          m_encoderPlugins(),
          m_PerfFILE(nullptr),
          m_parName(),
          statisticsWindowSize(0),
          statisticsLogFile(nullptr),
          DumpLogFileName(),
          m_nTimeout(0),
          bRobustFlag(false),
          bSoftRobustFlag(false),
          shouldUseGreedyFormula(false),
          m_lines() {} //CmdProcessor::CmdProcessor()

CmdProcessor::~CmdProcessor() {
    m_SessionArray.clear();
    m_decoderPlugins.clear();
    m_encoderPlugins.clear();
    if (m_PerfFILE)
        fclose(m_PerfFILE);
    if (statisticsLogFile)
        fclose(statisticsLogFile);

} //CmdProcessor::~CmdProcessor()

void CmdProcessor::PrintParFileName() {
    if (!m_parName.empty() && m_PerfFILE) {
        msdk_fprintf(m_PerfFILE, MSDK_STRING("Input par file: %s\n\n"), m_parName.c_str());
    }
}

msdk_string CmdProcessor::GetLine(mfxU32 n) {
    if (m_lines.size() > n)
        return m_lines[n];
    return msdk_string();
}

mfxStatus CmdProcessor::ParseCmdLine(int argc, msdk_char* argv[]) {
    mfxStatus sts = MFX_ERR_UNSUPPORTED;

    if (1 == argc) {
        PrintError(MSDK_STRING("Too few parameters"), NULL);
        return MFX_ERR_UNSUPPORTED;
    }

    --argc;
    ++argv;

    while (argv[0]) {
        if (0 == msdk_strcmp(argv[0], MSDK_STRING("-par"))) {
            --argc;
            ++argv;
            if (!argv[0]) {
                msdk_printf(MSDK_STRING("error: no argument given for '-par' option\n"));
            }
            m_parName = msdk_string(argv[0]);
        }
        else if (0 == msdk_strcmp(argv[0], MSDK_STRING("-timeout"))) {
            --argc;
            ++argv;
            if (!argv[0]) {
                msdk_printf(MSDK_STRING("error: no argument given for '-timeout' option\n"));
            }
            if (MFX_ERR_NONE != msdk_opt_read(argv[0], m_nTimeout)) {
                msdk_printf(MSDK_STRING("error: -timeout \"%s\" is invalid"), argv[0]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[0], MSDK_STRING("-robust"))) {
            bRobustFlag = true;
        }
        else if (0 == msdk_strcmp(argv[0], MSDK_STRING("-robust:soft"))) {
            bSoftRobustFlag = true;
        }
        else if (0 == msdk_strcmp(argv[0], MSDK_STRING("-?"))) {
            PrintHelp();
            return MFX_WRN_OUT_OF_RANGE;
        }
        else if (0 == msdk_strcmp(argv[0], MSDK_STRING("-greedy"))) {
            shouldUseGreedyFormula = true;
        }
        else if (0 == msdk_strcmp(argv[0], MSDK_STRING("-p"))) {
            if (m_PerfFILE) {
                msdk_printf(MSDK_STRING("error: only one performance file is supported"));
                return MFX_ERR_UNSUPPORTED;
            }
            --argc;
            ++argv;
            if (!argv[0]) {
                msdk_printf(MSDK_STRING("error: no argument given for '-p' option\n"));
            }
            MSDK_FOPEN(m_PerfFILE, argv[0], MSDK_STRING("w"));
            if (NULL == m_PerfFILE) {
                msdk_printf(MSDK_STRING("error: performance file \"%s\" not found"), argv[0]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[0], MSDK_STRING("--"))) {
            // just skip separator "--" which delimits cmd options and pipeline settings
            break;
        }
        else if (0 == msdk_strcmp(argv[0], MSDK_STRING("-stat"))) {
            --argc;
            ++argv;
            if (!argv[0]) {
                msdk_printf(MSDK_STRING("error: no argument given for 'stat' option\n"));
            }
            if (MFX_ERR_NONE != msdk_opt_read(argv[0], statisticsWindowSize)) {
                msdk_printf(MSDK_STRING("error: stat \"%s\" is invalid"), argv[0]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[0], MSDK_STRING("-stat-log"))) {
            if (statisticsLogFile) {
                msdk_printf(MSDK_STRING("error: only one statistics file is supported"));
                return MFX_ERR_UNSUPPORTED;
            }
            --argc;
            ++argv;
            if (!argv[0]) {
                msdk_printf(MSDK_STRING("error: no argument given for 'stat-log' option\n"));
            }
            MSDK_FOPEN(statisticsLogFile, argv[0], MSDK_STRING("w"));
            if (NULL == statisticsLogFile) {
                msdk_printf(MSDK_STRING("error: statistics file \"%s\" not found"), argv[0]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[0], MSDK_STRING("-stat-per-frame"))) {
            if (!DumpLogFileName.empty()) {
                msdk_printf(MSDK_STRING("error: only one dump file is supported"));
                return MFX_ERR_UNSUPPORTED;
            }
            --argc;
            ++argv;
            if (!argv[0]) {
                msdk_printf(MSDK_STRING("error: no argument given for 'stat-dump' option\n"));
            }
            DumpLogFileName = argv[0];
        }
        else {
            break;
        }
        --argc;
        ++argv;
    }

    msdk_printf(MSDK_STRING("Multi Transcoding Sample Version %s\n\n"),
                GetMSDKSampleVersion().c_str());

    //Read pipeline from par file
    if (!m_parName.empty() && !argv[0]) {
        sts = ParseParFile(m_parName);

        if (MFX_ERR_NONE != sts) {
            return sts;
        }
    }
    //Read pipeline from cmd line
    else if (!argv[0]) {
        msdk_printf(MSDK_STRING("error: pipeline description not found\n"));
        return MFX_ERR_UNSUPPORTED;
    }
    else if (argv[0] && !m_parName.empty()) {
        msdk_printf(MSDK_STRING(
            "error: simultaneously enabling parfile and description pipeline from command line forbidden\n"));
        return MFX_ERR_UNSUPPORTED;
    }
    else {
        sts = ParseParamsForOneSession(argc, argv);
        if (MFX_ERR_NONE != sts) {
            msdk_printf(MSDK_STRING("error: pipeline description is invalid\n"));
            return sts;
        }
    }

    return sts;

} //mfxStatus CmdProcessor::ParseCmdLine(int argc, msdk_char *argv[])

mfxStatus CmdProcessor::ParseParFile(const msdk_string& filename) {
    mfxStatus sts = MFX_ERR_UNSUPPORTED;
    if (filename.empty()) {
        return MFX_ERR_UNSUPPORTED;
    }
    msdk_ifstream in_stream(filename);
    if (!in_stream.good()) {
        msdk_printf(MSDK_STRING("error: ParFile \"%s\" not found\n"), m_parName.c_str());
        return MFX_ERR_UNSUPPORTED;
    }
    if (!in_stream.is_open()) {
        msdk_printf(MSDK_STRING("error: ParFile \"%s\" could not be opened\n"), m_parName.c_str());
        return MFX_ERR_UNSUPPORTED;
    }
    msdk_string line;
    while (std::getline(in_stream, line)) {
        if (line.empty()) {
            continue;
        }
        sts = TokenizeLine(line);
        MSDK_CHECK_STATUS(sts, "TokenizeLine failed");
    }
    return MFX_ERR_NONE;

} //mfxStatus CmdProcessor::ParseParFile(const msdk_string& filename)

// calculate length of string literal, including leading and trailing "
// pTempLine = start of string (must begin with ")
// length = remaining characters in pTempLine
// returns length of string, or 0 if error
size_t CmdProcessor::GetStringLength(msdk_char* pTempLine, size_t length) {
    size_t i;

    // error - no leading " found
    if (pTempLine[0] != '\"')
        return 0;

    for (i = 1; i < length; i++) {
        if (pTempLine[i] == '\"')
            break;
    }

    // error - no closing " found
    if (i == length)
        return 0;

    return i + 1;
}

mfxStatus CmdProcessor::TokenizeLine(const msdk_string& line) {
    return TokenizeLine(line.c_str(), line.size());
}

mfxStatus CmdProcessor::TokenizeLine(const msdk_char* pLine, size_t length) {
    size_t i, strArgLen;
    const mfxU8 maxArgNum = 255;
    msdk_char* argv[maxArgNum + 1];
    mfxU32 argc   = 0;
    auto pMemLine = std::make_unique<msdk_char[]>(length + 2);

    msdk_char* pTempLine = pMemLine.get();
    pTempLine[0]         = ' ';
    pTempLine++;

    MSDK_MEMCPY_BUF(pTempLine, 0, length * sizeof(msdk_char), pLine, length * sizeof(msdk_char));

    // parse into command streams
    for (i = 0; i < length; i++) {
        // check if separator
        if (IS_SEPARATOR(pTempLine[-1]) && !IS_SEPARATOR(pTempLine[0])) {
            argv[argc++] = pTempLine;
            if (argc > maxArgNum) {
                PrintError(MSDK_STRING("Too many parameters (reached maximum of %d)"), maxArgNum);
                return MFX_ERR_UNSUPPORTED;
            }
        }

        if (*pTempLine == '\"') {
            strArgLen = GetStringLength(pTempLine, length - i);
            if (!strArgLen) {
                PrintError(MSDK_STRING("Error parsing string literal"));
                return MFX_ERR_UNSUPPORTED;
            }

            // remove leading and trailing ", bump pointer ahead to next argument
            pTempLine[0]             = ' ';
            pTempLine[strArgLen - 1] = ' ';
            pTempLine += strArgLen;
            i += strArgLen;
        }

        if (*pTempLine == ' ' || *pTempLine == '\r' || *pTempLine == '\n') {
            *pTempLine = 0;
        }
        pTempLine++;
    }

    // EOL for last parameter
    pTempLine[0] = 0;

    return ParseParamsForOneSession(argc, argv);
}

bool CmdProcessor::isspace(char a) {
    return (std::isspace(a) != 0);
}

bool CmdProcessor::is_not_allowed_char(char a) {
    return (std::isdigit(a) == 0) && (std::isspace(a) == 0) && (a != ';') && (a != '-');
}

bool CmdProcessor::ParseROIFile(const msdk_string& roi_file_name,
                                std::vector<mfxExtEncoderROI>& m_ROIData) {
    FILE* roi_file = NULL;
    MSDK_FOPEN(roi_file, roi_file_name.c_str(), MSDK_STRING("rb"));

    m_ROIData.clear();

    if (roi_file) {
        // read file to buffer
        fseek(roi_file, 0, SEEK_END);
        long file_size = ftell(roi_file);
        rewind(roi_file);
        std::vector<char> buffer(file_size);
        char* roi_data = &buffer[0];
        if (file_size < 0 || (size_t)file_size != fread(roi_data, 1, file_size, roi_file)) {
            fclose(roi_file);
            return false;
        }
        fclose(roi_file);

        // search for not allowed characters
        char* not_allowed_char = std::find_if(roi_data, roi_data + file_size, is_not_allowed_char);
        if (not_allowed_char != (roi_data + file_size)) {
            return false;
        }

        // get unformatted roi data
        std::string unformatted_roi_data;
        unformatted_roi_data.clear();
        std::remove_copy_if(roi_data,
                            roi_data + file_size,
                            std::inserter(unformatted_roi_data, unformatted_roi_data.end()),
                            isspace);

        // split data to items
        std::stringstream unformatted_roi_data_ss(unformatted_roi_data);
        std::vector<std::string> items;
        items.clear();
        split(unformatted_roi_data, items, msdk_char(';'));

        // parse data and store roi data for each frame
        unsigned int item_ind = 0;
        while (1) {
            if (item_ind >= items.size())
                break;

            mfxExtEncoderROI frame_roi;
            std::memset(&frame_roi, 0, sizeof(frame_roi));
            frame_roi.Header.BufferId = MFX_EXTBUFF_ENCODER_ROI;
            frame_roi.ROIMode         = MFX_ROI_MODE_QP_DELTA;

            int roi_num = std::atoi(items[item_ind].c_str());
            if (roi_num < 0 || roi_num > (int)(sizeof(frame_roi.ROI) / sizeof(frame_roi.ROI[0]))) {
                m_ROIData.clear();
                return false;
            }
            if ((item_ind + 5 * roi_num) >= items.size()) {
                m_ROIData.clear();
                return false;
            }

            for (int i = 0; i < roi_num; i++) {
                // do not handle out of range integer errors
                frame_roi.ROI[i].Left    = std::atoi(items[item_ind + i * 5 + 1].c_str());
                frame_roi.ROI[i].Top     = std::atoi(items[item_ind + i * 5 + 2].c_str());
                frame_roi.ROI[i].Right   = std::atoi(items[item_ind + i * 5 + 3].c_str());
                frame_roi.ROI[i].Bottom  = std::atoi(items[item_ind + i * 5 + 4].c_str());
                frame_roi.ROI[i].DeltaQP = (mfxI16)std::atoi(items[item_ind + i * 5 + 5].c_str());
            }
            frame_roi.NumROI = (mfxU16)roi_num;
            m_ROIData.push_back(frame_roi);
            item_ind = item_ind + roi_num * 5 + 1;
        }
    }
    else {
        return false;
    }
    return true;
}
#ifdef ENABLE_MCTF

int ParseMCTFParamsFileContent(msdk_fstream& input, sInputParams* pParams, int maxParams);
void ParseMCTFParamsByValue(const msdk_string& argument, sInputParams* pParams);

void ParseMCTFParams(msdk_char* strInput[], mfxU32 nArgNum, mfxU32& curArg, sInputParams* pParams) {
    if (0 != msdk_strcmp(strInput[curArg], MSDK_STRING("-mctf"))) {
        msdk_printf(MSDK_STRING("MCTF options should start with -mcft.\n"));
        return;
    }

    pParams->mctfParam.mode                  = VPP_FILTER_ENABLED_DEFAULT;
    pParams->mctfParam.params.FilterStrength = 0;
    pParams->mctfParam.rtParams.Reset();
    #if defined ENABLE_MCTF_EXT
    pParams->mctfParam.params.TemporalMode      = MFX_MCTF_TEMPORAL_MODE_2REF; // default
    pParams->mctfParam.params.BitsPerPixelx100k = 0;
    pParams->mctfParam.params.Deblocking        = MFX_CODINGOPTION_OFF;
    pParams->mctfParam.params.Overlap           = MFX_CODINGOPTION_OFF;
    pParams->mctfParam.params.MVPrecision       = MFX_MVPRECISION_INTEGER;
    #endif

    if (curArg + 1 >= nArgNum) {
        msdk_printf(
            MSDK_STRING("MCTF option should have at least one value. Enabling default settings\n"));
        return;
    }
    curArg++;

    msdk_string argument = msdk_string(strInput[curArg]);

    if (argument.empty()) {
        msdk_printf(MSDK_STRING(
            "MCTF option should have at least one non-empty value. Enabling default settings\n"));
        return;
    }

    // Try to read argument as a file
    msdk_fstream fs_file(argument, std::ios_base::in);
    if (fs_file.is_open()) {
        ParseMCTFParamsFileContent(fs_file, pParams, 10000);
        return;
    }

    // If file could not be read treat parameter as a value
    ParseMCTFParamsByValue(argument, pParams);
}
#endif // ENABLE_MCTF

int ParseMCTFParamsFileContent(msdk_fstream& input, sInputParams* pParams, int maxParams) {
    int nOfRTParams = 0;
    msdk_string line;
    msdk_string temp_str;
    while (getline(input, line, msdk_char(','))) {
        if (nOfRTParams >= maxParams) {
            break;
        }
        std::vector<msdk_string> elem_parts;
        msdk_stringstream elem_stream(line);
        // currently, there is just 1 param in the file;
        sMctfRunTimeParam tmp = { 0 };
        if (elem_parts.size() < 1) {
            msdk_printf(
                MSDK_STRING(
                    "there was an error met during parsing FS file. %d entries were parsed.\n"),
                nOfRTParams);
            break;
        }

        try {
            tmp.FilterStrength = (mfxU16)std::stoul(elem_parts[0]);
            pParams->mctfParam.rtParams.RunTimeParams.push_back(tmp);
        }
        catch (...) {
            if (nOfRTParams == 0) {
                msdk_printf(MSDK_STRING("Nothing found in FS file.\n"));
            }
            else {
                msdk_printf(
                    MSDK_STRING(
                        "There was an error reading entry %d in the FS file. ending parsing.\n"),
                    nOfRTParams);
            }
            break;
        }
        nOfRTParams++;
    }
    pParams->mctfParam.mode = VPP_FILTER_ENABLED_CONFIGURED;
    // take very first FS value from the file and use it as a value for FilterStrength
    pParams->mctfParam.rtParams.Restart();
    auto cur_param = pParams->mctfParam.rtParams.GetCurParam();
    if (cur_param) {
        pParams->mctfParam.params.FilterStrength = cur_param->FilterStrength;
    }
    else {
        pParams->mctfParam.params.FilterStrength = 0;
    }
    return nOfRTParams;
}

void ParseMCTFParamsByValue(const msdk_string& argument, sInputParams* pParams) {
    std::vector<msdk_string> mctf_param_parts;
    msdk_stringstream mctf_param_stream(argument);
    msdk_string temp_str;
    split(argument, mctf_param_parts, msdk_char(':'));

    mfxU16 _strength = 0;
#ifndef ENABLE_MCTF_EXT
    try {
        _strength = std::stoi(mctf_param_parts[0]);
    }
    catch (const std::invalid_argument&) {
        msdk_printf(MSDK_STRING("Error reading MCTF strength setting.\n"));
    }
    catch (const std::out_of_range&) {
        msdk_printf(MSDK_STRING("MCTF strength out of bounds.\n"));
    }
#else
    // Order of arguments is:
    // MctfMode:BitsPerPixel:Strength:ME   :Overlap:DB
    // <int>   :<float>     :<int>   :<int>:<int>  :<int>
    mfxU16 _temporal_mode = 2;
    mfxF64 _bitsperpixel  = 0.0;
    mfxU16 _me_precision  = 0;
    mfxU16 _overlap       = 0;
    mfxU16 _deblock       = 0;
    try {
        _temporal_mode = std::stoi(mctf_param_parts[0]);
    }
    catch (const std::invalid_argument&) {
        msdk_printf(MSDK_STRING("Error reading MCTF temporal mode setting.\n"));
    }
    catch (const std::out_of_range&) {
        msdk_printf(MSDK_STRING("MCTF temporal mode setting out of bounds.\n"));
    }
    try {
        _bitsperpixel = std::stof(mctf_param_parts[1]);
    }
    catch (const std::invalid_argument&) {
        msdk_printf(MSDK_STRING("Error reading MCTF bits per pixel setting.\n"));
    }
    catch (const std::out_of_range&) {
        msdk_printf(MSDK_STRING("MCTF bits per pixel setting out of bounds.\n"));
    }
    try {
        _strength = std::stoi(mctf_param_parts[2]);
    }
    catch (const std::invalid_argument&) {
        msdk_printf(MSDK_STRING("Error reading MCTF strength setting.\n"));
    }
    catch (const std::out_of_range&) {
        msdk_printf(MSDK_STRING("MCTF strength out of bounds.\n"));
    }
    try {
        _me_precision = std::stoi(mctf_param_parts[3]);
    }
    catch (const std::invalid_argument&) {
        msdk_printf(MSDK_STRING("Error reading MCTF MV precision setting.\n"));
    }
    catch (const std::out_of_range&) {
        msdk_printf(MSDK_STRING("MCTF MV precision out of bounds.\n"));
    }
    try {
        _overlap = std::stoi(mctf_param_parts[4]);
    }
    catch (const std::invalid_argument&) {
        msdk_printf(MSDK_STRING("Error reading MCTF overlap setting.\n"));
    }
    catch (const std::out_of_range&) {
        msdk_printf(MSDK_STRING("MCTF overlap out of bounds.\n"));
    }
    try {
        _deblock = std::stoi(mctf_param_parts[5]);
    }
    catch (const std::invalid_argument&) {
        msdk_printf(MSDK_STRING("Error reading MCTF deblock setting.\n"));
    }
    catch (const std::out_of_range&) {
        msdk_printf(MSDK_STRING("MCTF deblock out of bounds.\n"));
    }
#endif // ENABLE_MCTF_EXT
    pParams->mctfParam.mode = VPP_FILTER_ENABLED_CONFIGURED;
    pParams->mctfParam.rtParams.Restart();
    pParams->mctfParam.params.FilterStrength = _strength;
#if defined ENABLE_MCTF_EXT
    pParams->mctfParam.params.BitsPerPixelx100k = mfxU32(_bitsperpixel * MCTF_BITRATE_MULTIPLIER);
    switch (_temporal_mode) {
        case 0:
            pParams->mctfParam.params.TemporalMode = MFX_MCTF_TEMPORAL_MODE_SPATIAL;
            break;
        case 1:
            pParams->mctfParam.params.TemporalMode = MFX_MCTF_TEMPORAL_MODE_1REF;
            break;
        case 2:
            pParams->mctfParam.params.TemporalMode = MFX_MCTF_TEMPORAL_MODE_2REF;
            break;
        case 3:
            pParams->mctfParam.params.TemporalMode = MFX_MCTF_TEMPORAL_MODE_4REF;
            break;
        default:
            pParams->mctfParam.params.TemporalMode = MFX_MCTF_TEMPORAL_MODE_UNKNOWN;
    };
    switch (_deblock) {
        case 0:
            pParams->mctfParam.params.Deblocking = MFX_CODINGOPTION_OFF;
            break;
        case 1:
            pParams->mctfParam.params.Deblocking = MFX_CODINGOPTION_ON;
            break;
        default:
            pParams->mctfParam.params.Deblocking = MFX_CODINGOPTION_UNKNOWN;
    };
    switch (_overlap) {
        case 0:
            pParams->mctfParam.params.Overlap = MFX_CODINGOPTION_OFF;
            break;
        case 1:
            pParams->mctfParam.params.Overlap = MFX_CODINGOPTION_ON;
            break;
        default:
            pParams->mctfParam.params.Overlap = MFX_CODINGOPTION_UNKNOWN;
    };
    switch (_me_precision) {
        case 0:
            pParams->mctfParam.params.MVPrecision = MFX_MVPRECISION_INTEGER;
            break;
        case 1:
            pParams->mctfParam.params.MVPrecision = MFX_MVPRECISION_QUARTERPEL;
            break;
        default:
            pParams->mctfParam.params.MVPrecision = MFX_MVPRECISION_UNKNOWN;
    };
#endif // ENABLE_MCTF_EXT
}

// return values:
//   0 if argv[i] is processed successfully (MFX_ERR_NONE)
// < 1 if argv[i] is processed and generates an error OR argv[i] is not processed (no match)
mfxStatus ParseAdditionalParams(msdk_char* argv[],
                                mfxU32 argc,
                                mfxU32& i,
                                TranscodingSample::sInputParams& InputParams) {
    if (0 == msdk_strcmp(argv[i], MSDK_STRING("-AvcTemporalLayers"))) {
        InputParams.nAvcTemp = 1;
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        mfxU16 arr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        int j;
        size_t k;
        std::vector<msdk_string> args;
        k = split(argv[i + 1], args);
        if (k != 8) {
            PrintError(argv[0], MSDK_STRING("Invalid number of layers for AvcTemporalLayers"));
            return MFX_ERR_UNSUPPORTED;
        }

        for (int j = 0; j < 8; j++) {
            try {
                arr[j] = (mfxU16)std::stoul(args[j]);
            }
            catch (const std::invalid_argument&) {
                msdk_printf(MSDK_STRING("Error reading AvcTemporalLayers setting.\n"));
                return MFX_ERR_UNSUPPORTED;
            }
            catch (const std::out_of_range&) {
                msdk_printf(MSDK_STRING("AvcTemporalLayers setting out of bounds.\n"));
                return MFX_ERR_UNSUPPORTED;
            }
        }

        for (j = 0; j < 8; j++) {
            InputParams.nAvcTemporalLayers[j] = arr[j];
        }
        i += 1;
    }
#if defined(_WIN32) || defined(_WIN64)
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-TemporalLayers"))) {
        InputParams.bTemporalLayers = true;
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        mfxU16 arr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        int j;
        size_t k;

        std::vector<msdk_string> args;
        k = split(argv[i + 1], args);
        if (k != 8) {
            PrintError(argv[0], MSDK_STRING("Invalid number of layers for TemporalLayers"));
            return MFX_ERR_UNSUPPORTED;
        }

        for (int j = 0; j < 8; j++) {
            try {
                arr[j] = (mfxU16)std::stoul(args[j]);
            }
            catch (const std::invalid_argument&) {
                msdk_printf(MSDK_STRING("Error reading TemporalLayers setting.\n"));
                return MFX_ERR_UNSUPPORTED;
            }
            catch (const std::out_of_range&) {
                msdk_printf(MSDK_STRING("TemporalLayers setting out of bounds.\n"));
                return MFX_ERR_UNSUPPORTED;
            }
        }

        for (j = 0; j < 8; j++) {
            InputParams.temporalLayers[j].FrameRateScale = arr[j];
        }
        i += 1;
    }
#endif
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-BaseLayerPID"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nBaseLayerPID)) {
            PrintError(argv[0], MSDK_STRING("BaseLayerPID is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-SPSId"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nSPSId)) {
            PrintError(argv[0], MSDK_STRING("SPSId is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-PPSId"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nPPSId)) {
            PrintError(argv[0], MSDK_STRING("PPSId is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-VuiTC"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nTransferCharacteristics)) {
            PrintError(NULL, MSDK_STRING("-VuiTC TransferCharacteristics is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-lowpower:on"))) {
        InputParams.enableQSVFF = true;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-lowpower:off"))) {
        InputParams.enableQSVFF = false;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-PicTimingSEI:on"))) {
        InputParams.nPicTimingSEI = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-PicTimingSEI:off"))) {
        InputParams.nPicTimingSEI = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-NalHrdConformance:on"))) {
        InputParams.nNalHrdConformance = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-NalHrdConformance:off"))) {
        InputParams.nNalHrdConformance = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-VuiNalHrdParameters:on"))) {
        InputParams.nVuiNalHrdParameters = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-VuiNalHrdParameters:off"))) {
        InputParams.nVuiNalHrdParameters = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-BitrateLimit:on"))) {
        InputParams.BitrateLimit = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-BitrateLimit:off"))) {
        InputParams.BitrateLimit = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-AdaptiveI:on"))) {
        InputParams.AdaptiveI = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-AdaptiveI:off"))) {
        InputParams.AdaptiveI = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-AdaptiveB:on"))) {
        InputParams.AdaptiveB = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-AdaptiveB:off"))) {
        InputParams.AdaptiveB = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-iGfx"))) {
        InputParams.adapterType = mfxMediaAdapterType::MFX_MEDIA_INTEGRATED;
#if (defined(_WIN32) || defined(_WIN64))
        InputParams.bPreferiGfx = true;
#endif
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-dGfx"))) {
        InputParams.adapterType = mfxMediaAdapterType::MFX_MEDIA_DISCRETE;
        if (i + 1 < argc && isdigit(*argv[1 + i])) {
            if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.dGfxIdx)) {
                PrintError(argv[0], MSDK_STRING("value of -dGfx is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
#if (defined(_WIN32) || defined(_WIN64))
        InputParams.bPreferdGfx = true;
#endif
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-AdapterNum"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.adapterNum)) {
            PrintError(argv[0], MSDK_STRING("Value of -AdapterNum is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-tcbrctestfile"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        InputParams.TCBRCFileMode = true;
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.strTCBRCFilePath)) {
            PrintError(argv[0], MSDK_STRING("Value of -tcbrctestfile is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-dispatcher:fullSearch"))) {
        InputParams.dispFullSearch = true;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-dispatcher:lowLatency"))) {
        InputParams.dispFullSearch = false;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-dec::sys"))) {
        InputParams.DecOutPattern = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    }

    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-HdrSEI:mdcv"))) {
        InputParams.bEnableMDCV = true;
        auto pInMDCV            = &(InputParams.SEIMetaMDCV);
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        mfxU32 arr[10] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        size_t k;
        std::vector<msdk_string> args;
        k = split(argv[i + 1], args, msdk_char(','));
        if (k != 10) {
            PrintError(argv[0], MSDK_STRING("Invalid number of layers for MDCV"));
            return MFX_ERR_UNSUPPORTED;
        }

        for (int j = 0; j < 8; j++) {
            try {
                arr[j] = (mfxU32)std::stoul(args[j]);
            }
            catch (const std::invalid_argument&) {
                msdk_printf(MSDK_STRING("Error reading MDCV Layers setting.\n"));
                return MFX_ERR_UNSUPPORTED;
            }
            catch (const std::out_of_range&) {
                msdk_printf(MSDK_STRING("MDCV Layers setting out of bounds.\n"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        pInMDCV->DisplayPrimariesX[0]         = arr[0];
        pInMDCV->DisplayPrimariesX[1]         = arr[1];
        pInMDCV->DisplayPrimariesX[2]         = arr[2];
        pInMDCV->DisplayPrimariesY[0]         = arr[3];
        pInMDCV->DisplayPrimariesY[1]         = arr[4];
        pInMDCV->DisplayPrimariesY[2]         = arr[5];
        pInMDCV->WhitePointX                  = arr[6];
        pInMDCV->WhitePointY                  = arr[7];
        pInMDCV->MaxDisplayMasteringLuminance = arr[8];
        pInMDCV->MinDisplayMasteringLuminance = arr[9];
        i += 1;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-HdrSEI:clli"))) {
        InputParams.bEnableCLLI = true;
        auto pInCLLI            = &(InputParams.SEIMetaCLLI);
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        int k;
        k = msdk_sscanf(argv[i + 1],
                        MSDK_STRING("%hu,%hu"),
                        &(pInCLLI->MaxContentLightLevel),
                        &(pInCLLI->MaxPicAverageLightLevel));
        if (k != 2) {
            PrintError(argv[0], MSDK_STRING("Invalid number of layers for CLLI"));
            return MFX_ERR_UNSUPPORTED;
        }
        i += 1;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-SignalInfoIn"))) {
        auto pSignalInfoIn     = &(InputParams.SignalInfoIn);
        pSignalInfoIn->Enabled = true;
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        int k;
        k = msdk_sscanf(argv[i + 1],
                        MSDK_STRING("%hu,%hu"),
                        &(pSignalInfoIn->VideoFullRange),
                        &(pSignalInfoIn->ColourPrimaries));
        if (k != 2) {
            PrintError(argv[0], MSDK_STRING("Invalid number of layers for SignalInfoIn"));
            return MFX_ERR_UNSUPPORTED;
        }
        i += 1;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-SignalInfoOut"))) {
        auto pSignalInfoOut     = &(InputParams.SignalInfoOut);
        pSignalInfoOut->Enabled = true;
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        int k;
        k = msdk_sscanf(argv[i + 1],
                        MSDK_STRING("%hu,%hu"),
                        &(pSignalInfoOut->VideoFullRange),
                        &(pSignalInfoOut->ColourPrimaries));
        if (k != 2) {
            PrintError(argv[0], MSDK_STRING("Invalid number of layers for SignalInfoOut"));
            return MFX_ERR_UNSUPPORTED;
        }
        i += 1;
    }

#ifdef ENABLE_MCTF
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-mctf"))) {
        ParseMCTFParams(argv, argc, i, &InputParams);
    }
#endif
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-ext_allocator")) ||
             0 == msdk_strcmp(argv[i], MSDK_STRING("-MemType::video"))) {
        InputParams.bForceSysMem = false;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-sys")) ||
             0 == msdk_strcmp(argv[i], MSDK_STRING("-MemType::system"))) {
        InputParams.bForceSysMem = true;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-opaq")) ||
             0 == msdk_strcmp(argv[i], MSDK_STRING("-MemType::opaque"))) {
        msdk_printf(MSDK_STRING(
            "WARNING: -opaq option is ignored, opaque memory support is disabled in opeVPL.\n"));
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-memory"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nMemoryModel)) {
            PrintError(MSDK_STRING("-memory %s is invalid"), argv[i]);
            return MFX_ERR_UNSUPPORTED;
        }
        if (InputParams.nMemoryModel < 1 || InputParams.nMemoryModel > 3) {
            PrintError(MSDK_STRING(" \"%s\" memory type is invalid. Default (1) will be used."),
                       argv[i]);
            InputParams.nMemoryModel = GENERAL_ALLOC;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-MemModel::GeneralAlloc"))) {
        InputParams.nMemoryModel = GENERAL_ALLOC;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-MemModel::VisibleIntAlloc"))) {
        InputParams.nMemoryModel = VISIBLE_INT_ALLOC;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-MemModel::HiddenIntAlloc"))) {
        InputParams.nMemoryModel = HIDDEN_INT_ALLOC;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-AllocPolicy::optimal"))) {
        InputParams.AllocPolicy   = MFX_ALLOCATION_OPTIMAL;
        InputParams.useAllocHints = true;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-AllocPolicy::limited"))) {
        InputParams.AllocPolicy   = MFX_ALLOCATION_LIMITED;
        InputParams.useAllocHints = true;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-AllocPolicy::unlimited"))) {
        InputParams.AllocPolicy   = MFX_ALLOCATION_UNLIMITED;
        InputParams.useAllocHints = true;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-preallocate"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);

        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.preallocate)) {
            PrintError(MSDK_STRING("preallocate param is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-TargetBitDepthLuma"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);

        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.TargetBitDepthLuma)) {
            PrintError(MSDK_STRING("TargetBitDepthLuma param is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-TargetBitDepthChroma"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);

        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.TargetBitDepthChroma)) {
            PrintError(MSDK_STRING("TargetBitDepthChroma param is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-cs"))) {
        InputParams.CascadeScaler = true;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-trace"))) {
        InputParams.EnableTracing = true;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-trace_buffer_size"))) {
        VAL_CHECK(i + 1 == argc, i, argv[i]);
        i++;
        if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.TraceBufferSize)) {
            PrintError(MSDK_STRING("-trace_buffer_size \"%s\" is invalid"), argv[i]);
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-trace::E2E"))) {
        InputParams.EnableTracing = true;
        InputParams.LatencyType   = SMTTracer::LatencyType::E2E;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-trace::ENC"))) {
        InputParams.EnableTracing = true;
        InputParams.LatencyType   = SMTTracer::LatencyType::ENC;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-parallel_encoding"))) {
        InputParams.ParallelEncoding = true;
    }
#if (defined(_WIN64) || defined(_WIN32))
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-dual_gfx::on"))) {
        InputParams.isDualMode = true;
        InputParams.hyperMode  = MFX_HYPERMODE_ON;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-dual_gfx::off"))) {
        InputParams.isDualMode = true;
        InputParams.hyperMode  = MFX_HYPERMODE_OFF;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-dual_gfx::adaptive"))) {
        InputParams.isDualMode = true;
        InputParams.hyperMode  = MFX_HYPERMODE_ADAPTIVE;
    }
#endif
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-idr_interval"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nIdrInterval)) {
            PrintError(argv[0], MSDK_STRING("IdrInterval is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-EmbeddedDenoise"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.EmbeddedDenoiseMode)) {
            PrintError(argv[0], MSDK_STRING("DenoiseMode is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.EmbeddedDenoiseLevel)) {
            PrintError(argv[0], MSDK_STRING("DenoiseLevel is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
        InputParams.bEmbeddedDenoiser = true;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-MinQPI"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nMinQPI)) {
            PrintError(MSDK_STRING("Min Quantizer for I frames is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-MinQPP"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nMinQPP)) {
            PrintError(MSDK_STRING("Min Quantizer for P frames is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-MinQPB"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nMinQPB)) {
            PrintError(MSDK_STRING("Min Quantizer for B frames is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-MaxQPI"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nMaxQPI)) {
            PrintError(MSDK_STRING("Max Quantizer for I frames is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-MaxQPP"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nMaxQPP)) {
            PrintError(MSDK_STRING("Max Quantizer for P frames is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-MaxQPB"))) {
        VAL_CHECK(i + 1 >= argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nMaxQPB)) {
            PrintError(MSDK_STRING("Max Quantizer for B frames is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-NumActiveRefP"))) {
        VAL_CHECK(i + 1 == argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nNumRefActiveP)) {
            PrintError(
                MSDK_STRING("Number of active reference frames for P frames \"%s\" is invalid"),
                argv[i]);
            return MFX_ERR_UNSUPPORTED;
        }
    }
#ifdef ONEVPL_EXPERIMENTAL
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-pci"))) {
        msdk_string deviceInfo;
        VAL_CHECK(i + 1 == argc, i, argv[i]);
        i++;
        if (MFX_ERR_NONE != msdk_opt_read(argv[i], deviceInfo)) {
            PrintError(MSDK_STRING("-pci is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }

        // template: <domain:bus:device.function>
        std::string temp = std::string(deviceInfo.begin(), deviceInfo.end());
        const std::regex pieces_regex("([0-9]+):([0-9]+):([0-9]+).([0-9]+)");
        std::smatch pieces_match;

        // pieces_match = [full match, PCIDomain, PCIBus, PCIDevice, PCIFunction]
        if (std::regex_match(temp, pieces_match, pieces_regex) && pieces_match.size() == 5) {
            InputParams.PCIDomain      = std::atoi(pieces_match[1].str().c_str());
            InputParams.PCIBus         = std::atoi(pieces_match[2].str().c_str());
            InputParams.PCIDevice      = std::atoi(pieces_match[3].str().c_str());
            InputParams.PCIFunction    = std::atoi(pieces_match[4].str().c_str());
            InputParams.PCIDeviceSetup = true;
        }
        else {
            PrintError(
                MSDK_STRING(
                    "format of -pci \"%s\" is invalid, please, use: domain:bus:device.function"),
                argv[i]);
            return MFX_ERR_UNSUPPORTED;
        }
    }
    #if defined(_WIN32)
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-LUID"))) {
        // <HighPart:LowPart>
        msdk_string luid;
        VAL_CHECK(i + 1 == argc, i, argv[i]);
        i++;
        if (MFX_ERR_NONE != msdk_opt_read(argv[i], luid)) {
            PrintError(MSDK_STRING("-LUID is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }

        std::string temp = std::string(luid.begin(), luid.end());
        const std::regex pieces_regex("(0[xX][0-9a-fA-F]+):(0[xX][0-9a-fA-F]+)");
        std::smatch pieces_match;

        // pieces_match = [full match, HighPart, LowPart]
        if (std::regex_match(temp, pieces_match, pieces_regex) && pieces_match.size() == 3) {
            InputParams.luid.HighPart = std::strtol(pieces_match[1].str().c_str(), 0, 16);
            InputParams.luid.LowPart  = std::strtol(pieces_match[2].str().c_str(), 0, 16);
        }
        else {
            PrintError(
                MSDK_STRING("format of -LUID \"%s\" is invalid, please, use: HighPart:LowPart"),
                argv[i]);
            return MFX_ERR_UNSUPPORTED;
        }
    }
    #endif
#endif
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-ivf:on"))) {
        InputParams.nIVFHeader = MFX_CODINGOPTION_ON;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-ivf:off"))) {
        InputParams.nIVFHeader = MFX_CODINGOPTION_OFF;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-msb10"))) {
        InputParams.IsSourceMSB = true;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-syncop_timeout"))) {
        VAL_CHECK(i + 1 == argc, i, argv[i]);
        if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nSyncOpTimeout)) {
            PrintError(MSDK_STRING("syncop_timeout is invalid"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-api_ver_init::1x"))) {
        InputParams.verSessionInit = API_1X;
    }
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-api_ver_init::2x"))) {
        InputParams.verSessionInit = API_2X;
    }
#ifdef ONEVPL_EXPERIMENTAL
    else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-perc_enc_filter"))) {
        InputParams.PercEncPrefilter = true;
    }
#endif
    else {
        // no matching argument was found
        return MFX_ERR_NOT_FOUND;
    }

    return MFX_ERR_NONE;
}

mfxStatus ParseVPPCmdLine(msdk_char* argv[],
                          mfxU32 argc,
                          mfxU32& index,
                          TranscodingSample::sInputParams* params,
                          mfxU32& skipped) {
    if (0 == msdk_strcmp(argv[index], MSDK_STRING("-VppHvsDenoise"))) {
        VAL_CHECK(index + 1 == argc, index, argv[index]);
        index++;
        if (MFX_ERR_NONE != msdk_opt_read(argv[index], params->VppDenoiseMode)) {
            PrintError(NULL, MSDK_STRING("-vpp denoise mode\"%s\" is invalid"), argv[index]);
            return MFX_ERR_UNSUPPORTED;
        }
        index++;
        if (MFX_ERR_NONE != msdk_opt_read(argv[index], params->VppDenoiseLevel) ||
            !(params->VppDenoiseLevel >= 0 && params->VppDenoiseLevel <= 100)) {
            PrintError(NULL, MSDK_STRING("-vpp denoise level\"%s\" is invalid"), argv[index]);
            return MFX_ERR_UNSUPPORTED;
        }
        if (!((params->VppDenoiseMode == 0) ||
              (params->VppDenoiseMode > MFX_DENOISE_MODE_VENDOR &&
               params->VppDenoiseMode < MFX_DENOISE_MODE_INTEL_HVS_POST_MANUAL + 1))) {
            PrintError(NULL,
                       MSDK_STRING("-vpp denoise mode\"%s\" is invalid"),
                       params->VppDenoiseMode);
            return MFX_ERR_UNSUPPORTED;
        }
        params->bVppDenoiser = true;
        skipped += 2;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-denoise"))) {
        VAL_CHECK(index + 1 == argc, index, argv[index]);
        index++;
        if (MFX_ERR_NONE != msdk_opt_read(argv[index], params->VppDenoiseLevel) ||
            !(params->VppDenoiseLevel >= 0 && params->VppDenoiseLevel <= 100)) {
            PrintError(NULL, MSDK_STRING("-vpp denoise level\"%s\" is invalid"), argv[index]);
            return MFX_ERR_UNSUPPORTED;
        }
        skipped += 2;
        return MFX_ERR_NONE;
    }

    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-detail"))) {
        VAL_CHECK(index + 1 == argc, index, argv[index]);
        index++;
        if (MFX_ERR_NONE != msdk_opt_read(argv[index], params->DetailLevel) ||
            !(params->DetailLevel >= 0 && params->DetailLevel <= 100)) {
            PrintError(NULL, MSDK_STRING("-detail \"%s\" is invalid"), argv[index]);
            return MFX_ERR_UNSUPPORTED;
        }
        skipped += 2;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-FRC::PT"))) {
        params->FRCAlgorithm = MFX_FRCALGM_PRESERVE_TIMESTAMP;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-FRC::DT"))) {
        params->FRCAlgorithm = MFX_FRCALGM_DISTRIBUTED_TIMESTAMP;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-FRC::INTERP"))) {
        params->FRCAlgorithm = MFX_FRCALGM_FRAME_INTERPOLATION;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-deinterlace"))) {
        params->bEnableDeinterlacing = true;
        params->DeinterlacingMode    = 0;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-deinterlace::ADI"))) {
        params->bEnableDeinterlacing = true;
        params->DeinterlacingMode    = MFX_DEINTERLACING_ADVANCED;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-deinterlace::ADI_SCD"))) {
        params->bEnableDeinterlacing = true;
        params->DeinterlacingMode    = MFX_DEINTERLACING_ADVANCED_SCD;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-deinterlace::BOB"))) {
        params->bEnableDeinterlacing = true;
        params->DeinterlacingMode    = MFX_DEINTERLACING_BOB;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-deinterlace::ADI_NO_REF"))) {
        params->bEnableDeinterlacing = true;
        params->DeinterlacingMode    = MFX_DEINTERLACING_ADVANCED_NOREF;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-scaling_mode"))) {
        VAL_CHECK(index + 1 == argc, index, argv[index]);
        index++;
        if (0 == msdk_strcmp(argv[index], MSDK_STRING("lowpower")))
            params->ScalingMode = MFX_SCALING_MODE_LOWPOWER;
        else if (0 == msdk_strcmp(argv[index], MSDK_STRING("quality")))
            params->ScalingMode = MFX_SCALING_MODE_QUALITY;
        else if (0 == msdk_strcmp(argv[index], MSDK_STRING("eu")))
            params->ScalingMode = MFX_SCALING_MODE_INTEL_GEN_COMPUTE;
        else {
            PrintError(NULL, MSDK_STRING("-scaling_mode \"%s\" is invalid"), argv[index]);
            return MFX_ERR_UNSUPPORTED;
        }
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-ec::rgb4"))) {
        params->EncoderFourCC = MFX_FOURCC_RGB4;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-ec::yuy2"))) {
        params->EncoderFourCC = MFX_FOURCC_YUY2;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-ec::nv12"))) {
        params->EncoderFourCC = MFX_FOURCC_NV12;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-ec::nv16"))) {
        params->EncoderFourCC = MFX_FOURCC_NV16;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-ec::p010"))) {
        params->EncoderFourCC = MFX_FOURCC_P010;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-ec::p210"))) {
        params->EncoderFourCC = MFX_FOURCC_P210;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-dc::p010"))) {
        params->DecoderFourCC = MFX_FOURCC_P010;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-dc::p016"))) {
        params->DecoderFourCC = MFX_FOURCC_P016;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-dc::y216"))) {
        params->DecoderFourCC = MFX_FOURCC_Y216;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-dc::y416"))) {
        params->DecoderFourCC = MFX_FOURCC_Y216;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-ec::p016"))) {
        params->EncoderFourCC = MFX_FOURCC_P016;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-ec::y216"))) {
        params->EncoderFourCC = MFX_FOURCC_Y216;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-dc::rgb4"))) {
        params->DecoderFourCC = MFX_FOURCC_RGB4;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-dc::yuy2"))) {
        params->DecoderFourCC = MFX_FOURCC_YUY2;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-dc::nv12"))) {
        params->DecoderFourCC = MFX_FOURCC_NV12;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-dc::y210"))) {
        params->DecoderFourCC = MFX_FOURCC_Y210;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-dc::y410"))) {
        params->DecoderFourCC = MFX_FOURCC_Y410;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-ec::y210"))) {
        params->EncoderFourCC = MFX_FOURCC_Y210;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-ec::y410"))) {
        params->EncoderFourCC = MFX_FOURCC_Y410;
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-field_processing"))) {
        VAL_CHECK(index + 1 == argc, index, argv[index]);
        index++;
        if (0 == msdk_strcmp(argv[index], MSDK_STRING("t2t")))
            params->fieldProcessingMode = FC_T2T;
        else if (0 == msdk_strcmp(argv[index], MSDK_STRING("t2b")))
            params->fieldProcessingMode = FC_T2B;
        else if (0 == msdk_strcmp(argv[index], MSDK_STRING("b2t")))
            params->fieldProcessingMode = FC_B2T;
        else if (0 == msdk_strcmp(argv[index], MSDK_STRING("b2b")))
            params->fieldProcessingMode = FC_B2B;
        else if (0 == msdk_strcmp(argv[index], MSDK_STRING("fr2fr")))
            params->fieldProcessingMode = FC_FR2FR;
        else {
            PrintError(NULL, MSDK_STRING("-field_processing \"%s\" is invalid"), argv[index]);
            return MFX_ERR_UNSUPPORTED;
        }
        return MFX_ERR_NONE;
    }
    else if (0 == msdk_strcmp(argv[index], MSDK_STRING("-3dlut"))) {
        VAL_CHECK(index + 1 == argc, index, argv[index]);
        index++;
        // 3dlut file
        if (msdk_strlen(argv[index]) < MSDK_ARRAY_LEN(params->str3DLutFile)) {
            msdk_opt_read(argv[index], params->str3DLutFile);
            params->bEnable3DLut = true;
        }
        else {
            params->bEnable3DLut = false;
            return MFX_ERR_UNSUPPORTED;
        }
        return MFX_ERR_NONE;
    }

    return MFX_ERR_MORE_DATA;
}

mfxStatus CmdProcessor::ParseParamsForOneSession(mfxU32 argc, msdk_char* argv[]) {
    mfxStatus sts           = MFX_ERR_NONE;
    mfxStatus stsExtBuf     = MFX_ERR_NONE;
    mfxStatus stsAddlParams = MFX_ERR_NONE;
    mfxU32 skipped          = 0;

    // save original cmd line for debug purpose
    msdk_stringstream cmd;
    for (mfxU32 i = 0; i < argc; i++)
        cmd << argv[i] << MSDK_STRING(" ");
    m_lines.push_back(cmd.str());

    TranscodingSample::sInputParams InputParams;
    if (m_nTimeout)
        InputParams.nTimeout = m_nTimeout;
    if (bRobustFlag)
        InputParams.bRobustFlag = true;
    if (bSoftRobustFlag)
        InputParams.bSoftRobustFlag = true;

    InputParams.shouldUseGreedyFormula = shouldUseGreedyFormula;

    InputParams.statisticsWindowSize = statisticsWindowSize;
    InputParams.statisticsLogFile    = statisticsLogFile;

    //bind to a dump-log-file name
    InputParams.DumpLogFileName = DumpLogFileName;

    if (0 == msdk_strcmp(argv[0], MSDK_STRING("set"))) {
        if (argc != 3) {
            msdk_printf(MSDK_STRING("error: number of arguments for 'set' options is wrong"));
            return MFX_ERR_UNSUPPORTED;
        }
        sts = ParseOption__set(argv[1], argv[2]);
        return sts;
    }
    // default implementation
    InputParams.libType = MFX_IMPL_HARDWARE_ANY;
#if defined(_WIN32) || defined(_WIN64)
    InputParams.libType = MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D11;
#endif
    InputParams.eModeExt = Native;

    for (mfxU32 i = 0; i < argc; i++) {
        // process multi-character options
        if ((0 == msdk_strncmp(MSDK_STRING("-i::"), argv[i], msdk_strlen(MSDK_STRING("-i::")))) &&
            (0 != msdk_strncmp(argv[i] + 4,
                               MSDK_STRING("source"),
                               msdk_strlen(MSDK_STRING("source"))))) {
            sts = StrFormatToCodecFormatFourCC(argv[i] + 4, InputParams.DecodeId);
            if (sts != MFX_ERR_NONE) {
                return MFX_ERR_UNSUPPORTED;
            }
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            msdk_opt_read(argv[i], InputParams.strSrcFile);
            if (InputParams.eMode == Source) {
                switch (InputParams.DecodeId) {
                    case MFX_CODEC_MPEG2:
                    case MFX_CODEC_HEVC:
                    case MFX_CODEC_AVC:
                    case MFX_CODEC_VC1:
                    case MFX_CODEC_VP9:
                    case MFX_CODEC_AV1:
                    case CODEC_MVC:
                    case MFX_CODEC_JPEG:
                        return MFX_ERR_UNSUPPORTED;
                }
            }
            if (InputParams.DecodeId == CODEC_MVC) {
                InputParams.DecodeId = MFX_CODEC_AVC;
                InputParams.bIsMVC   = true;
            }
        }
        else if ((0 ==
                  msdk_strncmp(MSDK_STRING("-o::"), argv[i], msdk_strlen(MSDK_STRING("-o::")))) &&
                 (0 != msdk_strncmp(argv[i] + 4,
                                    MSDK_STRING("sink"),
                                    msdk_strlen(MSDK_STRING("sink"))))) {
            sts = StrFormatToCodecFormatFourCC(argv[i] + 4, InputParams.EncodeId);

            if (sts != MFX_ERR_NONE) {
                return MFX_ERR_UNSUPPORTED;
            }
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            msdk_opt_read(argv[i], InputParams.strDstFile);
            if (InputParams.eMode == Sink || InputParams.bIsMVC) {
                switch (InputParams.EncodeId) {
                    case MFX_CODEC_MPEG2:
                    case MFX_CODEC_HEVC:
                    case MFX_CODEC_AVC:
                    case MFX_CODEC_JPEG:
                    case MFX_CODEC_DUMP:
                        return MFX_ERR_UNSUPPORTED;
                }
            }
            if (InputParams.EncodeId == CODEC_MVC) {
                if (InputParams.eMode == Sink)
                    return MFX_ERR_UNSUPPORTED;

                InputParams.EncodeId = MFX_CODEC_AVC;
                InputParams.bIsMVC   = true;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-roi_file"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;

            msdk_string strRoiFile;
            msdk_opt_read(argv[i], strRoiFile);

            if (!ParseROIFile(strRoiFile, InputParams.m_ROIData)) {
                PrintError(MSDK_STRING("Incorrect ROI file: \"%s\" "), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-p"))) {
            if (m_PerfFILE) {
                msdk_printf(MSDK_STRING("error: only one performance file is supported"));
                return MFX_ERR_UNSUPPORTED;
            }
            --argc;
            ++argv;
            if (!argv[i]) {
                msdk_printf(MSDK_STRING("error: no argument given for '-p' option\n"));
            }
            MSDK_FOPEN(m_PerfFILE, argv[i], MSDK_STRING("w"));
            if (NULL == m_PerfFILE) {
                msdk_printf(MSDK_STRING("error: performance file \"%s\" not found"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-roi_qpmap"))) {
            InputParams.bROIasQPMAP = true;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-extmbqp"))) {
            InputParams.bExtMBQP = true;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-sw"))) {
            InputParams.libType = MFX_IMPL_SOFTWARE;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-hw"))) {
#if defined(_WIN32) || defined(_WIN64)
            InputParams.libType = MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D11;
#elif defined(LIBVA_SUPPORT)
            InputParams.libType = MFX_IMPL_HARDWARE_ANY;
#endif
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-hw_d3d11"))) {
            InputParams.libType = MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D11;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-hw_d3d9"))) {
            InputParams.libType = MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D9;
        }
#if (defined(LINUX32) || defined(LINUX64))
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-device"))) {
            if (!InputParams.strDevicePath.empty()) {
                msdk_printf(MSDK_STRING("error: you can specify only one device\n"));
                return MFX_ERR_UNSUPPORTED;
            }

            VAL_CHECK(i + 1 == argc, i, argv[i]);
            InputParams.strDevicePath = argv[++i];

            size_t pos = InputParams.strDevicePath.find("renderD");
            if (pos != std::string::npos) {
                InputParams.DRMRenderNodeNum =
                    std::stoi(InputParams.strDevicePath.substr(pos + 7, 3));
            }
        }
#endif
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-robust"))) {
            InputParams.bRobustFlag = true;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-robust:soft"))) {
            InputParams.bSoftRobustFlag = true;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-threads"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nThreadsNum)) {
                PrintError(MSDK_STRING("Threads number is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-f"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            // Temporary check for giving priority to -fe option
            if (!InputParams.dVPPOutFramerate) {
                if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.dVPPOutFramerate)) {
                    PrintError(MSDK_STRING("FrameRate \"%s\" is invalid"), argv[i]);
                    return MFX_ERR_UNSUPPORTED;
                }
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-fe"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.dVPPOutFramerate)) {
                PrintError(MSDK_STRING("FrameRate \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-fps"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nFPS)) {
                PrintError(MSDK_STRING("FPS limit \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-b"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nBitRate)) {
                PrintError(MSDK_STRING("BitRate \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-bm"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nBitRateMultiplier)) {
                PrintError(MSDK_STRING("Bitrate multiplier \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-wb"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.WinBRCMaxAvgKbps)) {
                PrintError(MSDK_STRING("Maximum bitrate for sliding window \"%s\" is invalid"),
                           argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-ws"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.WinBRCSize)) {
                PrintError(MSDK_STRING("Sliding window size \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-hrd"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.BufferSizeInKB)) {
                PrintError(MSDK_STRING("Frame buffer size \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-dist"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.GopRefDist)) {
                PrintError(MSDK_STRING("GOP reference distance \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-gop_size"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.GopPicSize)) {
                PrintError(MSDK_STRING("GOP size \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-num_ref"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.NumRefFrame)) {
                PrintError(MSDK_STRING("Number of reference frames \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-trows"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nEncTileRows)) {
                PrintError(MSDK_STRING("Encoding tile row count \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-tcols"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nEncTileCols)) {
                PrintError(MSDK_STRING("Encoding tile column count \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-CodecLevel"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.CodecLevel)) {
                PrintError(MSDK_STRING("CodecLevel \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-CodecProfile"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.CodecProfile)) {
                PrintError(MSDK_STRING("CodecProfile \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-MaxKbps"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.MaxKbps)) {
                PrintError(MSDK_STRING("MaxKbps \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-InitialDelayInKB"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.InitialDelayInKB)) {
                PrintError(MSDK_STRING("InitialDelayInKB \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-GopOptFlag:closed"))) {
            InputParams.GopOptFlag = MFX_GOP_CLOSED;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-GopOptFlag:strict"))) {
            InputParams.GopOptFlag = MFX_GOP_STRICT;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-bref"))) {
            InputParams.nBRefType = MFX_B_REF_PYRAMID;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-nobref"))) {
            InputParams.nBRefType = MFX_B_REF_OFF;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-gpb:on"))) {
            InputParams.GPB = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-gpb:off"))) {
            InputParams.GPB = MFX_CODINGOPTION_OFF;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-TransformSkip:on"))) {
            InputParams.nTransformSkip = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-TransformSkip:off"))) {
            InputParams.nTransformSkip = MFX_CODINGOPTION_OFF;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-u"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            InputParams.nTargetUsage = StrToTargetUsage(argv[i]);
            if (!InputParams.nTargetUsage) {
                PrintError(MSDK_STRING(" \"%s\" target usage is invalid. Balanced will be used."),
                           argv[i]);
                InputParams.nTargetUsage = MFX_TARGETUSAGE_BALANCED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-WeightedPred::default"))) {
            InputParams.WeightedPred = MFX_WEIGHTED_PRED_DEFAULT;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-WeightedPred::implicit"))) {
            InputParams.WeightedPred = MFX_WEIGHTED_PRED_IMPLICIT;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-WeightedBiPred::default"))) {
            InputParams.WeightedBiPred = MFX_WEIGHTED_PRED_DEFAULT;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-WeightedBiPred::implicit"))) {
            InputParams.WeightedBiPred = MFX_WEIGHTED_PRED_IMPLICIT;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-q"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nQuality)) {
                PrintError(MSDK_STRING(" \"%s\" quality is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-w"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nDstWidth)) {
                PrintError(MSDK_STRING("width \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-h"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nDstHeight)) {
                PrintError(MSDK_STRING("height \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-l"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nSlices)) {
                PrintError(MSDK_STRING("numSlices \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-mss"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nMaxSliceSize)) {
                PrintError(MSDK_STRING("maxSliceSize \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-async"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nAsyncDepth)) {
                PrintError(MSDK_STRING("async \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-join"))) {
            InputParams.bIsJoin = true;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-priority"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.priority)) {
                PrintError(MSDK_STRING("priority \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-i::source"))) {
            if (InputParams.eMode != Native) {
                PrintError(NULL, "-i::source cannot be used here");
                return MFX_ERR_UNSUPPORTED;
            }

            InputParams.eMode = Source;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-o::sink"))) {
            if (InputParams.eMode != Native) {
                PrintError(NULL, "-o::sink cannot be used here");
                return MFX_ERR_UNSUPPORTED;
            }

            InputParams.eMode = Sink;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vpp_comp"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            /* NB! numSurf4Comp should be equal to Number of decoding session */
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.numSurf4Comp)) {
                PrintError(MSDK_STRING("-n \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
            /* This is can init early */
            if (InputParams.eModeExt == Native)
                InputParams.eModeExt = VppComp;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vpp_comp_only"))) {
            /* for VPP comp with rendering we have to use ext allocator */
#ifdef LIBVA_SUPPORT
            InputParams.libvaBackend = MFX_LIBVA_DRM;
#endif

            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            /* NB! numSurf4Comp should be equal to Number of decoding session */
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.numSurf4Comp)) {
                PrintError(MSDK_STRING("-n \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
            /* This is can init early */
            if (InputParams.eModeExt == Native)
                InputParams.eModeExt = VppCompOnly;
        }
        else if (0 == msdk_strncmp(MSDK_STRING("-vpp_comp_dump"),
                                   argv[i],
                                   msdk_strlen(MSDK_STRING("-vpp_comp_dump")))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            msdk_opt_read(argv[i], InputParams.strDumpVppCompFile);
        }
#if defined(LIBVA_X11_SUPPORT)
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-rx11"))) {
            InputParams.libvaBackend = MFX_LIBVA_X11;
        }
#endif

#if defined(LIBVA_WAYLAND_SUPPORT)
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-rwld"))) {
            InputParams.nRenderWinX  = 0;
            InputParams.nRenderWinY  = 0;
            InputParams.bPerfMode    = false;
            InputParams.libvaBackend = MFX_LIBVA_WAYLAND;
        }
#endif

#if defined(LIBVA_DRM_SUPPORT)
        else if (0 == msdk_strncmp(argv[i], MSDK_STRING("-rdrm"), 5)) {
            InputParams.libvaBackend = MFX_LIBVA_DRM_MODESET;
            InputParams.monitorType  = getMonitorType(&argv[i][5]);
            if (argv[i][5]) {
                if (argv[i][5] != '-') {
                    PrintError(MSDK_STRING("unsupported monitor type"));
                    return MFX_ERR_UNSUPPORTED;
                }
                InputParams.monitorType = getMonitorType(&argv[i][6]);
                if (InputParams.monitorType >= MFX_MONITOR_MAXNUMBER) {
                    PrintError(MSDK_STRING("unsupported monitor type"));
                    return MFX_ERR_UNSUPPORTED;
                }
            }
            else {
                InputParams.monitorType = MFX_MONITOR_AUTO; // that's case of "-rdrm" pure option
            }
        }
#endif
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vpp::sys"))) {
            InputParams.VppOutPattern = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vpp::vid"))) {
            InputParams.VppOutPattern = MFX_IOPATTERN_OUT_VIDEO_MEMORY;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vpp_comp_dst_x"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nVppCompDstX)) {
                PrintError(MSDK_STRING("vpp_comp_dst_x %s is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
            if (InputParams.eModeExt != VppComp)
                InputParams.eModeExt = VppComp;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vpp_comp_dst_y"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nVppCompDstY)) {
                PrintError(MSDK_STRING("-vpp_comp_dst_y %s is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
            if (InputParams.eModeExt != VppComp)
                InputParams.eModeExt = VppComp;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vpp_comp_dst_w"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nVppCompDstW)) {
                PrintError(MSDK_STRING("-vpp_comp_dst_w %s is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
            if (InputParams.eModeExt != VppComp)
                InputParams.eModeExt = VppComp;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vpp_comp_dst_h"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nVppCompDstH)) {
                PrintError(MSDK_STRING("-vpp_comp_dst_h %s is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
            if (InputParams.eModeExt != VppComp)
                InputParams.eModeExt = VppComp;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vpp_comp_src_w"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nVppCompSrcW)) {
                PrintError(MSDK_STRING("-vpp_comp_src_w %s is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
            if (InputParams.eModeExt != VppComp)
                InputParams.eModeExt = VppComp;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vpp_comp_src_h"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nVppCompSrcH)) {
                PrintError(MSDK_STRING("-vpp_comp_src_h %s is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
            if (InputParams.eModeExt != VppComp)
                InputParams.eModeExt = VppComp;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vpp_comp_num_tiles"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.numTiles4Comp)) {
                PrintError(MSDK_STRING("-vpp_comp_num_tiles %s is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
            if (InputParams.eModeExt != VppCompOnly)
                InputParams.eModeExt = VppCompOnly;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vpp_comp_tile_id"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nVppCompTileId)) {
                PrintError(MSDK_STRING("-vpp_comp_tile_id %s is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vpp_comp_render"))) {
            if (InputParams.eModeExt != VppComp)
                InputParams.eModeExt = VppComp;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-dec_postproc"))) {
            InputParams.bDecoderPostProcessing = true;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-n"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.MaxFrameNumber)) {
                PrintError(MSDK_STRING("-n %s is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-angle"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nRotationAngle)) {
                PrintError(MSDK_STRING("-angle %s is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
            if (!InputParams.strVPPPluginDLLPath[0]) {
                msdk_opt_read(MSDK_CPU_ROTATE_PLUGIN, InputParams.strVPPPluginDLLPath);
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-mfe_frames"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.numMFEFrames)) {
                PrintError(MSDK_STRING("-mfe_frames %s num frames is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-mfe_mode"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.MFMode)) {
                PrintError(MSDK_STRING("-mfe_mode %s is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-mfe_timeout"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.mfeTimeout)) {
                PrintError(MSDK_STRING("-mfe_timeout %s is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-timeout"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nTimeout)) {
                PrintError(MSDK_STRING("-timeout %s is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
            skipped += 2;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-dump"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.strMfxParamsDumpFile)) {
                PrintError(MSDK_STRING("Dump file name \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-robust"))) {
            InputParams.bRobustFlag = true;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-opencl"))) {
            msdk_opt_read(MSDK_OCL_ROTATE_PLUGIN, InputParams.strVPPPluginDLLPath);
            InputParams.bOpenCL = true;
        }

        // output PicStruct
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-la_ext"))) {
            InputParams.bEnableExtLA = true;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-la"))) {
            InputParams.bLABRC             = true;
            InputParams.nRateControlMethod = MFX_RATECONTROL_LA;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vbr"))) {
            InputParams.nRateControlMethod = MFX_RATECONTROL_VBR;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-cbr"))) {
            InputParams.nRateControlMethod = MFX_RATECONTROL_CBR;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-bpyr"))) {
            InputParams.bEnableBPyramid = true;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-vcm"))) {
            InputParams.nRateControlMethod = MFX_RATECONTROL_VCM;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-lad"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.nLADepth)) {
                PrintError(MSDK_STRING("look ahead depth \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-pe"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            InputParams.encoderPluginParams = ParsePluginGuid(argv[i + 1]);
            if (AreGuidsEqual(InputParams.encoderPluginParams.pluginGuid, MSDK_PLUGINGUID_NULL)) {
                PrintError(MSDK_STRING("Invalid encoder guid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
            i++;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-pd"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            InputParams.decoderPluginParams = ParsePluginGuid(argv[i + 1]);
            if (AreGuidsEqual(InputParams.decoderPluginParams.pluginGuid, MSDK_PLUGINGUID_NULL)) {
                PrintError(MSDK_STRING("Invalid decoder guid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
            i++;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-override_decoder_framerate"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.dDecoderFrameRateOverride)) {
                PrintError(MSDK_STRING("Framerate \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-override_encoder_framerate"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.dEncoderFrameRateOverride)) {
                PrintError(MSDK_STRING("Framerate \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-override_encoder_picstruct"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            i++;
            if (MFX_ERR_NONE != msdk_opt_read(argv[i], InputParams.EncoderPicstructOverride)) {
                PrintError(MSDK_STRING("Picstruct \"%s\" is invalid"), argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-gpucopy::on"))) {
            InputParams.nGpuCopyMode = MFX_GPUCOPY_ON;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-gpucopy::off"))) {
            InputParams.nGpuCopyMode = MFX_GPUCOPY_OFF;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-repartitioncheck::on"))) {
            InputParams.RepartitionCheckMode = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-repartitioncheck::off"))) {
            InputParams.RepartitionCheckMode = MFX_CODINGOPTION_OFF;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-cqp"))) {
            InputParams.nRateControlMethod = MFX_RATECONTROL_CQP;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-qpi"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nQPI)) {
                PrintError(MSDK_STRING("Quantizer for I frames is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-qpp"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nQPP)) {
                PrintError(MSDK_STRING("Quantizer for P frames is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-qpb"))) {
            VAL_CHECK(i + 1 == argc, i, argv[i]);
            if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nQPB)) {
                PrintError(MSDK_STRING("Quantizer for B frames is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-DisableQPOffset"))) {
            InputParams.bDisableQPOffset = true;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-qsv-ff"))) {
            InputParams.enableQSVFF = true;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-single_texture_d3d11"))) {
            InputParams.bSingleTexture = true;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-extbrc::on"))) {
            InputParams.nExtBRC = EXTBRC_ON;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-extbrc::off"))) {
            InputParams.nExtBRC = EXTBRC_OFF;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-extbrc::implicit"))) {
            InputParams.nExtBRC = EXTBRC_IMPLICIT;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-ExtBrcAdaptiveLTR:on"))) {
            InputParams.ExtBrcAdaptiveLTR = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-ExtBrcAdaptiveLTR:off"))) {
            InputParams.ExtBrcAdaptiveLTR = MFX_CODINGOPTION_OFF;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-pp"))) {
            InputParams.shouldPrintPresets = true;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-preset"))) {
            msdk_string presetName;
            VAL_CHECK(i + 1 >= argc, i, argv[i]);
            if (MFX_ERR_NONE != msdk_opt_read(argv[++i], presetName)) {
                PrintError(MSDK_STRING("Preset Name is not defined"));
                return MFX_ERR_UNSUPPORTED;
            }

            InputParams.PresetMode = CPresetManager::PresetNameToMode(presetName);
            if (InputParams.PresetMode == PRESET_MAX_MODES) {
                PrintError(MSDK_STRING("Preset Name is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-forceSyncAllSession:on"))) {
            InputParams.forceSyncAllSession = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-forceSyncAllSession:off"))) {
            InputParams.forceSyncAllSession = MFX_CODINGOPTION_OFF;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-ir_type"))) {
            VAL_CHECK(i + 1 >= argc, i, argv[i]);

            if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.IntRefType)) {
                PrintError(MSDK_STRING("Intra refresh type is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-ir_cycle_size"))) {
            VAL_CHECK(i + 1 >= argc, i, argv[i]);

            if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.IntRefCycleSize)) {
                PrintError(MSDK_STRING("IR refresh cycle size param is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-ir_qp_delta"))) {
            VAL_CHECK(i + 1 >= argc, i, argv[i]);

            if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.IntRefQPDelta)) {
                PrintError(MSDK_STRING("IR QP delta param is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-ir_cycle_dist"))) {
            VAL_CHECK(i + 1 >= argc, i, argv[i]);

            if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.IntRefCycleDist)) {
                PrintError(MSDK_STRING("IR cycle distance param is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-LowDelayBRC"))) {
            InputParams.LowDelayBRC = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-amfs:on"))) {
            InputParams.nAdaptiveMaxFrameSize = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-amfs:off"))) {
            InputParams.nAdaptiveMaxFrameSize = MFX_CODINGOPTION_OFF;
        }
        else if (0 == msdk_strcmp(argv[i], MSDK_STRING("-mfs"))) {
            VAL_CHECK(i + 1 >= argc, i, argv[i]);
            if (MFX_ERR_NONE != msdk_opt_read(argv[++i], InputParams.nMaxFrameSize)) {
                PrintError(MSDK_STRING("MaxFrameSize is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        MOD_SMT_PARSE_INPUT
        else if ((stsExtBuf = ParseVPPCmdLine(argv, argc, i, &InputParams, skipped)) !=
                 MFX_ERR_MORE_DATA) {
            if (stsExtBuf == MFX_ERR_UNSUPPORTED) {
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else {
            // WA for compiler error C1061 (too many chained else-if clauses)
            // ParseAdditionalParams returns:
            //    0  if argv[i] is processed successfully by this function
            //  < 1  if argv[i] is processed and generates an error
            //          OR
            //       if argv[i] was not processed (did not match any switches)
            stsAddlParams = ParseAdditionalParams(argv, argc, i, InputParams);

            // either unrecognized parameter, or parse error with recognized parameter
            if (stsAddlParams) {
                if (stsAddlParams == MFX_ERR_NOT_FOUND)
                    PrintError(MSDK_STRING("Invalid input argument number %d %s"), i, argv[i]);
                return MFX_ERR_UNSUPPORTED;
            }
        }
    }

    if (skipped < argc) {
        sts = VerifyAndCorrectInputParams(InputParams);
        MSDK_CHECK_STATUS(sts, "VerifyAndCorrectInputParams failed");
        m_SessionArray.push_back(InputParams);
    }

    return MFX_ERR_NONE;

} //mfxStatus CmdProcessor::ParseParamsForOneSession(msdk_char *pLine, mfxU32 length)

mfxStatus CmdProcessor::ParseOption__set(msdk_char* strCodecType, msdk_char* strPluginGuid) {
    mfxU32 codecid = 0;
    mfxU32 type    = 0;
    sPluginParams pluginParams;

    //Parse codec type - decoder or encoder
    if (0 == msdk_strncmp(MSDK_STRING("-i::"), strCodecType, 4)) {
        type = MSDK_VDECODE;
    }
    else if (0 == msdk_strncmp(MSDK_STRING("-o::"), strCodecType, 4)) {
        type = MSDK_VENCODE;
    }
    else {
        msdk_printf(MSDK_STRING("error: incorrect definition codec type (must be -i:: or -o::)\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (StrFormatToCodecFormatFourCC(strCodecType + 4, codecid) != MFX_ERR_NONE) {
        msdk_printf(MSDK_STRING("error: codec is unknown\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (!IsPluginCodecSupported(codecid)) {
        msdk_printf(MSDK_STRING("error: codec is unsupported\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    pluginParams = ParsePluginGuid(strPluginGuid);
    if (AreGuidsEqual(pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL)) {
        msdk_printf(MSDK_STRING("error: invalid codec guid\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (type == MSDK_VDECODE)
        m_decoderPlugins.insert(std::pair<mfxU32, sPluginParams>(codecid, pluginParams));
    else
        m_encoderPlugins.insert(std::pair<mfxU32, sPluginParams>(codecid, pluginParams));

    return MFX_ERR_NONE;
};

mfxStatus CmdProcessor::VerifyAndCorrectInputParams(TranscodingSample::sInputParams& InputParams) {
    if (InputParams.strSrcFile.empty() &&
        (InputParams.eMode == Sink || InputParams.eMode == Native)) {
        PrintError(MSDK_STRING("Source file name not found"));
        return MFX_ERR_UNSUPPORTED;
    };

    if (InputParams.strDstFile.empty() &&
        (InputParams.eMode == Source || InputParams.eMode == Native ||
         InputParams.eMode == VppComp) &&
        InputParams.eModeExt != VppCompOnly) {
        PrintError(MSDK_STRING("Destination file name not found"));
        return MFX_ERR_UNSUPPORTED;
    };

    if (MFX_CODEC_JPEG != InputParams.EncodeId && MFX_CODEC_MPEG2 != InputParams.EncodeId &&
        MFX_CODEC_AVC != InputParams.EncodeId && MFX_CODEC_HEVC != InputParams.EncodeId &&
        MFX_CODEC_VP9 != InputParams.EncodeId && MFX_CODEC_DUMP != InputParams.EncodeId &&
        MFX_CODEC_AV1 != InputParams.EncodeId && InputParams.eMode != Sink &&
        InputParams.eModeExt != VppCompOnly) {
        PrintError(MSDK_STRING("Unknown encoder\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_CODEC_MPEG2 != InputParams.DecodeId && MFX_CODEC_AVC != InputParams.DecodeId &&
        MFX_CODEC_HEVC != InputParams.DecodeId && MFX_CODEC_VC1 != InputParams.DecodeId &&
        MFX_CODEC_JPEG != InputParams.DecodeId && MFX_CODEC_VP9 != InputParams.DecodeId &&
        MFX_CODEC_VP8 != InputParams.DecodeId && MFX_CODEC_AV1 != InputParams.DecodeId &&
        MFX_CODEC_RGB4 != InputParams.DecodeId && MFX_CODEC_NV12 != InputParams.DecodeId &&
        MFX_CODEC_I420 != InputParams.DecodeId && MFX_CODEC_P010 != InputParams.DecodeId &&
        InputParams.eMode != Source) {
        PrintError(MSDK_STRING("Unknown decoder\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_CODEC_I420 == InputParams.DecodeId || MFX_CODEC_NV12 == InputParams.DecodeId ||
        MFX_CODEC_P010 == InputParams.DecodeId) {
        if (InputParams.nMemoryModel == VISIBLE_INT_ALLOC ||
            InputParams.nMemoryModel == HIDDEN_INT_ALLOC) {
            PrintError(MSDK_STRING("raw input is not supported with memory model 2.0\n"));
            return MFX_ERR_UNSUPPORTED;
        }
        msdk_printf(MSDK_STRING("warning: system memory will be used\n"));
        InputParams.rawInput = true;
    }
    else {
        InputParams.rawInput = false;
    }

    if (MFX_CODEC_RGB4 == InputParams.DecodeId &&
        (!InputParams.nVppCompSrcH || !InputParams.nVppCompSrcW)) {
        PrintError(MSDK_STRING(
            "VppCompSrcH and VppCompSrcW must be specified in case of -i::rgb4_frame\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((!InputParams.FRCAlgorithm && !InputParams.bEnableDeinterlacing) &&
        InputParams.dVPPOutFramerate) {
        msdk_printf(MSDK_STRING(
            "-f option is ignored, it can be used with FRC or deinterlace options only. \n"));
        InputParams.dVPPOutFramerate = 0;
    }

    if (InputParams.FRCAlgorithm && InputParams.bEnableExtLA) {
        PrintError(MSDK_STRING("-la_ext and FRC options cannot be used together\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (InputParams.nQuality && InputParams.EncodeId && (MFX_CODEC_JPEG != InputParams.EncodeId)) {
        PrintError(MSDK_STRING("-q option is supported only for JPEG encoder\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((InputParams.nTargetUsage || InputParams.nBitRate) &&
        (MFX_CODEC_JPEG == InputParams.EncodeId)) {
        PrintError(MSDK_STRING(
            "-b and -u options are supported only for H.264, MPEG2 and MVC encoders. For JPEG encoder use -q\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    // valid target usage range is: [MFX_TARGETUSAGE_BEST_QUALITY .. MFX_TARGETUSAGE_BEST_SPEED] (at the moment [1..7])
    // If target usage is kept unknown - presets manager will fill in proper value
    if ((InputParams.nTargetUsage < MFX_TARGETUSAGE_UNKNOWN) ||
        (InputParams.nTargetUsage > MFX_TARGETUSAGE_BEST_SPEED)) {
        PrintError(NULL, "Unsupported target usage");
        return MFX_ERR_UNSUPPORTED;
    }

    // Ignoring user-defined Async Depth for LA
    if (InputParams.nMaxSliceSize) {
        InputParams.nAsyncDepth = 1;
    }

    // For decoder session of inter-session case, let's set AsyncDepth to 4 by default
    if (InputParams.eMode == Sink && !InputParams.nAsyncDepth) {
        InputParams.nAsyncDepth = 4;
    }

    if (InputParams.bLABRC && !(InputParams.libType & MFX_IMPL_HARDWARE_ANY)) {
        PrintError(MSDK_STRING("Look ahead BRC is supported only with -hw option!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (InputParams.bLABRC && (InputParams.EncodeId != MFX_CODEC_AVC) &&
        (InputParams.eMode == Source)) {
        PrintError(MSDK_STRING("Look ahead BRC is supported only with H.264 encoder!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((InputParams.nRateControlMethod == MFX_RATECONTROL_LA ||
         InputParams.nRateControlMethod == MFX_RATECONTROL_LA_EXT ||
         InputParams.nRateControlMethod == MFX_RATECONTROL_LA_ICQ ||
         InputParams.nRateControlMethod == MFX_RATECONTROL_LA_HRD) &&
        (InputParams.nLADepth > 100)) {
        PrintError(MSDK_STRING(
            "Unsupported value of -lad parameter, must be in range [1,100] or 0 for automatic selection"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (!InputParams.nRateControlMethod && InputParams.nLADepth) {
        InputParams.nRateControlMethod = MFX_RATECONTROL_LA;
    }

    if ((InputParams.nMaxSliceSize) && !(InputParams.libType & MFX_IMPL_HARDWARE_ANY)) {
        PrintError(MSDK_STRING("MaxSliceSize option is supported only with -hw option!"));
        return MFX_ERR_UNSUPPORTED;
    }
    if ((InputParams.nMaxSliceSize) && (InputParams.nSlices)) {
        PrintError(MSDK_STRING("-mss and -l options are not compatible!"));
    }
    if ((InputParams.nMaxSliceSize) && (InputParams.EncodeId != MFX_CODEC_AVC)) {
        PrintError(MSDK_STRING("MaxSliceSize option is supported only with H.264 encoder!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (InputParams.BitrateLimit == MFX_CODINGOPTION_UNKNOWN) {
        InputParams.BitrateLimit = MFX_CODINGOPTION_OFF;
    }

    if (InputParams.enableQSVFF && InputParams.eMode == Sink) {
        msdk_printf(MSDK_STRING(
            "WARNING: -lowpower(-qsv-ff) option is not valid for decoder-only sessions, this parameter will be ignored.\n"));
    }

    std::map<mfxU32, sPluginParams>::iterator it;

    // Set decoder plugins parameters only if they were not set before
    if (!memcmp(InputParams.decoderPluginParams.pluginGuid.Data,
                MSDK_PLUGINGUID_NULL.Data,
                sizeof(MSDK_PLUGINGUID_NULL)) &&
        InputParams.decoderPluginParams.strPluginPath.empty()) {
        it = m_decoderPlugins.find(InputParams.DecodeId);
        if (it != m_decoderPlugins.end())
            InputParams.decoderPluginParams = it->second;
    }
    else {
        // Decoding plugin was set manually, so let's check if codec supports plugins
        if (!IsPluginCodecSupported(InputParams.DecodeId)) {
            msdk_printf(MSDK_STRING("error: decoder does not support plugins\n"));
            return MFX_ERR_UNSUPPORTED;
        }
    }

    // Set encoder plugins parameters only if they were not set before
    if (!memcmp(InputParams.encoderPluginParams.pluginGuid.Data,
                MSDK_PLUGINGUID_NULL.Data,
                sizeof(MSDK_PLUGINGUID_NULL)) &&
        InputParams.encoderPluginParams.strPluginPath.empty()) {
        it = m_encoderPlugins.find(InputParams.EncodeId);
        if (it != m_encoderPlugins.end())
            InputParams.encoderPluginParams = it->second;
    }
    else {
        // Encoding plugin was set manually, so let's check if codec supports plugins
        if (!IsPluginCodecSupported(InputParams.EncodeId)) {
            msdk_printf(MSDK_STRING("error: encoder does not support plugins\n"));
            return MFX_ERR_UNSUPPORTED;
        }
    }

    if (InputParams.EncoderFourCC && InputParams.eMode == Sink) {
        msdk_printf(MSDK_STRING(
            "WARNING: -ec option is used in session without encoder, this parameter will be ignored \n"));
    }

    if (InputParams.DecoderFourCC && InputParams.eMode != Native && InputParams.eMode != Sink) {
        msdk_printf(MSDK_STRING(
            "WARNING: -dc option is used in session without decoder, this parameter will be ignored \n"));
    }

    if (InputParams.EncoderFourCC && InputParams.EncoderFourCC != MFX_FOURCC_NV12 &&
        InputParams.EncoderFourCC != MFX_FOURCC_RGB4 &&
        InputParams.EncoderFourCC != MFX_FOURCC_YUY2 && InputParams.EncodeId == MFX_CODEC_DUMP) {
        PrintError(MSDK_STRING(
            "-o::raw option can be used with NV12, RGB4 and YUY2 color formats only.\n"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((InputParams.nEncTileRows || InputParams.nEncTileCols) &&
        (InputParams.EncodeId != MFX_CODEC_HEVC) && (InputParams.EncodeId != MFX_CODEC_VP9)) {
        msdk_printf(MSDK_STRING("WARNING: -trows and -tcols are only supported for") MSDK_STRING(
            " VP9 and") MSDK_STRING(" HEVC encoder, these parameters will be ignored.\n"));
        InputParams.nEncTileRows = 0;
        InputParams.nEncTileCols = 0;
    }

    if (InputParams.forceSyncAllSession && InputParams.nMemoryModel == GENERAL_ALLOC) {
        msdk_printf(MSDK_STRING(
            "WARNING: forceSyncAllSession option is not valid for general memory type, this parameter will be ignored.\n"));
    }

#if (defined(_WIN32) || defined(_WIN64))
    if (InputParams.bPreferiGfx && InputParams.bPreferdGfx) {
        msdk_printf(MSDK_STRING("WARNING: both dGfx and iGfx flags set. iGfx will be preferred\n"));
        InputParams.bPreferdGfx = false;
    }
#endif

    mfxU16 mfxU16Limit = std::numeric_limits<mfxU16>::max();
    if (InputParams.MaxKbps > mfxU16Limit || InputParams.nBitRate > mfxU16Limit ||
        InputParams.InitialDelayInKB > mfxU16Limit || InputParams.BufferSizeInKB > mfxU16Limit) {
        mfxU32 maxVal = std::max<mfxU32>({ InputParams.MaxKbps,
                                           InputParams.nBitRate,
                                           InputParams.InitialDelayInKB,
                                           InputParams.BufferSizeInKB });
        InputParams.nBitRateMultiplier =
            (mfxU16)std::ceil(static_cast<double>(maxVal) / mfxU16Limit);
        msdk_printf(MSDK_STRING("WARNING: BitRateMultiplier(-bm) was updated, new value: %d. \n"),
                    (int)InputParams.nBitRateMultiplier);

        auto recalculate = [mfxU16Limit, InputParams](mfxU32& param, std::string paramName) {
            if (param) {
                if (param > mfxU16Limit) {
                    msdk_printf(MSDK_STRING("WARNING: %s (%d) > allow limit (%d). \n"),
                                paramName.c_str(),
                                (int)param,
                                (int)mfxU16Limit);
                }
                param = param / InputParams.nBitRateMultiplier;
                msdk_printf(MSDK_STRING("WARNING: %s was updated, new value: %d. \n"),
                            paramName.c_str(),
                            (int)param);
            }
        };

        recalculate(InputParams.MaxKbps, "MaxKbps");
        recalculate(InputParams.nBitRate, "nBitRate(-b)");
        recalculate(InputParams.InitialDelayInKB, "InitialDelayInKB");
        recalculate(InputParams.BufferSizeInKB, "BufferSizeInKB");
    }

    return MFX_ERR_NONE;
} //mfxStatus CmdProcessor::VerifyAndCorrectInputParams(TranscodingSample::sInputParams &InputParams)

bool CmdProcessor::GetNextSessionParams(TranscodingSample::sInputParams& InputParams) {
    if (!m_SessionArray.size())
        return false;
    if (m_SessionParamId == m_SessionArray.size()) {
        return false;
    }
    InputParams = m_SessionArray[m_SessionParamId];

    m_SessionParamId++;
    return true;

} //bool  CmdProcessor::GetNextSessionParams(TranscodingSample::sInputParams &InputParams)
