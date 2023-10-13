/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __SAMPLE_PIPELINE_TRANSCODE_H__
#define __SAMPLE_PIPELINE_TRANSCODE_H__

#include <stddef.h>

#include <chrono>
#include <condition_variable>
#include <ctime>
#include <future>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "base_allocator.h"
#include "mfx_multi_vpp.h"
#include "rotate_plugin_api.h"
#include "sample_defs.h"
#include "sample_utils.h"
#include "sysmem_allocator.h"

#include "brc_routines.h"
#include "hw_device.h"
#include "mfxdeprecated.h"
#include "mfxplugin.h"
#include "plugin_utils.h"
#include "preset_manager.h"
#include "sample_defs.h"
#include "smt_tracer.h"
#include "vpl/mfxdispatcher.h"
#include "vpl/mfxjpeg.h"
#include "vpl/mfxmvc.h"
#include "vpl/mfxvideo++.h"
#include "vpl/mfxvideo.h"
#include "vpl/mfxvp8.h"

#define TIME_STATS 1 // Enable statistics processing
#include "time_statistics.h"

#if defined(_WIN32) || defined(_WIN64)
    #include "decode_render.h"
#endif

#include "smt_cli_params.h"

#define MAX_PREF_LEN 256

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

#ifdef ENABLE_MCTF
const mfxU16 MCTF_MID_FILTER_STRENGTH = 10;
const mfxF64 MCTF_LOSSLESS_BPP        = 0.0;
#endif

namespace TranscodingSample {

enum ProlongStatus { NormalFrame = 0x5F, BlackFrame = 0xBF, AllBlack = 0xAB };

enum VppCompDumpMode { NULL_RENDER_VPP_COMP = 1, DUMP_FILE_VPP_COMP = 2 };

enum MemoryModel {
    UNKNOWN_ALLOC     = 0, // GENERAL_ALLOC will be used by default
    GENERAL_ALLOC     = 1,
    VISIBLE_INT_ALLOC = 2,
    HIDDEN_INT_ALLOC  = 3
};

static const mfxU32 DecoderTargetID = 100;
static const mfxU32 DecoderPoolID   = 10;
class CascadeScalerConfig {
public:
    class TargetDescriptor {
    public:
        mfxU32 TargetID     = 0;
        mfxU16 SrcWidth     = 0;
        mfxU16 SrcHeight    = 0;
        mfxU16 DstWidth     = 0;
        mfxU16 DstHeight    = 0;
        double SrcFrameRate = 0.;
        double DstFrameRate = 0.;
        mfxU16 SrcPicStruct = MFX_PICSTRUCT_UNKNOWN;
        mfxU16 DstPicStruct = MFX_PICSTRUCT_UNKNOWN;

        bool CascadeScaler = false; //use cascade scaler for this target
        bool FRC           = false; //this cascade performs FRC
        bool DI            = false; //this cascade performs DI

        mfxU32 PoolID =
            0; //surface pool for this target, it is output of decoder side VPP and input of encoder side VPP
    };

    class PoolDescritpor {
    public:
        mfxU32 ID            = 0; //ID of the current pool
        mfxU32 PrevID        = 0; //ID of the previous pool in cascade
        mfxU32 TargetID      = 0; //ID of the target channel
        mfxU16 SurfaceWidth  = 0; //not aligned
        mfxU16 SurfaceHeight = 0;
        mfxU16 Size          = 20;

        mfxFrameAllocRequest AllocReq{};
        mfxFrameAllocResponse AllocResp{};
    };

    TargetDescriptor GetDesc(mfxU32 id);
    void PropagateCascadeParameters();
    void CreatePoolList();
    bool SkipFrame(mfxU32 targetID, mfxU32 frameNum);

    bool ParFileImported          = false;
    bool CascadeScalerRequired    = false;
    bool ParallelEncodingRequired = false;
    mfxU32 GopSize                = 0;

