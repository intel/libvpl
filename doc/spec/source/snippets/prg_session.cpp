/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "mfxdefs.h"
#include "mfxdispatcher.h"
#include "mfxstructures.h"

#include <iostream>
#include <algorithm>

mfxLoader loader = 0;
int idx = 0;

static void prg_session1 () {
/*beg1*/
mfxHDL h;
// request pointer to the list. Assume that implementation supports that.
// Assume that `loader` is configured before.
mfxStatus sts = MFXEnumImplementations(loader, idx, MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS, &h);
// break if no idx
if (sts != MFX_ERR_NOT_FOUND) {
  // Cast typeless handle to structure pointer
  mfxImplementedFunctions *implemented_functions = (mfxImplementedFunctions*)h;

  // print out list of functions' name
  std::for_each(implemented_functions->FunctionsName, implemented_functions->FunctionsName +
                                                      implemented_functions->NumFunctions,
          [](mfxChar*  functionName) {
              std::cout << functionName << " is implemented" << std::endl;
          });
  // Release resource
  MFXDispReleaseImplDescription(loader, h);
}
/*end1*/
}

static void prg_session2 () {
/*beg2*/
mfxSession session_handle;
loader = mfxLoader();

// We want to search for the implementation with Decode+VPP domain functions support.
// i.e we search for the MFXVideoDECODE_VPP_Init and MFXVideoDECODE_VPP_DecodeFrameAsync
// implemented functions
mfxConfig init_funct_prop = MFXCreateConfig(loader);
mfxVariant value;

// Filter property for the Init function
value.Type = mfxVariantType::MFX_VARIANT_TYPE_PTR;
value.Data.Ptr = (mfxHDL)"MFXVideoDECODE_VPP_Init";
MFXSetConfigFilterProperty(init_funct_prop, (const mfxU8*)"mfxImplementedFunctions.FunctionsName",
                           value);

// Filter property for the Process function
mfxConfig process_func_prop = MFXCreateConfig(loader);
value.Data.Ptr = (mfxHDL)"MFXVideoDECODE_VPP_DecodeFrameAsync";
MFXSetConfigFilterProperty(process_func_prop, (const mfxU8*)"mfxImplementedFunctions.FunctionsName",
                           value);

// create session from first matched implementation
MFXCreateSession(loader, 0, &session_handle);
/*end2*/
}

static void prg_session3 () {
/*beg3*/
mfxSession hevc_session_handle;
loader = mfxLoader();

// We want to search for the HEVC decoder implementation
mfxConfig hevc_decoder_config = MFXCreateConfig(loader);
mfxVariant value;

// Filter property for the implementations with HEVC decoder
value.Type     = MFX_VARIANT_TYPE_U32;
value.Data.U32 = MFX_CODEC_HEVC;

MFXSetConfigFilterProperty(hevc_decoder_config
      , (const mfxU8*)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID"
      , value);

// create session from first matched implementation
MFXCreateSession(loader, 0, &hevc_session_handle);
/*end3*/
}

static void prg_session4 () {
/*beg4*/
mfxSession mirror_session_handle;
loader = mfxLoader();

// We want to search for the VPP mirror implementation
mfxConfig  mirror_flt_config = MFXCreateConfig(loader);
mfxVariant value;

// Filter property for the implementations with VPP mirror
value.Type     = MFX_VARIANT_TYPE_U32;
value.Data.U32 = MFX_EXTBUFF_VPP_MIRRORING;

MFXSetConfigFilterProperty(mirror_flt_config
      , (const mfxU8*)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC"
      , value);

// create session from first matched implementation
MFXCreateSession(loader, 0, &mirror_session_handle);
/*end4*/
}

static void prg_session5 () {
/*beg5*/
mfxHDL h;
mfxSession def_session;

loader = mfxLoader();

// Create session from the first available implementation.
// That's why we no any filters need to be set.
// First available implementation has index equal to the 0.
MFXCreateSession(loader, 0, &def_session);

// Get and print out OS path to the loaded shared library
// with the implementation. It is absolutely OK to call
// MFXEnumImplementations after session creation just need to make
// sure that the same index of implementation is provided to the
// function call.
MFXEnumImplementations(loader, 0, MFX_IMPLCAPS_IMPLPATH, &h);
mfxChar* path = reinterpret_cast<mfxChar*>(h);

// Print out the path
std::cout << "Loaded shared library: " << path << std::endl;

// Release the memory for the string with path.
MFXDispReleaseImplDescription(loader, h);
/*end5*/
}

mfxSession session;

