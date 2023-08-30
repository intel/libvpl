/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./vpl-timing.h" //NOLINT(build/include)

#if defined(_WIN32) || defined(_WIN64)
    #include <Windows.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "vpl/mfx.h"

struct mfxDispatcherVersion {
    mfxU16 Major; // match API version
    mfxU16 Minor; // match API version
    mfxU16 Patch; // increment with dispatcher implementation updates
    mfxU16 Tweak; // optional
};

static mfxStatus GetDispatcherVersion(mfxDispatcherVersion *dispatcherVersion);

static void SetDefaultParamsEncode(mfxVideoParam *par) {
    par->mfx.CodecId                  = MFX_CODEC_AVC;
    par->mfx.TargetUsage              = MFX_TARGETUSAGE_BALANCED;
    par->mfx.TargetKbps               = 1000;
    par->mfx.RateControlMethod        = MFX_RATECONTROL_VBR;
    par->mfx.FrameInfo.FrameRateExtN  = 30;
    par->mfx.FrameInfo.FrameRateExtD  = 1;
    par->mfx.FrameInfo.FourCC         = MFX_FOURCC_NV12;
    par->mfx.FrameInfo.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
    par->mfx.FrameInfo.Width          = 1280;
    par->mfx.FrameInfo.Height         = 720;
    par->mfx.FrameInfo.CropX          = 0;
    par->mfx.FrameInfo.CropY          = 0;
    par->mfx.FrameInfo.CropW          = par->mfx.FrameInfo.Width;
    par->mfx.FrameInfo.CropH          = par->mfx.FrameInfo.Height;
    par->mfx.GopRefDist               = 4;
    par->mfx.GopPicSize               = 0;
    par->mfx.FrameInfo.BitDepthLuma   = 8;
    par->mfx.FrameInfo.BitDepthChroma = 8;
    par->mfx.FrameInfo.PicStruct      = 1;

    par->IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
}

int main(int argc, char *argv[]) {
    mfxSession session = nullptr;
    mfxStatus sts      = MFX_ERR_NONE;
    mfxU32 adapterNum  = 0;

    bool bEnumImpls     = false;
    bool bUseFastLoad   = false;
    bool bPrintImplPath = false;

    for (int i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "-e", 2)) {
            bEnumImpls = true;
        }
        else if (!strncmp(argv[i], "-f", 2)) {
            bUseFastLoad = true;
        }
        else if (!strncmp(argv[i], "-p", 2)) {
            bPrintImplPath = true;
        }
        else if (!strncmp(argv[i], "-adapterNum", 11)) {
            i++;
            adapterNum = atol(argv[i]);
        }
        else {
            printf("Error - invalid argument\n\n");
            printf("Usage: vpl-timing [options]\n");
            printf("       -e ................ enable EnumImplementations (description)\n");
            printf("       -f ................ enable fast loading\n");
            printf("       -p ................ print paths of loaded implementation\n");
            printf("       -adapterNum n ..... use device adapter number n (default = 0)\n");
            return -1;
        }
    }

    VPL_LOG_TIME_START(totaltime, "Total time");

    VPL_LOG_TIME_START(mfxload, "MFXLoad");

    mfxLoader loader = nullptr;
    loader           = MFXLoad();
    if (loader == NULL) {
        printf("Error - loader is null - no libraries found\n");
        return -1;
    }

    VPL_LOG_TIME_END(mfxload);

    VPL_LOG_TIME_START(dispversion, "GetDispatcherVersion");
    mfxDispatcherVersion dispatcherVersion = {};

    sts = GetDispatcherVersion(&dispatcherVersion);
    if (sts == MFX_ERR_NONE) {
        printf("  Dispatcher version: %d.%d.%d.%d\n",
               dispatcherVersion.Major,
               dispatcherVersion.Minor,
               dispatcherVersion.Patch,
               dispatcherVersion.Tweak);
    }
    else {
        printf("  Warning - dispatcher version not detected\n");
    }
    VPL_LOG_TIME_END(dispversion);

    if (bUseFastLoad) {
        VPL_LOG_TIME_START(setprops, "MFXSetConfig (enable fast loading)");

        mfxConfig config = MFXCreateConfig(loader);

        mfxVariant var      = {};
        var.Version.Version = MFX_VARIANT_VERSION;

        var.Type     = MFX_VARIANT_TYPE_U32;
        var.Data.U32 = MFX_IMPL_TYPE_HARDWARE;
        MFXSetConfigFilterProperty(config, (const mfxU8 *)"mfxImplDescription.Impl", var);

        var.Type     = MFX_VARIANT_TYPE_PTR;
        var.Data.Ptr = (mfxHDL) "mfx-gen";
        MFXSetConfigFilterProperty(config, (const mfxU8 *)"mfxImplDescription.ImplName", var);

        var.Type     = MFX_VARIANT_TYPE_U32;
        var.Data.U32 = 0x8086;
        MFXSetConfigFilterProperty(config, (const mfxU8 *)"mfxImplDescription.VendorID", var);

        var.Type = MFX_VARIANT_TYPE_U32;
#if defined(_WIN32) || defined(_WIN64)
        var.Data.U32 = MFX_ACCEL_MODE_VIA_D3D11;
#else
        var.Data.U32 = MFX_ACCEL_MODE_VIA_VAAPI;
#endif
        MFXSetConfigFilterProperty(config,
                                   (const mfxU8 *)"mfxImplDescription.AccelerationMode",
                                   var);

        // set which minimum version is required
        mfxVersion ver = {};
        ver.Major      = 1;
        ver.Minor      = 0;

        var.Type     = MFX_VARIANT_TYPE_U32;
        var.Data.U32 = ver.Version;
        sts          = MFXSetConfigFilterProperty(config,
                                         (const mfxU8 *)"mfxImplDescription.ApiVersion.Version",
                                         var);

        if (adapterNum > 0) {
#if defined(_WIN32) || defined(_WIN64)
            printf("Using adapterNum = %d\n", adapterNum);

            var.Type     = MFX_VARIANT_TYPE_U32;
            var.Data.U32 = adapterNum;
            sts = MFXSetConfigFilterProperty(config, (const mfxU8 *)"DXGIAdapterIndex", var);
#else
            printf("adapterNum ignored\n");
#endif
        }

        VPL_LOG_TIME_END(setprops);
    }

    if (bEnumImpls) {
        mfxU32 idx = 0;
        while (1) {
            mfxImplDescription *idesc = nullptr;

            char logStr[1024] = {};
            snprintf(logStr, sizeof(logStr), "MFXEnumImpl(IMPLDESC - idx = %d)", idx);
            VPL_LOG_TIME_START(mfxenum, logStr);

            sts = MFXEnumImplementations(loader,
                                         idx,
                                         MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                         reinterpret_cast<mfxHDL *>(&idesc));

            if (sts != MFX_ERR_NONE || idesc == nullptr)
                break;

            VPL_LOG_TIME_END(mfxenum); // don't print log on last pass (no impl)

            printf("  Implementation name: %s\n", idesc->ImplName);

            MFXDispReleaseImplDescription(loader, idesc);
            idx++;
        }
    }

    if (bPrintImplPath) {
        mfxU32 idx = 0;
        while (1) {
            mfxHDL hImplPath = nullptr;

            char logStr[1024] = {};
            snprintf(logStr, sizeof(logStr), "MFXEnumImpl(IMPLPATH - idx = %d)", idx);
            VPL_LOG_TIME_START(mfxenum, logStr);

            sts = MFXEnumImplementations(loader, idx, MFX_IMPLCAPS_IMPLPATH, &hImplPath);

            if (sts != MFX_ERR_NONE || hImplPath == nullptr)
                break;

            VPL_LOG_TIME_END(mfxenum); // don't print log on last pass (no impl)

            printf("  Implementation path[%d]: %s\n", idx, reinterpret_cast<mfxChar *>(hImplPath));

            MFXDispReleaseImplDescription(loader, hImplPath);
            idx++;
        }
    }

    VPL_LOG_TIME_START(mfxcreatesession, "MFXCreateSession");

    // try to create session
    sts = MFXCreateSession(loader, 0, &session);
    if (sts != MFX_ERR_NONE) {
        printf("Error - MFXCreateSession returned %d\n", sts);
        return -1;
    }

    VPL_LOG_TIME_END(mfxcreatesession);

    VPL_LOG_TIME_END(totaltime);

    printf("\n");

    mfxVersion actualVersion = {};

    sts = MFXQueryVersion(session, &actualVersion);
    if (sts == MFX_ERR_NONE) {
        printf("  Loaded API version = %d.%d\n", actualVersion.Major, actualVersion.Minor);
    }
    else {
        printf("  Warning - MFXQueryVersion returned %d\n", sts);
    }

    // try to create basic encoder
    mfxVideoParam par = {};
    SetDefaultParamsEncode(&par);

    sts = MFXVideoENCODE_Init(session, &par);
    if (sts != MFX_ERR_NONE) {
        printf("Error - MFXVideoENCODE_Init returned %d\n", sts);
        MFXClose(session);
        MFXUnload(loader);
        return -1;
    }
    printf("\nMFXVideoENCODE_Init succeeded\n");

    MFXVideoENCODE_Close(session);

    // teardown
    MFXClose(session);
    MFXUnload(loader);

    printf("Finished\n");

    return 0;
}

