/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include <assert.h>

#include "src/dispatcher_common.h"

// these headers include the actual caps structure definitions, so only include once in vpl-tests
#include "src/caps_dec.h"
#include "src/caps_enc.h"
#include "src/caps_vpp.h"

// special codes for unit test logic
#define QUERY_NOT_FOUND 88888 // code to indicate index was not found
#define QUERY_ALL       99999 // code to indicate query for all dec/enc/vpp

#define QUERY_MAX_ITEMS 3 // set >= the largest number of codecs/filters in the stub RT's

enum {
    PROP_TYPE_IMPL_ONLY = 0,

    PROP_TYPE_ALL_DEC,
    PROP_TYPE_ALL_ENC,
    PROP_TYPE_ALL_VPP,

    PROP_TYPE_SINGLE_DEC,
    PROP_TYPE_SINGLE_ENC,
    PROP_TYPE_SINGLE_VPP,
};

typedef struct {
    mfxU32 numDec;
    mfxU32 listDec[QUERY_MAX_ITEMS];

    mfxU32 numEnc;
    mfxU32 listEnc[QUERY_MAX_ITEMS];

    mfxU32 numVPP;
    mfxU32 listVPP[QUERY_MAX_ITEMS];
} PropQueryTest;

// clang-format off
static const PropQueryTest propQueryTests[] = {
    { 0,         {}, 0,         {}, 0,         {}, },    // 0 - basic:all (shallow query, no codec info)

    { QUERY_ALL, {}, 0,         {}, 0,         {}, },    // 1 - dec:all
    { 0,         {}, QUERY_ALL, {}, 0,         {}, },    // 2 - enc:all
    { 0,         {}, 0,         {}, QUERY_ALL, {}, },    // 3 - vpp:all
    
    { QUERY_ALL, {}, QUERY_ALL, {}, 0,         {}, },    // 4 - dec:all + enc:all
    { QUERY_ALL, {}, 0,         {}, QUERY_ALL, {}, },    // 5 - dec:all + vpp:all
    { 0,         {}, QUERY_ALL, {}, QUERY_ALL, {}, },    // 6 - enc:all + vpp:all
    { QUERY_ALL, {}, QUERY_ALL, {}, QUERY_ALL, {}, },    // 7 - dec:all + enc:all + vpp:all

    {   // 8 - dec:mpeg2
        1, {MFX_CODEC_MPEG2},
        0, {}, 
        0, {},
    },
    {   // 9 - enc:avc
        0, {},
        1, {MFX_CODEC_AVC}, 
        0, {},
    },
    {   // 10 - dec:mpeg2 + enc:avc
        1, {MFX_CODEC_MPEG2},
        1, {MFX_CODEC_AVC}, 
        0, {},
    },
    {   // 11 - dec:mpeg2 + dec:av1 + enc:avc
        2, {MFX_CODEC_MPEG2, MFX_CODEC_AV1},
        1, {MFX_CODEC_AVC}, 
        0, {},
    },
    {   // 12 - vpp:deint
        0, {},
        0, {},
        1, {MFX_EXTBUFF_VPP_DEINTERLACING},
    },
    {   // 13 - dec:mpeg2 + vpp:deint
        1, {MFX_CODEC_MPEG2},
        0, {}, 
        1, {MFX_EXTBUFF_VPP_DEINTERLACING},
    },
    {   // 14 - dec:mpeg2 + enc:avc + vpp:deint
        1, {MFX_CODEC_MPEG2},
        1, {MFX_CODEC_AVC}, 
        1, {MFX_EXTBUFF_VPP_DEINTERLACING},
    },
    {   // 15 - dec:mpeg2 + vpp:deint + vpp:csc
        1, {MFX_CODEC_MPEG2},
        0, {}, 
        2, {MFX_EXTBUFF_VPP_DEINTERLACING, MFX_EXTBUFF_VPP_COLOR_CONVERSION},
    },
    {   // 16 - dec:mpeg2 + enc:all + vpp:csc
        1, {MFX_CODEC_MPEG2},
        QUERY_ALL, {}, 
        1, {MFX_EXTBUFF_VPP_COLOR_CONVERSION},
    },
    {   // 17 - dec:all + enc:hevc
        QUERY_ALL, {}, 
        1, {MFX_CODEC_HEVC},
        0, {},
    },
};

