/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_CPU_SRC_FRAME_LOCK_H_
#define SRC_CPU_SRC_FRAME_LOCK_H_

#include "src/cpu_common.h"

class FrameLock {
public:
    FrameLock();
    ~FrameLock();

    mfxStatus Lock(mfxFrameSurface1 *surface,
                   mfxU32 flags                 = 0,
                   mfxFrameAllocator *allocator = nullptr);
    void Unlock();

    mfxFrameData *GetData();
    AVFrame *GetAVFrame(mfxFrameSurface1 *surface,
                        mfxU32 flags                 = 0,
                        mfxFrameAllocator *allocator = nullptr);

private:
    mfxFrameSurface1 *m_surface;
    mfxFrameAllocator *m_allocator;
    bool m_newapi;
    mfxFrameData *m_data;
    mfxFrameData m_locked_data;
    mfxMemId mem_id;
    AVFrame *m_avframe;
};

#endif // SRC_CPU_SRC_FRAME_LOCK_H_
