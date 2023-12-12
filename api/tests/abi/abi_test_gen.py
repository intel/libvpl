# /*############################################################################
#   # Copyright (C) 2020 Intel Corporation
#   #
#   # SPDX-License-Identifier: MIT
#   ############################################################################*/
"""
Generate ABI tests.
"""
# pylint: disable=invalid-name

# To run this script:
# 1. Install clang
# 2. Install clang python binding (pip3 install clang)
# 3. export LD_LIBRARY_PATH=<path to clang SO lib>

import argparse
import os
import glob
import sys
import clang.cindex

ignore_list = [
    'mfxEncryptedData', 'mfxSyncPoint', 'mfxSession', 'mfxLoader', 'mfxConfig'
]

structures = {}


def find_structs(node, mfxStruct):
    """Find structures in the headers"""
    if node.kind == clang.cindex.CursorKind.TYPEDEF_DECL:
        sp = node.underlying_typedef_type.spelling
        if sp.startswith(
                "struct"
        ):  # this is trick. Don't know right way to associate typedef with referenced node
            structures[node.displayname] = node.displayname
        if mfxStruct in sp:
            # print(type(node))
            # print(node.underlying_typedef_type.kind)
            fields = []
            for cc in node.walk_preorder():
                if cc.kind == clang.cindex.CursorKind.FIELD_DECL:
                    if "reserved" not in cc.spelling:
                        fields.append(cc.spelling)

            with open("gen__.c", 'w', encoding="utf-8") as gf:
                original_stdout = sys.stdout
                sys.stdout = gf
                print("#include <stddef.h>")
                print("#include <stdio.h>")
                print("#include <mfx.h>")
                print("int main() {")
                print("printf(\"MSDK_STATIC_ASSERT_STRUCT_SIZE(" + mfxStruct +
                      ", %lu)\\n\", sizeof(" + mfxStruct + "));")
                for f in fields:
                    # MSDK_STATIC_ASSERT_STRUCT_OFFSET(mfxInitializationParam, AccelerationMode  ,0)
                    print(
                        "printf(\"        MSDK_STATIC_ASSERT_STRUCT_OFFSET(" +
                        mfxStruct + ", " + f + ", %lu)\\n\", offsetof(" +
                        mfxStruct + ", " + f + "));")
                print("return 0;}")
                sys.stdout = original_stdout

            print("Done")
            sys.exit(0)
    # Recurse for children of this node
    for c in node.get_children():
        find_structs(c, mfxStruct)


parser = argparse.ArgumentParser(
    description=
    'Searches for the structs with missed ABI backward compatibility tests.')
parser.add_argument('-i', '--ifolder', help='Input folder with header files.')
parser.add_argument('-s', '--structure', help='Name of the structure.')

args = parser.parse_args()

for filename in glob.glob(os.path.join(args.ifolder, '*.h')):
    print("Parsing of " + filename)
    index = clang.cindex.Index.create()
    tu = index.parse(filename, ["-DONEVPL_EXPERIMENTAL"])
    find_structs(tu.cursor, args.structure)

print("Structure definition wasn't located: " + args.structure)
sys.exit(-1)
