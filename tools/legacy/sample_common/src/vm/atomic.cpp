/*###########################################################################
  # Copyright (C) 2005-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#if defined(_WIN32) || defined(_WIN64)

    #include "vm/atomic_defs.h"

    //#define _interlockedbittestandset      fake_set
    //#define _interlockedbittestandreset    fake_reset
    //#define _interlockedbittestandset64    fake_set64
    //#define _interlockedbittestandreset64  fake_reset64
    #include <intrin.h>
    //#undef _interlockedbittestandset
    //#undef _interlockedbittestandreset
    //#undef _interlockedbittestandset64
    //#undef _interlockedbittestandreset64
    #pragma intrinsic(_InterlockedIncrement16)
    #pragma intrinsic(_InterlockedDecrement16)
    #pragma intrinsic(_InterlockedIncrement)
    #pragma intrinsic(_InterlockedDecrement)

mfxU16 msdk_atomic_inc16(volatile mfxU16 *pVariable) {
    return _InterlockedIncrement16((volatile short *)pVariable);
}

/* Thread-safe 16-bit variable decrementing */
mfxU16 msdk_atomic_dec16(volatile mfxU16 *pVariable) {
    return _InterlockedDecrement16((volatile short *)pVariable);
}

mfxU32 msdk_atomic_inc32(volatile mfxU32 *pVariable) {
    return _InterlockedIncrement((volatile long *)pVariable);
}

/* Thread-safe 16-bit variable decrementing */
mfxU32 msdk_atomic_dec32(volatile mfxU32 *pVariable) {
    return _InterlockedDecrement((volatile long *)pVariable);
}

#endif // #if defined(_WIN32) || defined(_WIN64)
