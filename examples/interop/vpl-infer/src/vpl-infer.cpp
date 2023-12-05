//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal Intel® Video Processing Library (Intel® VPL) decode, vpp and infer application,
/// using 2.x API with internal memory management,
/// showing zerocopy with remoteblob
///
/// @file

#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <openvino/openvino.hpp>

#ifdef ZEROCOPY
    #if defined(_WIN32) || defined(_WIN64)
        #include <openvino/runtime/intel_gpu/ocl/dx.hpp>
    #else
        #include <openvino/runtime/intel_gpu/ocl/va.hpp>
    #endif
    #include <openvino/runtime/intel_gpu/properties.hpp>
#endif

#include "util.hpp"

#define BITSTREAM_BUFFER_SIZE      2000000
#define SYNC_TIMEOUT               60000
#define MAJOR_API_VERSION_REQUIRED 2
#define MINOR_API_VERSION_REQUIRED 2

using namespace ov::preprocess;

void Usage(void) {
    printf("\n");
    printf("   Usage    :    vpl-infer \n\n");
    printf("     -i          input file name (HEVC elementary stream)\n");
    printf("     -m          input model name (object detection)\n");
#ifdef ZEROCOPY
    printf("     -zerocopy   process without creating an additional copy of the data\n");
#endif
    printf("     -legacy     run sample in legacy gen (ex: gen 9.x - SKL, KBL, CFL, etc)\n\n");
    printf("   Example  :    vpl-infer -i in.h265 -m mobilenet-ssd.xml\n\n");
    return;
}

mfxSession CreateVPLSession(mfxLoader *loader, Params *cli);

#ifdef ZEROCOPY
    #if defined(_WIN32) || defined(_WIN64)
