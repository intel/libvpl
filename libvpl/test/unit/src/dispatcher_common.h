/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef LIBVPL_TEST_UNIT_SRC_DISPATCHER_COMMON_H_
#define LIBVPL_TEST_UNIT_SRC_DISPATCHER_COMMON_H_

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>

    #include <shlwapi.h>

    #define PATH_SEPARATOR "\\"
#else
    #include <dirent.h>
    #include <limits.h>
    #include <sys/stat.h>

    #define PATH_SEPARATOR "/"
#endif

#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "vpl/mfx.h"

#include "src/unit_api.h"

// define expected API version used to build CPU runtime used in with unit tests
#define CPU_VERSION_MAJOR 2
#define CPU_VERSION_MINOR 9

// define special stub impl types for testing only
#define MFX_IMPL_TYPE_STUB      ((mfxImplType)0xFFFF)
#define MFX_IMPL_TYPE_STUB_1X   ((mfxImplType)0xAAAA)
#define MFX_IMPL_TYPE_STUB_NOFN ((mfxImplType)0xBBBB)

#define CAPTURE_LOG_DEF_FILENAME "utestLogFile_vpl.txt"

typedef enum {
    CAPTURE_LOG_DISABLED = 0,

    CAPTURE_LOG_DISPATCHER = 1, // capture the dispatcher log output (enables ONEVPL_DISPATCHER_LOG)
    CAPTURE_LOG_FILE       = 2, // capture log output which is sent to a file
    CAPTURE_LOG_COUT       = 3, // capture log output which is sent to std::cout
} CaptureLogType;

// helper functions for dispatcher tests
mfxStatus SetConfigImpl(mfxLoader loader, mfxU32 implType, bool bRequire2xGPU = false);

// start capturing log output, behavior determined by CaptureLogType
void CaptureOutputLog(CaptureLogType type);

// check whether expectedString is found in the captured log
// set expectMatch to false to check that expectedString is NOT in the log
void CheckOutputLog(const char *expectedString, bool expectMatch = true);

// delete log files, reset log type, reset cout
void CleanupOutputLog(void);

// helper functions for testing string API, C-style alloc/free to illustrate possible FFmpeg integration
mfxStatus AllocateExtBuf(mfxVideoParam &par,
                         std::vector<mfxExtBuffer *> &extBufVector,
                         mfxExtBuffer &extBuf);
void ReleaseExtBufs(std::vector<mfxExtBuffer *> &extBufVector);
mfxExtBuffer *FindExtBuf(mfxVideoParam &par, mfxU32 BufferId);

int CreateWorkingDirectory(const char *dirPath);

void SetWorkingDirectoryPath(std::string workDirPath);
void GetWorkingDirectoryPath(std::string &workDirPath);

int AddDeviceID(std::string deviceID);
void GetDeviceIDList(std::list<std::string> &deviceIDList);

// create mfxConfig object and apply to loader
template <typename varDataType>
mfxStatus SetConfigFilterProperty(mfxLoader loader, const char *name, varDataType data);

template <typename varDataType>
mfxStatus SetConfigFilterProperty(mfxLoader loader,
                                  mfxConfig cfg,
                                  const char *name,
                                  varDataType data);

// set property with a new cfg object
template <typename varDataType>
mfxStatus SetSingleProperty(mfxLoader loader,
                            const char *name,
                            mfxVariantType varType,
                            varDataType data);

// use same cfg instead of creating a new one
template <typename varDataType>
mfxStatus SetSingleProperty(mfxLoader loader,
                            mfxConfig cfg,
                            const char *name,
                            mfxVariantType varType,
                            varDataType data);

// common kernels - set implType for stub, SW, GPU, etc.
void Dispatcher_CreateSession_SimpleConfigCanCreateSession(mfxImplType implType);
void Dispatcher_CreateSession_SetValidNumThreadCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_SetInvalidNumThreadTypeReturnsErrUnsupported(mfxImplType implType);

