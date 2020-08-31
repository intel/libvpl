/*###########################################################################
  # Copyright (C) 2005-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#include "mfx_samples_config.h"

#if defined(_WIN32) || defined(_WIN64)

    #include "vm/time_defs.h"

msdk_tick msdk_time_get_tick(void) {
    LARGE_INTEGER t1;

    QueryPerformanceCounter(&t1);
    return t1.QuadPart;
}

msdk_tick msdk_time_get_frequency(void) {
    LARGE_INTEGER t1;

    QueryPerformanceFrequency(&t1);
    return t1.QuadPart;
}

mfxU64 rdtsc() {
    return __rdtsc();
}

#endif // #if defined(_WIN32) || defined(_WIN64)
