/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "mfx_samples_config.h"
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

#include "sample_multi_transcode.h"

#if defined(LIBVA_WAYLAND_SUPPORT)
    #include "class_wayland.h"
#endif

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

#include <future>
#include <iomanip>
#include <memory>

using namespace std;
using namespace TranscodingSample;

Launcher::Launcher()
        : m_parser(),
          m_pThreadContextArray(),
          m_pAllocArray(),
          m_InputParamsArray(),
          m_pBufferArray(),
          m_pExtBSProcArray(),
          m_pAllocParams(),
          m_hwdevs(),
          m_StartTime(0),
          m_eDevType(static_cast<mfxHandleType>(0)),
          m_accelerationMode(MFX_ACCEL_MODE_NA),
          m_pLoader(),
          m_VppDstRects(),
          m_CSConfig(),
          m_Tracer() {} // Launcher::Launcher()

Launcher::~Launcher() {
    Close();
} // Launcher::~Launcher()

CTranscodingPipeline* CreatePipeline() {
    MOD_SMT_CREATE_PIPELINE;

    return new CTranscodingPipeline;
}

mfxStatus Launcher::Init(int argc, msdk_char* argv[]) {
    mfxStatus sts;
    mfxU32 i                     = 0;
    SafetySurfaceBuffer* pBuffer = NULL;
    mfxU32 BufCounter            = 0;
    mfxHDL hdl                   = NULL;
    std::vector<mfxHDL> hdls;
    sInputParams InputParams;
    bool bNeedToCreateDevice = true;
    bool lowLatencyMode      = true;

    //parent transcode pipeline
    CTranscodingPipeline* pParentPipeline = NULL;
    // source transcode pipeline use instead parent in heterogeneous pipeline
    CTranscodingPipeline* pSinkPipeline = NULL;

    // parse input par file
    sts = m_parser.ParseCmdLine(argc, argv);
    MSDK_CHECK_PARSE_RESULT(sts, MFX_ERR_NONE, sts);
    if (sts == MFX_WRN_OUT_OF_RANGE) {
        // There's no error in parameters parsing, but we should not continue further. For instance, in case of -? option
        return sts;
    }

    // get parameters for each session from parser
    mfxU32 id = DecoderTargetID;
    while (m_parser.GetNextSessionParams(InputParams)) {
        InputParams.TargetID = id++;
        m_InputParamsArray.push_back(InputParams);
    }

    m_CSConfig.Tracer = &m_Tracer;

    // check correctness of input parameters
    sts = VerifyCrossSessionsOptions();
    MSDK_CHECK_STATUS(sts, "VerifyCrossSessionsOptions failed");

    m_pLoader.reset(new VPLImplementationLoader);

    // new memory models are suppotred in lib with version >2.0 and not supported SetHandle, so lowLatencyMode need to turn off
    if (m_InputParamsArray[0].nMemoryModel == VISIBLE_INT_ALLOC ||
        m_InputParamsArray[0].nMemoryModel == HIDDEN_INT_ALLOC) {
        m_pLoader->ConfigureVersion({ { 0, 2 } });
        msdk_printf(MSDK_STRING("warning: internal memory allocation requires 2.x API\n"));

        lowLatencyMode = false;
    }

    if (m_InputParamsArray[0].dGfxIdx >= 0) {
        m_pLoader->SetDiscreteAdapterIndex(m_InputParamsArray[0].dGfxIdx);
    }
    else
        m_pLoader->SetAdapterType(m_InputParamsArray[0].adapterType);

    if (m_InputParamsArray[0].adapterNum >= 0)
        m_pLoader->SetAdapterNum(m_InputParamsArray[0].adapterNum);

    sts = m_pLoader->ConfigureAndEnumImplementations(m_InputParamsArray[0].libType,
                                                     m_accelerationMode,
                                                     lowLatencyMode);
    MSDK_CHECK_STATUS(sts, "EnumImplementations failed");

    for (i = 0; i < m_InputParamsArray.size(); i++) {
        /* In the case of joined sessions, need to create device only for a zero session
         * In the case of a shared buffer, need to create device only for decode */
        if ((m_InputParamsArray[i].bIsJoin && i != 0) || m_InputParamsArray[i].eMode == Source)
            bNeedToCreateDevice = false;

#if defined(_WIN32) || defined(_WIN64)
        if (m_eDevType == MFX_HANDLE_D3D9_DEVICE_MANAGER) {
            if (bNeedToCreateDevice) {
                auto pAllocParam = std::make_shared<D3DAllocatorParams>();
                D3DAllocatorParams* pD3DParams =
                    dynamic_cast<D3DAllocatorParams*>(pAllocParam.get());
                auto hwdev = std::make_unique<CD3D9Device>();

                /* The last param set in vector always describe VPP+ENCODE or Only VPP
                 * So, if we want to do rendering we need to do pass HWDev to CTranscodingPipeline */
                if (m_InputParamsArray[m_InputParamsArray.size() - 1].eModeExt == VppCompOnly) {
                    /* Rendering case */
                    sts = hwdev->Init(NULL, 1, MSDKAdapter::GetNumber(m_pLoader.get()));
                    m_InputParamsArray[m_InputParamsArray.size() - 1].m_hwdev = hwdev.get();
                }
                else /* NO RENDERING */
                {
                    sts = hwdev->Init(NULL, 0, MSDKAdapter::GetNumber(m_pLoader.get()));
                }
                MSDK_CHECK_STATUS(sts, "hwdev->Init failed");
                sts = hwdev->GetHandle(MFX_HANDLE_D3D9_DEVICE_MANAGER, (mfxHDL*)&hdl);
                MSDK_CHECK_STATUS(sts, "hwdev->GetHandle failed");
                // set Device Manager to external dx9 allocator
                pD3DParams->pManager = (IDirect3DDeviceManager9*)hdl;

                m_pAllocParams.push_back(pAllocParam);
                m_hwdevs.push_back(std::move(hwdev));
                hdls.push_back(hdl);
            }
            else {
                if (!m_pAllocParams.empty() && !hdls.empty()) {
                    m_pAllocParams.push_back(m_pAllocParams.back());
                    hdls.push_back(hdls.back());
                }
                else {
                    msdk_printf(MSDK_STRING("error: failed to initialize alloc parameters\n"));
                    return MFX_ERR_MEMORY_ALLOC;
                }
            }
        }
    #if MFX_D3D11_SUPPORT
        else if (m_eDevType == MFX_HANDLE_D3D11_DEVICE) {
            if (bNeedToCreateDevice) {
                auto pAllocParam = std::make_shared<D3D11AllocatorParams>();
                D3D11AllocatorParams* pD3D11Params =
                    dynamic_cast<D3D11AllocatorParams*>(pAllocParam.get());
                auto hwdev = std::make_unique<CD3D11Device>();

                /* The last param set in vector always describe VPP+ENCODE or Only VPP
                 * So, if we want to do rendering we need to do pass HWDev to CTranscodingPipeline */
                if (m_InputParamsArray[m_InputParamsArray.size() - 1].eModeExt == VppCompOnly) {
                    /* Rendering case */
                    sts = hwdev->Init(NULL, 1, MSDKAdapter::GetNumber(m_pLoader.get()));
                    m_InputParamsArray[m_InputParamsArray.size() - 1].m_hwdev = hwdev.get();
                }
                else /* NO RENDERING */
                {
                    sts = hwdev->Init(NULL, 0, MSDKAdapter::GetNumber(m_pLoader.get()));
                }
                MSDK_CHECK_STATUS(sts, "hwdev->Init failed");
                sts = hwdev->GetHandle(MFX_HANDLE_D3D11_DEVICE, (mfxHDL*)&hdl);
                MSDK_CHECK_STATUS(sts, "hwdev->GetHandle failed");

                pD3D11Params->bUseSingleTexture = m_InputParamsArray[i].bSingleTexture;
                // set Device to external dx11 allocator
                pD3D11Params->pDevice = (ID3D11Device*)hdl;

                m_pAllocParams.push_back(pAllocParam);
                m_hwdevs.push_back(std::move(hwdev));
                hdls.push_back(hdl);
            }
            else {
                if (!m_pAllocParams.empty() && !hdls.empty()) {
                    m_pAllocParams.push_back(m_pAllocParams.back());
                    hdls.push_back(hdls.back());
                }
                else {
                    msdk_printf(MSDK_STRING("error: failed to initialize alloc parameters\n"));
                    return MFX_ERR_MEMORY_ALLOC;
                }
            }
        }
    #endif
#elif defined(LIBVA_X11_SUPPORT) || defined(LIBVA_DRM_SUPPORT) || defined(ANDROID)
        if (m_eDevType == MFX_HANDLE_VA_DISPLAY) {
            if (bNeedToCreateDevice) {
                mfxI32 libvaBackend = 0;
                mfxAllocatorParams* pAllocParam(new vaapiAllocatorParams);
                std::unique_ptr<CHWDevice> hwdev;

                vaapiAllocatorParams* pVAAPIParams =
                    dynamic_cast<vaapiAllocatorParams*>(pAllocParam);
                /* The last param set in vector always describe VPP+ENCODE or Only VPP
                 * So, if we want to do rendering we need to do pass HWDev to CTranscodingPipeline */
                if (m_InputParamsArray[m_InputParamsArray.size() - 1].eModeExt == VppCompOnly) {
                    sInputParams& params = m_InputParamsArray[m_InputParamsArray.size() - 1];
                    libvaBackend         = params.libvaBackend;

                    /* Rendering case */
                    hwdev.reset(CreateVAAPIDevice(InputParams.strDevicePath, params.libvaBackend));
                    if (!hwdev.get()) {
                        msdk_printf(MSDK_STRING("error: failed to initialize VAAPI device\n"));
                        return MFX_ERR_DEVICE_FAILED;
                    }
                    sts = hwdev->Init(&params.monitorType,
                                      1,
                                      MSDKAdapter::GetNumber(m_pLoader.get()));
    #if defined(LIBVA_X11_SUPPORT) || defined(LIBVA_DRM_SUPPORT)
                    if (params.libvaBackend == MFX_LIBVA_DRM_MODESET) {
                        CVAAPIDeviceDRM* drmdev     = dynamic_cast<CVAAPIDeviceDRM*>(hwdev.get());
                        pVAAPIParams->m_export_mode = vaapiAllocatorParams::CUSTOM_FLINK;
                        pVAAPIParams->m_exporter =
                            dynamic_cast<vaapiAllocatorParams::Exporter*>(drmdev->getRenderer());
                    }
                    else if (params.libvaBackend == MFX_LIBVA_X11) {
                        pVAAPIParams->m_export_mode = vaapiAllocatorParams::PRIME;
                    }
    #endif
    #if defined(LIBVA_WAYLAND_SUPPORT)
                    else if (params.libvaBackend == MFX_LIBVA_WAYLAND) {
                        VADisplay va_dpy = NULL;
                        sts = hwdev->GetHandle(MFX_HANDLE_VA_DISPLAY, (mfxHDL*)&va_dpy);
                        MSDK_CHECK_STATUS(sts, "hwdev->GetHandle failed");
                        hdl = pVAAPIParams->m_dpy = (VADisplay)va_dpy;

                        CVAAPIDeviceWayland* w_dev =
                            dynamic_cast<CVAAPIDeviceWayland*>(hwdev.get());
                        if (!w_dev) {
                            MSDK_CHECK_STATUS(MFX_ERR_DEVICE_FAILED,
                                              "Failed to reach Wayland VAAPI device");
                        }
                        Wayland* wld = w_dev->GetWaylandHandle();
                        if (!wld) {
                            MSDK_CHECK_STATUS(MFX_ERR_DEVICE_FAILED,
                                              "Failed to reach Wayland VAAPI device");
                        }

                        wld->SetRenderWinPos(params.nRenderWinX, params.nRenderWinY);
                        wld->SetPerfMode(params.bPerfMode);

                        pVAAPIParams->m_export_mode = vaapiAllocatorParams::PRIME;
                    }
    #endif // LIBVA_WAYLAND_SUPPORT
                    params.m_hwdev = hwdev.get();
                }
                else /* NO RENDERING*/
                {
                    hwdev.reset(CreateVAAPIDevice(InputParams.strDevicePath));

                    if (!hwdev.get()) {
                        msdk_printf(MSDK_STRING("error: failed to initialize VAAPI device\n"));
                        return MFX_ERR_DEVICE_FAILED;
                    }
                    sts = hwdev->Init(NULL, 0, MSDKAdapter::GetNumber(m_pLoader.get()));
                }
                if (libvaBackend != MFX_LIBVA_WAYLAND) {
                    MSDK_CHECK_STATUS(sts, "hwdev->Init failed");
                    sts = hwdev->GetHandle(MFX_HANDLE_VA_DISPLAY, (mfxHDL*)&hdl);
                    MSDK_CHECK_STATUS(sts, "hwdev->GetHandle failed");
                    // set Device to external vaapi allocator
                    pVAAPIParams->m_dpy = (VADisplay)hdl;
                }

                m_pAllocParams.push_back(std::shared_ptr<mfxAllocatorParams>(pAllocParam));
                m_hwdevs.push_back(std::move(hwdev));
                hdls.push_back(hdl);
            }
            else {
                if (!m_pAllocParams.empty() && !hdls.empty()) {
                    m_pAllocParams.push_back(m_pAllocParams.back());
                    hdls.push_back(hdls.back());
                }
                else {
                    msdk_printf(MSDK_STRING("error: failed to initialize alloc parameters\n"));
                    return MFX_ERR_MEMORY_ALLOC;
                }
            }
        }
#endif
    }
    if (m_pAllocParams.empty()) {
        m_pAllocParams.push_back(std::make_shared<mfxAllocatorParams>());
        hdls.push_back(NULL);

        for (i = 1; i < m_InputParamsArray.size(); i++) {
            m_pAllocParams.push_back(m_pAllocParams.back());
            hdls.push_back(NULL);
        }
    }

    // each pair of source and sink has own safety buffer
    sts = CreateSafetyBuffers();
    MSDK_CHECK_STATUS(sts, "CreateSafetyBuffers failed");

    /* One more hint. Example you have 3 dec + 1 enc sessions
    * (enc means vpp_comp call invoked. m_InputParamsArray.size() is 4.
    * You don't need take vpp comp params from last one session as it is enc session.
    * But you need process {0, 1, 2} sessions - totally 3.
    * So, you need start from 0 and end at 2.
    * */
    for (mfxI32 jj = 0; jj < (mfxI32)m_InputParamsArray.size() - 1; jj++) {
        /* Save params for VPP composition */
        sVppCompDstRect tempDstRect;
        tempDstRect.DstX   = m_InputParamsArray[jj].nVppCompDstX;
        tempDstRect.DstY   = m_InputParamsArray[jj].nVppCompDstY;
        tempDstRect.DstW   = m_InputParamsArray[jj].nVppCompDstW;
        tempDstRect.DstH   = m_InputParamsArray[jj].nVppCompDstH;
        tempDstRect.TileId = m_InputParamsArray[jj].nVppCompTileId;
        m_VppDstRects.push_back(tempDstRect);
    }

    // create sessions, allocators
    for (i = 0; i < m_InputParamsArray.size(); i++) {
        msdk_printf(MSDK_STRING("Session %d:\n"), (int)i);
        auto pAllocator = std::make_unique<GeneralAllocator>();
        sts             = pAllocator->Init(m_pAllocParams[i].get());
        MSDK_CHECK_STATUS(sts, "pAllocator->Init failed");

        m_pAllocArray.push_back(std::move(pAllocator));

        auto pThreadPipeline = std::make_unique<ThreadTranscodeContext>();
        // extend BS processing init
        m_pExtBSProcArray.push_back(std::make_unique<FileBitstreamProcessor>());

        pThreadPipeline->pPipeline.reset(CreatePipeline());

        // no need, in lowLatencyMode adapter type is unknown
        if (!lowLatencyMode) {
            pThreadPipeline->pPipeline->SetAdapterType(m_pLoader->GetAdapterType());
            pThreadPipeline->pPipeline->SetPrefferdGfx(m_InputParamsArray[i].dGfxIdx);
        }

        pThreadPipeline->pPipeline->SetAdapterNum(m_pLoader->GetDeviceIDAndAdapter().second);

        pThreadPipeline->pBSProcessor = m_pExtBSProcArray.back().get();

        std::unique_ptr<CSmplBitstreamReader> reader;
        std::unique_ptr<CSmplYUVReader> yuvreader;
        if (m_InputParamsArray[i].DecodeId == MFX_CODEC_VP9 ||
            m_InputParamsArray[i].DecodeId == MFX_CODEC_VP8 ||
            m_InputParamsArray[i].DecodeId == MFX_CODEC_AV1) {
            reader.reset(new CIVFFrameReader());
        }
        else if (m_InputParamsArray[i].DecodeId == MFX_CODEC_RGB4 ||
                 m_InputParamsArray[i].DecodeId == MFX_CODEC_I420 ||
                 m_InputParamsArray[i].DecodeId == MFX_CODEC_NV12) {
            // YUV reader for RGB4 overlay and raw input
            yuvreader.reset(new CSmplYUVReader());
        }
        else {
            reader.reset(new CSmplBitstreamReader());
        }

        if (reader.get()) {
            sts = reader->Init(m_InputParamsArray[i].strSrcFile);
            if (sts == MFX_ERR_UNSUPPORTED && m_InputParamsArray[i].DecodeId == MFX_CODEC_AV1) {
                reader.reset(new CSmplBitstreamReader());
                msdk_printf(MSDK_STRING("WARNING: Stream is not IVF, default reader\n"));
            }
            MSDK_CHECK_STATUS(sts, "reader->Init failed");
            sts = m_pExtBSProcArray.back()->SetReader(reader);
            MSDK_CHECK_STATUS(sts, "m_pExtBSProcArray.back()->SetReader failed");
        }
        else if (yuvreader.get()) {
            std::list<msdk_string> input;
            input.push_back(m_InputParamsArray[i].strSrcFile);
            sts = yuvreader->Init(input, m_InputParamsArray[i].DecodeId);
            MSDK_CHECK_STATUS(sts, "m_YUVReader->Init failed");
            sts = m_pExtBSProcArray.back()->SetReader(yuvreader);
            MSDK_CHECK_STATUS(sts, "m_pExtBSProcArray.back()->SetReader failed");
        }

        if (msdk_strncmp(MSDK_STRING("null"),
                         m_InputParamsArray[i].strDstFile,
                         msdk_strlen(MSDK_STRING("null")))) {
            auto writer = std::make_unique<CSmplBitstreamWriter>();
            sts         = writer->Init(m_InputParamsArray[i].strDstFile);

            sts = m_pExtBSProcArray.back()->SetWriter(writer);
            MSDK_CHECK_STATUS(sts, "m_pExtBSProcArray.back()->SetWriter failed");
        }

        if (Sink == m_InputParamsArray[i].eMode) {
            /* N_to_1 mode */
            if ((VppComp == m_InputParamsArray[i].eModeExt) ||
                (VppCompOnly == m_InputParamsArray[i].eModeExt)) {
                // Taking buffers from tail because they are stored in m_pBufferArray in reverse order
                // So, by doing this we'll fill buffers properly according to order from par file
                pBuffer = m_pBufferArray[m_pBufferArray.size() - 1 - BufCounter].get();
                BufCounter++;
            }
            else /* 1_to_N mode*/
            {
                pBuffer = m_pBufferArray[m_pBufferArray.size() - 1].get();
            }
            pSinkPipeline = pThreadPipeline->pPipeline.get();
        }
        else if (Source == m_InputParamsArray[i].eMode) {
            /* N_to_1 mode */
            if ((VppComp == m_InputParamsArray[i].eModeExt) ||
                (VppCompOnly == m_InputParamsArray[i].eModeExt)) {
                pBuffer = m_pBufferArray[m_pBufferArray.size() - 1].get();
            }
            else /* 1_to_N mode*/
            {
                pBuffer = m_pBufferArray[BufCounter].get();
                BufCounter++;
            }
        }
        else {
            pBuffer = NULL;
        }

        /**/
        /* Vector stored linearly in the memory !*/
        m_InputParamsArray[i].pVppCompDstRects = m_VppDstRects.empty() ? NULL : &m_VppDstRects[0];

        // if session has VPP plus ENCODE only (-i::source option)
        // use decode source session as input
        sts = MFX_ERR_MORE_DATA;

        auto pipeline = Source == m_InputParamsArray[i].eMode ? pSinkPipeline : pParentPipeline;
        sts           = CheckAndFixAdapterDependency(i, pipeline);
        MSDK_CHECK_STATUS(sts, "CheckAndFixAdapterDependency failed");
        // force implementation type based on iGfx/dGfx parameters
        if (sts == MFX_WRN_VIDEO_PARAM_CHANGED &&
            m_InputParamsArray[i].libType != MFX_IMPL_SOFTWARE) {
            if (m_InputParamsArray[i].dGfxIdx >= 0)
                m_pLoader->SetDiscreteAdapterIndex(m_InputParamsArray[i].dGfxIdx);
            else
                m_pLoader->SetAdapterType(m_InputParamsArray[i].adapterType);

            if (m_InputParamsArray[i].adapterNum >= 0)
                m_pLoader->SetAdapterNum(m_InputParamsArray[i].adapterNum);

            sts = m_pLoader->ConfigureAndEnumImplementations(m_InputParamsArray[i].libType,
                                                             m_accelerationMode,
                                                             lowLatencyMode);
            MSDK_CHECK_STATUS(sts, "ConfigureAndEnumImplementations failed");
        }
        sts = pThreadPipeline->pPipeline->Init(&m_InputParamsArray[i],
                                               m_pAllocArray[i].get(),
                                               hdls[i],
                                               pipeline,
                                               pBuffer,
                                               m_pExtBSProcArray.back().get(),
                                               m_pLoader.get(),
                                               CreateCascadeScalerConfig());

        MSDK_CHECK_STATUS(sts, "pThreadPipeline->pPipeline->Init failed");

        if (!pParentPipeline && m_InputParamsArray[i].bIsJoin)
            pParentPipeline = pThreadPipeline->pPipeline.get();

        // set the session's start status (like it is waiting)
        pThreadPipeline->startStatus = MFX_WRN_DEVICE_BUSY;
        // set other session's parameters
        pThreadPipeline->implType = m_InputParamsArray[i].libType;
        m_pThreadContextArray.push_back(std::move(pThreadPipeline));

        mfxVersion ver = { { 0, 0 } };
        sts            = m_pThreadContextArray[i]->pPipeline->QueryMFXVersion(&ver);
        MSDK_CHECK_STATUS(sts, "m_pThreadContextArray[i]->pPipeline->QueryMFXVersion failed");

        PrintInfo(i, &m_InputParamsArray[i], &ver);
    }

    if (m_InputParamsArray[0].forceSyncAllSession == MFX_CODINGOPTION_ON) {
        auto maxNumFrameForAllocIter = std::max_element(
            std::begin(m_pThreadContextArray),
            std::end(m_pThreadContextArray),
            [](auto const& l, auto const& r) {
                return l->pPipeline->GetNumFrameForAlloc() < r->pPipeline->GetNumFrameForAlloc();
            });

        auto surfaceUtilizationSynchronizer = std::make_shared<SurfaceUtilizationSynchronizer>(
            (*maxNumFrameForAllocIter)->pPipeline->GetNumFrameForAlloc());

        for (size_t i = 0; i < m_pThreadContextArray.size(); ++i) {
            m_pThreadContextArray[i]->pPipeline->SetSurfaceUtilizationSynchronizer(
                surfaceUtilizationSynchronizer);
        }
    }

    for (i = 0; i < m_InputParamsArray.size(); i++) {
        sts = m_pThreadContextArray[i]->pPipeline->CompleteInit();
        MSDK_CHECK_STATUS(sts, "m_pThreadContextArray[i]->pPipeline->CompleteInit failed");

        if (m_pThreadContextArray[i]->pPipeline->GetJoiningFlag())
            msdk_printf(MSDK_STRING("Session %d was joined with other sessions\n"), (int)i);
        else
            msdk_printf(MSDK_STRING("Session %d was NOT joined with other sessions\n"), (int)i);

        m_pThreadContextArray[i]->pPipeline->SetPipelineID(i);
    }

    msdk_printf(MSDK_STRING("\n"));

    return sts;

} // mfxStatus Launcher::Init()

