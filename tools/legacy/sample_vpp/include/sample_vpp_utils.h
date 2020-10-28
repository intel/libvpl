/*###########################################################################
  # Copyright (C) 2005-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#ifndef __SAMPLE_VPP_UTILS_H
    #define __SAMPLE_VPP_UTILS_H

/* ************************************************************************* */

    #if defined(_WIN32) || defined(_WIN64)
        #define D3D_SURFACES_SUPPORT
    #endif // #if defined(_WIN32) || defined(_WIN64)

    #ifdef D3D_SURFACES_SUPPORT
        #pragma warning(disable : 4201)
        #include <d3d9.h>
        #include <dxva2api.h>
        #include <math.h>
        #include <psapi.h>
        #include <windows.h>
    #endif
    #ifndef DISABLE_NON_VPL
        #include "hw_device.h"
    #endif

    #include "sample_defs.h"

    #ifdef MFX_D3D11_SUPPORT
        #include <d3d11.h>
    #endif

    #ifdef LIBVA_SUPPORT
        #include "vaapi_allocator.h"
        #include "vaapi_utils.h"
    #endif

    #include <stdio.h>
    #include <map>
    #include <memory>

    #include "vm/strings_defs.h"

    #include "mfxvideo++.h"
    #include "vpl/mfxmvc.h"
    #include "vpl/mfxvideo.h"

    #ifndef DISABLE_NON_VPL
        #include "mfxplugin.h"
    #endif

    #include "base_allocator.h"
    #include "sample_vpp_config.h"
    #include "sample_vpp_roi.h"

    // we introduce new macros without error message (returned status only)
    // it allows to remove final error message due to EOF
    #define IOSTREAM_MSDK_CHECK_NOT_EQUAL(P, X, ERR) \
        {                                            \
            if ((X) != (P)) {                        \
                return ERR;                          \
            }                                        \
        }

    #define IMPL_VIA_MASK 0xF00
/* ************************************************************************* */

    #ifndef MFX_VERSION
        #error MFX_VERSION not defined
    #endif

enum {
    VPP_IN  = 0,
    VPP_OUT = 1,
};

enum {
    ALLOC_IMPL_VIA_SYS   = 0,
    ALLOC_IMPL_VIA_D3D9  = 1,
    ALLOC_IMPL_VIA_D3D11 = 2,
    ALLOC_IMPL_VIA_VAAPI = 4
};
    #ifdef ENABLE_MCTF
const mfxU16 MAX_NUM_OF_ATTACHED_BUFFERS_FOR_IN_SUFACE = 2;
const mfxU16 MCTF_MID_FILTER_STRENGTH                  = 10;
const mfxF64 MCTF_AUTO_BPP                             = 0.0;
const mfxF64 MCTF_LOSSLESS_BPP                         = 0.0;
const mfxU16 MCTF_TEMPORAL_2REF_MODE                   = 3;
    #endif
//#define BACKWARD_COMPATIBILITY

typedef struct _ownFrameInfo {
    mfxU16 Shift;
    mfxU16 BitDepthLuma;
    mfxU16 BitDepthChroma;
    mfxU16 nWidth;
    mfxU16 nHeight;
    // ROI
    mfxU16 CropX;
    mfxU16 CropY;
    mfxU16 CropW;
    mfxU16 CropH;
    mfxU32 FourCC;
    mfxU16 PicStruct;
    mfxF64 dFrameRate;

} sOwnFrameInfo;

typedef struct _filtersParam {
    sOwnFrameInfo* pOwnFrameInfo;
    sDIParam* pDIParam;
    sProcAmpParam* pProcAmpParam;
    sDetailParam* pDetailParam;
    sDenoiseParam* pDenoiseParam;
    #ifdef ENABLE_MCTF
    sMCTFParam* pMctfParam;
    #endif
    sVideoAnalysisParam* pVAParam;
    sIDetectParam* pIDetectParam;
    sFrameRateConversionParam* pFRCParam;
    sMultiViewParam* pMultiViewParam;
    sGamutMappingParam* pGamutParam;
    sTccParam* pClrSaturationParam;
    sAceParam* pContrastParam;
    sSteParam* pSkinParam;
    sIStabParam* pImgStabParam;
    sSVCParam* pSVCParam;
    sVideoSignalInfoParam* pVideoSignalInfo;
    sMirroringParam* pMirroringParam;
    sColorFillParam* pColorfillParam;
} sFiltersParam;

struct sInputParams {
    /* smart filters defined by mismatch btw src & dst */
    std::vector<sOwnFrameInfo> frameInfoIn; // [0] - in, [1] - out
    std::vector<sOwnFrameInfo> frameInfoOut; // [0] - in, [1] - out

