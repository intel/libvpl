/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __SMT_CLI_PARAMS_H__
#define __SMT_CLI_PARAMS_H__

#include "smt_tracer.h"
#include "vpl/mfx.h"
namespace TranscodingSample {

typedef enum ExtBRCType { EXTBRC_DEFAULT, EXTBRC_OFF, EXTBRC_ON, EXTBRC_IMPLICIT } ExtBRCType;

typedef enum EFieldCopyMode {
    FC_NONE  = 0,
    FC_T2T   = 1,
    FC_T2B   = 2,
    FC_B2T   = 4,
    FC_B2B   = 8,
    FC_FR2FR = 16
} EFieldCopyMode;

typedef enum PipelineMode {
    Native =
        0, // means that pipeline is based depends on the cmd parameters (decode/encode/transcode)
    Sink, // means that pipeline makes decode only and put data to shared buffer
    Source, // means that pipeline makes vpp + encode and get data from shared buffer
    VppComp, // means that pipeline makes vpp composition + encode and get data from shared buffer
    VppCompOnly, // means that pipeline makes vpp composition and get data from shared buffer
    VppCompOnlyEncode // means that pipeline makes vpp composition + encode and get data from shared buffer
} PipelineMode;

typedef struct sVppCompDstRect {
    sVppCompDstRect() : DstX(0), DstY(0), DstW(0), DstH(0), TileId(0) {}
    mfxU32 DstX;
    mfxU32 DstY;
    mfxU32 DstW;
    mfxU32 DstH;
    mfxU16 TileId;
} sVppCompDstRect;

typedef struct sSEIMetaMasteringDisplay {
    sSEIMetaMasteringDisplay()
            : InsertPayloadToggle(0),
              DisplayPrimariesX{ 0 },
              DisplayPrimariesY{ 0 },
              WhitePointX(0),
              WhitePointY(0),
              MaxDisplayMasteringLuminance(0),
              MinDisplayMasteringLuminance(0) {}

    mfxU16 InsertPayloadToggle;
    mfxU16 DisplayPrimariesX[3];
    mfxU16 DisplayPrimariesY[3];
    mfxU16 WhitePointX;
    mfxU16 WhitePointY;
    mfxU32 MaxDisplayMasteringLuminance;
    mfxU32 MinDisplayMasteringLuminance;
} sSEIMetaMasteringDisplay;

typedef struct sSEIMetaContentLightLevel {
    sSEIMetaContentLightLevel()
            : InsertPayloadToggle(0),
              MaxContentLightLevel(0),
              MaxPicAverageLightLevel(0) {}
    mfxU16 InsertPayloadToggle;
    mfxU16 MaxContentLightLevel;
    mfxU16 MaxPicAverageLightLevel;
} sSEIMetaContentLightLevel;

typedef struct sVideoSignalInfo {
    sVideoSignalInfo() : Enabled(false), VideoFullRange(0), ColourPrimaries(0) {}
    bool Enabled;
    mfxU16 VideoFullRange;
    mfxU16 ColourPrimaries;
} sVideoSignalInfo;

#ifdef ENABLE_MCTF

// this is a structure with mctf-parameteres
// that can be changed in run-time;
typedef struct sMctfRunTimeParam {
    mfxU16 FilterStrength;
} sMctfRunTimeParam;

typedef struct sMctfRunTimeParams {
    sMctfRunTimeParams() : CurIdx(0), RunTimeParams() {}

    mfxU32 CurIdx;
    std::vector<sMctfRunTimeParam> RunTimeParams;
    // returns rt-param corresponding to CurIdx or NULL if
    // CurIdx is behind available info
    const sMctfRunTimeParam* GetCurParam();
    // move CurIdx forward
    void MoveForward();
    // set CurIdx to the begining; restart indexing;
    void Restart();
    // reset vector & index
    void Reset();
    // test for emptiness
    bool Empty() {
        return RunTimeParams.empty();
    };
} sMctfRunTimeParams;

typedef enum VPPFilterMode {
    VPP_FILTER_DISABLED           = 0,
    VPP_FILTER_ENABLED_DEFAULT    = 1,
    VPP_FILTER_ENABLED_CONFIGURED = 7
} VPPFilterMode;

typedef struct sMCTFParam {
    sMCTFParam() : rtParams(), params{ 0 }, mode(VPP_FILTER_DISABLED) {}
    sMctfRunTimeParams rtParams;
    mfxExtVppMctf params;
    VPPFilterMode mode;
} sMCTFParam;
#endif

// the default api version is the latest one
// it is located at 0
typedef enum eAPIVersion { API_2X, API_1X } eAPIVersion;

typedef struct sInputParams {
    mfxU32 TargetID;
    bool CascadeScaler;
    bool EnableTracing;
    mfxU32 TraceBufferSize;
    SMTTracer::LatencyType LatencyType;
    bool ParallelEncoding;

    // session parameters
    bool bIsJoin;
    mfxPriority priority;
    // common parameters
    mfxIMPL libType; // Type of used mediaSDK library
#if defined(LINUX32) || defined(LINUX64)
    std::string strDevicePath;
#endif
// Extended device ID info, available in 2.6 and newer APIs
#if (defined(_WIN64) || defined(_WIN32))
    LUID luid;
#else
    mfxU32 DRMRenderNodeNum;
#endif
    mfxU32 PCIDomain;
    mfxU32 PCIBus;
    mfxU32 PCIDevice;
    mfxU32 PCIFunction;
    bool PCIDeviceSetup;

#if (defined(_WIN64) || defined(_WIN32))
    bool isDualMode;
    mfxHyperMode hyperMode;
#endif
#if (defined(_WIN32) || defined(_WIN64))
    //Adapter type
    bool bPreferiGfx;
    bool bPreferdGfx;
#endif

    mfxU16 nIdrInterval;

    //Adapter type
    mfxU16 adapterType;
    mfxI32 dGfxIdx;
    mfxI32 adapterNum;

    bool dispFullSearch;

    mfxU16 nThreadsNum; // number of internal session threads number
    bool bRobustFlag; // Robust transcoding mode. Allows auto-recovery after hardware errors
    bool bSoftRobustFlag;

    mfxU32 EncodeId; // type of output coded video
    mfxU32 DecodeId; // type of input coded video

    std::string strSrcFile; // source bitstream file
    std::string strDstFile; // destination bitstream file
    std::string strDumpVppCompFile; // VPP composition output dump file
    std::string dump_file;

    std::string strTCBRCFilePath;

    std::string m_encode_cfg;
    std::string m_decode_cfg;
    std::string m_vpp_cfg;

    // specific encode parameters
    mfxU16 nTargetUsage;
    mfxF64 dDecoderFrameRateOverride;
    mfxF64 dEncoderFrameRateOverride;
    mfxU16 EncoderPicstructOverride;
    mfxF64 dVPPOutFramerate;
    mfxU32 nBitRate;
    mfxU16 nBitRateMultiplier;
    mfxU16 nQuality; // quality parameter for JPEG encoder
    mfxU16 nDstWidth; // destination picture width, specified if resizing required
    mfxU16 nDstHeight; // destination picture height, specified if resizing required

    mfxU16 nEncTileRows; // number of rows for encoding tiling
    mfxU16 nEncTileCols; // number of columns for encoding tiling

    bool
        bEmbeddedDenoiser; // enable or disable embeded HVS Denoiser (HVS denoiser is integrated into encoder)
    mfxU16
        EmbeddedDenoiseMode; // embeded HVS Denoiser Mode (HVS denoiser is integrated into encoder)
    int EmbeddedDenoiseLevel; // embeded HVS Denoiser Level (HVS denoiser is integrated into encoder)

    bool bEnableDeinterlacing;
    mfxU16 DeinterlacingMode;
    bool bVppDenoiser;
    int VppDenoiseLevel;
    mfxU16 VppDenoiseMode;
    int DetailLevel;
    mfxU16 FRCAlgorithm;
    EFieldCopyMode fieldProcessingMode;
    mfxU16 ScalingMode;

    mfxU16 nAsyncDepth; // asyncronous queue

    PipelineMode eMode;
    PipelineMode eModeExt;

    mfxU32 FrameNumberPreference; // how many surfaces user wants
    mfxU32 MaxFrameNumber; // maximum frames for transcoding
    mfxU16 prolonged;
    mfxU16 ExactNframe;
    mfxU32 numSurf4Comp;
    mfxU16 numTiles4Comp;

    mfxU16 nSlices; // number of slices for encoder initialization
    mfxU16 nMaxSliceSize; //maximum size of slice

    mfxU16 WinBRCMaxAvgKbps;
    mfxU16 WinBRCSize;
    mfxU32 BufferSizeInKB;
    mfxU16 GopPicSize;
    mfxU16 GopRefDist;
    mfxU16 NumRefFrame;
    mfxU16 nNumRefActiveP;
    mfxU16 nBRefType;
    mfxU16 RepartitionCheckMode;
    mfxU16 GPB;
    mfxU16 nTransformSkip;

    mfxU16 CodecLevel;
    mfxU16 CodecProfile;
    mfxU32 MaxKbps;
    mfxU32 InitialDelayInKB;
    mfxU16 GopOptFlag;
    mfxU16 AdaptiveI;
    mfxU16 AdaptiveB;
    mfxU16 AdaptiveCQM;

    mfxU16 WeightedPred;
    mfxU16 WeightedBiPred;
    mfxU16 ExtBrcAdaptiveLTR;

    bool bExtMBQP;

    // MVC Specific Options
    bool bIsMVC; // true if Multi-View-Codec is in use
    mfxU32 numViews; // number of views for Multi-View-Codec

    mfxU16 nRotationAngle; // if specified, enables rotation plugin in mfx pipeline
    std::string strVPPPluginDLLPath; // plugin dll path and name

    sPluginParams decoderPluginParams;
    sPluginParams encoderPluginParams;

    mfxU32 nTimeout; // how long transcoding works in seconds
    mfxU32 nFPS; // limit transcoding to the number of frames per second

    mfxU32 statisticsWindowSize;
    FILE* statisticsLogFile;

    bool bLABRC; // use look ahead bitrate control algorithm
    mfxU16 nLADepth; // depth of the look ahead bitrate control  algorithm
    bool bEnableExtLA;
    bool bEnableBPyramid;
    mfxU16 nRateControlMethod;
    mfxU16 nQPI;
    mfxU16 nQPP;
    mfxU16 nQPB;
    bool bDisableQPOffset;
    bool bSetQPOffset;
    mfxU16 QPOffset[8];
    mfxU8 nMinQPI;
    mfxU8 nMaxQPI;
    mfxU8 nMinQPP;
    mfxU8 nMaxQPP;
    mfxU8 nMinQPB;
    mfxU8 nMaxQPB;

    mfxU16 nAvcTemp;
    mfxU16 nBaseLayerPID;
    mfxU16 nAvcTemporalLayers[8];
#if defined(_WIN32) || defined(_WIN64)
    mfxU16 bTemporalLayers;
    mfxTemporalLayer temporalLayers[8];
#endif
    mfxU16 nSPSId;
    mfxU16 nPPSId;
    mfxU16 nPicTimingSEI;
    mfxU16 nNalHrdConformance;
    mfxU16 nVuiNalHrdParameters;
    mfxU16 nTransferCharacteristics;

    bool bOpenCL;
    mfxU16 reserved[4];

    mfxU16 nVppCompDstX;
    mfxU16 nVppCompDstY;
    mfxU16 nVppCompDstW;
    mfxU16 nVppCompDstH;
    mfxU16 nVppCompSrcW;
    mfxU16 nVppCompSrcH;
    mfxU16 nVppCompTileId;

    mfxU32 DecoderFourCC;
    mfxU32 EncoderFourCC;

    sVppCompDstRect* pVppCompDstRects;

    bool bForceSysMem;
    mfxU16 DecOutPattern;
    mfxU16 VppOutPattern;
    mfxU16 nGpuCopyMode;

    mfxU16 nRenderColorForamt; /*0 NV12 - default, 1 is ARGB*/

    mfxI32 monitorType;
    bool shouldUseGreedyFormula;
    bool enableQSVFF;
    bool bSingleTexture;

    ExtBRCType nExtBRC;

    mfxU16 nAdaptiveMaxFrameSize;
    mfxU16 LowDelayBRC;

    mfxU16 IntRefType;
    mfxU16 IntRefCycleSize;
    mfxU16 IntRefQPDelta;
    mfxU16 IntRefCycleDist;

    mfxU32 nMaxFrameSize;

    mfxU16 BitrateLimit;

    mfxU16 numMFEFrames;
    mfxU16 MFMode;
    mfxU32 mfeTimeout;
    sSEIMetaMasteringDisplay SEIMetaMDCV;
    sSEIMetaContentLightLevel SEIMetaCLLI;

    bool bEnableMDCV;
    bool bEnableCLLI;

    sVideoSignalInfo SignalInfoIn;
    sVideoSignalInfo SignalInfoOut;

    mfxU16 TargetBitDepthLuma;
    mfxU16 TargetBitDepthChroma;

#if defined(LIBVA_WAYLAND_SUPPORT)
    mfxU16 nRenderWinX;
    mfxU16 nRenderWinY;
    bool bPerfMode;
#endif

#if defined(LIBVA_SUPPORT)
    mfxI32 libvaBackend;
#endif // defined(MFX_LIBVA_SUPPORT)

    CHWDevice* m_hwdev;

    EPresetModes PresetMode;
    bool shouldPrintPresets;

    bool rawInput;

    // 3DLut Binary File
    std::string str3DLutFile;
    bool bEnable3DLut;

    mfxU16 nMemoryModel;

    mfxPoolAllocationPolicy AllocPolicy;
    bool useAllocHints;
    mfxU32 preallocate;

    mfxU16 forceSyncAllSession;

    mfxU16 nIVFHeader;

    bool IsSourceMSB;
    mfxU32 nSyncOpTimeout; // SyncOperation timeout in msec

    bool TCBRCFileMode;

    std::string DumpLogFileName;

    std::vector<mfxExtEncoderROI> m_ROIData;

    bool bDecoderPostProcessing;
    bool bROIasQPMAP;
#ifdef ENABLE_MCTF
    sMCTFParam mctfParam;
#endif
#ifdef ONEVPL_EXPERIMENTAL
    bool PercEncPrefilter;
    mfxU32 TuneEncodeQuality;
#endif
    mfxU16 ScenarioInfo;
    mfxU16 ContentInfo;
    eAPIVersion verSessionInit;

    // set structure to define values
    sInputParams()
            : TargetID(0),
              CascadeScaler(false),
              EnableTracing(false),
              TraceBufferSize(0),
              LatencyType(SMTTracer::LatencyType::DEFAULT),
              ParallelEncoding(false),
              bIsJoin(false),
              priority(MFX_PRIORITY_NORMAL),
              libType(MFX_IMPL_SOFTWARE),
#if defined(LINUX32) || defined(LINUX64)
              strDevicePath(""),
#endif
#if (defined(_WIN64) || defined(_WIN32))
              luid{ 0 },
#else
              DRMRenderNodeNum(0),
#endif
              PCIDomain(0),
              PCIBus(0),
              PCIDevice(0),
              PCIFunction(0),
              PCIDeviceSetup(false),

#if (defined(_WIN64) || defined(_WIN32))
              isDualMode(false),
              hyperMode(MFX_HYPERMODE_OFF),
#endif
#if (defined(_WIN32) || defined(_WIN64))
              bPreferiGfx(false),
              bPreferdGfx(false),
#endif
              nIdrInterval(0),
              adapterType(mfxMediaAdapterType::MFX_MEDIA_UNKNOWN),
              dGfxIdx(-1),
              adapterNum(-1),
              dispFullSearch(DEF_DISP_FULLSEARCH),
              nThreadsNum(0),
              bRobustFlag(false),
              bSoftRobustFlag(false),
              EncodeId(0),
              DecodeId(0),
              strSrcFile(),
              strDstFile(),
              strDumpVppCompFile(),
              dump_file(),
              strTCBRCFilePath(),
              m_encode_cfg(),
              m_decode_cfg(),
              m_vpp_cfg(),
              nTargetUsage(0),
              dDecoderFrameRateOverride(0.0),
              dEncoderFrameRateOverride(0.0),
              EncoderPicstructOverride(0),
              dVPPOutFramerate(0.0),
              nBitRate(0),
              nBitRateMultiplier(0),
              nQuality(0),
              nDstWidth(0),
              nDstHeight(0),
              nEncTileRows(0),
              nEncTileCols(0),
              bEmbeddedDenoiser(false),
              EmbeddedDenoiseMode(0),
              EmbeddedDenoiseLevel(-1),
              bEnableDeinterlacing(false),
              DeinterlacingMode(0),
              bVppDenoiser(false),
              VppDenoiseLevel(-1),
              VppDenoiseMode(0),
              DetailLevel(-1),
              FRCAlgorithm(0),
              fieldProcessingMode(FC_NONE),
              ScalingMode(0),
              nAsyncDepth(0),
              eMode(Native),
              eModeExt(Native),
              FrameNumberPreference(0),
              MaxFrameNumber(MFX_INFINITE),
              ExactNframe(0),
              numSurf4Comp(0),
              numTiles4Comp(0),
              nSlices(0),
              nMaxSliceSize(0),
              WinBRCMaxAvgKbps(0),
              WinBRCSize(0),
              BufferSizeInKB(0),
              GopPicSize(0),
              GopRefDist(0),
              NumRefFrame(0),
              nNumRefActiveP(0),
              nBRefType(0),
              RepartitionCheckMode(0),
              GPB(0),
              nTransformSkip(0),
              CodecLevel(0),
              CodecProfile(0),
              MaxKbps(0),
              InitialDelayInKB(0),
              GopOptFlag(0),
              AdaptiveI(0),
              AdaptiveB(0),
              AdaptiveCQM(0),
              WeightedPred(0),
              WeightedBiPred(0),
              ExtBrcAdaptiveLTR(0),
              bExtMBQP(false),
              bIsMVC(false),
              numViews(0),
              nRotationAngle(0),
              strVPPPluginDLLPath(),
              decoderPluginParams(),
              encoderPluginParams(),
              nTimeout(0),
              nFPS(0),
              statisticsWindowSize(0),
              statisticsLogFile(nullptr),
              bLABRC(false),
              nLADepth(0),
              bEnableExtLA(false),
              bEnableBPyramid(false),
              nRateControlMethod(0),
              nQPI(0),
              nQPP(0),
              nQPB(0),
              bDisableQPOffset(false),
              bSetQPOffset(false),
              QPOffset{ 0 },
              nMinQPI(0),
              nMaxQPI(0),
              nMinQPP(0),
              nMaxQPP(0),
              nMinQPB(0),
              nMaxQPB(0),
              nAvcTemp(0),
              nBaseLayerPID(0),
              nAvcTemporalLayers{ 0 },
#if defined(_WIN32) || defined(_WIN64)
              bTemporalLayers(false),
              temporalLayers{ 0 },
#endif
              nSPSId(0),
              nPPSId(0),
              nPicTimingSEI(0),
              nNalHrdConformance(0),
              nVuiNalHrdParameters(0),
              nTransferCharacteristics(0),
              bOpenCL(false),
              reserved{ 0 },
              nVppCompDstX(0),
              nVppCompDstY(0),
              nVppCompDstW(0),
              nVppCompDstH(0),
              nVppCompSrcW(0),
              nVppCompSrcH(0),
              nVppCompTileId(0),
              DecoderFourCC(0),
              EncoderFourCC(0),
              pVppCompDstRects(nullptr),
              bForceSysMem(false),
              DecOutPattern(0),
              VppOutPattern(0),
              nGpuCopyMode(0),
              nRenderColorForamt(0),
              monitorType(0),
              shouldUseGreedyFormula(false),
              enableQSVFF(false),
              bSingleTexture(false),
              nExtBRC(EXTBRC_DEFAULT),
              nAdaptiveMaxFrameSize(0),
              LowDelayBRC(0),
              IntRefType(0),
              IntRefCycleSize(0),
              IntRefQPDelta(0),
              IntRefCycleDist(0),
              nMaxFrameSize(0),
              BitrateLimit(0),
              numMFEFrames(0),
              MFMode(0),
              mfeTimeout(0),
              SEIMetaMDCV(),
              SEIMetaCLLI(),
              bEnableMDCV(false),
              bEnableCLLI(false),
              SignalInfoIn(),
              SignalInfoOut(),
              TargetBitDepthLuma(0),
              TargetBitDepthChroma(0),
#if defined(LIBVA_WAYLAND_SUPPORT)
              nRenderWinX(0),
              nRenderWinY(0),
              bPerfMode(false),
#endif
#if defined(LIBVA_SUPPORT)
              libvaBackend(0),
#endif // defined(MFX_LIBVA_SUPPORT)
              m_hwdev(nullptr),
              PresetMode(PRESET_DEFAULT),
              shouldPrintPresets(false),
              rawInput(false),
              str3DLutFile(),
              bEnable3DLut(false),
              nMemoryModel(0),
              AllocPolicy(MFX_ALLOCATION_UNLIMITED),
              useAllocHints(false),
              preallocate(0),
              forceSyncAllSession(MFX_CODINGOPTION_UNKNOWN),
              nIVFHeader(0),
              IsSourceMSB(false),
              nSyncOpTimeout(MSDK_WAIT_INTERVAL),
              TCBRCFileMode(false),
              DumpLogFileName(),
              m_ROIData(),
              bDecoderPostProcessing(false),
              bROIasQPMAP(false),
#ifdef ENABLE_MCTF
              mctfParam(),
#endif
#ifdef ONEVPL_EXPERIMENTAL
              PercEncPrefilter(false),
              TuneEncodeQuality(0),
#endif
              ScenarioInfo(0),
              ContentInfo(0),
              verSessionInit(API_2X) {
#ifdef ENABLE_MCTF
        mctfParam.mode                  = VPP_FILTER_DISABLED;
        mctfParam.params.FilterStrength = 0;
        mctfParam.rtParams.Reset();
#endif
    }
} sInputParams;
} // namespace TranscodingSample
#endif // __SMT_CLI_PARAMS_H__
