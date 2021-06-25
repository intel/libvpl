# pylint: disable=import-error,fixme,invalid-name,too-many-locals,too-many-arguments
# ==============================================================================
#  Copyright Intel Corporation
#
#  SPDX-License-Identifier: MIT
# ==============================================================================
#
#  A minimal oneAPI Video Processing Library (oneVPL) decode application,
#  using 2.x API with internal memory management
"""
Example:  hello-decode -sw  -i in.h265
To view:  ffplay -f rawvideo -pixel_format yuv420p -video_size [width]x[height] out.raw

* Decode HEVC/H265 elementary stream to raw frames in out.raw

  CPU native color format is I420/yuv420p.  GPU native color format is NV12
"""

import argparse
import os
import sys
import pyvpl


def read_command_line():
    """
    Read command line arguments
    """
    parser = argparse.ArgumentParser(
        description=globals()['__doc__'],
        formatter_class=argparse.RawTextHelpFormatter)
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
                        help='input file name (HEVC elementary stream).')
    args = parser.parse_args()
    args.input = os.path.abspath(args.input)
    return args


def verify(epectation, msg):
    """Check for expected condition and exit if not met"""
    if not epectation:
        print(msg)
        sys.exit(1)


def main(args):
    """Main example"""
    frame_count = 0

    with pyvpl.bitstream_file_reader_name(args.input) as source:
        with open("raw.out", "wb") as sink:
            opts = []
            if args.impl == 'sw':
                opts.append(
                    pyvpl.property("mfxImplDescription.Impl",
                                   pyvpl.implementation.software))
            elif args.impl == 'hw':
                opts.append(
                    pyvpl.property("mfxImplDescription.Impl",
                                   pyvpl.implementation.hardware))
            sel_default = pyvpl.default_selector(opts)

            # Load session and initialize decoder
            params = pyvpl.decoder_video_param()
            params.IOPattern = pyvpl.io_pattern.out_system_memory
            params.CodecId = pyvpl.codec_format_fourcc.hevc
            decoder = pyvpl.decode_session(sel_default, params, source)
            # check and report implementation details
            version = decoder.version
            print(f"{version.Major}.{version.Minor}")
            verify(version.Major > 1,
                   "Sample requires 2.x API implementation, exiting")
            i, v = decoder.implementation

            print(f"{str(i)} {v}")

            init_header_list = pyvpl.decoder_init_header_list()
            init_reset_list = pyvpl.decoder_init_reset_list()
            decoder.init_by_header(init_header_list, init_reset_list)

            print(f"Decoding {args.input} -> raw.out")

            print("Output colorspace: ")
            fourcc = decoder.Params.frame_info.FourCC
            print(f"  {fourcc}")

            for frame in decoder:
                frame_count += 1
                info, data = frame.map(pyvpl.memory_access.read)
                try:
                    planes = data.get_planes(info)
                    for plane in planes:
                        sink.write(plane)
                finally:
                    frame.unmap()

                # TODO: debug why we get a segfault if we don't do this.
                frame = None

    print(f"Decoded {frame_count} frames")


if __name__ == '__main__':
    main(read_command_line())
