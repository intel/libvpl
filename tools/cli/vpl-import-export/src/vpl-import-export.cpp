//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./util.h"

int main(int argc, char **argv) {
    int err           = -1;
    Params params     = {};
    FileInfo fileInfo = {};

    //-- Parse command line args to params
    if (ParseArgsAndValidate(argc, argv, &params) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    //-- Open input file
    if (!params.infileName.empty()) {
        fileInfo.infile.open(params.infileName.c_str(), std::ios::binary);
        VERIFY(fileInfo.infile, "ERROR: could not open input file");
    }

    //-- Open output file
    if (!params.outfileName.empty()) {
        fileInfo.outfile.open(params.outfileName.c_str(), std::ios::binary);
        VERIFY(fileInfo.outfile, "ERROR: could not open output file");
    }

    if (params.testMode == TEST_MODE_DECVPP_FILE || params.testMode == TEST_MODE_RENDER)
        err = RunDecodeVPP(&params, &fileInfo);
    else if (params.testMode == TEST_MODE_ENC_FILE || params.testMode == TEST_MODE_CAPTURE)
        err = RunEncode(&params, &fileInfo);

    // print report of hardware resources still in use (if supported, DEBUG builds only)
    DebugDumpHardwareInterface();

    return err;
}
