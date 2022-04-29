/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#define MFXVIDEO_CPP_USE_DEPRECATED

#ifndef MFX_DEPRECATED_OFF
    #define MFX_DEPRECATED_OFF // disable compile warnings for use of MFXInit, MFXInitEx
#endif

#define LEGACY_TEST_VER 1

#include "./legacycpp-session-test.cpp" // NOLINT(build/include)
