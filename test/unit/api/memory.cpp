/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>
#include "vpl/mfxvideo.h"

/*
   Memory functions have the same states
   MFX_ERR_NONE The function completed successfully. \n
   MFX_ERR_NULL_PTR If surface is NULL. \n
   MFX_ERR_INVALID_HANDLE If session was not initialized. \n
   MFX_ERR_NOT_INITIALIZED If VPP wasn't initialized
*/

//GetSurfaceForVPP
TEST(GetSurfaceForVPP, DISABLED_InitializedVPPReturnsSurface) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForVPP, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForVPP, DISABLED_NullSessionReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForVPP, DISABLED_UninitializedVPPReturnsNotInitialized) {
    FAIL() << "Test not implemented";
}

//GetSurfaceForEncode
TEST(GetSurfaceForEncode, DISABLED_InitializedEncodeReturnsSurface) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForEncode, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForEncode, DISABLED_NullSessionReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForEncode, DISABLED_UninitializedEncodeReturnsNotInitialized) {
    FAIL() << "Test not implemented";
}

//GetSurfaceForDecode
TEST(GetSurfaceForDecode, DISABLED_InitializedDecodeReturnsSurface) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForDecode, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForDecode, DISABLED_NullSessionReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(GetSurfaceForDecode, DISABLED_UninitializedDecodeReturnsNotInitialized) {
    FAIL() << "Test not implemented";
}

//Simplified decode

TEST(FrameInterface, DISABLED_NoDecodeHeaderCanDecode) {
    FAIL() << "Test not implemented";
}

//AddRef
TEST(FrameInterfaceAddRef, DISABLED_ValidInputReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceAddRef, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceAddRef, DISABLED_NullHandleReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

//Release
TEST(FrameInterfaceRelease, DISABLED_ValidInputReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceRelease, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceRelease, DISABLED_NullHandleReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceRelease, DISABLED_ZeroRefcountReturnsErrUnknown) {
    FAIL() << "Test not implemented";
}

//GetRefCounter
TEST(FrameInterfaceGetRefCounter, DISABLED_ValidInputReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetRefCounter, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetRefCounter, DISABLED_NullHandleReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

//Map
TEST(FrameInterfaceMap, DISABLED_ValidInputReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceMap, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceMap, DISABLED_NullHandleReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceMap, DISABLED_InvalidFlagValReturnsUnsupported) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceMap, DISABLED_WriteToWriteFlagSurfaceReturnsErrLock) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceMap, DISABLED_WriteToReadWriteFlagSurfaceReturnsErrLock) {
    FAIL() << "Test not implemented";
}

//Unmap
TEST(FrameInterfaceUnmap, DISABLED_ValidInputReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceUnmap, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceUnmap, DISABLED_NullHandleReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceUnmap, DISABLED_AlreadyUnmappedReturnsUnsupported) {
    FAIL() << "Test not implemented";
}

//GetNativeHandle
TEST(FrameInterfaceGetNativeHandle, DISABLED_ValidInputReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetNativeHandle, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetNativeHandle, DISABLED_NullResourceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetNativeHandle, DISABLED_NullResourceTypeReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetNativeHandle, DISABLED_NullSurfaceReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetNativeHandle, DISABLED_NullResourceReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetNativeHandle,
     DISABLED_NullResourceTypeReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetNativeHandle, DISABLED_SystemMemoryReturnsUnsupported) {
    FAIL() << "Test not implemented";
}

//GetDeviceHandle
TEST(FrameInterfaceGetDeviceHandle, DISABLED_ValidInputReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetDeviceHandle, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetDeviceHandle, DISABLED_NullHandleReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetDeviceHandle, DISABLED_NullDeviceTypeReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetDeviceHandle,
     DISABLED_InvalidSurfaceReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetDeviceHandle,
     DISABLED_InvalidHandleReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetDeviceHandle,
     DISABLED_InvalidDeviceTypeReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetDeviceHandle, DISABLED_SystemMemoryReturnsUnsupported) {
    FAIL() << "Test not implemented";
}

//GetDeviceHandle
TEST(FrameInterfaceSynchronize, DISABLED_ValidInputReturnsErrNone) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceSynchronize, DISABLED_NullSurfaceReturnsErrNull) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceSynchronize, DISABLED_InvalidSurfaceReturnsInvalidHandle) {
    FAIL() << "Test not implemented";
}

TEST(FrameInterfaceGetDeviceHandle, DISABLED_AsyncDependencyReturnsAborted) {
    FAIL() << "Test not implemented";
}
