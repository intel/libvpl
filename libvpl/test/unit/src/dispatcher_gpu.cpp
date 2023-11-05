/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "src/dispatcher_common.h"

TEST(Dispatcher_GPU_CreateSession, SimpleConfigCanCreateSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_SimpleConfigCanCreateSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, SetValidNumThreadCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_SetValidNumThreadCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, SetInvalidNumThreadTypeReturnsErrUnsupported) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_SetInvalidNumThreadTypeReturnsErrUnsupported(MFX_IMPL_TYPE_HARDWARE);
}

// fully-implemented test cases (not using common kernels)
TEST(Dispatcher_GPU_CreateSession, ExtDeviceID_ValidProps) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_HARDWARE);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // set valid, basic properties
    SetConfigFilterProperty<mfxU16>(loader, "mfxExtendedDeviceId.VendorID", 0x8086);
#if defined(_WIN32) || defined(_WIN64)
    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.LUIDDeviceNodeMask", 1);
#else
    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.DRMRenderNodeNum", 128);
    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.DRMPrimaryNodeNum", 0);
#endif
    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}

TEST(Dispatcher_GPU_CreateSession, ExtDeviceID_InvalidProps) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_HARDWARE);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // set valid, basic properties
    SetConfigFilterProperty<mfxU16>(loader, "mfxExtendedDeviceId.VendorID", 0x8086);
#if defined(_WIN32) || defined(_WIN64)
    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.LUIDDeviceNodeMask", 333);
#else
    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.DRMRenderNodeNum", 999);
    SetConfigFilterProperty<mfxU32>(loader, "mfxExtendedDeviceId.DRMPrimaryNodeNum", 555);
#endif
    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NOT_FOUND);

    // free internal resources
    MFXUnload(loader);
}

TEST(Dispatcher_GPU_CloneSession, Basic_Clone_Succeeds) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_HARDWARE);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // require 2.x RT
    sts = SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Major", 2);
    sts = SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Minor", 0);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSession cloneSession = nullptr;
    sts                     = MFXCloneSession(session, &cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // disjoin the child (cloned) session
    sts = MFXDisjoinSession(cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

TEST(Dispatcher_GPU_CloneSession, Basic_Clone_Succeeds_Legacy) {
    SKIP_IF_DISP_GPU_MSDK_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_HARDWARE);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // require 1.x RT
    sts = SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Major", 1);
    sts = SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Minor", 0);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSession cloneSession = nullptr;
    sts                     = MFXCloneSession(session, &cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // disjoin the child (cloned) session
    sts = MFXDisjoinSession(cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

TEST(Dispatcher_GPU_CloneSession, Double_Clone_Succeeds) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_HARDWARE);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // require 2.x RT
    sts = SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Major", 2);
    sts = SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Minor", 0);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    mfxSession cloneSession = nullptr;
    sts                     = MFXCloneSession(session, &cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // not clear from docs whether this is expected to be supported in the RT,
    //   but it succeeds for now so include this to test the dispatcher-level logic
    // if RT changes to disallow clone of a clone, this test can be disabled
    mfxSession cloneSession2 = nullptr;
    sts                      = MFXCloneSession(cloneSession, &cloneSession2);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // disjoin the child (cloned) session
    sts = MFXDisjoinSession(cloneSession2);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // disjoin the child (cloned) session
    sts = MFXDisjoinSession(cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(cloneSession2);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

TEST(Dispatcher_GPU_JoinSession, Basic_Join_Succeeds) {
    SKIP_IF_DISP_GPU_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_HARDWARE);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // require 2.x RT
    sts = SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Major", 1);
    sts = SetConfigFilterProperty<mfxU16>(loader, "mfxImplDescription.ApiVersion.Minor", 0);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // create another session with first implementation
    mfxSession cloneSession = nullptr;
    sts                     = MFXCreateSession(loader, 0, &cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXJoinSession(session, cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // disjoin the child (cloned) session
    sts = MFXDisjoinSession(cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    sts = MFXClose(cloneSession);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    MFXUnload(loader);
}

#if defined(_WIN32) || defined(_WIN64)
TEST(Dispatcher_GPU_CreateSession, D3D9CanCreateSession) {
    SKIP_IF_DISP_GPU_MSDK_DISABLED();

    mfxLoader loader = MFXLoad();
    EXPECT_FALSE(loader == nullptr);

    mfxStatus sts = SetConfigImpl(loader, MFX_IMPL_TYPE_HARDWARE);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    SetConfigFilterProperty<mfxU32>(loader,
                                    "mfxImplDescription.AccelerationMode",
                                    MFX_ACCEL_MODE_VIA_D3D9);

    // create session with first implementation
    mfxSession session = nullptr;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE);

    // free internal resources
    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE);
    MFXUnload(loader);
}
#endif

// MFXEnumImplementations
TEST(Dispatcher_GPU_EnumImplementations, ValidInputsReturnValidDesc) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_EnumImplementations_ValidInputsReturnValidDesc(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_EnumImplementations, NullLoaderReturnsErrNull) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_EnumImplementations_NullLoaderReturnsErrNull(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_EnumImplementations, NullDescReturnsErrNull) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_EnumImplementations_NullDescReturnsErrNull(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_EnumImplementations, IndexOutOfRangeReturnsNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_EnumImplementations_IndexOutOfRangeReturnsNotFound(MFX_IMPL_TYPE_HARDWARE);
}

// MFXCreateSession
TEST(Dispatcher_GPU_CreateSession, UnusedCfgCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_UnusedCfgCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestSWImplCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestSWImplCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, DoubleLoadersCreatesTwoSWSessions) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_DoubleLoadersCreatesTwoSWSessions(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, DoubleConfigObjsCreatesTwoSessions) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_DoubleConfigObjsCreatesTwoSessions(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, NullLoaderReturnsErrNull) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_NullLoaderReturnsErrNull(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, NullSessionReturnsErrNull) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_NullSessionReturnsErrNull(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, InvalidIndexReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_InvalidIndexReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestSupportedDecoderCreatesSession) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestSupportedDecoderCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestSupportedEncoderCreatesSession) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestSupportedEncoderCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestSupportedVPPCreatesSession) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestSupportedVPPCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestUnsupportedDecoderReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestUnsupportedDecoderReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestUnsupportedEncoderReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestUnsupportedEncoderReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestTwoSupportedDecodersReturnsErrNone) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestTwoSupportedDecodersReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestMixedDecodersReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestMixedDecodersReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestSupportedAccelModeCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestSupportedAccelModeCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestAccelInvalidReturnsNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestUnsupportedAccelModeReturnsNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestCurrentAPIVersionCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestCurrentAPIVersionCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestLowerAPIVersionCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestLowerAPIVersionCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestHigherAPIVersionReturnsNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestHigherAPIVersionReturnsNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestImplementedFunctionCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestImplementedFunctionCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestNotImplementedFunctionReturnsNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestNotImplementedFunctionReturnsNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestCurrentAPIMajorMinorCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestCurrentAPIMajorMinorCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestHigherAPIMajorReturnsNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestHigherAPIMajorReturnsNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestHigherAPIMinorReturnsNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestHigherAPIMinorReturnsNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestDeviceIDInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestDeviceIDInvalidReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestMediaAdapterTypeValidReturnsErrNone) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestMediaAdapterTypeValidReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestMediaAdapterTypeInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestMediaAdapterTypeInvalidReturnsErrNotFound(
        MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestPoolAllocationPolicyValidReturnsErrNone) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestPoolAllocationPolicyValidReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestPoolAllocationPolicyInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestPoolAllocationPolicyInvalidReturnsErrNotFound(
        MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestImplNameValidReturnsErrNone) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_CreateSession_RequestImplNameValidReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestImplNameInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestImplNameInvalidReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

// TO DO - GPU RT has not implemented license yet
TEST(DISABLED_Dispatcher_GPU_CreateSession, RequestLicenseValidReturnsErrNone) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestLicenseValidReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestLicenseInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestLicenseInvalidReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestLicenseMixedReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestLicenseMixedReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

// TO DO - GPU RT has not implemented keywords yet
TEST(DISABLED_Dispatcher_GPU_CreateSession, RequestKeywordsValidReturnsErrNone) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestKeywordsValidReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestKeywordsMixedReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestKeywordsMixedReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

// TO DO - need to create real device in order to set handle
TEST(DISABLED_Dispatcher_GPU_CreateSession, ConfigHandleReturnsHandle) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_ConfigHandleReturnsHandle(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestValidDXGIAdapterCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestValidDXGIAdapterCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, RequestInvalidDXGIAdapterReturnsErrNotFound) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_RequestInvalidDXGIAdapterReturnsErrNotFound(MFX_IMPL_TYPE_HARDWARE);
}

