/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include "sample_utils.h"
#include "vpl/mfxstructures.h"

/*
    Rationale: locks allocator if necessary to get RAW pointers, unlock it at the end
*/
class SurfaceAutoLock {
private:
    SurfaceAutoLock(const SurfaceAutoLock&) {}

public:
    SurfaceAutoLock(mfxFrameAllocator& alloc, mfxFrameSurface1& srf)
            : m_alloc(alloc),
              m_srf(srf),
              m_lockRes(MFX_ERR_NONE),
              m_bLocked() {
        LockFrame();
    }
    operator mfxStatus() {
        return m_lockRes;
    }
    ~SurfaceAutoLock() {
        UnlockFrame();
    }

protected:
    mfxFrameAllocator& m_alloc;
    mfxFrameSurface1& m_srf;
    mfxStatus m_lockRes;
    bool m_bLocked;

    void LockFrame() {
        //no allocator used, no need to do lock
        if (m_srf.Data.Y != 0)
            return;
        //lock required
        m_lockRes = m_alloc.Lock(m_alloc.pthis, m_srf.Data.MemId, &m_srf.Data);
        if (m_lockRes == MFX_ERR_NONE) {
            m_bLocked = true;
        }
    }

    void UnlockFrame() {
        if (m_lockRes != MFX_ERR_NONE || !m_bLocked) {
            return;
        }
        //unlock required
        m_alloc.Unlock(m_alloc.pthis, m_srf.Data.MemId, &m_srf.Data);
    }
};
