/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __PIPELINE_ENCODE_H__
#define __PIPELINE_ENCODE_H__

#include "hw_device.h"
#include "sample_defs.h"

#ifdef D3D_SURFACES_SUPPORT
    #pragma warning(disable : 4201)
#endif

#include "base_allocator.h"
#include "sample_utils.h"
#include "time_statistics.h"

#include "mfxplugin.h"
#include "vpl/mfxmvc.h"
#include "vpl/mfxvideo++.h"
#include "vpl/mfxvideo.h"

#include "vpl/mfxdispatcher.h"

#include "plugin_utils.h"

#include <memory>
#include <vector>

#include "preset_manager.h"

#if defined(ENABLE_V4L2_SUPPORT)
    #include "v4l2_util.h"
#endif

#include "brc_routines.h"

#if defined(_WIN64) || defined(_WIN32)
    #include "vpl/mfxadapter.h"
#endif

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

#define SAFE_FREAD(PTR, SZ, COUNT, FPTR, ERR)                 \
    {                                                         \
        if (FPTR && (fread(PTR, SZ, COUNT, FPTR) != COUNT)) { \
            return ERR;                                       \
        }                                                     \
    }

msdk_tick time_get_tick(void);
msdk_tick time_get_frequency(void);

enum {
    MVC_DISABLED   = 0x0,
    MVC_ENABLED    = 0x1,
    MVC_VIEWOUTPUT = 0x2, // 2 output bitstreams
};

enum MemType {
    SYSTEM_MEMORY = 0x00,
    D3D9_MEMORY   = 0x01,
    D3D11_MEMORY  = 0x02,
};

// the default api version is the latest one
// it is located at 0
enum eAPIVersion { API_2X, API_1X };

struct sInputParams {
    mfxU16 nTargetUsage;
    mfxU32 CodecId;
    mfxU32 FileInputFourCC;
    mfxU32 EncodeFourCC;
    mfxU16 nPicStruct;
    mfxU16 nWidth; // source picture width
    mfxU16 nHeight; // source picture height
    mfxF64 dFrameRate;
    mfxU32 nNumFrames;
    mfxU32 nBitRate;
    mfxU16 MVC_flags;
    mfxU16 nGopPicSize;
    mfxU16 nGopRefDist;
    mfxU16 nNumRefFrame;
    mfxU16 nBRefType;
    mfxU16 nPRefType;
    mfxU16 nIdrInterval;
    mfxU16 nNumRefActiveP;
    mfxU16 nNumRefActiveBL0;
    mfxU16 nNumRefActiveBL1;
    mfxU16 reserved[4];

    mfxU16 nQuality; // quality parameter for JPEG encoder

    mfxU32 numViews; // number of views for Multi-View Codec
    mfxU16 nDstWidth; // destination picture width, specified if resizing required
    mfxU16 nDstHeight; // destination picture height, specified if resizing required

    mfxU16 nEncTileRows; // number of rows for encoding tiling
    mfxU16 nEncTileCols; // number of columns for encoding tiling

    msdk_string strQPFilePath;
    msdk_string strTCBRCFilePath;

    mfxAccelerationMode accelerationMode;

    mfxIMPL libType;
    MemType memType;
    bool bUseHWLib; // true if application wants to use HW MSDK library

#if defined(LINUX32) || defined(LINUX64)
    std::string strDevicePath;
#endif
#if (defined(_WIN64) || defined(_WIN32)) && (MFX_VERSION >= 1031)
    bool bPreferdGfx;
    bool bPreferiGfx;
#endif
// Extended device ID info, available in 2.6 and newer APIs
#if (defined(_WIN64) || defined(_WIN32))
    LUID luid;
#else
    mfxU32 DRMRenderNodeNum = 0;
#endif
    mfxU32 PCIDomain    = 0;
    mfxU32 PCIBus       = 0;
    mfxU32 PCIDevice    = 0;
    mfxU32 PCIFunction  = 0;
    bool PCIDeviceSetup = false;

#if (defined(_WIN64) || defined(_WIN32))
    bool isDualMode;
    mfxHyperMode hyperMode;
#endif
    mfxU16 adapterType;
    mfxI32 dGfxIdx;
    mfxI32 adapterNum;
    bool dispFullSearch;

    std::list<msdk_string> InputFiles;

    sPluginParams pluginParams;

    std::vector<msdk_char*> dstFileBuff;

