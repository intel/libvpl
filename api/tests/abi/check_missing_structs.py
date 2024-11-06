# /*############################################################################
#   # Copyright (C) 2020 Intel Corporation
#   #
#   # SPDX-License-Identifier: MIT
#   ############################################################################*/
"""
Check for structures which are not present in ABI tests.
"""
# pylint: disable=invalid-name,consider-using-f-string

# To run this script:
# 1. Install clang
# 2. Install clang python binding (pip3 install clang)
# 3. export LD_LIBRARY_PATH=<path to clang SO lib>

import argparse
import os
import glob
import re
import sys
import clang.cindex

ignore_list = [
    'mfxEncryptedData', 'mfxSyncPoint', 'mfxSession', 'mfxLoader', 'mfxConfig'
]

structures = {}


def find_structs(node):
    """Find structures in the headers"""
    if node.kind == clang.cindex.CursorKind.TYPEDEF_DECL:
        sp = node.underlying_typedef_type.spelling
        if sp.startswith(
                "struct"
        ):  # this is trick. Don't know right way to assotiate typedef with referenced node
            structures[node.displayname] = node.displayname
    # Recurse for children of this node
    for c in node.get_children():
        find_structs(c)


parser = argparse.ArgumentParser(
    description=
    'Searches for the structs with missed ABI backward compatibility tests.')
parser.add_argument('-i', '--ifolder', help='Input folder with header files.')
parser.add_argument('-t', '--test_file', help='Input file with ABI tests.')

args = parser.parse_args()

errCount = 0
state = 'search'
for filename in glob.glob(os.path.join(args.ifolder, '*.h')):
    print("Parsing of " + filename)
    index = clang.cindex.Index.create()
    tu = index.parse(filename, ["-DONEVPL_EXPERIMENTAL"])
    find_structs(tu.cursor)

print(structures)
print("Found structures: {}".format(len(structures)))

print("Parsing of test file: " + args.test_file)

with open(args.test_file, 'r', encoding="utf-8") as file:
    data = file.read()
    for s in structures:
        if not re.search(s, data):
            if s not in ignore_list:
                print("struct {} missed".format(s))
                errCount = errCount + 1

if errCount == 0:
    print("Passed")
else:
    print("Failed with {} errors.".format(errCount))

sys.exit(-errCount)
