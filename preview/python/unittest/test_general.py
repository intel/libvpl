# pylint: disable=import-error,fixme,invalid-name,too-many-locals,too-many-arguments,too-many-statements,too-many-public-methods
# ==============================================================================
#  Copyright Intel Corporation
#
#  SPDX-License-Identifier: MIT
# ==============================================================================
"""
Test basic use cases
"""
import unittest
import os
import math
import pyvpl

# Folder this script is in
SCRIPT_PATH = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))

# Folder content is in
CONTENT_PATH = os.path.join(SCRIPT_PATH, '..', '..', 'examples', 'content')
HEVC_CLIP = os.path.join(CONTENT_PATH, 'cars_128x96.h265')
I420_CLIP = os.path.join(CONTENT_PATH, 'cars_128x96.i420')


def roundup(value, base=10):
    """Round value up to next multiple of base"""
    result = math.floor((value + base - 1) / base) * base
    return result


class TestUseCases(unittest.TestCase):
    """
    Test basic use cases
    """
    def test_decode(self):
        """Test Decode"""
        frame_count = 0
        with pyvpl.bitstream_file_reader_name(HEVC_CLIP) as source:
            with open("raw.out", "wb") as sink:
                opts = []
                opts.append(
                    pyvpl.property("mfxImplDescription.Impl",
                                   pyvpl.implementation.software))
                sel_default = pyvpl.default_selector(opts)

                # Load session and initialize decoder
                params = pyvpl.decoder_video_param()
                params.IOPattern = pyvpl.io_pattern.out_system_memory
                params.CodecId = pyvpl.codec_format_fourcc.hevc
                decoder = pyvpl.decode_session(sel_default, params, source)
                # check and report implementation details
                version = decoder.version
                self.assertGreater(
                    version.Major, 1,
                    "Sample requires 2.x API implementation, exiting")
                init_header_list = pyvpl.decoder_init_header_list()
                init_reset_list = pyvpl.decoder_init_reset_list()
                decoder.init_by_header(init_header_list, init_reset_list)

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
        self.assertEqual(frame_count, 60)

    def test_encode(self):
        """Test Encode"""
        frame_count = 0
        with pyvpl.raw_frame_file_reader_by_name(
                128, 96, pyvpl.color_format_fourcc.i420, I420_CLIP) as source:
            with open("out.h265", "wb") as sink:
                opts = []
                opts.append(
                    pyvpl.property("mfxImplDescription.Impl",
                                   pyvpl.implementation.software))
                input_fourcc = pyvpl.color_format_fourcc.i420
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
                info.frame_size = (roundup(128, 16), roundup(96, 16))
                info.FourCC = input_fourcc
                info.ChromaFormat = pyvpl.chroma_format_idc.yuv420
                info.ROI = ((0, 0), (128, 96))

                params.RateControlMethod = pyvpl.rate_control_method.cqp
                params.frame_info = info
                params.CodecId = pyvpl.codec_format_fourcc.hevc
                params.IOPattern = pyvpl.io_pattern.in_system_memory

                # Getting VAAPI handle not supported in Python sample
                init_list = pyvpl.encoder_init_list()
                session.Init(params, init_list)

                # check and report implementation details
                version = session.version
                self.assertGreater(
                    version.Major, 1,
                    "Sample requires 2.x API implementation, exiting")

                for bits in session:
                    # frame_count += 1
                    frame_count += 1
                    sink.write(bits)

        self.assertEqual(frame_count, 60)

    def test_vpp(self):
        """Test VPP"""
        frame_count = 0

        with pyvpl.raw_frame_file_reader_by_name(
                128, 96, pyvpl.color_format_fourcc.i420, I420_CLIP) as source:
            with open("raw.out", "wb") as sink:
                opts = []
                opts.append(
                    pyvpl.property("mfxImplDescription.Impl",
                                   pyvpl.implementation.software))
                sel_default = pyvpl.default_selector(opts)

                # Load session and initialize decoder
                params = pyvpl.vpp_video_param()
                in_frame = pyvpl.frame_info()
                in_frame.FourCC = pyvpl.color_format_fourcc.i420
                in_frame.ChromaFormat = pyvpl.chroma_format_idc.yuv420
                in_frame.PicStruct = pyvpl.pic_struct.progressive
                in_frame.frame_rate = (30, 1)
                in_frame.ROI = ((0, 0), (128, 96))
                in_frame.frame_size = (roundup(128, 16), roundup(96, 16))
                params.in_frame_info = in_frame
                out_frame = pyvpl.frame_info()
                out_frame.FourCC = pyvpl.color_format_fourcc.i420
                out_frame.ChromaFormat = pyvpl.chroma_format_idc.yuv420
                out_frame.PicStruct = pyvpl.pic_struct.progressive
                out_frame.frame_rate = (30, 1)
                out_frame.ROI = ((0, 0), (640, 480))
                out_frame.frame_size = (roundup(640, 16), roundup(480, 16))
                params.out_frame_info = out_frame
                params.IOPattern = pyvpl.io_pattern.io_system_memory
                init_reset_list = pyvpl.vpp_init_reset_list()
                session = pyvpl.vpp_session(sel_default, source)
                session.Init(params, init_reset_list)
                # check and report implementation details
                version = session.version
                self.assertGreater(
                    version.Major, 1,
                    "Sample requires 2.x API implementation, exiting")
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
        self.assertEqual(frame_count, 60)


if __name__ == '__main__':
    unittest.main()
