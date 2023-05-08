/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __THREAD_DEFS_H__
#define __THREAD_DEFS_H__

#include "vm/strings_defs.h"
#include "vpl/mfxdefs.h"

typedef unsigned int(MFX_STDCALL* msdk_thread_callback)(void*);

#if defined(_WIN32) || defined(_WIN64)

    #include <process.h>
    #include <windows.h>

struct msdkSemaphoreHandle {
    void* m_semaphore;
};

struct msdkEventHandle {
    void* m_event;
};

struct msdkThreadHandle {
    void* m_thread;
};

#else // #if defined(_WIN32) || defined(_WIN64)

    #include <errno.h>
    #include <pthread.h>
    #include <sys/resource.h>
    #include <sys/time.h>

struct msdkSemaphoreHandle {
    msdkSemaphoreHandle(mfxU32 count) : m_count(count) {}

    mfxU32 m_count;
    pthread_cond_t m_semaphore;
    pthread_mutex_t m_mutex;
};

struct msdkEventHandle {
    msdkEventHandle(bool manual, bool state) : m_manual(manual), m_state(state) {}

    bool m_manual;
    bool m_state;
    pthread_cond_t m_event;
    pthread_mutex_t m_mutex;
};

class MSDKEvent;

struct msdkThreadHandle {
    msdkThreadHandle(msdk_thread_callback func, void* arg)
            : m_func(func),
              m_arg(arg),
              m_event(0),
              m_thread(0) {}

    msdk_thread_callback m_func;
    void* m_arg;
    MSDKEvent* m_event;
    pthread_t m_thread;
};

#endif // #if defined(_WIN32) || defined(_WIN64)

class MSDKSemaphore : public msdkSemaphoreHandle {
public:
    MSDKSemaphore(mfxStatus& sts, mfxU32 count = 0);
    ~MSDKSemaphore(void);

    mfxStatus Post(void);
    mfxStatus Wait(void);

private:
    MSDKSemaphore(const MSDKSemaphore&);
    void operator=(const MSDKSemaphore&);
};

class MSDKEvent : public msdkEventHandle {
public:
    MSDKEvent(mfxStatus& sts, bool manual, bool state);
    ~MSDKEvent(void);

    mfxStatus Signal(void);
    mfxStatus Reset(void);
    mfxStatus Wait(void);
    mfxStatus TimedWait(mfxU32 msec);

private:
    MSDKEvent(const MSDKEvent&);
    void operator=(const MSDKEvent&);
};

class MSDKThread : public msdkThreadHandle {
public:
    MSDKThread(mfxStatus& sts, msdk_thread_callback func, void* arg);
    ~MSDKThread(void);

    mfxStatus Wait(void);
    mfxStatus TimedWait(mfxU32 msec);
    mfxStatus GetExitCode();

#if !defined(_WIN32) && !defined(_WIN64)
    friend void* msdk_thread_start(void* arg);
#endif

private:
    MSDKThread(const MSDKThread&);
    void operator=(const MSDKThread&);
};

mfxU32 msdk_get_current_pid();
mfxStatus msdk_setrlimit_vmem(mfxU64 size);
mfxStatus msdk_thread_get_schedtype(const char*, mfxI32& type);
void msdk_thread_printf_scheduling_help();

#endif //__THREAD_DEFS_H__