    mfxU32 HEVCPluginVersion;
    mfxU8 nRotationAngle; // if specified, enables rotation plugin in mfx pipeline
    msdk_char strPluginDLLPath[MSDK_MAX_FILENAME_LEN]; // plugin dll path and name

    mfxU16
        nAsyncDepth; // depth of asynchronous pipeline, this number can be tuned to achieve better performance
    mfxU16 gpuCopy; // GPU Copy mode (three-state option)

    mfxU16 nRateControlMethod;
    mfxU16 nLADepth; // depth of the look ahead bitrate control  algorithm
    mfxU16 nMaxSliceSize; //maximum size of slice
    mfxU16 nQPI;
    mfxU16 nQPP;
    mfxU16 nQPB;
    mfxU16 nTemp;
    mfxU16 nBaseLayerPID;
    mfxU16 nTemporalLayers[8];
    mfxU16 nSPSId;
    mfxU16 nPPSId;
    mfxU16 nPicTimingSEI;
    mfxU16 nNalHrdConformance;
    mfxU16 nVuiNalHrdParameters;

    mfxU16 nGPB;
    mfxU16 nTransformSkip;
    ExtBRCType nExtBRC;
    mfxU16 nAdaptiveMaxFrameSize;

    mfxU16 WeightedPred;
    mfxU16 WeightedBiPred;

    mfxU16 TransferMatrix;

    bool enableQSVFF;

    bool bSoftRobustFlag;

    bool QPFileMode;
    bool TCBRCFileMode;

    mfxU32 nTimeout;
    mfxU16 nPerfOpt; // size of pre-load buffer which used for loop encode
    mfxU16 nMaxFPS; // limits overall fps

    mfxU32 nSyncOpTimeout; // SyncOperation timeout in msec

    mfxU16 nNumSlice;
    bool UseRegionEncode;

    bool isV4L2InputEnabled;

    // enctools config
    bool bEncTools;
    mfxU16 etAdaptiveI;
    mfxU16 etAdaptiveB;
    mfxU16 etArefP;
    mfxU16 etArefB;
    mfxU16 etSceneChange;
    mfxU16 etALTR;
    mfxU16 etApyrQP;
    mfxU16 etApyrQB;
    mfxU16 etBRCHints;
    mfxU16 etBRC;

    mfxU16 CodecLevel;
    mfxU16 CodecProfile;
    mfxU32 MaxKbps;
    mfxU32 BufferSizeInKB;
    mfxU32 InitialDelayInKB;
    mfxU16 nBitRateMultiplier;
    mfxU16 GopOptFlag;
    mfxU16 AdaptiveI;
    mfxU16 AdaptiveB;
    mfxU32 nMaxFrameSize;

    mfxU16 BitrateLimit;

    mfxU16 WinBRCSize;
    mfxU16 WinBRCMaxAvgKbps;

    mfxU16 ICQQuality;
    mfxU16 QVBRQuality;
    mfxU16 Convergence;
    mfxU16 Accuracy;
    mfxU16 LowDelayBRC;
    mfxU16 ExtBrcAdaptiveLTR;
    mfxU16 PartialOutputMode;
    mfxU16 PartialOutputBlockSize;
    mfxU16 ScenarioInfo;

    mfxU16 IntRefType;
    mfxU16 IntRefCycleSize;
    mfxU16 IntRefQPDelta;
    mfxU16 IntRefCycleDist;

    bool bUncut;
    bool shouldUseShifted10BitEnc;
    bool shouldUseShifted10BitVPP;
    bool IsSourceMSB;

    bool bSingleTexture;

    mfxU16 nIVFHeader;

    msdk_char* RoundingOffsetFile;
    mfxU16 TargetBitDepthLuma;
    mfxU16 TargetBitDepthChroma;
    msdk_char DumpFileName[MSDK_MAX_FILENAME_LEN];
    msdk_char uSEI[MSDK_MAX_USER_DATA_UNREG_SEI_LEN];

    EPresetModes PresetMode;
    bool shouldPrintPresets;

#if defined(ENABLE_V4L2_SUPPORT)
    msdk_char DeviceName[MSDK_MAX_FILENAME_LEN];
    msdk_char MipiModeName[MSDK_MAX_FILENAME_LEN];
    enum V4L2PixelFormat v4l2Format;
    int MipiPort;
    enum AtomISPMode MipiMode;
#endif

#if (MFX_VERSION >= MFX_VERSION_NEXT)
    mfxI16 DeblockingAlphaTcOffset;
    mfxI16 DeblockingBetaOffset;
#endif
    eAPIVersion verSessionInit;
    bool bReadByFrame;
};

