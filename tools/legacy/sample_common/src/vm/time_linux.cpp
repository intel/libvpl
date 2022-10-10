/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#if !defined(_WIN32) && !defined(_WIN64)

    #include <sys/time.h>
    #include "vm/time_defs.h"

    #if !defined(__i386__) && !defined(__x86_64__)
        #include <chrono>
    #else
        #define MSDK_USE_INTRINSIC
        #include <x86intrin.h>
    #endif

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
    mfxU64 result = 0;
    #if defined(MSDK_USE_INTRINSIC)
    result = __rdtsc();
    #else
    result = std::chrono::duration_cast<std::chrono::nanoseconds>(
                 std::chrono::high_resolution_clock::now().time_since_epoch())
                 .count();
    #endif
    return result;
}

#endif // #if !defined(_WIN32) && !defined(_WIN64)