// MFXDispReleaseImplDescription
TEST(Dispatcher_GPU_DispReleaseImplDescription, ValidInputReturnsErrNone) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_DispReleaseImplDescription_ValidInputReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_DispReleaseImplDescription, NullLoaderReturnsErrNull) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_DispReleaseImplDescription_NullLoaderReturnsErrNull(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_DispReleaseImplDescription, NullDescReturnsErrNull) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_DispReleaseImplDescription_NullDescReturnsErrNull(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_DispReleaseImplDescription, HandleMismatchReturnsInvalidHandle) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_DispReleaseImplDescription_HandleMismatchReturnsInvalidHandle(
        MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_DispReleaseImplDescription, ReleaseTwiceReturnsErrNone) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_DispReleaseImplDescription_ReleaseTwiceReturnsErrNone(MFX_IMPL_TYPE_HARDWARE);
}

// MFXSetConfigFilterProperty - multiple props per cfg object

TEST(Dispatcher_GPU_MultiProp, DecEncValid) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_MultiProp_DecEncValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, DecEncInvalid) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_MultiProp_DecEncInvalid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, APIMajorMinorValid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_APIMajorMinorValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, APIMajorInvalid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_APIMajorInvalid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, APIMinorInvalid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_APIMinorInvalid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, APIPartialValid) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_MultiProp_APIPartialValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigMultiPropValid) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiPropValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigMultiPropInvalid) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiPropInvalid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigOverwriteValid) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_MultiProp_MultiConfigOverwriteValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigOverwriteInvalid) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_MultiProp_MultiConfigOverwriteInvalid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigCodecProfileValid) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_MultiProp_MultiConfigCodecProfileValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigCodecProfileInvalid) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_MultiProp_MultiConfigCodecProfileInvalid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigMultiCodecMultiProfileValid) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigMultiCodecMultiProfileReorderValid) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorderValid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigMultiCodecMultiProfileReorder2Valid) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorder2Valid(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_MultiProp, MultiConfigMultiCodecMultiProfileReorderInvalid) {
    SKIP_IF_DISP_GPU_VPL_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorderInvalid(MFX_IMPL_TYPE_HARDWARE);
}
