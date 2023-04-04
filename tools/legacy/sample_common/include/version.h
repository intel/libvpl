/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include "sample_defs.h"

#ifndef MSDK_MAJOR
    #define MSDK_MAJOR 8
#endif

#ifndef MSDK_MINOR
    #define MSDK_MINOR 4
#endif

#ifndef MSDK_RELEASE
    #define MSDK_TARGETAPIMINOR 27
#endif

#ifndef MSDK_BUILD
    #define MSDK_BUILD 0
#endif

static std::string GetMSDKSampleVersion() {
    std::stringstream ss;
    ss << MSDK_MAJOR << "." << MSDK_MINOR << "." << MSDK_TARGETAPIMINOR << "." << MSDK_BUILD;
    return ss.str();
}
