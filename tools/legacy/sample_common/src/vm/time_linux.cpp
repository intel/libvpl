/*###########################################################################
  # Copyright (C) 2005-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#include <sys/time.h>
#include "vm/time_defs.h"

#define MSDK_TIME_MHZ 1000000

msdk_tick msdk_time_get_tick(void) {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (msdk_tick)tv.tv_sec * (msdk_tick)MSDK_TIME_MHZ + (msdk_tick)tv.tv_usec;
}

msdk_tick msdk_time_get_frequency(void) {
    return (msdk_tick)MSDK_TIME_MHZ;
}

mfxU64 rdtsc(void) {
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((mfxU64)hi << 32) | lo;
}
