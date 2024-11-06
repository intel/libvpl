#! /usr/bin/python3

##################################################################################
# Copyright (c) 2020 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
##################################################################################

##################################################################################
# By default script attempts to run all possible scenarios for the input clip.
# For example, if HEVC input it will run HEVC-AVC, HEVC-HEVC, etc. If encoder
# does not support input video (typically - resolution), we need to skip scenario
# and avoid error to let other scenarios execute.
##################################################################################
# pylint: disable=line-too-long,superfluous-parens,anomalous-backslash-in-string
# pylint: disable=invalid-name,missing-module-docstring,global-at-module-level
# pylint: disable=missing-class-docstring,too-many-instance-attributes,too-many-lines
# pylint: disable=missing-function-docstring,method-hidden,function-redefined
# pylint: disable=too-many-locals,consider-using-with,raise-missing-from
# pylint: disable=consider-using-sys-exit,import-outside-toplevel,used-before-assignment
# pylint: disable=consider-using-in,multiple-statements,global-variable-undefined
# pylint: disable=simplifiable-if-expression,too-many-nested-blocks,too-many-statements
# pylint: disable=condition-evals-to-constant,unused-variable,too-many-branches
# pylint: disable=redefined-builtin,too-many-arguments,no-else-continue,unneeded-not
# pylint: disable=unidiomatic-typecheck,unused-argument,redefined-outer-name,eval-used
# pylint: disable=unused-import
# pylint: disable=unspecified-encoding,inconsistent-return-statements,redundant-u-string-prefix,consider-using-max-builtin,too-many-positional-arguments,use-implicit-booleaness-not-comparison,consider-using-dict-items,consider-using-f-string
import subprocess  # nosec
import sys
import os
import re
import argparse
import time
import statistics
import signal
import getpass
global temp_path


#################################################
class MediaContent:
    width = encode_bitrate = fps_target = performance_stream = performance_fps = init_stream_number = linux_perf_cmdlines = 0

    def name(self, name):
        self.name = name

    def encode_bitrate(self, encode_bitrate):
        self.encode_bitrate = encode_bitrate

    def height(self, height):
        self.height = height

    def fps_target(self, fps_target):
        self.fps_target = fps_target

    def width(self, width):
        self.width = width

    def codec(self, codec):
        self.codec = codec

    def performance_stream(self, performance_stream):
        self.performance_stream = performance_stream

    def performance_fps(self, performance_fps):
        self.performance_fps = performance_fps

    def init_stream_number(self, init_stream_number):
        self.init_stream_number = init_stream_number

    def dispatch_cmdline(self, dispatch_cmdline):
        self.dispatch_cmdline = dispatch_cmdline

    def temp_path(self, temp_path):
        self.temp_path = temp_path

    def ffmpeg_mode(self, ffmpeg_mode):
        self.ffmpeg_mode = ffmpeg_mode

# GemObject Tools

    def filename_gpumem_trace(self, filename_gpumem_trace):
        self.filename_gpumem_trace = filename_gpumem_trace

    def gemobject_gpumem_trace_dump(self, gemobject_gpumem_trace_dump):
        self.gemobject_gpumem_trace_dump = gemobject_gpumem_trace_dump

# Top Tools

    def filename_cpumem_trace(self, filename_cpumem_trace):
        self.filename_cpumem_trace = filename_cpumem_trace

    def top_cpumem_trace_dump(self, top_cpumem_trace_dump):
        self.top_cpumem_trace_dump = top_cpumem_trace_dump

# Linux Perf Tools

    def linux_perf_dump(self, linux_perf_dump):
        self.linux_perf_dump = linux_perf_dump

    def linux_perf_gpu_freq_trace_dump(self, linux_perf_gpu_freq_trace_dump):
        self.linux_perf_gpu_freq_trace_dump = linux_perf_gpu_freq_trace_dump

    def filename_gpu_freq_trace(self, filename_gpu_freq_trace):
        self.filename_gpu_freq_trace = filename_gpu_freq_trace

    def linux_perf_mem_bw_trace_dump(self, linux_perf_mem_bw_trace_dump):
        self.linux_perf_mem_bw_trace_dump = linux_perf_mem_bw_trace_dump

    def filename_mem_bw_trace(self, filename_mem_bw_trace):
        self.filename_mem_bw_trace = filename_mem_bw_trace

    def tool_linux_perf_trace(self, tool_linux_perf_trace):
        self.tool_linux_perf_trace = tool_linux_perf_trace

    def linux_perf_cmdlines(self, linux_perf_cmdlines):
        self.linux_perf_cmdlines = linux_perf_cmdlines


