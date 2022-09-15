/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "smt_tracer.h"

namespace TranscodingSample {

SMTTracer::SMTTracer() : Log(), AddonLog(), E2ELatency(), EncLatency(), TracerMutex() {
    TimeBase = GetCurrentTS();
}

SMTTracer::~SMTTracer() {
    if (!Enabled)
        return;

    //these functions are intentionally called from destructor to try to save traces in case of a crash
    AdjustOverlappingEvents();
    AddFlowEvents();
    mfxU32 FileID = 0xfffffff & GetCurrentTS();
    SaveTrace(FileID);

    ComputeE2ELatency();
    SaveLatency(LatencyType::E2E, FileID, E2ELatency);

    ComputeEncLatency();
    SaveLatency(LatencyType::ENC, FileID, EncLatency);
}

void SMTTracer::Init(const PipelineType type,
                     const mfxU32 numOfChannels,
                     const LatencyType latency,
                     const mfxU32 TraceBufferSize) {
    if (Enabled) {
        return;
    }
    Enabled = true;

    if (TraceBufferSize > TraceBufferSizeInMBytes && TraceBufferSize < MaxTraceBufferSizeInMBytes) {
        TraceBufferSizeInMBytes = TraceBufferSize;
    }
    Log.reserve(TraceBufferSizeInMBytes * 1024 * 1024 / sizeof(Event));

    TypeOfPipeline = type;
    NumOfChannels  = numOfChannels;
    TypeOfLatency  = latency;
}

void SMTTracer::BeginEvent(const ThreadType thType,
                           const mfxU32 thID,
                           const EventName name,
                           const void* inID,
                           const void* outID) {
    if (!Enabled)
        return;
    AddEvent(EventType::DurationStart, thType, thID, name, inID, outID);
}

void SMTTracer::EndEvent(const ThreadType thType,
                         const mfxU32 thID,
                         const EventName name,
                         const void* inID,
                         const void* outID) {
    if (!Enabled)
        return;
    AddEvent(EventType::DurationEnd, thType, thID, name, inID, outID);
}

void SMTTracer::AddCounterEvent(const ThreadType thType,
                                const mfxU32 thID,
                                const EventName name,
                                const mfxU64 counter) {
    if (!Enabled)
        return;
    AddEvent(EventType::Counter, thType, thID, name, reinterpret_cast<void*>(counter), nullptr);
}

void SMTTracer::BeforeDecodeStart() {
    if (TypeOfLatency == LatencyType::E2E || TypeOfLatency == LatencyType::ENC) {
        std::unique_lock<std::mutex> guard(TracerMutex);
        if (NumOfActiveEncoders <= 0) {
            return;
        }

        DecSync.wait_for(guard, std::chrono::milliseconds(MaxFrameLatencyInMilliseconds), [&] {
            return NumOfActiveEncoders <= 0;
        });
    }
}

void SMTTracer::AfterDecodeStart() {
    if (TypeOfLatency == LatencyType::E2E || TypeOfLatency == LatencyType::ENC) {
        std::lock_guard<std::mutex> guard(TracerMutex);
        NumOfActiveEncoders = NumOfChannels;
        if (TypeOfLatency == LatencyType::ENC) {
            NumOfActiveDecoders = 1;
        }
    }
}

void SMTTracer::BeforeEncodeStart() {
    if (TypeOfLatency == LatencyType::ENC) {
        std::unique_lock<std::mutex> guard(TracerMutex);
        EncSync.wait_for(guard, std::chrono::milliseconds(MaxFrameLatencyInMilliseconds), [&] {
            return NumOfActiveDecoders <= 0;
        });
    }
}

void SMTTracer::AfterDecodeSync() {
    if (TypeOfLatency == LatencyType::ENC) {
        std::unique_lock<std::mutex> guard(TracerMutex);
        NumOfActiveDecoders = 0;
        guard.unlock();
        EncSync.notify_all();
    }
}

void SMTTracer::AfterEncodeSync() {
    if (TypeOfLatency == LatencyType::E2E || TypeOfLatency == LatencyType::ENC) {
        std::unique_lock<std::mutex> guard(TracerMutex);
        NumOfActiveEncoders--;

        if (NumOfActiveEncoders <= 0) {
            guard.unlock();
            DecSync.notify_one();
        }
    }
}

void SMTTracer::AdjustOverlappingEvents() {
    //If two duration events in the same thread overlap, then they or related flow event
    //may be displayed incorrectly. We move event a bit to avoid overlapping.
    for (EventIt evABegin = Log.begin(); evABegin != Log.end(); evABegin++) {
        //find beginning of event A
        if (evABegin->EvType != EventType::DurationStart) {
            continue;
        }

        auto evAEnd   = FindEventInThread(evABegin, EventType::DurationEnd);
        auto evBBegin = FindEventInThread(evAEnd, EventType::DurationStart);
        auto evBEnd   = FindEventInThread(evBBegin, EventType::DurationEnd);
        if (evAEnd == Log.end() || evBBegin == Log.end() || evBEnd == Log.end()) {
            continue;
        }

        //now we have two events A and B in the same thread, check if they overlap
        if (evAEnd->TS == evBBegin->TS) {
            //events overlap, try to shorten event A
            if (evABegin->TS != evAEnd->TS) {
                evAEnd->TS--;
            }
            else {
                //event A is too short already, try to shorten event B
                if (evBBegin->TS != evBEnd->TS) {
                    evBBegin->TS++;
                }
                else {
                    //both events are too short already, leave them as is
                }
            }
        }
    }
}

void SMTTracer::SaveTrace(mfxU32 FileID) {
    std::string FileName = "smt_trace_" + std::to_string(FileID) + ".json";
    std::ofstream trace_file(FileName, std::ios::out);
    if (!trace_file) {
        return;
    }

    printf("\n### trace buffer usage %.2f%%\n", 100. * Log.size() / Log.capacity());
    printf("trace file name %s\n", FileName.c_str());

    trace_file << "[" << std::endl;

    for (const Event ev : Log) {
        WriteEvent(trace_file, ev);
    }
    for (const Event ev : AddonLog) {
        WriteEvent(trace_file, ev);
    }

    trace_file.close();
}

SMTTracer::TimeInterval::TimeInterval(mfxU64 ts, mfxU64 duration) {
    TS       = ts;
    Duration = duration;
}

void SMTTracer::AddEvent(const EventType evType,
                         const ThreadType thType,
                         const mfxU32 thID,
                         const EventName name,
                         const void* inID,
                         const void* outID) {
    Event ev;
    ev.EvType = evType;
    ev.ThType = thType;
    ev.ThID   = thID;
    ev.Name   = name;
    ev.InID   = reinterpret_cast<mfxU64>(inID);
    ev.OutID  = reinterpret_cast<mfxU64>(outID);
    ev.TS     = GetCurrentTS();

    if (Log.size() == Log.capacity()) {
        return;
    }

    std::lock_guard<std::mutex> guard(TracerMutex);
    Log.push_back(ev);
}

mfxU64 SMTTracer::GetCurrentTS() {
    //Note, that standard specifies that "steady_clock" is monotonic but not system wide,
    //"system_clock" is system wide but not monotonic. We use "steady_clock", so please
    //check your implementation before comparing traces from different processes.
    using namespace std::chrono;
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}

void SMTTracer::AddFlowEvents() {
    for (auto it = Log.begin(); it != Log.end(); ++it) {
        if (it->EvType != EventType::DurationStart) {
            continue;
        }

        if (it->InID == 0) {
            continue;
        }

        auto itc = std::reverse_iterator<decltype(it)>(it);

        auto itp = find_if(itc, Log.rend(), [&it, this](Event ev) {
            return ev.EvType == EventType::DurationEnd && it->InID == ev.OutID &&
                   (TypeOfPipeline == PipelineType::_1xN || it->ThID == ev.ThID);
        });
        if (itp == Log.rend()) {
            continue;
        }

        AddFlowEvent(*itp, *it);
    }
}

void SMTTracer::ComputeE2ELatency() {
    NumOfErrors = 0;

    for (auto it = Log.begin(); it != Log.end(); ++it) {
        //look for end of sync operation in enc channel
        if (it->EvType != EventType::DurationEnd || it->ThType != ThreadType::ENC ||
            it->Name != EventName::SYNC) {
            continue;
        }

        EventIt bit = FindBeginningOfDependencyChain(it);
        if (bit == Log.end() || bit->ThType != ThreadType::DEC) {
            NumOfErrors++;
            continue;
        }

        E2ELatency[it->ThID].push_back(TimeInterval(bit->TS, it->TS - bit->TS));
    }
}

void SMTTracer::ComputeEncLatency() {
    NumOfErrors = 0;

    for (auto it = Log.begin(); it != Log.end(); ++it) {
        if (it->EvType != EventType::DurationEnd || it->ThType != ThreadType::ENC ||
            it->Name != EventName::SYNC) {
            continue;
        }

        EventIt syncEnd   = it;
        EventIt syncBegin = FindBeginningOfDurationEvent(syncEnd);
        EventIt encEnd    = FindEndOfPreviosDurationEvent(syncBegin);
        EventIt encBegin  = FindBeginningOfDurationEvent(encEnd);

        if (encBegin == Log.end() || encBegin->ThType != ThreadType::ENC) {
            NumOfErrors++;
            continue;
        }

        EncLatency[encBegin->ThID].push_back(
            TimeInterval(encBegin->TS, syncEnd->TS - encBegin->TS));
    }
}

void SMTTracer::SaveLatency(LatencyType type,
                            mfxU32 FileID,
                            std::map<mfxU32, std::vector<TimeInterval>>& latency) {
    if (latency.empty()) {
        return;
    }

    std::string FileName = std::string(type == LatencyType::E2E ? "e2e" : "enc") + "_latency_" +
                           std::to_string(FileID) + ".csv";
    std::ofstream trace_file(FileName, std::ios::out);
    if (!trace_file) {
        return;
    }

    printf("latency stat file name %s\n", FileName.c_str());

    trace_file << "File format, four columns per channel:" << std::endl;
    trace_file << "    frame number" << std::endl;
    trace_file << "    start of frame processing, wall clock, e.g. from system reboot, ms"
               << std::endl;
    trace_file << "    start of frame processing, from SMT start, ms" << std::endl;
    trace_file << "    frame processing duration, ms" << std::endl;

    //header
    for (const auto& v : latency) {
        for (int i = 0; i < 4; i++) {
            trace_file << "enc" << v.first << ",";
        }
    }
    trace_file << std::endl;

    for (size_t i = 0; i < latency.size(); i++) {
        trace_file << "fr#, ts, ts, latency,";
    }
    trace_file << std::endl;

    //data, note, that channels may have different number of frames, for example due to DI
    size_t NumOfFrames = 0;
    for (const auto& v : latency) {
        NumOfFrames = std::max(NumOfFrames, v.second.size());
    }

    for (size_t i = 0; i < NumOfFrames; i++) {
        for (const auto& v : latency) {
            if (v.second.size() > i) {
                trace_file << i << ",";
                TimeInterval ti = v.second[i];
                trace_file << ti.TS / 1000 << ",";
                trace_file << (ti.TS - TimeBase) / 1000 << ",";
                trace_file << ti.Duration / 1000. << ",";
            }
            else {
                trace_file << ",,,,";
            }
        }
        trace_file << std::endl;
    }

    trace_file.close();
}

SMTTracer::EventIt SMTTracer::FindBeginningOfDependencyChain(EventIt it) {
    //"it" should point to the end of duration event
    if (it == Log.begin() || it == Log.end() || it->EvType != EventType::DurationEnd) {
        return Log.end();
    }

    for (;;) {
        it = FindBeginningOfDurationEvent(it);
        if (it == Log.end() || it->InID == 0)
            break;

        it = FindEndOfPreviosDurationEvent(it);
        if (it == Log.end())
            break;
    }

    return it;
}

SMTTracer::EventIt SMTTracer::FindBeginningOfDurationEvent(EventIt it) {
    //"it" should point to the end of duration event
    if (it == Log.begin() || it == Log.end() || it->EvType != EventType::DurationEnd) {
        return Log.end();
    }

    EventIt itc = it;
    for (; itc != Log.begin();) {
        --itc;
        if (itc->EvType == EventType::DurationStart && itc->ThType == it->ThType &&
            itc->ThID == it->ThID && itc->Name == it->Name) {
            return itc;
        }
    }

    return Log.end();
}

SMTTracer::EventIt SMTTracer::FindEndOfPreviosDurationEvent(EventIt it) {
    //"it" should point to the beginnig of duration event
    if (it == Log.begin() || it == Log.end() || it->EvType != EventType::DurationStart) {
        return Log.end();
    }

    EventIt itc = it;
    for (; itc != Log.begin();) {
        --itc;
        if (itc->EvType == EventType::DurationEnd && itc->OutID == it->InID &&
            (TypeOfPipeline == PipelineType::_1xN || itc->ThID == it->ThID)) {
            return itc;
        }
    }
    return Log.end();
}

SMTTracer::EventIt SMTTracer::FindEventInThread(EventIt first, EventType type) {
    return std::find_if(first, Log.end(), [&first, &type](Event ev) {
        return ev.EvType == type && first->ThType == ev.ThType && first->ThID == ev.ThID;
    });
}

void SMTTracer::AddFlowEvent(const Event a, const Event b) {
    if (a.EvType != EventType::DurationEnd || b.EvType != EventType::DurationStart) {
        return;
    }

    Event ev;
    ev.EvType = EventType::FlowStart;
    ev.ThType = a.ThType;
    ev.ThID   = a.ThID;
    ev.EvID   = ++EvID;
    ev.TS     = a.TS;
    AddonLog.push_back(ev);

    ev.EvType = EventType::FlowEnd;
    ev.ThType = b.ThType;
    ev.ThID   = b.ThID;
    ev.EvID   = EvID;
    ev.TS     = b.TS;

    if (a.TS == b.TS) {
        ev.TS++;
    }

    AddonLog.push_back(ev);
}

void SMTTracer::WriteEvent(std::ofstream& trace_file, const Event ev) {
    switch (ev.EvType) {
        case EventType::DurationStart:
        case EventType::DurationEnd:
            WriteDurationEvent(trace_file, ev);
            break;
        case EventType::FlowStart:
        case EventType::FlowEnd:
            WriteFlowEvent(trace_file, ev);
            break;
        case EventType::Counter:
            WriteCounterEvent(trace_file, ev);
            break;
        default:;
    }
}

void SMTTracer::WriteDurationEvent(std::ofstream& trace_file, const Event ev) {
    trace_file << "{";
    WriteEventPID(trace_file);
    WriteComma(trace_file);
    WriteEventTID(trace_file, ev);
    WriteComma(trace_file);
    WriteEventTS(trace_file, ev);
    WriteComma(trace_file);
    WriteEventPhase(trace_file, ev);
    WriteComma(trace_file);
    WriteEventName(trace_file, ev);
    WriteComma(trace_file);
    WriteEventInOutIDs(trace_file, ev);
    trace_file << "}," << std::endl;
}

void SMTTracer::WriteFlowEvent(std::ofstream& trace_file, const Event ev) {
    trace_file << "{";
    WriteEventPID(trace_file);
    WriteComma(trace_file);
    WriteEventTID(trace_file, ev);
    WriteComma(trace_file);
    WriteEventTS(trace_file, ev);
    WriteComma(trace_file);
    WriteEventPhase(trace_file, ev);
    WriteComma(trace_file);
    WriteEventName(trace_file, ev);
    WriteComma(trace_file);
    WriteBindingPoint(trace_file, ev);
    WriteComma(trace_file);
    WriteEventCategory(trace_file);
    WriteComma(trace_file);
    WriteEvID(trace_file, ev);
    trace_file << "}," << std::endl;
}

void SMTTracer::WriteCounterEvent(std::ofstream& trace_file, const Event ev) {
    trace_file << "{";
    WriteEventPID(trace_file);
    WriteComma(trace_file);
    WriteEventTID(trace_file, ev);
    WriteComma(trace_file);
    WriteEventTS(trace_file, ev);
    WriteComma(trace_file);
    WriteEventPhase(trace_file, ev);
    WriteComma(trace_file);
    WriteEventName(trace_file, ev);
    WriteComma(trace_file);
    WriteEventCounter(trace_file, ev);
    trace_file << "}," << std::endl;
}

void SMTTracer::WriteEventPID(std::ofstream& trace_file) {
    trace_file << "\"pid\":\"smt\"";
}

void SMTTracer::WriteEventTID(std::ofstream& trace_file, const Event ev) {
    trace_file << "\"tid\":\"";
    switch (ev.ThType) {
        case ThreadType::DEC:
            trace_file << "dec" << ev.ThID;
            break;
        case ThreadType::VPP:
            trace_file << "enc" << ev.ThID; //it puts VPP events in enc thread
            break;
        case ThreadType::ENC:
            trace_file << "enc" << ev.ThID;
            break;
        case ThreadType::CSVPP:
            trace_file << "vpp" << ev.ThID;
            break;
        default:
            trace_file << "unknown";
            break;
    }
    trace_file << "\"";
}

void SMTTracer::WriteEventTS(std::ofstream& trace_file, const Event ev) {
    trace_file << "\"ts\":" << ev.TS;
}

void SMTTracer::WriteEventPhase(std::ofstream& trace_file, const Event ev) {
    trace_file << "\"ph\":\"";

    switch (ev.EvType) {
        case EventType::DurationStart:
            trace_file << "B";
            break;
        case EventType::DurationEnd:
            trace_file << "E";
            break;
        case EventType::FlowStart:
            trace_file << "s";
            break;
        case EventType::FlowEnd:
            trace_file << "f";
            break;
        case EventType::Counter:
            trace_file << "C";
            break;
        default:
            trace_file << "unknown";
            break;
    }
    trace_file << "\"";
}

void SMTTracer::WriteEventName(std::ofstream& trace_file, const Event ev) {
    trace_file << "\"name\":\"";
    if (ev.EvType == EventType::FlowStart || ev.EvType == EventType::FlowEnd) {
        trace_file << "link";
    }
    else if (ev.EvType == EventType::Counter) {
        switch (ev.ThType) {
            case ThreadType::DEC:
                trace_file << "dec_pool" << ev.ThID;
                break;
            case ThreadType::VPP:
                trace_file << "enc_pool" << ev.ThID;
                break;
            case ThreadType::ENC:
                trace_file << "enc_pool" << ev.ThID;
                break;
            case ThreadType::CSVPP:
                trace_file << "vpp_pool" << ev.ThID;
                break;
            default:
                trace_file << "unknown";
                break;
        }
    }
    else if (ev.Name != EventName::UNDEF) {
        switch (ev.Name) {
            case EventName::BUSY:
                trace_file << "busy";
                break;
            case EventName::SYNC:
                trace_file << "syncp";
                break;
            case EventName::READ_YUV:
            case EventName::READ_BS:
                trace_file << "read";
                break;
            case EventName::WRITE_BS:
                trace_file << "write";
                break;
            case EventName::SURF_WAIT:
                trace_file << "wait";
                break;
            default:
                trace_file << "unknown";
                break;
        }
    }
    else {
        switch (ev.ThType) {
            case ThreadType::DEC:
                trace_file << "dec";
                break;
            case ThreadType::VPP:
                trace_file << "vpp";
                break;
            case ThreadType::ENC:
                trace_file << "enc";
                break;
            case ThreadType::CSVPP:
                trace_file << "csvpp";
                break;
            default:
                trace_file << "unknown";
                break;
        }
    }
    trace_file << "\"";
}

void SMTTracer::WriteBindingPoint(std::ofstream& trace_file, const Event ev) {
    if (ev.EvType != EventType::FlowStart && ev.EvType != EventType::FlowEnd) {
        return;
    }
    trace_file << "\"bp\":\"e\"";
}

void SMTTracer::WriteEventInOutIDs(std::ofstream& trace_file, const Event ev) {
    trace_file << "\"args\":{\"InID\":" << ev.InID << ",\"OutID\":" << ev.OutID << "}";
}

void SMTTracer::WriteEventCounter(std::ofstream& trace_file, const Event ev) {
    trace_file << "\"args\":{\"free surfaces\":" << ev.InID << "}";
}

void SMTTracer::WriteEventCategory(std::ofstream& trace_file) {
    trace_file << "\"cat\":\"link\"";
}

void SMTTracer::WriteEvID(std::ofstream& trace_file, const Event ev) {
    trace_file << "\"id\":\"id_" << ev.EvID << "\"";
}

void SMTTracer::WriteComma(std::ofstream& trace_file) {
    trace_file << ",";
}

} // namespace TranscodingSample