// tests which are expected to fail (codec not supported, etc.)
static const PropQueryTest propQueryTestsInvalid[] = {
    {   // 0
        1, {MFX_CODEC_AVC},
        0, {},
        0, {},
    },
    {   // 1
        0, {},
        1, {MFX_CODEC_VP9},
        0, {},
    },
    {   // 2
        0, {},
        0, {},
        1, {MFX_EXTBUFF_VPP_MIRRORING},
    },
    {   // 3
        1, {MFX_CODEC_VP8},
        QUERY_ALL, {},
        0, {},
    },
    {   // 4
        QUERY_ALL, {},
        1, {MFX_CODEC_VP8},
        QUERY_ALL, {},
    },
    {   // 5
        1, {MFX_CODEC_MPEG2},
        1, {MFX_CODEC_AVC},
        1, {MFX_EXTBUFF_VPP_MIRRORING},
    },
    {   // 6
        1, {MFX_CODEC_MPEG2},
        QUERY_ALL, {},
        2, {MFX_EXTBUFF_VPP_DEINTERLACING, MFX_EXTBUFF_VPP_MIRRORING},
    },
    {   // 7
        1, {MFX_CODEC_MPEG2},
        QUERY_ALL, {},
        2, {MFX_EXTBUFF_VPP_MIRRORING, MFX_EXTBUFF_VPP_DEINTERLACING},
    },
    {   // 8
        2, {MFX_CODEC_MPEG2, MFX_CODEC_VP9},
        0, {},
        0, {},
    },
    {   // 9
        2, {MFX_CODEC_MPEG2, MFX_CODEC_VP9},
        1, {MFX_CODEC_AVC},
        0, {},
    },
    {   // 90
        2, {MFX_CODEC_MPEG2, MFX_CODEC_VP9},
        1, {MFX_CODEC_AVC},
        QUERY_ALL, {},
    },
};
// clang-format on

#define NUM_TESTS_MULTIPLE         (sizeof(propQueryTests) / sizeof(PropQueryTest))
#define NUM_TESTS_MULTIPLE_INVALID (sizeof(propQueryTestsInvalid) / sizeof(PropQueryTest))

#ifdef ONEVPL_EXPERIMENTAL

static void SetQueryProp(mfxLoader loader, mfxU32 propType, mfxU32 propVal = 0) {
    mfxConfig cfg = MFXCreateConfig(loader);

    // query property - all decoders
    std::string propStr;
    switch (propType) {
        // shallow query
        case PROP_TYPE_IMPL_ONLY:
            propStr = "mfxImplDescription";
            break;

        // query all codecs/filters
        case PROP_TYPE_ALL_DEC:
            propStr = "mfxImplDescription.mfxDecoderDescription";
            break;
        case PROP_TYPE_ALL_ENC:
            propStr = "mfxImplDescription.mfxEncoderDescription";
            break;
        case PROP_TYPE_ALL_VPP:
            propStr = "mfxImplDescription.mfxVPPDescription";
            break;

        // query single codecs/filter
        case PROP_TYPE_SINGLE_DEC:
            propStr = "mfxImplDescription.mfxDecoderDescription.decoder.CodecID";
            break;
        case PROP_TYPE_SINGLE_ENC:
            propStr = "mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
            break;
        case PROP_TYPE_SINGLE_VPP:
            propStr = "mfxImplDescription.mfxVPPDescription.filter.FilterFourCC";
            break;

        default:
            break;
    }

    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
    var.Data.U32 = propVal;

    mfxStatus sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_NONE);
}

static void CheckImpl(mfxImplDescription *testDesc, bool bCheckFallback = false) {
    // check top-level implDesc info - see stub config.cpp for values
    // not necessary to check every single field in stub RT here - just make sure it's getting filled out
    EXPECT_EQ(testDesc->Version.Major, 1);
    EXPECT_EQ(testDesc->Version.Minor, 2);
    EXPECT_EQ(testDesc->Impl, MFX_IMPL_TYPE_SOFTWARE);
    EXPECT_EQ(testDesc->AccelerationMode, MFX_ACCEL_MODE_NA);
    EXPECT_EQ(testDesc->ApiVersion.Major, MFX_VERSION_MAJOR);
    EXPECT_EQ(testDesc->ApiVersion.Minor, MFX_VERSION_MINOR);
    EXPECT_EQ(testDesc->VendorID, 0x8086);
    EXPECT_EQ(testDesc->VendorImplID, 0xFFFF);

    if (bCheckFallback)
        EXPECT_EQ(testDesc->ImplName, std::string("Stub Implementation - no fn"));
    else
        EXPECT_EQ(testDesc->ImplName, std::string("Stub Implementation"));
}

static mfxI32 FindDecIdx(const mfxDecoderDescription *desc, mfxU32 expectedCodec) {
    for (mfxU32 idx = 0; idx < desc->NumCodecs; idx++) {
        if (desc->Codecs[idx].CodecID == expectedCodec)
            return idx;
    }

    return QUERY_NOT_FOUND;
}

static mfxI32 FindEncIdx(const mfxEncoderDescription *desc, mfxU32 expectedCodec) {
    for (mfxU32 idx = 0; idx < desc->NumCodecs; idx++) {
        if (desc->Codecs[idx].CodecID == expectedCodec)
            return idx;
    }

    return QUERY_NOT_FOUND;
}

static mfxI32 FindVPPIdx(const mfxVPPDescription *desc, mfxU32 expectedFilter) {
    for (mfxU32 idx = 0; idx < desc->NumFilters; idx++) {
        if (desc->Filters[idx].FilterFourCC == expectedFilter)
            return idx;
    }

    return QUERY_NOT_FOUND;
}

