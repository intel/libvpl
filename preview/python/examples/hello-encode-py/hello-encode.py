# pylint: disable=import-error,fixme,invalid-name,too-many-locals,too-many-arguments
# ==============================================================================
#  Copyright Intel Corporation
#
#  SPDX-License-Identifier: MIT
# ==============================================================================
#
#  A minimal oneAPI Video Processing Library (oneVPL) encode application,
#  using 2.x API with internal memory management
"""
Example:  hello-encode -i in.i420 -w 128 -h 96
To view:  ffplay out.h265

* Encode raw frames to HEVC/H265 elementary stream in out.h265
  CPU native color format is I420/yuv420p.  GPU native color format is NV12
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
        formatter_class=argparse.RawTextHelpFormatter,
        # Special handling for help because we use -h
        add_help=False,
    )
    # Special handling for help because we use -h
    parser.add_argument('--help', action='help', help=argparse.SUPPRESS)
    parser.add_argument('--impl',
                        action="store",
                        dest='impl',
                        default='',
                        help=argparse.SUPPRESS)
    parser.add_argument('-sw',
                        action="store_const",
                        const='sw',
                        dest='impl',
                        help='Use software implementation.')
    parser.add_argument('-hw',
                        action="store_const",
                        const='hw',
                        dest='impl',
                        help='Use hardware implementation.')
    parser.add_argument('-i',
                        action="store",
                        dest='input',
                        required=True,
                        help='input file name (raw frames).')
    parser.add_argument('-w',
                        action="store",
                        dest='width',
                        type=int,
                        required=True,
                        help='input width.')
    parser.add_argument('-h',
                        action="store",
                        dest='height',
                        type=int,
                        required=True,
                        help='input height.')
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
        with open("out.h265", "wb") as sink:
            opts = []
            if args.impl == 'sw':
                opts.append(
                    pyvpl.property("mfxImplDescription.Impl",
                                   pyvpl.implementation.software))
                input_fourcc = pyvpl.color_format_fourcc.i420
            elif args.impl == 'hw':
                opts.append(pyvpl.property.HW_ImlpOption)
                input_fourcc = pyvpl.color_format_fourcc.nv12
            opts.append(
                pyvpl.property(
                    "mfxImplDescription.mfxEncoderDescription.encoder.CodecID",
                    pyvpl.codec_format_fourcc.hevc))
            sel_default = pyvpl.default_selector(opts)

            # Load session and initialize encoder
            session = pyvpl.encode_session(sel_default, source)

            params = pyvpl.encoder_video_param()
            info = pyvpl.frame_info()

            info.frame_rate = (30, 1)
            info.frame_size = (roundup(args.width,
                                       16), roundup(args.height, 16))
            info.FourCC = input_fourcc
            info.ChromaFormat = pyvpl.chroma_format_idc.yuv420
            info.ROI = ((0, 0), (args.width, args.height))

            params.RateControlMethod = pyvpl.rate_control_method.cqp
            params.frame_info = info
            params.CodecId = pyvpl.codec_format_fourcc.hevc
            params.IOPattern = pyvpl.io_pattern.in_system_memory

            # Getting VAAPI handle not supported in Python sample
            init_list = pyvpl.encoder_init_list()
            session.Init(params, init_list)

            # check and report implementation details
            version = session.version
            print(f"{version.Major}.{version.Minor}")
            verify(version.Major > 1,
                   "Sample requires 2.x API implementation, exiting")
            impl = session.implementation
            print(impl)

            for bits in session:
                # frame_count += 1
                frame_count += 1
                sink.write(bits)

    print(f"Processed {frame_count} frames")


if __name__ == '__main__':
    print(sys.argv)
    main(read_command_line())