struct sTask {
    mfxBitstreamWrapper mfxBS;
    mfxEncodeCtrlWrap encCtrl;
    bool bUseHWLib;
    mfxSyncPoint EncSyncP;
    std::list<mfxSyncPoint> DependentVppTasks;
    void* pWriter;
    mfxU32 codecID;

    sTask();
    mfxStatus WriteBitstream(bool isCompleteFrame = true);
    mfxStatus Reset();
    mfxStatus Init(mfxU32 nBufferSize, mfxU32 nCodecID, void* pWriter = NULL, bool bHWLib = false);
    mfxStatus Close();
};

class CEncTaskPool {
public:
    CEncTaskPool();
    virtual ~CEncTaskPool();

    virtual mfxStatus Init(MFXVideoSession* pmfxSession,
                           void* pWriter,
                           mfxU32 nPoolSize,
                           mfxU32 nBufferSize,
                           mfxU32 CodecID,
                           void* pOtherWriter = NULL,
                           bool bUseHWLib     = false);
    virtual mfxStatus GetFreeTask(sTask** ppTask);
    virtual mfxStatus SynchronizeFirstTask(mfxU32 syncOpTimeout);

    virtual CTimeStatistics& GetOverallStatistics() {
        return m_statOverall;
    }
    virtual CTimeStatistics& GetFileStatistics() {
        return m_statFile;
    }
    virtual void Close();
    virtual void SetGpuHangRecoveryFlag();
    virtual void ClearTasks();

    msdk_tick firstOut_total;
    msdk_tick firstOut_start;
    msdk_tick lastOut_total;
    msdk_tick lastOut_start;

protected:
    sTask* m_pTasks;
    mfxU32 m_nPoolSize;
    mfxU32 m_nTaskBufferStart;

    bool m_bGpuHangRecovery;

    MFXVideoSession* m_pmfxSession;

    CTimeStatistics m_statOverall;
    CTimeStatistics m_statFile;
    virtual mfxU32 GetFreeTaskIndex();
};

/* This class implements a pipeline with 2 mfx components: vpp (video preprocessing) and encode */
class CEncodingPipeline {
public:
    CEncodingPipeline();
    virtual ~CEncodingPipeline();

    virtual mfxStatus Init(sInputParams* pParams);
    virtual mfxStatus Run();
    virtual void Close();
    virtual mfxStatus ResetMFXComponents(sInputParams* pParams);
    virtual mfxStatus ResetDevice();

    void SetNumView(mfxU32 numViews) {
        m_nNumView = numViews;
    }
    virtual void PrintInfo();

    void InitV4L2Pipeline(sInputParams* pParams);
    mfxStatus CaptureStartV4L2Pipeline();
    void CaptureStopV4L2Pipeline();

    static void InsertIDR(mfxEncodeCtrl& ctrl, bool forceIDR);

    virtual mfxStatus OpenRoundingOffsetFile(sInputParams* pInParams);
    mfxStatus InitEncFrameParams(sTask* pTask);
    mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height);

#if defined(ENABLE_V4L2_SUPPORT)
    v4l2Device v4l2Pipeline;
    pthread_t m_PollThread;
#endif

protected:
    std::pair<CSmplBitstreamWriter*, CSmplBitstreamWriter*> m_FileWriters;
    CSmplYUVReader m_FileReader;
    CEncTaskPool m_TaskPool;
    QPFile::Reader m_QPFileReader;
    TCBRCTestFile::Reader m_TCBRCFileReader;

    std::unique_ptr<VPLImplementationLoader> m_pLoader;
    MainVideoSession m_mfxSession;
    MFXVideoENCODE* m_pmfxENC;
    MFXVideoVPP* m_pmfxVPP;

    bool m_bNoOutFile;

    MfxVideoParamsWrapper m_mfxEncParams;
    MfxVideoParamsWrapper m_mfxVppParams;

    mfxU16 m_MVCflags; // MVC codec is in use

    mfxU32 m_InputFourCC;

    MFXFrameAllocator* m_pMFXAllocator;
    mfxAllocatorParams* m_pmfxAllocatorParams;
    MemType m_memType;
    mfxU16 m_nPerfOpt; // size of pre-load buffer which used for loop encode
    bool m_bExternalAlloc; // use memory allocator as external for Media SDK

    mfxFrameSurface1* m_pEncSurfaces; // frames array for encoder input (vpp output)
    mfxFrameSurface1* m_pVppSurfaces; // frames array for vpp input
    mfxFrameAllocResponse m_EncResponse; // memory allocation response for encoder
    mfxFrameAllocResponse m_VppResponse; // memory allocation response for vpp

    std::vector<mfxEncodeCtrl> m_EncCtrls; // controls array for encoder input

    mfxU32 m_nNumView;
    mfxU32 m_nFramesToProcess; // number of frames to process