// check number and names of supported decoders
static void CheckDecode(mfxDecoderDescription *testDesc,
                        mfxU32 numCodecs,
                        const mfxU32 *codecs,
                        bool bCheckFallback = false) {
    mfxU32 expectedNumCodecs               = 0;
    mfxU32 expectedCodecs[QUERY_MAX_ITEMS] = {};

    // generate the list of codecs we expect to see in the description
    if (numCodecs == QUERY_ALL || bCheckFallback) {
        expectedNumCodecs = decoderDesc.NumCodecs;
        for (mfxU32 idx = 0; idx < decoderDesc.NumCodecs; idx++)
            expectedCodecs[idx] = decoderDesc.Codecs[idx].CodecID;
    }
    else {
        expectedNumCodecs = numCodecs;
        for (mfxU32 idx = 0; idx < numCodecs; idx++)
            expectedCodecs[idx] = codecs[idx];
    }

    // check that the correct number of codec descriptions was returned
    EXPECT_EQ(testDesc->NumCodecs, expectedNumCodecs);

    // check that the correct codec descriptions were returned and match the stub RT definitions
    for (mfxU32 idx = 0; idx < expectedNumCodecs; idx++) {
        // find index of the expected codec in both the test and reference descriptions
        mfxU32 testIdx = FindDecIdx(testDesc, expectedCodecs[idx]);
        EXPECT_NE(testIdx, QUERY_NOT_FOUND);

        mfxU32 refIdx = FindDecIdx(&decoderDesc, expectedCodecs[idx]);
        EXPECT_NE(refIdx, QUERY_NOT_FOUND);

        // test failed - exit here to avoid using invalid array index
        if (testIdx == QUERY_NOT_FOUND || refIdx == QUERY_NOT_FOUND)
            return;

        DecCodec *testDec = &(testDesc->Codecs[testIdx]);
        DecCodec *refDec  = &(decoderDesc.Codecs[refIdx]);

        // compare a few of the values in the decoder description
        EXPECT_EQ(testDec->CodecID, refDec->CodecID);
        EXPECT_EQ(testDec->MaxcodecLevel, refDec->MaxcodecLevel);
        EXPECT_EQ(testDec->NumProfiles, refDec->NumProfiles);
    }
}

static void CheckEncode(mfxEncoderDescription *testDesc,
                        mfxU32 numCodecs,
                        const mfxU32 *codecs,
                        bool bCheckFallback = false) {
    mfxU32 expectedNumCodecs               = 0;
    mfxU32 expectedCodecs[QUERY_MAX_ITEMS] = {};

    // generate the list of codecs we expect to see in the description
    if (numCodecs == QUERY_ALL || bCheckFallback) {
        expectedNumCodecs = encoderDesc.NumCodecs;
        for (mfxU32 idx = 0; idx < encoderDesc.NumCodecs; idx++)
            expectedCodecs[idx] = encoderDesc.Codecs[idx].CodecID;
    }
    else {
        expectedNumCodecs = numCodecs;
        for (mfxU32 idx = 0; idx < numCodecs; idx++)
            expectedCodecs[idx] = codecs[idx];
    }

    // check that the correct number of codec descriptions was returned
    EXPECT_EQ(testDesc->NumCodecs, expectedNumCodecs);

    // check that the correct codec descriptions were returned and match the stub RT definitions
    for (mfxU32 idx = 0; idx < expectedNumCodecs; idx++) {
        // find index of the expected codec in both the test and reference descriptions
        mfxU32 testIdx = FindEncIdx(testDesc, expectedCodecs[idx]);
        EXPECT_NE(testIdx, QUERY_NOT_FOUND);

        mfxU32 refIdx = FindEncIdx(&encoderDesc, expectedCodecs[idx]);
        EXPECT_NE(refIdx, QUERY_NOT_FOUND);

        // test failed - exit here to avoid using invalid array index
        if (testIdx == QUERY_NOT_FOUND || refIdx == QUERY_NOT_FOUND)
            return;

        EncCodec *testEnc = &(testDesc->Codecs[testIdx]);
        EncCodec *refEnc  = &(encoderDesc.Codecs[refIdx]);

        // compare a few of the values in the encoder description
        EXPECT_EQ(testEnc->CodecID, refEnc->CodecID);
        EXPECT_EQ(testEnc->MaxcodecLevel, refEnc->MaxcodecLevel);
        EXPECT_EQ(testEnc->BiDirectionalPrediction, refEnc->BiDirectionalPrediction);
        EXPECT_EQ(testEnc->NumProfiles, refEnc->NumProfiles);
    }
}

