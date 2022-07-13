//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) decode and infer application
/// using the core API subset.  For more information see:
/// https://software.intel.com/content/www/us/en/develop/articles/upgrading-from-msdk-to-onevpl.html
/// https://oneapi-src.github.io/oneAPI-spec/elements/oneVPL/source/index.html
/// @file

#include <inference_engine.hpp>
#include "util.h"
using namespace InferenceEngine;

#define BITSTREAM_BUFFER_SIZE 2000000
#define MAX_RESULTS           5

void Usage(void) {
    printf("\n");
    printf("   Usage  :  legacy-decode-infer \n\n");
    printf("     -sw/-hw        use software or hardware implementation\n");
    printf("     -i             input file name (HEVC elementary stream)\n\n");
    printf("     -m             input model name (OpenVINO)\n\n");
    printf("   Example:  legacy-decode-infer -sw  -i in.h265 -m alexnet.xml\n");
    return;
}

// Sort and extract top n results from output blob
void PrintTopResults(const Blob::Ptr &output) {
    SizeVector dims = output->getTensorDesc().getDims();
    if (0 == dims.size() || 1 != dims[0]) {
        printf("Output blob has incorrect dimensions, skipping\n");
        return;
    }

    unsigned n       = static_cast<unsigned>(std::min<size_t>((size_t)MAX_RESULTS, output->size()));
    float *batchData = output->cbuffer().as<float *>();
    std::vector<unsigned> indexes(output->size());

    std::iota(std::begin(indexes), std::end(indexes), 0);
    std::partial_sort(std::begin(indexes),
                      std::begin(indexes) + n,
                      std::end(indexes),
                      [&batchData](unsigned l, unsigned r) {
                          return batchData[l] > batchData[r];
                      });

    printf("\nTop %d results for video frame:", n);
    printf("\nclassid probability\n");
    printf("------- -----------\n");
    for (unsigned j = 0; j < n; j++) {
        printf("%d %f\n", indexes.at(j), batchData[indexes.at(j)]);
    }
}

// Perform classify inference on video frame
void InferFrame(mfxFrameSurface1 *surface,
                InferRequest *infer_request,
                std::string input_name,
                std::string output_name) {
    mfxFrameInfo *info = &surface->Info;
    mfxFrameData *data = &surface->Data;
    Blob::Ptr in_blob, out_blob;
    size_t h = info->Height;
    size_t p = data->Pitch;

    switch (info->FourCC) {
        case MFX_FOURCC_I420: {
            TensorDesc y_desc(Precision::U8, { 1, 1, h, p }, Layout::NHWC);
            TensorDesc uv_desc(Precision::U8, { 1, 1, h / 2, p / 2 }, Layout::NHWC);

            Blob::Ptr y_blob = make_shared_blob<uint8_t>(y_desc, data->Y);
            Blob::Ptr u_blob = make_shared_blob<uint8_t>(uv_desc, data->U);
            Blob::Ptr v_blob = make_shared_blob<uint8_t>(uv_desc, data->V);

            in_blob = make_shared_blob<I420Blob>(y_blob, u_blob, v_blob);
        } break;

        case MFX_FOURCC_NV12: {
            TensorDesc y_desc(Precision::U8, { 1, 1, h, p }, Layout::NHWC);
            TensorDesc uv_desc(Precision::U8, { 1, 2, h / 2, p / 2 }, Layout::NHWC);

            Blob::Ptr y_blob  = make_shared_blob<uint8_t>(y_desc, data->Y);
            Blob::Ptr uv_blob = make_shared_blob<uint8_t>(uv_desc, data->UV);

            in_blob = make_shared_blob<NV12Blob>(y_blob, uv_blob);
        } break;

        default:
            printf("Unsupported FourCC code, skip InferFrame\n");
            return;
    }

    infer_request->SetBlob(input_name, in_blob);
    infer_request->Infer();
    out_blob = infer_request->GetBlob(output_name);

    PrintTopResults(out_blob);
}

