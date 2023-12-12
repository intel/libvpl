//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./vpp.h" //NOLINT
#include <algorithm>
#include "./hw-device.h"
#include "./util.h"

CVPPTest::CVPPTest()
        : m_tIndex(0),
          m_loader(nullptr),
          m_session(nullptr),
          m_pDevCtx(nullptr),
#ifdef TOOLS_ENABLE_OPENCL
          m_pOclCtx(nullptr),
#endif
          m_vppParams(),
          m_pOpts(nullptr),
          m_frameInfo(),
          m_fileInfo(),
          m_memoryInterface(nullptr) {
}

CVPPTest::~CVPPTest() {
    if (m_session)
        MFXClose(m_session);
    m_session = nullptr;

    if (m_loader)
        MFXUnload(m_loader);
    m_loader = nullptr;

    if (m_pDevCtx)
        delete m_pDevCtx;
    m_pDevCtx = nullptr;

#ifdef TOOLS_ENABLE_OPENCL
    if (m_pOclCtx)
        delete m_pOclCtx;
    m_pOclCtx = nullptr;
#endif
}

mfxStatus CVPPTest::CreateVPLSession() {
    mfxStatus sts = MFX_ERR_NONE;

    // variables used only in 2.x version
    mfxConfig cfg[5] = {};
    mfxVariant cfgVal;

    //-- Create m_session
    m_loader = MFXLoad();
    VERIFY(nullptr != m_loader,
           "ERROR: MFXLoad failed -- is implementation in path?",
           MFX_ERR_NULL_PTR);

    // Implementation used must be the hardware implementation
    cfg[0]          = MFXCreateConfig(m_loader);
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = MFX_IMPL_TYPE_HARDWARE;
    sts = MFXSetConfigFilterProperty(cfg[0], (mfxU8 *)"mfxImplDescription.Impl", cfgVal);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for Impl", sts);

    // Implementation used must be d3d11 acceleration mode
    cfg[1]      = MFXCreateConfig(m_loader);
    cfgVal.Type = MFX_VARIANT_TYPE_U32;
#ifdef _WIN32
    cfgVal.Data.U32 = MFX_ACCEL_MODE_VIA_D3D11;
#else
    cfgVal.Data.U32 = MFX_ACCEL_MODE_VIA_VAAPI;
#endif
    sts =
        MFXSetConfigFilterProperty(cfg[1], (mfxU8 *)"mfxImplDescription.AccelerationMode", cfgVal);
    VERIFY(MFX_ERR_NONE == sts,
           "ERROR: MFXSetConfigFilterProperty failed for acceleration mode",
           sts);

    // Implementation used must have VPP scaling capability
    cfg[2]          = MFXCreateConfig(m_loader);
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = MFX_EXTBUFF_VPP_SCALING;
    sts             = MFXSetConfigFilterProperty(
        cfg[2],
        (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC",
        cfgVal);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for VPP scale", sts);

    // Implementation used must have VPP scaling capability
    cfg[3]          = MFXCreateConfig(m_loader);
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = MFX_EXTBUFF_VPP_COLOR_CONVERSION;
    sts             = MFXSetConfigFilterProperty(
        cfg[3],
        (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC",
        cfgVal);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for VPP csc", sts);

    // Implementation used must provide API version 2.10 or newer
    cfg[4]          = MFXCreateConfig(m_loader);
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = VPLVERSION(MAJOR_API_VERSION_REQUIRED, MINOR_API_VERSION_REQUIRED);
    sts             = MFXSetConfigFilterProperty(cfg[4],
                                     (mfxU8 *)"mfxImplDescription.ApiVersion.Version",
                                     cfgVal);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for API version", sts);

    if (m_pOpts->bTestMultiSession == true) {
        mfxVariant cfgVal;
        mfxConfig cfg   = MFXCreateConfig(m_loader);
        cfgVal.Type     = MFX_VARIANT_TYPE_U32;
        cfgVal.Data.U32 = m_pOpts->adapterToRun;
#ifdef _WIN32
        sts = MFXSetConfigFilterProperty(cfg, (mfxU8 *)"DXGIAdapterIndex", cfgVal);
#else
        sts = MFXSetConfigFilterProperty(cfg,
                                         (mfxU8 *)"mfxExtendedDeviceId.DRMRenderNodeNum",
                                         cfgVal);
#endif
        VERIFY(MFX_ERR_NONE == sts,
               "ERROR: MFXSetConfigFilterProperty failed for adapter index",
               sts);
    }

    // Get runtime path
    mfxHDL descPath;
    sts = MFXEnumImplementations(m_loader, 0, MFX_IMPLCAPS_IMPLPATH, &descPath);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXEnumImplementations failed for implpath", sts);
    MFXDispReleaseImplDescription(m_loader, descPath);

    sts = MFXCreateSession(m_loader, 0, &m_session);
    VERIFY(MFX_ERR_NONE == sts,
           "ERROR: cannot create m_session -- no implementations meet selection criteria",
           sts);

    printf("Created m_session with library: %s\n", (char *)descPath);

    return MFX_ERR_NONE;
}

mfxStatus CVPPTest::Init(int tIndex, Options *opts, std::vector<mfxU32> *adapterNumbers) {
    mfxStatus sts = MFX_ERR_NONE;
    m_pOpts       = opts;
    m_tIndex      = tIndex;

    //-- Open input file
    if (!m_pOpts->infileName.empty()) {
        m_fileInfo.infile.open(m_pOpts->infileName.c_str(), std::ios::binary);
        VERIFY(m_fileInfo.infile, "ERROR: could not open input file", MFX_ERR_NOT_FOUND);
    }

    //-- Open output file
    if (!m_pOpts->outfileName.empty()) {
        m_fileInfo.outfile.open(m_pOpts->outfileName.c_str(), std::ios::binary);
        VERIFY(m_fileInfo.outfile, "ERROR: could not open output file", MFX_ERR_NOT_FOUND);
    }

    sts = CreateVPLSession();
    VERIFY(MFX_ERR_NONE == sts, "ERROR: CreateVPLSession", sts);

    // create HW device - automatically released when devCtx goes out of scope
    mfxHandleType handleType = {};
    mfxHDL handle            = nullptr;

    m_pDevCtx = new DevCtx;
    sts       = m_pDevCtx->InitDevice(0, &handleType, &handle);
    VERIFY((MFX_ERR_NONE == sts) && (handle != nullptr), "ERROR: InitDevice", sts);

    // pass device handle to runtime
    sts = MFXVideoCORE_SetHandle(m_session, handleType, handle);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: SetHandle", sts);

    m_vppParams.vpp.In.FourCC = m_pOpts->inFourCC;
    m_vppParams.vpp.In.ChromaFormat =
        (m_pOpts->inFourCC == MFX_FOURCC_NV12) ? MFX_CHROMAFORMAT_YUV420 : MFX_CHROMAFORMAT_YUV444;
    m_vppParams.vpp.In.CropW         = m_pOpts->srcWidth;
    m_vppParams.vpp.In.CropH         = m_pOpts->srcHeight;
    m_vppParams.vpp.In.Width         = ALIGN16(m_vppParams.mfx.FrameInfo.CropW);
    m_vppParams.vpp.In.Height        = ALIGN16(m_vppParams.mfx.FrameInfo.CropH);
    m_vppParams.vpp.In.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    m_vppParams.vpp.In.FrameRateExtN = 30;
    m_vppParams.vpp.In.FrameRateExtD = 1;
    m_vppParams.vpp.Out.FourCC       = m_pOpts->outFourCC;
    m_vppParams.vpp.Out.ChromaFormat =
        (m_pOpts->outFourCC == MFX_FOURCC_NV12) ? MFX_CHROMAFORMAT_YUV420 : MFX_CHROMAFORMAT_YUV444;
    m_vppParams.vpp.Out.Width         = ALIGN16(m_pOpts->dstWidth);
    m_vppParams.vpp.Out.Height        = ALIGN16(m_pOpts->dstHeight);
    m_vppParams.vpp.Out.CropW         = m_pOpts->dstWidth;
    m_vppParams.vpp.Out.CropH         = m_pOpts->dstHeight;
    m_vppParams.vpp.Out.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    m_vppParams.vpp.Out.FrameRateExtN = 30;
    m_vppParams.vpp.Out.FrameRateExtD = 1;

    m_vppParams.IOPattern  = MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;
    m_vppParams.AsyncDepth = 1;

    sts = MFXVideoVPP_Init(m_session, &m_vppParams);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing VPP", sts);

    mfxMemoryInterface *iface = nullptr;
    sts                       = MFXGetMemoryInterface(m_session, &iface);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXGetMemoryInterface", sts);
    m_memoryInterface = iface;

    m_frameInfo.fourcc = m_pOpts->inFourCC;
    m_frameInfo.width  = m_pOpts->srcWidth;
    m_frameInfo.height = m_pOpts->srcHeight;
    m_frameInfo.pitch = (m_pOpts->inFourCC == MFX_FOURCC_RGB4) ? m_vppParams.mfx.FrameInfo.Width * 4
                                                               : m_vppParams.mfx.FrameInfo.Width;

#ifdef TOOLS_ENABLE_OPENCL
    if (m_pOpts->surfaceType == MFX_SURFACE_TYPE_OPENCL_IMG2D) {
        m_pOclCtx = new OpenCLCtx;
        sts       = m_pOclCtx->OpenCLInit(handleType, handle);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing OpenCL", sts);

        m_pOclCtx->SetOCLSurfaceFrameInfo(&m_frameInfo);

        if (m_pOpts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_OUTPUT) {
            FrameInfo frameInfoOut = {};
            frameInfoOut.width     = m_pOpts->dstWidth;
            frameInfoOut.height    = m_pOpts->dstHeight;
            frameInfoOut.fourcc    = m_pOpts->outFourCC;

            sts = m_pOclCtx->AllocCpuFrameForExport(&frameInfoOut);
            VERIFY(MFX_ERR_NONE == sts, "ERROR: AllocCpuFrame", sts);
        }
    }
#endif
    return MFX_ERR_NONE;
}

mfxStatus CVPPTest::ProcessStreamVPP() {
    bool isStillGoing = true;
    bool isDraining   = false;
    mfxU32 frameNum   = 0;
    mfxU32 surfaceSize;
    mfxStatus sts = MFX_ERR_NONE;

    if (m_pOpts->inFourCC == MFX_FOURCC_NV12)
        surfaceSize = m_frameInfo.width * m_frameInfo.height * 3 / 2;
    else { // rgb4
        surfaceSize = m_frameInfo.width * m_frameInfo.height * 4;
    }

    std::vector<mfxU8> bsInput(surfaceSize, 0);

#ifdef __linux__
    VASurfaceID vaSurfaceID;
#endif

    while (isStillGoing == true) {
        mfxFrameSurface1 *pmfxVPPInSurface  = nullptr;
        mfxFrameSurface1 *pmfxVPPOutSurface = nullptr;
        if (isDraining == false) {
            if (m_pOpts->bNoSSA == true ||
                m_pOpts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_OUTPUT) {
                sts = MFXMemory_GetSurfaceForVPPIn(m_session, &pmfxVPPInSurface);
                VERIFY(MFX_ERR_NONE == sts, "Could not get vpp surface", sts);

                sts = ReadRawFrame(pmfxVPPInSurface, m_fileInfo.infile);
                if (sts != MFX_ERR_NONE)
                    isDraining = true;
            }
            else { // m_pOpts->surfaceComponent is MFX_SURFACE_VPP_INPUT
                sts = ReadRawFrame(m_frameInfo, bsInput.data(), m_fileInfo.infile);
                if (sts != MFX_ERR_NONE)
                    isDraining = true;
                else {
#ifdef _WIN32
                    if (m_pOpts->surfaceType == MFX_SURFACE_TYPE_D3D11_TEX2D) {
                        mfxSurfaceD3D11Tex2D surfaceForImport = {};
                        surfaceForImport.SurfaceInterface.Header.SurfaceFlags =
                            m_pOpts->surfaceFlag;
                        surfaceForImport.SurfaceInterface.Header.SurfaceType = m_pOpts->surfaceType;
                        surfaceForImport.SurfaceInterface.Header.StructSize =
                            sizeof(mfxSurfaceD3D11Tex2D);

                        // BindFlag for vpp texture2d should be 'render_target'
                        CComPtr<ID3D11Texture2D> tex2D =
                            m_pDevCtx->CreateSurfaceToShare(&m_frameInfo,
                                                            bsInput.data(),
                                                            true /* brender */);
                        VERIFY(nullptr != tex2D,
                               "ERROR: CreateSurfaceToShare",
                               MFX_ERR_DEVICE_FAILED);

                        surfaceForImport.texture2D = tex2D;

                        sts = m_memoryInterface->ImportFrameSurface(
                            m_memoryInterface,
                            MFX_SURFACE_COMPONENT_VPP_INPUT,
                            &surfaceForImport.SurfaceInterface.Header,
                            &pmfxVPPInSurface);
                        VERIFY(MFX_ERR_NONE == sts, "ERROR: ImportFrameSurface", sts);
                    }
                    else if (m_pOpts->surfaceType == MFX_SURFACE_TYPE_OPENCL_IMG2D) {
    #ifdef TOOLS_ENABLE_OPENCL
                        CPUFrame cpuFrame = {};
                        if (m_frameInfo.fourcc == MFX_FOURCC_NV12) {
                            cpuFrame.Y  = bsInput.data();
                            cpuFrame.UV = bsInput.data() + (m_frameInfo.width * m_frameInfo.height);
                        }
                        else if (m_frameInfo.fourcc == MFX_FOURCC_RGB4) {
                            cpuFrame.BGRA = bsInput.data();
                        }

                        cl_mem clData[2] = {}; // bgra -> [0]: BGRA, nv12 -> [0]: Y, [1]: UV

                        // get writable OCL surface to be the the input from OCL kernel
                        sts = m_pOclCtx->GetOCLInputSurface(clData);
                        VERIFY(MFX_ERR_NONE == sts, "ERROR: GetOCLInputSurface", sts);

                        // copy to system memory for saving
                        sts = m_pOclCtx->CopySurfaceSystemToOCL(&cpuFrame, clData);
                        VERIFY(MFX_ERR_NONE == sts, "ERROR: CopySurfaceSystemToOCL", sts);

                        mfxSurfaceOpenCLImg2D surfaceForImport = {};
                        surfaceForImport.SurfaceInterface.Header.SurfaceFlags =
                            m_pOpts->surfaceFlag;
                        surfaceForImport.SurfaceInterface.Header.SurfaceType = m_pOpts->surfaceType;
                        surfaceForImport.SurfaceInterface.Header.StructSize =
                            sizeof(mfxSurfaceOpenCLImg2D);

                        surfaceForImport.ocl_context       = m_pOclCtx->GetOpenCLContext();
                        surfaceForImport.ocl_command_queue = m_pOclCtx->GetOpenCLCommandQueue();

                        surfaceForImport.ocl_image_num =
                            m_frameInfo.fourcc == MFX_FOURCC_NV12 ? 2 : 1;
                        for (mfxU32 i = 0; i < surfaceForImport.ocl_image_num; i++)
                            surfaceForImport.ocl_image[i] = clData[i];

                        sts = m_memoryInterface->ImportFrameSurface(
                            m_memoryInterface,
                            MFX_SURFACE_COMPONENT_VPP_INPUT,
                            &surfaceForImport.SurfaceInterface.Header,
                            &pmfxVPPInSurface);
                        VERIFY(MFX_ERR_NONE == sts, "ERROR: ImportFrameSurface", sts);

                        // release OCL surfaces
                        sts = m_pOclCtx->ReleaseOCLSurface(clData);
                        VERIFY(MFX_ERR_NONE == sts, "ERROR: ReleaseOCLSurface", sts);
    #endif
                    }
#else
                    mfxSurfaceVAAPI surfaceForImport                      = {};
                    surfaceForImport.SurfaceInterface.Header.SurfaceFlags = m_pOpts->surfaceFlag;
                    surfaceForImport.SurfaceInterface.Header.SurfaceType  = m_pOpts->surfaceType;
                    surfaceForImport.SurfaceInterface.Header.StructSize   = sizeof(mfxSurfaceVAAPI);

                    surfaceForImport.vaDisplay   = m_pDevCtx->GetVADisplay();
                    surfaceForImport.vaSurfaceID = vaSurfaceID =
                        m_pDevCtx->CreateSurfaceToShare(&m_frameInfo, bsInput.data());
                    VERIFY(surfaceForImport.vaSurfaceID != VA_INVALID_SURFACE,
                           "ERROR: CreateSurfaceToShare",
                           MFX_ERR_DEVICE_FAILED);

                    sts = m_memoryInterface->ImportFrameSurface(
                        m_memoryInterface,
                        MFX_SURFACE_COMPONENT_VPP_INPUT,
                        &surfaceForImport.SurfaceInterface.Header,
                        &pmfxVPPInSurface);
                    VERIFY(MFX_ERR_NONE == sts, "ERROR: ImportFrameSurface", sts);
#endif
                }
            }
        }

        sts = MFXMemory_GetSurfaceForVPPOut(m_session, &pmfxVPPOutSurface);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXMemory_GetSurfaceForVPPOut", sts);

        // vpp next frame
        mfxSyncPoint syncp = {};
        sts                = MFXVideoVPP_RunFrameVPPAsync(m_session,
                                           (isDraining ? nullptr : pmfxVPPInSurface),
                                           pmfxVPPOutSurface,
                                           nullptr,
                                           &syncp);

        if (!isDraining) {
            sts = pmfxVPPInSurface->FrameInterface->Release(pmfxVPPInSurface);
            VERIFY(sts == MFX_ERR_NONE, "Could not release vpp input surface", sts);
        }

        switch (sts) {
            case MFX_ERR_NONE:
                if (syncp) {
                    do {
                        sts = MFXVideoCORE_SyncOperation(m_session, syncp, WAIT_100_MILLISECONDS);
                        if (m_pOpts->bNoSSA == true ||
                            m_pOpts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_INPUT) {
                            if (MFX_ERR_NONE == sts) {
                                sts = WriteRawFrame(pmfxVPPOutSurface, m_fileInfo.outfile);
                                VERIFY(MFX_ERR_NONE == sts, "Could not write vpp output", sts);
                                frameNum++;
                            }
                        }
                        else { // m_pOpts->surfaceComponent is MFX_SURFACE_COMPONENT_VPP_OUTPUT
                            if (MFX_ERR_NONE == sts) {
                                mfxSurfaceHeader descOfExport = {};

                                // set header, all other fields should be empty
                                descOfExport.SurfaceType  = m_pOpts->surfaceType;
                                descOfExport.SurfaceFlags = m_pOpts->surfaceFlag;

                                // export mfxFrameSurface1 to a D3D11 texture
                                mfxSurfaceHeader *exportedSurfaceGeneral = nullptr;
                                mfxStatus stsSurf = pmfxVPPOutSurface->FrameInterface->Export(
                                    pmfxVPPOutSurface,
                                    descOfExport,
                                    &exportedSurfaceGeneral);
                                VERIFY(MFX_ERR_NONE == stsSurf,
                                       "ERROR: MFXMemory_ExportFrameSurface",
                                       stsSurf);
#ifdef _WIN32
                                if (m_pOpts->surfaceType == MFX_SURFACE_TYPE_D3D11_TEX2D) {
                                    mfxSurfaceD3D11Tex2D *exportedSurface =
                                        reinterpret_cast<mfxSurfaceD3D11Tex2D *>(
                                            exportedSurfaceGeneral);

                                    stsSurf = WriteRawFrame(
                                        m_pDevCtx->GetDeviceHandle(),
                                        (ID3D11Texture2D *)(exportedSurface->texture2D),
                                        m_fileInfo.outfile);
                                    VERIFY(MFX_ERR_NONE == stsSurf,
                                           "ERROR: WriteRawFrame",
                                           stsSurf);
                                    stsSurf = exportedSurface->SurfaceInterface.Release(
                                        &(exportedSurface->SurfaceInterface));
                                }
                                else if (m_pOpts->surfaceType == MFX_SURFACE_TYPE_OPENCL_IMG2D) {
    #ifdef TOOLS_ENABLE_OPENCL
                                    mfxSurfaceOpenCLImg2D *exportedSurface =
                                        reinterpret_cast<mfxSurfaceOpenCLImg2D *>(
                                            exportedSurfaceGeneral);

                                    if (m_vppParams.vpp.Out.FourCC == MFX_FOURCC_NV12)
                                        stsSurf = m_pOclCtx->WriteRawFrame_NV12(exportedSurface,
                                                                                m_fileInfo.outfile);
                                    else if (m_vppParams.vpp.Out.FourCC == MFX_FOURCC_RGB4)
                                        stsSurf = m_pOclCtx->WriteRawFrame_BGRA(exportedSurface,
                                                                                m_fileInfo.outfile);

                                    VERIFY(MFX_ERR_NONE == stsSurf,
                                           "ERROR: WriteRawFrame_",
                                           stsSurf);

                                    stsSurf = exportedSurface->SurfaceInterface.Release(
                                        &(exportedSurface->SurfaceInterface));
    #endif
                                }
#else
                                mfxSurfaceVAAPI *exportedSurface =
                                    reinterpret_cast<mfxSurfaceVAAPI *>(exportedSurfaceGeneral);
                                stsSurf = WriteRawFrame(m_pDevCtx->GetVADisplay(),
                                                        exportedSurface->vaSurfaceID,
                                                        m_fileInfo.outfile);
                                VERIFY(MFX_ERR_NONE == stsSurf, "ERROR: WriteRawFrame", stsSurf);
                                stsSurf = exportedSurface->SurfaceInterface.Release(
                                    &(exportedSurface->SurfaceInterface));
#endif
                                VERIFY(MFX_ERR_NONE == stsSurf,
                                       "ERROR: SurfaceInterface.Release",
                                       stsSurf);

                                frameNum++;
                            }
                        }

                        if (sts != MFX_WRN_IN_EXECUTION) {
                            sts = pmfxVPPOutSurface->FrameInterface->Release(pmfxVPPOutSurface);
                            VERIFY(sts == MFX_ERR_NONE,
                                   "Could not release vpp output surface",
                                   sts);

#ifdef __linux__
                            if (!m_pOpts->bNoSSA && !isDraining &&
                                m_pOpts->surfaceComponent != MFX_SURFACE_COMPONENT_VPP_OUTPUT) {
                                VAStatus vaSts =
                                    vaDestroySurfaces(m_pDevCtx->GetVADisplay(), &vaSurfaceID, 1);
                                VERIFY(vaSts == VA_STATUS_SUCCESS,
                                       "ERROR: vaDestroySurfaces failed",
                                       MFX_ERR_DEVICE_FAILED);
                            }
#endif
                        }
                    } while (sts == MFX_WRN_IN_EXECUTION);
                }
                break;
            case MFX_ERR_MORE_DATA:
                if (isDraining == true) {
                    isStillGoing = false;
                    sts          = MFX_ERR_NONE;
                }
                break;
            case MFX_WRN_DEVICE_BUSY:
                break;
            default:
                isStillGoing = false;
                if (sts < 0 && sts != MFX_ERR_MORE_DATA)
                    printf("ERROR: MFXVideoVPP_RunFrameVPPAsync(), %d\n", sts);
                break;
        }
    }

    printf("VPP %d frames\n", frameNum);

    return sts;
}

mfxStatus CVPPTest::Run() {
    mfxStatus sts = MFX_ERR_NONE;

    ShowTestInfo(m_pOpts);

    // run main processing loop
    sts = ProcessStreamVPP();
    VERIFY(MFX_ERR_NONE == sts, "ERROR: ProcessStreamVPP()", sts);

    return MFX_ERR_NONE;
}
