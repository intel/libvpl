//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
// Example using Intel® Video Processing Library (Intel® VPL)
#include "./util.h"

#include "./hw-device.h"

#ifdef TOOLS_ENABLE_RENDER
#include "./render-frames.h"
#endif

#ifdef TOOLS_ENABLE_OPENCL
#include "./process-frames-ocl.h"
#endif

struct DecodeCtx {
    DevCtx *devCtx = nullptr;

#ifdef TOOLS_ENABLE_RENDER
    RenderCtx *rc = nullptr;
#endif

#ifdef TOOLS_ENABLE_OPENCL
    OpenCLCtx *oclCtx = nullptr;
#endif

    mfxU32 surfaceFlags = MFX_SURFACE_FLAG_EXPORT_SHARED;

    bool bEnableRender = false;
    bool bEnableOpenCL = false;
    bool bEnableTiming = false;
};

// main processing loop (decode + VPP)
static int ProcessStreamDecodeVPP(mfxSession session, FileInfo *fileInfo, DecodeCtx *decCtx, mfxU32 dbgMask) {
    bool bIsStillGoing  = true;
    bool bIsDrainingDec = false;
    bool bIsDrainingVPP = false;
    mfxU32 frameNum     = 0;

    mfxStatus sts = MFX_ERR_NONE;

#ifdef TOOLS_ENABLE_RENDER
    RenderCtx *rc = decCtx->rc;
#endif

#ifdef TOOLS_ENABLE_OPENCL
    OpenCLCtx *oclCtx = decCtx->oclCtx;
    cl_int clError    = CL_SUCCESS;
#endif

    if (decCtx->bEnableRender && decCtx->bEnableOpenCL)
        std::cout << "Rendering to display. Hit 'Q' or 'esc' to exit. Hit 'R' to toggle OpenCL processing...\n";
    else if (decCtx->bEnableRender)
        std::cout << "Rendering to display. Hit 'Q' or 'esc' to exit...\n";
    else
        std::cout << "Decoding to file. Hit 'Q' or 'esc' to exit...\n";

    VPL_TOTAL_TIME_INIT(totalStreamTime);
    VPL_TOTAL_TIME_INIT(totalExportTime);
    VPL_TOTAL_TIME_INIT(totalMapTime);

    VPL_TOTAL_TIME_START(totalStreamTime);

    // main processing loop
    while (bIsStillGoing == true) {
        mfxFrameSurface1 *pmfxDecOutSurface = NULL;

        sts = MFX_ERR_NONE;

        // fill input bitstream buffer
        if (bIsDrainingDec == false) {
            sts = ReadEncodedStream(fileInfo->bitstream, fileInfo->infile);
            if (sts != MFX_ERR_NONE)
                bIsDrainingDec = true; // end of input file, start draining decoder
        }

        // decode next frame
        mfxSyncPoint syncpD = {};
        if (bIsDrainingVPP == false) {
            sts = MFXVideoDECODE_DecodeFrameAsync(session, (bIsDrainingDec ? NULL : &fileInfo->bitstream), NULL, &pmfxDecOutSurface, &syncpD);

            if (MFX_ERR_NONE < sts && syncpD)
                sts = MFX_ERR_NONE; // ignore warnings if output is available

            if (DBG_MASK(dbgMask, DBG_MASK_NATIVE_SURFACE_DESC)) {
                sts = pmfxDecOutSurface->FrameInterface->Synchronize(pmfxDecOutSurface, 0xFFFFFFF);
                DebugDumpNativeSurfaceDesc(pmfxDecOutSurface);
            }
        }

        // run VPP on decoded frame
        if (sts == MFX_ERR_NONE) {
            mfxFrameSurface1 *pmfxVPPOutSurface = NULL;

            // send null ptr as input to drain VPP
            if (bIsDrainingVPP == true)
                pmfxDecOutSurface = NULL;

            // API >= 2.1, internal allocation of the output surface
            sts = MFXVideoVPP_ProcessFrameAsync(session, pmfxDecOutSurface, &pmfxVPPOutSurface);

            // avoid memory leak - decrease refcount for decOutSurface (we are done with it after passing into VPP_ProcessFrameAsync)
            if (pmfxDecOutSurface)
                pmfxDecOutSurface->FrameInterface->Release(pmfxDecOutSurface);

            if (sts == MFX_ERR_NONE) {
                do {
                    sts = pmfxVPPOutSurface->FrameInterface->Synchronize(pmfxVPPOutSurface, WAIT_100_MILLISECONDS);
                    VERIFY(((MFX_ERR_NONE == sts) || (MFX_WRN_IN_EXECUTION == sts)), "mfxFrameSurfaceInterface->Release failed");

                    if (MFX_ERR_NONE == sts) {
#ifdef TOOLS_ENABLE_RENDER
                        if (decCtx->bEnableRender) {
                            // during playback, 'F' key will toggle export mode between shared and copy
                            // run with -dbg 0x02 to print actual export mode used by RT
                            if (rc->bToggleSurfaceFlags) {
                                decCtx->surfaceFlags = (decCtx->surfaceFlags == MFX_SURFACE_FLAG_EXPORT_COPY ? MFX_SURFACE_FLAG_EXPORT_SHARED : MFX_SURFACE_FLAG_EXPORT_COPY);
                                std::cout << "INFO: Export SurfaceFlags is now set to " << DebugGetStringSurfaceFlags(decCtx->surfaceFlags) << std::endl;
                                rc->bToggleSurfaceFlags = false;
                            }

#ifdef TOOLS_ENABLE_OPENCL
                            if (decCtx->bEnableOpenCL && rc->bRotateOpenCL) {
                                cl_mem mem_Y  = nullptr;
                                cl_mem mem_UV = nullptr;

                                mfxExtSurfaceOpenCLImg2DExportDescription export_header_buf = {};
                                export_header_buf.Header.BufferId                           = MFX_EXTBUFF_EXPORT_SHARING_DESC_OCL;
                                export_header_buf.ocl_context                               = oclCtx->GetOpenCLContext();
                                export_header_buf.ocl_command_queue                         = oclCtx->GetOpenCLCommandQueue();
                                export_header_buf.Header.BufferSz                           = sizeof(mfxExtSurfaceOpenCLImg2DExportDescription);

                                mfxSurfaceHeader export_header = {};
                                export_header.SurfaceType      = MFX_SURFACE_TYPE_OPENCL_IMG2D;
                                export_header.SurfaceFlags     = decCtx->surfaceFlags;
                                export_header.NumExtParam      = 1;

                                std::vector<mfxExtBuffer *> extBufs;
                                extBufs.push_back((mfxExtBuffer *)&export_header_buf);
                                export_header.ExtParam = extBufs.data();

                                VPL_TOTAL_TIME_START(totalExportTime);

                                // get exported OCL surface which will be input to the OCL kernel
                                mfxSurfaceOpenCLImg2D *extSurfOCL = nullptr;
                                sts = pmfxVPPOutSurface->FrameInterface->Export(pmfxVPPOutSurface, export_header, (mfxSurfaceHeader **)&extSurfOCL);
                                VERIFY(MFX_ERR_NONE == sts, "ERROR: Export (OpenCL)");

                                VPL_TOTAL_TIME_STOP(totalExportTime);

                                // optional (debug) - print actual surface flags used by Export
                                if (DBG_MASK(dbgMask, DBG_MASK_ACTUAL_SURFACE_FLAGS))
                                    printf("[% 5d] Actual export mode = %s\n", frameNum, DebugGetStringSurfaceFlags(extSurfOCL->SurfaceInterface.Header.SurfaceFlags));

                                // lock the shared surface for OCL exclusive use (0 = Y plane, 1 = UV plane)
                                // this is the responsibility of the application - not performed by the runtime
                                clError = oclCtx->EnqueueAcquireSurfaces((cl_mem)extSurfOCL->ocl_image[0], (cl_mem)extSurfOCL->ocl_image[1]);
                                VERIFY(CL_SUCCESS == clError, "ERROR: EnqueueAcquireSurfaces");

                                // get offscreen D3D11 surface to contain the frame to render
                                ID3D11Texture2D *pD3D11RenderSurface = rc->GetRenderSurface();
                                VERIFY(pD3D11RenderSurface != nullptr, "ERROR: GetRenderSurface");

                                // Get writable OCL surface to be the the output from OCL kernel.
                                // By providing the D3D11 render surface as an input, this maps the render surface into an OCL texture,
                                //   rather than allocating a new one.
                                // Thus we can render the output of OCL processing directly to screen via the D3D rendering pipeline.
                                clError = oclCtx->GetOCLOutputSurface(&mem_Y, &mem_UV, true, pD3D11RenderSurface);
                                VERIFY(CL_SUCCESS == clError, "ERROR: GetOCLOutputSurface");

                                // run OpenCL kernels on exported surface
                                clError = oclCtx->OpenCLProcessSurface((cl_mem)extSurfOCL->ocl_image[0], (cl_mem)extSurfOCL->ocl_image[1], mem_Y, mem_UV);
                                VERIFY(CL_SUCCESS == clError, "ERROR: OpenCLProcessSurface");

                                // unlock the shared exported surface from OCL exclusive use
                                clError = oclCtx->EnqueueReleaseSurfaces((cl_mem)extSurfOCL->ocl_image[0], (cl_mem)extSurfOCL->ocl_image[1]);
                                VERIFY(CL_SUCCESS == clError, "ERROR: EnqueueReleaseSurfaces");

                                // release exported surface (OCL kernel processed into a separate output frame)
                                sts = extSurfOCL->SurfaceInterface.Release(&(extSurfOCL->SurfaceInterface));
                                VERIFY(MFX_ERR_NONE == sts, "ERROR: extSurfOCL->Release");

                                // release OCL output surface (done with OCL processing, it's now accessible to D3D11)
                                clError = oclCtx->ReleaseOCLSurface(mem_Y, mem_UV, true);
                                VERIFY(CL_SUCCESS == clError, "ERROR: ReleaseOCLSurface");

                                // create minimal surface structure just for passing the output of OCL (i.e. the mapped render surface) to the render stage
                                mfxSurfaceD3D11Tex2D extSurfRender                = {};
                                extSurfRender.SurfaceInterface.Header.SurfaceType = MFX_SURFACE_TYPE_D3D11_TEX2D;
                                extSurfRender.texture2D                           = pD3D11RenderSurface;

                                if (!decCtx->bEnableTiming) {
                                    // render the processed texture (output of OCL kernel)
                                    sts = rc->RenderFrame(&extSurfRender);
                                    VERIFY(MFX_ERR_NONE == sts, "ERROR: RenderFrame");
                                }
                            }
                            else
#endif
                            {
                                mfxSurfaceHeader export_header = {};
#ifdef _WIN32
                                export_header.SurfaceType     = MFX_SURFACE_TYPE_D3D11_TEX2D;
                                export_header.SurfaceFlags    = decCtx->surfaceFlags;
                                mfxSurfaceD3D11Tex2D *extSurf = nullptr;
#else
                                export_header.SurfaceType  = MFX_SURFACE_TYPE_VAAPI;
                                export_header.SurfaceFlags = decCtx->surfaceFlags;
                                mfxSurfaceVAAPI *extSurf   = nullptr;
#endif
                                VPL_TOTAL_TIME_START(totalExportTime);

                                // export mfxFrameSurface1 from Intel® VPL to a D3D11 or VAAPI surface
                                sts = pmfxVPPOutSurface->FrameInterface->Export(pmfxVPPOutSurface, export_header, (mfxSurfaceHeader **)&extSurf);
                                VERIFY(MFX_ERR_NONE == sts, "ERROR: Export");

                                VPL_TOTAL_TIME_STOP(totalExportTime);

                                // optional (debug) - print actual surface flags used by Export
                                if (DBG_MASK(dbgMask, DBG_MASK_ACTUAL_SURFACE_FLAGS))
                                    printf("[% 5d] Actual export mode = %s\n", frameNum, DebugGetStringSurfaceFlags(extSurf->SurfaceInterface.Header.SurfaceFlags));

                                if (!decCtx->bEnableTiming) {
                                    // render the original texture (output of VPP)
                                    sts = rc->RenderFrame(extSurf);
                                    VERIFY(MFX_ERR_NONE == sts, "ERROR: RenderFrame");
                                }

                                // release exported surface
                                sts = extSurf->SurfaceInterface.Release(&(extSurf->SurfaceInterface));
                                VERIFY(MFX_ERR_NONE == sts, "ERROR: extSurf->Release");
                            }
                        }
#endif

                        if (!decCtx->bEnableRender) {
#ifdef TOOLS_ENABLE_OPENCL
                            if (decCtx->bEnableOpenCL) {
                                cl_mem mem_Y  = nullptr;
                                cl_mem mem_UV = nullptr;

                                CPUFrameInfo_NV12 cpuFrameInfo;
                                std::vector<mfxU8> mem_ptr(pmfxVPPOutSurface->Info.Width * pmfxVPPOutSurface->Info.Height * 3);
                                cpuFrameInfo.height = pmfxVPPOutSurface->Info.Height;
                                cpuFrameInfo.width = cpuFrameInfo.pitch = pmfxVPPOutSurface->Info.Width;
                                cpuFrameInfo.Y                          = &mem_ptr[0];
                                cpuFrameInfo.UV                         = cpuFrameInfo.Y + cpuFrameInfo.height * cpuFrameInfo.width;

                                mfxExtSurfaceOpenCLImg2DExportDescription export_header_buf = {};
                                export_header_buf.Header.BufferId                           = MFX_EXTBUFF_EXPORT_SHARING_DESC_OCL;
                                export_header_buf.ocl_context                               = oclCtx->GetOpenCLContext();
                                export_header_buf.ocl_command_queue                         = oclCtx->GetOpenCLCommandQueue();
                                export_header_buf.Header.BufferSz                           = sizeof(mfxExtSurfaceOpenCLImg2DExportDescription);

                                mfxSurfaceHeader export_header = {};
                                export_header.SurfaceType      = MFX_SURFACE_TYPE_OPENCL_IMG2D;
                                export_header.SurfaceFlags     = decCtx->surfaceFlags;
                                export_header.NumExtParam      = 1;

                                std::vector<mfxExtBuffer *> extBufs;
                                extBufs.push_back((mfxExtBuffer *)&export_header_buf);
                                export_header.ExtParam = extBufs.data();

                                VPL_TOTAL_TIME_START(totalExportTime);

                                // export surface from Intel® VPL runtime (output of VPP) as an OCL surface (input to OCL kernel)
                                mfxSurfaceOpenCLImg2D *extSurfOCL = nullptr;
                                sts = pmfxVPPOutSurface->FrameInterface->Export(pmfxVPPOutSurface, export_header, (mfxSurfaceHeader **)&extSurfOCL);
                                VERIFY(MFX_ERR_NONE == sts, "ERROR: Export (OpenCL)");

                                VPL_TOTAL_TIME_STOP(totalExportTime);

                                // get writable OCL surface to be the the output from OCL kernel
                                clError = oclCtx->GetOCLOutputSurface(&mem_Y, &mem_UV);
                                VERIFY(CL_SUCCESS == clError, "ERROR: GetOCLOutputSurface");

                                // run OpenCL kernels on exported surface
                                clError = oclCtx->OpenCLProcessSurface((cl_mem)extSurfOCL->ocl_image[0], (cl_mem)extSurfOCL->ocl_image[1], mem_Y, mem_UV);
                                VERIFY(CL_SUCCESS == clError, "ERROR: OpenCLProcessSurface");

                                // release exported surface
                                sts = extSurfOCL->SurfaceInterface.Release(&(extSurfOCL->SurfaceInterface));
                                VERIFY(MFX_ERR_NONE == sts, "ERROR: extSurfOCL->Release");

                                // copy to system memory for saving
                                clError = oclCtx->CopySurfaceOCLToSystem(mem_Y, mem_UV, &cpuFrameInfo);
                                VERIFY(CL_SUCCESS == clError, "ERROR: CopySurfaceOCLToSystem");

                                // release OCL output surface (it's been copied to sysmem buffer)
                                clError = oclCtx->ReleaseOCLSurface(mem_Y, mem_UV);
                                VERIFY(CL_SUCCESS == clError, "ERROR: ReleaseOCLSurface");

                                // save to outfile
                                if (!decCtx->bEnableTiming && fileInfo->outfile.is_open()) {
                                    sts = WriteRawFrameCPU_NV12(cpuFrameInfo, fileInfo->outfile);
                                    VERIFY(MFX_ERR_NONE == sts, "ERROR: WriteRawFrameCPU_NV12");
                                }
                            }
                            else
#endif
                            {
                                VPL_TOTAL_TIME_START(totalMapTime);

                                pmfxVPPOutSurface->FrameInterface->Map(pmfxVPPOutSurface, MFX_MAP_READ);
                                VERIFY(MFX_ERR_NONE == sts, "mfxFrameSurfaceInterface->Map failed");

                                VPL_TOTAL_TIME_STOP(totalMapTime);

                                if (!decCtx->bEnableTiming && fileInfo->outfile.is_open()) {
                                    sts = WriteRawFrame(pmfxVPPOutSurface, fileInfo->outfile);
                                    VERIFY(MFX_ERR_NONE == sts, "Could not write decoded output");
                                }

                                sts = pmfxVPPOutSurface->FrameInterface->Unmap(pmfxVPPOutSurface);
                                VERIFY(MFX_ERR_NONE == sts, "mfxFrameSurfaceInterface->Unmap failed");
                            }
                        }

                        // avoid memory leak - decrease refcount for VPPOutSurface
                        sts = pmfxVPPOutSurface->FrameInterface->Release(pmfxVPPOutSurface);
                        VERIFY(MFX_ERR_NONE == sts, "mfxFrameSurfaceInterface->Release failed");

                        frameNum++;
                    }
                } while (sts == MFX_WRN_IN_EXECUTION);
            }
            else if (sts == MFX_ERR_MORE_DATA) {
                // no more input frames available to send to VPP
                if (bIsDrainingVPP == true)
                    bIsStillGoing = false;
            }
            else if (sts < 0) {
                // unknown error from VPP
                bIsStillGoing = false;
            }
        }
        else if (sts == MFX_ERR_MORE_DATA) {
            // decoder needs more data - if already at EOF then there are no more frames to decode, so start VPP draining
            if (bIsDrainingDec)
                bIsDrainingVPP = true;
        }
        else if (sts < 0) {
            // unknown error from decode
            bIsStillGoing = false;
        }

#ifdef TOOLS_ENABLE_RENDER
        // check message queue (if rendering) or console (if not rendering) for keystrokes
        // break by typing 'Q' or 'esc'
        if (decCtx->bEnableRender) {
            rc->ProcessMessages();
            if (rc->bQuit)
                bIsStillGoing = false;
        }
#endif

        // check console for keystrokes
        // break by typing 'Q' or 'esc'
        if (!decCtx->bEnableRender) {
            if (CheckKB_Quit())
                bIsStillGoing = false;
        }
    }

    VPL_TOTAL_TIME_STOP(totalStreamTime);

    std::cout << "Decoded and processed " << frameNum << " frames\n\n";

    if (decCtx->bEnableTiming) {
        // print timing for overall stream processing
        mfxF32 fStreamTime;
        mfxU64 uStreamCount;
        VPL_TOTAL_TIME_CHECK(totalStreamTime, fStreamTime, uStreamCount);
        if (uStreamCount)
            printf("Stream time:  FPS = % 8.2f   TOTAL = % 9.3f msec   NUM FRAMES = % 5d\n\n", frameNum * 1000.0f / fStreamTime, fStreamTime, frameNum);

        // print timing for export operations
        mfxF32 fExportTime;
        mfxU64 uExportCount;
        VPL_TOTAL_TIME_CHECK(totalExportTime, fExportTime, uExportCount);
        if (uExportCount)
            printf("Export time:  COUNT = % 6lld   TOTAL = % 9.3f msec   PER FRAME = % 7.3f msec\n", uExportCount, fExportTime, fExportTime / uExportCount);

        // print timing for map to system memory
        mfxF32 fMapTime;
        mfxU64 uMapCount;
        VPL_TOTAL_TIME_CHECK(totalMapTime, fMapTime, uMapCount);
        if (uMapCount)
            printf("Map time:     COUNT = % 6lld   TOTAL = % 9.3f msec   PER FRAME = % 7.3f msec\n", uMapCount, fMapTime, fMapTime / uMapCount);
    }

    return 0;
}