static void CheckVPP(mfxVPPDescription *testDesc,
                     mfxU32 numFilters,
                     const mfxU32 *filters,
                     bool bCheckFallback = false) {
    mfxU32 expectedNumFilters               = 0;
    mfxU32 expectedFilters[QUERY_MAX_ITEMS] = {};

    // generate the list of codecs we expect to see in the description
    if (numFilters == QUERY_ALL || bCheckFallback) {
        expectedNumFilters = vppDesc.NumFilters;
        for (mfxU32 idx = 0; idx < vppDesc.NumFilters; idx++)
            expectedFilters[idx] = vppDesc.Filters[idx].FilterFourCC;
    }
    else {
        expectedNumFilters = numFilters;
        for (mfxU32 idx = 0; idx < numFilters; idx++)
            expectedFilters[idx] = filters[idx];
    }

    // check that the correct number of codec descriptions was returned
    EXPECT_EQ(testDesc->NumFilters, expectedNumFilters);

    // check that the correct codec descriptions were returned and match the stub RT definitions
    for (mfxU32 idx = 0; idx < expectedNumFilters; idx++) {
        // find index of the expected codec in both the test and reference descriptions
        mfxU32 testIdx = FindVPPIdx(testDesc, expectedFilters[idx]);
        EXPECT_NE(testIdx, QUERY_NOT_FOUND);

        mfxU32 refIdx = FindVPPIdx(&vppDesc, expectedFilters[idx]);
        EXPECT_NE(refIdx, QUERY_NOT_FOUND);

        // test failed - exit here to avoid using invalid array index
        if (testIdx == QUERY_NOT_FOUND || refIdx == QUERY_NOT_FOUND)
            return;

        VPPFilter *testVPP = &(testDesc->Filters[testIdx]);
        VPPFilter *refVPP  = &(vppDesc.Filters[refIdx]);

        // compare a few of the values in the VPP description
        EXPECT_EQ(testVPP->FilterFourCC, refVPP->FilterFourCC);
        EXPECT_EQ(testVPP->MaxDelayInFrames, refVPP->MaxDelayInFrames);
        EXPECT_EQ(testVPP->NumMemTypes, refVPP->NumMemTypes);
    }
}

static mfxStatus TestMultipleQueries(const PropQueryTest *pqt,
                                     mfxU32 implType,
                                     bool bCreateSession = false,
                                     bool bExpectFail    = false) {
    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, implType);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // shallow query only - no codecs/filters
    if (pqt->numDec == 0 && pqt->numEnc == 0 && pqt->numVPP == 0)
        SetQueryProp(loader, PROP_TYPE_IMPL_ONLY);

    // decoder query
    if (pqt->numDec == QUERY_ALL) {
        SetQueryProp(loader, PROP_TYPE_ALL_DEC);
    }
    else {
        for (mfxU32 idx = 0; idx < pqt->numDec; idx++)
            SetQueryProp(loader, PROP_TYPE_SINGLE_DEC, pqt->listDec[idx]);
    }

    // encoder query
    if (pqt->numEnc == QUERY_ALL) {
        SetQueryProp(loader, PROP_TYPE_ALL_ENC);
    }
    else {
        for (mfxU32 idx = 0; idx < pqt->numEnc; idx++)
            SetQueryProp(loader, PROP_TYPE_SINGLE_ENC, pqt->listEnc[idx]);
    }

    // VPP query
    if (pqt->numVPP == QUERY_ALL) {
        SetQueryProp(loader, PROP_TYPE_ALL_VPP);
    }
    else {
        for (mfxU32 idx = 0; idx < pqt->numVPP; idx++)
            SetQueryProp(loader, PROP_TYPE_SINGLE_VPP, pqt->listVPP[idx]);
    }

    mfxSession session           = nullptr;
    mfxImplDescription *implDesc = nullptr;

    if (bCreateSession) {
        // create a stub session without checking caps
        sts = MFXCreateSession(loader, 0, &session);
    }
    else {
        // enumerate implementations, check capabilities of first one
        sts = MFXEnumImplementations(loader,
                                     0,
                                     MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                     reinterpret_cast<mfxHDL *>(&implDesc));
    }

    if (bExpectFail) {
        // check for appropriate error code, free internal resources, exit test
        EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);
        MFXUnload(loader);
        return MFX_ERR_NONE;
    }
    else {
        EXPECT_EQ(sts, MFX_ERR_NONE);
    }

    if (bCreateSession) {
        sts = MFXClose(session);
        EXPECT_EQ(sts, MFX_ERR_NONE);
    }
    else {
        // _NOFN stub does not support props-based query, so we should fallback to full query
        bool bCheckFallback = false;
        if (implType == MFX_IMPL_TYPE_STUB_NOFN)
            bCheckFallback = true;

        // check top-level implDesc info
        CheckImpl(implDesc, bCheckFallback);

        // check number and names of supported decoders
        CheckDecode(&implDesc->Dec, pqt->numDec, pqt->listDec, bCheckFallback);

        // check number and names of supported encoders
        CheckEncode(&implDesc->Enc, pqt->numEnc, pqt->listEnc, bCheckFallback);

        // check number and names of supported VPP filters
        CheckVPP(&implDesc->VPP, pqt->numVPP, pqt->listVPP, bCheckFallback);

        sts = MFXDispReleaseImplDescription(loader, implDesc);
        EXPECT_EQ(sts, MFX_ERR_NONE);
    }

    // free internal resources
    MFXUnload(loader);

    return MFX_ERR_NONE;
}

    // run tests with stub RT which supports MFXQueryImplsProperties()
    #define TEST_PROPQUERY_VALID(idx)                                            \
        TEST(Dispatcher_Stub_PropQuery, ValidEnumImpls_##idx) {                  \
            SKIP_IF_DISP_STUB_DISABLED();                                        \
            TestMultipleQueries(&propQueryTests[idx], MFX_IMPL_TYPE_STUB);       \
        }                                                                        \
        TEST(Dispatcher_Stub_PropQuery, ValidCreateSession_##idx) {              \
            SKIP_IF_DISP_STUB_DISABLED();                                        \
            TestMultipleQueries(&propQueryTests[idx], MFX_IMPL_TYPE_STUB, true); \
        }

    // run the same tests with the 'nofn' stub, which will fallback to full query
    #define TEST_PROPQUERY_VALID_NOFN(idx)                                            \
        TEST(Dispatcher_Stub_PropQuery, ValidEnumImplsNoFN_##idx) {                   \
            SKIP_IF_DISP_STUB_DISABLED();                                             \
            TestMultipleQueries(&propQueryTests[idx], MFX_IMPL_TYPE_STUB_NOFN);       \
        }                                                                             \
                                                                                      \
        TEST(Dispatcher_Stub_PropQuery, ValidCreateSessionNoFN_##idx) {               \
            SKIP_IF_DISP_STUB_DISABLED();                                             \
            TestMultipleQueries(&propQueryTests[idx], MFX_IMPL_TYPE_STUB_NOFN, true); \
        }

    // run tests which are expected to fail
    #define TEST_PROPQUERY_INVALID(idx)                                                        \
        TEST(Dispatcher_Stub_PropQuery, InvalidEnumImpls_##idx) {                              \
            SKIP_IF_DISP_STUB_DISABLED();                                                      \
            TestMultipleQueries(&propQueryTestsInvalid[idx], MFX_IMPL_TYPE_STUB, false, true); \
        }                                                                                      \
                                                                                               \
        TEST(Dispatcher_Stub_PropQuery, InvalidCreateSession_##idx) {                          \
            SKIP_IF_DISP_STUB_DISABLED();                                                      \
            TestMultipleQueries(&propQueryTestsInvalid[idx], MFX_IMPL_TYPE_STUB, true, true);  \
        }

