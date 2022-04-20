/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <assert.h>
#include <stdio.h>

#include <algorithm>
#include <string>

#include "vpl/mfx.h"

#define DECODE_FOURCC(ch) ch & 0xff, ch >> 8 & 0xff, ch >> 16 & 0xff, ch >> 24 & 0xff

#define DECODE_FOURCC_2(ch, s) \
    s[0] = ch & 0xff;          \
    s[1] = ch >> 8 & 0xff;     \
    s[2] = ch >> 16 & 0xff;    \
    s[3] = ch >> 24 & 0xff;

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                                   \
    ((mfxU32)(mfxU8)(ch0) | ((mfxU32)(mfxU8)(ch1) << 8) | ((mfxU32)(mfxU8)(ch2) << 16) | \
     ((mfxU32)(mfxU8)(ch3) << 24))
#define STRING_OPTION(x) \
    case x:              \
        return #x

const char *_print_fourcc(int ch) {
    static char str[5];
    if (0 == ch) {
        str[0] = 'U';
        str[1] = 'N';
        str[2] = 'K';
        str[3] = 'N';
        str[4] = '\0';
    }
    else if (41 == ch) {
        str[0] = 'P';
        str[1] = '8';
        str[2] = '\0';
    }
    else {
        DECODE_FOURCC_2(ch, str);
        str[4] = '\0';
    }
    return str;
}

const char *_print_Impl(mfxIMPL impl) {
    switch (impl) {
        STRING_OPTION(MFX_IMPL_TYPE_SOFTWARE);
        STRING_OPTION(MFX_IMPL_TYPE_HARDWARE);
    }

    return "<unknown implementation>";
}

const char *_print_AccelMode(mfxAccelerationMode mode) {
    switch (mode) {
        STRING_OPTION(MFX_ACCEL_MODE_NA);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_D3D9);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_D3D11);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_DRM_MODESET);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_GLX);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_X11);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_WAYLAND);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_HDDLUNITE);
    }

    return "<unknown acceleration mode>";
}

const char *_print_PoolPolicy(mfxPoolAllocationPolicy policy) {
    switch (policy) {
        STRING_OPTION(MFX_ALLOCATION_OPTIMAL);
        STRING_OPTION(MFX_ALLOCATION_UNLIMITED);
        STRING_OPTION(MFX_ALLOCATION_LIMITED);
    }

    return "<unknown pool allocation policy>";
}

const char *_print_MediaAdapterType(mfxMediaAdapterType type) {
    switch (type) {
        STRING_OPTION(MFX_MEDIA_UNKNOWN);
        STRING_OPTION(MFX_MEDIA_INTEGRATED);
        STRING_OPTION(MFX_MEDIA_DISCRETE);
    }

    return "<unknown media adapter type>";
}

const char *_print_EncodeStatsType(mfxU16 type) {
    switch (type) {
        STRING_OPTION(MFX_ENCODESTATS_LEVEL_BLK);
        STRING_OPTION(MFX_ENCODESTATS_LEVEL_SLICE);
        STRING_OPTION(MFX_ENCODESTATS_LEVEL_TILE);
        STRING_OPTION(MFX_ENCODESTATS_LEVEL_FRAME);
    }

    return "<unknown encode stats type>";
}

const char *_print_ResourceType(mfxResourceType type) {
    switch (type) {
        STRING_OPTION(MFX_RESOURCE_SYSTEM_SURFACE);
        STRING_OPTION(MFX_RESOURCE_VA_SURFACE_PTR);
        STRING_OPTION(MFX_RESOURCE_VA_BUFFER_PTR);
        STRING_OPTION(MFX_RESOURCE_DX9_SURFACE);
        STRING_OPTION(MFX_RESOURCE_DX11_TEXTURE);
        STRING_OPTION(MFX_RESOURCE_DX12_RESOURCE);
        STRING_OPTION(MFX_RESOURCE_DMA_RESOURCE);
        STRING_OPTION(MFX_RESOURCE_HDDLUNITE_REMOTE_MEMORY);
    }

    return "<unknown resource type>";
}

