/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./vpl-common.h"

const char *MemoryModeString[MEM_MODE_COUNT] = { "MEM_MODE_EXTERNAL",
                                                 "MEM_MODE_INTERNAL",
                                                 "MEM_MODE_AUTO" };

const char *DispatcherModeString[DISPATCHER_MODE_COUNT] = {
    "DISPATCHER_MODE_LEGACY",
    "DISPATCHER_MODE_VPL_20"
};

// check if this implementation can decode our stream
bool CheckDecoderImplCaps(mfxImplDescription *implDesc, mfxU32 codecID) {
    mfxU32 i;

    for (i = 0; i < implDesc->Dec.NumCodecs; i++) {
        mfxDecoderDescription::decoder *currDec = &(implDesc->Dec.Codecs[i]);

        if (currDec->CodecID == codecID) {
            return true;
        }
    }

    return false;
}

// check if this implementation can encode our stream
bool CheckEncoderImplCaps(mfxImplDescription *implDesc,
                          mfxU32 rawFormat,
                          mfxU32 codecID) {
    mfxU32 i, j, k, n;

    for (i = 0; i < implDesc->Enc.NumCodecs; i++) {
        mfxEncoderDescription::encoder *currEnc = &(implDesc->Enc.Codecs[i]);
        if (currEnc->CodecID == codecID) {
            for (j = 0; j < currEnc->NumProfiles; j++) {
                mfxEncoderDescription::encoder::encprofile *currProfile =
                    &(currEnc->Profiles[j]);

                for (k = 0; k < currProfile->NumMemTypes; k++) {
                    mfxEncoderDescription::encoder::encprofile::encmemdesc
                        *currMemDesc = &(currProfile->MemDesc[k]);

                    for (n = 0; n < currMemDesc->NumColorFormats; n++) {
                        if (currMemDesc->ColorFormats[n] == rawFormat)
                            return true;
                    }
                }
            }
        }
    }

    return false;
}

// check if this implementation can process our stream (just check in/out colorspaces)
bool CheckVPPImplCaps(mfxImplDescription *implDesc,
                      mfxU32 inFormat,
                      mfxU32 outFormat) {
    mfxU32 i, j, k, n;

    for (i = 0; i < implDesc->VPP.NumFilters; i++) {
        mfxVPPDescription::filter *currFilter = &(implDesc->VPP.Filters[i]);
        if (currFilter->FilterFourCC == MFX_EXTBUFF_VPP_COLOR_CONVERSION) {
            for (j = 0; j < currFilter->NumMemTypes; j++) {
                mfxVPPDescription::filter::memdesc *currMemDesc =
                    &(currFilter->MemDesc[j]);
                for (k = 0; k < currMemDesc->NumInFormats; k++) {
                    mfxVPPDescription::filter::memdesc::format *currFormat =
                        &(currMemDesc->Formats[k]);
                    if (currFormat->InFormat == inFormat) {
                        for (n = 0; n < currFormat->NumOutFormat; n++) {
                            if (currFormat->OutFormats[n] == outFormat)
                                return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

mfxStatus InitNewDispatcher(WSType wsType,
                            Params *params,
                            mfxSession *session) {
    mfxStatus sts = MFX_ERR_NONE;
    *session      = nullptr;

    // load 2.0 dispatcher
    mfxLoader loader = MFXLoad();
    if (!loader) {
        printf("Error - MFXLoad() returned NULL\n");
        return MFX_ERR_UNSUPPORTED;
    }

    mfxVariant ImplValue;
    mfxConfig cfg;

    // basic filtering - test for SW implementation
    cfg                = MFXCreateConfig(loader);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
    MFXSetConfigFilterProperty(cfg,
                               (const mfxU8 *)"mfxImplDescription.Impl",
                               ImplValue);

    mfxU32 implIdx = 0;
    while (1) {
        // enumerate all implementations, check capabilities
        mfxImplDescription *implDesc;
        sts = MFXEnumImplementations(loader,
                                     implIdx,
                                     MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                     reinterpret_cast<mfxHDL *>(&implDesc));

        // out of range - we've tested all implementations
        if (sts == MFX_ERR_NOT_FOUND)
            break;

        bool isSupported = false;

        if (wsType == WSTYPE_DECODE) {
            isSupported = CheckDecoderImplCaps(implDesc, params->srcFourCC);
        }
        else if (wsType == WSTYPE_ENCODE) {
            isSupported = CheckEncoderImplCaps(implDesc,
                                               params->srcFourCC,
                                               params->dstFourCC);
        }
        else if (wsType == WSTYPE_VPP) {
            isSupported = CheckVPPImplCaps(implDesc,
                                           params->srcFourCC,
                                           params->dstFourCC);
        }

        if (isSupported) {
            // this implementation is capable of processing the stream
            sts = MFXCreateSession(loader, implIdx, session);
            if (sts != MFX_ERR_NONE) {
                printf("Error in MFXCreateSession, sts = %d", sts);
                return sts;
            }
            MFXDispReleaseImplDescription(loader, implDesc);
            break;
        }
        else {
            MFXDispReleaseImplDescription(loader, implDesc);
        }

        implIdx++;
    }

    return sts;
}

// debugging functions

#define TEST_CFG(type, dType, val)                                           \
    cfg                  = MFXCreateConfig(loader);                          \
    ImplValue.Type       = (type);                                           \
    ImplValue.Data.dType = (val);                                            \
    sts                  = MFXSetConfigFilterProperty(cfg, name, ImplValue); \
    printf("Test config: sts = %d, name = %s\n", sts, name);

static void TestCfgPropsMain(mfxLoader loader) {
    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;
    const mfxU8 *name;

    name = (const mfxU8 *)"mfxImplDescription.Impl";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_IMPL_TYPE_SOFTWARE);

    name = (const mfxU8 *)"mfxImplDescription.AccelerationMode";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 3);

    name = (const mfxU8 *)"mfxImplDescription.VendorID";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, 0xabcd);

    name = (const mfxU8 *)"mfxImplDescription.VendorImplID";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, 0x1234);
}

static void TestCfgPropsDec(mfxLoader loader) {
    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;
    const mfxU8 *name;

    name = (const mfxU8
                *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_CODEC_HEVC);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.MaxcodecLevel";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 54);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, 150);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.MemHandleType";
    TEST_CFG(MFX_VARIANT_TYPE_I32, I32, MFX_RESOURCE_SYSTEM_SURFACE);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.ColorFormats";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_FOURCC_I420);
}

static void TestCfgPropsEnc(mfxLoader loader) {
    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;
    const mfxU8 *name;

    name = (const mfxU8
                *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_CODEC_HEVC);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.MaxcodecLevel";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 54);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.BiDirectionalPrediction";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 1);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.encprofile.Profile";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, 150);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.MemHandleType";
    TEST_CFG(MFX_VARIANT_TYPE_I32, I32, MFX_RESOURCE_SYSTEM_SURFACE);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.ColorFormats";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_FOURCC_I420);
}

static void TestCfgPropsVPP(mfxLoader loader) {
    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;
    const mfxU8 *name;

    name = (const mfxU8
                *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_CODEC_HEVC);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxVPPDescription.filter.MaxDelayInFrames";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 3);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxVPPDescription.filter.memdesc.MemHandleType";
    TEST_CFG(MFX_VARIANT_TYPE_I32, I32, MFX_RESOURCE_SYSTEM_SURFACE);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxVPPDescription.filter.memdesc.format.OutFormats";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_FOURCC_I420);
}
