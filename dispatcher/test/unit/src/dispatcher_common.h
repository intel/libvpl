/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef DISPATCHER_TEST_UNIT_SRC_DISPATCHER_COMMON_H_
#define DISPATCHER_TEST_UNIT_SRC_DISPATCHER_COMMON_H_

#include "vpl/mfxdispatcher.h"
#include "vpl/mfximplcaps.h"
#include "vpl/mfxvideo.h"

#include "src/unit_api.h"

// define special stub impl type for testing only
#define MFX_IMPL_TYPE_STUB ((mfxImplType)0xFFFF)

// helper functions for dispatcher tests
mfxStatus SetConfigImpl(mfxLoader loader, mfxU32 implType);

void CaptureDispatcherLog();
void CheckDispatcherLog(const char *expectedString, bool expectMatch = true);

void CaptureRuntimeLog();
void CheckRuntimeLog(const char *expectedString, bool expectMatch = true);

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

#endif // DISPATCHER_TEST_UNIT_SRC_DISPATCHER_COMMON_H_