void Dispatcher_EnumImplementations_ValidInputsReturnValidDesc(mfxImplType implType);
void Dispatcher_EnumImplementations_NullLoaderReturnsErrNull(mfxImplType implType);
void Dispatcher_EnumImplementations_NullDescReturnsErrNull(mfxImplType implType);
void Dispatcher_EnumImplementations_IndexOutOfRangeReturnsNotFound(mfxImplType implType);

void Dispatcher_CreateSession_SimpleConfigCanCreateSession(mfxImplType implType);
void Dispatcher_CreateSession_UnusedCfgCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestSWImplCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_DoubleLoadersCreatesTwoSWSessions(mfxImplType implType);
void Dispatcher_CreateSession_DoubleConfigObjsCreatesTwoSessions(mfxImplType implType);
void Dispatcher_CreateSession_NullLoaderReturnsErrNull(mfxImplType implType);
void Dispatcher_CreateSession_NullSessionReturnsErrNull(mfxImplType implType);
void Dispatcher_CreateSession_InvalidIndexReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestSupportedDecoderCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestSupportedEncoderCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestSupportedVPPCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestUnsupportedDecoderReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestUnsupportedEncoderReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestTwoSupportedDecodersReturnsErrNone(mfxImplType implType);
void Dispatcher_CreateSession_RequestMixedDecodersReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestSupportedAccelModeCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestUnsupportedAccelModeReturnsNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestCurrentAPIVersionCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestLowerAPIVersionCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestHigherAPIVersionReturnsNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestImplementedFunctionCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestNotImplementedFunctionReturnsNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestCurrentAPIMajorMinorCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestHigherAPIMajorReturnsNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestHigherAPIMinorReturnsNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestDeviceIDValidReturnsErrNone(mfxImplType implType);
void Dispatcher_CreateSession_RequestDeviceIDInvalidReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestMediaAdapterTypeValidReturnsErrNone(mfxImplType implType);
void Dispatcher_CreateSession_RequestMediaAdapterTypeInvalidReturnsErrNotFound(
    mfxImplType implType);
void Dispatcher_CreateSession_RequestPoolAllocationPolicyValidReturnsErrNone(mfxImplType implType);
void Dispatcher_CreateSession_RequestPoolAllocationPolicyInvalidReturnsErrNotFound(
    mfxImplType implType);
void Dispatcher_CreateSession_RequestImplNameValidReturnsErrNone(mfxImplType implType);
void Dispatcher_CreateSession_RequestImplNameInvalidReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestLicenseValidReturnsErrNone(mfxImplType implType);
void Dispatcher_CreateSession_RequestLicenseInvalidReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestLicenseMixedReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_RequestKeywordsValidReturnsErrNone(mfxImplType implType);
void Dispatcher_CreateSession_RequestKeywordsMixedReturnsErrNotFound(mfxImplType implType);
void Dispatcher_CreateSession_ConfigHandleReturnsHandle(mfxImplType implType);
void Dispatcher_CreateSession_RequestValidDXGIAdapterCreatesSession(mfxImplType implType);
void Dispatcher_CreateSession_RequestInvalidDXGIAdapterReturnsErrNotFound(mfxImplType implType);

void Dispatcher_DispReleaseImplDescription_ValidInputReturnsErrNone(mfxImplType implType);
void Dispatcher_DispReleaseImplDescription_NullLoaderReturnsErrNull(mfxImplType implType);
void Dispatcher_DispReleaseImplDescription_NullDescReturnsErrNull(mfxImplType implType);
void Dispatcher_DispReleaseImplDescription_HandleMismatchReturnsInvalidHandle(mfxImplType implType);
void Dispatcher_DispReleaseImplDescription_ReleaseTwiceReturnsErrNone(mfxImplType implType);

