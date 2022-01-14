/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "src/dispatcher_common.h"

TEST(Dispatcher_GPU_CreateSession, SimpleConfigCanCreateSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_SimpleConfigCanCreateSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, SetValidNumThreadCreatesSession) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_SetValidNumThreadCreatesSession(MFX_IMPL_TYPE_HARDWARE);
}

TEST(Dispatcher_GPU_CreateSession, SetInvalidNumThreadTypeReturnsErrUnsupported) {
    SKIP_IF_DISP_GPU_DISABLED();
    Dispatcher_CreateSession_SetInvalidNumThreadTypeReturnsErrUnsupported(MFX_IMPL_TYPE_HARDWARE);
}
