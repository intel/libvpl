//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) decode, vpp and infer application,
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
#include <openvino/runtime/intel_gpu/ocl/va.hpp>
#include <openvino/runtime/intel_gpu/properties.hpp>

#include "util.hpp"

#define VERIFY(x, y)               \
    if (!(x)) {                    \
        throw std::logic_error(y); \
    }

#define VERIFY2(x, y)      \
    if (!(x)) {            \
        printf("%s\n", y); \
        return NULL;       \
    }

using namespace ov::preprocess;

#define BITSTREAM_BUFFER_SIZE      2000000
#define SYNC_TIMEOUT               60000
#define MAJOR_API_VERSION_REQUIRED 2
#define MINOR_API_VERSION_REQUIRED 2

void Usage(void) {
    printf("\n");
    printf("   Usage    :    vpl-infer \n\n");
    printf("     -i          input file name (HEVC elementary stream)\n");
    printf("     -m          input model name (object detection)\n");
    printf("     -hw         use hardware implementation\n\n");
    printf("   Example  :    vpl-infer -i in.h265 -m mobilenet-ssd.xml -hw\n\n");
    return;
}

mfxSession CreateVPLSession(mfxLoader *loader, Params *cli);

mfxStatus InferFrame(ov::intel_gpu::ocl::VAContext context,
                     mfxFrameSurface1 *surface,
                     ov::InferRequest inferRequest,
                     std::string inputName,
                     std::string outputName,
                     mfxU16 oriWidth,
                     mfxU16 oriHeight);