    std::vector<TargetDescriptor> Targets;
    std::map<mfxU32, PoolDescritpor> Pools; //key is pool ID
    std::map<mfxU32, sInputParams>
        InParams; //key is target ID, copy of par file for cascade VPP initialization

    SMTTracer* Tracer            = nullptr;
    SMTTracer::PipelineType type = SMTTracer::PipelineType::unknown;

    CascadeScalerConfig()
            : ParFileImported(false),
              CascadeScalerRequired(false),
              Targets(),
              Pools(),
              InParams(),
              Tracer(nullptr) {}
};

struct PreEncAuxBuffer {
    mfxEncodeCtrl encCtrl;
    mfxU16 Locked;
    mfxENCInput encInput;
    mfxENCOutput encOutput;
};

struct ExtendedSurface {
    mfxU32 TargetID;
    mfxU8 FrameAttrib;

    mfxFrameSurface1* pSurface;
    PreEncAuxBuffer* pAuxCtrl;
    mfxEncodeCtrl* pEncCtrl;
    mfxSyncPoint Syncp;
};

struct ExtendedBS {
    bool IsFree = true;
    mfxBitstreamWrapper Bitstream;
    mfxSyncPoint Syncp     = nullptr;
    PreEncAuxBuffer* pCtrl = nullptr;
};

class CIOStat : public CTimeStatistics {
public:
    CIOStat() : CTimeStatistics(), DumpLogFileName(), ofile(stdout), bufDir() {}

    CIOStat(const std::string& dir) : CTimeStatistics(), DumpLogFileName(), ofile(stdout) {
        bufDir = dir;
    }

    ~CIOStat() {}

    inline void SetOutputFile(FILE* file) {
        ofile = file;
    }

    inline void SetDumpName(const std::string& name) {
        DumpLogFileName = name;
        if (!DumpLogFileName.empty()) {
            TurnOnDumping();
        }
        else {
            TurnOffDumping();
        }
    }

    inline void SetDirection(const std::string& dir) {
        bufDir = dir;
    }

    inline void PrintStatistics(mfxU32 numPipelineid,
                                mfxF64 target_framerate = -1 /*default stands for infinite*/) {
        // print timings in ms
        fprintf(
            ofile,
            "stat[%u.%llu]: %s=%d;Framerate=%.3f;Total=%.3lf;Samples=%lld;StdDev=%.3lf;Min=%.3lf;Max=%.3lf;Avg=%.3lf\n",
            (unsigned int)msdk_get_current_pid(),
            (unsigned long long int)rdtsc(),
            bufDir.c_str(),
            (int)numPipelineid,
            (double)target_framerate,
            (double)GetTotalTime(false),
            (long long int)GetNumMeasurements(),
            (double)GetTimeStdDev(false),
            (double)GetMinTime(false),
            (double)GetMaxTime(false),
            (double)GetAvgTime(false));
        fflush(ofile);

        if (!DumpLogFileName.empty()) {
            std::stringstream dump_deltas_log_file_name_sstr;
            dump_deltas_log_file_name_sstr << DumpLogFileName << "_ID_" << numPipelineid << ".log";
            DumpDeltas(dump_deltas_log_file_name_sstr.str());
        }
    }

