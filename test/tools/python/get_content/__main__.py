############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################
"""Get content for testing
URI may either be a normal url or an "auto" uri describing
the desired content of the form: auto:CODEC(?OPTION(&OPTION)*)?

OPTION may be:
size: WIDTHxHEIGHT(@RATE)(i|p)?
bitdepth: [0-9]b
NAME=VALUE: names suppored vary by codec, but include:
  * frames: Number of frames requested
  * quality: Quality (as an integer from 1-100)
  * bitrate: Bitrate in kbps
  * gop: GOP size
  * kfdist: Distance between key frames

"""
import argparse
from .main import get_content


def main():
    """Main entrypoint"""
    parser = argparse.ArgumentParser(
        description=globals()['__doc__'],
        formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('uri', action='store')
    parser.add_argument('-o', dest='folder', action='store')
    args = parser.parse_args()

    location = get_content(args.uri, args.folder)
    if location:
        print(location)
        return 0
    print("Could not get content for {}".format(location))
    return 1


if __name__ == "__main__":
    main()
