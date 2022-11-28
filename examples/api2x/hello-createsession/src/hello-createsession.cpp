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

#define MAJOR_API_VERSION_REQUIRED 2
#define MINOR_API_VERSION_REQUIRED 2

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
    Params cliParams;
    bool isFailed = false;

    //variables used only in 2.x version
    mfxConfig cfg[3];
    mfxVariant cfgVal[3];
    mfxLoader loader = NULL;

    //Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_CREATESESSION) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    // Initialize VPL session
    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed -- is implementation in path?");

    // Implementation used must be the type requested from command line
    cfg[0] = MFXCreateConfig(loader);
    VERIFY(NULL != cfg[0], "MFXCreateConfig failed")

    sts =
        MFXSetConfigFilterProperty(cfg[0], (mfxU8 *)"mfxImplDescription.Impl", cliParams.implValue);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    // Implementation used must provide API version 2.2 or newer
    cfg[2] = MFXCreateConfig(loader);
    VERIFY(NULL != cfg[2], "MFXCreateConfig failed")
    cfgVal[2].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[2].Data.U32 = VPLVERSION(MAJOR_API_VERSION_REQUIRED, MINOR_API_VERSION_REQUIRED);
    sts                = MFXSetConfigFilterProperty(cfg[2],
                                     (mfxU8 *)"mfxImplDescription.ApiVersion.Version",
                                     cfgVal[2]);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for API version");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts,
           "Cannot create session -- no implementations meet selection criteria");

    // Print info about implementation loaded
    ShowImplementationInfo(loader, 0);

end:

    MFXClose(session);

    if (isFailed) {
        return -1;
    }
    else {
        return 0;
    }
}