    inline void DumpDeltas(const std::string& file_name) {
        if (m_time_deltas.empty())
            return;

        try {
            std::ofstream dump_file(file_name, std::ofstream::app);
            dump_file << std::fixed << std::setprecision(3);
            for (std::vector<mfxF64>::const_iterator it = m_time_deltas.begin();
                 it != m_time_deltas.end();
                 ++it) {
                dump_file << (*it) << ", ";
            }
        }
        catch (std::ios::failure&) {
            perror("DumpDeltas: file cannot be open");
        }
    }

protected:
    std::string DumpLogFileName;
    FILE* ofile;
    std::string bufDir;
};

class ExtendedBSStore {
public:
    explicit ExtendedBSStore(mfxU32 size) {
        m_pExtBS.resize(size);
    }
    virtual ~ExtendedBSStore() {
        m_pExtBS.clear();
    }
    ExtendedBS* GetNext() {
        for (mfxU32 i = 0; i < m_pExtBS.size(); i++) {
            if (m_pExtBS[i].IsFree) {
                m_pExtBS[i].IsFree = false;
                return &m_pExtBS[i];
            }
        }
        return NULL;
    }
    void Release(ExtendedBS* pBS) {
        for (mfxU32 i = 0; i < m_pExtBS.size(); i++) {
            if (&m_pExtBS[i] == pBS) {
                m_pExtBS[i].IsFree = true;
                return;
            }
        }
        return;
    }
    void ReleaseAll() {
        for (mfxU32 i = 0; i < m_pExtBS.size(); i++) {
            m_pExtBS[i].IsFree = true;
        }
        return;
    }
    void FlushAll() {
        for (mfxU32 i = 0; i < m_pExtBS.size(); i++) {
            m_pExtBS[i].Bitstream.DataLength = 0;
            m_pExtBS[i].Bitstream.DataOffset = 0;
        }
        return;
    }

protected:
    std::vector<ExtendedBS> m_pExtBS;

private:
    DISALLOW_COPY_AND_ASSIGN(ExtendedBSStore);
};

class CTranscodingPipeline;
// thread safety buffer heterogeneous pipeline
// only for join sessions
class SafetySurfaceBuffer {
public:
    //this is used only for sanity check
    mfxU32 TargetID       = 0;
    ProlongStatus Prolong = NormalFrame;

    struct SurfaceDescriptor {
        SurfaceDescriptor() : ExtSurface(), Locked(false) {}
        ExtendedSurface ExtSurface;
        mfxU32 Locked;
    };

    SafetySurfaceBuffer(SafetySurfaceBuffer* pNext);
    virtual ~SafetySurfaceBuffer();

    mfxU32 GetLength();
    mfxStatus WaitForSurfaceRelease(mfxU32 msec);
    mfxStatus WaitForSurfaceInsertion(mfxU32 msec);
    void AddSurface(ExtendedSurface Surf);
    mfxStatus GetSurface(ExtendedSurface& Surf);
    mfxStatus ReleaseSurface(mfxFrameSurface1* pSurf);
    mfxStatus ReleaseSurfaceAll();
    void CancelBuffering();

    SafetySurfaceBuffer* m_pNext;

protected:
    std::mutex m_mutex;
    std::list<SurfaceDescriptor> m_SList;
    bool m_IsBufferingAllowed;
    MSDKEvent* pRelEvent;
    MSDKEvent* pInsEvent;

private:
    DISALLOW_COPY_AND_ASSIGN(SafetySurfaceBuffer);
};

class FileBitstreamProcessor {
public:
    FileBitstreamProcessor();
    virtual ~FileBitstreamProcessor();
    virtual mfxStatus SetReader(std::unique_ptr<CSmplBitstreamReader>& reader);
    virtual mfxStatus SetReader(std::unique_ptr<CSmplYUVReader>& reader);
    virtual mfxStatus SetWriter(std::shared_ptr<CSmplBitstreamWriter>& writer);
    virtual mfxStatus GetInputBitstream(mfxBitstreamWrapper** pBitstream);
    virtual mfxStatus GetInputFrame(mfxFrameSurface1* pSurface);
    virtual mfxStatus ProcessOutputBitstream(mfxBitstreamWrapper* pBitstream);
    virtual mfxStatus ProcessOutputBitstream(mfxBitstreamWrapper* pBitstream,
                                             mfxU32 targetID,
                                             mfxU32 frameNum);
    virtual mfxStatus ResetInput();
    virtual mfxStatus ResetOutput();
    virtual bool IsNulOutput();

protected:
    std::unique_ptr<CSmplBitstreamReader> m_pFileReader;
    std::unique_ptr<CSmplYUVReader> m_pYUVFileReader;
    // for performance options can be zero
    std::shared_ptr<CSmplBitstreamWriter> m_pFileWriter;
    mfxBitstreamWrapper m_Bitstream;

private:
    DISALLOW_COPY_AND_ASSIGN(FileBitstreamProcessor);
};

typedef std::vector<mfxFrameSurface1*> SurfPointersArray;
typedef std::vector<PreEncAuxBuffer> PreEncAuxArray;
typedef std::list<ExtendedBS*> BSList;

// Bitstream is external via BitstreamProcessor
class CTranscodingPipeline {
public:
    CTranscodingPipeline();
    virtual ~CTranscodingPipeline();

