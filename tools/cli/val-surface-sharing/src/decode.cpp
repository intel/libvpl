//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./decode.h" //NOLINT
#include "./hw-device.h"
#include "./timing.h"
#include "./util.h"

CDecodeTest::CDecodeTest()
        : m_tIndex(0),
          m_loader(nullptr),
          m_session(nullptr),
          m_pDevCtx(nullptr),
#ifdef TOOLS_ENABLE_OPENCL
          m_pOclCtx(nullptr),
#endif
          m_decParams(),
          m_pOpts(nullptr),
          m_frameInfo(),
          m_fileInfo(),
          m_bsInput(BITSTREAM_BUFFER_SIZE) {
}

CDecodeTest::~CDecodeTest() {
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

mfxStatus CDecodeTest::CreateVPLSession() {
    mfxStatus sts = MFX_ERR_NONE;

    // variables used only in 2.x version
    mfxConfig cfg[4] = {};
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
        (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        cfgVal);
    VERIFY(MFX_ERR_NONE == sts,
           "ERROR: MFXSetConfigFilterProperty failed for decoder CodecID",
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

    sts = MFXCreateSession(m_loader, 0, &m_session);
    VERIFY(MFX_ERR_NONE == sts,
           "ERROR: cannot create m_session -- no implementations meet selection criteria",
           sts);

    printf("Created m_session with library: %s\n", (char *)descPath);

    return MFX_ERR_NONE;
}

mfxStatus CDecodeTest::Init(int tIndex, Options *opts, std::vector<mfxU32> *adapterNumbers) {
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

    // init bitstream
    m_fileInfo.bitstream.MaxLength = static_cast<mfxU32>(m_bsInput.size());
    m_fileInfo.bitstream.Data      = reinterpret_cast<mfxU8 *>(m_bsInput.data());
    VERIFY(m_fileInfo.bitstream.Data, "ERROR: not able to allocate input buffer", sts);
    m_fileInfo.bitstream.CodecId = m_pOpts->codecID;

    sts = ReadEncodedStream(m_fileInfo.bitstream, m_fileInfo.infile);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: reading bitstream", sts);

    // Retrieve the frame information from input stream
    m_decParams.mfx.CodecId = m_pOpts->codecID;
    m_decParams.IOPattern   = MFX_IOPATTERN_OUT_VIDEO_MEMORY;
    m_decParams.AsyncDepth  = 1;

    sts = MFXVideoDECODE_DecodeHeader(m_session, &m_fileInfo.bitstream, &m_decParams);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: decoding header", sts);

    // Input parameters finished, now initialize decode
    sts = MFXVideoDECODE_Init(m_session, &m_decParams);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing Decode", sts);

    m_frameInfo.fourcc = m_decParams.mfx.FrameInfo.FourCC;
    m_frameInfo.width  = m_decParams.mfx.FrameInfo.CropW;
    m_frameInfo.height = m_decParams.mfx.FrameInfo.CropH;
    m_frameInfo.pitch  = (m_decParams.mfx.FrameInfo.FourCC == MFX_FOURCC_RGB4)
                             ? m_decParams.mfx.FrameInfo.Width * 4
                             : m_decParams.mfx.FrameInfo.Width;

    m_pOpts->srcWidth  = m_frameInfo.width;
    m_pOpts->srcHeight = m_frameInfo.height;
    m_pOpts->inFourCC  = m_frameInfo.fourcc;

#ifdef TOOLS_ENABLE_OPENCL
    if (m_pOpts->surfaceType == MFX_SURFACE_TYPE_OPENCL_IMG2D) {
        m_pOclCtx = new OpenCLCtx;
        sts       = m_pOclCtx->OpenCLInit(handleType, handle);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing OpenCL", sts);

        sts = m_pOclCtx->AllocCpuFrameForExport(&m_frameInfo);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: AllocCpuFrame", sts);
    }
#endif
    return MFX_ERR_NONE;
}

mfxStatus CDecodeTest::ProcessStreamDecode() {
    bool isStillGoing = true;
    bool isDraining   = false;
    mfxU32 frameNum   = 0;
    mfxStatus sts     = MFX_ERR_NONE;
    StopWatch total_time;
    StopWatch file_read_time;
    StopWatch file_write_time;
    StopWatch processing_time;

    total_time.start_lap();

    while (isStillGoing == true) {
        mfxFrameSurface1 *pmfxDecOutSurface = nullptr;

        // fill input bitstream buffer
        if (isDraining == false) {
            file_read_time.start_lap();
            sts = ReadEncodedStream(m_fileInfo.bitstream, m_fileInfo.infile);
            if (sts != MFX_ERR_NONE)
                isDraining = true;
            file_read_time.end_lap();
        }

        processing_time.start_lap();
        mfxSyncPoint syncp = {};
        sts                = MFXVideoDECODE_DecodeFrameAsync(m_session,
                                              (isDraining ? nullptr : &m_fileInfo.bitstream),
                                              nullptr,
                                              &pmfxDecOutSurface,
                                              &syncp);
        processing_time.end_lap();

        switch (sts) {
            case MFX_ERR_NONE:
                if (syncp) {
                    do {
                        processing_time.start_lap();
                        sts = MFXVideoCORE_SyncOperation(m_session, syncp, WAIT_100_MILLISECONDS);
                        processing_time.end_lap();
                        if (m_pOpts->bNoSSA == true) {
                            if (MFX_ERR_NONE == sts) {
                                file_write_time.start_lap();
                                sts = WriteRawFrame(pmfxDecOutSurface, m_fileInfo.outfile);
                                VERIFY(MFX_ERR_NONE == sts, "Could not write decode output", sts);
                                frameNum++;
                                file_write_time.end_lap();
                            }
                        }
                        else {
                            if (MFX_ERR_NONE == sts) {
                                processing_time.start_lap();
                                mfxSurfaceHeader descOfExport = {};

                                // set header, all other fields should be empty
                                descOfExport.SurfaceType  = m_pOpts->surfaceType;
                                descOfExport.SurfaceFlags = m_pOpts->surfaceFlag;

                                mfxSurfaceHeader *exportedSurfaceGeneral = nullptr;
                                mfxStatus stsSurf = pmfxDecOutSurface->FrameInterface->Export(
                                    pmfxDecOutSurface,
                                    descOfExport,
                                    &exportedSurfaceGeneral);
                                VERIFY(MFX_ERR_NONE == stsSurf,
                                       "ERROR: MFXMemory_ExportFrameSurface",
                                       stsSurf);
                                processing_time.end_lap();
#ifdef _WIN32
                                if (m_pOpts->surfaceType == MFX_SURFACE_TYPE_D3D11_TEX2D) {
                                    mfxSurfaceD3D11Tex2D *exportedSurface =
                                        reinterpret_cast<mfxSurfaceD3D11Tex2D *>(
                                            exportedSurfaceGeneral);
                                    file_write_time.start_lap();
                                    stsSurf = WriteRawFrame(
                                        m_pDevCtx->GetDeviceHandle(),
                                        (ID3D11Texture2D *)(exportedSurface->texture2D),
                                        m_fileInfo.outfile);
                                    VERIFY(MFX_ERR_NONE == stsSurf,
                                           "ERROR: WriteRawFrame",
                                           stsSurf);
                                    file_write_time.end_lap();
                                    processing_time.start_lap();
                                    stsSurf = exportedSurface->SurfaceInterface.Release(
                                        &(exportedSurface->SurfaceInterface));
                                }
                                else if (m_pOpts->surfaceType == MFX_SURFACE_TYPE_OPENCL_IMG2D) {
    #ifdef TOOLS_ENABLE_OPENCL
                                    mfxSurfaceOpenCLImg2D *exportedSurface =
                                        reinterpret_cast<mfxSurfaceOpenCLImg2D *>(
                                            exportedSurfaceGeneral);
                                    file_write_time.start_lap();

                                    stsSurf = m_pOclCtx->WriteRawFrame_NV12(exportedSurface,
                                                                            m_fileInfo.outfile);
                                    VERIFY(MFX_ERR_NONE == stsSurf,
                                           "ERROR: WriteRawFrame_NV12",
                                           stsSurf);

                                    file_write_time.end_lap();
                                    processing_time.start_lap();
                                    stsSurf = exportedSurface->SurfaceInterface.Release(
                                        &(exportedSurface->SurfaceInterface));
    #endif
                                }
#else
                                mfxSurfaceVAAPI *exportedSurface =
                                    reinterpret_cast<mfxSurfaceVAAPI *>(exportedSurfaceGeneral);
                                file_write_time.start_lap();
                                stsSurf = WriteRawFrame(m_pDevCtx->GetVADisplay(),
                                                        exportedSurface->vaSurfaceID,
                                                        m_fileInfo.outfile);
                                VERIFY(MFX_ERR_NONE == stsSurf, "ERROR: WriteRawFrame", stsSurf);
                                file_write_time.end_lap();
                                processing_time.start_lap();
                                stsSurf = exportedSurface->SurfaceInterface.Release(
                                    &(exportedSurface->SurfaceInterface));
#endif
                                VERIFY(MFX_ERR_NONE == stsSurf,
                                       "ERROR: SurfaceInterface.Release",
                                       stsSurf);

                                frameNum++;
                                processing_time.end_lap();
                            }
                        }

                        processing_time.start_lap();
                        if (sts != MFX_WRN_IN_EXECUTION) {
                            sts = pmfxDecOutSurface->FrameInterface->Release(pmfxDecOutSurface);
                            VERIFY(MFX_ERR_NONE == sts,
                                   "Could not release decode output surface",
                                   sts);
                        }
                        processing_time.end_lap();
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
                    printf("ERROR: MFXVideoDECODE_DecodeFrameAsync(), %d\n", sts);
                break;
        }
    }

    total_time.end_lap();

    printf("Decoded %d frames\n", frameNum);
    printf("Total time: %f ms\n", total_time.elapsed_ms());
    printf("Read time: %f ms\n", file_read_time.elapsed_ms());
    printf("Processing time: %f ms\n", processing_time.elapsed_ms());
    printf("Write time: %f ms\n", file_write_time.elapsed_ms());

    return sts;
}

mfxStatus CDecodeTest::Run() {
    mfxStatus sts = MFX_ERR_NONE;

    ShowTestInfo(m_pOpts);

    // run main processing loop
    sts = ProcessStreamDecode();
    VERIFY(MFX_ERR_NONE == sts, "ERROR: ProcessStreamDecode()", sts);

    return MFX_ERR_NONE;
}
