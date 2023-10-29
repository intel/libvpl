/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "src/dispatcher_common.h"

// leave table formatting alone
// clang-format off

// MFXSetConfigFilterProperty - multiple props per cfg object

// create loader, set config type, allocate and return an unused mfxConfig object
//   which the caller can set one or more filter props with
static mfxStatus MultiPropInit(mfxImplType implType, mfxLoader *loader, mfxConfig *cfg, bool bRequire2xGPU = false) {
    mfxStatus sts = MFX_ERR_NONE;

    *loader = MFXLoad();
    EXPECT_FALSE(*loader == nullptr);

    sts = SetConfigImpl(*loader, implType, bRequire2xGPU);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    *cfg = MFXCreateConfig(*loader);
    EXPECT_NE(cfg, nullptr);

    return sts;
}

// try creating session with the loader after configuring filters
// may be expected to succeed or fail (set stsExpected accordingly)
static mfxStatus MultiPropTest(mfxLoader loader, mfxStatus stsExpected) {
    mfxStatus sts = MFX_ERR_NONE;

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, stsExpected);

    if (stsExpected == MFX_ERR_NONE) {
        // test is expected to pass (create a valid session)
        EXPECT_NE(session, nullptr);
        sts = MFXClose(session);
        EXPECT_EQ(sts, MFX_ERR_NONE);
    }

    // free internal resources
    MFXUnload(loader);

    return sts;
}

void Dispatcher_MultiProp_DecEncValid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_AVC);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxEncoderDescription.encoder.CodecID", MFX_CODEC_HEVC);

    // expect to pass
    MultiPropTest(loader, MFX_ERR_NONE);
}

void Dispatcher_MultiProp_DecEncInvalid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // pass something invalid first
    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxEncoderDescription.encoder.CodecID", MFX_CODEC_VC1);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_AVC);

    // expect to fail
    MultiPropTest(loader, MFX_ERR_NOT_FOUND);
}

void Dispatcher_MultiProp_APIMajorMinorValid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxVersion ver = {};
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        ver.Major = CPU_VERSION_MAJOR;
        ver.Minor = CPU_VERSION_MINOR;
    }
    else {
        // MSDK may have 1.x API
        ver.Major = 1;
        ver.Minor = 0;
    }

    SetConfigFilterProperty<mfxU16>(loader, cfg, "mfxImplDescription.ApiVersion.Major", ver.Major);

    SetConfigFilterProperty<mfxU16>(loader, cfg, "mfxImplDescription.ApiVersion.Minor", ver.Minor);

    // expect to pass
    MultiPropTest(loader, MFX_ERR_NONE);
}

void Dispatcher_MultiProp_APIMajorInvalid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxVersion ver = {};
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        ver.Major = CPU_VERSION_MAJOR + 1;
        ver.Minor = CPU_VERSION_MINOR;
    }
    else {
        // MSDK may have 1.x API
        ver.Major = 1000;
        ver.Minor = 0;
    }

    SetConfigFilterProperty<mfxU16>(loader, cfg, "mfxImplDescription.ApiVersion.Major", ver.Major);

    SetConfigFilterProperty<mfxU16>(loader, cfg, "mfxImplDescription.ApiVersion.Minor", ver.Minor);

    // expect to fail
    MultiPropTest(loader, MFX_ERR_NOT_FOUND);
}

void Dispatcher_MultiProp_APIMinorInvalid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxVersion ver = {};
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        ver.Major = CPU_VERSION_MAJOR;
        ver.Minor = CPU_VERSION_MINOR + 1;
    }
    else {
        ver.Major = MFX_VERSION_MAJOR;
        ver.Minor = 1000;
    }

    SetConfigFilterProperty<mfxU16>(loader, cfg, "mfxImplDescription.ApiVersion.Major", ver.Major);

    SetConfigFilterProperty<mfxU16>(loader, cfg, "mfxImplDescription.ApiVersion.Minor", ver.Minor);

    // expect to fail
    MultiPropTest(loader, MFX_ERR_NOT_FOUND);
}

void Dispatcher_MultiProp_APIPartialValid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // if we set only major or minor (not both) the filter will not be applied
    if (implType == MFX_IMPL_TYPE_SOFTWARE) {
        SetConfigFilterProperty<mfxU16>(loader, cfg, "mfxImplDescription.ApiVersion.Major", CPU_VERSION_MAJOR + 1);
    } else {
        SetConfigFilterProperty<mfxU16>(loader, cfg, "mfxImplDescription.ApiVersion.Major", MFX_VERSION_MAJOR + 1);
    }

    // expect to pass
    MultiPropTest(loader, MFX_ERR_NONE);
}