int main(int argc, char *argv[]) {
    //Variables used for legacy and 2.x
    bool isDraining                 = false;
    bool isStillGoing               = true;
    FILE *source                    = NULL;
    int accel_fd                    = 0;
    mfxBitstream bitstream          = {};
    mfxFrameSurface1 *decSurfaceOut = NULL;
    mfxSession session              = NULL;
    mfxStatus sts                   = MFX_ERR_NONE;
    mfxSyncPoint syncp              = {};
    mfxU32 framenum                 = 0;
    Params cliParams                = {};
    void *accelHandle               = NULL;
    mfxVideoParam mfxDecParams      = {};

    //variables used only in legacy version
    int nIndex                      = -1;
    mfxFrameAllocRequest decRequest = {};
    mfxFrameSurface1 *decSurfPool   = NULL;
    mfxU8 *decOutBuf                = NULL;

    // variables used only in 2.x version
    mfxConfig cfg;
    mfxLoader loader = NULL;

    // OpenVINO
    Core ie;
    CNNNetwork network;
    std::string input_name, output_name;
    InputInfo::Ptr input_info;
    DataPtr output_info;
    ExecutableNetwork executable_network;
    InferRequest infer_request;

    //Parse command line args to cliParams
    if (ParseArgsAndValidate(argc, argv, &cliParams, PARAMS_DECODE) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    source = fopen(cliParams.infileName, "rb");
    VERIFY(source, "Could not open input file");

    // Initialize oneVPL session
    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed -- is implementation in path?");

    // Implementation used must be the type requested from command line
    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "MFXCreateConfig failed")

    sts = MFXSetConfigFilterProperty(cfg, (mfxU8 *)"mfxImplDescription.Impl", cliParams.implValue);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed for Impl");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts,
           "Cannot create session -- no implementations meet selection criteria");

    // Print info about implementation loaded
    ShowImplementationInfo(loader, 0);

    // Convenience function to initialize available accelerator(s)
    accelHandle = InitAcceleratorHandle(session, &accel_fd);

    // Prepare input bitstream and start decoding
    bitstream.MaxLength = BITSTREAM_BUFFER_SIZE;
    bitstream.Data      = (mfxU8 *)calloc(bitstream.MaxLength, sizeof(mfxU8));
    VERIFY(bitstream.Data, "Not able to allocate input buffer");

    //Pre-parse input stream
    sts = ReadEncodedStream(bitstream, source);
    VERIFY(MFX_ERR_NONE == sts, "Error reading bitstream\n");

    mfxDecParams.mfx.CodecId = MFX_CODEC_HEVC;
    mfxDecParams.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    sts                      = MFXVideoDECODE_DecodeHeader(session, &bitstream, &mfxDecParams);
    VERIFY(MFX_ERR_NONE == sts, "Error decoding header\n");

    // input parameters finished, now initialize decode
    sts = MFXVideoDECODE_Init(session, &mfxDecParams);
    VERIFY(MFX_ERR_NONE == sts, "Error initializing decode\n");

    // Query number required surfaces for decoder
    MFXVideoDECODE_QueryIOSurf(session, &mfxDecParams, &decRequest);

    // External (application) allocation of decode surfaces
    decSurfPool =
        (mfxFrameSurface1 *)calloc(sizeof(mfxFrameSurface1), decRequest.NumFrameSuggested);
    sts = AllocateExternalSystemMemorySurfacePool(&decOutBuf,
                                                  decSurfPool,
                                                  mfxDecParams.mfx.FrameInfo,
                                                  decRequest.NumFrameSuggested);
    VERIFY(MFX_ERR_NONE == sts, "Error in external surface allocation\n");

    // Setup OpenVINO Inference Engine
    try {
        network = ie.ReadNetwork(cliParams.inmodelName);
    }
    catch (InferenceEngine::GeneralError) {
        printf("Could not open model file at %s\n", cliParams.inmodelName);
        goto end;
    }
    VERIFY(network.getInputsInfo().size() == 1, "Sample supports topologies with 1 input only");
    VERIFY(network.getOutputsInfo().size() == 1, "Sample supports topologies with 1 output only");

    input_info = network.getInputsInfo().begin()->second;
    input_info->getPreProcess().setResizeAlgorithm(RESIZE_BILINEAR);
    input_info->getPreProcess().setColorFormat(
        mfxDecParams.mfx.FrameInfo.FourCC == MFX_FOURCC_I420 ? I420 : NV12);
    input_info->setLayout(Layout::NHWC);
    input_info->setPrecision(Precision::U8);
    input_name = network.getInputsInfo().begin()->first;

    output_info = network.getOutputsInfo().begin()->second;
    output_info->setPrecision(Precision::FP32);
    output_name = network.getOutputsInfo().begin()->first;

    executable_network =
        ie.LoadNetwork(network, cliParams.impl == MFX_IMPL_SOFTWARE ? "CPU" : "GPU");
    infer_request = executable_network.CreateInferRequest();

    printf("Decoding and infering %s with %s\n", cliParams.infileName, cliParams.inmodelName);

    nIndex = GetFreeSurfaceIndex(decSurfPool, decRequest.NumFrameSuggested);
    while (isStillGoing == true) {
        // Load encoded stream if not draining
        if (isDraining == false) {
            sts = ReadEncodedStream(bitstream, source);
            if (sts != MFX_ERR_NONE)
                isDraining = true;
        }

        sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                              (isDraining) ? NULL : &bitstream,
                                              &decSurfPool[nIndex],
                                              &decSurfaceOut,
                                              &syncp);

        switch (sts) {
            case MFX_ERR_NONE:
                do {
                    sts = MFXVideoCORE_SyncOperation(session, syncp, WAIT_100_MILLISECONDS);
                    if (MFX_ERR_NONE == sts) {
                        InferFrame(decSurfaceOut, &infer_request, input_name, output_name);
                        framenum++;
                    }
                } while (sts == MFX_WRN_IN_EXECUTION);
                break;
            case MFX_ERR_MORE_DATA:
                // The function requires more bitstream at input before decoding can proceed
                if (isDraining)
                    isStillGoing = false;
                break;
            case MFX_ERR_MORE_SURFACE:
                // The function requires more frame surface at output before decoding can proceed.
                // This applies to external memory allocations and should not be expected for
                // a simple internal allocation case like this
                nIndex = GetFreeSurfaceIndex(decSurfPool, decRequest.NumFrameSuggested);
                break;
            case MFX_ERR_DEVICE_LOST:
                // For non-CPU implementations,
                // Cleanup if device is lost
                break;
            case MFX_WRN_DEVICE_BUSY:
                // For non-CPU implementations,
                // Wait a few milliseconds then try again
                break;
            case MFX_WRN_VIDEO_PARAM_CHANGED:
                // The decoder detected a new sequence header in the bitstream.
                // Video parameters may have changed.
                // In external memory allocation case, might need to reallocate the output surface
                break;
            case MFX_ERR_INCOMPATIBLE_VIDEO_PARAM:
                // The function detected that video parameters provided by the application
                // are incompatible with initialization parameters.
                // The application should close the component and then reinitialize it
                break;
            case MFX_ERR_REALLOC_SURFACE:
                // Bigger surface_work required. May be returned only if
                // mfxInfoMFX::EnableReallocRequest was set to ON during initialization.
                // This applies to external memory allocations and should not be expected for
                // a simple internal allocation case like this
                break;
            default:
                printf("unknown status %d\n", sts);
                isStillGoing = false;
                break;
        }
    }

end:
    printf("Decoded %d frames\n", framenum);

    // Clean up resources - It is recommended to close components first, before
    // releasing allocated surfaces, since some surfaces may still be locked by
    // internal resources.

    if (session) {
        MFXVideoDECODE_Close(session);
        MFXClose(session);
    }

    if (bitstream.Data)
        free(bitstream.Data);

    if (decSurfPool) {
        FreeExternalSystemMemorySurfacePool(decOutBuf, decSurfPool);
    }

    if (source)
        fclose(source);

    FreeAcceleratorHandle(accelHandle, accel_fd);
    accelHandle = NULL;
    accel_fd    = 0;

    if (loader)
        MFXUnload(loader);

    return 0;
}