void Launcher::Run() {
    msdk_printf(MSDK_STRING("Transcoding started\n"));

    // mark start time
    m_StartTime = GetTick();

    // Robust flag is applied to every seession if enabled in one
    if (m_pThreadContextArray[0]->pPipeline->GetRobustFlag()) {
        DoRobustTranscoding();
    }
    else {
        DoTranscoding();
    }

    msdk_printf(MSDK_STRING("\nTranscoding finished\n"));

} // mfxStatus Launcher::Init()

void Launcher::DoTranscoding() {
    auto RunTranscodeRoutine = [](ThreadTranscodeContext* context) {
        context->handle = std::async(std::launch::async, [context]() {
            context->TranscodeRoutine();
        });
    };

    bool isOverlayUsed = false;
    for (const auto& context : m_pThreadContextArray) {
        MSDK_CHECK_POINTER_NO_RET(context);
        RunTranscodeRoutine(context.get());

        MSDK_CHECK_POINTER_NO_RET(context->pPipeline);
        isOverlayUsed = isOverlayUsed || context->pPipeline->IsOverlayUsed();
    }

    // Transcoding threads waiting cycle
    bool aliveNonOverlaySessions = true;
    while (aliveNonOverlaySessions) {
        aliveNonOverlaySessions = false;

        for (size_t i = 0; i < m_pThreadContextArray.size(); ++i) {
            if (!m_pThreadContextArray[i]->handle.valid())
                continue;

            //Payslip interval to check the state of working threads:
            //such interval is usually a realtime, i.e. for 30 fps this would be 33ms,
            //66ms typically mean either 1/fps or 2/fps payslip checks.
            auto waitSts = m_pThreadContextArray[i]->handle.wait_for(std::chrono::milliseconds(66));
            if (waitSts == std::future_status::ready) {
                // Invoke get() of the handle just to reset the valid state.
                // This allows to skip already processed sessions
                m_pThreadContextArray[i]->handle.get();

                // Session is completed, let's check for its status
                if (m_pThreadContextArray[i]->transcodingSts < MFX_ERR_NONE) {
                    // Stop all the sessions if an error happened in one
                    // But do not stop in robust mode when gpu hang's happened
                    if (m_pThreadContextArray[i]->transcodingSts != MFX_ERR_GPU_HANG ||
                        !m_pThreadContextArray[i]->pPipeline->GetRobustFlag()) {
                        msdk_stringstream ss;
                        ss << MSDK_STRING("\n\n session ") << i << MSDK_STRING(" [")
                           << m_pThreadContextArray[i]->pPipeline->GetSessionText()
                           << MSDK_STRING("] failed with status ")
                           << StatusToString(m_pThreadContextArray[i]->transcodingSts)
                           << MSDK_STRING(" shutting down the application...") << std::endl
                           << std::endl;
                        msdk_printf(MSDK_STRING("%s"), ss.str().c_str());

                        for (const auto& context : m_pThreadContextArray) {
                            context->pPipeline->StopSession();
                        }
                    }
                }
                else if (m_pThreadContextArray[i]->transcodingSts > MFX_ERR_NONE) {
                    msdk_stringstream ss;
                    ss << MSDK_STRING("\n\n session ") << i << MSDK_STRING(" [")
                       << m_pThreadContextArray[i]->pPipeline->GetSessionText()
                       << MSDK_STRING("] returned warning status ")
                       << StatusToString(m_pThreadContextArray[i]->transcodingSts) << std::endl
                       << std::endl;
                    msdk_printf(MSDK_STRING("%s"), ss.str().c_str());
                }
            }
            else {
                aliveNonOverlaySessions = aliveNonOverlaySessions ||
                                          !m_pThreadContextArray[i]->pPipeline->IsOverlayUsed();
            }
        }

        // Stop overlay sessions
        // Note: Overlay sessions never stop themselves so they should be forcibly stopped
        // after stopping of all non-overlay sessions
        if (!aliveNonOverlaySessions && isOverlayUsed) {
            // Sending stop message
            for (const auto& context : m_pThreadContextArray) {
                if (context->pPipeline->IsOverlayUsed()) {
                    context->pPipeline->StopSession();
                }
            }

            // Waiting for them to be stopped
            for (const auto& context : m_pThreadContextArray) {
                if (!context->handle.valid())
                    continue;

                context->handle.wait();
            }
        }
    }
}