void Dispatcher_MultiProp_MultiConfigMultiPropValid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_AVC);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxEncoderDescription.encoder.CodecID", MFX_CODEC_HEVC);

    // another config object - something valid
    mfxConfig cfg2 = MFXCreateConfig(loader);

    SetConfigFilterProperty<mfxU32>(loader, cfg2, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_AV1);

    // expect to pass
    MultiPropTest(loader, MFX_ERR_NONE);
}

void Dispatcher_MultiProp_MultiConfigMultiPropInvalid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_AVC);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxEncoderDescription.encoder.CodecID", MFX_CODEC_HEVC);

    // another config object - something invalid
    mfxConfig cfg2 = MFXCreateConfig(loader);

    SetConfigFilterProperty<mfxU32>(loader, cfg2, "mfxImplDescription.mfxEncoderDescription.encoder.CodecID", MFX_CODEC_VC1);

    // expect to fail
    MultiPropTest(loader, MFX_ERR_NOT_FOUND);
}

void Dispatcher_MultiProp_MultiConfigOverwriteValid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_AVC);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxEncoderDescription.encoder.CodecID", MFX_CODEC_HEVC);

    // overwrite same field with a different but valid codec
    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_HEVC);

    // expect to pass
    MultiPropTest(loader, MFX_ERR_NONE);
}

void Dispatcher_MultiProp_MultiConfigOverwriteInvalid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_AVC);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxEncoderDescription.encoder.CodecID", MFX_CODEC_HEVC);

    // overwrite same field with an invalid codec
    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxEncoderDescription.encoder.CodecID", MFX_CODEC_VC1);

    // expect to fail
    MultiPropTest(loader, MFX_ERR_NOT_FOUND);
}

void Dispatcher_MultiProp_MultiConfigCodecProfileValid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // matching codec and profile
    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_AVC);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile", MFX_PROFILE_AVC_HIGH);

    // expect to pass
    MultiPropTest(loader, MFX_ERR_NONE);
}

void Dispatcher_MultiProp_MultiConfigCodecProfileInvalid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // codec and profile from mismatched codecs
    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_AVC);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile", MFX_PROFILE_HEVC_MAIN);

    // expect to fail
    MultiPropTest(loader, MFX_ERR_NOT_FOUND);
}

void Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileValid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // matching codec and profile
    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_AVC);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile", MFX_PROFILE_AVC_HIGH);

    // another matching codec and profile - different config objects, so it should pass
    mfxConfig cfg2 = MFXCreateConfig(loader);

    SetConfigFilterProperty<mfxU32>(loader, cfg2, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_HEVC);

    SetConfigFilterProperty<mfxU32>(loader, cfg2, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile", MFX_PROFILE_HEVC_MAIN);

    // expect to pass
    MultiPropTest(loader, MFX_ERR_NONE);
}

void Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorderValid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // matching codec and profile
    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_AVC);

    // another matching codec and profile - different config objects, so it should pass
    // mix up the order: cfg+codec1, cfg2+codec2, cfg+prof1, cfg2+prof2
    mfxConfig cfg2 = MFXCreateConfig(loader);

    SetConfigFilterProperty<mfxU32>(loader, cfg2, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_HEVC);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile", MFX_PROFILE_AVC_HIGH);

    SetConfigFilterProperty<mfxU32>(loader, cfg2, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile", MFX_PROFILE_HEVC_MAIN);

    // expect to pass
    MultiPropTest(loader, MFX_ERR_NONE);
}

void Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorder2Valid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // matching codec and profile
    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_AVC);

    // another matching codec and profile - different config objects, so it should pass
    // mix up the order: cfg+codec1, cfg2+prof2, cfg2+codec2, cfg+prof1
    mfxConfig cfg2 = MFXCreateConfig(loader);

    SetConfigFilterProperty<mfxU32>(loader, cfg2, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile", MFX_PROFILE_HEVC_MAIN);

    SetConfigFilterProperty<mfxU32>(loader, cfg2, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_HEVC);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile", MFX_PROFILE_AVC_HIGH);

    // expect to pass
    MultiPropTest(loader, MFX_ERR_NONE);
}

void Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorderInvalid(mfxImplType implType) {
    mfxLoader loader = nullptr;
    mfxConfig cfg    = nullptr;

    mfxStatus sts = MultiPropInit(implType, &loader, &cfg, (implType == MFX_IMPL_TYPE_HARDWARE));
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // single cfg object for multiple codec/profile combinations - should fail
    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_CODEC_AVC);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", MFX_PROFILE_HEVC_MAIN);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile", MFX_PROFILE_AVC_HIGH);

    SetConfigFilterProperty<mfxU32>(loader, cfg, "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile", MFX_PROFILE_HEVC_MAIN);

    // expect to pass
    MultiPropTest(loader, MFX_ERR_NOT_FOUND);
}

// end table formatting
// clang-format on