const char *_print_ProfileType(mfxU32 fourcc, mfxU32 type) {
    switch (fourcc) {
        case MFX_CODEC_JPEG: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_JPEG_BASELINE);

                default:
                    return "<unknown MFX_CODEC_JPEG profile>";
            }
        }

        case MFX_CODEC_AVC: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);

                STRING_OPTION(MFX_PROFILE_AVC_BASELINE);
                STRING_OPTION(MFX_PROFILE_AVC_MAIN);
                STRING_OPTION(MFX_PROFILE_AVC_EXTENDED);
                STRING_OPTION(MFX_PROFILE_AVC_HIGH);
                STRING_OPTION(MFX_PROFILE_AVC_HIGH10);
                STRING_OPTION(MFX_PROFILE_AVC_HIGH_422);
                STRING_OPTION(MFX_PROFILE_AVC_CONSTRAINED_BASELINE);
                STRING_OPTION(MFX_PROFILE_AVC_CONSTRAINED_HIGH);
                STRING_OPTION(MFX_PROFILE_AVC_PROGRESSIVE_HIGH);

                default:
                    return "<unknown MFX_CODEC_AVC profile>";
            }
        }

        case MFX_CODEC_HEVC: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_HEVC_MAIN);
                STRING_OPTION(MFX_PROFILE_HEVC_MAIN10);
                STRING_OPTION(MFX_PROFILE_HEVC_MAINSP);
                STRING_OPTION(MFX_PROFILE_HEVC_REXT);
                STRING_OPTION(MFX_PROFILE_HEVC_SCC);

                default:
                    return "<unknown MFX_CODEC_HEVC profile>";
            }
        }

        case MFX_CODEC_MPEG2: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_MPEG2_SIMPLE);
                STRING_OPTION(MFX_PROFILE_MPEG2_MAIN);
                STRING_OPTION(MFX_LEVEL_MPEG2_HIGH);
                STRING_OPTION(MFX_LEVEL_MPEG2_HIGH1440);

                default:
                    return "<unknown MFX_CODEC_MPEG2 profile>";
            }
        }

        case MFX_CODEC_VP8: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_VP8_0);
                STRING_OPTION(MFX_PROFILE_VP8_1);
                STRING_OPTION(MFX_PROFILE_VP8_2);
                STRING_OPTION(MFX_PROFILE_VP8_3);

                default:
                    return "<unknown MFX_CODEC_VP9 profile>";
            }
        }

        case MFX_CODEC_VC1: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_VC1_SIMPLE);
                STRING_OPTION(MFX_PROFILE_VC1_MAIN);
                STRING_OPTION(MFX_PROFILE_VC1_ADVANCED);

                default:
                    return "<unknown MFX_CODEC_VC1 profile>";
            }
        }

        case MFX_CODEC_VP9: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_VP9_0);
                STRING_OPTION(MFX_PROFILE_VP9_1);
                STRING_OPTION(MFX_PROFILE_VP9_2);
                STRING_OPTION(MFX_PROFILE_VP9_3);

                default:
                    return "<unknown MFX_CODEC_VP9 profile>";
            }
        }

        case MFX_CODEC_AV1: {
            switch (type) {
                STRING_OPTION(MFX_PROFILE_UNKNOWN);
                STRING_OPTION(MFX_PROFILE_AV1_MAIN);
                STRING_OPTION(MFX_PROFILE_AV1_HIGH);
                STRING_OPTION(MFX_PROFILE_AV1_PRO);

                default:
                    return "<unknown MFX_CODEC_AV1 profile>";
            }
        }
    }

    return "<unknown codec format>";
}