int main(int argc, char **argv) {
    Params cliParams = {};
    FILE *source     = NULL;

    //-- Params for oneVPL decode/vpp session
    mfxLoader loader                    = NULL;
    mfxSession session                  = NULL;
    mfxBitstream bitstream              = {};
    mfxSyncPoint syncpD                 = {};
    mfxVideoParam mfxDecParams          = {};
    mfxVideoParam mfxVPPParams          = {};
    mfxFrameSurface1 *pmfxDecOutSurface = NULL;
    mfxFrameSurface1 *pmfxVPPOutSurface = NULL;
    mfxU32 frameNum                     = 0;
    bool isStillGoing                   = true;
    bool isDrainingDec                  = false;
    bool isDrainingVPP                  = false;
    mfxStatus sts                       = MFX_ERR_NONE;

    mfxConfig cfg[4]     = {};
    mfxVariant cfgVal[4] = {};

    VADisplay lvaDisplay;

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

        //-- [OpenVINO] Get OpenVINO runtime version
        std::cout << ov::get_openvino_version() << std::endl;

        //-- [OpenVINO] Initialize OpenVINO Runtime Core
        ov::Core core;

        //-- [OpenVINO] Read a network model
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

        // Stores output layer dimension for oneVPL vpp output configuration
        inputDimWidth  = inputShape[ov::layout::width_idx(inputLayout)];
        inputDimHeight = inputShape[ov::layout::height_idx(inputLayout)];

        //-- [OpenVINO] Configure preprocessing
        PrePostProcessor ppp(model);
        InputInfo &inputInfo = ppp.input(inputTensorName);

        // Set the input tensor
        inputInfo.tensor()
            .set_element_type(ov::element::u8)
            .set_color_format(ColorFormat::NV12_TWO_PLANES, { "y", "uv" })
            .set_memory_type(ov::intel_gpu::memory_type::surface);
        // Convert oneVPL vpp output to BGR plannar in OpenVINO
        inputInfo.preprocess().convert_color(ov::preprocess::ColorFormat::BGR);
        inputInfo.model().set_layout("NCHW");

        model = ppp.build();

        //-- [oneVPL] Create VPL session
        session = CreateVPLSession(&loader, &cliParams);
        VERIFY(session != NULL, "ERROR: not able to create VPL session");

        //-- [oneVPL] Initialize oneVPL decoder
        // Prepare input bitstream
        bitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
        bitstream.Data      = (mfxU8 *)calloc(bitstream.MaxLength, sizeof(mfxU8));
        VERIFY(bitstream.Data, "ERROR: not able to allocate input buffer");
        bitstream.CodecId = MFX_CODEC_HEVC;

        sts = ReadEncodedStream(bitstream, source);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: reading bitstream");

        // Retrieve the frame information from input stream
        mfxDecParams.mfx.CodecId = MFX_CODEC_HEVC;
        mfxDecParams.IOPattern   = MFX_IOPATTERN_OUT_VIDEO_MEMORY;
        sts                      = MFXVideoDECODE_DecodeHeader(session, &bitstream, &mfxDecParams);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: decoding header");

        // Original image size
        oriImgWidth  = mfxDecParams.mfx.FrameInfo.Width;
        oriImgHeight = mfxDecParams.mfx.FrameInfo.Height;

        //-- [oneVPL] Initialize oneVPL VPP for resizing and color space conversion
        // Prepare vpp in/out params
        // vpp in:  decode output image size
        // vpp out: network model input size
        vppInImgWidth   = mfxDecParams.mfx.FrameInfo.Width;
        vppInImgHeight  = mfxDecParams.mfx.FrameInfo.Height;
        vppOutImgWidth  = inputDimWidth;
        vppOutImgHeight = inputDimHeight;

        mfxVPPParams.vpp.In.FourCC        = mfxDecParams.mfx.FrameInfo.FourCC;
        mfxVPPParams.vpp.In.ChromaFormat  = mfxDecParams.mfx.FrameInfo.ChromaFormat;
        mfxVPPParams.vpp.In.Width         = vppInImgWidth;
        mfxVPPParams.vpp.In.Height        = vppInImgHeight;
        mfxVPPParams.vpp.In.CropW         = vppInImgWidth;
        mfxVPPParams.vpp.In.CropH         = vppInImgHeight;
        mfxVPPParams.vpp.In.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
        mfxVPPParams.vpp.In.FrameRateExtN = 30;
        mfxVPPParams.vpp.In.FrameRateExtD = 1;

        mfxVPPParams.vpp.Out.FourCC        = MFX_FOURCC_NV12;
        mfxVPPParams.vpp.Out.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
        mfxVPPParams.vpp.Out.Width         = ALIGN16(vppOutImgWidth);
        mfxVPPParams.vpp.Out.Height        = ALIGN16(vppOutImgHeight);
        mfxVPPParams.vpp.Out.CropW         = vppOutImgWidth;
        mfxVPPParams.vpp.Out.CropH         = vppOutImgHeight;
        mfxVPPParams.vpp.Out.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
        mfxVPPParams.vpp.Out.FrameRateExtN = 30;
        mfxVPPParams.vpp.Out.FrameRateExtD = 1;

        mfxVPPParams.IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;

        // Input parameters finished, now initialize decode
        sts = MFXVideoDECODE_Init(session, &mfxDecParams);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing Decode");

        sts = MFXVideoVPP_Init(session, &mfxVPPParams);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: initializing VPP");

        // Get the vaapi device handle
        sts = MFXVideoCORE_GetHandle(session, MFX_HANDLE_VA_DISPLAY, &lvaDisplay);
        VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXVideoCore_GetHandle error");

        //-- [OpenVINO] Create inference request from shared context object
        auto sharedVAContext = ov::intel_gpu::ocl::VAContext(core, lvaDisplay);

        // Compile network within a shared context
        ov::CompiledModel compiledModel = core.compile_model(model, sharedVAContext);

        // Create inference request
        ov::InferRequest inferRequest = compiledModel.create_infer_request();

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
                //-- [oneVPL] Decode a frame
                sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                                      (isDrainingDec) ? NULL : &bitstream,
                                                      NULL,
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

                    //-- [oneVPL] Resize and convert color space of decoded surface
                    sts = MFXVideoVPP_ProcessFrameAsync(session,
                                                        (isDrainingVPP) ? NULL : pmfxDecOutSurface,
                                                        &pmfxVPPOutSurface);

                    if (sts == MFX_ERR_NONE) {
                        sts = pmfxVPPOutSurface->FrameInterface->Synchronize(pmfxVPPOutSurface,
                                                                             SYNC_TIMEOUT);
                        VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXVideoCORE_SyncOperation failed");

                        //-- [OpenVINO] Infer from shared context and va surface
                        auto context    = compiledModel.get_context();
                        auto &vaContext = static_cast<ov::intel_gpu::ocl::VAContext &>(context);
                        InferFrame(vaContext,
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

        if (loader)
            MFXUnload(loader);

        return 1;
    }

    std::cout << "Decoded " << frameNum << " frames and detected objects\n\n";

    if (bitstream.Data)
        free(bitstream.Data);

    if (source)
        fclose(source);

    if (loader)
        MFXUnload(loader);

    return 0;
}

mfxStatus InferFrame(ov::intel_gpu::ocl::VAContext context,
                     mfxFrameSurface1 *surface,
                     ov::InferRequest inferRequest,
                     std::string inputName,
                     std::string outputName,
                     mfxU16 oriWidth,
                     mfxU16 oriHeight) {
    mfxStatus sts = MFX_ERR_NONE;
    VASurfaceID lvaSurfaceID;
    mfxHDL lresource;
    mfxResourceType lresourceType;

    sts = surface->FrameInterface->GetNativeHandle(surface, &lresource, &lresourceType);

    if (sts != MFX_ERR_NONE)
        return sts;

    std::cout << "Result: " << std::endl;

    lvaSurfaceID = *(VASurfaceID *)lresource;

    // Wrap VPP output into remoteblobs and set it as inference input tensor
    auto nv12Tensor =
        context.create_tensor_nv12(surface->Info.CropH, surface->Info.CropW, lvaSurfaceID);

    inferRequest.set_input_tensor(0, nv12Tensor.first);
    inferRequest.set_input_tensor(1, nv12Tensor.second);

    //-- [OpenVINO] Infers specified input(s) in synchronous mode
    inferRequest.infer();

    auto outputTensor = inferRequest.get_tensor(outputName);

    // Display bounding box rect info and confidence % of detected object
    size_t lastDim = outputTensor.get_shape().back();
    if (lastDim == 7) {
        float *data = (float *)outputTensor.data();
        for (size_t i = 0; i < outputTensor.get_size() / lastDim; i++) {
            int imageId      = static_cast<int>(data[i * lastDim + 0]);
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
            std::cout << "  bbox " << x_min << ", " << y_min << ", " << x_max << ", " << y_max
                      << ", confidence = " << confidence << std::endl;
        }
    }

    std::cout << std::endl;
    return sts;
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

    // Implementation used must provide API version 2.2 or newer
    cfg[3] = MFXCreateConfig(*loader);
    VERIFY2(NULL != cfg[3], "MFXCreateConfig failed")
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = VPLVERSION(MAJOR_API_VERSION_REQUIRED, MINOR_API_VERSION_REQUIRED);
    sts             = MFXSetConfigFilterProperty(cfg[3],
                                     (mfxU8 *)"mfxImplDescription.ApiVersion.Version",
                                     cfgVal);
    VERIFY2(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for API version");

    sts = MFXCreateSession(*loader, 0, &session);
    VERIFY2(MFX_ERR_NONE == sts,
            "ERROR: cannot create session -- no implementations meet selection criteria");

    // Print info about implementation loaded
    ShowImplementationInfo(*loader, 0);
    return session;
}
