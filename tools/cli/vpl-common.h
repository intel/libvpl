/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
#ifndef TOOLS_CLI_VPL_COMMON_H_
#define TOOLS_CLI_VPL_COMMON_H_

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

#define DEFAULT_VERSION_MAJOR 2
#define DEFAULT_VERSION_MINOR 1

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
    WSTYPE_VPPENC,
    WSTYPE_DECENC,

    WSTYPE_COUNT
};

enum InputFrameReadMode {
    INPUT_FRAME_READ_MODE_PITCH = 0,
    INPUT_FRAME_READ_MODE_FRAME,

    INPUT_FRAME_READ_MODE_COUNT
};

enum VppProcessFunctionName { FN_RUNFRAMEVPPASYNC = 0, FN_PROCESSFRAMEASYNC, FN_COUNT };

extern const char* MemoryModeString[MEM_MODE_COUNT];
extern const char* DispatcherModeString[DISPATCHER_MODE_COUNT];
extern const char* InputFrameReadModeString[INPUT_FRAME_READ_MODE_COUNT];

typedef struct _Params {
    char* infileName;
    char* outfileName;

    char* infileFormat;
    char* outfileFormat;
    char* outResolution;

    char* targetDeviceType;

    char* gpuCopyMode;

    bool verboseMode;

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

    // loop counter
    mfxU32 repeat;

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
    mfxIMPL impl;
    InputFrameReadMode inFrameReadMode;

    mfxU32 outWidth;
    mfxU32 outHeight;

    // av1 film-grain denoise
    // -1: default by decoder
    //  0: disable
    //  1: enable (will be ignored if not enabled in the content)
    int filmGrain;

    // process function name
    VppProcessFunctionName vppProcFnName;
} Params;

// vpl-new-dispatcher.cpp
mfxStatus InitNewDispatcher(WSType wsType, Params* params, mfxSession* session);
mfxStatus CloseNewDispatcher(void);

#endif // TOOLS_CLI_VPL_COMMON_H_