static void prg_session6 () {
/*beg6*/
mfxLoader loader = MFXLoad();

int i = 0;
mfxImplDescription *idesc;
mfxStructVersion minVersionEncoderDescription = { {1, 1} };
while (MFX_ERR_NONE == MFXEnumImplementations(loader, i, MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                              reinterpret_cast<mfxHDL *>(&idesc))) {
    mfxEncoderDescription *enc = &idesc->Enc;
    for (int codec = 0; codec < enc->NumCodecs; codec++) {
        printf("%4sCodecID: %u\n", "", enc->Codecs[codec].CodecID);
        printf("%4sMaxcodecLevel: %hu\n", "", enc->Codecs[codec].MaxcodecLevel);
        printf("%4sBiDirectionalPrediction: %hu\n", "",
               enc->Codecs[codec].BiDirectionalPrediction);
        if (enc->Version.Version >= minVersionEncoderDescription.Version) {
            if (enc->Codecs[codec].EncExtDesc) {
                printf("%4sRateControlMethods: ", "");
                for (int RateControls = 0;
                    RateControls < enc->Codecs[codec].EncExtDesc->NumRateControlMethods;
                    RateControls++) {
                    if (0 != RateControls)
                        printf(", ");
                    printf("%hu", enc->Codecs[codec].EncExtDesc->RateControlMethods[RateControls]);
                }
                printf("\n");
                printf("%4sExtBuffers: ", "");
                for (int ExtBufs = 0; ExtBufs < enc->Codecs[codec].EncExtDesc->NumExtBufferIDs;
                    ExtBufs++) {
                    if (0 != ExtBufs)
                        printf(", ");
                    printf("%u", enc->Codecs[codec].EncExtDesc->ExtBufferIDs[ExtBufs]);
                }
                printf("\n");
            }
        }
        for (int profile = 0; profile < enc->Codecs[codec].NumProfiles; profile++) {
            printf("%6sProfile: %u\n", "", enc->Codecs[codec].Profiles[profile].Profile);
            for (int memtype = 0; memtype < enc->Codecs[codec].Profiles[profile].NumMemTypes;
                memtype++) {
                printf("%8sMemHandleType: %u\n", "",
                       enc->Codecs[codec].Profiles[profile].MemDesc[memtype].MemHandleType);
                printf("%10sWidth Min: %u\n", "",
                       enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Min);
                printf("%10sWidth Max: %u\n", "",
                       enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Max);
                printf("%10sWidth Step: %u\n", "",
                       enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Width.Step);
                printf("%10sHeight Min: %u\n", "",
                       enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Min);
                printf("%10sHeight Max: %u\n", "",
                       enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Max);
                printf("%10sHeight Step: %u\n", "",
                       enc->Codecs[codec].Profiles[profile].MemDesc[memtype].Height.Step);

                if (enc->Version.Version >= minVersionEncoderDescription.Version) {
                    if (enc->Codecs[codec].Profiles[profile].MemDesc[memtype].MemExtDesc) {
                        printf("%10sTargetMaxBitDepth: %hu\n", "",
                                enc->Codecs[codec].Profiles[profile].MemDesc[memtype]
                                     .MemExtDesc->TargetMaxBitDepth);
                        printf("%10sTargetChromaSubsamplings: ", "");
                        for (int ChromaSubsampling = 0;
                             ChromaSubsampling <
                             enc->Codecs[codec].Profiles[profile].MemDesc[memtype]
                                  .MemExtDesc->NumTargetChromaSubsamplings;
                             ChromaSubsampling++) {
                            if (0 != ChromaSubsampling)
                                printf(", ");
                            printf("%hu",enc->Codecs[codec].Profiles[profile]
                                              .MemDesc[memtype].MemExtDesc
                                              ->TargetChromaSubsamplings[ChromaSubsampling]);
                        }
                        printf("\n");
                    }
                }

                printf("%10sColorFormats: ", "");
                for (int colorformat = 0;
                     colorformat <
                     enc->Codecs[codec].Profiles[profile].MemDesc[memtype].NumColorFormats;
                     colorformat++) {
                     if (0 != colorformat)
                         printf(", ");
                     printf("%u", enc->Codecs[codec].Profiles[profile]
                                       .MemDesc[memtype].ColorFormats[colorformat]);
                }
                printf("\n");
            }
        }
    }

    // Release resource
    MFXDispReleaseImplDescription(loader, idesc);

    i++;
}

// create session from first matched implementation
MFXCreateSession(loader, 0, &session);

/*end6*/
}

static void prg_session7 () {
/*beg7*/
mfxLoader loader = MFXLoad();
mfxConfig cfg = MFXCreateConfig(loader);
mfxVariant ImplValue;
ImplValue.Type = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
ImplValue.Data.U32 = 0;

//Only need shallow info for session creation
MFXSetConfigFilterProperty(cfg,(const mfxU8 *)"mfxImplDescription",ImplValue);

// create session from first matched implementation
MFXCreateSession(loader, 0, &session);
/*end7*/
}

