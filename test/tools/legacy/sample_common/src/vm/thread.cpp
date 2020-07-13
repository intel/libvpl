/*###########################################################################
  # Copyright (C) 2005-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#include <new> // std::bad_alloc

#include "vm/thread_defs.h"

AutomaticMutex::AutomaticMutex(MSDKMutex& mutex)
        : m_rMutex(mutex),
          m_bLocked(false){
              // if (MFX_ERR_NONE != Lock())
              //     throw std::bad_alloc();
          };
AutomaticMutex::~AutomaticMutex(void) {
    Unlock();
}

mfxStatus AutomaticMutex::Lock(void) {
    mfxStatus sts = MFX_ERR_NONE;
    if (!m_bLocked) {
        if (!m_rMutex.Try()) {
            // add time measurement here to estimate how long you sleep on mutex...
            sts = m_rMutex.Lock();
        }
        m_bLocked = true;
    }
    return sts;
}

mfxStatus AutomaticMutex::Unlock(void) {
    mfxStatus sts = MFX_ERR_NONE;
    if (m_bLocked) {
        sts       = m_rMutex.Unlock();
        m_bLocked = false;
    }
    return sts;
}