TEST_PROPQUERY_VALID(0)
TEST_PROPQUERY_VALID(1)
TEST_PROPQUERY_VALID(2)
TEST_PROPQUERY_VALID(3)
TEST_PROPQUERY_VALID(4)
TEST_PROPQUERY_VALID(5)
TEST_PROPQUERY_VALID(6)
TEST_PROPQUERY_VALID(7)

TEST_PROPQUERY_VALID(8)
TEST_PROPQUERY_VALID(9)
TEST_PROPQUERY_VALID(10)
TEST_PROPQUERY_VALID(11)
TEST_PROPQUERY_VALID(12)
TEST_PROPQUERY_VALID(13)
TEST_PROPQUERY_VALID(14)
TEST_PROPQUERY_VALID(15)
TEST_PROPQUERY_VALID(16)
TEST_PROPQUERY_VALID(17)

// confirm that we ran all of the tests - error at compile time if mismatch
static_assert((NUM_TESTS_MULTIPLE) == 18,
              "Unexpected number of tests created: Dispatcher_Stub_PropQuery.EnumImplsValid");

TEST_PROPQUERY_VALID_NOFN(0)
TEST_PROPQUERY_VALID_NOFN(1)
TEST_PROPQUERY_VALID_NOFN(2)
TEST_PROPQUERY_VALID_NOFN(3)
TEST_PROPQUERY_VALID_NOFN(4)
TEST_PROPQUERY_VALID_NOFN(5)
TEST_PROPQUERY_VALID_NOFN(6)
TEST_PROPQUERY_VALID_NOFN(7)

TEST_PROPQUERY_VALID_NOFN(8)
TEST_PROPQUERY_VALID_NOFN(9)
TEST_PROPQUERY_VALID_NOFN(10)
TEST_PROPQUERY_VALID_NOFN(11)
TEST_PROPQUERY_VALID_NOFN(12)
TEST_PROPQUERY_VALID_NOFN(13)
TEST_PROPQUERY_VALID_NOFN(14)
TEST_PROPQUERY_VALID_NOFN(15)
TEST_PROPQUERY_VALID_NOFN(16)
TEST_PROPQUERY_VALID_NOFN(17)

// confirm that we ran all of the tests - error at compile time if mismatch
static_assert((NUM_TESTS_MULTIPLE) == 18,
              "Unexpected number of tests created: Dispatcher_Stub_PropQuery.EnumImplsValidNoFN");

TEST_PROPQUERY_INVALID(0)
TEST_PROPQUERY_INVALID(1)
TEST_PROPQUERY_INVALID(2)
TEST_PROPQUERY_INVALID(3)
TEST_PROPQUERY_INVALID(4)
TEST_PROPQUERY_INVALID(5)
TEST_PROPQUERY_INVALID(6)
TEST_PROPQUERY_INVALID(7)

TEST_PROPQUERY_INVALID(8)
TEST_PROPQUERY_INVALID(9)
TEST_PROPQUERY_INVALID(10)

// confirm that we ran all of the tests - error at compile time if mismatch
static_assert((NUM_TESTS_MULTIPLE_INVALID) == 11,
              "Unexpected number of tests created: Dispatcher_Stub_PropQuery.EnumImplsInvalid");

