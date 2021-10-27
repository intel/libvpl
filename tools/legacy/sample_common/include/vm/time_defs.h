/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __TIME_DEFS_H__
#define __TIME_DEFS_H__

#include "mfx_itt_trace.h"
#include "vpl/mfxdefs.h"

#if defined(_WIN32) || defined(_WIN64)

    #include <windows.h>

    #define MSDK_SLEEP(msec) Sleep(msec)

    #define MSDK_USLEEP(usec)                                     \
        {                                                         \
            LARGE_INTEGER due;                                    \
            due.QuadPart = -(10 * (int)usec);                     \
            HANDLE t     = CreateWaitableTimer(NULL, TRUE, NULL); \
            SetWaitableTimer(t, &due, 0, NULL, NULL, 0);          \
            WaitForSingleObject(t, INFINITE);                     \
            CloseHandle(t);                                       \
        }

#else // #if defined(_WIN32) || defined(_WIN64)

    #include <unistd.h>

    #define MSDK_SLEEP(msec)            \
        do {                            \
            MFX_ITT_TASK("MSDK_SLEEP"); \
            usleep(1000 * msec);        \
        } while (0)

    #define MSDK_USLEEP(usec)            \
        do {                             \
            MFX_ITT_TASK("MSDK_USLEEP"); \
            usleep(usec);                \
        } while (0)

#endif // #if defined(_WIN32) || defined(_WIN64)

#define MSDK_GET_TIME(T, S, F) ((mfxF64)((T) - (S)) / (mfxF64)(F))

typedef mfxI64 msdk_tick;

msdk_tick msdk_time_get_tick(void);
msdk_tick msdk_time_get_frequency(void);
mfxU64 rdtsc(void);

#endif // #ifndef __TIME_DEFS_H__
