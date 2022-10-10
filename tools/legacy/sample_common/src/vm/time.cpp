/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "mfx_samples_config.h"

#if defined(_WIN32) || defined(_WIN64)

    #include "vm/time_defs.h"

    #if !defined(__i386__) && !defined(__x86_64__)
        #include <chrono>
    #else
        #define MSDK_USE_INTRINSIC
        #include <x86intrin.h>
    #endif

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
    #if defined(MSDK_USE_INTRINSIC)
    return __rdtsc();
    #else
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
    #endif
}

#endif // #if defined(_WIN32) || defined(_WIN64)
