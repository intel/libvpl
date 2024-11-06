# /*############################################################################
#   # Copyright (C) 2020 Intel Corporation
#   #
#   # SPDX-License-Identifier: MIT
#   ############################################################################*/
"""
Check for expected structure packing.
"""
# pylint: disable=invalid-name,consider-using-f-string

import argparse
import os
import glob
import re
import sys

typedefs_with_ptr_list = [
    'mfxHDL', 'mfxThreadTask', 'mfxEncryptedData', 'mfxSyncPoint',
    'mfxSession', 'mfxLoader', 'mfxConfig'
]


def CheckPtr(p_line):
    """Check pointer"""
    p_newline = re.sub(
        r" *", '', p_line
    )  # trim spaces. can be embedded into regexp below but will meake it more complex for reader
    p_result = re.search(
        r"\S+\*+\S+;",
        p_newline)  # search for the pointer declaration like "xxx*yyy;"
    if p_result:
        return 1
    return 0


def CheckTypedefWithPtr(p_line):
    """Check typedef"""
    for typedef in typedefs_with_ptr_list:
        p_result = re.search(
            typedef, p_line
        )  # search for the field with the type defines through the typedef
        if p_result:
            return 1
    return 0


checkers = [CheckPtr, CheckTypedefWithPtr]

parser = argparse.ArgumentParser(
    description='Check that structures with pointers have correct pragme pack.'
)
parser.add_argument('-i', '--ifolder', help='Input folder with header files.')

args = parser.parse_args()

errCount = 0
state = 'search'
for filename in glob.glob(os.path.join(args.ifolder, '*.h')):
    with open(filename, 'r', encoding="utf-8") as f:
        print("Parsing of " + filename)
        Lines = f.readlines()
        for line in Lines:
            result = re.search(r"^\s*MFX_PACK_BEGIN_USUAL_STRUCT", line)
            if result and state == "search":
                state = 'inmacro'
                continue
            result = re.search(r"^\s*MFX_PACK_END", line)
            if result and state == "inmacro":
                state = 'search'
                continue

            if state == 'inmacro':
                newline = re.sub(
                    r"\/\*.*\*\/", '',
                    line)  # remove /* */ comment within the single line
                newline = re.sub(
                    r"/\*.*", '', newline
                )  # remove begin of the comment when we have multi line comments
                newline = re.sub(
                    r".*\*\/$", '', newline
                )  # remove end of the comment when we have multi line comments
                for checker in checkers:
                    r = checker(newline)
                    if r:
                        print(
                            "Line{}: {}".format(errCount, newline.strip())
                        )  # wrong pack is found. structure with pointers is wrongly packed.
                        errCount += 1

if errCount == 0:
    print("Passed")
else:
    print("Failed with {} errors.".format(errCount))

sys.exit(-errCount)