#################################################
def main():
    startTime = time.time()
    # Collect command line arguments
    parser = argparse.ArgumentParser(
        description='MSPERF (MULTI STREAM PERFORMANCE) v0.20.06.03',
        conflict_handler='resolve')
    parser.add_argument(
        'workloads_path',
        help=
        'Full path option for workload_directories/ OR for single workload_file'
    )
    parser.add_argument('--skip-ffmpeg',
                        '--skip_ffmpeg',
                        action='store_true',
                        default=False,
                        help='skipping measure that use FFMPEG app')
    parser.add_argument('--skip-msdk',
                        '--skip_msdk',
                        action='store_true',
                        default=False,
                        help='skipping measure that use MSDK/Sample app')
    parser.add_argument(
        '--skip-perf',
        '--skip_perf',
        action='store_true',
        default=False,
        help='skipping linux perf stat Utilization, such as VD0/VD1/RCS/etc')
    parser.add_argument(
        '--skip-perf-trace',
        '--skip_perf_trace',
        action='store_true',
        default=False,
        help=
        'skipping linux perf stat additional Traces, such as GT-Freq/BW-Rd/BW-Wr/etc'
    )
    parser.add_argument(
        '--enable-debugfs',
        '--enable_debugfs',
        action='store_true',
        default=False,
        help='enabling further analysis tools such as  CPU_mem, GPU_mem, etc')
    parser.add_argument('--density-decode',
                        '--density_decode',
                        action='store_true',
                        default=False,
                        help='Enabling Density Decode support, HEVC and AVC')
    parser.add_argument('--use-vdenc',
                        '--use_vdenc',
                        action='store_true',
                        default=False,
                        help='Enabling Fixed Function VDENC and LowPower mode')
    parser.add_argument('--use-enctools',
                        '--use_enctools',
                        action='store_true',
                        default=False,
                        help='Enabling Enc tools default config file ')
    parser.add_argument('--enctools-lad',
                        '--enctools_lad',
                        help='Custom Look Ahead Depth , Default=8')
    parser.add_argument('--tu',
                        '--tu',
                        help='Custom Target Usage preset , Default=medium')
    parser.add_argument(
        '--async-depth',
        '--async_depth',
        help='Custom Async depth , Default depends on resoultion')
    parser.add_argument(
        '-c',
        '--codec',
        help=
        'To choose Encoder Codec type, AVC or HEVC, Default will execute all')
    parser.add_argument(
        '-s',
        '--startStreams',
        help=
        'To set starting of multi stream performance measurement, e.g. --startStreams 720p:8,1080p:5,2160p:2 or all:2, Default=all:1'
    )
    parser.add_argument(
        '-e',
        '--endStreams',
        help=
        'To set ending number of multi stream performance measurement, e.g. --endStreams 5, Default=NoLimit'
    )
    parser.add_argument(
        '-n',
        '--numbers_of_iteration',
        help=
        'Custom limit the number of iteration of each same execution (max is 4), Default=1'
    )
    parser.add_argument(
        '--no-fps-limit',
        '--no_fps_limit',
        action='store_true',
        default=False,
        help='to run workload unconstraint, or as fast as possible')
    parser.add_argument('--fps-target',
                        '--fps_target',
                        help='to overwrite fps limit, Default=input-fps')
    parser.add_argument(
        '-w_max',
        '--numbers_of_Workloads',
        help='Custom limit the number of total workloads to be executed')
    parser.add_argument('-o',
                        '--outdir',
                        help='output directory for any artifacts')
    parser.add_argument(
        '-log',
        '--output_log_file',
        help=
        'print any run-log into this file onto main directory, Default=msperf.txt'
    )
    parser.add_argument(
        '-v',
        '--verbose',
        action='store_true',
        default=False,
        help=
        'Dump debug related printout, such as each-cmdlines/version-log/etc')
    global ARGS
    ARGS = parser.parse_args()

    ################################# Set artifacts path ##################################################
    artifacts_path_default = os.getenv("HOME")
    artifacts = os.getenv("ARTIFACTS")

    if str(ARGS.outdir) != "None":
        artifacts_path_users = os.path.realpath(ARGS.outdir)
        artifact_path = artifacts_path_users + "/"
    elif str(artifacts) != "None":
        artifact_path = artifacts + "/measure/perf/"
    else:
        artifact_path = artifacts_path_default + "/measure/perf/"

    ################################# Multiple Device default/user overwrite check #########################
    os_env_DEVICE = os.environ.get('DEVICE', "/dev/dri/renderD128")
    device_name = os_env_DEVICE.split('/')[3]

    ################################# Variable Assignment #################################################
    starting_streamnumber = str(
        ARGS.startStreams) if ARGS.startStreams else "all:1"
    maximum_iteration = int(
        ARGS.numbers_of_iteration) if ARGS.numbers_of_iteration else 1
    endStreams = int(ARGS.endStreams) if ARGS.endStreams else 99
    maximum_workloads = int(
        ARGS.numbers_of_Workloads) if ARGS.numbers_of_Workloads else 20
    debug_verbose = ARGS.verbose
    no_fps_limit = ARGS.no_fps_limit
    tool_linux_perf = not ARGS.skip_perf
    tool_linux_perf_trace = not ARGS.skip_perf_trace
    enable_debugfs = ARGS.enable_debugfs
    skip_ffmpeg = ARGS.skip_ffmpeg
    skip_msdk = ARGS.skip_msdk
    density_decode = ARGS.density_decode
    use_vdenc = ARGS.use_vdenc
    encode_codec = str(ARGS.codec).lower() if ARGS.codec else "all"
    fps_target = float(ARGS.fps_target) if ARGS.fps_target else 0
    script_root_path = os.path.dirname(os.path.realpath(__file__))
    output_log_filename = str(ARGS.output_log_file) if str(
        ARGS.output_log_file) != "None" else "msperf.txt"
    temp_path = "/tmp/perf_" + device_name + "/"
    ##################################################################################
    # Initiate artifacts directory
    ##################################################################################
    try:
        # checks if path is a directory
        isDirectory = os.path.isdir(artifact_path)
        if not isDirectory:
            os.system("mkdir -p " + artifact_path)

        output_log_file = artifact_path + output_log_filename
        output_log_handle = open(output_log_file, 'w')
    except:
        message_block(output_log_handle, 'red',
                      'Unable to create artifact path ' + artifact_path)
        raise sys.exit(1)
    ######################################################################################################
    if debug_verbose:
        printLog(output_log_handle, '#' * 69)
        printLog(output_log_handle,
                 'MSPERF (MULTI STREAMS PERFORMANCE) v0.20.06.03')
        printLog(output_log_handle, '#' * 69 + '\n')

    if use_vdenc:
        isSupported_vdenc_h264high = subprocess.Popen([
            os.path.dirname(os.path.realpath(__file__)) + '/isVdenc.sh',
            'H264High'
        ])
        isSupported_vdenc_hevcmain = subprocess.Popen([
            os.path.dirname(os.path.realpath(__file__)) + '/isVdenc.sh',
            'HEVCMain'
        ])
        if (isSupported_vdenc_h264high.returncode
                == "None") and (isSupported_vdenc_hevcmain.returncode
                                == "None"):
            printLog(output_log_handle, "VDENC supports has a problem",
                     isSupported_vdenc_h264high.returncode,
                     isSupported_vdenc_hevcmain.returncode)
            exit(1)
        elif isSupported_vdenc_h264high.returncode == 0:
            printLog(output_log_handle, "VDENC supports H264High")
        elif isSupported_vdenc_hevcmain.returncode == 0:
            printLog(output_log_handle, "VDENC supports HEVCMain")

    ######################################################################################################
    # Linux Perf pre-req
    ######################################################################################################
    try:
        if enable_debugfs and (sudo_password_request() != 0):
            print(
                "REJECTED !!! and continue without [utilization/metric/statistic] profiles"
            )
            exit(1)
        else:
            print(
                "ACCEPTED !!! to continue with [utilization/metric/statistic] profiles"
            )

        if tool_linux_perf and tool_linux_perf_trace:
            import matplotlib
            matplotlib.use('Agg')
    except:
        message_block(output_log_handle, 'red',
                      'Unable to import matplotlib: ')
        raise sys.exit(1)
    ######################################################################################################
    # output path and header creation
    ######################################################################################################
    try:
        ##################################################################################
        # Initiate temporary directory for calculation and post-processing data.
        ##################################################################################
        directory_check = os.path.isdir(temp_path)
        if not directory_check:
            os.system("mkdir " + temp_path)
        else:
            os.system("rm -rf " + temp_path)
            os.system("mkdir " + temp_path)

        ###############################################
        # assigning initial multistream as per resolution options
        ###############################################
        init_stream_720p = 1
        init_stream_1080p = 1
        init_stream_2160p = 1
        if starting_streamnumber != "all:1":
            starting_streamnumber_split = starting_streamnumber.split(',')
            for resolution_mode in starting_streamnumber_split:
                resolution_mode_split = resolution_mode.split(':')
                if resolution_mode_split[1].isdigit():
                    if "720p:" in resolution_mode:
                        init_stream_720p = int(resolution_mode_split[1])
                    elif "1080p:" in resolution_mode:
                        init_stream_1080p = int(resolution_mode_split[1])
                    elif "2160p:" in resolution_mode:
                        init_stream_2160p = int(resolution_mode_split[1])
                    else:
                        printLog(
                            output_log_handle,
                            "ERROR: Syntax incorrect, Can not find resolution, please follow an example: -s 720p:8,1080p:5,2160p:2, and Try again"
                        )
                        return ()
                else:
                    printLog(
                        output_log_handle,
                        "ERROR: Syntax incorrect, Value is not integer,",
                        resolution_mode_split[1],
                        "please follow an example: -s 720p:8,1080p:5,2160p:2, and Try again"
                    )
                    return ()
    except:
        message_block(
            output_log_handle, 'red',
            'Unable to create directory or inaccessible: ' + local_output_path)
        raise sys.exit(1)

    ######################################################################################################
    # Setup for Workloads directory or a  single Workload performance
    ######################################################################################################
    try:
        ARGS.workloads_path = os.path.realpath(ARGS.workloads_path)
        # checks if path is a directory
        isDirectory = os.path.isdir(ARGS.workloads_path)

        # checks if path is a file
        isFile = os.path.isfile(ARGS.workloads_path)

        performance_sweeping_table = {}
        performance_object_list = {}
        printLog(output_log_handle, "\n")
        printLog(output_log_handle, '#' * 69)

        if (isDirectory):
            content_path = str(ARGS.workloads_path)
            if (content_path[-1] != "/"):
                content_path = content_path + "/"
            #################################################################################
            # Contentlist naming convention array <content_name>_<resolution>_<bitrate>_<fps>_<totalFrames>.<codec>
            # jiwan
            ##################################################################################
            content_list_filename = temp_path + "content.list"
            cmd_generate_content_list = "ls " + content_path + " | grep -E '\.h264|\.hevc|\.h265|\.mp4|\.ivf|\.av1' > " + content_list_filename  # Add more content container support into the grep list.
            generate_list_status = os.system(cmd_generate_content_list)

            with open(content_list_filename, "r") as content_list_temp_fh:
                for content_filename in content_list_temp_fh:
                    content_filename = content_filename.rstrip()

                    printLog(output_log_handle,
                             " Profiling: " + content_filename)

                    if ffmpegffprobeCheck(output_log_handle, content_path,
                                          content_filename, temp_path,
                                          debug_verbose,
                                          performance_sweeping_table,
                                          performance_object_list):
                        if debug_verbose:
                            printLog(output_log_handle,
                                     " PASS: via FFMPEG/FFPROBE")
                            printLog(
                                output_log_handle, " content_fps =",
                                performance_object_list[content_filename].
                                fps_target, ", content_height =",
                                performance_object_list[content_filename].
                                height, ", content_codec =",
                                performance_object_list[content_filename].
                                codec, "\n")

                    if (float(fps_target) > 0):
                        performance_object_list[
                            content_filename].fps_target = float(fps_target)

            content_list_temp_fh.close()

        elif (isFile):
            content_path, content_filename = os.path.split(ARGS.workloads_path)
            content_path = content_path + "/"  # required because the extraction dir/filename above doesn't include the "/" character.
            printLog(output_log_handle, " Profiling: " + content_filename)
            content_filename = content_filename.rstrip()
            if ffmpegffprobeCheck(output_log_handle, content_path,
                                  content_filename, temp_path, debug_verbose,
                                  performance_sweeping_table,
                                  performance_object_list):
                if debug_verbose:
                    printLog(output_log_handle, " PASS: via FFMPEG/FFPROBE")
                    printLog(
                        output_log_handle, " content_fps =",
                        performance_object_list[content_filename].fps_target,
                        ", content_height =",
                        performance_object_list[content_filename].height,
                        ", content_codec =",
                        performance_object_list[content_filename].codec, "\n")

            if (float(fps_target) > 0):
                performance_object_list[content_filename].fps_target = float(
                    fps_target)

        else:
            message_block(
                output_log_handle, 'red',
                'Unable to locate required workload path: ' +
                ARGS.workloads_path)
            raise sys.exit(1)

    except:
        message_block(
            output_log_handle, 'red',
            'Unable to profile required workload path: ' + ARGS.workloads_path)
        raise sys.exit(1)

    ##################################################################################
    # Iterating measure applications
    # 1st SMT
    # 2nd FFMPEG
    # 3rd TBD/continue..
    ##################################################################################
    performance_starttime = time.time()
    performance_datetime = str(
        time.ctime())  # to be used for archiving purposes.
    for performance_applications in range(2):
        required_information_file = os.path.dirname(os.path.realpath(__file__))
        if (performance_applications == 0) and not skip_msdk:
            ffmpeg_mode = False
            performance_app_tag = "SMT"
            if not ARGS.use_enctools:
                required_information_file += "/por_SMT_LB.txt"
            else:
                required_information_file += "/por_SMT_LB_enctools.txt"

        elif (performance_applications == 1) and not skip_ffmpeg:
            if not ARGS.use_enctools:
                required_information_file += "/por_FFMPEG_LB.txt"
            else:
                required_information_file += "/por_FFMPEG_LB_enctools.txt"
            ffmpeg_mode = True
            performance_app_tag = "FFMPEG"
        else:
            continue
        ######################################################################################################
        # command line based on resolution
        ######################################################################################################
        try:
            cmdline_config_hevc2avc_exist = cmdline_config_avc2avc_exist = cmdline_config_hevc2hevc_exist = cmdline_config_avc2hevc_exist = cmdline_config_av12av1_exist = cmdline_config_decode_hevc_exist = cmdline_config_decode_avc_exist = False
            with open(required_information_file, 'r') as configfile:
                for workloadline in configfile:
                    if (not re.search("^#", str(workloadline))):
                        if (re.search(r"hevc-avc:", str(workloadline))):
                            cmdline_config_hevc2avc_exist = True
                        if (re.search(r"avc-avc:", str(workloadline))):
                            cmdline_config_avc2avc_exist = True
                        if (re.search(r"hevc-hevc:", str(workloadline))):
                            cmdline_config_hevc2hevc_exist = True
                        if (re.search(r"avc-hevc:", str(workloadline))):
                            cmdline_config_avc2hevc_exist = True
                        if (re.search(r"av1-av1:", str(workloadline))):
                            cmdline_config_av12av1_exist = True
                        if (re.search(r"decode-hevc:", str(workloadline))):
                            cmdline_config_decode_hevc_exist = True
                        if (re.search(r"decode-avc:", str(workloadline))):
                            cmdline_config_decode_avc_exist = True
                        if (re.search(r"^720p_hevc-avc:\s",
                                      str(workloadline))):
                            performance_cmdline_720p_hevc2avc = workloadline.replace(
                                "720p_hevc-avc: ", "")
                        if (re.search(r"^1080p_hevc-avc:\s",
                                      str(workloadline))):
                            performance_cmdline_1080p_hevc2avc = workloadline.replace(
                                "1080p_hevc-avc: ", "")
                        if (re.search(r"^2160p_hevc-avc:\s",
                                      str(workloadline))):
                            performance_cmdline_2160p_hevc2avc = workloadline.replace(
                                "2160p_hevc-avc: ", "")
                        if (re.search(r"^720p_avc-avc:\s", str(workloadline))):
                            performance_cmdline_720p_avc2avc = workloadline.replace(
                                "720p_avc-avc: ", "")
                        if (re.search(r"^1080p_avc-avc:\s",
                                      str(workloadline))):
                            performance_cmdline_1080p_avc2avc = workloadline.replace(
                                "1080p_avc-avc: ", "")
                        if (re.search(r"^2160p_avc-avc:\s",
                                      str(workloadline))):
                            performance_cmdline_2160p_avc2avc = workloadline.replace(
                                "2160p_avc-avc: ", "")
                        if (re.search(r"^720p_hevc-hevc:\s",
                                      str(workloadline))):
                            performance_cmdline_720p_hevc2hevc = workloadline.replace(
                                "720p_hevc-hevc: ", "")
                        if (re.search(r"^1080p_hevc-hevc:\s",
                                      str(workloadline))):
                            performance_cmdline_1080p_hevc2hevc = workloadline.replace(
                                "1080p_hevc-hevc: ", "")
                        if (re.search(r"^2160p_hevc-hevc:\s",
                                      str(workloadline))):
                            performance_cmdline_2160p_hevc2hevc = workloadline.replace(
                                "2160p_hevc-hevc: ", "")
                        if (re.search(r"^720p_avc-hevc:\s",
                                      str(workloadline))):
                            performance_cmdline_720p_avc2hevc = workloadline.replace(
                                "720p_avc-hevc: ", "")
                        if (re.search(r"^1080p_avc-hevc:\s",
                                      str(workloadline))):
                            performance_cmdline_1080p_avc2hevc = workloadline.replace(
                                "1080p_avc-hevc: ", "")
                        if (re.search(r"^2160p_avc-hevc:\s",
                                      str(workloadline))):
                            performance_cmdline_2160p_avc2hevc = workloadline.replace(
                                "2160p_avc-hevc: ", "")
                        if (re.search(r"^720p_av1-av1:\s", str(workloadline))):
                            performance_cmdline_720p_av12av1 = workloadline.replace(
                                "720p_av1-av1: ", "")
                        if (re.search(r"^1080p_av1-av1:\s",
                                      str(workloadline))):
                            performance_cmdline_1080p_av12av1 = workloadline.replace(
                                "1080p_av1-av1: ", "")
                        if (re.search(r"^2160p_av1-av1:\s",
                                      str(workloadline))):
                            performance_cmdline_2160p_av12av1 = workloadline.replace(
                                "2160p_av1-av1: ", "")
                        if (re.search(r"decode-hevc:\s", str(workloadline))):
                            performance_cmdline_decode_hevc = workloadline.replace(
                                "decode-hevc: ", "")
                        if (re.search(r"decode-avc:\s", str(workloadline))):
                            performance_cmdline_decode_avc = workloadline.replace(
                                "decode-avc: ", "")
        except:
            message_block(
                output_log_handle, 'red', 'Unable to locate required file: ' +
                ARGS.required_information_file)
            raise sys.exit(1)

        ##################################################################################
        # Iterating measure sequence
        ##################################################################################
        Workloads = [
            "HEVC-AVC", "AVC-AVC", "HEVC-HEVC", "AVC-HEVC", "AV1-AV1",
            "DECODE-HEVC", "DECODE-AVC"
        ]

        ##################################################################################
        startTime_sequence = time.time()
        for performance_tag in Workloads:
            ##################################################################################
            # Initiate outputfile measure result as per last best stream# and fps#
            # Initiate outputfile measure table sweep as per last best stream# and fps#
            ##################################################################################

            if performance_tag == "HEVC-AVC" and (
                    encode_codec == "all" or encode_codec
                    == "avc") and cmdline_config_hevc2avc_exist:
                sequence_mode = "TRANSCODE"
            elif performance_tag == "AVC-AVC" and (
                    encode_codec == "all"
                    or encode_codec == "avc") and cmdline_config_avc2avc_exist:
                sequence_mode = "TRANSCODE"
            elif performance_tag == "HEVC-HEVC" and (
                    encode_codec == "all" or encode_codec
                    == "hevc") and cmdline_config_hevc2hevc_exist:
                sequence_mode = "TRANSCODE"
            elif performance_tag == "AVC-HEVC" and (
                    encode_codec == "all" or encode_codec
                    == "hevc") and cmdline_config_avc2hevc_exist:
                sequence_mode = "TRANSCODE"
            elif performance_tag == "AV1-AV1" and (
                    encode_codec == "all"
                    or encode_codec == "av1") and cmdline_config_av12av1_exist:
                sequence_mode = "TRANSCODE"
            elif performance_tag == "DECODE-HEVC" and density_decode and cmdline_config_decode_hevc_exist:
                sequence_mode = "DECODE"
            elif performance_tag == "DECODE-AVC" and density_decode and cmdline_config_decode_avc_exist:
                sequence_mode = "DECODE"
            else:
                continue

            output_log_file_performance_media = re.sub(
                r'.txt', "_" + performance_app_tag + "_" + performance_tag +
                "_performance.csv", output_log_file)
            output_log_file_performance_sweep = re.sub(
                r'.txt', "_" + performance_app_tag + "_" + performance_tag +
                "_performance_table_sweep.csv", output_log_file)

            ##################################################################################
            # Archiving potential duplicate runs.
            ##################################################################################
            isFile = os.path.isfile(output_log_file_performance_media)
            if (isFile):
                archived_filepath = "archived_" + performance_datetime
                archived_filepath = re.sub(r'\:', "", archived_filepath)
                archived_filepath = re.sub(r'\s', "_", archived_filepath)
                isDirectory = os.path.isdir(artifact_path + archived_filepath)
                if not isDirectory:
                    archived_command = "mkdir " + artifact_path + archived_filepath
                    os.system(archived_command)
                move_command = "mv " + artifact_path + "*" + performance_app_tag + "*" + performance_tag + "* " + artifact_path + archived_filepath + "/."
                os.system(move_command)

            ##################################################################################
            # Initiate temporary directory for calculation and post-processing data. ReCreated for every measure sequence.
            ##################################################################################
            directory_check = os.path.isdir(temp_path)
            filepath_free_info = temp_path + performance_app_tag + "_" + performance_tag + "_" + "free.txt"
            filepath_lscpu_info = temp_path + performance_app_tag + "_" + performance_tag + "_" + "lscpu.txt"

            if not directory_check:
                os.system("mkdir " + temp_path)
                os.system("free > " + filepath_free_info)
                os.system("lscpu > " + filepath_lscpu_info)
            else:
                os.system("rm -rf " + temp_path)
                os.system("mkdir " + temp_path)
                os.system("free > " + filepath_free_info)
                os.system("lscpu > " + filepath_lscpu_info)

            local_output_path = artifact_path + "output/"  # all the Metrics/Summary/Traces will be copy further into the current directory.
            directory_check = os.path.isdir(local_output_path)
            if not directory_check: os.system("mkdir " + local_output_path)

            for a in range(int(maximum_iteration)):
                iteration_path_cmd = temp_path + "iteration_" + str(a)
                directory_check = os.path.isdir(iteration_path_cmd)
                if not directory_check:
                    os.system("mkdir " + iteration_path_cmd)

            bm_output_handle = open(output_log_file_performance_media, 'w')
            measure = "clipname, performance_stream, fps, runtime(s), GPU_Vid0(%), GPU_Vid1(%), GPU_Render(%), AVG_CPU(%), RC6(%), GPU_Freq_Avg(MHz), MEM_RES_Avg(MB/stream), MEM_RES_Total(MB), AVG_MEM(%), PHYSICAL_MEM(MB), CPU_IPC, TOTAL_CPU(%), GPU_MEM_RES_Avg(MB/stream), GPU_MEM_RES_Total(MB)\n"
            bm_output_handle.write(measure)
            performance_table_sweep = output_log_file_performance_sweep
            bt_output_handle = open(performance_table_sweep, 'w')

            ##################################################################################
            # ITERATION/WORKLOADS/etc START HERE
            ##################################################################################
            for key in performance_object_list:
                curContent = key.rstrip()

                if curContent == "":
                    printLog(output_log_handle,
                             "SCRIPT ERROR: empty content")  # exit.
                    exit(1)
                elif (re.search(r'.*.mp4$',
                                curContent)) and (performance_app_tag
                                                  == "SMT"):
                    printLog(output_log_handle, "\n")
                    printLog(
                        output_log_handle,
                        "[SKIPPED]: files in container formats (.mp4, .mkv, etc.) are not supported by MSDK samples ",
                        curContent)  # skip
                    continue

                performance_sweeping_table[curContent] = []

                if (performance_tag == "HEVC-AVC") and (
                        encode_codec == "all" or encode_codec == "avc") and (
                            performance_object_list[curContent].height
                            <= 2160):  # HEVC-AVC measure sequence
                    if performance_object_list[
                            curContent].codec != "hevc":  # skip if its not HEVC input clip, or if output Encode AVC tried height higher from 2160
                        continue
                elif (performance_tag == "AVC-AVC") and (
                        encode_codec == "all" or encode_codec == "avc") and (
                            performance_object_list[curContent].height
                            <= 2160):  # AVC-AVC measure sequence
                    if performance_object_list[
                            curContent].codec != "h264":  # skip if its not h264 input clip, or if output Encode AVC tried height higher from 2160
                        continue
                elif (performance_tag == "HEVC-HEVC") and (
                        encode_codec == "all" or encode_codec
                        == "hevc"):  # HEVC-HEVC measure sequence
                    if performance_object_list[
                            curContent].codec != "hevc":  # skip if its not HEVC input clip
                        continue
                elif (performance_tag == "AVC-HEVC") and (
                        encode_codec == "all" or encode_codec
                        == "hevc"):  # AVC-HEVC measure sequence
                    if performance_object_list[
                            curContent].codec != "h264":  # skip if its not h264 input clip
                        continue
                elif (performance_tag
                      == "AV1-AV1") and (encode_codec == "all" or encode_codec
                                         == "av1"):  # AV1-AV1 measure sequence
                    if performance_object_list[
                            curContent].codec != "av1":  # skip if its not av1 input clip
                        continue
                elif (performance_tag == "DECODE-HEVC"
                      ):  # Decode-HEVC measure sequence
                    if performance_object_list[
                            curContent].codec != "hevc":  # skip if its not HEVC input clip
                        continue
                elif (performance_tag == "DECODE-AVC"
                      ):  # Decode-AVC measure sequence
                    if performance_object_list[
                            curContent].codec != "h264":  # skip if its not H264 input clip
                        continue
                else:
                    printLog(output_log_handle, "\n")
                    printLog(output_log_handle, '#' * 69)
                    printLog(output_log_handle, "[SKIPPED]: ",
                             performance_app_tag, " ", performance_tag, " ",
                             curContent)  # skip
                    printLog(output_log_handle, '#' * 69)
                    continue

                ##################################################################################
                # Create initialize streamnumber for each resolution
                ##################################################################################
                if performance_object_list[
                        curContent].encode_bitrate == "sd_bitrate":
                    streamnumber = init_stream_720p
                    performance_object_list[
                        curContent].init_stream_number = init_stream_720p
                elif performance_object_list[
                        curContent].encode_bitrate == "hd_bitrate":
                    streamnumber = init_stream_1080p
                    performance_object_list[
                        curContent].init_stream_number = init_stream_1080p
                elif performance_object_list[
                        curContent].encode_bitrate == "4k_bitrate":
                    streamnumber = init_stream_2160p
                    performance_object_list[
                        curContent].init_stream_number = init_stream_2160p
                else:
                    streamnumber = 1
                    performance_object_list[curContent].init_stream_number = 1

                nextStreamNumber = True if int(streamnumber) > 0 else False
                content_fps_limit = 0
                performance_stream = 0
                performance_fps = 0
                performance_exetime = 0
                performance_vid0_u = 0
                performance_vid1_u = 0
                performance_render_u = 0
                performance_cpu_ipc = 0
                performance_rc6_u = 0
                performance_avg_freq = 0
                performance_avg_res_mem = 0
                performance_total_res_mem = 0
                performance_avg_mem_percentage = 0
                performance_avg_cpu_percentage = 0
                performance_max_cpu_percentage = 0
                performance_max_physical_mem = 0
                performance_avg_res_gpumem = 0
                performance_total_res_gpumem = 0

                while nextStreamNumber and (streamnumber <= endStreams):
                    shell_script_mms = temp_path + "mms.sh"  # Move this to /tmp/perf/
                    shell_script_handle = open(shell_script_mms, 'w')
                    avg_fps = exetime = vid0_u = vid1_u = render_u = avg_freq = 0
                    avg_fps_split = []
                    ##################################################################################
                    # Creates Unique Concurrent Command Lines
                    ##################################################################################
                    content_split = key.replace('.', '_').split('_')
                    clip_name = content_split[0]
                    clip_resolution = content_split[1]
                    clip_stream_iter_tag = performance_app_tag + "_" + performance_tag + "_" + clip_name + "_" + clip_resolution + "_" + str(
                        streamnumber)
                    fps_constraint = int(
                        performance_object_list[curContent].fps_target)
                    shell_script_handle.write("echo " + clip_stream_iter_tag +
                                              "\n")

                    for m in range(int(streamnumber)):
                        ##################################################################################
                        # Construct 720p/1080p/2160p Command lines for
                        ##################################################################################
                        dispatch_cmdline = transcode_input_clip = "N/A"
                        if performance_tag == "HEVC-AVC" and (
                                encode_codec == "all" or encode_codec
                                == "avc"):  # HEVC-AVC measure sequence
                            if performance_object_list[
                                    curContent].encode_bitrate == "sd_bitrate":
                                dispatch_cmdline = performance_cmdline_720p_hevc2avc
                            elif performance_object_list[
                                    curContent].encode_bitrate == "hd_bitrate":
                                dispatch_cmdline = performance_cmdline_1080p_hevc2avc
                            elif performance_object_list[
                                    curContent].encode_bitrate == "4k_bitrate":
                                dispatch_cmdline = performance_cmdline_2160p_hevc2avc
                        elif performance_tag == "AVC-AVC" and (
                                encode_codec == "all" or encode_codec
                                == "avc"):  # AVC-AVC measure sequence
                            if performance_object_list[
                                    curContent].encode_bitrate == "sd_bitrate":
                                dispatch_cmdline = performance_cmdline_720p_avc2avc
                            elif performance_object_list[
                                    curContent].encode_bitrate == "hd_bitrate":
                                dispatch_cmdline = performance_cmdline_1080p_avc2avc
                            elif performance_object_list[
                                    curContent].encode_bitrate == "4k_bitrate":
                                dispatch_cmdline = performance_cmdline_2160p_avc2avc
                        elif performance_tag == "HEVC-HEVC" and (
                                encode_codec == "all" or encode_codec
                                == "hevc"):  # HEVC-HEVC measure sequence
                            if performance_object_list[
                                    curContent].encode_bitrate == "sd_bitrate":
                                dispatch_cmdline = performance_cmdline_720p_hevc2hevc
                            elif performance_object_list[
                                    curContent].encode_bitrate == "hd_bitrate":
                                dispatch_cmdline = performance_cmdline_1080p_hevc2hevc
                            elif performance_object_list[
                                    curContent].encode_bitrate == "4k_bitrate":
                                dispatch_cmdline = performance_cmdline_2160p_hevc2hevc
                        elif performance_tag == "AVC-HEVC" and (
                                encode_codec == "all" or encode_codec
                                == "hevc"):  # AVC-HEVC measure sequence
                            if performance_object_list[
                                    curContent].encode_bitrate == "sd_bitrate":
                                dispatch_cmdline = performance_cmdline_720p_avc2hevc
                            elif performance_object_list[
                                    curContent].encode_bitrate == "hd_bitrate":
                                dispatch_cmdline = performance_cmdline_1080p_avc2hevc
                            elif performance_object_list[
                                    curContent].encode_bitrate == "4k_bitrate":
                                dispatch_cmdline = performance_cmdline_2160p_avc2hevc
                        elif performance_tag == "AV1-AV1" and (
                                encode_codec == "all" or encode_codec
                                == "av1"):  # AV1-AV1 measure sequence
                            if performance_object_list[
                                    curContent].encode_bitrate == "sd_bitrate":
                                dispatch_cmdline = performance_cmdline_720p_av12av1
                            elif performance_object_list[
                                    curContent].encode_bitrate == "hd_bitrate":
                                dispatch_cmdline = performance_cmdline_1080p_av12av1
                            elif performance_object_list[
                                    curContent].encode_bitrate == "4k_bitrate":
                                dispatch_cmdline = performance_cmdline_2160p_av12av1
                        elif performance_tag == "DECODE-HEVC":  # DECODE HEVC
                            dispatch_cmdline = performance_cmdline_decode_hevc
                        elif performance_tag == "DECODE-AVC":  # DECODE AVC
                            dispatch_cmdline = performance_cmdline_decode_avc

                        else:
                            break

                        async_default = '2'
                        if performance_object_list[
                                curContent].encode_bitrate == "hd_bitrate" or performance_object_list[
                                    curContent].encode_bitrate == "sd_bitrate":
                            async_default = '1'
                        if (ffmpeg_mode):
                            transcode_input_clip = "-i " + content_path + key
                            if not no_fps_limit:
                                transcode_input_clip = "-re " + transcode_input_clip

                            dispatch_cmdline = dispatch_cmdline.replace(
                                "-i <>", transcode_input_clip)
                            dispatch_cmdline = dispatch_cmdline.replace(
                                "-preset <>", "-preset " +
                                (ARGS.tu if ARGS.tu else 'veryfast'))
                            dispatch_cmdline = dispatch_cmdline.replace(
                                "-async_depth <>", "-async_depth " +
                                (ARGS.async_depth
                                 if ARGS.async_depth else async_default))
                            if ARGS.use_enctools:
                                dispatch_cmdline = dispatch_cmdline.replace(
                                    "-look_ahead_depth <>",
                                    "-look_ahead_depth " +
                                    (ARGS.enctools_lad
                                     if ARGS.enctools_lad else '8'))
                                dispatch_cmdline = dispatch_cmdline.replace(
                                    "-extra_hw_frames <>",
                                    "-extra_hw_frames " +
                                    (ARGS.enctools_lad
                                     if ARGS.enctools_lad else '8'))

                        elif (sequence_mode == "TRANSCODE"):  # SMT Transcode
                            dict_tu = {
                                'veryslow': '1',
                                'slower': '2',
                                'slow': '3',
                                'medium': '4',
                                'fast': '5',
                                'faster': '6',
                                'veryfast': '7'
                            }

                            dispatch_cmdline = dispatch_cmdline.replace(
                                "-u <>",
                                "-u " + (dict_tu[ARGS.tu] if ARGS.tu else '7'))
                            dispatch_cmdline = dispatch_cmdline.replace(
                                "-async <>", "-async " +
                                (ARGS.async_depth
                                 if ARGS.async_depth else async_default))
                            if ARGS.use_enctools:
                                dispatch_cmdline = dispatch_cmdline.replace(
                                    "-lad <>",
                                    "-lad " + (ARGS.enctools_lad
                                               if ARGS.enctools_lad else '8'))
                            if performance_object_list[
                                    curContent].codec == "hevc":
                                transcode_input_clip = "-i::h265 " + content_path + key
                                if not no_fps_limit:
                                    transcode_input_clip = "-fps " + str(
                                        fps_constraint
                                    ) + " " + transcode_input_clip

                                dispatch_cmdline = dispatch_cmdline.replace(
                                    "-i::h265 <>", transcode_input_clip)
                            elif performance_object_list[
                                    curContent].codec == "h264":
                                transcode_input_clip = "-i::h264 " + content_path + key
                                if not no_fps_limit:
                                    transcode_input_clip = "-fps " + str(
                                        fps_constraint
                                    ) + " " + transcode_input_clip

                                dispatch_cmdline = dispatch_cmdline.replace(
                                    "-i::h264 <>", transcode_input_clip)
                            elif performance_object_list[
                                    curContent].codec == "av1" or "ivf":
                                transcode_input_clip = "-i::av1 " + content_path + key
                                if not no_fps_limit:
                                    transcode_input_clip = "-fps " + str(
                                        fps_constraint
                                    ) + " " + transcode_input_clip

                                dispatch_cmdline = dispatch_cmdline.replace(
                                    "-i::av1 <>", transcode_input_clip)

                        elif (sequence_mode == "DECODE"):  # SMT Decode
                            transcode_input_clip = "-i " + content_path + key
                            if not no_fps_limit:
                                transcode_input_clip = "-fps " + str(
                                    fps_constraint
                                ) + " " + transcode_input_clip
                            dispatch_cmdline = dispatch_cmdline.replace(
                                "-i <>", transcode_input_clip)

                        ######################################################################################################
                        # this below code is for constructing unique output for each stream.
                        # e.g. Crowdrun_720p_output_1.264, Crowdrun_720p_output_2.264,
                        # and constructing unique log file for each stream for further post processing
                        # e.g. Crowdrun_720p_log_1.txt, Crowdrun_720p_log_2.txt,
                        # jiwan
                        ##################################################################################
                        # Adding Constraint and UnConstraint FPS
                        if (ffmpeg_mode):
                            transcode_output_clip = local_output_path + clip_name + "_" + clip_resolution + "_" + str(
                                m)
                            if use_vdenc:
                                transcode_output_clip = " -low_power true " + transcode_output_clip
                            dispatch_cmdline = dispatch_cmdline.replace(
                                "-y <>", "-y " + transcode_output_clip)

                        else:  # SMT section (DEFAULT)
                            transcode_output_clip = local_output_path + clip_name + "_" + clip_resolution + "_" + str(
                                m)
                            dispatch_cmdline = dispatch_cmdline.replace(
                                "-o::h264 <>",
                                "-o::h264 " + transcode_output_clip)
                            dispatch_cmdline = dispatch_cmdline.replace(
                                "-o::h265 <>",
                                "-o::h265 " + transcode_output_clip)
                            dispatch_cmdline = dispatch_cmdline.replace(
                                "-o::av1 <>",
                                "-o::av1 " + transcode_output_clip)

                        # Adding Transcode_Output_Log file and Multiple Device knobs
                        if (ffmpeg_mode):
                            transcode_output_logfile = " 2> " + temp_path + clip_name + "_" + clip_resolution + "_" + str(
                                streamnumber) + "_" + str(
                                    m) + "_transcode_log.txt"
                            ffmpeg_device_knob = " -qsv_device " + os_env_DEVICE
                            transcode_output_logfile = ffmpeg_device_knob + transcode_output_logfile
                            dispatch_cmdline = dispatch_cmdline.replace(
                                "-report", transcode_output_logfile).rstrip()

                        elif sequence_mode == "TRANSCODE":  # Transcode
                            transcode_output_logfile = " -p " + temp_path + clip_name + "_" + clip_resolution + "_" + str(
                                m) + "_transcode_log.txt"
                            smt_device_knob = " -device " + os_env_DEVICE
                            transcode_output_logfile = smt_device_knob + transcode_output_logfile
                            if use_vdenc:
                                transcode_output_logfile = " -lowpower:on " + transcode_output_logfile
                            dispatch_cmdline = dispatch_cmdline.replace(
                                "-p <>", transcode_output_logfile).rstrip()

                        elif (sequence_mode
                              == "DECODE") and not ffmpeg_mode:  # SMT DECODE
                            transcode_output_logfile = " > " + temp_path + clip_name + "_" + clip_resolution + "_" + str(
                                m) + "_transcode_log.txt"
                            smt_device_knob = " -device " + os_env_DEVICE
                            transcode_output_logfile = smt_device_knob + transcode_output_logfile
                            dispatch_cmdline = dispatch_cmdline.replace(
                                "-p <>", transcode_output_logfile).rstrip()

                        ##################################################################################
                        # Adding piping to log file and strip out the error messages
                        ##################################################################################
                        if (ffmpeg_mode):
                            dispatch_cmdline = dispatch_cmdline + "\n"

                        elif (sequence_mode == "TRANSCODE"):  # SMT TRANSCODE
                            dispatch_cmdline = dispatch_cmdline + " >> " + temp_path + "console_log.txt 2>&1" + "\n"
                        elif (sequence_mode == "DECODE"):  # SMT DECODE
                            dispatch_cmdline = dispatch_cmdline + " 2>&1" + "\n"

                        performance_object_list[
                            curContent].dispatch_cmdline = dispatch_cmdline
                        shell_script_handle.write(dispatch_cmdline)
                    shell_script_handle.close()

                    nextStreamNumber0 = False
                    nextStreamNumber1 = False
                    nextStreamNumber2 = False
                    nextStreamNumber3 = False

                    ##################################################################################
                    # Dispatch and Post Process each result +-2%
                    ##################################################################################
                    for j in range(maximum_iteration):
                        printLog(output_log_handle, "\n")
                        printLog(output_log_handle, '#' * 69)
                        print_performance_label = "PNP MEDIA " + performance_app_tag + " " + performance_tag + ": " + "MULTISTREAM: " + str(
                            streamnumber) + " & ITERATION: " + str(j)
                        if (debug_verbose):
                            print_performance_label += " - " + clip_name
                        printLog(output_log_handle, print_performance_label)
                        printLog(output_log_handle, '#' * 69)

                        linux_perf_dump = temp_path + clip_stream_iter_tag + "_" + str(
                            j) + "_metrics.txt"
                        filename_gpu_freq_trace = clip_stream_iter_tag + "_" + str(
                            j) + "_gpu_freq_traces"
                        linux_perf_gpu_freq_trace_dump = temp_path + filename_gpu_freq_trace + ".txt"
                        filename_mem_bw_trace = clip_stream_iter_tag + "_" + str(
                            j) + "_mem_bw_traces"
                        linux_perf_mem_bw_trace_dump = temp_path + filename_mem_bw_trace + ".txt"

                        performance_object_list[
                            curContent].temp_path = temp_path
                        performance_object_list[
                            curContent].linux_perf_dump = linux_perf_dump
                        performance_object_list[
                            curContent].filename_gpu_freq_trace = filename_gpu_freq_trace
                        performance_object_list[
                            curContent].linux_perf_gpu_freq_trace_dump = linux_perf_gpu_freq_trace_dump
                        performance_object_list[
                            curContent].filename_mem_bw_trace = filename_mem_bw_trace
                        performance_object_list[
                            curContent].linux_perf_mem_bw_trace_dump = linux_perf_mem_bw_trace_dump
                        performance_object_list[
                            curContent].tool_linux_perf_trace = tool_linux_perf_trace
                        performance_object_list[
                            curContent].ffmpeg_mode = ffmpeg_mode

                        graphic_model = "generic"  # must search for profiling what graphic model that we performance.
                        metrics, gpu_freq_traces, mem_bw_traces = selectLinuxPerfMetrics(
                            temp_path, graphic_model)

                        linux_perf_cmdlines = os.path.dirname(
                            os.path.realpath(__file__)) + "/MSGo.py"
                        if tool_linux_perf:
                            if metrics != "":
                                linux_perf_cmdlines = "perf stat -a -e {} -o " + linux_perf_dump + " " + linux_perf_cmdlines
                                linux_perf_cmdlines = linux_perf_cmdlines.format(
                                    metrics)
                            if gpu_freq_traces != "":
                                linux_perf_cmdlines = "perf stat -I 100 -a -e {} -o " + linux_perf_gpu_freq_trace_dump + " " + linux_perf_cmdlines
                                linux_perf_cmdlines = linux_perf_cmdlines.format(
                                    gpu_freq_traces)
                            if mem_bw_traces != "":
                                linux_perf_cmdlines = "perf stat -I 100 -a -e {} -o " + linux_perf_mem_bw_trace_dump + " " + linux_perf_cmdlines
                                linux_perf_cmdlines = linux_perf_cmdlines.format(
                                    mem_bw_traces)

                        performance_object_list[
                            curContent].linux_perf_cmdlines = linux_perf_cmdlines
                        p = subprocess.Popen(linux_perf_cmdlines,
                                             shell=True,
                                             stderr=subprocess.PIPE)
                        #p.send_signal(signal.SIGINT)
                        p.wait()

                        if (p.returncode != 0):
                            if debug_verbose:
                                printLog(
                                    output_log_handle, " [VERBOSE][CMD]",
                                    performance_object_list[curContent].
                                    dispatch_cmdline)
                                printLog(
                                    output_log_handle,
                                    " [VERBOSE][LINUX_PERF_TOOLS]",
                                    performance_object_list[curContent].
                                    linux_perf_cmdlines, "\n")
                                printLog(
                                    output_log_handle,
                                    " Exit early, hang/error in submitted commands:",
                                    str(p.returncode))
                            else:
                                printLog(
                                    output_log_handle,
                                    " Exit early, hang/error in submitted commands:",
                                    str(p.returncode),
                                    "(please use -v to debug further)")

                            sys.exit(p.returncode)

                        os.system('stty sane')
                        ############################################################################################
                        # rename file with iteration# and put into the content object information
                        ############################################################################################
                        filename_gpumem_trace = clip_stream_iter_tag + "_" + str(
                            j) + "_gpumem_trace"
                        gemobject_gpumem_trace_dump = temp_path + filename_gpumem_trace + ".txt"
                        performance_object_list[
                            curContent].filename_gpumem_trace = filename_gpumem_trace
                        performance_object_list[
                            curContent].gemobject_gpumem_trace_dump = gemobject_gpumem_trace_dump
                        os.system("mv " + temp_path + clip_stream_iter_tag +
                                  "_gpumem_trace.txt " +
                                  gemobject_gpumem_trace_dump)

                        ############################################################################################
                        # rename file with iteration# and put into the content object information
                        ############################################################################################
                        filename_cpumem_trace = clip_stream_iter_tag + "_" + str(
                            j) + "_cpumem_trace"
                        top_cpumem_trace_dump = temp_path + filename_cpumem_trace + ".txt"
                        performance_object_list[
                            curContent].filename_cpumem_trace = filename_cpumem_trace
                        performance_object_list[
                            curContent].top_cpumem_trace_dump = top_cpumem_trace_dump
                        os.system("mv " + temp_path + clip_stream_iter_tag +
                                  "_cpumem_trace.txt " + top_cpumem_trace_dump)

                        ############################################################################################

                        nextStreamNumber0, avg_fps, exetime = \
                        postprocess_multistream(output_log_handle, streamnumber, j, debug_verbose, filepath_free_info, filepath_lscpu_info, performance_object_list[curContent],sequence_mode)

                        if j == 0:
                            move_command = "mv " + temp_path + "*transcode_log.txt " + temp_path + "iteration_0/."
                            os.system(move_command)
                            avg_fps_split.append(avg_fps)
                        if j == 1:
                            move_command = "mv " + temp_path + "*transcode_log.txt " + temp_path + "iteration_1/."
                            os.system(move_command)
                            avg_fps_split.append(avg_fps)
                        if j == 2:
                            move_command = "mv " + temp_path + "*transcode_log.txt " + temp_path + "iteration_2/."
                            os.system(move_command)
                            avg_fps_split.append(avg_fps)
                        if j == 3:
                            move_command = "mv " + temp_path + "*transcode_log.txt " + temp_path + "iteration_3/."
                            os.system(move_command)
                            avg_fps_split.append(avg_fps)

                        printLog(output_log_handle, "ITERATION: Done")

                    ##################################################################################
                    # Compare and Decide
                    ##################################################################################
                    if nextStreamNumber0 or nextStreamNumber1 or nextStreamNumber2 or nextStreamNumber3:
                        performance_stream = streamnumber
                        performance_fps = round(statistics.mean(avg_fps_split),
                                                1)
                        performance_exetime = exetime
                        performance_avg_freq = avg_freq
                        performance_sweeping_table[key].append(performance_fps)
                        nextStreamNumber = True
                        if int(streamnumber) == 1:
                            performance_table = key + "," + str(
                                performance_fps)
                            bt_output_handle.write(performance_table)
                        else:
                            performance_table = "," + str(performance_fps)
                            bt_output_handle.write(performance_table)

                        streamnumber = streamnumber + 1
                    elif int(streamnumber) == 1:
                        performance_stream = 0
                        performance_fps = round(statistics.mean(avg_fps_split),
                                                1)
                        performance_exetime = exetime
                        performance_avg_freq = avg_freq
                        printLog(output_log_handle, " average current TPT: ",
                                 performance_stream, "average_fps: ",
                                 performance_fps)
                        performance_sweeping_table[key].append(performance_fps)
                        nextStreamNumber = False
                        performance_table = key + "," + str(performance_fps)
                        bt_output_handle.write(performance_table)
                    else:
                        last_failing_fps = round(
                            statistics.mean(avg_fps_split), 1)
                        performance_sweeping_table[key].append(
                            last_failing_fps
                        )  # to add the last failing multistreams.
                        nextStreamNumber = False
                        performance_table = "," + str(last_failing_fps)
                        bt_output_handle.write(performance_table)

                    printLog(output_log_handle, "MULTISTREAM: Done")

                ##################################################################################
                # Before go to next workload, captured the measure result
                ##################################################################################
                measure = key + "," + str(performance_stream) + "," \
                            + str(performance_fps)  + "," + str(performance_exetime) + "," \
                            + str(performance_avg_freq) + "\n"
                bm_output_handle.write(measure)

                performance_table = "\n"
                bt_output_handle.write(performance_table)

                if str(performance_sweeping_table[key])[1:-1] == "":
                    printLog(output_log_handle, key, ": SKIPPED")
                else:
                    printLog(output_log_handle, " measure streams of", key,
                             ":",
                             len(performance_sweeping_table[key]) - 1, ":",
                             str(performance_sweeping_table[key])[1:-1])
                printLog(output_log_handle, "WORKLOAD: Done")

            ##################################################################################
            # Performance sequence is done
            ##################################################################################

            ##################################################################################
            # Saved traces.
            ##################################################################################

            output_directory_archived = re.sub(
                r'.txt',
                "_" + performance_app_tag + "_" + performance_tag + "_traces",
                output_log_filename)
            output_directory_archived = artifact_path + output_directory_archived
            move_command = "mv " + temp_path + " " + output_directory_archived
            os.system(move_command)

            # print out total time during the measure sequence
            end_message = "PERFORMANCE SEQUENCE : " + performance_tag + " : Done"
            execution_time(output_log_handle, end_message, startTime_sequence,
                           time.time())

            ##################################################################################
            # start new start time of next measure sequence
            ##################################################################################
            startTime_sequence = time.time()

        end_message = "PERFORMANCE APPLICATION: " + performance_app_tag + " : Done"
        execution_time(output_log_handle, end_message, performance_starttime,
                       time.time())

        ##################################################################################
        # Rename output/ path into output_<Application>/
        ##################################################################################
        archived_local_output_path = re.sub(r'output',
                                            "output_" + performance_app_tag,
                                            local_output_path)
        isDirectory = os.path.isdir(archived_local_output_path)
        if (isDirectory):
            rename_application_output_cmd = ("mv " + local_output_path + "* " +
                                             archived_local_output_path + ".")
            os.system(rename_application_output_cmd)
            os.system("rm -r local_output_path"
                      )  # remove empty output directory after the archiving.
        else:
            rename_application_output_cmd = ("mv " + local_output_path + " " +
                                             archived_local_output_path)
            os.system(rename_application_output_cmd)
        ##################################################################################
        # start new start time of next measure application
        ##################################################################################
        performance_starttime = time.time()

    end_message = "PNP MEDIA PERFORMANCE: Done"
    execution_time(output_log_handle, end_message, startTime, time.time())

    printLog(output_log_handle, "\nDate:", time.ctime())


