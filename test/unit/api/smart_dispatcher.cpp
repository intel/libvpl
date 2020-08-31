/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxdispatcher.h"
#include "vpl/mfximplcaps.h"
#include "vpl/mfxvideo.h"

//MFXLoad
TEST(Load, DISABLED_CallReturnsLoader) {
    FAIL() << "Test not implemented";
}

TEST(Load, DISABLED_NoImplementationDllsReturnsNullLoader) {
    FAIL() << "Test not implemented";
}

//MFXCreateConfig
TEST(CreateConfig, DISABLED_InitializedLoaderReturnsConfig) {
    FAIL() << "Test not implemented";
}

TEST(CreateConfig, DISABLED_NullLoaderReturnsErrNull) {
    FAIL() << "Test not implemented";
}

//MFXSetConfigFilterProperty
TEST(SetConfigFilterProperty, DISABLED_VPLImplInReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(SetConfigFilterProperty, DISABLED_NullConfigReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(SetConfigFilterProperty, DISABLED_NullNameReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(SetConfigFilterProptery, DISABLED_UnknownParamReturnsNotFound) {
    FAIL() << "Test not implemented";
}

TEST(SetConfigFilterProptery, DISABLED_ValueTypeMismatchReturnsErrUnsupported) {
    FAIL() << "Test not implemented";
}

//MFXEnumImplementations
TEST(EnumImplementations, DISABLED_ValidInputsReturnValidDesc) {
    FAIL() << "Test not implemented";
}

TEST(EnumImplementations, DISABLED_NullLoaderReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(EnumImplementations, DISABLED_NullSessionReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(EnumImplementations, DISABLED_IndexOutOfRangeReturnsNotFound) {
    FAIL() << "Test not implemented";
}

//MFXCreateSession
TEST(CreateSession, DISABLED_SimpleConfigCanCreateSession) {
    FAIL() << "Test not implemented";
    /*
 Usage example:
			   mfxLoader loader = MFXLoad();
			   mfxConfig cfg = MFXCreateConfig(loader);
			   MFXCreateSession(loader,0,&session);
			   //returns ERR_NONE
*/
}

TEST(CreateSession, DISABLED_RequestLegacyHWImplCreatesSession) {
    FAIL() << "Test not implemented";
    /*
	 Usage example:
		   mfxLoader loader = MFXLoad();
			 mfxConfig cfg = MFXCreateConfig(loader);
			 mfxVariant ImplValue;
			 ImplValue.Type = MFX_VARIANT_TYPE_U32;
			 ImplValue.Data.U32 = MFX_IMPL_TYPE_HARDWARE;
			 MFXSetConfigFilterProperty(cfg,"mfxImplDescription.Impl",ImplValue);
			 MFXCreateSession(loader,0,&session);
	*/
}

TEST(CreateSession, DISABLED_DoubleLoadersCreatesTwoSessions) {
    FAIL() << "Test not implemented";
    /*
	 Usage example:
             // Create session with software based implementation
			 mfxLoader loader1 = MFXLoad();
			 mfxConfig cfg1 = MFXCreateConfig(loader1);
			 mfxVariant ImplValueSW;
			 ImplValueSW.Type = MFX_VARIANT_TYPE_U32;
			 ImplValueSW.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
			 MFXSetConfigFilterProperty(cfg1,"mfxImplDescription.Impl",ImplValueSW);
			 MFXCreateSession(loader1,0,&sessionSW);

			 // Create session with hardware based implementation
			 mfxLoader loader2 = MFXLoad();
			 mfxConfig cfg2 = MFXCreateConfig(loader2);
			 mfxVariant ImplValueHW;
			 ImplValueHW.Type = MFX_VARIANT_TYPE_U32;
			 ImplValueHW.Data.U32 = MFX_IMPL_TYPE_HARDWARE;
			 MFXSetConfigFilterProperty(cfg2,"mfxImplDescription.Impl",ImplValueHW);
			 MFXCreateSession(loader2,0,&sessionHW);

			 // use both sessionSW and sessionHW
			 // ...
			 // Close everything
			 MFXClose(sessionSW);
			 MFXClose(sessionHW);
			 MFXUnload(loader1); // cfg1 will be destroyed here.
			 MFXUnload(loader2); // cfg2 will be destroyed here.
	*/
}

TEST(CreateSession, DISABLED_DoubleConfigObjsCreatesTwoSessions) {
    FAIL() << "Test not implemented";
    /*
	 Usage example:
	 mfxLoader loader = MFXLoad();
             mfxConfig cfg1 = MFXCreateConfig(loader);
             mfxVariant ImplValue;
             val.Type = MFX_VARIANT_TYPE_U32;
             val.Data.U32 = MFX_CODEC_AVC;
             MFXSetConfigFilterProperty(cfg1,"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",ImplValue);

             mfxConfig cfg2 = MFXCreateConfig(loader);
             mfxVariant ImplValue;
             val.Type = MFX_VARIANT_TYPE_U32;
             val.Data.U32 = MFX_CODEC_HEVC;
             MFXSetConfigFilterProperty(cfg2,"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",ImplValue);

             MFXCreateSession(loader,0,&sessionAVC);
             MFXCreateSession(loader,0,&sessionHEVC);
	*/
}

TEST(CreateSession, DISABLED_NullLoaderReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(CreateSession, DISABLED_NullSessionReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(CreateSession, DISABLED_InvalidIndexReturnsErrNotFound) {
    FAIL() << "Test not implemented";
}

//MFXDispReleaseImplDescription
TEST(DispReleaseImplDescription, DISABLED_ValidInputReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(DispReleaseImplDescription, DISABLED_NullLoaderReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(DispReleaseImplDescription, DISABLED_HandleMismatchReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

// smart dispatcher operations
//QueryImplsDescription
TEST(QueryImplsDescription, DISABLED_DeliveryFormatInReturnsHdl) {
    FAIL() << "Test not implemented";
}

TEST(QueryImplsDescription, DISABLED_NullDeliveryFormatInReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(QueryImplsDescription, DISABLED_NullNumImplsInReturnsErrNull) {
    FAIL() << "Test not implemented";
}

//ReleaseImplDescription
TEST(ReleaseImplDescription, DISABLED_InitializedHdlReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(ReleaseImplDescription, DISABLED_UninitializedHdlReturnsErrNull) {
    FAIL() << "Test not implemented";
}