    virtual mfxStatus Init(sInputParams* pParams,
                           MFXFrameAllocator* pMFXAllocator,
                           void* hdl,
                           CTranscodingPipeline* pParentPipeline,
                           SafetySurfaceBuffer* pBuffer,
                           FileBitstreamProcessor* pBSProc,
                           VPLImplementationLoader* mfxLoader,
                           CascadeScalerConfig& CSConfig);

    // frames allocation is suspended for heterogeneous pipeline
    virtual mfxStatus CompleteInit();
    virtual void Close();
    virtual mfxStatus Reset(); // for 1.X init
    virtual mfxStatus Reset(VPLImplementationLoader* mfxLoader);
    virtual mfxStatus Join(MFXVideoSession* pChildSession);
    virtual mfxStatus Run();
    virtual mfxStatus FlushLastFrames() {
        return MFX_ERR_NONE;
    }

    mfxU32 GetProcessFrames() {
        return m_nProcessedFramesNum;
    }

    bool GetJoiningFlag() {
        return m_bIsJoinSession;
    }

    mfxStatus QueryMFXVersion(mfxVersion* version) {
        MSDK_CHECK_POINTER(m_pmfxSession.get(), MFX_ERR_NULL_PTR);
        return m_pmfxSession->QueryVersion(version);
    };
    inline mfxU32 GetPipelineID() {
        return m_nID;
    }
    inline void SetPipelineID(mfxU32 id) {
        m_nID = id;
    }
    void StopSession();
    mfxStatus CheckStopCondition();
    void SetSurfaceUtilizationSynchronizer(
        std::shared_ptr<SurfaceUtilizationSynchronizer>& surfaceUtilizationSynchronizer);
    mfxU16 GetNumFrameForAlloc() const;
    bool IsOverlayUsed();
    size_t GetRobustFlag();
    eAPIVersion GetVersionOfSessionInitAPI();

    std::string GetSessionText() {
        std::stringstream ss;
        ss << m_pmfxSession->operator mfxSession();

        return ss.str();
    }

    //Adapter type
    void SetAdapterType(mfxU16 adapterType) {
        m_adapterType = adapterType;
    };
    void SetPreferdGfx(mfxI32 dGfxIdx = 0) {
        m_dGfxIdx = dGfxIdx;
    };
    void SetAdapterNum(mfxU32 adapterNum = 0) {
        m_adapterNum = adapterNum;
    };
    void SetSurfaceWaitInterval(mfxU32 surface_wait_interval = MSDK_SURFACE_WAIT_INTERVAL) {
        m_surface_wait_interval =
            surface_wait_interval > 0 ? surface_wait_interval : MSDK_SURFACE_WAIT_INTERVAL;
    };
    void SetSyncOpTimeout(mfxU32 syncOpTimeout = MSDK_WAIT_INTERVAL) {
        m_nSyncOpTimeout = syncOpTimeout;
    };