######################## PnP SiLab Sumack ########################################
def message_block(output_log_handle, block_color, block_str):
    color_str = {
        'red': u'\u001b[41;1m',
        'green': u'\u001b[42;1m',
        'white': u'\u001b[7m',
        'yellow': u'\u001b[43:1m'
    }
    formatted_string = '\n' + color_str[block_color] + ' ' * (4 + len(block_str)) + u'\u001b[0m\n' + color_str[
        block_color] \
                       + '  ' + block_str + u'  \u001b[0m\n' + color_str[block_color] + ' ' * (
                               4 + len(block_str)) + u'\u001b[0m\n'
    printLog(output_log_handle, formatted_string)


##################################################################################
# Post Process Multistream FPS/stream against Content_FPS definition
# jiwan
##################################################################################
def postprocess_multistream(output_log_handle, stream_number, iteration_number,
                            debug_verbose, tools_free_info, tools_lscpu_info,
                            performance_object, sequence_mode):
    if debug_verbose:
        printLog(output_log_handle, " [VERBOSE][CMD]",
                 performance_object.dispatch_cmdline)
        printLog(output_log_handle, " [VERBOSE][LINUX_PERF_TOOLS]",
                 performance_object.linux_perf_cmdlines, "\n")

    next = True
    runtime = vid0_utilization = vid1_utilization = render_utilization = cpu_ipc = \
    rc6_utilization = avg_frequency = cpu_task_clock = \
    total_vm_mem_value = total_res_mem_value = total_shr_mem_value = \
    avg_vm_mem_value = avg_res_mem_value = avg_shr_mem_value = \
    avg_cpu_percentage = avg_mem_utilization = total_CPU_percents_streams = \
    total_MEM_percents_streams = max_cpu_percentage = max_physical_mem = total_res_gpumem = avg_res_gpumem = \
    avg_avg_cpu_percents_streams = avg_avg_mem_percents_streams = 0

    ##################################################################################
    # Example of Free info dump file
    #               total        used        free      shared  buff/cache   available
    # Mem:       16319120      702844      551948        9224    15064328    15294664
    ##################################################################################
    free_list = {}
    if os.path.isfile(tools_free_info):
        with open(tools_free_info, "r") as free_info_handle:
            for free_info in free_info_handle:
                free_info = re.sub(r'\s+', " ", free_info)
                if "Mem:" in free_info:
                    free_info_split = free_info.split(" ")
                    free_list["Mem_Total"] = float(free_info_split[1]) / 1000
                    free_list["Mem_Used"] = float(free_info_split[2]) / 1000
                    free_list["Mem_Avail"] = float(free_info_split[6]) / 1000

    ##################################################################################
    # Example of LSCPU info dump file
    # Architecture:                    x86_64
    # CPU op-mode(s):                  32-bit, 64-bit
    # Byte Order:                      Little Endian
    # Address sizes:                   39 bits physical, 48 bits virtual
    # CPU(s):                          6
    # On-line CPU(s) list:             0-5
    # Thread(s) per core:              1
    # Core(s) per socket:              6
    # Socket(s):                       1
    # Model name:                      Intel(R) Core(TM) i5-9600T CPU @ 2.30GHz
    # Stepping:                        10
    # CPU MHz:                         3611.692
    # CPU max MHz:                     3900.0000
    # CPU min MHz:                     800.0000
    ##################################################################################
    lscpu_list = {}
    if os.path.isfile(tools_lscpu_info):
        lscpu_list["Sockets"] = lscpu_list["CoresPerSocket"] = lscpu_list[
            "ThreadsPerCore"] = 0
        with open(tools_lscpu_info, "r") as lscpu_info_handle:
            for lscpu_info in lscpu_info_handle:
                lscpu_info = re.sub(r'\s+', " ", lscpu_info)
                if re.compile(r'^CPU\(s\)\:').search(lscpu_info):
                    lscpu_info_split = lscpu_info.split(":")
                    lscpu_list["CPUs"] = int(
                        lscpu_info_split[1]
                    ) * 100  # Total percentage from all CPUs
                elif re.compile(r'^Thread\(s\) per core:').search(lscpu_info):
                    lscpu_info_split = lscpu_info.split(":")
                    lscpu_list["ThreadsPerCore"] = lscpu_info_split[1]
                elif re.compile(r'^Core\(s\) per socket:').search(lscpu_info):
                    lscpu_info_split = lscpu_info.split(":")
                    lscpu_list["CoresPerSocket"] = lscpu_info_split[1]
                elif re.compile(r'^Socket\(s\):').search(lscpu_info):
                    lscpu_info_split = lscpu_info.split(":")
                    lscpu_list["Sockets"] = lscpu_info_split[1]
                elif re.compile(r'^CPU MHz:').search(lscpu_info):
                    lscpu_info_split = lscpu_info.split(":")
                    lscpu_list["CurCPUFreq"] = lscpu_info_split[1]

    temp_file = performance_object.temp_path + "iteration_temp.txt"
    average_fps = 0

    # 5% (default)
    fps_target = round(0.95 * float(performance_object.fps_target), 2)

    if (performance_object.ffmpeg_mode):
        cmd_grep = "grep -b1 'video:' " + performance_object.temp_path + "*transcode_log.txt " + "> " + temp_file
        exit_progress = os.system(cmd_grep)
        cmd_perl = "perl -pi -e 's/^.*video\:.*\n//' " + performance_object.temp_path + "iteration_temp.txt"
        os.system(cmd_perl)
        cmd_perl = "perl -pi -e 's/^.*fps=\s*/fps /' " + performance_object.temp_path + "iteration_temp.txt"
        os.system(cmd_perl)
        cmd_perl = "perl -pi -e 's/q=.*\n//' " + performance_object.temp_path + "iteration_temp.txt"
        os.system(cmd_perl)
    else:
        if (sequence_mode == "DECODE"):
            cmd_grep = "grep -H ' fps' " + performance_object.temp_path + "*transcode_log.txt " + "> " + temp_file
            exit_progress = os.system(cmd_grep)
            cmd_perl = "perl -pi -e 's/^.*\sfps:\s*/fps /' " + performance_object.temp_path + "iteration_temp.txt"
            os.system(cmd_perl)
            cmd_perl = "perl -pi -e 's/,.*\n/\n/' " + performance_object.temp_path + "iteration_temp.txt"
            os.system(cmd_perl)
        else:
            cmd_grep = "grep -H ' fps' " + performance_object.temp_path + "*transcode_log.txt " + "> " + temp_file
            exit_progress = os.system(cmd_grep)
            cmd_perl = "perl -pi -e 's/_transcode_log.*,//' " + performance_object.temp_path + "iteration_temp.txt"
            os.system(cmd_perl)

    if exit_progress == 0:
        # using With to open file to ensure the next codes are blocked until the file is exited.
        with open(temp_file, "r") as b2b_temp_fh:
            fps_per_stream_previous = 0
            average_iter = 0
            average_fps_split = []
            for line in b2b_temp_fh:
                line_split = line.split(' ')
                if len(line_split) < 2:
                    continue
                elif not float(line_split[1]):
                    continue
                fps_per_stream = round(float(line_split[1]), 2)
                average_fps_split.append(fps_per_stream)
                ##################################################################################
                # Post Process Multistream FPS/stream against Content FPS limit target
                ##################################################################################
                if fps_per_stream < fps_target:
                    next = False

                ##################################################################################
                # Post Process Multistream FPS/stream is within 2% margin among its own average
                ##################################################################################
                two_percent_margin = True
                delta_margin = 0
                if fps_per_stream_previous > 0:
                    delta_margin = round(
                        abs(1 - (fps_per_stream / fps_per_stream_previous)), 2)
                    two_percent_margin = True if delta_margin <= 0.02 else False
                else:
                    fps_per_stream_previous = fps_per_stream

                ##################################################################################
                # Readable console output for fps/stream, 2%margin, and limit
                ##################################################################################
                output_file_split = line_split[0].split("/")
                output_file_split_len = len(output_file_split)
                clipname = output_file_split[output_file_split_len - 1]
                print_check_result = " CONCURRENT: " + clipname + " " + str(
                    fps_per_stream
                ) + " fps/stream" + " , Meets ContentFPS " + str(
                    fps_target) + " : " + str(
                        next) + " , Within margin 2%: " + str(
                            delta_margin) + " " + str(two_percent_margin)
                printLog(output_log_handle, print_check_result)

                ##################################################################################
                # Report Summary print out
                ##################################################################################
                average_iter = average_iter + 1

            average_fps = statistics.mean(
                average_fps_split) if average_fps_split != [] else 0

    ##################################################################################
    # Post Process Top Tools for GPU Utilization and Memory Footprint/Usage
    # Example GEM Object printout
    # sample_multi_tr: 253 objects, 125353984 bytes (44523520 active, 29716480 inactive, 60416000 unbound, 0 closed)
    ##################################################################################
    if os.path.isfile(performance_object.gemobject_gpumem_trace_dump):
        gpumem_res_max = 0
        with open(performance_object.gemobject_gpumem_trace_dump,
                  "r") as tools_gemobject:
            for gemmem_line in tools_gemobject:
                gemmem_line = re.sub(r'\(', "", gemmem_line)
                gemmem_line_split = gemmem_line.split(" ")
                if len(gemmem_line_split) > 10:
                    gpumem_res_cur = float(
                        (int(gemmem_line_split[3]) - int(gemmem_line_split[9]))
                        / 1000000)
                    if gpumem_res_cur > gpumem_res_max:
                        gpumem_res_max = gpumem_res_cur

        avg_res_gpumem = round(gpumem_res_max, 2)
        total_res_gpumem = round(gpumem_res_max * stream_number, 2)

    ##################################################################################
    # Post Process Top Tools for CPU Utilization and Memory Footprint/Usage
    # Example TOP print out per process.
    #   PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND
    # 24480 intel     20   0  486824  27372  15172 R  33.3   0.2   0:00.05 sample_mu+
    #     1 1000      20   0    4084   3456   3024 S   0.0   0.0   0:00.24 sample_mu+
    ##################################################################################
    if os.path.isfile(performance_object.top_cpumem_trace_dump):
        import numpy as np
        dump_top_list = {}
        top_pid_list = []
        VM_footprint_max_per_pid_array = {}
        RES_footprint_max_per_pid_array = {}
        SHR_footprint_max_per_pid_array = {}
        CPU_percent_per_pid_array = {}
        MEM_percent_per_pid_array = {}
        CPU_percent_per_pid_average = {}
        MEM_percent_per_pid_average = {}

        with open(performance_object.top_cpumem_trace_dump, "r") as tools_top:
            for line in tools_top:
                line_list = re.sub(
                    r'^\s+', "", line)  # Remove any leading empty characters.
                line_list = re.sub(r'\s+', " ", line_list).split(" ")
                top_pid = int(line_list[0])
                top_vm_mem = int(line_list[4]) if line_list[4].isdigit() else 0
                top_res_mem = int(line_list[5])
                top_shr_mem = int(line_list[6])
                top_cpu_percent = float(line_list[8])
                top_mem_percent = float(line_list[9])

                if not (top_pid in top_pid_list):
                    top_pid_list.append(top_pid)
                    VM_footprint_max_per_pid_array[top_pid] = top_vm_mem
                    RES_footprint_max_per_pid_array[top_pid] = top_res_mem
                    SHR_footprint_max_per_pid_array[top_pid] = top_shr_mem
                    CPU_percent_per_pid_array[top_pid] = []
                    MEM_percent_per_pid_array[top_pid] = []
                    CPU_percent_per_pid_array[top_pid].append(top_cpu_percent)
                    MEM_percent_per_pid_array[top_pid].append(top_mem_percent)

                else:
                    CPU_percent_per_pid_array[top_pid].append(top_cpu_percent)
                    MEM_percent_per_pid_array[top_pid].append(top_mem_percent)

                    if VM_footprint_max_per_pid_array[top_pid] < top_vm_mem:
                        VM_footprint_max_per_pid_array[top_pid] = top_vm_mem

                    if RES_footprint_max_per_pid_array[top_pid] < top_res_mem:
                        RES_footprint_max_per_pid_array[top_pid] = top_res_mem

                    if SHR_footprint_max_per_pid_array[top_pid] < top_shr_mem:
                        SHR_footprint_max_per_pid_array[top_pid] = top_shr_mem

        for each_pid in VM_footprint_max_per_pid_array:
            total_vm_mem_value = round(
                float(total_vm_mem_value +
                      VM_footprint_max_per_pid_array[each_pid]) / 1000, 2)
        for each_pid in VM_footprint_max_per_pid_array:
            total_res_mem_value = round(
                float(total_res_mem_value +
                      RES_footprint_max_per_pid_array[each_pid]) / 1000, 2)
        for each_pid in VM_footprint_max_per_pid_array:
            total_shr_mem_value = round(
                float(total_shr_mem_value +
                      SHR_footprint_max_per_pid_array[each_pid]) / 1000, 2)

        if (avg_vm_mem_value != 0) and (avg_res_mem_value
                                        != 0) and (avg_shr_mem_value != 0):
            avg_vm_mem_value = round(total_vm_mem_value / len(top_pid_list),
                                     2)  #take average every each value
            avg_res_mem_value = round(total_res_mem_value / len(top_pid_list),
                                      2)  # take average every each value
            avg_shr_mem_value = round(total_shr_mem_value / len(top_pid_list),
                                      2)  # take average every each value

        dump_top_list["stream_ID_List:"] = str(top_pid_list) + " list"
        dump_top_list["Max_VM_Mem_list:"] = str(
            VM_footprint_max_per_pid_array) + " list"
        dump_top_list["Max_VM_Mem:"] = str(avg_vm_mem_value) + " (MB/stream)"
        dump_top_list["Max_RES_Mem_list:"] = str(
            RES_footprint_max_per_pid_array) + " list"
        dump_top_list["Max_RES_Mem:"] = str(
            avg_res_mem_value) + " (MB/stream), " + str(
                total_res_mem_value) + " (MB) total"
        dump_top_list["Max_SHR_Mem_list:"] = str(
            SHR_footprint_max_per_pid_array) + " list"
        dump_top_list["Max_SHR_Mem:"] = str(avg_shr_mem_value) + " (MB/stream)"

        for pid_value in top_pid_list:
            CPU_percent_per_pid_average[pid_value] = round(
                np.average(CPU_percent_per_pid_array[pid_value]), 2)
            MEM_percent_per_pid_average[pid_value] = round(
                np.average(MEM_percent_per_pid_array[pid_value]), 2)

        dump_top_list["CPU_Utilization_List:"] = str(
            CPU_percent_per_pid_average) + " list"
        dump_top_list["MEM_Utilization_List:"] = str(
            MEM_percent_per_pid_average) + " list"

        for each_pid in CPU_percent_per_pid_average:
            total_CPU_percents_streams = total_CPU_percents_streams + CPU_percent_per_pid_average[
                each_pid]
        for each_pid in MEM_percent_per_pid_average:
            total_MEM_percents_streams = total_MEM_percents_streams + MEM_percent_per_pid_average[
                each_pid]

        if (total_CPU_percents_streams != 0):
            avg_avg_cpu_percents_streams = round(
                total_CPU_percents_streams / len(top_pid_list),
                2)  # take average every each value
        if (total_MEM_percents_streams != 0):
            avg_avg_mem_percents_streams = round(
                total_MEM_percents_streams / len(top_pid_list),
                2)  # take average every each value
        print(avg_avg_cpu_percents_streams)
        dump_top_list["AVG_CPU_Util:"] = str(avg_avg_cpu_percents_streams)
        dump_top_list["AVG_MEM_Util:"] = str(avg_avg_mem_percents_streams)

    ##################################################################################
    # Post Process Linux Perf for GPU Utilization and Average GPU Frequency
    ##################################################################################
    if os.path.isfile(performance_object.linux_perf_dump):
        dump_lp_list = {}
        with open(performance_object.linux_perf_dump, "r") as tools_lp:
            for line in tools_lp:
                line = line.replace("\t", "")
                if re.search(r'\sns', line):
                    line = line.replace(" ns", ":")
                elif re.search(r'\smsec', line):
                    line = line.replace(" msec", ":")
                elif re.search(" M", line):
                    line = line.replace(" M", ":")
                elif re.search(" seconds", line):
                    line = line.replace(" seconds", ":")
                elif re.search("cpu-cycles", line):
                    line = line.replace("cpu-cycles", ":cpu-cycles")
                elif re.search("instructions", line):
                    line = line.replace("instructions", ":instructions")
                else:
                    continue

                line_split = line.split(":")

                if (len(line_split) > 1):
                    line_lp_value = line_split[0].replace(" ",
                                                          "").replace(",", "")
                    line_lp_description = line_split[1].strip()
                    if re.search(r'time elapsed', line_lp_description):
                        line_lp_description = "Runtime(seconds)"
                    else:
                        continue

                    dump_lp_list[line_lp_description] = float(
                        line_lp_value.strip())

                else:
                    continue

        runtime = round(dump_lp_list["Runtime(seconds)"], 2)

        printLog(output_log_handle, "\n [TOOLS][LINUX_PERF]")

        printLog(output_log_handle, " GPU analysis: ")
        printLog(output_log_handle, "\tRuntime\t\t:", runtime, "seconds")

        ##################################################################################
        # Post Process Linux Perf for Traces
        ##################################################################################
        if (performance_object.tool_linux_perf_trace):
            import matplotlib.pyplot as plt
            import numpy as np
            printLog(output_log_handle, " Traces Chart:")
            ##################################################################################
            # Post Process Linux Perf for GPU Analysis Traces - GT Freq
            ##################################################################################
            if os.path.isfile(
                    performance_object.linux_perf_gpu_freq_trace_dump):
                y_axis = []
                with open(performance_object.linux_perf_gpu_freq_trace_dump,
                          'r') as lp_traces:
                    for sampling_line in lp_traces:
                        sampling_line_split = re.sub('[^0-9a-zA-Z\.]+', "_",
                                                     sampling_line).split("_")
                        if len(sampling_line_split) < 3:
                            continue

                        if (sampling_line_split[1] == "Performance"):
                            break

                        if re.search(r'actual-frequency', sampling_line):
                            freq_trace = round(
                                float(int(sampling_line_split[2]) / 100), 2)
                            y_axis.append(freq_trace)

                x_axis = np.arange(len(y_axis))
                plot_output = plt.figure()
                subplot_output = plt.subplot()
                subplot_output.plot(x_axis, y_axis, label='GT-Freq(GHz)')
                trace_title = "MSPerf Trace - " + performance_object.filename_gpu_freq_trace
                plt.title(trace_title)
                subplot_output.legend()
                # plt.show()

                plot_filename = performance_object.temp_path + performance_object.filename_gpu_freq_trace + ".png"
                plot_output.savefig(plot_filename)
                printLog(output_log_handle, "\tGPU-Freq-Trace\t:",
                         re.sub(r'.*\/', "", plot_filename))

            ##################################################################################
            # Post Process Linux Perf for GPU Analysis Traces - Mem BW
            ##################################################################################
            if os.path.isfile(performance_object.linux_perf_mem_bw_trace_dump):
                mem_bw_rd = []
                mem_bw_wr = []
                with open(performance_object.linux_perf_mem_bw_trace_dump,
                          'r') as lp_gpubw_traces:
                    for membw_line in lp_gpubw_traces:
                        membw_line_split = re.sub('[^0-9a-zA-Z\.]+', "_",
                                                  membw_line).split("_")

                        if len(membw_line_split) < 3:
                            continue

                        if (membw_line_split[1] == "Performance"):
                            break

                        if re.search(r'data_reads', membw_line) and type(
                                membw_line_split[2]) == "float":
                            read_trace = round(
                                (float(membw_line_split[2]) * 1.024 / 100), 2)
                            mem_bw_rd.append(read_trace)
                        elif re.search(r'data_writes', membw_line) and type(
                                membw_line_split[2]) == "float":
                            write_trace = round(
                                (float(membw_line_split[2]) * 1.024 / 100), 2)
                            mem_bw_wr.append(write_trace)
                        else:
                            continue

                r = np.arange(len(mem_bw_rd))
                plot_output = plt.figure()
                subplot_output = plt.subplot()
                subplot_output.plot(r,
                                    mem_bw_rd,
                                    label='MEMORY-Read-BW-Traces(MB/s)')
                trace_title = "MSPerf Trace - " + performance_object.filename_mem_bw_trace
                plt.title(trace_title)
                subplot_output.legend()
                # plt.show()

                plot_filename = performance_object.temp_path + performance_object.filename_mem_bw_trace + "_read.png"
                plot_output.savefig(plot_filename)
                #printLog(output_log_handle, "\tMEM-RD-BW-Trace\t:", re.sub(r'.*\/',"" , plot_filename))

                w = np.arange(len(mem_bw_wr))
                plot_output = plt.figure()
                subplot_output = plt.subplot()
                subplot_output.plot(w,
                                    mem_bw_wr,
                                    label='MEMORY-Write-BW-Traces(MB/s)')
                trace_title = "MSPerf Trace - " + performance_object.filename_mem_bw_trace
                plt.title(trace_title)
                subplot_output.legend()
                # plt.show()

                plot_filename = performance_object.temp_path + performance_object.filename_mem_bw_trace + "_write.png"
                plot_output.savefig(plot_filename)
                #printLog(output_log_handle, "\tMEM-WR-BW-Trace\t:", re.sub(r'.*\/',"" , plot_filename))

            ###################################
            # myplotlib
            # Clean up after plot creation.
            ###################################
            plt.cla()  # Clear an axis
            plt.clf()  # Clears the entire current figure
            plt.close('all')  # close all open figures.

        if debug_verbose:
            printLog(output_log_handle, " [VERBOSE] RAW files:")
            printLog(output_log_handle, "\tmetrics\t\t:",
                     re.sub(r'.*\/', "", performance_object.linux_perf_dump))
            printLog(
                output_log_handle, "\tGPU-Freq traces\t:",
                re.sub(r'.*\/', "",
                       performance_object.linux_perf_gpu_freq_trace_dump))
            printLog(
                output_log_handle, "\tMEMORY traces\t:",
                re.sub(r'.*\/', "",
                       performance_object.linux_perf_mem_bw_trace_dump))

    return next, average_fps, runtime


