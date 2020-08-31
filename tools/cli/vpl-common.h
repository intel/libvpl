/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
#ifndef TOOLS_CLI_VPL_COMMON_H_
#define TOOLS_CLI_VPL_COMMON_H_

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

#include "vpl/mfxdispatcher.h"
#include "vpl/mfxjpeg.h"
#include "vpl/mfxvideo.h"

#define IS_ARG_EQ(a, b) (!strcmp((a), (b)))

enum MemoryMode {
    MEM_MODE_UNKNOWN  = -1,
    MEM_MODE_EXTERNAL = 0,
    MEM_MODE_INTERNAL,
    MEM_MODE_AUTO,

    MEM_MODE_COUNT
};

enum DispatcherMode {
    DISPATCHER_MODE_UNKNOWN = -1,
    DISPATCHER_MODE_LEGACY  = 0,
    DISPATCHER_MODE_VPL_20,

    DISPATCHER_MODE_COUNT
};

enum WSType {
    WSTYPE_DECODE = 0,
    WSTYPE_ENCODE,
    WSTYPE_VPP,

    WSTYPE_COUNT
};

extern const char* MemoryModeString[MEM_MODE_COUNT];
extern const char* DispatcherModeString[DISPATCHER_MODE_COUNT];

typedef struct _Params {
    char* infileName;
    char* outfileName;

    char* infileFormat;
    char* outfileFormat;
    char* outResolution;

    char* targetDeviceType;

    char* gpuCopyMode;

    mfxU32 srcFourCC;
    mfxU32 dstFourCC;

    mfxU32 maxFrames;
    mfxU32 srcWidth;
    mfxU32 srcHeight;
    mfxU32 dstWidth;
    mfxU32 dstHeight;
    mfxU32 timeout;
    mfxU32 frameRate;
    mfxU32 enableCinterface;

    mfxU32 srcbsbufSize;
    mfxU32 dstbsbufSize;

    // encoder specific
    mfxU32 bitRate;
    mfxU32 targetUsage;
    mfxU32 brcMode;
    mfxU32 gopSize;
    mfxU32 keyFrameDist;
    mfxU32 qp;

    // jpeg encoder specific
    mfxU32 quality;

    mfxU32 targetDevice;

    mfxU32 gpuCopy;

    // cropping
    mfxU32 srcCropX;
    mfxU32 srcCropY;
    mfxU32 srcCropW;
    mfxU32 srcCropH;
    mfxU32 dstCropX;
    mfxU32 dstCropY;
    mfxU32 dstCropW;
    mfxU32 dstCropH;

    MemoryMode memoryMode;
    DispatcherMode dispatcherMode;

    mfxU32 outWidth;
    mfxU32 outHeight;
} Params;

// vpl-new-dispatcher.cpp
mfxStatus InitNewDispatcher(WSType wsType, Params* params, mfxSession* session);

#endif // TOOLS_CLI_VPL_COMMON_H_
