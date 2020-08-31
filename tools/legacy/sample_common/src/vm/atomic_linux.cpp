/*###########################################################################
  # Copyright (C) 2005-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#include "vm/atomic_defs.h"

static mfxU16 msdk_atomic_add16(volatile mfxU16 *mem, mfxU16 val) {
    asm volatile("lock; xaddw %0,%1"
                 : "=r"(val), "=m"(*mem)
                 : "0"(val), "m"(*mem)
                 : "memory", "cc");
    return val;
}

static mfxU32 msdk_atomic_add32(volatile mfxU32 *mem, mfxU32 val) {
    asm volatile("lock; xaddl %0,%1"
                 : "=r"(val), "=m"(*mem)
                 : "0"(val), "m"(*mem)
                 : "memory", "cc");
    return val;
}

mfxU16 msdk_atomic_inc16(volatile mfxU16 *pVariable) {
    return msdk_atomic_add16(pVariable, 1) + 1;
}

/* Thread-safe 16-bit variable decrementing */
mfxU16 msdk_atomic_dec16(volatile mfxU16 *pVariable) {
    return msdk_atomic_add16(pVariable, (mfxU16)-1) + 1;
}

mfxU32 msdk_atomic_inc32(volatile mfxU32 *pVariable) {
    return msdk_atomic_add32(pVariable, 1) + 1;
}

/* Thread-safe 16-bit variable decrementing */
mfxU32 msdk_atomic_dec32(volatile mfxU32 *pVariable) {
    return msdk_atomic_add32(pVariable, (mfxU32)-1) + 1;
}
