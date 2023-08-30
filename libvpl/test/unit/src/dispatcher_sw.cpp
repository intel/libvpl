/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "src/dispatcher_common.h"

TEST(Dispatcher_SW_CreateSession, SimpleConfigCanCreateSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_SimpleConfigCanCreateSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, SetValidNumThreadCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_SetValidNumThreadCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, SetInvalidNumThreadTypeReturnsErrUnsupported) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_SetInvalidNumThreadTypeReturnsErrUnsupported(MFX_IMPL_TYPE_SOFTWARE);
}

//MFXEnumImplementations
TEST(Dispatcher_SW_EnumImplementations, ValidInputsReturnValidDesc) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_EnumImplementations_ValidInputsReturnValidDesc(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_EnumImplementations, NullLoaderReturnsErrNull) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_EnumImplementations_NullLoaderReturnsErrNull(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_EnumImplementations, NullDescReturnsErrNull) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_EnumImplementations_NullDescReturnsErrNull(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_EnumImplementations, IndexOutOfRangeReturnsNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_EnumImplementations_IndexOutOfRangeReturnsNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

//MFXCreateSession
TEST(Dispatcher_SW_CreateSession, UnusedCfgCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_UnusedCfgCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestSWImplCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestSWImplCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, DoubleLoadersCreatesTwoSWSessions) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_DoubleLoadersCreatesTwoSWSessions(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, DoubleConfigObjsCreatesTwoSessions) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_DoubleConfigObjsCreatesTwoSessions(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, NullLoaderReturnsErrNull) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_NullLoaderReturnsErrNull(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, NullSessionReturnsErrNull) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_NullSessionReturnsErrNull(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, InvalidIndexReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_InvalidIndexReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestSupportedDecoderCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestSupportedDecoderCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestSupportedEncoderCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestSupportedEncoderCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestSupportedVPPCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestSupportedVPPCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestUnsupportedDecoderReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestUnsupportedDecoderReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestUnsupportedEncoderReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestUnsupportedEncoderReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestTwoSupportedDecodersReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestTwoSupportedDecodersReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestMixedDecodersReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestMixedDecodersReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestSupportedAccelModeCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestSupportedAccelModeCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestUnsupportedAccelModeNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestUnsupportedAccelModeReturnsNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestCurrentAPIVersionCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestCurrentAPIVersionCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestLowerAPIVersionCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestLowerAPIVersionCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestHigherAPIVersionReturnsNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestHigherAPIVersionReturnsNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestImplementedFunctionCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestImplementedFunctionCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestNotImplementedFunctionReturnsNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestNotImplementedFunctionReturnsNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestCurrentAPIMajorMinorCreatesSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestCurrentAPIMajorMinorCreatesSession(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestHigherAPIMajorReturnsNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestHigherAPIMajorReturnsNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestHigherAPIMinorReturnsNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestHigherAPIMinorReturnsNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestDeviceIDValidReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestDeviceIDValidReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestDeviceIDInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestDeviceIDInvalidReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestMediaAdapterTypeValidReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestMediaAdapterTypeValidReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestMediaAdapterTypeInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestMediaAdapterTypeInvalidReturnsErrNotFound(
        MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestPoolAllocationPolicyValidReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestPoolAllocationPolicyValidReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestPoolAllocationPolicyInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestPoolAllocationPolicyInvalidReturnsErrNotFound(
        MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestImplNameValidReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestImplNameValidReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestImplNameInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestImplNameInvalidReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestLicenseValidReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestLicenseValidReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestLicenseInvalidReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestLicenseInvalidReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestLicenseMixedReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestLicenseMixedReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestKeywordsValidReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestKeywordsValidReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, RequestKeywordsMixedReturnsErrNotFound) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_RequestKeywordsMixedReturnsErrNotFound(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_CreateSession, ConfigHandleReturnsHandle) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_ConfigHandleReturnsHandle(MFX_IMPL_TYPE_SOFTWARE);
}

//MFXDispReleaseImplDescription
TEST(Dispatcher_SW_DispReleaseImplDescription, ValidInputReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_DispReleaseImplDescription_ValidInputReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_DispReleaseImplDescription, NullLoaderReturnsErrNull) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_DispReleaseImplDescription_NullLoaderReturnsErrNull(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_DispReleaseImplDescription, NullDescReturnsErrNull) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_DispReleaseImplDescription_NullDescReturnsErrNull(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_DispReleaseImplDescription, HandleMismatchReturnsInvalidHandle) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_DispReleaseImplDescription_HandleMismatchReturnsInvalidHandle(
        MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_DispReleaseImplDescription, ReleaseTwiceReturnsErrNone) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_DispReleaseImplDescription_ReleaseTwiceReturnsErrNone(MFX_IMPL_TYPE_SOFTWARE);
}

// MFXSetConfigFilterProperty - multiple props per cfg object

TEST(Dispatcher_SW_MultiProp, DecEncValid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_DecEncValid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, DecEncInvalid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_DecEncInvalid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, APIMajorMinorValid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_APIMajorMinorValid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, APIMajorInvalid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_APIMajorInvalid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, APIMinorInvalid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_APIMinorInvalid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, APIPartialValid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_APIPartialValid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, MultiConfigMultiPropValid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiPropValid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, MultiConfigMultiPropInvalid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiPropInvalid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, MultiConfigOverwriteValid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_MultiConfigOverwriteValid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, MultiConfigOverwriteInvalid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_MultiConfigOverwriteInvalid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, MultiConfigCodecProfileValid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_MultiConfigCodecProfileValid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, MultiConfigCodecProfileInvalid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_MultiConfigCodecProfileInvalid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, MultiConfigMultiCodecMultiProfileValid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileValid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, MultiConfigMultiCodecMultiProfileReorderValid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorderValid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, MultiConfigMultiCodecMultiProfileReorder2Valid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorder2Valid(MFX_IMPL_TYPE_SOFTWARE);
}

TEST(Dispatcher_SW_MultiProp, MultiConfigMultiCodecMultiProfileReorderInvalid) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorderInvalid(MFX_IMPL_TYPE_SOFTWARE);
}
