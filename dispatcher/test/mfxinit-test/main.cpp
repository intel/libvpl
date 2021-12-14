/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <stdio.h>

#include "vpl/mfx.h"

static const unsigned int implType[] = {
    MFX_IMPL_HARDWARE,
    MFX_IMPL_HARDWARE2,
    MFX_IMPL_HARDWARE3,
    MFX_IMPL_HARDWARE4,
};

static const char *implName[] = {
    "MFX_IMPL_HARDWARE",
    "MFX_IMPL_HARDWARE2",
    "MFX_IMPL_HARDWARE3",
    "MFX_IMPL_HARDWARE4",
};

static void TestCreateEncoder(mfxSession session) {
    // try to create basic encoder
    mfxVideoParam par = {};

    par.mfx.CodecId                  = MFX_CODEC_AVC;
    par.mfx.TargetUsage              = MFX_TARGETUSAGE_BALANCED;
    par.mfx.TargetKbps               = 1000;
    par.mfx.RateControlMethod        = MFX_RATECONTROL_VBR;
    par.mfx.FrameInfo.FrameRateExtN  = 30;
    par.mfx.FrameInfo.FrameRateExtD  = 1;
    par.mfx.FrameInfo.FourCC         = MFX_FOURCC_NV12;
    par.mfx.FrameInfo.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
    par.mfx.FrameInfo.Width          = 1280;
    par.mfx.FrameInfo.Height         = 720;
    par.mfx.FrameInfo.CropX          = 0;
    par.mfx.FrameInfo.CropY          = 0;
    par.mfx.FrameInfo.CropW          = par.mfx.FrameInfo.Width;
    par.mfx.FrameInfo.CropH          = par.mfx.FrameInfo.Height;
    par.mfx.GopRefDist               = 4;
    par.mfx.GopPicSize               = 0;
    par.mfx.FrameInfo.BitDepthLuma   = 8;
    par.mfx.FrameInfo.BitDepthChroma = 8;
    par.mfx.FrameInfo.PicStruct      = 1;

    par.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

#if defined(_WIN32) || defined(_WIN64)
    // get device ID and adapter type (req. API >= 1.19)
    mfxStatus sts = MFXVideoENCODE_Init(session, &par);
    if (sts != MFX_ERR_NONE) {
        printf("  Error - MFXVideoENCODE_Init returned %d\n", sts);
        return;
    }
    printf("  MFXVideoENCODE_Init succeeded\n");

    MFXVideoENCODE_Close(session);
#else
    printf(
        "  Warning - skipping test for MFXVideoENCODE_Init (external accelerator handle required)\n");
#endif
}

static void PrintSessionInfo(mfxSession session) {
    mfxStatus sts = MFX_ERR_NONE;

    // get actual API version
    mfxVersion actualVersion = {};
    sts                      = MFXQueryVersion(session, &actualVersion);
    if (sts == MFX_ERR_NONE)
        printf("  actualVersion    = %d.%d\n", actualVersion.Major, actualVersion.Minor);
    else
        printf("  MFXQueryVersion failed with code %d\n", sts);

#if defined(_WIN32) || defined(_WIN64)
    // get device ID and adapter type (req. API >= 1.19)
    // Linux requires creating accelerator handle
    mfxPlatform platform = {};
    sts                  = MFXVideoCORE_QueryPlatform(session, &platform);
    if (sts == MFX_ERR_NONE) {
        printf("  DeviceId         = 0x%x\n", platform.DeviceId);
        printf("  MediaAdapterType = %d\n", platform.MediaAdapterType);
    }
    else {
        printf("  MFXVideoCORE_QueryPlatform failed with code %d\n", sts);
    }
#endif
}

static mfxStatus MFXInit_GetSessionVersion(int idx) {
    mfxStatus sts = MFX_ERR_NONE;

    mfxSession session = nullptr;

    mfxVersion requiredVer = {};
    requiredVer.Major      = 1;
    requiredVer.Minor      = 0;

    printf("Calling MFXInit - %s... ", implName[idx]);
    sts = MFXInit(implType[idx], &requiredVer, &session);

    if (sts == MFX_ERR_NONE) {
        printf("  success!\n");

        PrintSessionInfo(session);
        TestCreateEncoder(session);

        MFXClose(session);
    }
    else {
        printf("  failed!\n");
    }
    printf("\n");

    return sts;
}

static mfxStatus MFXInitEx_GetSessionVersion(int idx) {
    mfxStatus sts = MFX_ERR_NONE;

    mfxSession session = nullptr;

    mfxInitParam initPar   = { 0 };
    initPar.Version.Major  = 1;
    initPar.Version.Minor  = 0;
    initPar.Implementation = implType[idx];

    printf("Calling MFXInitEx - %s... ", implName[idx]);
    sts = MFXInitEx(initPar, &session);

    if (sts == MFX_ERR_NONE) {
        printf("  success!\n");

        PrintSessionInfo(session);
        TestCreateEncoder(session);

        MFXClose(session);
    }
    else {
        printf("  failed!\n");
    }
    printf("\n");

    return sts;
}

int main(int argc, char **argv) {
    printf("\n*** MFXInit - get session version ***\n\n");

    MFXInit_GetSessionVersion(0);
    MFXInit_GetSessionVersion(1);
    MFXInit_GetSessionVersion(2);
    MFXInit_GetSessionVersion(3);

    printf("\n*** MFXInitEx - get session version ***\n\n");
    MFXInitEx_GetSessionVersion(0);
    MFXInitEx_GetSessionVersion(1);
    MFXInitEx_GetSessionVersion(2);
    MFXInitEx_GetSessionVersion(3);

    return 0;
}