#if defined(LINUX32) || defined(LINUX64)
    std::string m_strDevicePath; //path to device for processing
#endif

    std::vector<mfxPayload*> m_UserDataUnregSEI;

    CHWDevice* m_hwdev;

    bool m_bQPFileMode;
    bool m_bTCBRCFileMode;

    bool isV4L2InputEnabled;
    FILE* m_round_in;
    bool m_bSoftRobustFlag;

    mfxU32 m_nTimeout;

    mfxU32 m_nSyncOpTimeout; // SyncOperation timeout in msec

    bool m_bFileWriterReset;
    mfxU32 m_nFramesRead;
    bool m_bCutOutput;
    bool m_bInsertIDR;
    bool m_bTimeOutExceed;

    bool m_bIsFieldSplitting;
    bool m_bSingleTexture;
    bool m_bPartialOutput;

    CTimeStatisticsReal m_statOverall;
    CTimeStatisticsReal m_statFile;

    FPSLimiter m_fpsLimiter;

    eAPIVersion m_verSessionInit;
    bool m_bReadByFrame;

#if (defined(_WIN64) || defined(_WIN32)) && (MFX_VERSION >= 1031)
    mfxU32 GetPreferredAdapterNum(const mfxAdaptersInfo& adapters, const sInputParams& params);
#endif
    mfxStatus GetImpl(const sInputParams& params, mfxIMPL& impl);
    virtual mfxStatus InitMfxEncParams(sInputParams* pParams);
    virtual mfxStatus InitMfxVppParams(sInputParams* pParams);

    virtual mfxStatus InitFileWriters(sInputParams* pParams);
    virtual void FreeFileWriters();
    virtual mfxStatus InitFileWriter(CSmplBitstreamWriter** ppWriter, const msdk_char* filename);
    virtual mfxStatus InitFileWriter(CSmplBitstreamWriter** ppWriter,
                                     const msdk_char* filename,
                                     bool no_outfile);

    virtual mfxStatus InitIVFFileWriter(CIVFFrameWriter** ppWriter,
                                        const msdk_char* filename,
                                        const mfxU16 w,
                                        const mfxU16 h,
                                        const mfxU32 fr_nom,
                                        const mfxU32 fr_denom,
                                        const bool no_outfile);

    virtual mfxStatus InitVppFilters();
    virtual void FreeVppFilters();

    virtual void DeallocateExtMVCBuffers();

    virtual mfxStatus CreateAllocator();
    virtual void DeleteAllocator();

    virtual mfxStatus CreateHWDevice();
    virtual void DeleteHWDevice();

    virtual mfxStatus AllocFrames();
    virtual void DeleteFrames();

    virtual mfxU32 GetSufficientBufferSize();
    virtual mfxStatus AllocateSufficientBuffer(mfxBitstreamWrapper& bs);
    virtual mfxStatus FillBuffers();
    virtual mfxStatus LoadNextFrame(mfxFrameSurface1* pSurf);
    virtual mfxStatus LoadNextFrame(mfxFrameSurface1* pSurf, int bytes_to_read, mfxU8* buf_read);
    virtual void LoadNextControl(mfxEncodeCtrl*& pCtrl, mfxU32 encSurfIdx);

    virtual mfxStatus GetFreeTask(sTask** ppTask);
    virtual MFXVideoSession& GetFirstSession() {
        return m_mfxSession;
    }
    virtual MFXVideoENCODE* GetFirstEncoder() {
        return m_pmfxENC;
    }

    virtual mfxU32 FileFourCC2EncFourCC(mfxU32 fcc);

    void InitExtMVCBuffers(mfxExtMVCSeqDesc* mvcBuffer) const;
    mfxStatus ConfigTCBRCTest(mfxFrameSurface1* pSurf);
};

#endif // __PIPELINE_ENCODE_H__
