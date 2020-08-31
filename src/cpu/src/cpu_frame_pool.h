/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_CPU_SRC_CPU_FRAME_POOL_H_
#define SRC_CPU_SRC_CPU_FRAME_POOL_H_

#include <memory>
#include <vector>
#include "src/cpu_common.h"
#include "src/cpu_frame.h"

class CpuFramePool {
public:
    CpuFramePool() : m_info({}) {}

    mfxStatus Init(mfxU32 nPoolSize);
    mfxStatus Init(mfxU32 FourCC,
                   mfxU32 width,
                   mfxU32 height,
                   mfxU32 nPoolSize);
    mfxStatus GetFreeSurface(mfxFrameSurface1** surface);

private:
    std::vector<std::unique_ptr<CpuFrame>> m_surfaces;
    mfxFrameInfo m_info;
};

#endif // SRC_CPU_SRC_CPU_FRAME_POOL_H_
