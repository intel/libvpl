/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>

#include "vpl/mfxdispatcher.h"
#include "vpl/mfxjpeg.h"
#include "vpl/mfxstructures.h"
#include "vpl/mfxvp8.h"

#define DECODE_FOURCC(ch) ch & 0xff, ch >> 8 & 0xff, ch >> 16 & 0xff, ch >> 24 & 0xff
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                                   \
    ((mfxU32)(mfxU8)(ch0) | ((mfxU32)(mfxU8)(ch1) << 8) | ((mfxU32)(mfxU8)(ch2) << 16) | \
     ((mfxU32)(mfxU8)(ch3) << 24))
#define STRING_OPTION(x) \
    case x:              \
        return #x

const char *_print_Impl(mfxIMPL impl) {
    switch (impl) {
        STRING_OPTION(MFX_IMPL_SOFTWARE);
        STRING_OPTION(MFX_IMPL_HARDWARE);
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

const char *_print_ResourceType(mfxResourceType type) {
    switch (type) {
        STRING_OPTION(MFX_RESOURCE_SYSTEM_SURFACE);
        STRING_OPTION(MFX_RESOURCE_VA_SURFACE);
        STRING_OPTION(MFX_RESOURCE_VA_BUFFER);
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
    if (argc == 2 && !strncmp(argv[1], "-f", 2)) {
        bPrintImplementedFunctions = true;
    }

    int i = 0;
    mfxImplDescription *idesc;
    while (MFX_ERR_NONE == MFXEnumImplementations(loader,
                                                  i,
                                                  MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                                  reinterpret_cast<mfxHDL *>(&idesc))) {
        printf("\nImplementation: %s\n", idesc->ImplName);

        printf("  Version: %hu.%hu\n", idesc->Version.Major, idesc->Version.Minor);
        printf("  Impl: %s\n", _print_Impl(idesc->Impl));
        printf("  AccelerationMode: %s\n", _print_AccelMode(idesc->AccelerationMode));
        printf("  ApiVersion: %hu.%hu\n", idesc->ApiVersion.Major, idesc->ApiVersion.Minor);
        printf("  License: %s\n", idesc->License);
        printf("  Keywords: %s\n", idesc->Keywords);
        printf("  VendorID: 0x%04X\n", idesc->VendorID);
        printf("  VendorImplID: 0x%04X\n", idesc->VendorImplID);

        /* mfxAccelerationModeDescription */
        mfxAccelerationModeDescription *accel = &idesc->AccelerationModeDescription;
        printf("\n mfxAccelerationModeDescription:\n");
        printf("    Version: %hu.%hu\n", accel->Version.Major, accel->Version.Minor);
        for (int mode = 0; mode < accel->NumAccelerationModes; mode++) {
            printf("    Mode: %s\n", _print_AccelMode(accel->Mode[mode]));
        }

        /* mfxDeviceDescription */
        mfxDeviceDescription *dev = &idesc->Dev;
        printf("\n mfxDeviceDescription:\n");
        printf("    Version: %hu.%hu\n", dev->Version.Major, dev->Version.Minor);
        printf("    DeviceID: %s\n", dev->DeviceID);
        for (int subdevice = 0; subdevice < dev->NumSubDevices; subdevice++) {
            printf("    Index: %u\n", dev->SubDevices[subdevice].Index);
            printf("    SubDeviceID: %s\n", dev->SubDevices[subdevice].SubDeviceID);
        }

        /* mfxDecoderDescription */
        mfxDecoderDescription *dec = &idesc->Dec;
        printf("\n  mfxDecoderDescription:\n");
        printf("    Version: %hu.%hu\n", dec->Version.Major, dec->Version.Minor);
        for (int codec = 0; codec < dec->NumCodecs; codec++) {
            printf("    CodecID: %c%c%c%c\n", DECODE_FOURCC(dec->Codecs[codec].CodecID));
            printf("    MaxcodecLevel: %hu\n", dec->Codecs[codec].MaxcodecLevel);
            for (int profile = 0; profile < dec->Codecs[codec].NumProfiles; profile++) {
                printf("      Profile: %s\n",
                       _print_ProfileType(dec->Codecs[codec].CodecID,
                                          dec->Codecs[codec].Profiles[profile].Profile));
                for (int memtype = 0; memtype < dec->Codecs[codec].Profiles[profile].NumMemTypes;
                     memtype++) {
                    printf(
                        "        MemHandleType: %s\n",
                        _print_ResourceType(
                            dec->Codecs[codec].Profiles[profile].MemDesc[memtype].MemHandleType));
                    printf("        Width Min: %u\n",
                           dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Min);
                    printf("        Width Max: %u\n",
                           dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Max);
                    printf("        Width Step: %u\n",
                           dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Step);
                    printf("        Height Min: %u\n",
                           dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Min);
                    printf("        Height Max: %u\n",
                           dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Max);
                    printf("        Height Step: %u\n",
                           dec->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Step);
                    for (int colorformat = 0;
                         colorformat <
                         dec->Codecs[codec].Profiles[profile].MemDesc[memtype].NumColorFormats;
                         colorformat++) {
                        printf("        ColorFormat: %c%c%c%c\n",
                               DECODE_FOURCC(dec->Codecs[codec]
                                                 .Profiles[profile]
                                                 .MemDesc[memtype]
                                                 .ColorFormats[colorformat]));
                    }
                }
            }
        }

        /* mfxEncoderDescription */
        mfxEncoderDescription *enc = &idesc->Enc;
        printf("\n  mfxEncoderDescription:\n");
        printf("    Version: %hu.%hu\n", enc->Version.Major, enc->Version.Minor);
        for (int codec = 0; codec < enc->NumCodecs; codec++) {
            printf("    CodecID: %c%c%c%c\n", DECODE_FOURCC(enc->Codecs[codec].CodecID));
            printf("    MaxcodecLevel: %hu\n", enc->Codecs[codec].MaxcodecLevel);
            printf("    BiDirectionalPrediction: %hu\n",
                   enc->Codecs[codec].BiDirectionalPrediction);
            for (int profile = 0; profile < enc->Codecs[codec].NumProfiles; profile++) {
                printf("      Profile: %s\n",
                       _print_ProfileType(enc->Codecs[codec].CodecID,
                                          enc->Codecs[codec].Profiles[profile].Profile));
                for (int memtype = 0; memtype < enc->Codecs[codec].Profiles[profile].NumMemTypes;
                     memtype++) {
                    printf(
                        "        MemHandleType: %s\n",
                        _print_ResourceType(
                            enc->Codecs[codec].Profiles[profile].MemDesc[memtype].MemHandleType));
                    printf("        Width Min: %u\n",
                           enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Min);
                    printf("        Width Max: %u\n",
                           enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Max);
                    printf("        Width Step: %u\n",
                           enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Step);
                    printf("        Height Min: %u\n",
                           enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Min);
                    printf("        Height Max: %u\n",
                           enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Max);
                    printf("        Height Step: %u\n",
                           enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Step);
                    for (int colorformat = 0;
                         colorformat <
                         enc->Codecs[codec].Profiles[profile].MemDesc[memtype].NumColorFormats;
                         colorformat++) {
                        printf("        ColorFormat: %c%c%c%c\n",
                               DECODE_FOURCC(enc->Codecs[codec]
                                                 .Profiles[profile]
                                                 .MemDesc[memtype]
                                                 .ColorFormats[colorformat]));
                    }
                }
            }
        }

        /* mfxVPPDescription */
        mfxVPPDescription *vpp = &idesc->VPP;
        printf("\n  mfxVPPDescription:\n");
        printf("    Version: %hu.%hu\n", vpp->Version.Major, vpp->Version.Minor);
        for (int filter = 0; filter < vpp->NumFilters; filter++) {
            printf("    FilterFourCC: %c%c%c%c\n",
                   DECODE_FOURCC(vpp->Filters[filter].FilterFourCC));
            printf("    MaxDelayInFrames: %hu\n", vpp->Filters[filter].MaxDelayInFrames);
            for (int memtype = 0; memtype < vpp->Filters[filter].NumMemTypes; memtype++) {
                printf("      MemHandleType: %s\n",
                       _print_ResourceType(vpp->Filters[filter].MemDesc[memtype].MemHandleType));
                printf("      Width Min: %u\n", vpp->Filters[filter].MemDesc[memtype].Width.Min);
                printf("      Width Max: %u\n", vpp->Filters[filter].MemDesc[memtype].Width.Max);
                printf("      Width Step: %u\n", vpp->Filters[filter].MemDesc[memtype].Width.Step);
                printf("      Height Min: %u\n", vpp->Filters[filter].MemDesc[memtype].Width.Min);
                printf("      Height Max: %u\n", vpp->Filters[filter].MemDesc[memtype].Width.Max);
                printf("      Height Step: %u\n", vpp->Filters[filter].MemDesc[memtype].Width.Step);
                for (int informat = 0;
                     informat < vpp->Filters[filter].MemDesc[memtype].NumInFormats;
                     informat++) {
                    printf("          InFormat: %c%c%c%c\n",
                           DECODE_FOURCC(
                               vpp->Filters[filter].MemDesc[memtype].Formats[informat].InFormat));
                    for (int outformat = 0;
                         outformat <
                         vpp->Filters[filter].MemDesc[memtype].Formats[informat].NumOutFormat;
                         outformat++)
                        printf("            OutFormat: %c%c%c%c\n",
                               DECODE_FOURCC(vpp->Filters[filter]
                                                 .MemDesc[memtype]
                                                 .Formats[informat]
                                                 .OutFormats[outformat]));
                }
            }
        }

        printf("\n  NumExtParam: %d\n", idesc->NumExtParam);

        MFXDispReleaseImplDescription(loader, idesc);

        if (bPrintImplementedFunctions) {
            mfxImplementedFunctions *fdesc;

            mfxStatus sts = MFXEnumImplementations(loader,
                                                   i,
                                                   MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS,
                                                   reinterpret_cast<mfxHDL *>(&fdesc));

            if (sts == MFX_ERR_NONE) {
                // print out list of functions' name
                printf("\nImplemented functions:\n");
                std::for_each(fdesc->FunctionsName,
                              fdesc->FunctionsName + fdesc->NumFunctions,
                              [](mfxChar *functionName) {
                                  printf("  %s\n", functionName);
                              });
                printf("\n");

                MFXDispReleaseImplDescription(loader, fdesc);
            }
            else {
                printf("Warning - MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS not supported\n");
            }
        }

        i++;
    }

    if (i == 0)
        printf("\nWarning - no implementations found by MFXEnumImplementations()\n");
    else
        printf("\nTotal number of implementations found = %d\n", i);

    MFXUnload(loader);
    return 0;
}
