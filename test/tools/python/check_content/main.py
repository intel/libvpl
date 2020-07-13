############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################
"""Get content for testing"""
import os
import math
import array
import re
import copy
import filecmp
import sys
from collections import namedtuple
from .util import run_cmd

try:
    from robot.api import logger

    def info(msg, html=False, also_console=False):
        """Log info level messages"""
        logger.info(msg, html, also_console=also_console)
except ImportError:

    def info(msg, html=False, also_console=False):
        """Log info level messages"""
        del html
        del also_console
        print(msg, file=sys.stderr)


TOOLPATH = os.getenv('VPL_BUILD_DEPENDENCIES')
if TOOLPATH is not None:
    TOOLPATH += "/bin"
    print(TOOLPATH)

CODEC_MAP = {
    "H264": "h264",
    "H265": "hevc",
    "MPEG2": "mpeg2video",
    "MJPEG": "mjpeg",
    "BGRA": "bgra",
    "I420": "yuv420p",
    "NV12": "nv12",
    "I010": "yuv420p10le",
    "P010": "",
}

ENCODED = [
    None,
    "H264",
    "H265",
    "MPEG2",
    "MJPEG",
]

ContentSize = namedtuple('ContentSize',
                         ['width', 'height', 'framerate', 'interlaced'])


def convert_p010_to_i010(src, dest, width, height):
    """Convert P010 encoded file to I010"""
    y_size = width * height
    uv_size = y_size // 2
    while True:
        try:
            y_arr = array.array("H")
            u_arr = array.array("H")
            v_arr = array.array("H")
            uv_arr = array.array("H")
            y_arr.fromfile(src, y_size)
            uv_arr.fromfile(src, uv_size)
            u_arr = uv_arr[::2]
            v_arr = uv_arr[1::2]
            y_arr.tofile(dest)
            u_arr.tofile(dest)
            v_arr.tofile(dest)
        except EOFError:
            break


def _read_psnr_values(content):
    """Parse FFMpeg output to find PSNR statistics"""
    result = {}
    stats_re = re.compile(r"^\[.*\] PSNR (?P<stats>.*)$")
    for line in content.splitlines():
        match = stats_re.match(line)
        if match:
            for stat in match.group("stats").split():
                name, value = stat.split(":", 1)
                result[name] = float(value)
    return result


def _read_ssim_values(content):
    """Parse FFMpeg output to find SSIM statistics"""
    result = {}
    stats_re = re.compile(r"^\[.*\] SSIM (?P<stats>.*)$")
    for line in content.splitlines():
        match = stats_re.match(line)
        if match:
            for stat in match.group("stats").split():
                if stat.startswith('('):
                    continue
                name, value = stat.split(":", 1)
                result[name] = float(value.split()[0])
    return result


def fail_on_empty_file(name):
    """Raise an exception if a given file is empty"""
    size = os.path.getsize(name)
    if size == 0:
        raise Exception("{0} is Empty.".format(name))


def munge_format(src, codec):
    """Modify format of a file to one compatible with ffmpeg"""
    new_src = src
    new_codec = codec
    if codec == "P010":
        new_src = copy.copy(src)
        new_src.filename = src.filename + ".i010"
        new_codec = "I010"
        with open(new_src.filename, "wb") as destf:
            with open(src.filename, "rb") as srcf:
                convert_p010_to_i010(srcf, destf, src.width, src.height)
    fail_on_empty_file(new_src)
    return new_src, new_codec


