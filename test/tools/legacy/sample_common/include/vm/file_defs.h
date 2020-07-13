/*###########################################################################
  # Copyright (C) 2005-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#ifndef __FILE_DEFS_H__
#define __FILE_DEFS_H__

#include "vpl/mfxdefs.h"

#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)

    #define MSDK_FOPEN(file, name, mode)         \
        if (0 != _tfopen_s(&file, name, mode)) { \
            file = NULL;                         \
        }

    #define msdk_fgets _fgetts
#else // #if defined(_WIN32) || defined(_WIN64)
    #include <unistd.h>

    #define MSDK_FOPEN(file, name, mode) !(file = fopen(name, mode))

    #define msdk_fgets fgets
#endif // #if defined(_WIN32) || defined(_WIN64)

#endif // #ifndef __FILE_DEFS_H__
