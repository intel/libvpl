/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/cpu_workstream.h"
#include "src/cpu_common.h"

CpuWorkstream::CpuWorkstream() : m_allocator({}) {
    av_log_set_level(AV_LOG_QUIET);
}

CpuWorkstream::~CpuWorkstream() {}

mfxStatus CpuWorkstream::Sync(mfxSyncPoint &syncp, mfxU32 wait) {
    return MFX_ERR_NONE;
}