    /* Video Enhancement Algorithms */
    std::vector<sDIParam> deinterlaceParam;
    std::vector<sDenoiseParam> denoiseParam;
    #ifdef ENABLE_MCTF
    std::vector<sMCTFParam> mctfParam;
    #endif
    std::vector<sDetailParam> detailParam;
    std::vector<sProcAmpParam> procampParam;
    std::vector<sFrameRateConversionParam> frcParam;

    std::vector<sVideoSignalInfoParam> videoSignalInfoParam;
    std::vector<sMirroringParam> mirroringParam;

    // MSDK 3.0
    std::vector<sGamutMappingParam> gamutParam;
    std::vector<sMultiViewParam> multiViewParam;

    // MSDK API 1.5
    std::vector<sTccParam> tccParam;
    std::vector<sAceParam> aceParam;
    std::vector<sSteParam> steParam;
    std::vector<sIStabParam> istabParam;

    std::vector<sColorFillParam> colorfillParam;
    // flag describes type of memory
    // true  - frames in video memory (d3d surfaces),
    // false - in system memory
    //bool   bd3dAlloc;
    mfxU16 IOPattern;
    mfxIMPL ImpLib;
    mfxU16 asyncNum;
    mfxU32 vaType;

    #if (MFX_VERSION >= 2000)
    bool bUseVPLLib; // true to use VPL SW library
    #endif

    std::vector<mfxU16> rotate;

    bool bScaling;
    mfxU16 scalingMode;

    bool bChromaSiting;
    mfxU16 uChromaSiting;

    bool bInitEx;
    mfxU16 GPUCopyValue;

    bool bPartialAccel;

    bool bPerf;
    mfxU32 numFrames;
    mfxU16 numRepeat;
    bool isOutput;
    bool ptsCheck;
    bool ptsJump;
    bool ptsAdvanced;
    mfxF64 ptsFR;

    /* roi checking parameters */
    sROICheckParam roiCheckParam;

    /*  plug-in GUID */
    msdk_char strPlgGuid[MSDK_MAX_FILENAME_LEN];
    bool need_plugin;

    #ifdef ENABLE_VPP_RUNTIME_HSBC
    /* run-time ProcAmp parameters */
    typedef struct {
        bool isEnabled;
        mfxU32 interval;
        mfxF64 value1;
        mfxF64 value2;
    } sRtProcAmpParam;

    sRtProcAmpParam rtHue;
    sRtProcAmpParam rtSaturation;
    sRtProcAmpParam rtBrightness;
    sRtProcAmpParam rtContrast;
    #endif

    /* ********************** */
    /* input\output streams   */
    /* ********************** */
    msdk_char strSrcFile[MSDK_MAX_FILENAME_LEN];
    std::vector<msdk_tstring> strDstFiles;

    msdk_char strPerfFile[MSDK_MAX_FILENAME_LEN];
    mfxU32 forcedOutputFourcc;

    /* Use extended API (RunFrameVPPAsyncEx) */
    bool use_extapi;

    /* MFXVideoVPP_Reset */
    std::vector<mfxU32> resetFrmNums;

    sOwnFrameInfo inFrameInfo[MAX_INPUT_STREAMS];
    mfxU16 numStreams;

    sCompositionParam compositionParam;

    mfxU32 fccSource;
    sInputParams() {
        IOPattern = 0;
        ImpLib    = 0;
    #if (MFX_VERSION >= 2000)
        bUseVPLLib = false;
    #endif
        asyncNum           = 0;
        vaType             = 0;
        bScaling           = false;
        scalingMode        = 0;
        GPUCopyValue       = 0;
        bPartialAccel      = 0;
        numFrames          = 0;
        numRepeat          = 0;
        isOutput           = false;
        ptsCheck           = false;
        ptsJump            = false;
        ptsAdvanced        = false;
        ptsFR              = 0;
        forcedOutputFourcc = 0;
        numStreams         = 0;

        MSDK_ZERO_MEMORY(strPlgGuid);
        MSDK_ZERO_MEMORY(strSrcFile);
        MSDK_ZERO_MEMORY(strPerfFile);
        MSDK_ZERO_MEMORY(inFrameInfo);
        MSDK_ZERO_MEMORY(compositionParam);
        MSDK_ZERO_MEMORY(roiCheckParam);

        bInitEx     = false;
        bPerf       = false;
        need_plugin = false;
        use_extapi  = false;
        MSDK_ZERO_MEMORY(strPlgGuid);
        MSDK_ZERO_MEMORY(strSrcFile);
        MSDK_ZERO_MEMORY(strPerfFile);
        strDstFiles.clear();
        uChromaSiting = 0;
        bChromaSiting = false;
        fccSource     = 0;

    #ifdef ENABLE_VPP_RUNTIME_HSBC
        rtHue        = {};
        rtSaturation = {};
        rtBrightness = {};
        rtContrast   = {};
    #endif
    }
};