    mfxU16 GetAdapterType() const {
        return m_adapterType;
    };
    mfxI32 GetdGfxIdx() const {
        return m_dGfxIdx;
    };
    mfxI32 GetAdapterNum() const {
        return m_adapterNum;
    };
    mfxU32 GetSurfaceWaitInterval() const {
        return m_surface_wait_interval;
    };
    mfxU32 GetSyncOpTimeout() const {
        return m_nSyncOpTimeout;
    };

#if (defined(_WIN32) || defined(_WIN64))
    //Adapter type
    void SetPreferiGfx(bool preferiGfx) {
        bPreferiGfx = preferiGfx;
    };
    void SetPreferdGfx(bool preferdGfx) {
        bPreferdGfx = preferdGfx;
    };
    bool IsPreferiGfx() {
        return bPreferiGfx;
    };
    bool IsPreferdGfx() {
        return bPreferdGfx;
    };
#endif

    void PrintLibInfo(VPLImplementationLoader* Loader);

protected:
    virtual mfxStatus CheckRequiredAPIVersion(mfxVersion& version, sInputParams* pParams);

    virtual mfxStatus Decode();
    virtual mfxStatus Encode();
    virtual mfxStatus Transcode();
    virtual mfxStatus DecodeOneFrame(ExtendedSurface* pExtSurface);
    virtual mfxStatus CreateBlackFrame(ExtendedSurface* pExtSurface);
    virtual mfxStatus DecodeLastFrame(ExtendedSurface* pExtSurface);
    virtual mfxStatus VPPOneFrame(ExtendedSurface* pSurfaceIn,
                                  ExtendedSurface* pExtSurface,
                                  mfxU32 ID = 0);
    virtual mfxStatus EncodeOneFrame(ExtendedSurface* pExtSurface, mfxBitstreamWrapper* pBS);
    virtual mfxStatus DecodePreInit(sInputParams* pParams);
    virtual mfxStatus VPPPreInit(sInputParams* pParams);
    virtual mfxStatus EncodePreInit(sInputParams* pParams);
    mfxVideoParam GetDecodeParam(mfxU32 ID = 0);

    mfxExtMVCSeqDesc GetDecMVCSeqDesc() {
        mfxExtMVCSeqDesc* mvc = m_mfxDecParams;
        return mvc ? *mvc : mfxExtMVCSeqDesc();
    }

    static void ModifyParamsUsingPresets(sInputParams& params,
                                         mfxF64 fps,
                                         mfxU32 width,
                                         mfxU32 height);

    // alloc frames for all component
    mfxStatus AllocFrames(mfxFrameAllocRequest* pRequest, bool isDecAlloc);
    mfxStatus AllocFramesForCS();
    mfxStatus SetupSurfacePool(mfxU32 preallocateNum);

    // need for heterogeneous pipeline
    mfxStatus CalculateNumberOfReqFrames(mfxFrameAllocRequest& pRequestDecOut,
                                         mfxFrameAllocRequest& pRequestVPPOut);
    void CorrectNumberOfAllocatedFrames(mfxFrameAllocRequest* pNewReq, mfxU32 ID = 0);
    void FreeFrames();

    mfxStatus LoadStaticSurface();

    mfxFrameSurface1* GetFreeSurface(bool isDec, mfxU64 timeout);
    mfxFrameSurface1* GetFreeSurfaceForCS(bool isDec, mfxU64 timeout, mfxU32 ID);
    mfxU32 GetFreeSurfacesCount(bool isDec);
    PreEncAuxBuffer* GetFreePreEncAuxBuffer();
    void SetEncCtrlRT(ExtendedSurface& extSurface, bool bInsertIDR);

    // parameters configuration functions
    mfxStatus InitDecMfxParams(sInputParams* pInParams);
    mfxStatus InitVppMfxParams(MfxVideoParamsWrapper& par, sInputParams* pInParams, mfxU32 ID = 0);
    virtual mfxStatus InitEncMfxParams(sInputParams* pInParams);
    mfxStatus InitPluginMfxParams(sInputParams* pInParams);
    virtual mfxU32 FileFourCC2EncFourCC(mfxU32 fcc);
    void FillFrameInfoForEncoding(mfxFrameInfo& info, sInputParams* pInParams);

