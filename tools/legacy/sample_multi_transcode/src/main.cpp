/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "sample_multi_transcode.h"

int main(int argc, char* argv[]) {
    mfxStatus sts;
    TranscodingSample::Launcher transcode;
    if (argc < 2) {
        printf("[ERROR] Command line is empty. Use -? for getting help on available options.\n");
        return 0;
    }

    sts = transcode.Init(argc, argv);
    if (sts == MFX_WRN_OUT_OF_RANGE) {
        // There's no error in parameters parsing, but we should not continue further. For instance, in case of -? option
        return MFX_ERR_NONE;
    }

    fflush(stdout);
    fflush(stderr);

    MSDK_CHECK_STATUS(sts, "transcode.Init failed");

    transcode.Run();

    sts = transcode.ProcessResult();
    fflush(stdout);
    fflush(stderr);
    MSDK_CHECK_STATUS(sts, "transcode.ProcessResult failed");

    return 0;
}
