# /*############################################################################
#   # Copyright (C) 2020 Intel Corporation
#   #
#   # SPDX-License-Identifier: MIT
#   ############################################################################*/

import argparse
import os
import glob
import re

typedefs_with_ptr_list = ['mfxHDL', 'mfxThreadTask', 'mfxEncryptedData', 'mfxSyncPoint', 'mfxSession', 'mfxLoader', 'mfxConfig']

def CheckPtr(line):
   newline = re.sub(r" *", '', line)      # trim spaces. can be embedded into regexp below but will meake it more complex for reader
   result = re.search(r"\S+\*+\S+;",newline) # search for the pointer declaration like "xxx*yyy;"
   if result:
      return 1
   return 0

def CheckTypedefWithPtr(line):
   for typedef in typedefs_with_ptr_list:
      result = re.search(typedef,newline) # search for the field with the type defines through the typedef
      if result:
         return 1
   return 0

checkers = [CheckPtr, CheckTypedefWithPtr]

parser = argparse.ArgumentParser(description='Check that structures with pointers have correct pragme pack.')
parser.add_argument('-i', '--ifolder', help='Input folder with header files.')

args = parser.parse_args()

errCount = 0
state = 'search'
for filename in glob.glob(os.path.join(args.ifolder,'*.h')):
   with open(filename, 'r') as f: 
      print ("Parsing of " + filename)
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
            newline = re.sub(r"\/\*.*\*\/", '', line) # remove /* */ comment within the single line
            newline = re.sub(r"/\*.*", '', newline)   # remove begin of the comment when we have multi line comments
            newline = re.sub(r".*\*\/$", '', newline) # remove end of the comment when we have multi line comments
            for checker in checkers:
               r = checker(newline)
               if(r):
                  print("Line{}: {}".format(errCount, newline.strip())) # wrong pack of the structure is found. structure with pointers is wrongly packed.
                  errCount += 1

if errCount == 0:
    print("Passed")
else:
    print("Failed with {} errors.".format(errCount))

exit(-errCount)
