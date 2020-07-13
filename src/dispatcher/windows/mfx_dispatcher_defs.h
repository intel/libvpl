/*############################################################################
  # Copyright (C) 2013-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once
#include <cstdio>
#include <cstring>
#include "vpl/mfxdefs.h"

#if defined(MFX_DISPATCHER_LOG)
    #include <string.h>
    #include <string>
#endif

#define MAX_PLUGIN_PATH 4096
#define MAX_PLUGIN_NAME 4096

#if _MSC_VER < 1400
    #define wcscpy_s(to, to_size, from) wcscpy(to, from)
    #define wcscat_s(to, to_size, from) wcscat(to, from)
#endif

// declare library module's handle
typedef void* mfxModuleHandle;

typedef void(MFX_CDECL* mfxFunctionPointer)(void);
