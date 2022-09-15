/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __SAMPLE_MULTI_TRANSCODE_H__
#define __SAMPLE_MULTI_TRANSCODE_H__

#include "vpl/mfxdefs.h"
#if defined(_WIN32) || defined(_WIN64)
    #include "mfxadapter.h"
#endif

#include "pipeline_transcode.h"
#include "sample_utils.h"
#include "transcode_utils.h"
#include "vpl_implementation_loader.h"

#include <memory>
#include "d3d11_allocator.h"
#include "d3d11_device.h"
#include "d3d_allocator.h"
#include "d3d_device.h"
#include "general_allocator.h"
#include "hw_device.h"

#ifdef LIBVA_SUPPORT
    #include "vaapi_allocator.h"
    #include "vaapi_device.h"
    #include "vaapi_utils.h"

#endif

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

namespace TranscodingSample {
class Launcher {
public:
    Launcher();
    virtual ~Launcher();

    virtual mfxStatus Init(int argc, msdk_char* argv[]);
    virtual void Run();
    virtual mfxStatus ProcessResult();

protected:
#if (defined(_WIN32) || defined(_WIN64))
    mfxStatus QueryAdapters();
    void ForceImplForSession(mfxU32 idxSession);
    mfxStatus CheckAndFixAdapterDependency_1X(mfxU32 idxSession,
                                              CTranscodingPipeline* pParentPipeline);
#endif
    mfxStatus CheckAndFixAdapterDependency(mfxU32 idxSession,
                                           CTranscodingPipeline* pParentPipeline);
    virtual mfxStatus VerifyCrossSessionsOptions();
    virtual mfxStatus CreateSafetyBuffers();
    CascadeScalerConfig& CreateCascadeScalerConfig();
    virtual void DoTranscoding();
    virtual void DoRobustTranscoding();

    virtual void Close();

    // command line parser
    CmdProcessor m_parser;
    // threads contexts to process playlist
    std::vector<std::unique_ptr<ThreadTranscodeContext>> m_pThreadContextArray;
    // allocator for each session
    std::vector<std::unique_ptr<GeneralAllocator>> m_pAllocArray;
    // input parameters for each session
    std::vector<sInputParams> m_InputParamsArray;
    // safety buffers
    // needed for heterogeneous pipeline
    std::vector<std::unique_ptr<SafetySurfaceBuffer>> m_pBufferArray;

    std::vector<std::unique_ptr<FileBitstreamProcessor>> m_pExtBSProcArray;
    std::vector<std::shared_ptr<mfxAllocatorParams>> m_pAllocParams;
    std::vector<std::unique_ptr<CHWDevice>> m_hwdevs;
    msdk_tick m_StartTime;
    // need to work with HW pipeline
    mfxHandleType m_eDevType;
    mfxAccelerationMode m_accelerationMode;
    std::unique_ptr<VPLImplementationLoader> m_pLoader;

    std::vector<sVppCompDstRect> m_VppDstRects;

    CascadeScalerConfig m_CSConfig;
    SMTTracer m_Tracer;
    std::shared_ptr<CSmplBitstreamWriter> m_GlobalBitstreamWriter{};

private:
    DISALLOW_COPY_AND_ASSIGN(Launcher);

#if (defined(_WIN32) || defined(_WIN64))
    std::vector<mfxAdapterInfo> m_DisplaysData;
    mfxAdaptersInfo m_Adapters;
#endif
};
} // namespace TranscodingSample

#endif
