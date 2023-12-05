//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
// Example using Intel® Video Processing Library (Intel® VPL)

///
/// Utility library header file for sample code
///
/// @file

#ifndef EXAMPLES_UTIL_H_
#define EXAMPLES_UTIL_H_

#include <atlbase.h>
#include <conio.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <windowsx.h>
#include <fstream>
#include "vpl/mfx.h"

#define WAIT_100_MILLISECONDS 100
#define MAX_PATH              260
#define MAX_WIDTH             3840
#define MAX_HEIGHT            2160
#define IS_ARG_EQ(a, b)       (!strcmp((a), (b)))

#define VERIFY(x, y)       \
    if (!(x)) {            \
        printf("%s\n", y); \
        exit(-1);          \
    }

#define ALIGN16(value)           (((value + 15) >> 4) << 4)
#define ALIGN32(X)               (((mfxU32)((X) + 31)) & (~(mfxU32)31))
#define VPLVERSION(major, minor) (major << 16 | minor)

enum ExampleParams { PARAM_IMPL = 0, PARAM_INFILE, PARAM_INRES, PARAM_COUNT };
enum ParamGroup {
    PARAMS_CREATESESSION = 0,
    PARAMS_DECODE,
    PARAMS_ENCODE,
    PARAMS_VPP,
    PARAMS_TRANSCODE
};

typedef struct _Params {
    char *infileName;
    char *inmodelName;
    char *openCLfileName;

    mfxU16 srcWidth;
    mfxU16 srcHeight;
} Params;

struct CPUFrameInfo {
    mfxU8 *data;
    mfxU16 width;
    mfxU16 height;
    mfxU16 pitch;
};

struct CPUFrameInfo_NV12 {
    mfxU8 *Y;
    mfxU8 *UV;

    mfxU16 width;
    mfxU16 height;
    mfxU16 pitch;
};

char *ValidateFileName(char *in);

bool ValidateSize(char *in, mfxU16 *vsize, mfxU32 vmax);

bool ParseArgsAndValidate(int argc, char *argv[], Params *params, ParamGroup group);

// Show implementation info for Media SDK or Intel® VPL
mfxVersion ShowImplInfo(mfxSession session);

// Show implementation info with Intel® VPL
void ShowImplementationInfo(mfxLoader loader, mfxU32 implnum);

// Write encoded stream to file
void WriteEncodedStream(mfxBitstream &bs, FILE *f);

// Read NV12 stream from file
mfxStatus ReadRawFrameCPU_NV12(CPUFrameInfo_NV12 *cpuFrameInfo, FILE *f);

// Read encoded stream from file
mfxStatus ReadEncodedStream(mfxBitstream &bs, FILE *f);

// Write NV12 stream to file
mfxStatus WriteRawFrameCPU_NV12(CPUFrameInfo_NV12 *cpuFrameInfo, FILE *f);

#endif //EXAMPLES_UTIL_H_
