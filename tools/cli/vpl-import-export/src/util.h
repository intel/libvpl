//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_UTIL_H_
#define TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./defs.h"
#include "./vpl-timing.h"

#if defined(_WIN32) || defined(_WIN64)

#include <atlbase.h>
#include <conio.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <windows.h>
#include <windowsx.h>

#elif defined __linux__

#include <fcntl.h>
#include <unistd.h>

#include "va/va.h"
#include "va/va_drm.h"

#include "va/va_drmcommon.h"

#ifdef TOOLS_ENABLE_X11
#include "X11/Xutil.h"
#include "va/va_x11.h"
#endif

#endif

// decode-vpp.cpp
int RunDecodeVPP(Params *params, FileInfo *fileInfo);

// encode.cpp
int RunEncode(Params *params, FileInfo *fileInfo);

// util.cpp
void Usage(void);
bool ParseArgsAndValidate(int argc, char *argv[], Params *params);
const char *FourCCToString(mfxU32 fourCC);
mfxStatus CreateVPLSession(VPLSession &vplSession);
mfxStatus ReadEncodedStream(mfxBitstream &bs, std::ifstream &f);
void WriteEncodedStream(mfxBitstream &bs, std::ofstream &f);
mfxStatus WriteRawFrame(mfxFrameSurface1 *surface, std::ofstream &f);
mfxStatus WriteRawFrameCPU_NV12(CPUFrameInfo_NV12 &cpuFrameInfo, std::ofstream &f);
mfxStatus ReadRawFrame(mfxFrameSurface1 *surface, std::ifstream &f);
mfxStatus ReadRawFrameCPU_NV12(CPUFrameInfo_NV12 &cpuFrameInfo, std::ifstream &f);
bool CheckKB_Quit(void);

// debug.cpp
void DebugDumpNativeSurfaceDesc(mfxFrameSurface1 *pSrf, bool bRunOnce = true);
void DebugDumpHardwareInterface(void);
const char *DebugGetStringSurfaceFlags(mfxU32 surfaceFlags);

#endif // TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_UTIL_H_