static void prg_session8 () {
/*beg8*/
mfxLoader loader = MFXLoad();
mfxConfig cfg = MFXCreateConfig(loader);
mfxVariant ImplValue;
ImplValue.Type = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
ImplValue.Data.U32 = 0;

//Only need shallow info for exploring implementations
MFXSetConfigFilterProperty(cfg,(const mfxU8 *)"mfxImplDescription",ImplValue);

int i = 0;
mfxImplDescription *idesc;
while (MFX_ERR_NONE == MFXEnumImplementations(loader, i, MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                              reinterpret_cast<mfxHDL *>(&idesc))) {
    printf("\nImplementation #%d: %s\n", i, idesc->ImplName);

    printf("%2sApiVersion: %hu.%hu\n", "", idesc->ApiVersion.Major, idesc->ApiVersion.Minor);
    printf("%2sVendorImplID: 0x%04X\n", "", idesc->VendorImplID);
    printf("%2sVendorID: 0x%04X\n", "", idesc->VendorID);

    /* mfxDeviceDescription */
    mfxDeviceDescription *dev = &idesc->Dev;
    printf("%2smfxDeviceDescription:\n", "");
    printf("%4sDeviceID: %s\n", "", dev->DeviceID);

    // Release resource
    MFXDispReleaseImplDescription(loader, idesc);

    i++;
  }
/*end8*/
}

static void prg_session9 () {
/*beg9*/
mfxLoader loader = MFXLoad();
mfxConfig cfg = MFXCreateConfig(loader);
mfxVariant ImplValue;
ImplValue.Type = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
ImplValue.Data.U32 = 0;

//To find decoders for session creation
MFXSetConfigFilterProperty(cfg,(const mfxU8 *)"mfxImplDescription.mfxDecoderDescription",ImplValue);

// create session from first matched implementation
MFXCreateSession(loader, 0, &session);
/*end9*/
}

static void prg_session10 () {
/*beg10*/
mfxLoader loader = MFXLoad();
mfxConfig cfg = MFXCreateConfig(loader);
mfxVariant ImplValue;
ImplValue.Type = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
ImplValue.Data.U32 = MFX_CODEC_AV1;

//To find AV1 encoder for session creation
MFXSetConfigFilterProperty(cfg,(const mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID",ImplValue);

// create session from first matched implementation
// which supports AV1 encoder
MFXCreateSession(loader, 0, &session);
/*end10*/
}

static void prg_session11 () {
/*beg11*/
mfxLoader loader = MFXLoad();
mfxConfig cfg = MFXCreateConfig(loader);
mfxVariant ImplValue;
ImplValue.Type = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
ImplValue.Data.U32 = 0;

//To find VPP filters for session creation
MFXSetConfigFilterProperty(cfg,(const mfxU8 *)"mfxImplDescription.mfxVPPDescription",ImplValue);

// create session from first matched implementation
MFXCreateSession(loader, 0, &session);
/*end11*/
}

static void prg_session12 () {
/*beg12*/
mfxLoader loader = MFXLoad();
mfxConfig cfg = MFXCreateConfig(loader);
mfxVariant ImplValue;
ImplValue.Type = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
ImplValue.Data.U32 = MFX_CODEC_AV1;

//To get AV1 encoder capabilities
MFXSetConfigFilterProperty(cfg,(const mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID",ImplValue);

int i = 0;
mfxImplDescription *idesc;
while (MFX_ERR_NONE == MFXEnumImplementations(loader, i, MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                              reinterpret_cast<mfxHDL *>(&idesc))) {
    mfxEncoderDescription *enc = &idesc->Enc;
    for (int codec = 0; codec < enc->NumCodecs; codec++) {
        printf("%4sCodecID: %u\n", "", enc->Codecs[codec].CodecID);
        printf("%4sMaxcodecLevel: %hu\n", "", enc->Codecs[codec].MaxcodecLevel);
        printf("%4sBiDirectionalPrediction: %hu\n", "",
               enc->Codecs[codec].BiDirectionalPrediction);
        for (int profile = 0; profile < enc->Codecs[codec].NumProfiles; profile++) {
            printf("%6sProfile: %u\n", "", enc->Codecs[codec].Profiles[profile].Profile);
        }
    }

    // Release resource
    MFXDispReleaseImplDescription(loader, idesc);

    i++;
}

// create session from first matched implementation
// which supports AV1 encoder
MFXCreateSession(loader, 0, &session);
/*end12*/
}
