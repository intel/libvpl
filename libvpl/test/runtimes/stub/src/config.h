/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef LIBVPL_TEST_RUNTIMES_STUB_SRC_CONFIG_H_
#define LIBVPL_TEST_RUNTIMES_STUB_SRC_CONFIG_H_

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "vpl/mfx.h"

#if defined(__linux__)
    #define vsprintf_s(s, l, m, a) vsprintf(s, m, a)
#endif

struct _mfxSession {
    mfxU32 handleType;

    _mfxSession() {
        handleType = 0;
    }
};

#endif // LIBVPL_TEST_RUNTIMES_STUB_SRC_CONFIG_H_