int RunDecodeVPP(Params *params, FileInfo *fileInfo) {
    mfxStatus sts = MFX_ERR_NONE;

    // validate configuration
#ifndef TOOLS_ENABLE_RENDER
    if (params->testMode == TEST_MODE_RENDER)
        VERIFY(0, "ERROR: rebuild with TOOLS_ENABLE_RENDER set to ON\n");
#endif

#ifndef TOOLS_ENABLE_OPENCL
    if (params->bEnableOpenCL)
        VERIFY(0, "ERROR: rebuild with TOOLS_ENABLE_OPENCL set to ON\n");
#endif

    // initialize decode context
    DecodeCtx decCtx     = {};
    decCtx.bEnableTiming = params->bEnableTiming;

    if (params->surfaceMode == SURFACE_MODE_SHARED)
        decCtx.surfaceFlags = MFX_SURFACE_FLAG_EXPORT_SHARED;
    else if (params->surfaceMode == SURFACE_MODE_COPY)
        decCtx.surfaceFlags = MFX_SURFACE_FLAG_EXPORT_COPY;

    // create HW device context
    // automatically released when devCtx goes out of scope (make sure this happens after closing session)
    DevCtx devCtx            = {};
    mfxHandleType handleType = {};
    mfxHDL handle            = nullptr;

    sts = devCtx.InitDevice(0, &handleType, &handle);
    VERIFY((MFX_ERR_NONE == sts) && (handle != nullptr), "ERROR: InitDevice");
    decCtx.devCtx = &devCtx;

    // specify required capabilities for session creation
    std::list<SurfaceCaps> surfaceCapsList;

    // VPP output - native type
    SurfaceCaps scVPP = {};
    scVPP.SurfaceType =
#ifdef _WIN32
        MFX_SURFACE_TYPE_D3D11_TEX2D;
#else
        MFX_SURFACE_TYPE_VAAPI;
#endif
    scVPP.SurfaceComponent = MFX_SURFACE_COMPONENT_VPP_OUTPUT;
    scVPP.SurfaceFlags     = decCtx.surfaceFlags;
    surfaceCapsList.push_back(scVPP);

    // VPP output - OpenCL (if enabled)
    if (params->bEnableOpenCL) {
        SurfaceCaps scOCL      = {};
        scOCL.SurfaceType      = MFX_SURFACE_TYPE_OPENCL_IMG2D;
        scOCL.SurfaceComponent = MFX_SURFACE_COMPONENT_VPP_OUTPUT;
        scOCL.SurfaceFlags     = decCtx.surfaceFlags;
        surfaceCapsList.push_back(scOCL);
    }

    // initialize session
    VPLSession vplSession = {};
    sts                   = vplSession.Open(&surfaceCapsList);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: unable to create session");

    // pass device handle to runtime
    sts = MFXVideoCORE_SetHandle(vplSession.GetSession(), handleType, handle);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: SetHandle");

#ifdef TOOLS_ENABLE_RENDER
    // initialize render device context
    RenderCtx rc = {};
    if (params->testMode == TEST_MODE_RENDER) {
        sts = rc.RenderInit(&devCtx, params->dstWidth, params->dstHeight, params->infileName);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing renderer");
        decCtx.bEnableRender = true;
    }
    decCtx.rc = &rc;
#endif

#ifdef TOOLS_ENABLE_OPENCL
    // initialize OpenCL post-processing
    OpenCLCtx oclCtx = {};
    if (params->bEnableOpenCL && !params->openCLfileName.empty()) {
        sts = oclCtx.OpenCLInit(handleType, handle, params->dstWidth, params->dstHeight, params->openCLfileName);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing OpenCL");
        decCtx.bEnableOpenCL = true;
    }
    decCtx.oclCtx = &oclCtx;
#endif

    // init input bitstream
    std::vector<mfxU8 *> inbuf(BITSTREAM_BUFFER_SIZE);
    fileInfo->bitstream.MaxLength = static_cast<mfxU32>(inbuf.size());
    fileInfo->bitstream.Data      = reinterpret_cast<mfxU8 *>(inbuf.data());
    VERIFY(fileInfo->bitstream.Data, "ERROR: not able to allocate input buffer");
    fileInfo->bitstream.CodecId = MFX_CODEC_HEVC;

    // read first part of bitstream to parse header
    sts = ReadEncodedStream(fileInfo->bitstream, fileInfo->infile);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: reading bitstream");

    // retrieve the frame information from input stream
    mfxVideoParam mfxDecParams = {};
    mfxDecParams.mfx.CodecId   = MFX_CODEC_HEVC;
    mfxDecParams.IOPattern     = MFX_IOPATTERN_OUT_VIDEO_MEMORY;
    mfxDecParams.AsyncDepth    = 1;

    sts = MFXVideoDECODE_DecodeHeader(vplSession.GetSession(), &fileInfo->bitstream, &mfxDecParams);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: decoding header");

    // init VPP parameters
    mfxVideoParam mfxVPPParams         = {};
    mfxVPPParams.vpp.In.FourCC         = mfxDecParams.mfx.FrameInfo.FourCC;
    mfxVPPParams.vpp.In.ChromaFormat   = mfxDecParams.mfx.FrameInfo.ChromaFormat;
    mfxVPPParams.vpp.In.Width          = mfxDecParams.mfx.FrameInfo.Width;
    mfxVPPParams.vpp.In.Height         = mfxDecParams.mfx.FrameInfo.Height;
    mfxVPPParams.vpp.In.CropW          = mfxDecParams.mfx.FrameInfo.Width;
    mfxVPPParams.vpp.In.CropH          = mfxDecParams.mfx.FrameInfo.Height;
    mfxVPPParams.vpp.In.PicStruct      = MFX_PICSTRUCT_PROGRESSIVE;
    mfxVPPParams.vpp.In.FrameRateExtN  = 30;
    mfxVPPParams.vpp.In.FrameRateExtD  = 1;
    mfxVPPParams.vpp.Out.FourCC        = params->outFourCC;
    mfxVPPParams.vpp.Out.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxVPPParams.vpp.Out.Width         = ALIGN16(params->dstWidth);
    mfxVPPParams.vpp.Out.Height        = ALIGN16(params->dstHeight);
    mfxVPPParams.vpp.Out.CropW         = params->dstWidth;
    mfxVPPParams.vpp.Out.CropH         = params->dstHeight;
    mfxVPPParams.vpp.Out.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    mfxVPPParams.vpp.Out.FrameRateExtN = 30;
    mfxVPPParams.vpp.Out.FrameRateExtD = 1;

    mfxVPPParams.IOPattern  = MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;
    mfxVPPParams.AsyncDepth = 1;

    // init decode
    sts = MFXVideoDECODE_Init(vplSession.GetSession(), &mfxDecParams);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing Decode");

    // init vpp
    sts = MFXVideoVPP_Init(vplSession.GetSession(), &mfxVPPParams);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing VPP");

    std::cout << "Running: Decode (H265) + VPP (resize)" << std::endl;
    std::cout << "  infile  = " << params->infileName << std::endl;
    std::cout << "  outfile = " << (params->outfileName.empty() ? "none" : params->outfileName) << std::endl;
    std::cout << "  original resolution = " << mfxDecParams.mfx.FrameInfo.Width << "x" << mfxDecParams.mfx.FrameInfo.Height << std::endl;
    std::cout << "  output resolution   = " << params->dstWidth << "x" << params->dstHeight << std::endl;
    std::cout << "  output colorspace   = " << FourCCToString(params->outFourCC) << std::endl;
    std::cout << std::endl << std::endl;

    // run main processing loop
    int err = ProcessStreamDecodeVPP(vplSession.GetSession(), fileInfo, &decCtx, params->dbgMask);

    if (err) {
        std::cout << "ERROR: ProcessStream() returned " << err << std::endl;
        return err;
    }

    // dtor will call Close() automatically when it goes out of scope (e.g. error causes early exit).
    // But it's also okay to call it explicitly here - it won't try to teardown twice.
    vplSession.Close();

    return 0;
}
