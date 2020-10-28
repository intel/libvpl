/*###########################################################################
  # Copyright (C) 2005-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#pragma once

#include <stdio.h>
#include <vector>
#include "math.h"
#include "vm/strings_defs.h"
#include "vm/time_defs.h"
#include "vpl/mfxstructures.h"

#pragma warning(disable : 4100)

class CTimeStatisticsReal {
public:
    CTimeStatisticsReal() {
        ResetStatistics();
        start          = 0;
        m_bNeedDumping = false;
    }

    static msdk_tick GetFrequency() {
        if (!frequency) {
            frequency = msdk_time_get_frequency();
        }
        return frequency;
    }

    static mfxF64 ConvertToSeconds(msdk_tick elapsed) {
        return MSDK_GET_TIME(elapsed, 0, GetFrequency());
    }

    inline void StartTimeMeasurement() {
        start = msdk_time_get_tick();
    }

    inline void StopTimeMeasurement() {
        mfxF64 delta = GetDeltaTime();
        totalTime += delta;
        totalTimeSquares += delta * delta;
        // dump in ms:
        if (m_bNeedDumping)
            m_time_deltas.push_back(delta * 1000);

        if (delta < minTime) {
            minTime = delta;
        }

        if (delta > maxTime) {
            maxTime = delta;
        }
        numMeasurements++;
    }

    inline void StopTimeMeasurementWithCheck() {
        if (start) {
            StopTimeMeasurement();
        }
    }

    inline mfxF64 GetDeltaTime() {
        return MSDK_GET_TIME(msdk_time_get_tick(), start, GetFrequency());
    }

    inline mfxF64 GetDeltaTimeInMiliSeconds() {
        return GetDeltaTime() * 1000;
    }

    inline void TurnOnDumping() {
        m_bNeedDumping = true;
    }

    inline void TurnOffDumping() {
        m_bNeedDumping = false;
    }

    inline void PrintStatistics(const msdk_char* prefix) {
        msdk_printf(
            MSDK_STRING(
                "%s Total:%.3fms(%lld smpls),Avg %.3fms,StdDev:%.3fms,Min:%.3fms,Max:%.3fms\n"),
            prefix,
            static_cast<double>(totalTime),
            static_cast<long long int>(numMeasurements),
            static_cast<double>(GetAvgTime(false)),
            static_cast<double>(GetTimeStdDev(false)),
            static_cast<double>(GetMinTime(false)),
            static_cast<double>(GetMaxTime(false)));
    }

    inline mfxU64 GetNumMeasurements() {
        return numMeasurements;
    }

    inline mfxF64 GetAvgTime(bool inSeconds = true) {
        if (inSeconds) {
            return (numMeasurements ? totalTime / numMeasurements : 0);
        }
        else {
            return (numMeasurements ? totalTime / numMeasurements : 0) * 1000;
        }
    }

    inline mfxF64 GetTimeStdDev(bool inSeconds = true) {
        mfxF64 avg = GetAvgTime();
        mfxF64 ftmp =
            (numMeasurements ? sqrt(totalTimeSquares / numMeasurements - avg * avg) : 0.0);
        return inSeconds ? ftmp : ftmp * 1000;
    }

    inline mfxF64 GetMinTime(bool inSeconds = true) {
        return inSeconds ? minTime : minTime * 1000;
    }

    inline mfxF64 GetMaxTime(bool inSeconds = true) {
        return inSeconds ? maxTime : maxTime * 1000;
    }

    inline mfxF64 GetTotalTime(bool inSeconds = true) {
        return inSeconds ? totalTime : totalTime * 1000;
    }

    inline void ResetStatistics() {
        totalTime        = 0;
        totalTimeSquares = 0;
        minTime          = 1E100;
        maxTime          = -1;
        numMeasurements  = 0;
        m_time_deltas.clear();
        TurnOffDumping();
    }

protected:
    static msdk_tick frequency;

    msdk_tick start;
    mfxF64 totalTime;
    mfxF64 totalTimeSquares;
    mfxF64 minTime;
    mfxF64 maxTime;
    mfxU64 numMeasurements;
    std::vector<mfxF64> m_time_deltas;
    bool m_bNeedDumping;
};

class CTimeStatisticsDummy {
public:
    static msdk_tick GetFrequency() {
        if (!frequency) {
            frequency = msdk_time_get_frequency();
        }
        return frequency;
    }

    static mfxF64 ConvertToSeconds(msdk_tick elapsed) {
        return 0;
    }

    inline void StartTimeMeasurement() {}

    inline void StopTimeMeasurement() {}

    inline void StopTimeMeasurementWithCheck() {}

    inline mfxF64 GetDeltaTime() {
        return 0;
    }

    inline mfxF64 GetDeltaTimeInMiliSeconds() {
        return 0;
    }

    inline void TurnOnDumping() {}

    inline void TurnOffDumping() {}

    inline void PrintStatistics(const msdk_char* prefix) {}

    inline mfxU64 GetNumMeasurements() {
        return 0;
    }

    inline mfxF64 GetAvgTime(bool) {
        return 0;
    }

    inline mfxF64 GetTimeStdDev(bool) {
        return 0;
    }

    inline mfxF64 GetMinTime(bool) {
        return 0;
    }

    inline mfxF64 GetMaxTime(bool) {
        return 0;
    }

    inline mfxF64 GetTotalTime(bool) {
        return 0;
    }

    inline void ResetStatistics() {}

protected:
    static msdk_tick frequency;
};

#ifdef TIME_STATS
typedef CTimeStatisticsReal CTimeStatistics;
#else
typedef CTimeStatisticsDummy CTimeStatistics;
#endif
