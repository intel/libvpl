# pylint: disable=import-error,fixme,invalid-name,too-many-locals,too-many-arguments
# ==============================================================================
#  Copyright Intel Corporation
#
#  SPDX-License-Identifier: MIT
# ==============================================================================
#
#  A minimal oneAPI Video Processing Library (oneVPL) vpp application,
#  using 2.x API with internal memory management
"""
Example:  hello-vpp in.i420 128 96
To view:  ffplay -f rawvideo -pixel_format yuv420p -video_size 640x480 -pixel_format yuv420p out.raw

* Resize I420 raw frames to 640x480 size in out.raw
"""

import argparse
import os
import math
import sys
import pyvpl


def read_command_line():
    """
    Read command line arguments
    """
    parser = argparse.ArgumentParser(
        description=globals()['__doc__'],
        formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('input',
                        action="store",
                        help='input file name (i420 raw frames).')
    parser.add_argument('width',
                        action="store",
                        type=int,
                        choices=range(0, 3840),
                        metavar="[0-3840]",
                        help='input width.')
    parser.add_argument('height',
                        action="store",
                        type=int,
                        choices=range(0, 2160),
                        metavar="[0-2160]",
                        help='input height.')
    parser.add_argument('--dest-width',
                        action="store",
                        default=640,
                        dest="dest_width",
                        type=int,
                        help=argparse.SUPPRESS)
    parser.add_argument('--dest-height',
                        action="store",
                        default=480,
                        dest="dest_height",
                        type=int,
                        help=argparse.SUPPRESS)
    args = parser.parse_args()
    args.input = os.path.abspath(args.input)
    return args


def verify(epectation, msg):
    """Check for expected condition and exit if not met"""
    if not epectation:
        print(msg)
        sys.exit(1)


def roundup(value, base=10):
    """Round value up to next multiple of base"""
    result = math.floor((value + base - 1) / base) * base
    return result


def main(args):
    """Main example"""
    frame_count = 0

    with pyvpl.raw_frame_file_reader_by_name(args.width, args.height,
                                             pyvpl.color_format_fourcc.i420,
                                             args.input) as source:
        with open("raw.out", "wb") as sink:
            opts = pyvpl.properties()
            opts.api_version = (2, 5)
            opts.impl = pyvpl.implementation_type.sw
            sel_default = pyvpl.default_selector(opts)

            # Load session and initialize decoder
            params = pyvpl.vpp_video_param()
            in_frame = pyvpl.frame_info()
            in_frame.FourCC = pyvpl.color_format_fourcc.i420
            in_frame.ChromaFormat = pyvpl.chroma_format_idc.yuv420
            in_frame.PicStruct = pyvpl.pic_struct.progressive
            in_frame.frame_rate = (30, 1)
            in_frame.ROI = ((0, 0), (args.width, args.height))
            in_frame.frame_size = (roundup(args.width,
                                           16), roundup(args.height, 16))
            params.in_frame_info = in_frame
            out_frame = pyvpl.frame_info()
            out_frame.FourCC = pyvpl.color_format_fourcc.i420
            out_frame.ChromaFormat = pyvpl.chroma_format_idc.yuv420
            out_frame.PicStruct = pyvpl.pic_struct.progressive
            out_frame.frame_rate = (30, 1)
            out_frame.ROI = ((0, 0), (args.dest_width, args.dest_height))
            out_frame.frame_size = (roundup(args.dest_width,
                                            16), roundup(args.dest_height, 16))
            params.out_frame_info = out_frame
            params.IOPattern = pyvpl.io_pattern.io_system_memory
            init_reset_list = pyvpl.vpp_init_reset_list()
            session = pyvpl.vpp_session(sel_default, source)
            session.Init(params, init_reset_list)
            # check and report implementation details
            version = session.version
            print(f"{version.Major}.{version.Minor}")
            verify(version.Major > 1,
                   "Sample requires 2.x API implementation, exiting")
            impl = session.implementation
            print(impl)
            frame_count = 0
            for frame in session:
                frame_count += 1
                info, data = frame.map(pyvpl.memory_access.read)
                try:
                    planes = data.get_planes(info)
                    for plane in planes:
                        sink.write(plane)
                finally:
                    frame.unmap()

    print(f"Processed {frame_count} frames")


if __name__ == '__main__':
    main(read_command_line())
