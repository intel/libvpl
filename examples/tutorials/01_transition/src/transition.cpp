//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// Minimal session creation application to illustrate the transition from Media
/// SDK to Intel® Video Processing Library (Intel® VPL).  For more information see
/// https://intel.github.io/libvpl
/// @file

#ifdef USE_VPL_INCLUDE
    #include "vpl/mfx.h"
#else
    #include "mfxvideo.h"
#endif
#include <stdio.h>

int main() {
    mfxStatus sts = MFX_ERR_NONE;

#ifdef USE_VPL2X_INIT
    // Create loader with a list of all available implementations
    mfxLoader loader = MFXLoad();

    // Create a config filter so that loader list only contains HW implementation
    mfxConfig cfg = MFXCreateConfig(loader);
    mfxVariant implValue;
    implValue.Type     = MFX_VARIANT_TYPE_U32;
    implValue.Data.U32 = MFX_IMPL_TYPE_HARDWARE;
    MFXSetConfigFilterProperty(cfg, (mfxU8 *)"mfxImplDescription.Impl", implValue);

    printf("Hello from unconstrained Intel® VPL\n");
    mfxSession session = {};
    sts                = MFXCreateSession(loader, 0, &session);
    printf("Intel® VPL 2.x API init MFXCreateSession %s\n",
           (sts == MFX_ERR_NONE) ? "succeeded" : "failed");
#else
    printf("Hello from Intel® Media SDK\n");
    mfxIMPL impl       = MFX_IMPL_HARDWARE;
    mfxVersion ver     = { 0, 1 };
    mfxSession session = {};
    sts                = MFXInit(impl, &ver, &session);
    printf("Media SDK 1.x MFXInit %s\n", (sts == MFX_ERR_NONE) ? "succeeded" : "failed");
#endif

    if (sts == MFX_ERR_NONE) {
        // Print info about implementation loaded
        mfxVersion version = { 0, 0 };
        MFXQueryVersion(session, &version);

        printf("Session loaded: ApiVersion = %d.%d \n", version.Major, version.Minor);

        // Clean up resources
        if (session)
            MFXClose(session);
    }
    else {
        printf("Initialization failed, please check system setup\n");
    }
    return 0;
}