static mfxStatus GetDispatcherVersion(mfxDispatcherVersion *ver) {
#if defined(_WIN32) || defined(_WIN64)
    std::vector<char> fileInfoBuf;
    if (!ver)
        return MFX_ERR_NULL_PTR;

    #ifdef NDEBUG
    HMODULE hm = GetModuleHandle("libvpl.dll");
    #else
    HMODULE hm = GetModuleHandle("libvpld.dll");
    #endif
    if (hm == 0)
        return MFX_ERR_UNSUPPORTED;

    char fullPathLibVPL[1024];
    DWORD filePathSize = GetModuleFileName(hm, fullPathLibVPL, sizeof(fullPathLibVPL));
    if (filePathSize == 0)
        return MFX_ERR_UNSUPPORTED;

    DWORD dwHandle;
    DWORD fileVersionInfoSize = GetFileVersionInfoSize(fullPathLibVPL, &dwHandle);
    if (fileVersionInfoSize == 0)
        return MFX_ERR_UNSUPPORTED;

    fileInfoBuf.resize(fileVersionInfoSize);
    bool bRes = GetFileVersionInfo(fullPathLibVPL, 0, fileVersionInfoSize, &fileInfoBuf[0]);
    if (!bRes) {
        return MFX_ERR_UNSUPPORTED;
    }

    UINT verSize;
    VS_FIXEDFILEINFO *vf;
    bRes = VerQueryValue(&fileInfoBuf[0], "\\", (void **)&vf, &verSize);
    if (!bRes) {
        return MFX_ERR_UNSUPPORTED;
    }

    ver->Major = (vf->dwFileVersionMS >> 16) & 0x0000ffff;
    ver->Minor = (vf->dwFileVersionMS >> 0) & 0x0000ffff;
    ver->Patch = (vf->dwFileVersionLS >> 16) & 0x0000ffff;
    ver->Tweak = (vf->dwFileVersionLS >> 0) & 0x0000ffff;

    return MFX_ERR_NONE;
#else
    return MFX_ERR_UNSUPPORTED;
#endif
}