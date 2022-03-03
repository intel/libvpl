/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __SMT_TRACER_H__
#define __SMT_TRACER_H__

#include <algorithm>
#include <chrono>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "vpl/mfxdefs.h"

namespace TranscodingSample {
class SMTTracer {
public:
    enum class ThreadType { DEC, CSVPP, VPP, ENC };

    enum class EventName { UNDEF, BUSY, SYNC, READ_YUV, READ_BS, SURF_WAIT };

    enum class EventType { DurationStart, DurationEnd, FlowStart, FlowEnd, Counter };

    class Event {
    public:
        EventType EvType; //duration, flow, counter
        ThreadType ThType; //dec, vpp, enc, csvpp
        mfxU32 ThID; //channel or pool number in 1toN pipeline
        EventName Name; //optional, if not specifyed thread name will be used
        mfxU32 EvID; //unique event ID
        mfxU64 InID; //unique dependency ID, e.g. surface pointer
        mfxU64 OutID;
        mfxU64 TS; //time stamp
    };
    using EventIt = std::vector<Event>::iterator;

    SMTTracer();
    ~SMTTracer();

    void Init();
    void BeginEvent(const ThreadType thType,
                    const mfxU32 thID,
                    const EventName name,
                    const void* inID,
                    const void* outID);
    void EndEvent(const ThreadType thType,
                  const mfxU32 thID,
                  const EventName name,
                  const void* inID,
                  const void* outID);
    void AddCounterEvent(const ThreadType thType,
                         const mfxU32 thID,
                         const EventName name,
                         const mfxU64 counter);

private:
    //runtime functions
    void AddEvent(const EventType evType,
                  const ThreadType thType,
                  const mfxU32 thID,
                  const EventName name,
                  const void* inID,
                  const void* outID);
    mfxU64 GetCurrentTS();

    //log generation functions
    void SaveTrace(mfxU32 FileID);

    void AddFlowEvents();
    void AddFlowEvent(const Event a, const Event b);

    void ComputeE2ELatency();
    void PrintE2ELatency();

    void ComputeEncLatency();
    void PrintEncLatency();

    EventIt FindBeginningOfDependencyChain(EventIt it);
    EventIt FindBeginningOfDurationEvent(EventIt it);
    EventIt FindEndOfPreviosDurationEvent(EventIt it);

    void WriteEvent(std::ofstream& trace_file, const Event ev);
    void WriteDurationEvent(std::ofstream& trace_file, const Event ev);
    void WriteFlowEvent(std::ofstream& trace_file, const Event ev);
    void WriteCounterEvent(std::ofstream& trace_file, const Event ev);

    void WriteEventPID(std::ofstream& trace_file);
    void WriteEventTID(std::ofstream& trace_file, const Event ev);
    void WriteEventTS(std::ofstream& trace_file, const Event ev);
    void WriteEventPhase(std::ofstream& trace_file, const Event ev);
    void WriteEventName(std::ofstream& trace_file, const Event ev);
    void WriteBindingPoint(std::ofstream& trace_file, const Event ev);
    void WriteEventInOutIDs(std::ofstream& trace_file, const Event ev);
    void WriteEventCounter(std::ofstream& trace_file, const Event ev);
    void WriteEventCategory(std::ofstream& trace_file);
    void WriteEvID(std::ofstream& trace_file, const Event ev);
    void WriteComma(std::ofstream& trace_file);

    const static mfxU32 TraceBufferSizeInMBytes = 7;

    bool Enabled = false;
    mfxU32 EvID  = 0;
    std::vector<Event> Log;
    std::vector<Event> AddonLog;
    std::map<mfxU32, std::vector<mfxU64>> E2ELatency;
    std::map<mfxU32, std::vector<mfxU64>> EncLatency;
    mfxU32 NumOfErrors = 0;
    std::chrono::steady_clock::time_point TimeBase;
    std::mutex TracerFileMutex;
};

} // namespace TranscodingSample
#endif