    mfxStatus AllocAndInitVppDoNotUse(MfxVideoParamsWrapper& par, sInputParams* pInParams);
    mfxStatus AllocMVCSeqDesc();

    void FreeVppDoNotUse();
    void FreeMVCSeqDesc();

    mfxStatus AllocateSufficientBuffer(mfxBitstreamWrapper* pBS);
    mfxStatus PutBS();

    mfxStatus DumpSurface2File(mfxFrameSurface1* pSurface);
    mfxStatus ReplaceBlackSurface(mfxFrameSurface1* pSurface);
    mfxStatus Surface2BS(ExtendedSurface* pSurf, mfxBitstreamWrapper* pBS, mfxU32 fourCC);
    mfxStatus NV12toBS(mfxFrameSurface1* pSurface, mfxBitstreamWrapper* pBS);
    mfxStatus I420toBS(mfxFrameSurface1* pSurface, mfxBitstreamWrapper* pBS);
    mfxStatus NV12asI420toBS(mfxFrameSurface1* pSurface, mfxBitstreamWrapper* pBS);
    mfxStatus RGB4toBS(mfxFrameSurface1* pSurface, mfxBitstreamWrapper* pBS);
    mfxStatus YUY2toBS(mfxFrameSurface1* pSurface, mfxBitstreamWrapper* pBS);

    void NoMoreFramesSignal();
    mfxStatus AddLaStreams(mfxU16 width, mfxU16 height);

    void LockPreEncAuxBuffer(PreEncAuxBuffer* pBuff);
    void UnPreEncAuxBuffer(PreEncAuxBuffer* pBuff);

    mfxU32 GetNumFramesForReset();
    void SetNumFramesForReset(mfxU32 nFrames);

    void HandlePossibleGpuHang(mfxStatus& sts);

    mfxStatus SetAllocatorAndHandleIfRequired();
    mfxStatus LoadGenericPlugin();

    mfxBitstreamWrapper* m_pmfxBS; // contains encoded input data

    mfxVersion m_Version; // real API version with which library is initialized

    mfxLoader m_mfxLoader;

    //session and VPP for cascade scaling
    std::map<mfxU32, std::unique_ptr<MainVideoSession>> m_pmfxCSSession;
    std::map<mfxU32, std::unique_ptr<MFXVideoMultiVPP>> m_pmfxCSVPP;

    std::unique_ptr<MainVideoSession> m_pmfxSession;
    std::unique_ptr<MFXVideoDECODE> m_pmfxDEC;
    std::unique_ptr<MFXVideoENCODE> m_pmfxENC;
    std::unique_ptr<MFXVideoMultiVPP>
        m_pmfxVPP; // either VPP or VPPPlugin which wraps [VPP]-Plugin-[VPP] pipeline
    mfxFrameAllocResponse m_mfxDecResponse; // memory allocation response for decoder
    mfxFrameAllocResponse m_mfxEncResponse; // memory allocation response for encoder

    MFXFrameAllocator* m_pMFXAllocator;
    void* m_hdl; // Diret3D device manager
    bool m_bIsInterOrJoined;

    mfxU32 m_numEncoders;
    mfxU32 m_encoderFourCC;

    CSmplYUVWriter m_dumpVppCompFileWriter;
    mfxU32 m_vppCompDumpRenderMode;

#if defined(_WIN32) || defined(_WIN64)
    CDecodeD3DRender* m_hwdev4Rendering;
#else
    CHWDevice* m_hwdev4Rendering;
#endif

    SurfPointersArray m_pSurfaceDecPool;
    SurfPointersArray m_pSurfaceEncPool;

    mfxFrameAllocRequest m_DecOutAllocReques;
    mfxFrameAllocRequest m_VPPOutAllocReques;

    std::map<mfxU32, SurfPointersArray> m_CSSurfacePools;

