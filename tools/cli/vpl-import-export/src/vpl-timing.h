/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_VPL_TIMING_H_
#define TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_VPL_TIMING_H_

#include <stdint.h>

#include <chrono>
#include <string>

#define ENABLE_VPL_TIMING

#ifdef ENABLE_VPL_TIMING

class VPLTiming {
public:
    explicit VPLTiming(const char *logStr) : m_logString(), m_startTime(), m_totalTime(), m_totalCount(0) {
        if (logStr)
            m_logString = logStr;
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    ~VPLTiming() {}

    __inline std::chrono::microseconds GetDiffTime(void) {
        std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
        std::chrono::microseconds diff                         = std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_startTime);
        return diff;
    }

    __inline void StartTimer() {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    __inline void EndTimer() {
        // accumulator runs after conversion to usec, could also sum in time_point's if it's more accurate
        m_totalTime += GetDiffTime();
        m_totalCount++;
    }

    void PrintElapsedTime() {
        std::chrono::microseconds diff = GetDiffTime();
        fprintf(stdout, "Elapsed Time -- %-48s = % 8.2f msec\n", m_logString.c_str(), diff.count() / 1000.0f);
    }

    float GetTotalTime() {
        return (float)(m_totalTime.count() / 1000.0f);
    }

    uint64_t GetTotalCount() {
        return m_totalCount;
    }

private:
    std::string m_logString;
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::microseconds m_totalTime;
    uint64_t m_totalCount;
};

// basic usage - just measure and print the difference between START and END
#define VPL_BASIC_TIME_START(t, str) VPLTiming unique_prefix_log_time_##t(str);
#define VPL_BASIC_TIME_END(t)        unique_prefix_log_time_##t.PrintElapsedTime();

// tracking usage - keep a running total of the time differences
#define VPL_TOTAL_TIME_INIT(t)  VPLTiming unique_prefix_log_time_##t("");
#define VPL_TOTAL_TIME_START(t) unique_prefix_log_time_##t.StartTimer();
#define VPL_TOTAL_TIME_STOP(t)  unique_prefix_log_time_##t.EndTimer();
#define VPL_TOTAL_TIME_CHECK(t, fTime, uCount)               \
    {                                                        \
        fTime  = unique_prefix_log_time_##t.GetTotalTime();  \
        uCount = unique_prefix_log_time_##t.GetTotalCount(); \
    }

#else

#define VPL_BASIC_TIME_START(t, str)
#define VPL_BASIC_TIME_END(t)

// tracking usage - keep a running total of the time differences
#define VPL_TOTAL_TIME_INIT(t)
#define VPL_TOTAL_TIME_START(t)
#define VPL_TOTAL_TIME_STOP(t)
#define VPL_TOTAL_TIME_CHECK(t, fTime, uCount)

#endif

#endif // TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_VPL_TIMING_H_
