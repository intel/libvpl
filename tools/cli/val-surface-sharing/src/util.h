//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef TOOLS_CLI_VAL_SURFACE_SHARING_SRC_UTIL_H_
#define TOOLS_CLI_VAL_SURFACE_SHARING_SRC_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./defs.h"

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

#endif

// decode-vpp.cpp
int RunDecodeVPP(Options *opts, FileInfo *fileInfo);

// encode.cpp
int RunEncode(Options *opts, FileInfo *fileInfo);

// util.cpp
void Usage(void);
bool ParseArgsAndValidate(int argc, char *argv[], Options *opts);
mfxStatus GetAdaptersInfo(Options *opts, bool bPrint = false);
void ShowTestInfo(Options *opts);
const char *FourCCToString(mfxU32 fourCC);
mfxStatus ReadEncodedStream(mfxBitstream &bs, std::ifstream &f);
void WriteEncodedStream(mfxBitstream &bs, std::ofstream &f);
#ifdef _WIN32
mfxStatus WriteRawFrame(ID3D11Device *pDevice, ID3D11Texture2D *pTex2D, std::ofstream &f);
#else
mfxStatus WriteRawFrame(VADisplay vaDisplay, VASurfaceID vaSurfaceID, std::ofstream &f);
#endif
mfxStatus WriteRawFrame(mfxFrameSurface1 *surface, std::ofstream &f);
mfxStatus ReadRawFrame(FrameInfo fi, mfxU8 *bs, std::ifstream &f);
mfxStatus ReadRawFrame(mfxFrameSurface1 *surface, std::ifstream &f);
bool CheckKB_Quit(void);

#endif // TOOLS_CLI_VAL_SURFACE_SHARING_SRC_UTIL_H_
