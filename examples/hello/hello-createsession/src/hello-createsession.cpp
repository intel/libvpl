//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) session creation application
/// showcasing the transition from core API to oneVPL 2.2 API features. For more information see:
/// https://software.intel.com/content/www/us/en/develop/articles/upgrading-from-msdk-to-onevpl.html
/// https://oneapi-src.github.io/oneAPI-spec/elements/oneVPL/source/index.html
///
/// @file

#include "util.h"

void Usage(void) {
    printf("\n");
    printf("   Usage  :  hello-createsession\n");
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
    int accel_fd       = 0;
    Params cliParams;

#ifdef USE_MEDIASDK1
    //Variables used only in legacy version
    mfxVersion version = { 0, 1 };
#endif
    //variables used only in 2.x version
    mfxConfig cfg    = NULL;
    mfxLoader loader = NULL;

    //Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_CREATESESSION) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    // Initialize VPL session
#ifdef USE_MEDIASDK1
    sts = MFXInit(cliParams.impl, &version, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create session with MFXInit");
#else

    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed -- is implementation in path?");

    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "MFXCreateConfig failed")

    sts = MFXSetConfigFilterProperty(cfg, (mfxU8 *)"mfxImplDescription.Impl", cliParams.implValue);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session");
#endif

    // Print info about implementation loaded
    ShowImplInfo(session);

    // Convenience function to initialize available accelerator(s)
    accelHandle = InitAcceleratorHandle(session, &accel_fd);

end:

    MFXClose(session);

    FreeAcceleratorHandle(accelHandle, accel_fd);
    accelHandle = NULL;
    accel_fd    = 0;

#ifndef USE_MEDIASDK1
    if (loader)
        MFXUnload(loader);
#endif

    return 0;
}
