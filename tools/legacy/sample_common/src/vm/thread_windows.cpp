/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "mfx_samples_config.h"

#if defined(_WIN32) || defined(_WIN64)

    #include <new>
    #include "vm/thread_defs.h"

MSDKSemaphore::MSDKSemaphore(mfxStatus& sts, mfxU32 count) {
    sts         = MFX_ERR_NONE;
    m_semaphore = CreateSemaphore(NULL, count, LONG_MAX, 0);
    if (!m_semaphore)
        throw std::bad_alloc();
}

MSDKSemaphore::~MSDKSemaphore(void) {
    CloseHandle(m_semaphore);
}

mfxStatus MSDKSemaphore::Post(void) {
    return (ReleaseSemaphore(m_semaphore, 1, NULL) == false) ? MFX_ERR_UNKNOWN : MFX_ERR_NONE;
}

mfxStatus MSDKSemaphore::Wait(void) {
    return (WaitForSingleObject(m_semaphore, INFINITE) != WAIT_OBJECT_0) ? MFX_ERR_UNKNOWN
                                                                         : MFX_ERR_NONE;
}

/* ****************************************************************************** */

MSDKEvent::MSDKEvent(mfxStatus& sts, bool manual, bool state) {
    sts     = MFX_ERR_NONE;
    m_event = CreateEvent(NULL, manual, state, NULL);
    if (!m_event)
        throw std::bad_alloc();
}

MSDKEvent::~MSDKEvent(void) {
    CloseHandle(m_event);
}

mfxStatus MSDKEvent::Signal(void) {
    return (SetEvent(m_event) == false) ? MFX_ERR_UNKNOWN : MFX_ERR_NONE;
}

mfxStatus MSDKEvent::Reset(void) {
    return (ResetEvent(m_event) == false) ? MFX_ERR_UNKNOWN : MFX_ERR_NONE;
}

mfxStatus MSDKEvent::Wait(void) {
    return (WaitForSingleObject(m_event, INFINITE) != WAIT_OBJECT_0) ? MFX_ERR_UNKNOWN
                                                                     : MFX_ERR_NONE;
}

mfxStatus MSDKEvent::TimedWait(mfxU32 msec) {
    if (MFX_INFINITE == msec)
        return MFX_ERR_UNSUPPORTED;
    mfxStatus mfx_res = MFX_ERR_NOT_INITIALIZED;
    DWORD res         = WaitForSingleObject(m_event, msec);

    if (WAIT_OBJECT_0 == res)
        mfx_res = MFX_ERR_NONE;
    else if (WAIT_TIMEOUT == res)
        mfx_res = MFX_TASK_WORKING;
    else
        mfx_res = MFX_ERR_UNKNOWN;

    return mfx_res;
}

MSDKThread::MSDKThread(mfxStatus& sts, msdk_thread_callback func, void* arg) {
    sts      = MFX_ERR_NONE;
    m_thread = (void*)_beginthreadex(NULL, 0, func, arg, 0, NULL);
    if (!m_thread)
        throw std::bad_alloc();
}

MSDKThread::~MSDKThread(void) {
    CloseHandle(m_thread);
}

mfxStatus MSDKThread::Wait(void) {
    return (WaitForSingleObject(m_thread, INFINITE) != WAIT_OBJECT_0) ? MFX_ERR_UNKNOWN
                                                                      : MFX_ERR_NONE;
}

mfxStatus MSDKThread::TimedWait(mfxU32 msec) {
    if (MFX_INFINITE == msec)
        return MFX_ERR_UNSUPPORTED;

    mfxStatus mfx_res = MFX_ERR_NONE;
    DWORD res         = WaitForSingleObject(m_thread, msec);

    if (WAIT_OBJECT_0 == res)
        mfx_res = MFX_ERR_NONE;
    else if (WAIT_TIMEOUT == res)
        mfx_res = MFX_TASK_WORKING;
    else
        mfx_res = MFX_ERR_UNKNOWN;

    return mfx_res;
}

mfxStatus MSDKThread::GetExitCode() {
    mfxStatus mfx_res = MFX_ERR_NOT_INITIALIZED;

    DWORD code = 0;
    int sts    = 0;
    sts        = GetExitCodeThread(m_thread, &code);

    if (sts == 0)
        mfx_res = MFX_ERR_UNKNOWN;
    else if (STILL_ACTIVE == code)
        mfx_res = MFX_TASK_WORKING;
    else
        mfx_res = MFX_ERR_NONE;

    return mfx_res;
}

mfxU32 msdk_get_current_pid() {
    return GetCurrentProcessId();
}

#endif // #if defined(_WIN32) || defined(_WIN64)
