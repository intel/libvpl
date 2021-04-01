//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) session creation application,
/// using 2.x API
///
/// @file

#include "util/util.h"

void Usage(void) {
    printf("\n");
    printf("   Usage  :  legacy-createsession\n");
    printf("     -hw        use hardware implementation\n");
    printf("     -sw        use software implementation\n");
    printf("   Initializes oneVPL session\n\n");
    return;
}

int main(int argc, char *argv[]) {
    //Variables used for legacy and 2.x
    mfxSession session = NULL;
    mfxStatus sts      = MFX_ERR_NONE;
    void *accelHandle  = NULL;
    Params cliParams;

    //variables used only in 2.x version
    mfxConfig cfg    = NULL;
    mfxLoader loader = NULL;

    //Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_CREATESESSION) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    // Initialize VPL session
    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed -- is implementation in path?");

    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "MFXCreateConfig failed")

    sts = MFXSetConfigFilterProperty(cfg, (mfxU8 *)"mfxImplDescription.Impl", cliParams.implValue);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session");

    // Print info about implementation loaded
    ShowImplInfo(session);

    // Convenience function to initialize available accelerator(s)
    accelHandle = InitAcceleratorHandle(session);

end:

    MFXClose(session);

    if (accelHandle)
        FreeAcceleratorHandle(accelHandle);

    if (loader)
        MFXUnload(loader);

    return 0;
}