TEST(Dispatcher_Stub_PropQuery, SetMultiplePropsSingleConfig) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // set more than one query prop in the same mfxConfig object
    // this isn't the recommended approach since e.g. you can only specify one decoder or encoder per object,
    //   but it should still work if there are no conflicts
    mfxConfig cfg = MFXCreateConfig(loader);

    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;

    // query property - all decoders
    std::string propStr = "mfxImplDescription.mfxDecoderDescription";
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
    var.Data.U32 = 0;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // request these in two different mfxConfig objects
    mfxU32 numEnc     = 2;
    mfxU32 listEnc[2] = { MFX_CODEC_AVC, MFX_CODEC_HEVC };

    // query property - one encoder, same mfxConfig
    propStr      = "mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
    var.Data.U32 = listEnc[0];

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // now create a new mfxConfig object, and request the second encoder
    mfxConfig cfg2 = MFXCreateConfig(loader);

    propStr      = "mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
    var.Data.U32 = listEnc[1];

    sts = MFXSetConfigFilterProperty(cfg2, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // also request all VPP in the second mfxConfig
    propStr      = "mfxImplDescription.mfxVPPDescription";
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
    var.Data.U32 = 0;

    sts = MFXSetConfigFilterProperty(cfg2, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxImplDescription *implDesc = nullptr;

    // enumerate implementations, check capabilities of first one
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check top-level implDesc info
    CheckImpl(implDesc);

    // check number and names of supported codecs/filters
    CheckDecode(&implDesc->Dec, QUERY_ALL, nullptr);
    CheckEncode(&implDesc->Enc, numEnc, listEnc);
    CheckVPP(&implDesc->VPP, QUERY_ALL, nullptr);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, SetPropMissingFlagImplOnly) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);

    // query property - impl only
    std::string propStr = "mfxImplDescription";

    // do not OR with MFX_VARIANT_TYPE_QUERY
    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;
    var.Type            = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32);
    var.Data.U32        = 0;

    // expect error because this property is only valid with MFX_VARIANT_TYPE_QUERY set
    mfxStatus sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, SetPropMissingFlagAllDec) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);

    // query property - all decoders
    std::string propStr = "mfxImplDescription.mfxDecoderDescription";

    // do not OR with MFX_VARIANT_TYPE_QUERY
    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;
    var.Type            = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32);
    var.Data.U32        = 0;

    // expect error because this property is only valid with MFX_VARIANT_TYPE_QUERY set
    mfxStatus sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, SetPropMissingFlagAllEnc) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);

    // query property - all encoders
    std::string propStr = "mfxImplDescription.mfxEncoderDescription";

    // do not OR with MFX_VARIANT_TYPE_QUERY
    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;
    var.Type            = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32);
    var.Data.U32        = 0;

    // expect error because this property is only valid with MFX_VARIANT_TYPE_QUERY set
    mfxStatus sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, SetPropMissingFlagAllVPP) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);

    // query property - all VPP
    std::string propStr = "mfxImplDescription.mfxVPPDescription";

    // do not OR with MFX_VARIANT_TYPE_QUERY
    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;
    var.Type            = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32);
    var.Data.U32        = 0;

    // expect error because this property is only valid with MFX_VARIANT_TYPE_QUERY set
    mfxStatus sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, SetPropQueryFlagUnsupported) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxConfig cfg = MFXCreateConfig(loader);

    // query property - impl only
    std::string propStr = "mfxImplDescription.Impl";

    // OR an unsupported property type with MFX_VARIANT_TYPE_QUERY
    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
    var.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;

    // expect error because this property is only valid with MFX_VARIANT_TYPE_QUERY set
    mfxStatus sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_UNSUPPORTED);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, EnumBeforeSession) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // decoder query
    mfxU32 numDec     = 1;
    mfxU32 listDec[1] = { MFX_CODEC_MPEG2 };
    for (mfxU32 idx = 0; idx < numDec; idx++)
        SetQueryProp(loader, PROP_TYPE_SINGLE_DEC, listDec[idx]);

    mfxSession session           = nullptr;
    mfxImplDescription *implDesc = nullptr;

    // enumerate implementations, check capabilities of first one
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check top-level implDesc info
    CheckImpl(implDesc);

    // check number and names of supported decoders
    CheckDecode(&implDesc->Dec, numDec, listDec);

    // should be no Enc or VPP reported
    CheckEncode(&implDesc->Enc, 0, nullptr);
    CheckVPP(&implDesc->VPP, 0, nullptr);

    // create a stub session
    sts = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, SessionBeforeEnum) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSession session           = nullptr;
    mfxImplDescription *implDesc = nullptr;

    // create a stub session
    sts = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // decoder query
    mfxU32 numDec     = 1;
    mfxU32 listDec[1] = { MFX_CODEC_MPEG2 };
    for (mfxU32 idx = 0; idx < numDec; idx++)
        SetQueryProp(loader, PROP_TYPE_SINGLE_DEC, listDec[idx]);

    // enumerate implementations, check capabilities of first one
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check top-level implDesc info
    CheckImpl(implDesc);

    // because MFXCreateSession() was called first, dispatcher did a full query before the props-based query request was received
    CheckDecode(&implDesc->Dec, QUERY_ALL, nullptr);
    CheckEncode(&implDesc->Enc, QUERY_ALL, nullptr);
    CheckVPP(&implDesc->VPP, QUERY_ALL, nullptr);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, EnumBeforeAndAfterSession) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // decoder query
    mfxU32 numDec     = 1;
    mfxU32 listDec[1] = { MFX_CODEC_MPEG2 };
    for (mfxU32 idx = 0; idx < numDec; idx++)
        SetQueryProp(loader, PROP_TYPE_SINGLE_DEC, listDec[idx]);

    mfxSession session           = nullptr;
    mfxImplDescription *implDesc = nullptr;

    // enumerate implementations, check capabilities of first one
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check top-level implDesc info
    CheckImpl(implDesc);

    // check number and names of supported decoders
    CheckDecode(&implDesc->Dec, numDec, listDec);

    // should be no Enc or VPP reported
    CheckEncode(&implDesc->Enc, 0, nullptr);
    CheckVPP(&implDesc->VPP, 0, nullptr);

    // create a stub session
    sts = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // repeat enumerate implementations, results should be the same as before
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check top-level implDesc info
    CheckImpl(implDesc);

    // check number and names of supported decoders
    CheckDecode(&implDesc->Dec, numDec, listDec);

    // should be no Enc or VPP reported
    CheckEncode(&implDesc->Enc, 0, nullptr);
    CheckVPP(&implDesc->VPP, 0, nullptr);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, ChangePropsSameLoader) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // decoder query
    mfxU32 numDec     = 1;
    mfxU32 listDec[1] = { MFX_CODEC_MPEG2 };
    for (mfxU32 idx = 0; idx < numDec; idx++)
        SetQueryProp(loader, PROP_TYPE_SINGLE_DEC, listDec[idx]);

    mfxImplDescription *implDesc = nullptr;

    // enumerate implementations, check capabilities of first one
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check top-level implDesc info
    CheckImpl(implDesc);

    // check number and names of supported decoders
    CheckDecode(&implDesc->Dec, numDec, listDec);

    // should be no Enc or VPP reported
    CheckEncode(&implDesc->Enc, 0, nullptr);
    CheckVPP(&implDesc->VPP, 0, nullptr);

    // change query to a different (supported) codec
    // the query will not be repeated (RT query is only done once per MFXLoad sequence) so we will not get any new info here
    // the new codec also acts as a filter property, so MFXEnumImplementations will now return MFX_ERR_NOT_FOUND because implDesc does not include the new codec
    mfxU32 numDecNew     = 1;
    mfxU32 listDecNew[1] = { MFX_CODEC_AV1 };
    for (mfxU32 idx = 0; idx < numDecNew; idx++)
        SetQueryProp(loader, PROP_TYPE_SINGLE_DEC, listDecNew[idx]);

    // repeat enumerate implementations, results should be the same as before
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, ChangePropsNewLoader) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // decoder query
    mfxU32 numDec     = 1;
    mfxU32 listDec[1] = { MFX_CODEC_MPEG2 };
    for (mfxU32 idx = 0; idx < numDec; idx++)
        SetQueryProp(loader, PROP_TYPE_SINGLE_DEC, listDec[idx]);

    mfxImplDescription *implDesc = nullptr;

    // enumerate implementations, check capabilities of first one
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check top-level implDesc info
    CheckImpl(implDesc);

    // check number and names of supported decoders
    CheckDecode(&implDesc->Dec, numDec, listDec);

    // should be no Enc or VPP reported
    CheckEncode(&implDesc->Enc, 0, nullptr);
    CheckVPP(&implDesc->VPP, 0, nullptr);

    // unload this loader and create a new one so that we can query again with different props
    MFXUnload(loader);
    loader = nullptr;

    loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // change query to a different (supported) codec
    // this is a new loader object, so we can query for something else
    mfxU32 numDecNew     = 1;
    mfxU32 listDecNew[1] = { MFX_CODEC_AV1 };
    for (mfxU32 idx = 0; idx < numDec; idx++)
        SetQueryProp(loader, PROP_TYPE_SINGLE_DEC, listDecNew[idx]);

    // repeat enumerate implementations, results should be the same as before
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check top-level implDesc info
    CheckImpl(implDesc);

    // check number and names of supported decoders (from the NEW list)
    CheckDecode(&implDesc->Dec, numDecNew, listDecNew);

    // should be no Enc or VPP reported
    CheckEncode(&implDesc->Enc, 0, nullptr);
    CheckVPP(&implDesc->VPP, 0, nullptr);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, OverwriteProps_SingleConfig_Enable) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg = MFXCreateConfig(loader);

    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;

    // first set normal CodecID filter prop
    std::string propStr = "mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
    var.Type            = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32);
    var.Data.U32        = MFX_CODEC_AVC;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // now overwrite with a different codec, enabling props-based query (same mfxConfig object)
    propStr      = "mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
    var.Data.U32 = MFX_CODEC_HEVC;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxImplDescription *implDesc = nullptr;

    // enumerate implementations, check capabilities of first one - should succeed
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check top-level implDesc info
    CheckImpl(implDesc);

    // only the second codec should be present
    mfxU32 numEnc     = 1;
    mfxU32 listEnc[1] = { MFX_CODEC_HEVC };
    CheckEncode(&implDesc->Enc, numEnc, listEnc);

    CheckDecode(&implDesc->Dec, 0, nullptr);
    CheckVPP(&implDesc->VPP, 0, nullptr);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, OverwriteProps_SingleConfig_Disable) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg = MFXCreateConfig(loader);

    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;

    // first set CodecID with props-based query
    std::string propStr = "mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
    var.Data.U32 = MFX_CODEC_AVC;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // now overwrite with a normal filter (same mfxConfig object)
    propStr      = "mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32);
    var.Data.U32 = MFX_CODEC_HEVC;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxImplDescription *implDesc = nullptr;

    // enumerate implementations, check capabilities of first one - should succeed
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check top-level implDesc info
    CheckImpl(implDesc);

    // all codecs should be present because we overwrote the props query with a full query
    CheckDecode(&implDesc->Dec, QUERY_ALL, nullptr);
    CheckEncode(&implDesc->Enc, QUERY_ALL, nullptr);
    CheckVPP(&implDesc->VPP, QUERY_ALL, nullptr);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, OverwriteProps_MultipleConfigs_Enable) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg = MFXCreateConfig(loader);

    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;

    // first set normal CodecID filter prop
    std::string propStr = "mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
    var.Type            = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32);
    var.Data.U32        = MFX_CODEC_AVC;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg2 = MFXCreateConfig(loader);

    // now overwrite with a different codec, enabling props-based query (new mfxConfig object)
    propStr      = "mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
    var.Data.U32 = MFX_CODEC_HEVC;

    sts = MFXSetConfigFilterProperty(cfg2, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxImplDescription *implDesc = nullptr;

    // enumerate implementations - this is expected to fail because the first codec was a normal filter,
    //   but we ran a props-based query due to the second config object which only requested support for the second codec
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, OverwriteProps_MultipleConfigs_Disable) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg = MFXCreateConfig(loader);

    mfxVariant var      = {};
    var.Version.Version = MFX_VARIANT_VERSION;

    // first set CodecID with props-based query
    std::string propStr = "mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32 | MFX_VARIANT_TYPE_QUERY);
    var.Data.U32 = MFX_CODEC_AVC;

    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxConfig cfg2 = MFXCreateConfig(loader);

    // now add a normal filter (new mfxConfig object)
    propStr      = "mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
    var.Type     = static_cast<mfxVariantType>(MFX_VARIANT_TYPE_U32);
    var.Data.U32 = MFX_CODEC_HEVC;

    sts = MFXSetConfigFilterProperty(cfg2, (const mfxU8 *)(propStr.c_str()), var);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxImplDescription *implDesc = nullptr;

    // enumerate implementations - this is expected to fail because the props-based query is still enabled
    //   from the first mfxConfig object, so a full query will not happen and the second codec is not supported
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, RepeatSameImpl) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // request impl description twice - runtime should only report once
    SetQueryProp(loader, PROP_TYPE_IMPL_ONLY);
    SetQueryProp(loader, PROP_TYPE_IMPL_ONLY);

    mfxImplDescription *implDesc = nullptr;

    // enumerate implementations, check capabilities of first one
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check top-level implDesc info
    CheckImpl(implDesc);

    // should be no Dec, Enc, or VPP reported
    CheckDecode(&implDesc->Dec, 0, nullptr);
    CheckEncode(&implDesc->Enc, 0, nullptr);
    CheckVPP(&implDesc->VPP, 0, nullptr);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_Stub_PropQuery, RepeatSameDecoder) {
    SKIP_IF_DISP_STUB_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_STUB);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // request same decoder twice - runtime should only report once
    mfxU32 numDec     = 2;
    mfxU32 listDec[2] = { MFX_CODEC_MPEG2, MFX_CODEC_MPEG2 };
    for (mfxU32 idx = 0; idx < numDec; idx++)
        SetQueryProp(loader, PROP_TYPE_SINGLE_DEC, listDec[idx]);

    mfxImplDescription *implDesc = nullptr;

    // enumerate implementations, check capabilities of first one
    sts = MFXEnumImplementations(loader,
                                 0,
                                 MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                 reinterpret_cast<mfxHDL *>(&implDesc));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // check top-level implDesc info
    CheckImpl(implDesc);

    // check number and names of supported decoders
    mfxU32 numDec2     = 1;
    mfxU32 listDec2[1] = { MFX_CODEC_MPEG2 };
    CheckDecode(&implDesc->Dec, numDec2, listDec2);

    // should be no Enc or VPP reported
    CheckEncode(&implDesc->Enc, 0, nullptr);
    CheckVPP(&implDesc->VPP, 0, nullptr);

    // free internal resources
    MFXUnload(loader);
}

#endif // ONEVPL_EXPERIMENTAL
