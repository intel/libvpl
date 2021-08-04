/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef DISPATCHER_TEST_RUNTIMES_STUB_SRC_CAPS_H_
#define DISPATCHER_TEST_RUNTIMES_STUB_SRC_CAPS_H_

#include <string.h>

#include "vpl/mfxjpeg.h"
#include "vpl/mfxvideo.h"

// random values to return in MFXQueryImpl() instead of real impl type
// unit tests can check which mock DLL was actually inited from MFXCreateSession()
#define DBG_VALID_IMPL_CFG_ALL  0x19afb235
#define DBG_VALID_IMPL_CFG_NONE 0x478ae201

#define MFX_IMPL_TYPE_MOCK 0xffffffff

struct ImplDescriptionArray {
    mfxImplDescription implDesc; // MUST be the first element

    mfxHDL *basePtr; // pointer to the array of handles
    mfxU32 currImpl; // index of this handle in the array
    mfxU32 numImpl; // total number of implementations
};

#if defined(_WIN32) || defined(_WIN64)
#else
    // Linux
    #define strcpy_s(dst, size, src)       strcpy((dst), (src)) // NOLINT
    #define strncpy_s(dst, size, src, cnt) strcpy((dst), (src)) // NOLINT
#endif

#define DEF_STRUCT_VERSION_MAJOR 1
#define DEF_STRUCT_VERSION_MINOR 0

#define DEF_RANGE_MIN  64
#define DEF_RANGE_MAX  4096
#define DEF_RANGE_STEP 8

// typedef child structures for easier reading
typedef struct mfxDecoderDescription::decoder DecCodec;
typedef struct mfxDecoderDescription::decoder::decprofile DecProfile;
typedef struct mfxDecoderDescription::decoder::decprofile::decmemdesc DecMemDesc;

typedef struct mfxEncoderDescription::encoder EncCodec;
typedef struct mfxEncoderDescription::encoder::encprofile EncProfile;
typedef struct mfxEncoderDescription::encoder::encprofile::encmemdesc EncMemDesc;

typedef struct mfxVPPDescription::filter VPPFilter;
typedef struct mfxVPPDescription::filter::memdesc VPPMemDesc;
typedef struct mfxVPPDescription::filter::memdesc::format VPPFormat;

// top-level structures for auto-generated caps
extern const mfxDecoderDescription decoderDesc;
extern const mfxEncoderDescription encoderDesc;
extern const mfxVPPDescription vppDesc;

#endif // DISPATCHER_TEST_RUNTIMES_STUB_SRC_CAPS_H_