    mfxU16 m_EncSurfaceType; // actual type of encoder surface pool
    mfxU16 m_DecSurfaceType; // actual type of decoder surface pool

    PreEncAuxArray m_pPreEncAuxPool;

    // transcoding pipeline specific
    BSList m_BSPool;

    mfxInitParamlWrap m_initPar;

    volatile bool m_bForceStop;

    bool m_forceSyncAllSession;
    std::shared_ptr<SurfaceUtilizationSynchronizer> m_pSurfaceUtilizationSynchronizer;

    sPluginParams m_decoderPluginParams;
    sPluginParams m_encoderPluginParams;

    MfxVideoParamsWrapper m_mfxDecParams;
    MfxVideoParamsWrapper m_mfxEncParams;
    MfxVideoParamsWrapper m_mfxVppParams;

    std::map<mfxU32, MfxVideoParamsWrapper> m_mfxCSVppParams;

    MfxVideoParamsWrapper m_mfxPluginParams;
    bool m_bIsVpp; // true if there's VPP in the pipeline
    bool m_bIsFieldWeaving;
    bool m_bIsFieldSplitting;
    bool m_bIsPlugin; //true if there's Plugin in the pipeline
    RotateParam m_RotateParam;
    MfxVideoParamsWrapper m_mfxPreEncParams;
    mfxU32 m_nTimeout;
    bool m_bUseOverlay;

    bool m_bROIasQPMAP;
    bool m_bExtMBQP;
    // various external buffers
    bool
        m_bOwnMVCSeqDescMemory; // true if the pipeline owns memory allocated for MVCSeqDesc structure fields

    // to enable to-do list
    // number of video enhancement filters (denoise, procamp, detail, video_analysis, multi_view, ste, istab, tcc, ace, svc)
    constexpr static uint32_t ENH_FILTERS_COUNT = 20;
    mfxU32 m_tabDoUseAlg[ENH_FILTERS_COUNT];

    mfxU32 m_nID;
    mfxU16 m_AsyncDepth;
    mfxU32 m_nProcessedFramesNum;
    mfxU32 m_nTotalFramesNum;

    bool m_bIsJoinSession;

    bool m_bAllocHint;
    mfxU32 m_nPreallocate;

    bool m_bDecodeEnable;
    bool m_bEncodeEnable;
    mfxU32 m_nVPPCompMode;
    mfxI32 m_libvaBackend;

    mfxU16 m_MemoryModel;

    mfxSyncPoint m_LastDecSyncPoint;

    SafetySurfaceBuffer* m_pBuffer;
    CTranscodingPipeline* m_pParentPipeline;

    mfxFrameAllocRequest m_Request;
    bool m_bIsInit;

    mfxU32 m_NumFramesForReset;
    std::mutex m_mReset;
    std::mutex m_mStopSession;
    bool m_bRobustFlag;
    bool m_bSoftGpuHangRecovery;

    bool isHEVCSW;

    bool m_bInsertIDR;

    bool m_rawInput;
    bool m_shouldUseShifted10BitEnc;

    std::unique_ptr<ExtendedBSStore> m_pBSStore;

    mfxU32 m_FrameNumberPreference;
    mfxU32 m_MaxFramesForTranscode;
    mfxU32 m_MaxFramesForEncode;

    mfxU16 m_ExactNframe;
    mfxU16 m_Prolonged;

    // pointer to already extended bs processor
    FileBitstreamProcessor* m_pBSProcessor;

    msdk_tick m_nReqFrameTime; // time required to transcode one frame

    mfxU32 statisticsWindowSize; // Sliding window size for Statistics
    mfxU32 m_nOutputFramesNum;

    CIOStat inputStatistics;
    CIOStat outputStatistics;

    bool shouldUseGreedyFormula;

    // ROI data
    std::vector<mfxExtEncoderROI> m_ROIData;
    mfxU32 m_nSubmittedFramesNum;

