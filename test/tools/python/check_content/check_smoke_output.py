############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
############################################################################
"""Check smoke test outputs"""

import sys
import check_content

PSNR_AVG_THRESHOLD = 35.0

if __name__ == '__main__':
    if len(sys.argv) != 5:
        print(
            "Error - usage: get_psnr ref_file test_file codec, size (WxH@FR)")
        sys.exit(-1)

    [IS_PASS,
     PSNR_AVG] = check_content.check_content(sys.argv[1], sys.argv[2],
                                             sys.argv[3], "psnr", sys.argv[4],
                                             PSNR_AVG_THRESHOLD)

    print("Average PSNR = ", PSNR_AVG)
    print("Minimum PSNR = ", PSNR_AVG_THRESHOLD)

    if str(PSNR_AVG) == 'inf':
        print("Result: PASS (exact match)")
        sys.exit(0)
    elif float(PSNR_AVG) > PSNR_AVG_THRESHOLD:
        print("Result: PASS")
        sys.exit(0)
    else:
        print("Result: FAIL")
        sys.exit(-1)
