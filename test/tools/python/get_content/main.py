############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################
"""Get content for testing"""
import urllib.request
import posixpath
import shutil
import hashlib
import os
import array
import re
from collections import namedtuple
from .util import run_cmd

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
    "H264",
    "H265",
    "MPEG2",
    "MJPEG",
]


def convert_i010_to_p010(src, dest, width, height):
    """Convert I010 encoded file to P010"""
    y_size = width * height
    u_size = y_size // 4
    v_size = u_size
    while True:
        try:
            y_arr = array.array("H")
            u_arr = array.array("H")
            v_arr = array.array("H")
            uv_arr = array.array("H")
            y_arr.fromfile(src, y_size)
            u_arr.fromfile(src, u_size)
            v_arr.fromfile(src, v_size)
            uv_list = [u for v in zip(u_arr, v_arr) for u in v]
            uv_arr.fromlist(uv_list)
            y_arr.tofile(dest)
            uv_arr.tofile(dest)
        except EOFError:
            break


ContentSize = namedtuple('ContentSize',
                         ['width', 'height', 'framerate', 'interlaced'])
ContentDesc = namedtuple(
    'ContentDesc',
    ['mode', 'filename', 'codec', 'size', 'bitdepth', 'opts', 'kwopts'])

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
    match = SIZE_RE.match(text)
    if not match:
        return None
    width = int(match.group('width'))
    height = int(match.group('height'))
    if 'rate' in match.groupdict():
        framerate = float(match.group('rate'))
    interlaced = match.group('ip') == 'i'
    return ContentSize(width, height, framerate, interlaced)


BITDEPTH_RE = re.compile(r'^(?P<bits>[0-9]+)b$', re.VERBOSE)


def parse_bitdepth(text):
    """Parse a bitdepth string to a structured type"""
    match = BITDEPTH_RE.match(text)
    if not match:
        return None
    bits = int(match.group('bits'))
    return bits


AutoParams = namedtuple('AutoParams', ['args', 'kwargs', 'size', 'bitdepth'])


def parse_auto_query(terms):
    """Parse the query portion of an auto-generation uri"""
    args = []
    kwargs = {}
    size = None
    bitdepth = None
    if terms:
        terms = terms.split('&')
        for term in terms:
            value = parse_size(term)
            if value is not None:
                size = value
                continue
            value = parse_bitdepth(term)
            if value is not None:
                bitdepth = value
                continue
            if '=' in term:
                name, value = term.split('=', 1)
                value = urllib.parse.unquote(value)
                kwargs[name] = value
            else:
                args = urllib.parse.unquote(term)
    if size is None:
        size = ContentSize(128, 96, 60 / 1.001, False)
    if bitdepth is None:
        bitdepth = 8
    return AutoParams(args, kwargs, size, bitdepth)


def parse_content_uri(source):
    """Parse a uri that may be a url or an auto-generation configuration"""
    if source is None:
        return None
    mode, desc = source.split(':', 1)
    mode = mode.lower()
    if mode == 'file':
        return ContentDesc('file', desc, None, None, None)
    if mode == 'auto':
        if '?' in desc:
            codec, terms = desc.split('?', 1)
        else:
            codec = desc
            terms = None
        params = parse_auto_query(terms)
        return ContentDesc('auto', None, codec, params.size, params.bitdepth,
                           params.args, params.kwargs)
    return None


FileInfo = namedtuple('FileInfo', ['name', 'codec'])


def optional_convert_i010_to_p010(config, idest, dest):
    """convert i010 to p010, but only if needed"""
    if dest.codec == "P010":
        if idest.codec == "I010":
            with open(dest.name, "wb") as p010_file:
                with open(idest.name, "rb") as i010_file:
                    convert_i010_to_p010(i010_file, p010_file,
                                         config.size.width, config.size.height)