    // ROI with MBQP map data
    bool m_bUseQPMap;

    std::map<void*, mfxExtMBQP> m_bufExtMBQP;
    std::map<void*, std::vector<mfxU8>> m_qpMapStorage;
    std::map<void*, std::vector<mfxExtBuffer*>> m_extBuffPtrStorage;
    std::map<void*, mfxEncodeCtrl> encControlStorage;

    mfxU32 m_QPmapWidth;
    mfxU32 m_QPmapHeight;
    mfxU32 m_GOPSize;
    mfxU32 m_QPforI;
    mfxU32 m_QPforP;

    std::string m_sGenericPluginPath;
    mfxU16 m_nRotationAngle;

    std::string dump_file;

    void FillMBQPBuffer(mfxExtMBQP& qpMap, mfxU16 pictStruct);

    TCBRCTestFile::Reader m_TCBRCFileReader;
    bool m_bTCBRCFileMode;
    mfxStatus ConfigTCBRCTest(mfxFrameSurface1* pSurf);

#ifdef ENABLE_MCTF
    sMctfRunTimeParams m_MctfRTParams;
#endif

    //Adapter type
    mfxU16 m_adapterType = mfxMediaAdapterType::MFX_MEDIA_UNKNOWN;
    mfxI32 m_dGfxIdx     = -1;
    mfxI32 m_adapterNum  = -1;
    mfxU32 TargetID      = 0;

    CascadeScalerConfig m_ScalerConfig;

    mfxU32 m_surface_wait_interval =
        MSDK_SURFACE_WAIT_INTERVAL; // Surface wait when getting free surface from pool
    mfxU32 m_nSyncOpTimeout = MSDK_WAIT_INTERVAL; // SyncOperation timeout in msec

#if (defined(_WIN32) || defined(_WIN64))
        //Adapter type
    bool bPreferiGfx;
    bool bPreferdGfx;
#endif

    eAPIVersion m_verSessionInit;
    bool m_b3DLutEnable;
    // 3DLUT video memory default is 65*65*128*4*2 bytes
    mfxU32 m_n3DLutVMemId;
    mfxU32 m_n3DLutVWidth;
    mfxU32 m_n3DLutVHeight;
    std::string m_p3DLutFile;

#if (defined(_WIN32) || defined(_WIN64))
    mfxStatus CheckHyperEncodeParams(mfxHyperMode hyperMode);
#endif

private:
    DISALLOW_COPY_AND_ASSIGN(CTranscodingPipeline);
};

struct ThreadTranscodeContext {
    // Pointer to the session's pipeline
    std::unique_ptr<CTranscodingPipeline> pPipeline;
    // Pointer to bitstream handling object
    FileBitstreamProcessor* pBSProcessor = nullptr;
    // Session implementation type
    mfxIMPL implType = MFX_IMPL_AUTO;

    // Session's starting status
    mfxStatus startStatus = MFX_ERR_NONE;
    // Session's working time
    mfxF64 working_time = 0;

    // Number of processed frames
    mfxU32 numTransFrames = 0;
    // Status of the finished session
    mfxStatus transcodingSts = MFX_ERR_NONE;

    // Thread handle
    std::future<void> handle;

    void TranscodeRoutine() {
        using namespace std::chrono;
        MSDK_CHECK_POINTER_NO_RET(pPipeline);
        transcodingSts = MFX_ERR_NONE;

        auto start_time = system_clock::now();
        while (MFX_ERR_NONE == transcodingSts) {
            transcodingSts = pPipeline->Run();
        }
        working_time = duration_cast<duration<mfxF64>>(system_clock::now() - start_time).count();

        MSDK_IGNORE_MFX_STS(transcodingSts, MFX_WRN_VALUE_NOT_CHANGED);
        numTransFrames = pPipeline->GetProcessFrames();
    }
};
} // namespace TranscodingSample

#endif