void Launcher::DoRobustTranscoding() {
    mfxStatus sts = MFX_ERR_NONE;

    // Cycle for handling MFX_ERR_GPU_HANG during transcoding
    // If it's returned, reset all the pipelines and start over from the last point
    bool bGPUHang = false;
    for (;;) {
        if (bGPUHang) {
            for (size_t i = 0; i < m_pThreadContextArray.size(); i++) {
                sts = m_pThreadContextArray[i]->pPipeline->Reset(m_pLoader.get());
                if (sts) {
                    msdk_printf(
                        MSDK_STRING("\n[WARNING] GPU Hang recovery wasn't succeed. Exiting...\n"));
                    return;
                }
            }
            bGPUHang = false;
            msdk_printf(MSDK_STRING("\n[WARNING] Successfully recovered. Continue transcoding.\n"));
        }

        DoTranscoding();

        for (size_t i = 0; i < m_pThreadContextArray.size(); i++) {
            if (m_pThreadContextArray[i]->transcodingSts == MFX_ERR_GPU_HANG) {
                bGPUHang = true;
            }
        }
        if (!bGPUHang)
            break;
        msdk_printf(MSDK_STRING("\n[WARNING] GPU Hang has happened. Trying to recover...\n"));
    }
}

mfxStatus Launcher::ProcessResult() {
    FILE* pPerfFile = m_parser.GetPerformanceFile();

    msdk_stringstream ssTranscodingTime;
    ssTranscodingTime << std::endl
                      << MSDK_STRING("Common transcoding time is ") << GetTime(m_StartTime)
                      << MSDK_STRING(" sec") << std::endl;

    m_parser.PrintParFileName();

    msdk_printf(MSDK_STRING("%s"), ssTranscodingTime.str().c_str());
    if (pPerfFile) {
        msdk_fprintf(pPerfFile, MSDK_STRING("%s"), ssTranscodingTime.str().c_str());
    }

    mfxStatus FinalSts = MFX_ERR_NONE;
    msdk_printf(MSDK_STRING(
        "-------------------------------------------------------------------------------\n"));

    for (mfxU32 i = 0; i < m_pThreadContextArray.size(); i++) {
        mfxStatus transcodingSts = m_pThreadContextArray[i]->transcodingSts;
        mfxF64 workTime          = m_pThreadContextArray[i]->working_time;
        mfxU32 framesNum         = m_pThreadContextArray[i]->numTransFrames;

        if (!FinalSts)
            FinalSts = transcodingSts;

        msdk_string SessionStsStr = transcodingSts ? msdk_string(MSDK_STRING("FAILED"))
                                                   : msdk_string((MSDK_STRING("PASSED")));

        msdk_stringstream ss;
        ss << MSDK_STRING("*** session ") << i << MSDK_STRING(" [")
           << m_pThreadContextArray[i]->pPipeline->GetSessionText() << MSDK_STRING("] ")
           << SessionStsStr << MSDK_STRING(" (") << StatusToString(transcodingSts)
           << MSDK_STRING(") ") << workTime << MSDK_STRING(" sec, ") << framesNum
           << MSDK_STRING(" frames, ") << std::fixed << std::setprecision(3) << framesNum / workTime
           << MSDK_STRING(" fps") << std::endl
           << m_parser.GetLine(i) << std::endl
           << std::endl;

        msdk_printf(MSDK_STRING("%s"), ss.str().c_str());
        if (pPerfFile) {
            msdk_fprintf(pPerfFile, MSDK_STRING("%s"), ss.str().c_str());
        }
    }
    msdk_printf(MSDK_STRING(
        "-------------------------------------------------------------------------------\n"));

    msdk_stringstream ssTest;
    ssTest << std::endl
           << MSDK_STRING("The test ")
           << (FinalSts ? msdk_string(MSDK_STRING("FAILED")) : msdk_string(MSDK_STRING("PASSED")))
           << std::endl;

    msdk_printf(MSDK_STRING("%s"), ssTest.str().c_str());
    if (pPerfFile) {
        msdk_fprintf(pPerfFile, MSDK_STRING("%s"), ssTest.str().c_str());
    }
    return FinalSts;
} // mfxStatus Launcher::ProcessResult()