struct sFrameProcessor {
    MFXVideoSession mfxSession;
    MFXVideoVPP* pmfxVPP;

    #ifndef DISABLE_NON_VPL
    mfxPluginUID mfxGuid;
    bool plugin;
    #endif

    sFrameProcessor(void) {
        pmfxVPP = NULL;
    #ifndef DISABLE_NON_VPL
        plugin = false;
    #endif
        return;
    };
};

struct sMemoryAllocator {
    MFXFrameAllocator* pMfxAllocator;
    mfxAllocatorParams* pAllocatorParams;
    bool bUsedAsExternalAllocator;

    // storage for ext-buffers pointers assosiated with input surfaces;
    std::vector<mfxExtBuffer*> pExtBuffersStorageSurfaceIn[MAX_INPUT_STREAMS];
    mfxFrameSurface1* pSurfacesIn[MAX_INPUT_STREAMS]; // SINGLE_IN/OUT/MULTIPLE_INs
    mfxFrameSurface1* pSurfacesOut;
    mfxFrameAllocResponse responseIn[MAX_INPUT_STREAMS]; // SINGLE_IN/OUT/MULTIPLE_INs
    mfxFrameAllocResponse responseOut;

    mfxFrameSurface1* pSvcSurfaces[8]; //output surfaces per layer
    mfxFrameAllocResponse svcResponse[8]; //per layer
    #ifndef DISABLE_NON_VPL
    CHWDevice* pDevice;
    #endif

    /*#ifdef D3D_SURFACES_SUPPORT
    IDirect3DDeviceManager9* pd3dDeviceManager;
    #endif

    #ifdef MFX_D3D11_SUPPORT
    ID3D11Device *pD3D11Device;
    ID3D11DeviceContext *pD3D11DeviceContext;
    #endif

    #ifdef LIBVA_SUPPORT
    std::unique_ptr<CLibVA> libvaKeeper;
    #endif*/
};

class PTSMaker;

class CRawVideoReader {
public:
    CRawVideoReader();
    ~CRawVideoReader();

    void Close();

    mfxStatus Init(const msdk_char* strFileName, PTSMaker* pPTSMaker, mfxU32 fcc);

    mfxStatus PreAllocateFrameChunk(mfxVideoParam* pVideoParam,
                                    sInputParams* pParams,
                                    MFXFrameAllocator* pAllocator);

    mfxStatus GetNextInputFrame(sMemoryAllocator* pAllocator,
                                mfxFrameInfo* pInfo,
                                mfxFrameSurface1** pSurface,
                                mfxU16 streamIndex);

    mfxStatus LoadNextFrame(mfxFrameData* pData, mfxFrameInfo* pInfo);

private:
    mfxStatus GetPreAllocFrame(mfxFrameSurface1** pSurface);

    FILE* m_fSrc;
    std::list<mfxFrameSurface1>::iterator m_it;
    std::list<mfxFrameSurface1> m_SurfacesList;
    bool m_isPerfMode;
    mfxU16 m_Repeat;

    PTSMaker* m_pPTSMaker;
    mfxU32 m_initFcc;
};

class CRawVideoWriter {
public:
    CRawVideoWriter();
    ~CRawVideoWriter();

    void Close();

    mfxStatus Init(const msdk_char* strFileName,
                   PTSMaker* pPTSMaker,
                   mfxU32 forcedOutputFourcc = 0);

    mfxStatus PutNextFrame(sMemoryAllocator* pAllocator,
                           mfxFrameInfo* pInfo,
                           mfxFrameSurface1* pSurface);

private:
    mfxStatus WriteFrame(mfxFrameData* pData, mfxFrameInfo* pInfo);

    FILE* m_fDst;
    PTSMaker* m_pPTSMaker;
    mfxU32 m_forcedOutputFourcc;
};

class GeneralWriter // : public CRawVideoWriter
{
public:
    GeneralWriter();
    ~GeneralWriter();

    void Close();

    mfxStatus Init(const msdk_char* strFileName,
                   PTSMaker* pPTSMaker,
                   sSVCLayerDescr* pDesc     = NULL,
                   mfxU32 forcedOutputFourcc = 0);

