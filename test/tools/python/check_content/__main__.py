############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################
"""Check content against reference"""
import argparse
from .main import check_content


def main():
    """Main entrypoint"""
    parser = argparse.ArgumentParser(
        description=globals()['__doc__'],
        formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('ref', action='store')
    parser.add_argument('proc', action='store')
    parser.add_argument('--frames', '-n', action='store', type=int, default=50)
    parser.add_argument('--format', '-f', action='store', default=None)
    parser.add_argument('--size', '-s', action='store', default=None)
    parser.add_argument('--algorithm',
                        '-a',
                        default='ssim',
                        metavar='ALGORITHM',
                        action='store',
                        choices=['ssim', 'psnr', 'bit-exact'])
    parser.add_argument('--threshold',
                        '-t',
                        metavar='THRESHOLD',
                        action='store')
    args = parser.parse_args()
    is_pass, value = check_content(args.ref, args.proc, args.format,
                                   args.algorithm, args.size, args.threshold,
                                   args.frames)
    print("Got: {}".format(value))
    if is_pass:
        print("Pass")
    else:
        print("Fail")
    return 0 if is_pass else 1


if __name__ == "__main__":
    main()