mfxStatus Launcher::CheckAndFixAdapterDependency(mfxU32 idxSession,
                                                 CTranscodingPipeline* pParentPipeline) {
    if (!pParentPipeline)
        return MFX_ERR_NONE;

    // Inherited sessions must have the same adapter as parent
    if (pParentPipeline->GetAdapterType() != mfxMediaAdapterType::MFX_MEDIA_UNKNOWN &&
        m_InputParamsArray[idxSession].adapterType == mfxMediaAdapterType::MFX_MEDIA_UNKNOWN) {
        m_InputParamsArray[idxSession].adapterType = pParentPipeline->GetAdapterType();
        m_InputParamsArray[idxSession].dGfxIdx     = pParentPipeline->GetdGfxIdx();
        msdk_stringstream ss;
        ss << MSDK_STRING("\n\n session with index: ") << idxSession
           << MSDK_STRING(" adapter type was forced to ")
           << (pParentPipeline->GetAdapterType() == mfxMediaAdapterType::MFX_MEDIA_INTEGRATED
                   ? MSDK_STRING("integrated")
                   : MSDK_STRING("discrete"));
        if (pParentPipeline->GetdGfxIdx() != -1)
            ss << MSDK_STRING(" with index ") << pParentPipeline->GetdGfxIdx();
        ss << std::endl << std::endl;
        msdk_printf(MSDK_STRING("%s"), ss.str().c_str());

        return MFX_WRN_VIDEO_PARAM_CHANGED;
    }

    if (pParentPipeline->GetAdapterNum() != -1 && m_InputParamsArray[idxSession].adapterNum == -1) {
        m_InputParamsArray[idxSession].adapterNum = pParentPipeline->GetAdapterNum();
        msdk_stringstream ss;
        ss << MSDK_STRING("\n\n session with index: ") << idxSession
           << MSDK_STRING(" adapter num was forced to ") << pParentPipeline->GetAdapterNum()
           << std::endl
           << std::endl;
        msdk_printf(MSDK_STRING("%s"), ss.str().c_str());

        return MFX_ERR_NONE;
    }

    // App can't change initialization of the previous session (parent session)
    if (pParentPipeline->GetAdapterType() == mfxMediaAdapterType::MFX_MEDIA_UNKNOWN &&
        m_InputParamsArray[idxSession].adapterType != mfxMediaAdapterType::MFX_MEDIA_UNKNOWN) {
        msdk_stringstream ss;
        ss << MSDK_STRING("\n\n session with index: ") << idxSession
           << MSDK_STRING(" failed because parent session [") << pParentPipeline->GetSessionText()
           << MSDK_STRING("] doesn't have explicit adapter setting") << std::endl
           << std::endl;
        msdk_printf(MSDK_STRING("%s"), ss.str().c_str());

        return MFX_ERR_UNSUPPORTED;
    }

    // Inherited sessions must have the same adapter as parent
    if (pParentPipeline->GetAdapterType() != m_InputParamsArray[idxSession].adapterType ||
        pParentPipeline->GetdGfxIdx() != m_InputParamsArray[idxSession].dGfxIdx) {
        msdk_stringstream ss;
        ss << MSDK_STRING("\n\n session with index: ") << idxSession
           << MSDK_STRING(" failed because it has different adapter type with parent session [")
           << pParentPipeline->GetSessionText() << MSDK_STRING("]") << std::endl
           << std::endl;
        msdk_printf(MSDK_STRING("%s"), ss.str().c_str());

        return MFX_ERR_UNSUPPORTED;
    }

    if (pParentPipeline->GetAdapterNum() != m_InputParamsArray[idxSession].adapterNum) {
        msdk_stringstream ss;
        ss << MSDK_STRING("\n\n session with index: ") << idxSession
           << MSDK_STRING(" failed because it has different adapter num with parent session [")
           << pParentPipeline->GetSessionText() << MSDK_STRING("]") << std::endl
           << std::endl;
        msdk_printf(MSDK_STRING("%s"), ss.str().c_str());

        return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_NONE;
}

mfxStatus Launcher::VerifyCrossSessionsOptions() {
    bool IsSinkPresence       = false;
    bool IsSourcePresence     = false;
    bool IsHeterSessionJoin   = false;
    bool IsFirstInTopology    = true;
    bool IsInterOrJoined      = false;
    bool IsNeedToCreateDevice = false;

    mfxU16 minAsyncDepth      = 0;
    bool bSingleTexture       = false;
    bool allMFEModesEqual     = true;
    bool allMFEFramesEqual    = true;
    bool allMFESessionsJoined = true;

    mfxU16 usedMFEMaxFrames = 0;
    mfxU16 usedMFEMode      = 0;

    bool forceSyncAllSession = false;

    for (mfxU32 i = 0; i < m_InputParamsArray.size(); i++) {
        // loop over all sessions and check mfe-specific params
        // for mfe is required to have sessions joined, HW impl
        if (m_InputParamsArray[i].numMFEFrames > 1) {
            usedMFEMaxFrames = m_InputParamsArray[i].numMFEFrames;
            for (mfxU32 j = 0; j < m_InputParamsArray.size(); j++) {
                if (m_InputParamsArray[j].numMFEFrames &&
                    m_InputParamsArray[j].numMFEFrames != usedMFEMaxFrames) {
                    m_InputParamsArray[j].numMFEFrames = usedMFEMaxFrames;
                    allMFEFramesEqual                  = false;
                    m_InputParamsArray[j].MFMode       = m_InputParamsArray[j].MFMode < MFX_MF_AUTO
                                                       ? MFX_MF_AUTO
                                                       : m_InputParamsArray[j].MFMode;
                }
                if (m_InputParamsArray[j].bIsJoin == false) {
                    allMFESessionsJoined          = false;
                    m_InputParamsArray[j].bIsJoin = true;
                }
            }
        }
        if (m_InputParamsArray[i].MFMode >= MFX_MF_AUTO) {
            usedMFEMode = m_InputParamsArray[i].MFMode;
            for (mfxU32 j = 0; j < m_InputParamsArray.size(); j++) {
                if (m_InputParamsArray[j].MFMode && m_InputParamsArray[j].MFMode != usedMFEMode) {
                    m_InputParamsArray[j].MFMode = usedMFEMode;
                    allMFEModesEqual             = false;
                }
                if (m_InputParamsArray[j].bIsJoin == false) {
                    allMFESessionsJoined          = false;
                    m_InputParamsArray[j].bIsJoin = true;
                }
            }
        }
    }
    if (!allMFEFramesEqual)
        msdk_printf(
            MSDK_STRING(
                "WARNING: All sessions for MFE should have the same number of MFE frames!\n used ammount of frame for MFE: %d\n"),
            (int)usedMFEMaxFrames);
    if (!allMFEModesEqual)
        msdk_printf(
            MSDK_STRING(
                "WARNING: All sessions for MFE should have the same mode!\n, used mode: %d\n"),
            (int)usedMFEMode);
    if (!allMFESessionsJoined)
        msdk_printf(MSDK_STRING(
            "WARNING: Sessions for MFE should be joined! All sessions forced to be joined\n"));

    for (mfxU32 i = 0; i < m_InputParamsArray.size(); i++) {
        if (m_InputParamsArray[i].bSingleTexture) {
            bSingleTexture = true;
        }

        // All sessions have to know about timeout
        if (m_InputParamsArray[i].nTimeout && (m_InputParamsArray[i].eMode == Sink)) {
            for (mfxU32 j = 0; j < m_InputParamsArray.size(); j++) {
                if (m_InputParamsArray[j].MaxFrameNumber != MFX_INFINITE) {
                    msdk_printf(MSDK_STRING(
                        "\"-timeout\" option isn't compatible with \"-n\". \"-n\" will be ignored.\n"));
                    for (mfxU32 k = 0; k < m_InputParamsArray.size(); k++) {
                        m_InputParamsArray[k].MaxFrameNumber = MFX_INFINITE;
                    }
                    break;
                }
            }
            msdk_printf(MSDK_STRING("Timeout %d seconds has been set to all sessions\n"),
                        m_InputParamsArray[i].nTimeout);
            for (mfxU32 j = 0; j < m_InputParamsArray.size(); j++) {
                m_InputParamsArray[j].nTimeout = m_InputParamsArray[i].nTimeout;
            }
        }

        // All sessions have to know if robust mode enabled
        if (m_InputParamsArray[i].bRobustFlag) {
            for (mfxU32 j = 0; j < m_InputParamsArray.size(); j++) {
                m_InputParamsArray[j].bRobustFlag = m_InputParamsArray[i].bRobustFlag;
            }
        }

        if (Source == m_InputParamsArray[i].eMode) {
            if (m_InputParamsArray[i].nAsyncDepth < minAsyncDepth) {
                minAsyncDepth = m_InputParamsArray[i].nAsyncDepth;
            }
            // topology definition
            if (!IsSinkPresence) {
                PrintError(MSDK_STRING(
                    "Error in par file. Decode source session must be declared BEFORE encode sinks \n"));
                return MFX_ERR_UNSUPPORTED;
            }
            IsSourcePresence = true;

            if (IsFirstInTopology) {
                if (m_InputParamsArray[i].bIsJoin)
                    IsHeterSessionJoin = true;
                else
                    IsHeterSessionJoin = false;
            }
            else {
                if (m_InputParamsArray[i].bIsJoin && !IsHeterSessionJoin) {
                    PrintError(MSDK_STRING(
                        "Error in par file. All heterogeneous sessions must be joined \n"));
                    return MFX_ERR_UNSUPPORTED;
                }
                if (!m_InputParamsArray[i].bIsJoin && IsHeterSessionJoin) {
                    PrintError(MSDK_STRING(
                        "Error in par file. All heterogeneous sessions must be NOT joined \n"));
                    return MFX_ERR_UNSUPPORTED;
                }
            }

            if (IsFirstInTopology)
                IsFirstInTopology = false;
        }
        else if (Sink == m_InputParamsArray[i].eMode) {
            minAsyncDepth  = m_InputParamsArray[i].nAsyncDepth;
            IsSinkPresence = true;

            if (IsFirstInTopology) {
                if (m_InputParamsArray[i].bIsJoin)
                    IsHeterSessionJoin = true;
                else
                    IsHeterSessionJoin = false;
            }
            else {
                if (m_InputParamsArray[i].bIsJoin && !IsHeterSessionJoin) {
                    PrintError(MSDK_STRING(
                        "Error in par file. All heterogeneous sessions must be joined \n"));
                    return MFX_ERR_UNSUPPORTED;
                }
                if (!m_InputParamsArray[i].bIsJoin && IsHeterSessionJoin) {
                    PrintError(MSDK_STRING(
                        "Error in par file. All heterogeneous sessions must be NOT joined \n"));
                    return MFX_ERR_UNSUPPORTED;
                }
            }

            if (IsFirstInTopology)
                IsFirstInTopology = false;
        }

        if (m_InputParamsArray[i].nMemoryModel == UNKNOWN_ALLOC) {
            m_InputParamsArray[i].nMemoryModel = GENERAL_ALLOC;
        }

        // Creating a device is only necessary in case of using external memory (generall alloc) or inter/joined sessions.
        IsInterOrJoined = m_InputParamsArray[i].eMode == Sink ||
                          m_InputParamsArray[i].eMode == Source || m_InputParamsArray[i].bIsJoin;
        if ((m_InputParamsArray[i].nMemoryModel == GENERAL_ALLOC || IsInterOrJoined) &&
            !m_InputParamsArray[i].bForceSysMem) {
            IsNeedToCreateDevice = true;
        }

        if (MFX_IMPL_SOFTWARE != m_InputParamsArray[i].libType) {
            // TODO: can we avoid ifdef and use MFX_IMPL_VIA_VAAPI?
#if defined(_WIN32) || defined(_WIN64)
            if (MFX_IMPL_VIA_D3D11 == MFX_IMPL_VIA_MASK(m_InputParamsArray[i].libType)) {
                // If m_eDevType is not specified here, the device will not be created after.
                if (IsNeedToCreateDevice)
                    m_eDevType = MFX_HANDLE_D3D11_DEVICE;
                m_accelerationMode = MFX_ACCEL_MODE_VIA_D3D11;
            }
            else {
                if (IsNeedToCreateDevice)
                    m_eDevType = MFX_HANDLE_D3D9_DEVICE_MANAGER;
                m_accelerationMode = MFX_ACCEL_MODE_VIA_D3D9;
            }
#elif defined(LIBVA_SUPPORT)
            if (IsNeedToCreateDevice)
                m_eDevType = MFX_HANDLE_VA_DISPLAY;
            m_accelerationMode = MFX_ACCEL_MODE_VIA_VAAPI;
#endif
        }

        // forceSyncAllSession is ON by default for 1->N scenarios
        forceSyncAllSession |=
            m_InputParamsArray[i].forceSyncAllSession == MFX_CODINGOPTION_ON ||
            (m_InputParamsArray[i].forceSyncAllSession == MFX_CODINGOPTION_UNKNOWN &&
             m_InputParamsArray.size() > 1 &&
             (m_InputParamsArray[i].nMemoryModel == VISIBLE_INT_ALLOC ||
              m_InputParamsArray[i].nMemoryModel == HIDDEN_INT_ALLOC));
    }

    if (forceSyncAllSession)
        std::for_each(std::begin(m_InputParamsArray), std::end(m_InputParamsArray), [](auto& p) {
            p.forceSyncAllSession = MFX_CODINGOPTION_ON;
        });

    // Async depth between inter-sessions should be equal to the minimum async depth of all these sessions.
    for (mfxU32 i = 0; i < m_InputParamsArray.size(); i++) {
        if ((m_InputParamsArray[i].eMode == Source) || (m_InputParamsArray[i].eMode == Sink)) {
            m_InputParamsArray[i].nAsyncDepth = minAsyncDepth;
        }
    }

    if (IsSinkPresence && !IsSourcePresence) {
        PrintError(MSDK_STRING("Error: Sink must be defined"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (bSingleTexture) {
        bool showWarning = false;
        for (mfxU32 j = 0; j < m_InputParamsArray.size(); j++) {
            if (!m_InputParamsArray[j].bSingleTexture) {
                showWarning = true;
            }
            m_InputParamsArray[j].bSingleTexture = true;
        }
        if (showWarning) {
            msdk_printf(MSDK_STRING(
                "WARNING: At least one session has -single_texture_d3d11 option, all other sessions are modified to have this setting enabled al well.\n"));
        }
    }

    return MFX_ERR_NONE;

} // mfxStatus Launcher::VerifyCrossSessionsOptions()

mfxStatus Launcher::CreateSafetyBuffers() {
    SafetySurfaceBuffer* pBuffer     = NULL;
    SafetySurfaceBuffer* pPrevBuffer = NULL;

    for (mfxU32 i = 0; i < m_InputParamsArray.size(); i++) {
        /* this is for 1 to N case*/
        if ((Source == m_InputParamsArray[i].eMode) && (Native == m_InputParamsArray[0].eModeExt)) {
            pBuffer           = new SafetySurfaceBuffer(pPrevBuffer);
            pBuffer->TargetID = m_InputParamsArray[i].TargetID;
            pPrevBuffer       = pBuffer;
            m_pBufferArray.push_back((std::unique_ptr<SafetySurfaceBuffer>(pBuffer)));
        }

        /* And N_to_1 case: composition should be enabled!
         * else it is logic error */
        if ((Source != m_InputParamsArray[i].eMode) &&
            ((VppComp == m_InputParamsArray[0].eModeExt) ||
             (VppCompOnly == m_InputParamsArray[0].eModeExt))) {
            pBuffer     = new SafetySurfaceBuffer(pPrevBuffer);
            pPrevBuffer = pBuffer;
            m_pBufferArray.push_back(std::unique_ptr<SafetySurfaceBuffer>(pBuffer));
        }
    }
    return MFX_ERR_NONE;

} // mfxStatus Launcher::CreateSafetyBuffers

CascadeScalerConfig::TargetDescriptor CascadeScalerConfig::GetDesc(mfxU32 id) {
    auto itr = std::find_if(Targets.begin(), Targets.end(), [id](TargetDescriptor& d) {
        return d.TargetID == id;
    });
    if (itr != Targets.end()) {
        return *itr;
    }
    else {
        return TargetDescriptor();
    }
}

CascadeScalerConfig& Launcher::CreateCascadeScalerConfig() {
    CascadeScalerConfig& cfg = m_CSConfig;
    if (cfg.ParFileImported) {
        return m_CSConfig;
    }

    //process par file
    for (sInputParams& par : m_InputParamsArray) {
        if (par.eMode == Sink) {
            if (par.EnableTracing) {
                cfg.Tracer->Init();
            }
        }
        else if (par.eMode == Source) {
            //this is encoder, import par file params
            CascadeScalerConfig::TargetDescriptor desc;
            desc.TargetID  = par.TargetID;
            desc.DstHeight = par.nDstHeight;
            desc.DstWidth  = par.nDstWidth;

            desc.FRC = (par.FRCAlgorithm != 0);
            if (desc.FRC) {
                desc.DstFrameRate = par.dVPPOutFramerate;
            }

            desc.DI = par.bEnableDeinterlacing;
            if (desc.DI) {
                desc.DstPicStruct = MFX_PICSTRUCT_PROGRESSIVE;
            }

            desc.CascadeScaler = par.CascadeScaler;
            if (desc.CascadeScaler) {
                cfg.CascadeScalerRequired = true;
            }

            cfg.Targets.push_back(desc);
            cfg.InParams[desc.TargetID] = par;
        }
    }

    cfg.ParFileImported = true;
    cfg.CreatePoolList();

    return m_CSConfig;
}

//propagate cascade parameters, decoder parameters should be set before this call
void TranscodingSample::CascadeScalerConfig::PropagateCascadeParameters() {
    if (Targets.size() <= 1) {
        //we should have at least two enc channels to propagate something
        return;
    }

    //special case for first channel output, should be incorporated into loop below, somehow
    if (!Targets[0].FRC) {
        Targets[0].DstFrameRate = Targets[0].SrcFrameRate;
    }
    if (!Targets[0].DI) {
        Targets[0].DstPicStruct = Targets[0].SrcPicStruct;
    }

    //set current parameters equal to decoder output
    mfxU16 Width     = Targets[0].SrcWidth;
    mfxU16 Height    = Targets[0].SrcHeight;
    double FrameRate = Targets[0].SrcFrameRate;
    mfxU16 PicStruct = Targets[0].SrcPicStruct;

    for (mfxU32 i = 1; i < Targets.size(); i++) {
        //resolution
        if (Targets[i - 1].CascadeScaler) {
            Width = Targets[i].SrcWidth = Targets[i - 1].DstWidth;
            Height = Targets[i].SrcHeight = Targets[i - 1].DstHeight;
        }
        else {
            Targets[i].SrcWidth  = Width;
            Targets[i].SrcHeight = Height;
        }

        //framerate
        if (Targets[i - 1].CascadeScaler && Targets[i - 1].FRC) {
            FrameRate = Targets[i].SrcFrameRate = Targets[i - 1].DstFrameRate;
        }
        else {
            Targets[i].SrcFrameRate = FrameRate;
        }
        if (!Targets[i].FRC) {
            Targets[i].DstFrameRate = Targets[i].SrcFrameRate;
        }

        //PicStruct
        if (Targets[i - 1].CascadeScaler && Targets[i - 1].DI) {
            PicStruct = Targets[i].SrcPicStruct = Targets[i - 1].DstPicStruct;
        }
        else {
            Targets[i].SrcPicStruct = PicStruct;
        }
        if (!Targets[i].DI) {
            Targets[i].DstPicStruct = Targets[i].SrcPicStruct;
        }
    }

    PoolDescritpor& pool = Pools[DecoderPoolID];
    pool.SurfaceWidth    = Targets[0].SrcWidth;
    pool.SurfaceHeight   = Targets[0].SrcHeight;
}

void TranscodingSample::CascadeScalerConfig::CreatePoolList() {
    if (Targets.empty()) {
        return;
    }

    PoolDescritpor pool;
    pool.PrevID        = 0;
    pool.ID            = DecoderPoolID;
    pool.SurfaceWidth  = 0; // Targets[0].SrcWidth;
    pool.SurfaceHeight = 0; // Targets[0].SrcHeight;
    Pools[pool.ID]     = pool;

    for (TargetDescriptor& desc : Targets) {
        if (desc.CascadeScaler) {
            pool.PrevID        = pool.ID;
            pool.ID            = DecoderPoolID + (desc.TargetID - DecoderTargetID);
            pool.TargetID      = desc.TargetID;
            pool.SurfaceWidth  = desc.DstWidth;
            pool.SurfaceHeight = desc.DstHeight;
            Pools[pool.ID]     = pool;
        }
        desc.PoolID = pool.ID;
    }
}

SMTTracer::SMTTracer() : Log(), AddonLog(), TracerFileMutex() {
    TimeBase = std::chrono::steady_clock::now();
}

SMTTracer::~SMTTracer() {
    //this function is intentionally called from destructor to try to save traces in case of a crush
    if (!Enabled)
        return;

    AddFlowEvents();
    SaveTrace(0xffffff & GetCurrentTS());
}

void SMTTracer::Init() {
    if (Enabled) {
        return;
    }
    Enabled = true;
    Log.reserve(TraceBufferSizeInMBytes * 1024 * 1024 / sizeof(Event));
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

void SMTTracer::SaveTrace(mfxU32 FileID) {
    string FileName = "smt_trace_" + to_string(FileID) + ".json";
    std::ofstream trace_file(FileName, std::ios::out);
    if (!trace_file) {
        return;
    }

    printf("\n### trace buffer usage [%d/%d] %.2f%%\n",
           (int)Log.size(),
           (int)Log.capacity(),
           100. * Log.size() / Log.capacity());
    printf("trace file name %s\n", FileName.c_str());

    trace_file << "[" << endl;

    for (const Event ev : Log) {
        WriteEvent(trace_file, ev);
    }
    for (const Event ev : AddonLog) {
        WriteEvent(trace_file, ev);
    }

    trace_file.close();
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
        printf("### Logging stopped\n");
        return;
    }

    std::lock_guard<std::mutex> guard(TracerFileMutex);
    Log.push_back(ev);
}

mfxU64 SMTTracer::GetCurrentTS() {
    std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(time - TimeBase).count();
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

        auto itp = find_if(itc, Log.rend(), [&it](Event ev) {
            return ev.EvType == EventType::DurationEnd && it->InID == ev.OutID;
        });
        if (itp == Log.rend()) {
            continue;
        }

        AddFlowEvent(*itp, *it);
    }
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
    trace_file << "}," << endl;
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
    trace_file << "}," << endl;
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
    trace_file << "}," << endl;
}

void SMTTracer::WriteEventPID(std::ofstream& trace_file) {
    trace_file << "\"pid\":\"smt\"";
}

void SMTTracer::WriteEventTID(std::ofstream& trace_file, const Event ev) {
    trace_file << "\"tid\":\"";
    switch (ev.ThType) {
        case ThreadType::DEC:
            trace_file << "dec";
            break;
        case ThreadType::VPP:
            trace_file << "enc" << ev.ThID;
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
                trace_file << "dec_pool";
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

void Launcher::Close() {
    while (m_pThreadContextArray.size()) {
        m_pThreadContextArray[m_pThreadContextArray.size() - 1].reset();
        m_pThreadContextArray.pop_back();
    }

    m_pAllocArray.clear();
    m_pBufferArray.clear();
    m_pExtBSProcArray.clear();
    m_pAllocParams.clear();
    m_hwdevs.clear();

} // void Launcher::Close()

#if defined(_WIN32) || defined(_WIN64)
int _tmain(int argc, TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    mfxStatus sts;
    Launcher transcode;
    if (argc < 2) {
        msdk_printf(MSDK_STRING(
            "[ERROR] Command line is empty. Use -? for getting help on available options.\n"));
        return 0;
    }

    sts = transcode.Init(argc, argv);
    if (sts == MFX_WRN_OUT_OF_RANGE) {
        // There's no error in parameters parsing, but we should not continue further. For instance, in case of -? option
        return MFX_ERR_NONE;
    }

    fflush(stdout);
    fflush(stderr);

    MSDK_CHECK_STATUS(sts, "transcode.Init failed");

    transcode.Run();

    sts = transcode.ProcessResult();
    fflush(stdout);
    fflush(stderr);
    MSDK_CHECK_STATUS(sts, "transcode.ProcessResult failed");

    return 0;
}
