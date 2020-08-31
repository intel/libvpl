/*###########################################################################
  # Copyright (C) 2005-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#include "mfx_samples_config.h"

#include "pipeline_user.h"
#include "sysmem_allocator.h"

#ifndef MFX_VERSION
    #error MFX_VERSION not defined
#endif

mfxStatus CUserPipeline::AllocFrames() {
    MSDK_CHECK_POINTER(m_pmfxENC, MFX_ERR_NOT_INITIALIZED);

    mfxStatus sts = MFX_ERR_NONE;

    mfxFrameAllocRequest EncRequest, RotateRequest;

    mfxU16 nEncSurfNum = 0; // number of frames at encoder input (rotate output)
    mfxU16 nRotateSurfNum = 0; // number of frames at rotate input

    MSDK_ZERO_MEMORY(EncRequest);

    sts = m_pmfxENC->QueryIOSurf(&m_mfxEncParams, &EncRequest);
    MSDK_CHECK_STATUS(sts, "m_pmfxENC->QueryIOSurf failed");

    if (EncRequest.NumFrameSuggested < m_mfxEncParams.AsyncDepth)
        return MFX_ERR_MEMORY_ALLOC;

    nEncSurfNum = EncRequest.NumFrameSuggested;

    // The number of surfaces for plugin input - so that plugin can work at async depth = m_nAsyncDepth
    nRotateSurfNum = MSDK_MAX(m_mfxEncParams.AsyncDepth, m_nMemBuffer);

    // If surfaces are shared by 2 components, c1 and c2. NumSurf = c1_out + c2_in - AsyncDepth + 1
    nEncSurfNum += nRotateSurfNum - m_mfxEncParams.AsyncDepth + 1;

    // prepare allocation requests
    EncRequest.NumFrameSuggested = EncRequest.NumFrameMin = nEncSurfNum;
    RotateRequest.NumFrameSuggested = RotateRequest.NumFrameMin =
        nRotateSurfNum;

    mfxU16 mem_type = MFX_MEMTYPE_EXTERNAL_FRAME;
    mem_type |= (SYSTEM_MEMORY == m_memType)
                    ? (mfxU16)MFX_MEMTYPE_SYSTEM_MEMORY
                    : (mfxU16)MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;

    EncRequest.Type = RotateRequest.Type = mem_type;

    EncRequest.Type |= MFX_MEMTYPE_FROM_ENCODE;
    RotateRequest.Type |=
        MFX_MEMTYPE_FROM_VPPOUT; // THIS IS A WORKAROUND, NEED TO ADJUST ALLOCATOR

    MSDK_MEMCPY_VAR(EncRequest.Info,
                    &(m_mfxEncParams.mfx.FrameInfo),
                    sizeof(mfxFrameInfo));

    // alloc frames for encoder input
    sts = m_pMFXAllocator->Alloc(m_pMFXAllocator->pthis,
                                 &EncRequest,
                                 &m_EncResponse);
    MSDK_CHECK_STATUS(sts, "m_pMFXAllocator->Alloc failed");

    // prepare mfxFrameSurface1 array for components
    m_pEncSurfaces = new mfxFrameSurface1[nEncSurfNum];
    MSDK_CHECK_POINTER(m_pEncSurfaces, MFX_ERR_MEMORY_ALLOC);

    for (int i = 0; i < nEncSurfNum; i++) {
        MSDK_ZERO_MEMORY(m_pEncSurfaces[i]);
        MSDK_MEMCPY_VAR(m_pEncSurfaces[i].Info,
                        &(m_mfxEncParams.mfx.FrameInfo),
                        sizeof(mfxFrameInfo));
        if (SYSTEM_MEMORY != m_memType) {
            // external allocator used - provide just MemIds
            m_pEncSurfaces[i].Data.MemId = m_EncResponse.mids[i];
        }
        else {
            sts = m_pMFXAllocator->Lock(m_pMFXAllocator->pthis,
                                        m_EncResponse.mids[i],
                                        &(m_pEncSurfaces[i].Data));
            MSDK_CHECK_STATUS(sts, "m_pMFXAllocator->Lock failed");
        }
    }

    return MFX_ERR_NONE;
}

void CUserPipeline::DeleteFrames() {
    CEncodingPipeline::DeleteFrames();
}

CUserPipeline::CUserPipeline() : CEncodingPipeline() {
    m_MVCflags = MVC_DISABLED;
}

CUserPipeline::~CUserPipeline() {
    Close();
}

mfxStatus CUserPipeline::Init(sInputParams *pParams) {
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);

    mfxStatus sts = MFX_ERR_NONE;

    // prepare input file reader
    sts = m_FileReader.Init(pParams->InputFiles, pParams->FileInputFourCC);
    MSDK_CHECK_STATUS(sts, "m_FileReader.Init failed");

    // set memory type
    m_memType    = pParams->memType;
    m_nMemBuffer = pParams->nMemBuf;
    m_nTimeout   = pParams->nTimeout;
    m_bCutOutput = !pParams->bUncut;

    // prepare output file writer
    sts = InitFileWriters(pParams);
    MSDK_CHECK_STATUS(sts, "InitFileWriters failed");

    mfxIMPL impl = pParams->bUseHWLib ? MFX_IMPL_HARDWARE : MFX_IMPL_SOFTWARE;

    // if d3d11 surfaces are used ask the library to run acceleration through D3D11
    // feature may be unsupported due to OS or MSDK API version
    if (D3D11_MEMORY == pParams->memType)
        impl |= MFX_IMPL_VIA_D3D11;

    mfxVersion min_version;
    mfxVersion version; // real API version with which library is initialized

    // we set version to 1.0 and later we will query actual version of the library which will got leaded
    min_version.Major = 1;
    min_version.Minor = 0;

    // create a session for the second vpp and encode
    sts = m_mfxSession.Init(impl, &min_version);
    MSDK_CHECK_STATUS(sts, "m_mfxSession.Init failed");

    sts =
        MFXQueryVersion(m_mfxSession,
                        &version); // get real API version of the loaded library
    MSDK_CHECK_STATUS(sts, "MFXQueryVersion failed");

    // create encoder
    m_pmfxENC = new MFXVideoENCODE(m_mfxSession);
    MSDK_CHECK_POINTER(m_pmfxENC, MFX_ERR_MEMORY_ALLOC);

    sts = InitMfxEncParams(pParams);
    MSDK_CHECK_STATUS(sts, "InitMfxEncParams failed");

    // create and init frame allocator
    sts = CreateAllocator();
    MSDK_CHECK_STATUS(sts, "CreateAllocator failed");

    sts = ResetMFXComponents(pParams);
    MSDK_CHECK_STATUS(sts, "ResetMFXComponents failed");

    return MFX_ERR_NONE;
}

void CUserPipeline::Close() {
    CEncodingPipeline::Close();
}

mfxStatus CUserPipeline::ResetMFXComponents(sInputParams *pParams) {
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(m_pmfxENC, MFX_ERR_NOT_INITIALIZED);

    mfxStatus sts = MFX_ERR_NONE;

    sts = m_pmfxENC->Close();
    MSDK_IGNORE_MFX_STS(sts, MFX_ERR_NOT_INITIALIZED);
    MSDK_CHECK_STATUS(sts, "m_pmfxENC->Close failed");

    // free allocated frames
    DeleteFrames();

    m_TaskPool.Close();

    sts = AllocFrames();
    MSDK_CHECK_STATUS(sts, "AllocFrames failed");

    sts = m_pmfxENC->Init(&m_mfxEncParams);
    MSDK_CHECK_STATUS(sts, "m_pmfxENC->Init failed");

    mfxU32 nEncodedDataBufferSize = m_mfxEncParams.mfx.FrameInfo.Width *
                                    m_mfxEncParams.mfx.FrameInfo.Height * 4;
    sts = m_TaskPool.Init(&m_mfxSession,
                          m_FileWriters.first,
                          m_mfxEncParams.AsyncDepth,
                          nEncodedDataBufferSize,
                          m_FileWriters.second);
    MSDK_CHECK_STATUS(sts, "m_TaskPool.Init failed");

    sts = FillBuffers();
    MSDK_CHECK_STATUS(sts, "FillBuffers failed");

    return MFX_ERR_NONE;
}

mfxStatus CUserPipeline::Run() {
    m_statOverall.StartTimeMeasurement();
    MSDK_CHECK_POINTER(m_pmfxENC, MFX_ERR_NOT_INITIALIZED);

    mfxStatus sts = MFX_ERR_NONE;

    sTask *pCurrentTask = NULL; // a pointer to the current task
    mfxU16 nEncSurfIdx  = 0; // index of free surface for encoder input

    sts = MFX_ERR_NONE;

    // main loop, preprocessing and encoding
    while (MFX_ERR_NONE <= sts || MFX_ERR_MORE_DATA == sts) {
        // get a pointer to a free task (bit stream and sync point for encoder)
        sts = GetFreeTask(&pCurrentTask);
        MSDK_BREAK_ON_ERROR(sts);

        nEncSurfIdx =
            GetFreeSurface(m_pEncSurfaces, m_EncResponse.NumFrameActual);
        sts = LoadNextFrame(&m_pEncSurfaces[nEncSurfIdx]);
        MSDK_CHECK_ERROR(nEncSurfIdx,
                         MSDK_INVALID_SURF_IDX,
                         MFX_ERR_MEMORY_ALLOC);

        for (;;) {
            InsertIDR(m_bInsertIDR);
            sts          = m_pmfxENC->EncodeFrameAsync(&m_encCtrl,
                                              &m_pEncSurfaces[nEncSurfIdx],
                                              &pCurrentTask->mfxBS,
                                              &pCurrentTask->EncSyncP);
            m_bInsertIDR = false;

            if (MFX_ERR_NONE < sts &&
                !pCurrentTask
                     ->EncSyncP) // repeat the call if warning and no output
            {
                if (MFX_WRN_DEVICE_BUSY == sts)
                    MSDK_SLEEP(1); // wait if device is busy
            }
            else if (MFX_ERR_NONE < sts && pCurrentTask->EncSyncP) {
                sts = MFX_ERR_NONE; // ignore warnings if output is available
                break;
            }
            else if (MFX_ERR_NOT_ENOUGH_BUFFER == sts) {
                sts = AllocateSufficientBuffer(&pCurrentTask->mfxBS);
                MSDK_CHECK_STATUS(sts, "AllocateSufficientBuffer failed");
            }
            else {
                break;
            }
        }
    }

    // means that the input file has ended, need to go to buffering loops
    MSDK_IGNORE_MFX_STS(sts, MFX_ERR_MORE_DATA);
    // exit in case of other errors
    MSDK_CHECK_STATUS(sts, "m_pmfENC->EncodeFrameAsync failed");

    // rotate plugin doesn't buffer frames
    // loop to get buffered frames from encoder
    while (MFX_ERR_NONE <= sts) {
        // get a free task (bit stream and sync point for encoder)
        sts = GetFreeTask(&pCurrentTask);
        MSDK_BREAK_ON_ERROR(sts);

        for (;;) {
            InsertIDR(m_bInsertIDR);
            sts          = m_pmfxENC->EncodeFrameAsync(&m_encCtrl,
                                              NULL,
                                              &pCurrentTask->mfxBS,
                                              &pCurrentTask->EncSyncP);
            m_bInsertIDR = false;

            if (MFX_ERR_NONE < sts &&
                !pCurrentTask
                     ->EncSyncP) // repeat the call if warning and no output
            {
                if (MFX_WRN_DEVICE_BUSY == sts)
                    MSDK_SLEEP(1); // wait if device is busy
            }
            else if (MFX_ERR_NONE < sts && pCurrentTask->EncSyncP) {
                sts = MFX_ERR_NONE; // ignore warnings if output is available
                break;
            }
            else if (MFX_ERR_NOT_ENOUGH_BUFFER == sts) {
                sts = AllocateSufficientBuffer(&pCurrentTask->mfxBS);
                MSDK_CHECK_STATUS(sts, "AllocateSufficientBuffer failed");
            }
            else {
                break;
            }
        }
        MSDK_BREAK_ON_ERROR(sts);
    }

    // MFX_ERR_MORE_DATA is the correct status to exit buffering loop with
    // indicates that there are no more buffered frames
    MSDK_IGNORE_MFX_STS(sts, MFX_ERR_MORE_DATA);
    // exit in case of other errors
    MSDK_CHECK_STATUS(sts, "m_pmfxENC->EncodeFrameAsync failed");

    // synchronize all tasks that are left in task pool
    while (MFX_ERR_NONE == sts) {
        sts = m_TaskPool.SynchronizeFirstTask();
    }

    // MFX_ERR_NOT_FOUND is the correct status to exit the loop with,
    // EncodeFrameAsync and SyncOperation don't return this status
    MSDK_IGNORE_MFX_STS(sts, MFX_ERR_NOT_FOUND);
    // report any errors that occurred in asynchronous part
    MSDK_CHECK_STATUS(sts, "m_TaskPool.SynchronizeFirstTask failed");
    m_statOverall.StopTimeMeasurement();
    return sts;
}

mfxStatus CUserPipeline::FillBuffers() {
    return MFX_ERR_NONE;
}

void CUserPipeline::PrintInfo() {
    msdk_printf(MSDK_STRING("\nPipeline with rotation plugin"));
    msdk_printf(MSDK_STRING(
        "\nNOTE: Some of command line options may have been ignored as non-supported for this pipeline. For details see readme-encode.rtf.\n\n"));

    CEncodingPipeline::PrintInfo();
}