    mfxStatus PutNextFrame(sMemoryAllocator* pAllocator,
                           mfxFrameInfo* pInfo,
                           mfxFrameSurface1* pSurface);

private:
    std::unique_ptr<CRawVideoWriter> m_ofile[8];

    bool m_svcMode;
};

// need for support async
class SurfaceVPPStore {
public:
    struct SurfVPPExt {
        SurfVPPExt(mfxFrameSurface1* pSurf = 0, mfxExtVppAuxData* pExt = 0)
                : pSurface(pSurf),
                  pExtVpp(pExt){};
        mfxFrameSurface1* pSurface;
        mfxExtVppAuxData* pExtVpp;
    };
    SurfaceVPPStore(){};

    typedef std::pair<mfxSyncPoint, SurfVPPExt> SyncPair;
    std::list<SyncPair> m_SyncPoints;
};

struct sAppResources {
    CRawVideoReader* pSrcFileReaders[MAX_INPUT_STREAMS];
    mfxU16 numSrcFiles;

    //CRawVideoWriter*    pDstFileWriter;
    GeneralWriter* pDstFileWriters;
    mfxU32 dstFileWritersN;

    sFrameProcessor* pProcessor;
    mfxVideoParam* pVppParams;
    sMemoryAllocator* pAllocator;
    sInputParams* pParams;
    SurfaceVPPStore* pSurfStore;

    /* VPP extension */
    mfxExtVppAuxData* pExtVPPAuxData;
    mfxExtVPPDoUse extDoUse;
    mfxU32 tabDoUseAlg[ENH_FILTERS_COUNT];
    mfxExtBuffer* pExtBuf[1 + ENH_FILTERS_COUNT];

    /* config video enhancement algorithms */
    mfxExtVPPProcAmp procampConfig;
    mfxExtVPPDetail detailConfig;
    mfxExtVPPDenoise denoiseConfig;
    #ifdef ENABLE_MCTF
    mfxExtVppMctf mctfConfig;
    #endif
    mfxExtVPPRotation rotationConfig;
    mfxExtVPPScaling scalingConfig;
    #if MFX_VERSION >= 1025
    mfxExtColorConversion chromaSitingConfig;
    #endif
    mfxExtVPPFrameRateConversion frcConfig;
    mfxExtVPPDeinterlacing deinterlaceConfig;
    mfxExtVPPVideoSignalInfo videoSignalInfoConfig;
    mfxExtVPPMirroring mirroringConfig;
    mfxExtVPPComposite compositeConfig;

    // MSDK 3.0
    //  mfxExtVPPGamutMapping gamutConfig;
    mfxExtMVCSeqDesc multiViewConfig;

    ////MSDK API 1.5
    //mfxExtVPPSkinTone              steConfig;
    //mfxExtVPPColorSaturationLevel  tccConfig;
    mfxExtVPPImageStab istabConfig;

    mfxExtVPPColorFill colorfillConfig;
};

/* ******************************************************************* */
/*                        service functions                            */
/* ******************************************************************* */

void PrintInfo(sInputParams* pParams, mfxVideoParam* pMfxParams, MFXVideoSession* pMfxSession);

void PrintDllInfo();

mfxStatus InitParamsVPP(mfxVideoParam* pMFXParams, sInputParams* pInParams, mfxU32 paramID);

mfxStatus InitResources(sAppResources* pResources, mfxVideoParam* pParams, sInputParams* pInParams);

void WipeResources(sAppResources* pResources);
void WipeParams(sInputParams* pParams);

mfxStatus UpdateSurfacePool(mfxFrameInfo SurfacesInfo,
                            mfxU16 nPoolSize,
                            mfxFrameSurface1* pSurface);
mfxStatus GetFreeSurface(mfxFrameSurface1* pSurfacesPool,
                         mfxU16 nPoolSize,
                         mfxFrameSurface1** ppSurface);

const msdk_char* IOpattern2Str(mfxU32 IOpattern);

mfxStatus vppParseInputString(msdk_char* strInput[],
                              mfxU8 nArgNum,
                              sInputParams* pParams,
                              sFiltersParam* pDefaultFiltersParam);

bool CheckInputParams(msdk_char* strInput[], sInputParams* pParams);

void vppPrintHelp(const msdk_char* strAppName, const msdk_char* strErrorMessage);

mfxStatus ConfigVideoEnhancementFilters(sInputParams* pParams,
                                        sAppResources* pResources,
                                        mfxU32 paramID);

const msdk_char* PicStruct2Str(mfxU16 PicStruct);

mfxStatus ParseCompositionParfile(const msdk_char* parFileName, sInputParams* pParams);
#endif /* __SAMPLE_VPP_UTILS_H */
/* EOF */