int main(int argc, char *argv[]) {
    mfxLoader loader = MFXLoad();
    if (loader == NULL) {
        printf("Error - MFXLoad() returned null - no libraries found\n");
        return -1;
    }

    bool bPrintImplementedFunctions = false;
    bool bFullInfo                  = true;
    bool bPrintExtendedDeviceID     = false;
    bool bRequireD3D9               = false;

    for (int argIdx = 1; argIdx < argc; argIdx++) {
        std::string nextArg(argv[argIdx]);

        if (nextArg == "-f") {
            bPrintImplementedFunctions = true;
        }
#ifdef ONEVPL_EXPERIMENTAL
        else if (nextArg == "-ex") {
            bPrintExtendedDeviceID = true;
        }
#endif
        else if (nextArg == "-b") {
            bFullInfo = false;
        }
        else if (nextArg == "-d3d9") {
            bRequireD3D9 = true;
        }
        else {
            printf("Error - unknown option %s\n", nextArg.c_str());
            return -1;
        }
    }

    if (bRequireD3D9) {
        printf("Warning - Enumerating D3D9 implementations ONLY\n");
        mfxConfig cfg = MFXCreateConfig(loader);
        if (!cfg) {
            printf("Error - MFXCreateConfig() returned null\n");
            return -1;
        }

        mfxVariant var      = {};
        var.Version.Version = MFX_VARIANT_VERSION;
        var.Type            = MFX_VARIANT_TYPE_U32;
        var.Data.U32        = MFX_ACCEL_MODE_VIA_D3D9;

        mfxStatus sts =
            MFXSetConfigFilterProperty(cfg,
                                       (const mfxU8 *)"mfxImplDescription.AccelerationMode",
                                       var);
        if (sts) {
            printf("Error - MFXSetConfigFilterProperty() returned %d\n", sts);
            return -1;
        }
    }

    int i = 0;
    mfxImplDescription *idesc;
    while (MFX_ERR_NONE == MFXEnumImplementations(loader,
                                                  i,
                                                  MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                                  reinterpret_cast<mfxHDL *>(&idesc))) {
        printf("\nImplementation #%d: %s\n", i, idesc->ImplName);

        // get path if supported (available starting with API 2.4)
        mfxHDL hImplPath = nullptr;
        if (MFX_ERR_NONE == MFXEnumImplementations(loader, i, MFX_IMPLCAPS_IMPLPATH, &hImplPath)) {
            if (hImplPath) {
                printf("%2sLibrary path: %s\n", "", reinterpret_cast<mfxChar *>(hImplPath));
                MFXDispReleaseImplDescription(loader, hImplPath);
            }
        }

        printf("%2sAccelerationMode: %s\n", "", _print_AccelMode(idesc->AccelerationMode));
        printf("%2sApiVersion: %hu.%hu\n", "", idesc->ApiVersion.Major, idesc->ApiVersion.Minor);
        printf("%2sImpl: %s\n", "", _print_Impl(idesc->Impl));
        printf("%2sVendorImplID: 0x%04X\n", "", idesc->VendorImplID);
        printf("%2sImplName: %s\n", "", idesc->ImplName);
        printf("%2sLicense: %s\n", "", idesc->License);
        printf("%2sVersion: %hu.%hu\n", "", idesc->Version.Major, idesc->Version.Minor);
        printf("%2sKeywords: %s\n", "", idesc->Keywords);
        printf("%2sVendorID: 0x%04X\n", "", idesc->VendorID);

        /* mfxAccelerationModeDescription */
        mfxAccelerationModeDescription *accel = &idesc->AccelerationModeDescription;
        printf("%2smfxAccelerationModeDescription:\n", "");
        printf("%4sVersion: %hu.%hu\n", "", accel->Version.Major, accel->Version.Minor);
        for (int mode = 0; mode < accel->NumAccelerationModes; mode++) {
            printf("%4sMode: %s\n", "", _print_AccelMode(accel->Mode[mode]));
        }

        /* mfxPoolPolicyDescription */
        if (idesc->Version.Version >= MFX_STRUCT_VERSION(1, 2)) {
            mfxPoolPolicyDescription *poolPolicies = &idesc->PoolPolicies;
            printf("%2smfxPoolPolicyDescription:\n", "");
            printf("%4sVersion: %hu.%hu\n",
                   "",
                   poolPolicies->Version.Major,
                   poolPolicies->Version.Minor);
            for (int policy = 0; policy < poolPolicies->NumPoolPolicies; policy++) {
                printf("%4sPolicy: %s\n", "", _print_PoolPolicy(poolPolicies->Policy[policy]));
            }
        }

        /* mfxDeviceDescription */
        mfxDeviceDescription *dev = &idesc->Dev;
        printf("%2smfxDeviceDescription:\n", "");
        if (dev->Version.Version >= MFX_STRUCT_VERSION(1, 1)) {
            printf("%4sMediaAdapterType: %s\n",
                   "",
                   _print_MediaAdapterType((mfxMediaAdapterType)dev->MediaAdapterType));
        }
        printf("%4sDeviceID: %s\n", "", dev->DeviceID);
        printf("%4sVersion: %hu.%hu\n", "", dev->Version.Major, dev->Version.Minor);
        for (int subdevice = 0; subdevice < dev->NumSubDevices; subdevice++) {
            printf("%4sIndex: %u\n", "", dev->SubDevices[subdevice].Index);
            printf("%4sSubDeviceID: %s\n", "", dev->SubDevices[subdevice].SubDeviceID);
        }

        if (bFullInfo) {
            /* mfxDecoderDescription */
            mfxDecoderDescription *dec = &idesc->Dec;
            printf("%2smfxDecoderDescription:\n", "");
            printf("%4sVersion: %hu.%hu\n", "", dec->Version.Major, dec->Version.Minor);
            for (int codec = 0; codec < dec->NumCodecs; codec++) {
                printf("%4sCodecID: %c%c%c%c\n", "", DECODE_FOURCC(dec->Codecs[codec].CodecID));
                printf("%4sMaxcodecLevel: %hu\n", "", dec->Codecs[codec].MaxcodecLevel);
                for (int profile = 0; profile < dec->Codecs[codec].NumProfiles; profile++) {
                    printf("%6sProfile: %s\n",
                           "",
                           _print_ProfileType(dec->Codecs[codec].CodecID,
                                              dec->Codecs[codec].Profiles[profile].Profile));
                    for (int memtype = 0;
                         memtype < dec->Codecs[codec].Profiles[profile].NumMemTypes;
                         memtype++) {
                        printf("%8sMemHandleType: %s\n",
                               "",
                               _print_ResourceType(dec->Codecs[codec]
                                                       .Profiles[profile]
                                                       .MemDesc[memtype]
                                                       .MemHandleType));
                        printf("%10sWidth Min: %u\n",
                               "",
                               dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Min);
                        printf("%10sWidth Max: %u\n",
                               "",
                               dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Max);
                        printf("%10sWidth Step: %u\n",
                               "",
                               dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Step);
                        printf("%10sHeight Min: %u\n",
                               "",
                               dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Min);
                        printf("%10sHeight Max: %u\n",
                               "",
                               dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Max);
                        printf("%10sHeight Step: %u\n",
                               "",
                               dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Step);
                        printf("%10sColorFormats: ", "");
                        for (int colorformat = 0;
                             colorformat <
                             dec->Codecs[codec].Profiles[profile].MemDesc[memtype].NumColorFormats;
                             colorformat++) {
                            if (0 != colorformat)
                                printf(", ");
                            printf("%s",
                                   _print_fourcc(dec->Codecs[codec]
                                                     .Profiles[profile]
                                                     .MemDesc[memtype]
                                                     .ColorFormats[colorformat]));
                        }
                        printf("\n");
                    }
                }
            }

            /* mfxEncoderDescription */
            mfxEncoderDescription *enc = &idesc->Enc;
            printf("%2smfxEncoderDescription:\n", "");
            printf("%4sVersion: %hu.%hu\n", "", enc->Version.Major, enc->Version.Minor);
            for (int codec = 0; codec < enc->NumCodecs; codec++) {
                printf("%4sCodecID: %c%c%c%c\n", "", DECODE_FOURCC(enc->Codecs[codec].CodecID));
                printf("%4sMaxcodecLevel: %hu\n", "", enc->Codecs[codec].MaxcodecLevel);
                printf("%4sBiDirectionalPrediction: %hu\n",
                       "",
                       enc->Codecs[codec].BiDirectionalPrediction);

#ifdef ONEVPL_EXPERIMENTAL
                // Once ReportedStats is moved out of experimental API the struct version of mfxEncoderDescription should
                //   be updated, and that can be used to know whether this field is valid.
                // For now, just check implementation API version.
                mfxVersion reqApiVersionReportedStats = {};
                reqApiVersionReportedStats.Major      = 2;
                reqApiVersionReportedStats.Minor      = 7;
                if (idesc->ApiVersion.Version >= reqApiVersionReportedStats.Version) {
                    mfxU16 reportedStats = enc->Codecs[codec].ReportedStats;
                    if (reportedStats) {
                        for (mfxU16 statMask = 1; statMask != 0; statMask <<= 1) {
                            if (reportedStats & statMask) {
                                const char *statStr = _print_EncodeStatsType(statMask);
                                printf("%4sReportedStats: %s\n", "", statStr);
                            }
                        }
                    }
                    else {
                        printf("%4sReportedStats: 0\n", "");
                    }
                }
#endif
                for (int profile = 0; profile < enc->Codecs[codec].NumProfiles; profile++) {
                    printf("%6sProfile: %s\n",
                           "",
                           _print_ProfileType(enc->Codecs[codec].CodecID,
                                              enc->Codecs[codec].Profiles[profile].Profile));
                    for (int memtype = 0;
                         memtype < enc->Codecs[codec].Profiles[profile].NumMemTypes;
                         memtype++) {
                        printf("%8sMemHandleType: %s\n",
                               "",
                               _print_ResourceType(enc->Codecs[codec]
                                                       .Profiles[profile]
                                                       .MemDesc[memtype]
                                                       .MemHandleType));
                        printf("%10sWidth Min: %u\n",
                               "",
                               enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Min);
                        printf("%10sWidth Max: %u\n",
                               "",
                               enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Max);
                        printf("%10sWidth Step: %u\n",
                               "",
                               enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Step);
                        printf("%10sHeight Min: %u\n",
                               "",
                               enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Min);
                        printf("%10sHeight Max: %u\n",
                               "",
                               enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Max);
                        printf("%10sHeight Step: %u\n",
                               "",
                               enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Step);
                        printf("%10sColorFormats: ", "");
                        for (int colorformat = 0;
                             colorformat <
                             enc->Codecs[codec].Profiles[profile].MemDesc[memtype].NumColorFormats;
                             colorformat++) {
                            if (0 != colorformat)
                                printf(", ");
                            printf("%s",
                                   _print_fourcc(enc->Codecs[codec]
                                                     .Profiles[profile]
                                                     .MemDesc[memtype]
                                                     .ColorFormats[colorformat]));
                        }
                        printf("\n");
                    }
                }
            }

            /* mfxVPPDescription */
            mfxVPPDescription *vpp = &idesc->VPP;
            printf("%2smfxVPPDescription:\n", "");
            printf("%4sVersion: %hu.%hu\n", "", vpp->Version.Major, vpp->Version.Minor);
            for (int filter = 0; filter < vpp->NumFilters; filter++) {
                printf("%4sFilterFourCC: %c%c%c%c\n",
                       "",
                       DECODE_FOURCC(vpp->Filters[filter].FilterFourCC));
                printf("%4sMaxDelayInFrames: %hu\n", "", vpp->Filters[filter].MaxDelayInFrames);
                for (int memtype = 0; memtype < vpp->Filters[filter].NumMemTypes; memtype++) {
                    printf(
                        "%6sMemHandleType: %s\n",
                        "",
                        _print_ResourceType(vpp->Filters[filter].MemDesc[memtype].MemHandleType));
                    printf("%6sWidth Min: %u\n",
                           "",
                           vpp->Filters[filter].MemDesc[memtype].Width.Min);
                    printf("%6sWidth Max: %u\n",
                           "",
                           vpp->Filters[filter].MemDesc[memtype].Width.Max);
                    printf("%6sWidth Step: %u\n",
                           "",
                           vpp->Filters[filter].MemDesc[memtype].Width.Step);
                    printf("%6sHeight Min: %u\n",
                           "",
                           vpp->Filters[filter].MemDesc[memtype].Width.Min);
                    printf("%6sHeight Max: %u\n",
                           "",
                           vpp->Filters[filter].MemDesc[memtype].Width.Max);
                    printf("%6sHeight Step: %u\n",
                           "",
                           vpp->Filters[filter].MemDesc[memtype].Width.Step);
                    for (int informat = 0;
                         informat < vpp->Filters[filter].MemDesc[memtype].NumInFormats;
                         informat++) {
                        printf(
                            "%8sInFormat: %s\n",
                            "",
                            _print_fourcc(
                                vpp->Filters[filter].MemDesc[memtype].Formats[informat].InFormat));
                        printf("%10sOutFormats: ", "");
                        for (int outformat = 0;
                             outformat <
                             vpp->Filters[filter].MemDesc[memtype].Formats[informat].NumOutFormat;
                             outformat++) {
                            if (0 != outformat)
                                printf(", ");
                            printf("%s",
                                   _print_fourcc(vpp->Filters[filter]
                                                     .MemDesc[memtype]
                                                     .Formats[informat]
                                                     .OutFormats[outformat]));
                        }
                        printf("\n");
                    }
                }
            }

            printf("%2sNumExtParam: %d\n", "", idesc->NumExtParam);
        }

        MFXDispReleaseImplDescription(loader, idesc);

        if (bPrintImplementedFunctions) {
            mfxImplementedFunctions *fdesc;

            mfxStatus sts = MFXEnumImplementations(loader,
                                                   i,
                                                   MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS,
                                                   reinterpret_cast<mfxHDL *>(&fdesc));

            if (sts == MFX_ERR_NONE) {
                // print out list of functions' name
                printf("%2sImplemented functions:\n", "");
                std::for_each(fdesc->FunctionsName,
                              fdesc->FunctionsName + fdesc->NumFunctions,
                              [](mfxChar *functionName) {
                                  printf("%4s%s\n", "", functionName);
                              });

                MFXDispReleaseImplDescription(loader, fdesc);
            }
            else {
                printf("%2sWarning - MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS not supported\n", "");
            }
        }

#ifdef ONEVPL_EXPERIMENTAL
        if (bPrintExtendedDeviceID) {
            mfxExtendedDeviceId *idescDevice;

            mfxStatus sts = MFXEnumImplementations(loader,
                                                   i,
                                                   MFX_IMPLCAPS_DEVICE_ID_EXTENDED,
                                                   reinterpret_cast<mfxHDL *>(&idescDevice));
            if (sts == MFX_ERR_NONE) {
                printf("%2sExtended DeviceID's:\n", "");
                printf("%6sVendorID: 0x%04X\n", "", idescDevice->VendorID);
                printf("%6sDeviceID: 0x%04X\n", "", idescDevice->DeviceID);

                printf("%6sPCIDomain: 0x%08X\n", "", idescDevice->PCIDomain);
                printf("%6sPCIBus: 0x%08X\n", "", idescDevice->PCIBus);
                printf("%6sPCIdevice: 0x%08X\n", "", idescDevice->PCIDevice);
                printf("%6sPCIFunction: 0x%08X\n", "", idescDevice->PCIFunction);

                if (idescDevice->LUIDValid) {
                    printf("%6sDeviceLUID: ", "");
                    for (mfxU32 idx = 0; idx < 8; idx++) {
                        printf("%02x", idescDevice->DeviceLUID[7 - idx]);
                    }
                    printf("\n");
                    printf("%6sLUIDDeviceNodeMask: 0x%04X\n", "", idescDevice->LUIDDeviceNodeMask);
                }

                printf("%6sLUIDValid: 0x%04X\n", "", idescDevice->LUIDValid);

                printf("%6sDRMRenderNodeNum: %d\n", "", idescDevice->DRMRenderNodeNum);
                printf("%6sDRMPrimaryNodeNum: 0x%04X\n", "", idescDevice->DRMPrimaryNodeNum);

                printf("%6sDeviceName: %s\n", "", idescDevice->DeviceName);
                MFXDispReleaseImplDescription(loader, idescDevice);
            }

            else {
                printf("%2sWarning - MFX_IMPLCAPS_DEVICE_ID_EXTENDED not supported\n", "");
            }
        }
#endif

        i++;
    }

    if (i == 0)
        printf("\nWarning - no implementations found by MFXEnumImplementations()\n");
    else
        printf("\nTotal number of implementations found = %d\n", i);

    MFXUnload(loader);
    return 0;
}
