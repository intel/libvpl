/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <gtest/gtest.h>

#include "src/dispatcher_common.h"

TEST(Dispatcher_SW_CreateSession, SimpleConfigCanCreateSession) {
    SKIP_IF_DISP_SW_DISABLED();
    Dispatcher_CreateSession_SimpleConfigCanCreateSession(MFX_IMPL_TYPE_SOFTWARE);
}
