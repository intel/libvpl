/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __SMT_TRACER_H__
#define __SMT_TRACER_H__

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "vpl/mfxdefs.h"

namespace TranscodingSample {
class SMTTracer {
public:
    enum class PipelineType { unknown, _1x1, _1xN, _NxN };

    enum class ThreadType { DEC, CSVPP, VPP, ENC };

    enum class EventName { UNDEF, BUSY, SYNC, READ_YUV, READ_BS, WRITE_BS, SURF_WAIT };

    enum class EventType { DurationStart, DurationEnd, FlowStart, FlowEnd, Counter };

    enum class LatencyType { DEFAULT, E2E, ENC };

    SMTTracer();
    ~SMTTracer();

    void Init(const PipelineType type,
              const mfxU32 numOfChannels,
              const LatencyType latency,
              const mfxU32 TraceBufferSize);
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
    void BeforeDecodeStart();
    void AfterDecodeStart();
    void BeforeEncodeStart();
    void AfterDecodeSync();
    void AfterEncodeSync();

private:
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

    class TimeInterval {
    public:
        TimeInterval(mfxU64 ts, mfxU64 duration);
        mfxU64 TS; //beginning of time interval, wall clock
        mfxU64 Duration;
    };

    //runtime functions
    void AddEvent(const EventType evType,
                  const ThreadType thType,
                  const mfxU32 thID,
                  const EventName name,
                  const void* inID,
                  const void* outID);
    mfxU64 GetCurrentTS();

    //log generation functions
    void AdjustOverlappingEvents();
    void SaveTrace(mfxU32 FileID);

    void AddFlowEvents();
    void AddFlowEvent(const Event a, const Event b);

    void ComputeE2ELatency();
    void ComputeEncLatency();
    void SaveLatency(LatencyType type,
                     mfxU32 FileID,
                     std::map<mfxU32, std::vector<TimeInterval>>& latency);

    EventIt FindBeginningOfDependencyChain(EventIt it);
    EventIt FindBeginningOfDurationEvent(EventIt it);
    EventIt FindEndOfPreviosDurationEvent(EventIt it);
    EventIt FindEventInThread(EventIt first, EventType type);

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

    mfxU32 TraceBufferSizeInMBytes          = 7;
    const mfxU32 MaxTraceBufferSizeInMBytes = 128;

    bool Enabled                = false;
    PipelineType TypeOfPipeline = PipelineType::unknown;
    mfxU32 EvID                 = 0;
    std::vector<Event> Log;
    std::vector<Event> AddonLog;
    std::map<mfxU32, std::vector<TimeInterval>> E2ELatency;
    std::map<mfxU32, std::vector<TimeInterval>> EncLatency;
    mfxU32 NumOfErrors = 0;
    mfxU64 TimeBase    = 0; //moment of time when tracer has been created

    std::mutex TracerMutex;
    LatencyType TypeOfLatency = LatencyType::DEFAULT;
    int NumOfChannels         = 0; //this is "N" in 1toN
    int NumOfActiveDecoders   = 0;
    int NumOfActiveEncoders   = 0; //number of encoders that are still running
    std::condition_variable DecSync{}; //wake up decoder
    std::condition_variable EncSync{}; //wake up encoders
    const mfxU32 MaxFrameLatencyInMilliseconds = 200;
};

} // namespace TranscodingSample
#endif