#################################################
def execution_time(output_log_handle, message, start, end):
    time = float(end - start)
    total = time
    day = time // (24 * 3600)
    time = time % (24 * 3600)
    hour = time // 3600
    time %= 3600
    minutes = time // 60
    time %= 60
    seconds = time

    printLog(
        output_log_handle, message,
        " - execution time: %d s (%dD:%dhr:%dmin:%dsec)\n" %
        (total, day, hour, minutes, seconds))


#################################################
def ffmpegffprobeCheck(output_log_handle, filepath, filename, out_temp_path,
                       debug_verbose, performance_sweeping_table,
                       performance_object_list):
    status = True
    filename_split = filename.replace('.', '_').split('_')
    ffprobe_dump = out_temp_path + "/ffprobe_" + filename_split[0] + ".txt"
    ffprobe_cmd = "ffprobe -v error -hide_banner -loglevel panic -select_streams v:0 -show_streams "
    ffprobe_cmd += "-of default=noprint_wrappers=1 -i " + filepath + filename.rstrip(
    ) + " | grep -E '^height=|^r_frame_rate=|^codec_name=' " + " > " + ffprobe_dump
    os.system(ffprobe_cmd)
    height = encode_bitrate = ffprobe_frame_rate = ffprobe_codec_name = 0
    content_supported_codec = ["h264", "hevc", "av1"]
    with open(ffprobe_dump, "r") as ffprobedump_fh:
        for content_profile_line in ffprobedump_fh:
            content_profile_line_split = content_profile_line.rstrip().split(
                "=")
            attribute = content_profile_line_split[0]
            value = content_profile_line_split[1]

            if (attribute == "height"):
                height = int(value)
                if (height > 1088):
                    encode_bitrate = "4k_bitrate"  # 4K and above to set at same bitrate
                elif (height > 720):
                    encode_bitrate = "hd_bitrate"  # 2K content to set at same bitrate
                else:
                    encode_bitrate = "sd_bitrate"  # others to set at same bitrate
            elif (attribute == "r_frame_rate"):
                any_frame_rates_found = eval(value)
                if any_frame_rates_found != "0":
                    ffprobe_frame_rate = round(float(any_frame_rates_found), 2)
                    if (ffprobe_frame_rate == 1000.0
                        ):  # ffprobe issue for av1 frame rate
                        ffprobe_frame_rate = 60.0
            elif (attribute == "codec_name"):
                if (str(value) in content_supported_codec):
                    ffprobe_codec_name = str(value)

    ######################################################
    # ERROR Checking.
    ######################################################
    filename_only_without_extension = re.sub(r'\..*', "", filename.rstrip())
    duplicate_filename_status = False

    if (encode_bitrate == 0) | (ffprobe_frame_rate == 0) | (ffprobe_codec_name
                                                            == 0):
        if debug_verbose:
            printLog(
                output_log_handle,
                " FAIL: via FFMPEG/FFPROBE: content is not supported, Please reconfirm content height/frame_rate/codec"
            )
        status = False

    if status:
        performance_object_list[filename.rstrip()] = MediaContent()
        performance_object_list[
            filename.rstrip()].name = filename_only_without_extension
        performance_object_list[
            filename.rstrip()].fps_target = ffprobe_frame_rate
        performance_object_list[
            filename.rstrip()].encode_bitrate = encode_bitrate
        performance_object_list[filename.rstrip()].height = height
        performance_object_list[filename.rstrip()].codec = ffprobe_codec_name
        performance_sweeping_table[filename.rstrip()] = []

    return status


