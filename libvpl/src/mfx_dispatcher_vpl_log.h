/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef LIBVPL_SRC_MFX_DISPATCHER_VPL_LOG_H_
#define LIBVPL_SRC_MFX_DISPATCHER_VPL_LOG_H_

/* Intel® Video Processing Library (Intel® VPL) Dispatcher Debug Log
 * The debug output of the dispatcher is controlled with the ONEVPL_DISPATCHER_LOG environment variable.
 * To enable log output, set the ONEVPL_DISPATCHER_LOG environment variable value equals to "ON".
 * 
 * By default, Intel® VPL dispatcher prints all log messages to the console.
 * To redirect log output to the desired file, set the ONEVPL_DISPATCHER_LOG_FILE environmental 
 *   variable with the file name of the log file.
 */

#include <stdarg.h>
#include <stdio.h>

#include <string>

#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo.h"

#ifndef __FUNC_NAME__
    #if defined(_WIN32) || defined(_WIN64)
        #define __FUNC_NAME__ __FUNCTION__
    #else
        #define __FUNC_NAME__ __PRETTY_FUNCTION__
    #endif
#endif

class DispatcherLogVPL {
public:
    DispatcherLogVPL();
    ~DispatcherLogVPL();

    mfxStatus Init(mfxU32 logLevel, const std::string &logFileName);
    mfxStatus LogMessage(const char *msdk, ...);

    mfxU32 m_logLevel;

private:
    std::string m_logFileName;
    FILE *m_logFile;
    DispatcherLogVPL(const DispatcherLogVPL &other);
    DispatcherLogVPL &operator=(const DispatcherLogVPL &other);
};

class DispatcherLogVPLFunction {
public:
    DispatcherLogVPLFunction(DispatcherLogVPL *dispLog, const char *fnName)
            : m_dispLog(),
              m_fnName() {
        m_dispLog = dispLog;

        if (m_dispLog && m_dispLog->m_logLevel) {
            m_fnName = fnName;
            m_dispLog->LogMessage("function: %s (enter)", m_fnName.c_str());
        }
    }

    ~DispatcherLogVPLFunction() {
        if (m_dispLog && m_dispLog->m_logLevel)
            m_dispLog->LogMessage("function: %s (return)", m_fnName.c_str());
    }

private:
    DispatcherLogVPL *m_dispLog;
    std::string m_fnName;
    DispatcherLogVPLFunction(const DispatcherLogVPLFunction &other);
    DispatcherLogVPLFunction &operator=(const DispatcherLogVPLFunction &other);
};

#define DISP_LOG_FUNCTION(dispLog) DispatcherLogVPLFunction _dispLogFn(dispLog, __FUNC_NAME__);
#define DISP_LOG_MESSAGE(dispLog, ...)          \
    {                                           \
        if (dispLog) {                          \
            (dispLog)->LogMessage(__VA_ARGS__); \
        }                                       \
    }

#endif // LIBVPL_SRC_MFX_DISPATCHER_VPL_LOG_H_
