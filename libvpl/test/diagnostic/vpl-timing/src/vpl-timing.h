/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef LIBVPL_TEST_DIAGNOSTIC_VPL_TIMING_SRC_VPL_TIMING_H_
#define LIBVPL_TEST_DIAGNOSTIC_VPL_TIMING_SRC_VPL_TIMING_H_

#include <chrono>
#include <string>

#define ENABLE_VPL_LOG_TIME

#ifdef ENABLE_VPL_LOG_TIME

class VPLLogTiming {
public:
    explicit VPLLogTiming(const char *logStr) : m_logString(), m_startTime() {
        m_logString = logStr;
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    ~VPLLogTiming() {}

    void PrintElapsedTime() {
        std::chrono::high_resolution_clock::time_point endTime =
            std::chrono::high_resolution_clock::now();

        std::chrono::microseconds diff =
            std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_startTime);
        fprintf(stdout,
                "vpl-timing -- %-48s = % 8.2f msec\n",
                m_logString.c_str(),
                diff.count() / 1000.0f);
    }

private:
    std::string m_logString;
    std::chrono::high_resolution_clock::time_point m_startTime;
};

    #define VPL_LOG_TIME_START(t, str) VPLLogTiming unique_prefix_log_time_##t(str);
    #define VPL_LOG_TIME_END(t)        unique_prefix_log_time_##t.PrintElapsedTime();

#else

    #define VPL_LOG_TIME_START(t, str)
    #define VPL_LOG_TIME_END(t)

#endif

#endif // LIBVPL_TEST_DIAGNOSTIC_VPL_TIMING_SRC_VPL_TIMING_H_