def set_default_opts(config):
    """Most codecs use standard encoding options,
    this will set those options"""
    cmd = []
    if 'bitrate' in config.kwopts:
        bitrate = int(config.kwopts['bitrate'])
        cmd.extend(['-b:v', str(bitrate) + 'k'])
    if 'gop' in config.kwopts:
        gop = int(config.kwopts['gop'])
        cmd.extend(['-g', str(gop)])
    if 'kfdist' in config.kwopts:
        key_frame_distance = int(config.kwopts['kfdist'])
        cmd.extend(['-keyint_min', str(key_frame_distance)])
    return cmd


def set_jpeg_opts(config):
    """MJPEG does not use the standard set of encoding
    options, this will set the JPEG specific encoding
    options instead"""
    if 'quality' in config.kwopts:
        quality = int(config.kwopts['quality'])
        # convert scale from 1-100 (VPL, worst to best) to
        #   2-31 (ffmpeg, best to worst)
        quality = (int(quality) - 1) * (31 - 2)
        quality = 31 - int(quality / (100 - 1))
        quality = max(2, min(quality, 31))
        return ["-q:v", str(quality)]
    return []


def genterate_content(uri, dest_folder):
    """create content"""
    # the following has is used to ensure uniqueness
    unique = hashlib.sha1(uri.encode('utf-8')).hexdigest()

    config = parse_content_uri(uri)
    if 'frames' in config.kwopts:
        frames = int(config.kwopts['frames'])
    else:
        frames = 50

    if dest_folder is None:
        dest_folder = ''
    dest = FileInfo(
        os.path.join(
            dest_folder, "auto_" + str(config.size.width) + 'x' +
            str(config.size.height) + '_' + unique + '.' + config.codec),
        config.codec)
    if os.path.exists(dest.name):
        return dest.name

    idest = FileInfo(dest.name, dest.codec.upper())
    if dest.codec and dest.codec == "P010":
        idest.name = dest.name + ".i010"
        idest.codec = "I010"

    cmd = [
        'ffmpeg', '-y', '-f', 'lavfi', '-i',
        'testsrc2=size={w}x{h}:rate=30:duration={d}'.format(
            w=config.size.width,
            h=config.size.height,
            d=frames / config.size.framerate)
    ]

    if idest.codec in ENCODED:
        cmd.extend(['-c', CODEC_MAP[idest.codec]])
        if CODEC_MAP[idest.codec] == 'mpeg2video':
            cmd.extend(['-f', 'mpeg2video'])
        if CODEC_MAP[idest.codec] == "mjpeg":
            cmd.extend(set_jpeg_opts(config))
        else:
            cmd.extend(set_default_opts(config))
    else:
        cmd.extend([
            '-c', 'rawvideo', '-f', 'rawvideo', '-pix_fmt',
            CODEC_MAP[idest.codec]
        ])
    cmd.extend([idest.name])
    returncode, _, _ = run_cmd(cmd)
    if returncode != 0:
        raise Exception(
            "Generation failed with code: {0} ({0:#0x})".format(returncode))
    optional_convert_i010_to_p010(config, idest, dest)
    return dest.name


def download_content(url, dest_folder):
    """Download content from a URL"""
    if dest_folder is None:
        dest_folder = ''
    fields = urllib.parse.urlparse(url)
    # the following has is used to ensure uniqueness in case the same filename
    # is pulled from multiple sources.
    unique = hashlib.sha1(url.encode('utf-8')).hexdigest()
    name, ext = posixpath.splitext(posixpath.basename(fields.path))
    dest_path = os.path.join(dest_folder, name + "_" + unique + ext)
    if not os.path.exists(dest_path):
        with urllib.request.urlopen(url) as response, open(dest_path,
                                                           'wb') as dest_file:
            shutil.copyfileobj(response, dest_file)
    return dest_path


def get_content(uri, dest_folder=None):
    """Automation entrypoint"""
    if uri.startswith('auto:'):
        location = genterate_content(uri, dest_folder)
    else:
        location = download_content(uri, dest_folder)
    return location