mfxStatus InferFrame(ov::intel_gpu::ocl::D3DContext context,
    #else
mfxStatus InferFrame(ov::intel_gpu::ocl::VAContext context,
    #endif
                     mfxFrameSurface1 *surface,
                     ov::InferRequest inferRequest,
                     std::string inputName,
                     std::string outputName,
                     mfxU16 oriWidth,
                     mfxU16 oriHeight);
#endif

void InferFrame(mfxFrameSurface1 *surface,
                ov::InferRequest inferRequest,
                std::string inputName,
                std::string outputName,
                mfxU16 oriWidth,
                mfxU16 oriHeight);

int main(int argc, char **argv) {
    Params cliParams = {};
    FILE *source     = NULL;

    //-- Params for decode/vpp session
    mfxLoader loader                    = NULL;
    mfxSession session                  = NULL;
    mfxBitstream bitstream              = {};
    mfxSyncPoint syncpD                 = {};
    mfxSyncPoint syncpV                 = {};
    mfxVideoParam mfxDecParams          = {};
    mfxVideoParam mfxVPPParams          = {};
    mfxFrameSurface1 *pmfxDecOutSurface = NULL;
    mfxFrameSurface1 *pmfxVPPOutSurface = NULL;
    mfxU32 frameNum                     = 0;
    bool isStillGoing                   = true;
    bool isDrainingDec                  = false;
    bool isDrainingVPP                  = false;
    mfxStatus sts                       = MFX_ERR_NONE;

    //--> variables used only in legacy gen mode
    int accel_fd                       = 0;
    void *accelHandle                  = NULL;
    mfxFrameAllocRequest decRequest    = {};
    mfxFrameAllocRequest vppRequest[2] = {};
    mfxU16 nSurfNumDecVPP              = 0;
    mfxU16 nSurfNumVPPOut              = 0;
    mfxFrameSurface1 *pmfxDecSurfPool  = NULL;
    mfxU8 *decOutBuf                   = NULL;
    mfxFrameSurface1 *pmfxVPPSurfPool  = NULL;
    mfxU8 *vppOutBuf                   = NULL;
    int nIndex                         = -1;
    int nIndex2                        = -1;
    //<--

    mfxConfig cfg[4]     = {};
    mfxVariant cfgVal[4] = {};

#ifdef ZEROCOPY
    #if defined(_WIN32) || defined(_WIN64)
    ID3D11Device *pD3D11Device;
    #else
    VADisplay lvaDisplay;
    #endif
    bool bIsSharedContextReady = false;
#endif

    mfxU16 oriImgWidth, oriImgHeight;
    mfxU16 inputDimWidth, inputDimHeight;
    mfxU16 vppInImgWidth, vppInImgHeight;
    mfxU16 vppOutImgWidth, vppOutImgHeight;

    //-- Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_DECVPP) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    try {
        //-- Open input h265 stream
        source = fopen(cliParams.infileName, "rb");
        VERIFY(source != NULL, "ERROR: could not open input file");

        ov::InferRequest inferRequest;
        ov::CompiledModel compiledModel;

        //-- Get runtime version
        std::cout << ov::get_openvino_version() << std::endl;

        //-- Initialize Runtime Core
        ov::Core core;

        //-- Read a network model
        const std::string modelPath = TSTRING2STRING(cliParams.inmodelName);

        std::cout << "Loading network model files: " << modelPath << std::endl;
        std::shared_ptr<ov::Model> model = core.read_model(modelPath);
        PrintInputAndOutputsInfo(*model);

        VERIFY(model->inputs().size() == 1, "ERROR: sample supports topologies with 1 input only");
        VERIFY(model->outputs().size() == 1,
               "ERROR: sample supports topologies with 1 output only");

        auto input  = model->input();
        auto output = model->output();

        std::string inputTensorName  = input.get_any_name();
        std::string outputTensorName = output.get_any_name();

        // Check whether network model is for object detection
        VERIFY(outputTensorName.compare("detection_out") == 0,
               "ERROR: must use object detection network model (ex: mobilenet-ssd)");

        ov::Shape inputShape   = input.get_shape();
        ov::Layout inputLayout = ov::layout::get_layout(input);

        // Store output layer dimension for vpp output configuration
        inputDimWidth  = static_cast<mfxU16>(inputShape[ov::layout::width_idx(inputLayout)]);
        inputDimHeight = static_cast<mfxU16>(inputShape[ov::layout::height_idx(inputLayout)]);

        //-- Configure preprocessing
        PrePostProcessor ppp(model);
        InputInfo &inputInfo = ppp.input(inputTensorName);

        // Set the input tensor
#ifdef ZEROCOPY
        if (cliParams.bZeroCopy) {
            inputInfo.tensor()
                .set_element_type(ov::element::u8)
                .set_color_format(ColorFormat::NV12_TWO_PLANES, { "y", "uv" })
                .set_memory_type(ov::intel_gpu::memory_type::surface);
        }
        else
#endif
        {
            inputInfo.tensor()
                .set_element_type(ov::element::u8)
                .set_color_format(ColorFormat::NV12_TWO_PLANES, { "y", "uv" });
        }

        // Convert vpp output to BGR plannar
        inputInfo.preprocess().convert_color(ov::preprocess::ColorFormat::BGR);

        inputInfo.model().set_layout("NCHW");

        model = ppp.build();

        //-- Create session
        session = CreateVPLSession(&loader, &cliParams);
        VERIFY(session != NULL, "ERROR: unable to create session");

#ifdef __linux__
        if (cliParams.bLegacyGen) {
            // Convenience function to initialize available accelerator(s)
            accelHandle = InitAcceleratorHandle(session, &accel_fd);
        }
#endif

        //-- Initialize decoder
        // Prepare input bitstream
        bitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
        bitstream.Data      = (mfxU8 *)calloc(bitstream.MaxLength, sizeof(mfxU8));
        VERIFY(bitstream.Data, "ERROR: not able to allocate input buffer");
        bitstream.CodecId = MFX_CODEC_HEVC;

        sts = ReadEncodedStream(bitstream, source);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: reading bitstream");

        // Retrieve the frame information from input stream
        mfxDecParams.mfx.CodecId = MFX_CODEC_HEVC;
        if (cliParams.bLegacyGen) {
            mfxDecParams.IOPattern = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
        }
        else {
            mfxDecParams.IOPattern = MFX_IOPATTERN_OUT_VIDEO_MEMORY;
        }
        sts = MFXVideoDECODE_DecodeHeader(session, &bitstream, &mfxDecParams);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: decoding header");

        // Original image size
        oriImgWidth  = mfxDecParams.mfx.FrameInfo.Width;
        oriImgHeight = mfxDecParams.mfx.FrameInfo.Height;

        //-- Initialize VPP for resizing and color space conversion
        // Prepare vpp in/out params
        // vpp in:  decode output image size
        // vpp out: network model input size
        vppInImgWidth   = mfxDecParams.mfx.FrameInfo.Width;
        vppInImgHeight  = mfxDecParams.mfx.FrameInfo.Height;
        vppOutImgWidth  = inputDimWidth;
        vppOutImgHeight = inputDimHeight;

        mfxVPPParams.vpp.In.FourCC         = mfxDecParams.mfx.FrameInfo.FourCC;
        mfxVPPParams.vpp.In.ChromaFormat   = mfxDecParams.mfx.FrameInfo.ChromaFormat;
        mfxVPPParams.vpp.In.Width          = vppInImgWidth;
        mfxVPPParams.vpp.In.Height         = vppInImgHeight;
        mfxVPPParams.vpp.In.CropW          = vppInImgWidth;
        mfxVPPParams.vpp.In.CropH          = vppInImgHeight;
        mfxVPPParams.vpp.In.PicStruct      = MFX_PICSTRUCT_PROGRESSIVE;
        mfxVPPParams.vpp.In.FrameRateExtN  = 30;
        mfxVPPParams.vpp.In.FrameRateExtD  = 1;
        mfxVPPParams.vpp.Out.FourCC        = MFX_FOURCC_NV12;
        mfxVPPParams.vpp.Out.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
        mfxVPPParams.vpp.Out.Width         = ALIGN16(vppOutImgWidth);
        mfxVPPParams.vpp.Out.Height        = ALIGN16(vppOutImgHeight);
        mfxVPPParams.vpp.Out.CropW         = vppOutImgWidth;
        mfxVPPParams.vpp.Out.CropH         = vppOutImgHeight;
        mfxVPPParams.vpp.Out.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
        mfxVPPParams.vpp.Out.FrameRateExtN = 30;
        mfxVPPParams.vpp.Out.FrameRateExtD = 1;

        if (cliParams.bLegacyGen) {
            mfxVPPParams.IOPattern =
                MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
        }
        else {
            mfxVPPParams.IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;
        }

        if (cliParams.bLegacyGen) {
            // Query number required surfaces for decoder
            sts = MFXVideoDECODE_QueryIOSurf(session, &mfxDecParams, &decRequest);
            if (sts == MFX_WRN_PARTIAL_ACCELERATION)
                sts = MFX_ERR_NONE;
            VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXVideoDECODE_QueryIOSurf()\n");

            sts = MFXVideoVPP_QueryIOSurf(session, &mfxVPPParams, vppRequest);
            VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXVideoVPP_QueryIOSurf()\n");

            // Determine the required number of surfaces for decoder output (VPP input) and for VPP output
            nSurfNumDecVPP = decRequest.NumFrameSuggested + vppRequest[0].NumFrameSuggested;
            nSurfNumVPPOut = vppRequest[1].NumFrameSuggested;

            // External (application) allocation of decode out and vpp in surfaces
            pmfxDecSurfPool = (mfxFrameSurface1 *)calloc(sizeof(mfxFrameSurface1), nSurfNumDecVPP);
            sts             = AllocateExternalSystemMemorySurfacePool(&decOutBuf,
                                                          pmfxDecSurfPool,
                                                          mfxDecParams.mfx.FrameInfo,
                                                          nSurfNumDecVPP);
            VERIFY(MFX_ERR_NONE == sts,
                   "ERROR: in external surface allocation for decode out and vpp in\n");

            // External (application) allocation of vpp out surfaces
            pmfxVPPSurfPool = (mfxFrameSurface1 *)calloc(sizeof(mfxFrameSurface1), nSurfNumVPPOut);
            sts             = AllocateExternalSystemMemorySurfacePool(&vppOutBuf,
                                                          pmfxVPPSurfPool,
                                                          mfxVPPParams.vpp.Out,
                                                          nSurfNumVPPOut);
            VERIFY(MFX_ERR_NONE == sts, "ERROR: in external surface allocation for vpp out\n");
        }

        // Input parameters finished, now initialize decode
        sts = MFXVideoDECODE_Init(session, &mfxDecParams);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing Decode");

        sts = MFXVideoVPP_Init(session, &mfxVPPParams);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing VPP");

        // Compile network
        compiledModel = core.compile_model(model, "GPU");
        // Create inference request
        inferRequest = compiledModel.create_infer_request();

        //-- Start processing
        std::cout << "Decoding VPP, and inferring " << cliParams.infileName << " with "
                  << cliParams.inmodelName << std::endl;

        while (isStillGoing == true) {
            if (isDrainingDec == false) {
                sts = ReadEncodedStream(bitstream, source);
                if (sts != MFX_ERR_NONE)
                    isDrainingDec = true;
            }

            if (!isDrainingVPP) {
                if (cliParams.bLegacyGen) {
                    if (MFX_ERR_MORE_SURFACE == sts || MFX_ERR_NONE == sts) {
                        nIndex = GetFreeSurfaceIndex(pmfxDecSurfPool, nSurfNumDecVPP);
                        VERIFY(MFX_ERR_NOT_FOUND != nIndex,
                               "ERROR: no more available surface for decode out and vpp in");
                    }
                }
                //-- Decode a frame
                sts = MFXVideoDECODE_DecodeFrameAsync(
                    session,
                    (isDrainingDec) ? NULL : &bitstream,
                    (cliParams.bLegacyGen) ? &pmfxDecSurfPool[nIndex] : NULL,
                    &pmfxDecOutSurface,
                    &syncpD);
                // Ignore warnings if output is available,
                // if no output and no action required just repeat the DecodeFrameAsync call
                if (MFX_ERR_NONE < sts && syncpD) {
                    sts = MFX_ERR_NONE;
                }
            }
            else {
                sts = MFX_ERR_NONE;
            }

            switch (sts) {
                case MFX_ERR_NONE:
                    pmfxVPPOutSurface = NULL;

                    //-- Resize and convert color space of decoded surface
                    if (cliParams.bLegacyGen) {
                        if (MFX_ERR_MORE_SURFACE == sts || MFX_ERR_NONE == sts) {
                            nIndex2 = GetFreeSurfaceIndex(pmfxVPPSurfPool, nSurfNumVPPOut);
                            VERIFY(MFX_ERR_NOT_FOUND != nIndex2,
                                   "Error no more available surface for vpp out");
                            pmfxVPPOutSurface = &pmfxVPPSurfPool[nIndex2];
                        }
                        sts =
                            MFXVideoVPP_RunFrameVPPAsync(session,
                                                         (isDrainingVPP) ? NULL : pmfxDecOutSurface,
                                                         pmfxVPPOutSurface,
                                                         NULL,
                                                         &syncpV);

                        // Ignore warnings if output is available,
                        // if no output and no action required just repeat the RunFrameVPPAsync call
                        if (MFX_ERR_NONE < sts && syncpV) {
                            sts = MFX_ERR_NONE;
                        }
                    }
                    else {
                        sts = MFXVideoVPP_ProcessFrameAsync(
                            session,
                            (isDrainingVPP) ? NULL : pmfxDecOutSurface,
                            &pmfxVPPOutSurface);
                    }

                    if (sts == MFX_ERR_NONE) {
                        if (cliParams.bLegacyGen) {
                            sts = MFXVideoCORE_SyncOperation(
                                session,
                                syncpV,
                                60000); // Synchronize. Wait until decoded frame is ready
                            InferFrame(pmfxVPPOutSurface,
                                       inferRequest,
                                       inputTensorName,
                                       outputTensorName,
                                       oriImgWidth,
                                       oriImgHeight);
                            frameNum++;
                        }
                        else {
#ifdef ZEROCOPY
                            if (cliParams.bZeroCopy) {
                                sts = pmfxVPPOutSurface->FrameInterface->Synchronize(
                                    pmfxVPPOutSurface,
                                    SYNC_TIMEOUT);
                                VERIFY(MFX_ERR_NONE == sts,
                                       "ERROR: MFXVideoCORE_SyncOperation failed");

                                if (bIsSharedContextReady == false) {
                                    mfxHandleType device_type;
    #if defined(_WIN32) || defined(_WIN64)
                                    // Get the d3d device handle
                                    pmfxVPPOutSurface->FrameInterface->GetDeviceHandle(
                                        pmfxVPPOutSurface,
                                        (mfxHDL *)&pD3D11Device,
                                        &device_type);
                                    VERIFY(MFX_ERR_NONE == sts,
                                           "ERROR: mfxFrameInterface.GetDeviceHandle error");

                                    //-- Create inference request from shared context object
                                    auto sharedD3D11Context =
                                        ov::intel_gpu::ocl::D3DContext(core, pD3D11Device);

                                    // Compile network within a shared context
                                    compiledModel = core.compile_model(model, sharedD3D11Context);
    #else
                                    // Get the vaapi device handle
                                    pmfxVPPOutSurface->FrameInterface->GetDeviceHandle(
                                        pmfxVPPOutSurface,
                                        &lvaDisplay,
                                        &device_type);
                                    VERIFY(MFX_ERR_NONE == sts,
                                           "ERROR: mfxFrameInterface.GetDeviceHandle error");

                                    //-- Create inference request from shared context object
                                    auto sharedVAContext =
                                        ov::intel_gpu::ocl::VAContext(core, lvaDisplay);

                                    // Compile network within a shared context
                                    compiledModel = core.compile_model(model, sharedVAContext);
    #endif
                                    inferRequest = compiledModel.create_infer_request();

                                    bIsSharedContextReady = true;
                                }

                                //-- Infer from shared context and va surface
                                auto context = compiledModel.get_context();
    #if defined(_WIN32) || defined(_WIN64)
                                auto &sharedContext =
                                    static_cast<ov::intel_gpu::ocl::D3DContext &>(context);
    #else
                                auto &sharedContext =
                                    static_cast<ov::intel_gpu::ocl::VAContext &>(context);
    #endif
                                InferFrame(sharedContext,
                                           pmfxVPPOutSurface,
                                           inferRequest,
                                           inputTensorName,
                                           outputTensorName,
                                           oriImgWidth,
                                           oriImgHeight);

                                sts = pmfxVPPOutSurface->FrameInterface->Release(pmfxVPPOutSurface);
                                VERIFY(MFX_ERR_NONE == sts,
                                       "ERROR: mfxFrameSurfaceInterface->Release failed");

                                frameNum++;
                            }
                            else
#endif
                            {
                                do {
                                    sts = pmfxVPPOutSurface->FrameInterface->Synchronize(
                                        pmfxVPPOutSurface,
                                        WAIT_100_MILLISECONDS);

                                    if (MFX_ERR_NONE == sts) {
                                        pmfxVPPOutSurface->FrameInterface->Map(pmfxVPPOutSurface,
                                                                               MFX_MAP_READ);
                                        VERIFY(MFX_ERR_NONE == sts,
                                               "mfxFrameSurfaceInterface->Map failed");

                                        InferFrame(pmfxVPPOutSurface,
                                                   inferRequest,
                                                   inputTensorName,
                                                   outputTensorName,
                                                   oriImgWidth,
                                                   oriImgHeight);

                                        sts = pmfxVPPOutSurface->FrameInterface->Unmap(
                                            pmfxVPPOutSurface);
                                        VERIFY(MFX_ERR_NONE == sts,
                                               "mfxFrameSurfaceInterface->Unmap failed");

                                        sts = pmfxVPPOutSurface->FrameInterface->Release(
                                            pmfxVPPOutSurface);
                                        VERIFY(MFX_ERR_NONE == sts,
                                               "mfxFrameSurfaceInterface->Release failed");

                                        frameNum++;
                                    }

                                } while (sts == MFX_WRN_IN_EXECUTION);
                            }
                        }
                    }
                    else if (sts == MFX_ERR_MORE_DATA) {
                        if (isDrainingVPP == true)
                            isStillGoing = false;
                    }
                    else {
                        if (sts < 0)
                            isStillGoing = false;
                    }
                    break;
                case MFX_ERR_MORE_DATA:
                    // The function requires more bitstream at input before decoding can proceed
                    if (isDrainingDec)
                        isDrainingVPP = true;
                    break;
                default:
                    isStillGoing = false;
                    break;
            }
        }
    }
    catch (const std::exception &ex) {
        std::cerr << ex.what() << "\n\n";

        if (bitstream.Data)
            free(bitstream.Data);

        if (source)
            fclose(source);

        if (cliParams.bLegacyGen) {
            if (pmfxDecSurfPool) {
                FreeExternalSystemMemorySurfacePool(decOutBuf, pmfxDecSurfPool);
            }

            if (pmfxVPPSurfPool) {
                FreeExternalSystemMemorySurfacePool(vppOutBuf, pmfxVPPSurfPool);
            }

            FreeAcceleratorHandle(accelHandle, accel_fd);
            accelHandle = NULL;
            accel_fd    = 0;
        }

        if (loader)
            MFXUnload(loader);

        return 1;
    }

    std::cout << "Decoded " << frameNum << " frames and detected objects\n\n";

    if (bitstream.Data)
        free(bitstream.Data);

    if (source)
        fclose(source);

    if (cliParams.bLegacyGen) {
        if (pmfxDecSurfPool) {
            FreeExternalSystemMemorySurfacePool(decOutBuf, pmfxDecSurfPool);
        }

        if (pmfxVPPSurfPool) {
            FreeExternalSystemMemorySurfacePool(vppOutBuf, pmfxVPPSurfPool);
        }

        FreeAcceleratorHandle(accelHandle, accel_fd);
        accelHandle = NULL;
        accel_fd    = 0;
    }

    if (loader)
        MFXUnload(loader);

    return 0;
}

