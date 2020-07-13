/*############################################################################
  # Copyright (C) 2017-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_DISPATCHER_LINUX_MFXLOADER_H_
#define SRC_DISPATCHER_LINUX_MFXLOADER_H_

#include <limits.h>

#include <list>
#include <sstream>
#include <string>

#include "vpl/mfxdefs.h"

inline bool operator<(const mfxVersion& lhs, const mfxVersion& rhs) {
    return (lhs.Major < rhs.Major ||
            (lhs.Major == rhs.Major && lhs.Minor < rhs.Minor));
}

inline bool operator<=(const mfxVersion& lhs, const mfxVersion& rhs) {
    return (lhs < rhs || (lhs.Major == rhs.Major && lhs.Minor == rhs.Minor));
}

#endif // SRC_DISPATCHER_LINUX_MFXLOADER_H_