##################################################################################
# Print log
##################################################################################
def printLog(filehandle_printout, *args):
    console_printout = ' '.join([str(arg) for arg in args])
    print(console_printout)
    console_printout = console_printout + "\n"
    filehandle_printout.write(console_printout)


def selectLinuxPerfMetrics(temp_path, graphic_model):
    metrics = gpu_freq_traces = mem_bw_traces = ""
    os.system(
        "perf list | grep -E '^\s*i915|^\s*uncore_imc|^\s*cycle_|^\s*task-clock|^\s*cpu-cycles|^\s*instructions OR cpu/instructions/' > "
        + temp_path + "perf_list_metrics.txt")
    with open(temp_path + "perf_list_metrics.txt",
              "r") as linuxPerfMetricHandle:
        for linuxPerfMetric in linuxPerfMetricHandle:
            linuxPerfMetric = re.sub(
                r'^\s+', "", linuxPerfMetric)  # removing any leading spaces
            linuxPerfMetric = re.sub(
                r'\s+', " ", linuxPerfMetric)  # removing any multiple spaces
            linuxPerfMetric_split = linuxPerfMetric.split(" ")
            metrics = linuxPerfMetric_split[0] if (
                metrics == "") else metrics + "," + linuxPerfMetric_split[0]
            if "actual-frequency" in linuxPerfMetric:
                gpu_freq_traces = linuxPerfMetric_split[0]
            elif "uncore_" in linuxPerfMetric:
                mem_bw_traces = linuxPerfMetric_split[0] if (
                    mem_bw_traces
                    == "") else mem_bw_traces + "," + linuxPerfMetric_split[0]

    return metrics, gpu_freq_traces, mem_bw_traces


def sudo_password_request():
    status = 0
    if os.geteuid() != 0:  # pylint: disable=no-member
        msg = "Request password for enabling CPU/GPU analysis: %u:"
        status = subprocess.check_call("sudo -v -p '%s'" % msg, shell=True)
    return status


##################################################################################
# Execute
##################################################################################
main()