void Dispatcher_MultiProp_DecEncValid(mfxImplType implType);
void Dispatcher_MultiProp_DecEncInvalid(mfxImplType implType);
void Dispatcher_MultiProp_APIMajorMinorValid(mfxImplType implType);
void Dispatcher_MultiProp_APIMajorInvalid(mfxImplType implType);
void Dispatcher_MultiProp_APIMinorInvalid(mfxImplType implType);
void Dispatcher_MultiProp_APIPartialValid(mfxImplType implType);

void Dispatcher_MultiProp_MultiConfigMultiPropValid(mfxImplType implType);
void Dispatcher_MultiProp_MultiConfigMultiPropInvalid(mfxImplType implType);
void Dispatcher_MultiProp_MultiConfigOverwriteValid(mfxImplType implType);
void Dispatcher_MultiProp_MultiConfigOverwriteInvalid(mfxImplType implType);
void Dispatcher_MultiProp_MultiConfigCodecProfileValid(mfxImplType implType);
void Dispatcher_MultiProp_MultiConfigCodecProfileInvalid(mfxImplType implType);
void Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileValid(mfxImplType implType);
void Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorderValid(mfxImplType implType);
void Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorder2Valid(mfxImplType implType);
void Dispatcher_MultiProp_MultiConfigMultiCodecMultiProfileReorderInvalid(mfxImplType implType);

void Dispatcher_CreateSession_ExtDeviceID_VendorID_Valid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_VendorID_Invalid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_DeviceID_Valid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_DeviceID_Invalid(mfxImplType implType);

void Dispatcher_CreateSession_ExtDeviceID_PCIDomain_Valid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_PCIDomain_Invalid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_PCIBus_Valid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_PCIBus_Invalid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_PCIDevice_Valid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_PCIDevice_Invalid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_PCIFunction_Valid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_PCIFunction_Invalid(mfxImplType implType);

void Dispatcher_CreateSession_ExtDeviceID_DeviceLUID_Valid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_DeviceLUID_Invalid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_LUIDDeviceNodeMask_Valid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_LUIDDeviceNodeMask_Invalid(mfxImplType implType);

void Dispatcher_CreateSession_ExtDeviceID_DRMRenderNodeNum_Valid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_DRMRenderNodeNum_Invalid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_DRMPrimaryNodeNum_Valid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_DRMPrimaryNodeNum_Invalid(mfxImplType implType);

void Dispatcher_CreateSession_ExtDeviceID_RevisionID_Valid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_RevisionID_Invalid(mfxImplType implType);

void Dispatcher_CreateSession_ExtDeviceID_DeviceName_Valid(mfxImplType implType);
void Dispatcher_CreateSession_ExtDeviceID_DeviceName_Invalid(mfxImplType implType);

void Dispatcher_CreateSession_SurfaceSupport_SurfaceType_Valid(mfxImplType implType);
void Dispatcher_CreateSession_SurfaceSupport_SurfaceType_Invalid(mfxImplType implType);
void Dispatcher_CreateSession_SurfaceSupport_SurfaceComponent_Valid(mfxImplType implType);
void Dispatcher_CreateSession_SurfaceSupport_SurfaceComponent_Invalid(mfxImplType implType);
void Dispatcher_CreateSession_SurfaceSupport_SurfaceFlags_Valid(mfxImplType implType);
void Dispatcher_CreateSession_SurfaceSupport_SurfaceFlags_Invalid(mfxImplType implType);
void Dispatcher_CreateSession_SurfaceSupport_InvalidProperty(mfxImplType implType);
void Dispatcher_CreateSession_SurfaceSupport_SingleFramework_Valid(mfxImplType implType);
void Dispatcher_CreateSession_SurfaceSupport_SingleFramework_Invalid(mfxImplType implType);
void Dispatcher_CreateSession_SurfaceSupport_TwoFrameworks_Valid(mfxImplType implType);
void Dispatcher_CreateSession_SurfaceSupport_TwoFrameworks_Invalid(mfxImplType implType);

#endif // LIBVPL_TEST_UNIT_SRC_DISPATCHER_COMMON_H_