#ifdef ZEROCOPY
    #if defined(_WIN32) || defined(_WIN64)
mfxStatus InferFrame(ov::intel_gpu::ocl::D3DContext context,
    #else
mfxStatus InferFrame(ov::intel_gpu::ocl::VAContext context,
    #endif
                     mfxFrameSurface1 *surface,
                     ov::InferRequest inferRequest,
                     std::string inputName,
                     std::string outputName,
                     mfxU16 oriWidth,
                     mfxU16 oriHeight) {
    mfxStatus sts = MFX_ERR_NONE;
    #if defined(_WIN32) || defined(_WIN64)
    ID3D11Texture2D *pD3D11Texture;
    #else
    VASurfaceID lvaSurfaceID;
    #endif
    mfxHDL lresource;
    mfxResourceType lresourceType;

    sts = surface->FrameInterface->GetNativeHandle(surface, &lresource, &lresourceType);

    if (sts != MFX_ERR_NONE)
        return sts;

    std::cout << "Result: " << std::endl;

    #if defined(_WIN32) || defined(_WIN64)
    pD3D11Texture = (ID3D11Texture2D *)lresource;
    // Wrap VPP output into remoteblobs and set it as inference input tensor
    auto nv12Tensor =
        context.create_tensor_nv12(surface->Info.CropH, surface->Info.CropW, pD3D11Texture);
    #else
    lvaSurfaceID = *(VASurfaceID *)lresource;
    // Wrap VPP output into remoteblobs and set it as inference input tensor
    auto nv12Tensor =
        context.create_tensor_nv12(surface->Info.CropH, surface->Info.CropW, lvaSurfaceID);
    #endif

    inferRequest.set_input_tensor(0, nv12Tensor.first);
    inferRequest.set_input_tensor(1, nv12Tensor.second);

    //-- Infers specified input(s) in synchronous mode
    inferRequest.infer();

    auto outputTensor = inferRequest.get_tensor(outputName);

    // Display class id, bounding box rect info, and confidence score of detected object
    size_t lastDim = outputTensor.get_shape().back();

    if (lastDim == 7) {
        float *data = (float *)outputTensor.data();
        for (size_t i = 0; i < outputTensor.get_size() / lastDim; i++) {
            int imageId      = static_cast<int>(data[i * lastDim + 0]);
            int classId      = static_cast<int>(data[i * lastDim + 1]);
            float confidence = data[i * lastDim + 2];
            auto x_min       = static_cast<int>(data[i * lastDim + 3] * oriWidth);
            auto y_min       = static_cast<int>(data[i * lastDim + 4] * oriHeight);
            auto x_max       = static_cast<int>(data[i * lastDim + 5] * oriWidth);
            auto y_max       = static_cast<int>(data[i * lastDim + 6] * oriHeight);
            if (imageId < 0)
                break;
            if (confidence < 0.5) {
                continue;
            }

            printf("    Class ID (%d),  BBox (%4d, %4d, %4d, %4d),  Confidence (%5.3f)\n",
                   classId,
                   x_min,
                   y_min,
                   x_max,
                   y_max,
                   confidence);
        }
    }

    std::cout << std::endl;
    return sts;
}
#endif

void InferFrame(mfxFrameSurface1 *surface,
                ov::InferRequest inferRequest,
                std::string inputName,
                std::string outputName,
                mfxU16 oriWidth,
                mfxU16 oriHeight) {
    mfxFrameInfo *info = &surface->Info;
    mfxFrameData *data = &surface->Data;

    size_t w = info->CropW;
    size_t h = info->CropH;
    size_t p = data->Pitch;

    std::cout << "Result: " << std::endl;

    // Prepare input tensor with copying mfxFrameSurface data
    ov::Tensor inputTensorY{ ov::element::u8, { 1, h, w, 1 } };
    unsigned char *pDataY = (unsigned char *)inputTensorY.data();

    for (int i = 0; i < h; i++)
        memcpy(pDataY + i * w, (unsigned char *)(data->Y + (i * p)), w);

    if (info->FourCC == MFX_FOURCC_I420) {
        p = p / 2;
        h = h / 2;
        w = w / 2;

        ov::Tensor inputTensorU{ ov::element::u8, { 1, h, w, 1 } };
        ov::Tensor inputTensorV{ ov::element::u8, { 1, h, w, 1 } };

        // U
        unsigned char *pDataU = (unsigned char *)inputTensorU.data();

        for (int i = 0; i < h; i++)
            memcpy(pDataU + i * w, (unsigned char *)(data->U + (i * p)), w);

        // V
        unsigned char *pDataV = (unsigned char *)inputTensorV.data();

        for (int i = 0; i < h; i++)
            memcpy(pDataV + i * w, (unsigned char *)(data->V + (i * p)), w);

        inferRequest.set_input_tensor(0, inputTensorY);
        inferRequest.set_input_tensor(1, inputTensorU);
        inferRequest.set_input_tensor(2, inputTensorV);
    }
    else {
        // UV
        ov::Tensor inputTensorUV{ ov::element::u8, { 1, h / 2, w / 2, 2 } };
        unsigned char *pDataUV = (unsigned char *)inputTensorUV.data();

        h = h / 2;
        for (int i = 0; i < h; i++)
            memcpy(pDataUV + i * w, (unsigned char *)(data->UV + (i * p)), w);

        inferRequest.set_input_tensor(0, inputTensorY);
        inferRequest.set_input_tensor(1, inputTensorUV);
    }

    //-- Infers specified input(s) in synchronous mode
    inferRequest.infer();

    auto outputTensor = inferRequest.get_tensor(outputName);

    // Display class id, bounding box rect info, and confidence % of detected object
    size_t lastDim = outputTensor.get_shape().back();

    if (lastDim == 7) {
        float *data = (float *)outputTensor.data();
        for (size_t i = 0; i < outputTensor.get_size() / lastDim; i++) {
            int imageId      = static_cast<int>(data[i * lastDim + 0]);
            int classId      = static_cast<int>(data[i * lastDim + 1]);
            float confidence = data[i * lastDim + 2];
            auto x_min       = static_cast<int>(data[i * lastDim + 3] * oriWidth);
            auto y_min       = static_cast<int>(data[i * lastDim + 4] * oriHeight);
            auto x_max       = static_cast<int>(data[i * lastDim + 5] * oriWidth);
            auto y_max       = static_cast<int>(data[i * lastDim + 6] * oriHeight);
            if (imageId < 0)
                break;
            if (confidence < 0.5) {
                continue;
            }

            printf("    Class Id (%d),  BBox (%4d, %4d, %4d, %4d),  Confidence (%5.3f)\n",
                   classId,
                   x_min,
                   y_min,
                   x_max,
                   y_max,
                   confidence);
        }
    }

    std::cout << std::endl;
    return;
}

mfxSession CreateVPLSession(mfxLoader *loader, Params *cli) {
    mfxStatus sts = MFX_ERR_NONE;

    // variables used only in 2.x version
    mfxConfig cfg[4];
    mfxVariant cfgVal;
    mfxSession session = NULL;

    //-- Create session
    *loader = MFXLoad();
    VERIFY2(NULL != *loader, "ERROR: MFXLoad failed -- is implementation in path?\n");

    // Implementation used must be the hardware implementation
    cfg[0] = MFXCreateConfig(*loader);
    VERIFY2(NULL != cfg[0], "MFXCreateConfig failed")
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = MFX_IMPL_TYPE_HARDWARE;

    sts = MFXSetConfigFilterProperty(cfg[0], (mfxU8 *)"mfxImplDescription.Impl", cfgVal);
    VERIFY2(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for Impl");

    // Implementation must provide an HEVC decoder
    cfg[1] = MFXCreateConfig(*loader);
    VERIFY2(NULL != cfg[1], "MFXCreateConfig failed")
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = MFX_CODEC_HEVC;
    sts             = MFXSetConfigFilterProperty(
        cfg[1],
        (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        cfgVal);
    VERIFY2(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for decoder CodecID");

    // Implementation used must have VPP scaling capability
    cfg[2] = MFXCreateConfig(*loader);
    VERIFY2(NULL != cfg[2], "MFXCreateConfig failed")
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = MFX_EXTBUFF_VPP_SCALING;
    sts             = MFXSetConfigFilterProperty(
        cfg[2],
        (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC",
        cfgVal);
    VERIFY2(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for VPP scale");

    if (cli->bLegacyGen == false) {
        // Implementation used must provide API version 2.2 or newer
        cfg[3] = MFXCreateConfig(*loader);
        VERIFY2(NULL != cfg[3], "MFXCreateConfig failed")
        cfgVal.Type     = MFX_VARIANT_TYPE_U32;
        cfgVal.Data.U32 = VPLVERSION(MAJOR_API_VERSION_REQUIRED, MINOR_API_VERSION_REQUIRED);
        sts             = MFXSetConfigFilterProperty(cfg[3],
                                         (mfxU8 *)"mfxImplDescription.ApiVersion.Version",
                                         cfgVal);
        VERIFY2(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for API version");
    }

    sts = MFXCreateSession(*loader, 0, &session);
    VERIFY2(MFX_ERR_NONE == sts,
            "ERROR: cannot create session -- no implementations meet selection criteria");

    // Print info about implementation loaded
    ShowImplementationInfo(*loader, 0);
    return session;
}