# pylint: disable=too-many-locals
def psnr(ref, proc, codec, size, frames):
    """Compute PSNR of raw inputs"""
    isize = parse_size(size)
    fail_on_empty_file(ref)
    fail_on_empty_file(proc)
    iref, icodec = munge_format(ref, codec)
    iproc, _ = munge_format(proc, codec)
    if (icodec not in ENCODED) and (not isize):
        raise ValueError("Raw formats require a size setting")

    if TOOLPATH is not None:
        cmd = ([str(TOOLPATH) + '/' + 'ffmpeg'])
    else:
        cmd = (['ffmpeg'])

    cmd.extend(['-y'])

    config = []
    if isize and isize.framerate:
        config.extend(['-r', str(isize.framerate)])
    if icodec not in ENCODED:
        config.extend(['-s', str(isize.width) + "x" + str(isize.height)])
        config.extend(['-pix_fmt', CODEC_MAP[icodec]])
        config.extend(['-f', "rawvideo"])
    cmd.extend(config)
    cmd.extend(['-i', iproc])
    cmd.extend(config)
    cmd.extend(['-i', iref])
    cmd.extend(['-frames:v', str(frames)])
    cmd.extend(["-filter_complex", "psnr=", "-f", "null", "nullsink"])
    returncode, _, output = run_cmd(cmd)
    if returncode != 0:
        raise Exception(
            "PSNR calculation  failed with code: {0} ({0:#0x})".format(
                returncode))
    values = _read_psnr_values(output)
    info("metric: value")
    for name in values:
        info("{0}: {1}".format(name, values[name]))
    if "average" in values:
        return values["average"]
    raise Exception("Could not find PSNR 'average' in report")


# pylint: disable=too-many-locals
def ssim(ref, proc, codec, size, frames):
    """Compute SSIM of raw inputs"""
    isize = parse_size(size)
    fail_on_empty_file(ref)
    fail_on_empty_file(proc)
    iref, icodec = munge_format(ref, codec)
    iproc, _ = munge_format(proc, codec)
    if (icodec not in ENCODED) and (not isize):
        raise ValueError("Raw formats require a size setting")
    cmd = ['ffmpeg', '-y']
    config = []
    if isize and isize.framerate:
        config.extend(['-r', str(isize.framerate)])
    if icodec not in ENCODED:
        config.extend(['-s', str(isize.width) + "x" + str(isize.height)])
        config.extend(['-pix_fmt', CODEC_MAP[icodec]])
        config.extend(['-f', "rawvideo"])
    cmd.extend(config)
    cmd.extend(['-i', iproc])
    cmd.extend(config)
    cmd.extend(['-i', iref])
    cmd.extend(['-frames:v', str(frames)])
    cmd.extend(["-filter_complex", "ssim=", "-f", "null", "nullsink"])
    returncode, _, output = run_cmd(cmd)
    if returncode != 0:
        raise Exception(
            "SSIM calculation  failed with code: {0} ({0:#0x})".format(
                returncode))
    values = _read_ssim_values(output)
    info("metric: value")
    for name in values:
        info("{0}: {1}".format(name, values[name]))
    if "All" in values:
        return values["All"]
    raise Exception("Could not find SSIM 'All' in report")


SIZE_RE = re.compile(
    r'''^
                        (?P<width>[0-9]+)x(?P<height>[0-9]+)
                        (
                            @(?P<rate>[0-9]+(\.[0-9]+)?)
                        )?
                        (?P<ip>
                        [ip]?
                        )
                        $''', re.VERBOSE)


def parse_size(text):
    """Parse a size string to a structured type"""
    if not text:
        return None
    match = SIZE_RE.match(text)
    if not match:
        return None
    width = int(match.group('width'))
    height = int(match.group('height'))
    if 'rate' in match.groupdict():
        framerate = float(match.group('rate'))
    interlaced = match.group('ip') == 'i'
    return ContentSize(width, height, framerate, interlaced)


# pylint: disable=too-many-arguments
def check_content(ref,
                  proc,
                  codec,
                  algorithm,
                  size=None,
                  threshold=None,
                  frames=50):
    """check content"""
    # the following has is used to ensure uniqueness
    is_pass = False
    value = 0
    if algorithm == 'psnr':
        value = psnr(ref, proc, codec, size, frames)
        threshold_value = float(threshold) if threshold is not None else 30.0
        is_pass = math.isinf(value) or (value >= threshold_value)
    elif algorithm == 'ssim':
        value = ssim(ref, proc, codec, size, frames)
        threshold_value = float(threshold) if threshold is not None else 0.95
        is_pass = math.isinf(value) or (value >= threshold_value)
    elif algorithm == 'bit-exact':
        is_pass = filecmp.cmp(ref, proc)
        value = math.inf if is_pass else 0
    return is_pass, value
