/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#if !defined(_WIN32) && !defined(_WIN64)

    #include <sched.h>
    #include <stdio.h> // setrlimit
    #include <sys/syscall.h>
    #include <unistd.h>
    #include <new> // std::bad_alloc

    #include "sample_utils.h"
    #include "vm/thread_defs.h"

MSDKSemaphore::MSDKSemaphore(mfxStatus& sts, mfxU32 count) : msdkSemaphoreHandle(count) {
    sts     = MFX_ERR_NONE;
    int res = pthread_cond_init(&m_semaphore, NULL);
    if (res) {
        // If pthread_cond_init reports an error m_semaphore was not allocated
        throw std::bad_alloc();
    }
    res = pthread_mutex_init(&m_mutex, NULL);
    if (res) {
        if (!pthread_cond_destroy(&m_semaphore)) {
            // do nothing
        }
        throw std::bad_alloc();
    }
}

MSDKSemaphore::~MSDKSemaphore(void) {
    if (!pthread_mutex_destroy(&m_mutex)) {
        // do nothing
    }
    if (!pthread_cond_destroy(&m_semaphore)) {
        // do nothing
    }
}

mfxStatus MSDKSemaphore::Post(void) {
    int res = pthread_mutex_lock(&m_mutex);
    if (!res) {
        if (0 == m_count++) {
            res = pthread_cond_signal(&m_semaphore);
            if (!res) {
                // do nothing
            }
        }
    }
    int sts = pthread_mutex_unlock(&m_mutex);
    if (!sts) {
        // do nothing
    }
    if (!res)
        res = sts;
    return (res) ? MFX_ERR_UNKNOWN : MFX_ERR_NONE;
}

mfxStatus MSDKSemaphore::Wait(void) {
    int res = pthread_mutex_lock(&m_mutex);
    if (!res) {
        while (!m_count) {
            res = pthread_cond_wait(&m_semaphore, &m_mutex);
            if (!res) {
                // do nothing
            }
        }
        if (!res)
            --m_count;
        int sts = pthread_mutex_unlock(&m_mutex);
        if (!sts) {
            // do nothing
        }
        if (!res)
            res = sts;
    }
    return (res) ? MFX_ERR_UNKNOWN : MFX_ERR_NONE;
}

/* ****************************************************************************** */

MSDKEvent::MSDKEvent(mfxStatus& sts, bool manual, bool state) : msdkEventHandle(manual, state) {
    sts = MFX_ERR_NONE;
    //initialize m_event condition variable
    int res = pthread_cond_init(&m_event, NULL);

    //zero result indicates success
    if (!res) {
        //if successful, initialize the mutex
        res = pthread_mutex_init(&m_mutex, NULL);

        //zero result indicates success
        //if not successful, destroy m_event
        if (res) {
            if (!pthread_cond_destroy(&m_event)) {
                // do nothing
            }
        }
    }
    else {
        //non-zero means something is wrong, throw bad_alloc
        throw std::bad_alloc();
    }
}

MSDKEvent::~MSDKEvent(void) {
    if (!pthread_mutex_destroy(&m_mutex)) {
        // do nothing
    }
    if (!pthread_cond_destroy(&m_event)) {
        // do nothing
    }
}

mfxStatus MSDKEvent::Signal(void) {
    int res = pthread_mutex_lock(&m_mutex);
    if (!res) {
        if (!m_state) {
            m_state = true;
            if (m_manual)
                res = pthread_cond_broadcast(&m_event);
            else
                res = pthread_cond_signal(&m_event);
            if (!res) {
                // do nothing
            }
        }
        int sts = pthread_mutex_unlock(&m_mutex);
        if (!sts) {
            // do nothing
        }
        if (!res)
            res = sts;
    }
    return (res) ? MFX_ERR_UNKNOWN : MFX_ERR_NONE;
}

mfxStatus MSDKEvent::Reset(void) {
    int res = pthread_mutex_lock(&m_mutex);
    if (!res) {
        if (m_state)
            m_state = false;
        res = pthread_mutex_unlock(&m_mutex);
    }
    return (res) ? MFX_ERR_UNKNOWN : MFX_ERR_NONE;
}

mfxStatus MSDKEvent::Wait(void) {
    int res = pthread_mutex_lock(&m_mutex);
    if (!res) {
        while (!m_state) {
            res = pthread_cond_wait(&m_event, &m_mutex);
            if (!res) {
                // do nothing
            }
        }
        if (!m_manual)
            m_state = false;
        int sts = pthread_mutex_unlock(&m_mutex);
        if (!sts) {
            // do nothing
        }
        if (!res)
            res = sts;
    }
    return (res) ? MFX_ERR_UNKNOWN : MFX_ERR_NONE;
}

