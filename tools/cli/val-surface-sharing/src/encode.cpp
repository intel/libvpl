//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./encode.h" //NOLINT
#include <algorithm>
#include "./hw-device.h"
#include "./util.h"

CEncodeTest::CEncodeTest()
        : m_tIndex(0),
          m_loader(nullptr),
          m_session(nullptr),
          m_pDevCtx(nullptr),
#ifdef TOOLS_ENABLE_OPENCL
          m_pOclCtx(nullptr),
#endif
          m_encParams(),
          m_pOpts(nullptr),
          m_frameInfo(),
          m_fileInfo(),
          m_memoryInterface(nullptr) {
}

CEncodeTest::~CEncodeTest() {
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

mfxStatus CEncodeTest::CreateVPLSession() {
    mfxStatus sts = MFX_ERR_NONE;

    // variables used only in 2.x version
    mfxConfig cfg[4] = {};
    mfxVariant cfgVal;

    //-- Create session
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

    // Implementation used must be specified acceleration mode
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

    // Implementation must provide designated codec
    cfg[2]          = MFXCreateConfig(m_loader);
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = m_pOpts->codecID;
    sts             = MFXSetConfigFilterProperty(
        cfg[2],
        (mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID",
        cfgVal);
    VERIFY(MFX_ERR_NONE == sts,
           "ERROR: MFXSetConfigFilterProperty failed for encoder CodecID",
           sts);

    // Implementation used must provide API version 2.10 or newer
    cfg[3]          = MFXCreateConfig(m_loader);
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = VPLVERSION(MAJOR_API_VERSION_REQUIRED, MINOR_API_VERSION_REQUIRED);
    sts             = MFXSetConfigFilterProperty(cfg[3],
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

    printf("Created session with library: %s\n", (char *)descPath);

    sts = MFXCreateSession(m_loader, 0, &m_session);
    VERIFY(MFX_ERR_NONE == sts,
           "ERROR: cannot create session -- no implementations meet selection criteria",
           sts);

    return MFX_ERR_NONE;
}

mfxStatus CEncodeTest::Init(int tIndex, Options *opts, std::vector<mfxU32> *adapterNumbers) {
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

    // init encode parameters
    m_encParams.mfx.CodecId           = m_pOpts->codecID;
    m_encParams.mfx.TargetUsage       = MFX_TARGETUSAGE_BALANCED;
    m_encParams.mfx.TargetKbps        = 2000;
    m_encParams.mfx.RateControlMethod = MFX_RATECONTROL_VBR;
    m_encParams.mfx.LowPower          = MFX_CODINGOPTION_ON;

    m_encParams.mfx.FrameInfo.FrameRateExtN = 30;
    m_encParams.mfx.FrameInfo.FrameRateExtD = 1;
    m_encParams.mfx.FrameInfo.FourCC        = m_pOpts->inFourCC;
    m_encParams.mfx.FrameInfo.ChromaFormat =
        (m_pOpts->inFourCC == MFX_FOURCC_NV12) ? MFX_CHROMAFORMAT_YUV420 : MFX_CHROMAFORMAT_YUV444;

    m_encParams.mfx.FrameInfo.CropW = m_pOpts->srcWidth;
    m_encParams.mfx.FrameInfo.CropH = m_pOpts->srcHeight;

    m_encParams.mfx.FrameInfo.Width  = ALIGN16(m_encParams.mfx.FrameInfo.CropW);
    m_encParams.mfx.FrameInfo.Height = ALIGN16(m_encParams.mfx.FrameInfo.CropH);

    m_encParams.IOPattern  = MFX_IOPATTERN_IN_VIDEO_MEMORY;
    m_encParams.AsyncDepth = 1;

    // query encoder
    sts = MFXVideoENCODE_Query(m_session, &m_encParams, &m_encParams);
    VERIFY(MFX_ERR_NONE <= sts, "ERROR: query Encode", sts);
    if (sts)
        printf("Warning: MFXVideoENCODE_Query returned %d.\n", sts);

    // init encoder
    sts = MFXVideoENCODE_Init(m_session, &m_encParams);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing Encode", sts);

    mfxMemoryInterface *iface = nullptr;
    sts                       = MFXGetMemoryInterface(m_session, &iface);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXGetMemoryInterface", sts);
    m_memoryInterface = iface;

    m_frameInfo.fourcc = m_pOpts->inFourCC;
    m_frameInfo.width  = m_pOpts->srcWidth;
    m_frameInfo.height = m_pOpts->srcHeight;
    m_frameInfo.pitch = (m_pOpts->inFourCC == MFX_FOURCC_RGB4) ? m_encParams.mfx.FrameInfo.Width * 4
                                                               : m_encParams.mfx.FrameInfo.Width;

#ifdef TOOLS_ENABLE_OPENCL
    if (m_pOpts->surfaceType == MFX_SURFACE_TYPE_OPENCL_IMG2D) {
        m_pOclCtx = new OpenCLCtx;
        sts       = m_pOclCtx->OpenCLInit(handleType, handle);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing OpenCL", sts);

        m_pOclCtx->SetOCLSurfaceFrameInfo(&m_frameInfo);
    }
#endif
    return MFX_ERR_NONE;
}

mfxStatus CEncodeTest::ProcessStreamEncode() {
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
        mfxFrameSurface1 *pmfxEncSurface = nullptr;
        if (isDraining == false) {
            if (m_pOpts->bNoSSA == true) {
                sts = MFXMemory_GetSurfaceForEncode(m_session, &pmfxEncSurface);
                VERIFY(MFX_ERR_NONE == sts, "Could not get encode surface", sts);

                sts = ReadRawFrame(pmfxEncSurface, m_fileInfo.infile);
                if (sts != MFX_ERR_NONE)
                    isDraining = true;
            }
            else {
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

                        CComPtr<ID3D11Texture2D> tex2D =
                            m_pDevCtx->CreateSurfaceToShare(&m_frameInfo, bsInput.data());
                        VERIFY(nullptr != tex2D,
                               "ERROR: CreateSurfaceToShare",
                               MFX_ERR_DEVICE_FAILED);

                        surfaceForImport.texture2D = tex2D;

                        sts = m_memoryInterface->ImportFrameSurface(
                            m_memoryInterface,
                            MFX_SURFACE_COMPONENT_ENCODE,
                            &surfaceForImport.SurfaceInterface.Header,
                            &pmfxEncSurface);
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
                            MFX_SURFACE_COMPONENT_ENCODE,
                            &surfaceForImport.SurfaceInterface.Header,
                            &pmfxEncSurface);
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
                        MFX_SURFACE_COMPONENT_ENCODE,
                        &surfaceForImport.SurfaceInterface.Header,
                        &pmfxEncSurface);
                    VERIFY(MFX_ERR_NONE == sts, "ERROR: ImportFrameSurface", sts);

                    if (m_pOpts->surfaceMode == SURFACE_MODE_SHARED) {
                        m_importedVAsurfaces.emplace_back(pmfxEncSurface,
                                                          surfaceForImport.vaSurfaceID,
                                                          surfaceForImport.vaDisplay);
                    }
#endif
                }
            }
        }

        // encode next frame
        mfxSyncPoint syncp = {};
        sts                = MFXVideoENCODE_EncodeFrameAsync(m_session,
                                              nullptr,
                                              (isDraining ? nullptr : pmfxEncSurface),
                                              &m_fileInfo.bitstream,
                                              &syncp);

#ifdef __linux__
        if (m_pOpts->surfaceMode != SURFACE_MODE_SHARED)
#endif
        {
            if (pmfxEncSurface) {
                mfxStatus sts_release = pmfxEncSurface->FrameInterface->Release(
                    pmfxEncSurface); // use different error code
                VERIFY(sts_release == MFX_ERR_NONE, "ERROR: Release", sts_release);
            }
        }

        switch (sts) {
            case MFX_ERR_NONE:
                if (syncp) {
                    do {
                        sts = MFXVideoCORE_SyncOperation(m_session, syncp, WAIT_100_MILLISECONDS);
                        if (MFX_ERR_NONE == sts) {
                            WriteEncodedStream(m_fileInfo.bitstream, m_fileInfo.outfile);
                            frameNum++;
                        }
#ifdef __linux__
                        if (sts != MFX_WRN_IN_EXECUTION) {
                            if (m_pOpts->surfaceMode == SURFACE_MODE_SHARED)
                                ReleaseFreeSurfaces();
                            else if (!m_pOpts->bNoSSA && !isDraining) {
                                VAStatus vaSts =
                                    vaDestroySurfaces(m_pDevCtx->GetVADisplay(), &vaSurfaceID, 1);
                                VERIFY(vaSts == VA_STATUS_SUCCESS,
                                       "ERROR: vaDestroySurfaces failed",
                                       MFX_ERR_DEVICE_FAILED);
                            }
                        }
#endif
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
                // wait and try again
                break;
            default:
                isStillGoing = false;
                if (sts < 0 && sts != MFX_ERR_MORE_DATA)
                    printf("ERROR: MFXVideoENCODE_EncodeFrameAsync(), %d\n", sts);
                break;
        }
    }

    printf("Encoded %d frames\n", frameNum);

    return sts;
}

mfxStatus CEncodeTest::Run() {
    std::vector<mfxU8 *> outbuf(BITSTREAM_BUFFER_SIZE);
    mfxStatus sts = MFX_ERR_NONE;

    // init bitstream
    m_fileInfo.bitstream.MaxLength = static_cast<mfxU32>(outbuf.size());
    m_fileInfo.bitstream.Data      = reinterpret_cast<mfxU8 *>(outbuf.data());
    VERIFY(m_fileInfo.bitstream.Data,
           "ERROR: not able to allocate input buffer",
           MFX_ERR_MEMORY_ALLOC);
    m_fileInfo.bitstream.CodecId = m_pOpts->codecID;

    ShowTestInfo(m_pOpts);

    // run main processing loop
    sts = ProcessStreamEncode();
    VERIFY(MFX_ERR_NONE == sts, "ERROR: ProcessStreamEncode()", sts);

    return MFX_ERR_NONE;
}

#ifdef __linux__
void CEncodeTest::ReleaseFreeSurfaces() {
    std::list<ImportedVASurfaceWrapper> surfaces_to_delete;

    // splice_if
    for (auto it = std::begin(m_importedVAsurfaces); it != std::end(m_importedVAsurfaces);) {
        auto it_to_transfer = it++;
        if (it_to_transfer->m_imported_mfx_surface->Data.Locked == 0) {
            surfaces_to_delete.splice(std::end(surfaces_to_delete),
                                      m_importedVAsurfaces,
                                      it_to_transfer);
        }
    }
}
#endif