mfxStatus MSDKEvent::TimedWait(mfxU32 msec) {
    if (MFX_INFINITE == msec)
        return MFX_ERR_UNSUPPORTED;
    mfxStatus mfx_res = MFX_ERR_NOT_INITIALIZED;

    int res = pthread_mutex_lock(&m_mutex);
    if (!res) {
        if (!m_state) {
            struct timeval tval;
            struct timespec tspec;
            mfxI32 res;

            gettimeofday(&tval, NULL);
            msec          = 1000 * msec + tval.tv_usec;
            tspec.tv_sec  = tval.tv_sec + msec / 1000000;
            tspec.tv_nsec = (msec % 1000000) * 1000;
            res           = pthread_cond_timedwait(&m_event, &m_mutex, &tspec);
            if (!res)
                mfx_res = MFX_ERR_NONE;
            else if (ETIMEDOUT == res)
                mfx_res = MFX_TASK_WORKING;
            else
                mfx_res = MFX_ERR_UNKNOWN;
        }
        else
            mfx_res = MFX_ERR_NONE;
        if (!m_manual)
            m_state = false;

        res = pthread_mutex_unlock(&m_mutex);
        if (res)
            mfx_res = MFX_ERR_UNKNOWN;
    }
    else
        mfx_res = MFX_ERR_UNKNOWN;

    return mfx_res;
}

/* ****************************************************************************** */

void* msdk_thread_start(void* arg) {
    if (arg) {
        MSDKThread* thread = (MSDKThread*)arg;

        if (thread->m_func)
            thread->m_func(thread->m_arg);
        thread->m_event->Signal();
    }
    return NULL;
}

/* ****************************************************************************** */

MSDKThread::MSDKThread(mfxStatus& sts, msdk_thread_callback func, void* arg)
        : msdkThreadHandle(func, arg) {
    m_event = new MSDKEvent(sts, false, false);
    if (pthread_create(&(m_thread), NULL, msdk_thread_start, this)) {
        delete (m_event);
        throw std::bad_alloc();
    }
}

MSDKThread::~MSDKThread(void) {
    delete m_event;
}

mfxStatus MSDKThread::Wait(void) {
    int res = pthread_join(m_thread, NULL);
    return (res) ? MFX_ERR_UNKNOWN : MFX_ERR_NONE;
}

mfxStatus MSDKThread::TimedWait(mfxU32 msec) {
    if (MFX_INFINITE == msec)
        return MFX_ERR_UNSUPPORTED;

    mfxStatus mfx_res = m_event->TimedWait(msec);

    if (MFX_ERR_NONE == mfx_res) {
        return (pthread_join(m_thread, NULL)) ? MFX_ERR_UNKNOWN : MFX_ERR_NONE;
    }
    return mfx_res;
}

mfxStatus MSDKThread::GetExitCode() {
    if (!m_event)
        return MFX_ERR_NOT_INITIALIZED;

    /** @todo: Need to add implementation. */
    return MFX_ERR_NONE;
}

/* ****************************************************************************** */

mfxStatus msdk_setrlimit_vmem(mfxU64 size) {
    struct rlimit limit;

    limit.rlim_cur = size;
    limit.rlim_max = size;
    if (setrlimit(RLIMIT_AS, &limit))
        return MFX_ERR_UNKNOWN;
    return MFX_ERR_NONE;
}

mfxStatus msdk_thread_get_schedtype(const char* str, mfxI32& type) {
    if (msdk_match(str, "fifo")) {
        type = SCHED_FIFO;
    }
    else if (msdk_match(str, "rr")) {
        type = SCHED_RR;
    }
    else if (msdk_match(str, "other")) {
        type = SCHED_OTHER;
    }
    else if (msdk_match(str, "batch")) {
        type = SCHED_BATCH;
    }
    else if (msdk_match(str, "idle")) {
        type = SCHED_IDLE;
    }
    else {
        return MFX_ERR_UNSUPPORTED;
    }
    return MFX_ERR_NONE;
}

void msdk_thread_printf_scheduling_help() {
    printf("Note on the scheduling types and priorities:\n");
    printf("  - <sched_type>: <priority_min> .. <priority_max> (notes)\n");
    printf("The following scheduling types requires root privileges:\n");
    printf("  - fifo: %d .. %d (static priority: low .. high)\n",
           sched_get_priority_min(SCHED_FIFO),
           sched_get_priority_max(SCHED_FIFO));
    printf("  - rr: %d .. %d (static priority: low .. high)\n",
           sched_get_priority_min(SCHED_RR),
           sched_get_priority_max(SCHED_RR));
    printf("The following scheduling types can be used by non-privileged users:\n");
    printf("  - other: 0 .. 0 (static priority always 0)\n");
    printf("  - batch: 0 .. 0 (static priority always 0)\n");
    printf("  - idle: n/a\n");
    printf("If you want to adjust priority for the other or batch scheduling type,\n");
    printf("you can do that process-wise using dynamic priority - so called nice value.\n");
    printf("Range for the nice value is: %d .. %d (high .. low)\n", PRIO_MIN, PRIO_MAX);
    printf("Please, see 'man(1) nice' for details.\n");
}

mfxU32 msdk_get_current_pid() {
    return syscall(SYS_getpid);
}

#endif // #if !defined(_WIN32) && !defined(_WIN64)
