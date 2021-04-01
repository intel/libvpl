/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <string>
#include "./vpl-common.h"
#include "vpl/mfxvideo.h"

#if !defined(WIN32) && !defined(memcpy_s)
    // memcpy_s proxy to allow use safe version where supported
    #define memcpy_s(dest, destsz, src, count) memcpy(dest, src, count)
#endif

#define ALIGN_UP(addr, size) (((addr) + ((size)-1)) & (~((decltype(addr))(size)-1)))

#define MAX_LENGTH 260
#define MAX_WIDTH  3840
#define MAX_HEIGHT 2160
mfxU32 repeatCount = 0;

const char* InputFrameReadModeString[INPUT_FRAME_READ_MODE_COUNT] = {
    "INPUT_FRAME_READ_MODE_PITCH",
    "INPUT_FRAME_READ_MODE_FRAME"
};

mfxStatus LoadRawFrame(mfxFrameSurface1* pSurface, FILE* f, mfxU32 repeat);
mfxStatus LoadRawFrame2(mfxFrameSurface1* pSurface,
                        FILE* f,
                        int bytes_to_read,
                        mfxU8* buf_read,
                        mfxU32 repeat);
void WriteRawFrame(mfxFrameSurface1* pSurface, FILE* f);
mfxU32 GetSurfaceWidth(mfxU32 fourcc, mfxU16 img_width);
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height);
char** ValidateInput(int cnt, char* in[]);
void str_upper(char* str, int l);
char* ValidateFileName(char* in);
bool ValidateSize(char* in, mfxU32* vsize, mfxU32 vmax);
bool ValidateParams(Params* params);
bool ParseArgsAndValidate(int argc, char* argv[], Params* params);
void Usage(void);
mfxStatus InitializeSession(Params* params, mfxSession* session);
void PrintVppParams(mfxVideoParam* mfxVPPParams);
void InitializeVppParams(Params* params, mfxVideoParam* mfxVPPParams);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        Usage();
        return 1; // return 1 as error code
    }

    char** cmd_args;
    cmd_args = ValidateInput(argc, argv);
    if (cmd_args == NULL) {
        Usage();
        return 1; // return 1 as error code
    }

    Params params = { 0 };
    if (ParseArgsAndValidate(argc, cmd_args, &params) == false) {
        Usage();
        return 1; // return 1 as error code
    }

    printf("opening %s\n", params.infileName);

    FILE* fSource = fopen(params.infileName, "rb");
    if (!fSource) {
        printf("could not open input file, %s\n", params.infileName);
        return 1;
    }

    FILE* fSink = fopen(params.outfileName, "wb");
    if (!fSink) {
        fclose(fSource);
        printf("could not create output file, %s\n", params.outfileName);
        return 1;
    }

    mfxStatus sts      = MFX_ERR_NOT_INITIALIZED;
    mfxSession session = nullptr;

    sts = InitializeSession(&params, &session);
    if (sts != MFX_ERR_NONE) {
        fclose(fSource);
        fclose(fSink);
        return 1;
    }
    puts("Session initialized");

    printf("Dispatcher mode  = %s\n", DispatcherModeString[params.dispatcherMode]);
    printf("Memory mode      = %s\n", MemoryModeString[params.memoryMode]);
    printf("Frame mode       = %s\n", InputFrameReadModeString[params.inFrameReadMode]);

    if (params.vppProcFnName == FN_RUNFRAMEVPPASYNC)
        printf("Process function = RunFrameVPPAsync()\n");
    else // FN_PROCESSFRAMEASYNC
        printf("Process function = ProcessFrameAsync()\n");

    bool b_read_frame = false;

    if (params.inFrameReadMode == INPUT_FRAME_READ_MODE_FRAME) {
        b_read_frame = true;
    }

    mfxIMPL impl;
    MFXQueryIMPL(session, &impl);
    printf("Implementation  = %s\n", (MFX_IMPL_SOFTWARE == impl) ? "SOFTWARE" : "HARDWARE");

    mfxVersion ver;
    MFXQueryVersion(session, &ver);
    printf("API version     = %d.%d\n", ver.Major, ver.Minor);

    // Initialize VPP parameters
    // - For simplistic memory management, system memory surfaces are used to store the raw frames
    //   (Note that when using HW acceleration video surfaces are prefered, for better performance)
    mfxVideoParam mfxVPPParams = { 0 };
    InitializeVppParams(&params, &mfxVPPParams);

    // Query number of required surfaces for VPP
    mfxFrameAllocRequest VPPRequest[2]; // [0] - in, [1] - out
    memset(&VPPRequest, 0, sizeof(mfxFrameAllocRequest) * 2);
    sts = MFXVideoVPP_QueryIOSurf(session, &mfxVPPParams, VPPRequest);
    if (sts != MFX_ERR_NONE) {
        if (fSource) {
            fclose(fSource);
            fSource = NULL;
        }
        if (fSink) {
            fclose(fSink);
            fSink = NULL;
        }

        PrintVppParams(&mfxVPPParams);
        puts("QueryIOSurf error");
        return 1;
    }

    // We currently do not use the suggested number of
    mfxU16 nVPPSurfNumIn  = VPPRequest[0].NumFrameSuggested;
    mfxU16 nVPPSurfNumOut = VPPRequest[1].NumFrameSuggested;

    // Allocate surfaces for VPP: In
    // - Frame surface array keeps pointers all surface planes and general frame info
    mfxU32 surfaceSize = GetSurfaceSize(mfxVPPParams.vpp.In.FourCC,
                                        mfxVPPParams.vpp.In.Width,
                                        mfxVPPParams.vpp.In.Height);
    if (surfaceSize == 0) {
        if (fSource) {
            fclose(fSource);
            fSource = NULL;
        }
        if (fSink) {
            fclose(fSink);
            fSink = NULL;
        }
        puts("VPP-in surface size is wrong");
        return 1;
    }

    mfxU16 surf_w = GetSurfaceWidth(mfxVPPParams.vpp.In.FourCC, mfxVPPParams.vpp.In.Width);
    mfxU16 surf_h = mfxVPPParams.vpp.In.Height;

    // frame-by-frame
    std::vector<mfxFrameSurface1> pVPPSurfacesIn;
    std::vector<mfxU8> surfDataIn;
    mfxU8* surfaceBuffersIn;

    // start load all frames section
    std::vector<mfxFrameSurface1> pVPPSurfacesInAll;
    std::vector<mfxU8> surfDataInAll;

    // get frame size
    mfxU32 frame_size = GetSurfaceSize(mfxVPPParams.vpp.In.FourCC,
                                       mfxVPPParams.vpp.In.Width,
                                       mfxVPPParams.vpp.In.Height);

    if (frame_size == 0) {
        if (fSource) {
            fclose(fSource);
            fSource = NULL;
        }
        if (fSink) {
            fclose(fSink);
            fSink = NULL;
        }
        puts("Input frame size is wrong");
        return 1;
    }

    printf("Frame size       = %d\n", frame_size);

    mfxU8* buf_read = NULL;

    if (b_read_frame) {
        buf_read = reinterpret_cast<mfxU8*>(malloc(frame_size));
        if (!buf_read) {
            puts("Error allocating frame buffer.");
            return 1;
        }
    }

    // get file size
    fseek(fSource, 0, SEEK_END);
#ifdef _WIN32
    mfxU64 file_size = _ftelli64(fSource);
#else
    mfxU64 file_size = ftello(fSource);
#endif
    rewind(fSource);
    printf("File size        = %llu\n", file_size);
    mfxU64 num_frames = file_size / frame_size;
    printf("Frames estimated = %llu\n", num_frames);

    std::vector<mfxFrameSurface1> pVPPSurfacesOut;
    std::vector<mfxU8> surfDataOut;
    mfxU8* surfaceBuffersOut;

    if (params.memoryMode == MEM_MODE_EXTERNAL) {
        surfDataIn.resize(surfaceSize * nVPPSurfNumIn);
        surfaceBuffersIn = surfDataIn.data();

        pVPPSurfacesIn.resize(nVPPSurfNumIn);
        for (mfxI32 i = 0; i < nVPPSurfNumIn; i++) {
            memset(&pVPPSurfacesIn[i], 0, sizeof(mfxFrameSurface1));
            pVPPSurfacesIn[i].Info = mfxVPPParams.vpp.In;
            if (mfxVPPParams.vpp.In.FourCC == MFX_FOURCC_RGB4) {
                pVPPSurfacesIn[i].Data.B     = &surfaceBuffersIn[surfaceSize * i];
                pVPPSurfacesIn[i].Data.G     = pVPPSurfacesIn[i].Data.B + 1;
                pVPPSurfacesIn[i].Data.R     = pVPPSurfacesIn[i].Data.B + 2;
                pVPPSurfacesIn[i].Data.A     = pVPPSurfacesIn[i].Data.B + 3;
                pVPPSurfacesIn[i].Data.Pitch = surf_w;
            }
            else {
                pVPPSurfacesIn[i].Data.Y = &surfaceBuffersIn[surfaceSize * i];
                pVPPSurfacesIn[i].Data.U = pVPPSurfacesIn[i].Data.Y + (mfxU16)surf_w * surf_h;
                pVPPSurfacesIn[i].Data.V =
                    pVPPSurfacesIn[i].Data.U + (((mfxU16)surf_w / 2) * (surf_h / 2));
                pVPPSurfacesIn[i].Data.Pitch = surf_w;
            }
        }

        // Allocate surfaces for VPP: Out
        // - Frame surface array keeps pointers all surface planes and general frame info
        mfxU32 surfaceSizeOut = GetSurfaceSize(mfxVPPParams.vpp.Out.FourCC,
                                               mfxVPPParams.vpp.Out.Width,
                                               mfxVPPParams.vpp.Out.Height);
        if (surfaceSizeOut == 0) {
            if (fSource) {
                fclose(fSource);
                fSource = NULL;
            }
            if (fSink) {
                fclose(fSink);
                fSink = NULL;
            }
            if (buf_read) {
                free(buf_read);
                buf_read = NULL;
            }
            puts("VPP-out surface size is wrong");
            return 1;
        }

        surf_w = GetSurfaceWidth(mfxVPPParams.vpp.Out.FourCC, mfxVPPParams.vpp.Out.Width);
        surf_h = mfxVPPParams.vpp.Out.Height;

        surfDataOut.resize((mfxU32)surfaceSizeOut * nVPPSurfNumOut);
        surfaceBuffersOut = surfDataOut.data();

        pVPPSurfacesOut.resize(nVPPSurfNumOut);
        for (mfxI32 i = 0; i < nVPPSurfNumOut; i++) {
            memset(&pVPPSurfacesOut[i], 0, sizeof(mfxFrameSurface1));
            pVPPSurfacesOut[i].Info = mfxVPPParams.vpp.Out;
            if (mfxVPPParams.vpp.Out.FourCC == MFX_FOURCC_RGB4) {
                pVPPSurfacesOut[i].Data.B     = &surfaceBuffersOut[surfaceSizeOut * i];
                pVPPSurfacesOut[i].Data.G     = pVPPSurfacesOut[i].Data.B + 1;
                pVPPSurfacesOut[i].Data.R     = pVPPSurfacesOut[i].Data.B + 2;
                pVPPSurfacesOut[i].Data.A     = pVPPSurfacesOut[i].Data.B + 3;
                pVPPSurfacesOut[i].Data.Pitch = surf_w;
            }
            else {
                pVPPSurfacesOut[i].Data.Y = &surfaceBuffersOut[surfaceSizeOut * i];
                pVPPSurfacesOut[i].Data.U = pVPPSurfacesOut[i].Data.Y + (mfxU16)surf_w * surf_h;
                pVPPSurfacesOut[i].Data.V =
                    pVPPSurfacesOut[i].Data.U + (((mfxU16)surf_w / 2) * (surf_h / 2));
                pVPPSurfacesOut[i].Data.Pitch = surf_w;
            }
        }
    }

    // Initialize Media SDK VPP
    sts = MFXVideoVPP_Init(session, &mfxVPPParams);
    if (sts != MFX_ERR_NONE) {
        if (fSource) {
            fclose(fSource);
            fSource = NULL;
        }
        if (fSink) {
            fclose(fSink);
            fSink = NULL;
        }
        if (buf_read) {
            free(buf_read);
            buf_read = NULL;
        }
        PrintVppParams(&mfxVPPParams);
        puts("could not initialize vpp");
        return 1;
    }

    // Start processing the frames
    int nSurfIdxIn = 0, nSurfIdxOut = 0;
    mfxSyncPoint syncp;
    mfxU32 framenum = 0;

    printf("Processing %s -> %s\n", params.infileName, params.outfileName);

    // start timer
    auto t1 = std::chrono::high_resolution_clock::now();

    // Stage 1: Main processing loop
    while (MFX_ERR_NONE <= sts || MFX_ERR_MORE_DATA == sts) {
        mfxFrameSurface1* vppSurfaceIn  = nullptr;
        mfxFrameSurface1* vppSurfaceOut = nullptr;

        if (params.memoryMode == MEM_MODE_EXTERNAL) {
            // Find free frame surface for vpp in and out
            nSurfIdxIn = -1;
            for (size_t i = 0; i < pVPPSurfacesIn.size(); i++) {
                if (!pVPPSurfacesIn[i].Data.Locked) {
                    nSurfIdxIn = static_cast<int>(i);
                    break;
                }
            }

            if (nSurfIdxIn < 0) {
                if (fSource) {
                    fclose(fSource);
                    fSource = NULL;
                }
                if (fSink) {
                    fclose(fSink);
                    fSink = NULL;
                }
                if (buf_read) {
                    free(buf_read);
                    buf_read = NULL;
                }
                puts("no available surface");
                return 1;
            }

            vppSurfaceIn = &pVPPSurfacesIn[nSurfIdxIn];

            nSurfIdxOut = -1;
            for (size_t i = 0; i < pVPPSurfacesOut.size(); i++) {
                if (!pVPPSurfacesOut[i].Data.Locked) {
                    nSurfIdxOut = static_cast<int>(i);
                    break;
                }
            }

            if (nSurfIdxOut < 0) {
                if (fSource) {
                    fclose(fSource);
                    fSource = NULL;
                }
                if (fSink) {
                    fclose(fSink);
                    fSink = NULL;
                }
                puts("no available surface");
                return 1;
            }

            vppSurfaceOut = &pVPPSurfacesOut[nSurfIdxOut];
        }
        else if (params.memoryMode == MEM_MODE_INTERNAL) {
            sts = MFXMemory_GetSurfaceForVPPIn(session, &vppSurfaceIn);
            if (sts) {
                if (fSource) {
                    fclose(fSource);
                    fSource = NULL;
                }
                if (fSink) {
                    fclose(fSink);
                    fSink = NULL;
                }
                if (buf_read) {
                    free(buf_read);
                    buf_read = NULL;
                }
                printf("Unknown error in MFXMemory_GetSurfaceForVPPIn, sts = %d()\n", sts);
                return 1;
            }

            vppSurfaceIn->FrameInterface->Map(vppSurfaceIn, MFX_MAP_WRITE);

            if (params.vppProcFnName == FN_RUNFRAMEVPPASYNC) {
                sts = MFXMemory_GetSurfaceForVPPOut(session, &vppSurfaceOut);
                if (sts) {
                    if (fSource) {
                        fclose(fSource);
                        fSource = NULL;
                    }
                    if (fSink) {
                        fclose(fSink);
                        fSink = NULL;
                    }
                    if (buf_read) {
                        free(buf_read);
                        buf_read = NULL;
                    }
                    printf("Unknown error in MFXMemory_GetSurfaceForVPPOut, sts = %d()\n", sts);
                    return 1;
                }

                vppSurfaceOut->FrameInterface->Map(vppSurfaceOut, MFX_MAP_WRITE);
            }
        }

        if (vppSurfaceIn) {
            if (b_read_frame) {
                sts = LoadRawFrame2(vppSurfaceIn, fSource, frame_size, buf_read, params.repeat);
            }
            else {
                sts = LoadRawFrame(vppSurfaceIn, fSource, params.repeat);
            }
        }
        else {
            sts = MFX_ERR_UNKNOWN;
        }

        if (sts != MFX_ERR_NONE)
            break;

        for (;;) {
            // Process a frame asychronously (returns immediately)
            if (params.vppProcFnName == FN_RUNFRAMEVPPASYNC)
                sts = MFXVideoVPP_RunFrameVPPAsync(session,
                                                   vppSurfaceIn,
                                                   vppSurfaceOut,
                                                   NULL,
                                                   &syncp);
            else // FN_PROCESSFRAMEASYNC
                sts = MFXVideoVPP_ProcessFrameAsync(session, vppSurfaceIn, &vppSurfaceOut);

            if (MFX_ERR_NONE < sts &&
                (params.vppProcFnName == FN_RUNFRAMEVPPASYNC ? syncp : (mfxSyncPoint)(1))) {
                sts = MFX_ERR_NONE; // Ignore warnings if output is available
                break;
            }
            else if (MFX_ERR_NOT_ENOUGH_BUFFER == sts) {
                // Allocate more bitstream buffer memory here if needed...
                break;
            }
            else {
                break;
            }
        }
        if (MFX_ERR_NONE == sts) {
            if (params.vppProcFnName == FN_RUNFRAMEVPPASYNC) {
                // Synchronize. Wait until a frame is ready
                sts = MFXVideoCORE_SyncOperation(session, syncp, 60000);
            }
            if (!IS_ARG_EQ(params.outfileName, "null")) {
                WriteRawFrame(vppSurfaceOut, fSink);
            }

            if (params.memoryMode == MEM_MODE_INTERNAL) {
                vppSurfaceIn->FrameInterface->Unmap(
                    vppSurfaceIn); // Exception thrown: read access violation. vppSurfaceIn->FrameInterface was nullptr.
                vppSurfaceIn->FrameInterface->Release(vppSurfaceIn);

                vppSurfaceOut->FrameInterface->Unmap(
                    vppSurfaceOut); // Exception thrown: read access violation. vppSurfaceOut->FrameInterface was nullptr.
                vppSurfaceOut->FrameInterface->Release(vppSurfaceOut);
            }

            ++framenum;
            if (params.maxFrames) {
                if (framenum >= params.maxFrames) {
                    printf("Processed %d frames\n", framenum);

                    // Clean up resources - It is recommended to close Media SDK components
                    // first, before releasing allocated surfaces, since some surfaces may still
                    // be locked by internal Media SDK resources.
                    MFXVideoVPP_Close(session);

                    if (fSource) {
                        fclose(fSource);
                        fSource = NULL;
                    }
                    if (fSink) {
                        fclose(fSink);
                        fSink = NULL;
                    }

                    return 0;
                }
            }
        }
    }

    sts = MFX_ERR_NONE;

    // Stage 2: Retrieve the buffered encoded frames
    while (MFX_ERR_NONE <= sts) {
        mfxFrameSurface1* vppSurfaceOut = nullptr;

        if (params.memoryMode == MEM_MODE_EXTERNAL) {
            // Find free frame surface for vpp out
            nSurfIdxOut = -1;
            for (size_t i = 0; i < pVPPSurfacesOut.size(); i++) {
                if (!pVPPSurfacesOut[i].Data.Locked) {
                    nSurfIdxOut = static_cast<int>(i);
                    break;
                }
            }

            if (nSurfIdxOut < 0) {
                if (fSource) {
                    fclose(fSource);
                    fSource = NULL;
                }
                if (fSink) {
                    fclose(fSink);
                    fSink = NULL;
                }
                puts("no available surface");
                return 1;
            }

            vppSurfaceOut = &pVPPSurfacesOut[nSurfIdxOut];
        }
        else if (params.memoryMode == MEM_MODE_INTERNAL) {
            if (params.vppProcFnName == FN_RUNFRAMEVPPASYNC) {
                sts = MFXMemory_GetSurfaceForVPPOut(session, &vppSurfaceOut);
                if (sts) {
                    if (fSource) {
                        fclose(fSource);
                        fSource = NULL;
                    }
                    if (fSink) {
                        fclose(fSink);
                        fSink = NULL;
                    }
                    if (buf_read) {
                        free(buf_read);
                        buf_read = NULL;
                    }
                    printf("Unknown error in MFXMemory_GetSurfaceForVPPOut, sts = %d()\n", sts);
                    return 1;
                }

                vppSurfaceOut->FrameInterface->Map(vppSurfaceOut, MFX_MAP_WRITE);
            }
        }

        for (;;) {
            // Process a frame asychronously (returns immediately)
            if (params.vppProcFnName == FN_RUNFRAMEVPPASYNC)
                sts = MFXVideoVPP_RunFrameVPPAsync(session, NULL, vppSurfaceOut, NULL, &syncp);
            else // FN_PROCESSFRAMEASYNC
                sts = MFXVideoVPP_ProcessFrameAsync(session, NULL, &vppSurfaceOut);

            if (MFX_ERR_NONE < sts &&
                (params.vppProcFnName == FN_RUNFRAMEVPPASYNC ? syncp : (mfxSyncPoint)1)) {
                sts = MFX_ERR_NONE; // Ignore warnings if output is available
                break;
            }
            else {
                break;
            }
        }

        if (MFX_ERR_NONE == sts) {
            if (params.vppProcFnName == FN_RUNFRAMEVPPASYNC) {
                // Synchronize. Wait until a frame is ready
                sts = MFXVideoCORE_SyncOperation(session, syncp, 60000);
            }

            if (!IS_ARG_EQ(params.outfileName, "null")) {
                WriteRawFrame(&pVPPSurfacesOut[nSurfIdxOut], fSink);
            }

            if (params.memoryMode == MEM_MODE_INTERNAL) {
                vppSurfaceOut->FrameInterface->Unmap(
                    vppSurfaceOut); // Exception thrown: read access violation. vppSurfaceOut->FrameInterface was nullptr.
                vppSurfaceOut->FrameInterface->Release(vppSurfaceOut);
            }

            ++framenum;
            if (params.maxFrames) {
                if (framenum >= params.maxFrames) {
                    break;
                }
            }
        }
    }

    auto t2        = std::chrono::high_resolution_clock::now();
    auto loop_time = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    printf("Processed %d frames\n", framenum);
    if (framenum) {
        printf("fps avg=%.1f\n", (1.0e6 / loop_time) * framenum);
    }

    // Clean up resources - It is recommended to close Media SDK components
    // first, before releasing allocated surfaces, since some surfaces may still
    // be locked by internal Media SDK resources.
    MFXVideoVPP_Close(session);
    MFXClose(session);

    if (params.dispatcherMode == DISPATCHER_MODE_VPL_20)
        CloseNewDispatcher();

    if (fSource) {
        fclose(fSource);
        fSource = NULL;
    }
    if (fSink) {
        fclose(fSink);
        fSink = NULL;
    }
    if (buf_read) {
        free(buf_read);
        buf_read = NULL;
    }
    return 0;
}

mfxStatus LoadRawFrame2(mfxFrameSurface1* pSurface,
                        FILE* f,
                        int bytes_to_read,
                        mfxU8* buf_read,
                        mfxU32 repeat) {
    int nBytesRead = static_cast<int>(fread(buf_read, 1, bytes_to_read, f));

    if (bytes_to_read != nBytesRead) {
        if (repeatCount == repeat)
            return MFX_ERR_MORE_DATA;
        else {
            fseek(f, 0, SEEK_SET);
            repeatCount++;
        }
    }

    mfxU16 w, h;
    mfxFrameInfo* pInfo = &pSurface->Info;
    mfxFrameData* pData = &pSurface->Data;

    w = pInfo->Width;
    h = pInfo->Height;

    switch (pInfo->FourCC) {
        case MFX_FOURCC_NV12:
        case MFX_FOURCC_I420:
            pData->Y = buf_read;
            pData->U = pData->Y + w * h;
            pData->V = pData->U + ((w / 2) * (h / 2));
            break;

        case MFX_FOURCC_P010:
        case MFX_FOURCC_I010:
            pData->Y = buf_read;
            pData->U = pData->Y + w * 2 * h;
            pData->V = pData->U + (w * (h / 2));
            break;

        case MFX_FOURCC_RGB4:
            // read luminance plane (Y)
            pData->B = buf_read;
            break;
        default:
            break;
    }

    return MFX_ERR_NONE;
}

mfxStatus LoadRawFrame(mfxFrameSurface1* pSurface, FILE* f, mfxU32 repeat) {
    mfxU16 w, h, i, pitch;
    mfxU32 nBytesRead;
    mfxU8* ptr;
    mfxFrameInfo* pInfo = &pSurface->Info;
    mfxFrameData* pData = &pSurface->Data;

    w = pInfo->Width;
    h = pInfo->Height;

    switch (pInfo->FourCC) {
        case MFX_FOURCC_NV12:
        case MFX_FOURCC_I420:
            // read luminance plane (Y)
            pitch = pData->Pitch;
            ptr   = pData->Y;
            for (i = 0; i < h; i++) {
                nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, f);
                if (w != nBytesRead) {
                    if (repeatCount == repeat)
                        return MFX_ERR_MORE_DATA;
                    else {
                        fseek(f, 0, SEEK_SET);
                        repeatCount++;
                    }
                }
            }

            // read chrominance (U, V)
            pitch /= 2;
            h /= 2;
            w /= 2;
            ptr = pData->U;
            for (i = 0; i < h; i++) {
                nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, f);
                if (w != nBytesRead) {
                    if (repeatCount == repeat)
                        return MFX_ERR_MORE_DATA;
                    else {
                        fseek(f, 0, SEEK_SET);
                        repeatCount++;
                    }
                }
            }

            ptr = pData->V;
            for (i = 0; i < h; i++) {
                nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, f);
                if (w != nBytesRead) {
                    if (repeatCount == repeat)
                        return MFX_ERR_MORE_DATA;
                    else {
                        fseek(f, 0, SEEK_SET);
                        repeatCount++;
                    }
                }
            }
            break;

        case MFX_FOURCC_P010:
        case MFX_FOURCC_I010:
            // read luminance plane (Y)
            pitch = pData->Pitch;
            w *= 2;
            ptr = pData->Y;
            for (i = 0; i < h; i++) {
                nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, f);
                if (w != nBytesRead) {
                    if (repeatCount == repeat)
                        return MFX_ERR_MORE_DATA;
                    else {
                        fseek(f, 0, SEEK_SET);
                        repeatCount++;
                    }
                }
            }

            // read chrominance (U, V)
            pitch /= 2;
            w /= 2;
            h /= 2;
            ptr = pData->U;
            for (i = 0; i < h; i++) {
                nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, f);
                if (w != nBytesRead) {
                    if (repeatCount == repeat)
                        return MFX_ERR_MORE_DATA;
                    else {
                        fseek(f, 0, SEEK_SET);
                        repeatCount++;
                    }
                }
            }

            ptr = pData->V;
            for (i = 0; i < h; i++) {
                nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, f);
                if (w != nBytesRead) {
                    if (repeatCount == repeat)
                        return MFX_ERR_MORE_DATA;
                    else {
                        fseek(f, 0, SEEK_SET);
                        repeatCount++;
                    }
                }
            }
            break;

        case MFX_FOURCC_RGB4:
            // read luminance plane (Y)
            pitch = pData->Pitch;
            w *= 4;
            ptr = pData->B;
            for (i = 0; i < h; i++) {
                nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, f);
                if (w != nBytesRead) {
                    if (repeatCount == repeat)
                        return MFX_ERR_MORE_DATA;
                    else {
                        fseek(f, 0, SEEK_SET);
                        repeatCount++;
                    }
                }
            }
            break;
        default:
            break;
    }

    return MFX_ERR_NONE;
}

void WriteRawFrame(mfxFrameSurface1* pSurface, FILE* f) {
    mfxU16 w, h, i, pitch;
    mfxFrameInfo* pInfo = &pSurface->Info;
    mfxFrameData* pData = &pSurface->Data;

    w = pInfo->Width;
    h = pInfo->Height;

    // write the output to disk
    switch (pInfo->FourCC) {
        case MFX_FOURCC_NV12:
        case MFX_FOURCC_I420:
            //Y
            pitch = pData->Pitch;
            for (i = 0; i < h; i++) {
                fwrite(pData->Y + i * pitch, 1, w, f);
            }

            //U
            pitch /= 2;
            h /= 2;
            w /= 2;
            for (i = 0; i < h; i++) {
                fwrite(pData->U + i * pitch, 1, w, f);
            }
            //V
            for (i = 0; i < h; i++) {
                fwrite(pData->V + i * pitch, 1, w, f);
            }
            break;

        case MFX_FOURCC_P010:
        case MFX_FOURCC_I010:
            //Y
            pitch = pData->Pitch;
            w *= 2;
            for (i = 0; i < h; i++) {
                fwrite(pSurface->Data.Y + i * pitch, 1, w, f);
            }

            //U
            pitch /= 2;
            w /= 2;
            h /= 2;
            for (i = 0; i < h; i++) {
                fwrite(pSurface->Data.U + i * pitch, 1, w, f);
            }
            //V
            for (i = 0; i < h; i++) {
                fwrite(pSurface->Data.V + i * pitch, 1, w, f);
            }
            break;

        case MFX_FOURCC_RGB4:
            pitch = pData->Pitch;
            w *= 4;
            for (i = 0; i < h; i++) {
                fwrite(pSurface->Data.B + i * pitch, 1, w, f);
            }
            break;
        default:
            break;
    }

    return;
}

mfxU32 GetSurfaceWidth(mfxU32 FourCC, mfxU16 img_width) {
    mfxU16 W;
    switch (FourCC) {
        case MFX_FOURCC_NV12:
        case MFX_FOURCC_I420:
            W = img_width;
            break;
        case MFX_FOURCC_P010:
        case MFX_FOURCC_I010:
            W = img_width * 2;
            break;
        case MFX_FOURCC_BGRA:
        default:
            W = img_width * 4;
            break;
    }

    return W;
}

mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height) {
    mfxU32 nbytes = 0;

    switch (FourCC) {
        case MFX_FOURCC_NV12:
        case MFX_FOURCC_I420:
            nbytes = width * height + (width >> 1) * (height >> 1) + (width >> 1) * (height >> 1);
            break;
        case MFX_FOURCC_P010:
        case MFX_FOURCC_I010:
            nbytes = width * height + (width >> 1) * (height >> 1) + (width >> 1) * (height >> 1);
            nbytes *= 2;
            break;
        case MFX_FOURCC_BGRA:
        default:
            nbytes = width * height * 4;
            break;
    }

    return nbytes;
}

mfxI32 GetFreeSurfaceIndex(const std::vector<mfxFrameSurface1>& pSurfacesPool) {
    auto it = std::find_if(pSurfacesPool.begin(),
                           pSurfacesPool.end(),
                           [](const mfxFrameSurface1& surface) {
                               return 0 == surface.Data.Locked;
                           });

    if (it == pSurfacesPool.end())
        return MFX_ERR_NOT_FOUND;
    else
        return static_cast<mfxI32>(it - pSurfacesPool.begin());
}

char** ValidateInput(int cnt, char* in[]) {
    if (in) {
        for (int i = 0; i < cnt; i++) {
            if (strlen(in[i]) > MAX_LENGTH)
                return NULL;
        }
    }

    return in;
}

void str_upper(char* str, int l) {
    for (int i = 0; i < l; i++) {
        str[i] = static_cast<char>(toupper(str[i]));
    }
}

char* ValidateFileName(char* in) {
    if (in) {
        if (strlen(in) > MAX_LENGTH)
            return NULL;
    }

    return in;
}

bool ValidateSize(char* in, mfxU32* vsize, mfxU32 vmax) {
    if (in) {
        *vsize = static_cast<mfxU16>(strtol(in, NULL, 10));
        if (*vsize <= vmax)
            return true;
    }

    *vsize = 0;
    return false;
}

// perform basic parameter validation and setup
bool ValidateParams(Params* params) {
    // input file (required)
    if (!params->infileName) {
        printf("ERROR - input file name (-i) is required\n");
        return false;
    }

    // output file (required)
    if (!params->outfileName) {
        printf("ERROR - output file name (-o) is required\n");
        return false;
    }

    // input format (required)
    if (!params->infileFormat) {
        printf("ERROR - input format (-if) is required\n");
        return false;
    }

    // input format (required)
    if (!strncmp(params->infileFormat, "NV12", strlen("NV12"))) {
        params->srcFourCC = MFX_FOURCC_NV12;
    }
    else if (!strncmp(params->infileFormat, "BGRA", strlen("BGRA"))) {
        params->srcFourCC = MFX_FOURCC_BGRA;
    }
    else if (!strncmp(params->infileFormat, "I420", strlen("I420"))) {
        params->srcFourCC = MFX_FOURCC_I420;
    }
    else if (!strncmp(params->infileFormat, "I010", strlen("I010"))) {
        params->srcFourCC = MFX_FOURCC_I010;
    }
    else if (!strncmp(params->infileFormat, "P010", strlen("P010"))) {
        params->srcFourCC = MFX_FOURCC_P010;
    }
    else {
        printf("ERROR - unsupported input format %s\n", params->infileFormat);
        return false;
    }

    // input resolution (required)
    if (!params->srcWidth | !params->srcHeight) {
        printf("ERROR - input resolution is required (-sw, -sh)\n");
        return false;
    }

    // output resolution (required)
    if (!params->dstWidth | !params->dstHeight) {
        printf("ERROR - output resolution is required (-dw, -dh)\n");
        return false;
    }

    // output format (required)
    if (!params->outfileFormat) {
        printf("ERROR - output format (-of) is required\n");
        return false;
    }

    if (!strncmp(params->outfileFormat, "NV12", strlen("NV12"))) {
        params->dstFourCC = MFX_FOURCC_NV12;
    }
    else if (!strncmp(params->outfileFormat, "BGRA", strlen("BGRA"))) {
        params->dstFourCC = MFX_FOURCC_BGRA;
    }
    else if (!strncmp(params->outfileFormat, "I420", strlen("I420"))) {
        params->dstFourCC = MFX_FOURCC_I420;
    }
    else if (!strncmp(params->outfileFormat, "I010", strlen("I010"))) {
        params->dstFourCC = MFX_FOURCC_I010;
    }
    else if (!strncmp(params->outfileFormat, "P010", strlen("P010"))) {
        params->dstFourCC = MFX_FOURCC_P010;
    }
    else {
        printf("ERROR - unsupported output format %s\n", params->outfileFormat);
        return false;
    }

    return true;
}

bool ParseArgsAndValidate(int argc, char* argv[], Params* params) {
    int idx;
    char* s;

    // init all params to 0
    memset(params, 0, sizeof(Params));

    params->impl = MFX_IMPL_SOFTWARE;

    // set any non-zero defaults
    if (!params->memoryMode)
        params->memoryMode = MEM_MODE_EXTERNAL;

    if (argc < 2)
        return false;

    for (idx = 1; idx < argc;) {
        // all switches must start with '-'
        if (argv[idx][0] != '-') {
            printf("ERROR - invalid argument: %s\n", argv[idx]);
            return false;
        }

        // switch string, starting after the '-'
        s = &argv[idx][1];
        idx++;

        // search for match
        if (IS_ARG_EQ(s, "i")) {
            params->infileName = ValidateFileName(argv[idx++]);
            if (!params->infileName) {
                return false;
            }
        }
        else if (IS_ARG_EQ(s, "o")) {
            params->outfileName = ValidateFileName(argv[idx++]);
            if (!params->outfileName) {
                return false;
            }
        }
        else if (IS_ARG_EQ(s, "n")) {
            params->maxFrames = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "v")) {
            params->verboseMode = true;
        }
        else if (IS_ARG_EQ(s, "if")) {
            params->infileFormat = argv[idx++];
            str_upper(params->infileFormat,
                      static_cast<int>(strlen(params->infileFormat))); // to upper case
        }
        else if (IS_ARG_EQ(s, "of")) {
            params->outfileFormat = argv[idx++];
            str_upper(params->outfileFormat,
                      static_cast<int>(strlen(params->outfileFormat))); // to upper case
        }
        else if (IS_ARG_EQ(s, "sw")) {
            if (!ValidateSize(argv[idx++], &params->srcWidth, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "sh")) {
            if (!ValidateSize(argv[idx++], &params->srcHeight, MAX_HEIGHT))
                return false;
        }
        else if (IS_ARG_EQ(s, "dw")) {
            if (!ValidateSize(argv[idx++], &params->dstWidth, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "dh")) {
            if (!ValidateSize(argv[idx++], &params->dstHeight, MAX_HEIGHT))
                return false;
        }
        else if (IS_ARG_EQ(s, "td")) {
            params->targetDeviceType = argv[idx++];
        }
        else if (IS_ARG_EQ(s, "sbs")) {
            params->srcbsbufSize = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "dbs")) {
            params->dstbsbufSize = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "to")) {
            params->timeout = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "fr")) {
            params->frameRate = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "br")) {
            params->bitRate = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "tu")) {
            params->targetUsage = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "qu")) {
            params->quality = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "bm")) {
            params->brcMode = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "gs")) {
            params->gopSize = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "rp")) {
            if (atoi(argv[idx]) < 0) {
                printf("ERROR - invalid argument: value for -rp switch cannot be negative\n");
                return false;
            }

            params->repeat = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "kd")) {
            params->keyFrameDist = atoi(argv[idx++]);
        }
        else if (IS_ARG_EQ(s, "ci")) {
            params->enableCinterface = 1;
        }
        else if (IS_ARG_EQ(s, "gcm")) {
            params->gpuCopyMode = argv[idx++];
        }
        else if (IS_ARG_EQ(s, "ext")) {
            params->memoryMode = MEM_MODE_EXTERNAL;
        }
        else if (IS_ARG_EQ(s, "int")) {
            params->memoryMode = MEM_MODE_INTERNAL;
        }
        else if (IS_ARG_EQ(s, "dsp1")) {
            params->dispatcherMode = DISPATCHER_MODE_LEGACY;
        }
        else if (IS_ARG_EQ(s, "dsp2")) {
            params->dispatcherMode = DISPATCHER_MODE_VPL_20;
        }
        else if (IS_ARG_EQ(s, "fn1")) {
            params->vppProcFnName = FN_RUNFRAMEVPPASYNC;
        }
        else if (IS_ARG_EQ(s, "fn2")) {
            params->vppProcFnName = FN_PROCESSFRAMEASYNC;
        }
        else if (IS_ARG_EQ(s, "scrx")) {
            if (!ValidateSize(argv[idx++], &params->srcCropX, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "scry")) {
            if (!ValidateSize(argv[idx++], &params->srcCropY, MAX_HEIGHT))
                return false;
        }
        else if (IS_ARG_EQ(s, "scrw")) {
            if (!ValidateSize(argv[idx++], &params->srcCropW, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "scrh")) {
            if (!ValidateSize(argv[idx++], &params->srcCropH, MAX_HEIGHT))
                return false;
        }
        else if (IS_ARG_EQ(s, "dcrx")) {
            if (!ValidateSize(argv[idx++], &params->dstCropX, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "dcry")) {
            if (!ValidateSize(argv[idx++], &params->dstCropY, MAX_HEIGHT))
                return false;
        }
        else if (IS_ARG_EQ(s, "dcrw")) {
            if (!ValidateSize(argv[idx++], &params->dstCropW, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "dcrh")) {
            if (!ValidateSize(argv[idx++], &params->dstCropH, MAX_HEIGHT))
                return false;
        }
        else if (IS_ARG_EQ(s, "fpitch")) {
            params->inFrameReadMode = INPUT_FRAME_READ_MODE_PITCH;
        }
        else if (IS_ARG_EQ(s, "fframe")) {
            params->inFrameReadMode = INPUT_FRAME_READ_MODE_FRAME;
        }
        else {
            printf("ERROR - invalid argument: %s\n", argv[idx]);
            return false;
        }
    }

    // run basic parameter validation
    return ValidateParams(params);
}

void Usage(void) {
    printf("\nOptions - VPP:\n");
    printf("  -i     inputFile     ... input file name\n");
    printf("  -o     outputFile    ... output file name ('null' for no output file)\n");
    printf("  -n     maxFrames     ... max frames to process\n");
    printf("  -if    inputFormat   ... [i420, i010, bgra]\n");
    printf("  -of    outputFormat  ... [i420, i010, bgra]\n");
    printf("  -sw    srcWidth      ... source width\n");
    printf("  -sh    srcHeight     ... source height \n");
    printf("  -scrx  srcCropX      ... cropX  of src video (def: 0)\n");
    printf("  -scry  srcCropY      ... cropY  of src video (def: 0)\n");
    printf("  -scrw  srcCropW      ... cropW  of src video (def: width)\n");
    printf("  -scrh  srcCropH      ... cropH  of src video (def: height)\n");
    printf("  -dw    dstWidth      ... destination width\n");
    printf("  -dh    dstHeight     ... destination height \n");
    printf("  -rp    repeat        ... number of times to repeat encoding\n");
    printf("  -dcrx  dstCropX      ... cropX  of dst video (def: 0)\n");
    printf("  -dcry  dstCropY      ... cropY  of dst video (def: 0)\n");
    printf("  -dcrw  dstCropW      ... cropW  of dst video (def: width)\n");
    printf("  -dcrh  dstCropH      ... cropH  of dst video (def: height)\n");

    printf("\nMemory model (default = -ext)\n");
    printf("  -ext  = external memory (1.0 style)\n");
    printf("  -int  = internal memory with MFXMemory_GetSurfaceForVPPIn/Out\n");

    printf("\nProcess function (default = -fn1)\n");
    printf("  -fn1  = use RunFrameAPPAsync()\n");
    printf("  -fn2  = use ProcessFrameAsync()\n");

    printf("\nDispatcher (default = -dsp1)\n");
    printf("  -dsp1 = legacy dispatcher (MSDK 1.x)\n");
    printf("  -dsp2 = smart dispatcher (API %d.%d)\n",
           DEFAULT_VERSION_MAJOR,
           DEFAULT_VERSION_MINOR);

    printf("\nImplementation (default = software 2.0 API implementation\n");

    printf("\nFrame mode (optional, default = -fpitch)\n");
    printf("  -fpitch = load frame-by-frame (read data per pitch - legacy)\n");
    printf("  -fframe = load frame-by-frame (read data per frame)\n");

    printf("\nTo view:\n");
    printf(
        " ffplay -video_size [width]x[height] -pixel_format [pixel format] -f rawvideo [out filename]\n");
    return;
}

mfxStatus InitializeSession(Params* params, mfxSession* session) {
    std::string diagnoseText = "";
    mfxStatus sts            = MFX_ERR_UNDEFINED_BEHAVIOR;

    switch (params->dispatcherMode) {
        case DISPATCHER_MODE_VPL_20:
            diagnoseText = "InitNewDispatcher with WSTYPE_VPP in DISPATCHER_MODE_VPL_20";
            sts          = InitNewDispatcher(WSTYPE_VPP, params, session);
            break;
        case DISPATCHER_MODE_LEGACY: {
            if (MFX_IMPL_SOFTWARE == params->impl) {
                diagnoseText = "MFXInitEx with MFX_IMPL_SOFTWARE in DISPATCHER_MODE_LEGACY";
                // initialize session
                mfxInitParam initPar   = { 0 };
                initPar.Version.Major  = DEFAULT_VERSION_MAJOR;
                initPar.Version.Minor  = DEFAULT_VERSION_MINOR;
                initPar.Implementation = MFX_IMPL_SOFTWARE;

                sts = MFXInitEx(initPar, session);
            }
            else {
                diagnoseText = "Tool does not support MFX_IMPL_HARDWARE mode";
                sts          = MFX_ERR_UNSUPPORTED;
            }
        } break;
        default:
            diagnoseText = "Invalid dispatcher mode";
            break;
    }

    if (sts != MFX_ERR_NONE) {
        printf("MFXInitEx error. could not initialize session\n");

        std::cout << "Failed " << diagnoseText.c_str() << std::endl;
        if (sts == MFX_ERR_UNSUPPORTED) {
            printf("Did you configure the necessary environment variables?\n");
        }
        else {
            std::cout << "Error code: " << sts << std::endl;
        }
    }

    return sts;
}

void InitializeVppParams(Params* params, mfxVideoParam* mfxVPPParams) {
    // Input data
    (*mfxVPPParams).vpp.In.FourCC        = params->srcFourCC;
    (*mfxVPPParams).vpp.In.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    (*mfxVPPParams).vpp.In.CropX         = params->srcCropX;
    (*mfxVPPParams).vpp.In.CropY         = params->srcCropY;
    (*mfxVPPParams).vpp.In.CropW         = params->srcCropW ? params->srcCropW : params->srcWidth;
    (*mfxVPPParams).vpp.In.CropH         = params->srcCropH ? params->srcCropH : params->srcHeight;
    (*mfxVPPParams).vpp.In.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    (*mfxVPPParams).vpp.In.FrameRateExtN = 30;
    (*mfxVPPParams).vpp.In.FrameRateExtD = 1;
    // FFmpeg manages alignment, no need to make alignment here for cpu-plugin perspective.
    // BTW, if this app should be common to legacy mediasdk, we need to use same method and use alignment here.
    // Then, we need to have new param that we can deliver original size (before alignment) to vpl.
    (*mfxVPPParams).vpp.In.Width  = params->srcWidth;
    (*mfxVPPParams).vpp.In.Height = params->srcHeight;
    if (params->srcFourCC == MFX_FOURCC_P010) // enable ms10bit
        (*mfxVPPParams).vpp.In.Shift = 1;

    // Output data
    (*mfxVPPParams).vpp.Out.FourCC        = params->dstFourCC;
    (*mfxVPPParams).vpp.Out.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    (*mfxVPPParams).vpp.Out.CropX         = params->dstCropX;
    (*mfxVPPParams).vpp.Out.CropY         = params->dstCropY;
    (*mfxVPPParams).vpp.Out.CropW         = params->dstCropW ? params->dstCropW : params->dstWidth;
    (*mfxVPPParams).vpp.Out.CropH         = params->dstCropH ? params->dstCropH : params->dstHeight;
    (*mfxVPPParams).vpp.Out.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    (*mfxVPPParams).vpp.Out.FrameRateExtN = 30;
    (*mfxVPPParams).vpp.Out.FrameRateExtD = 1;
    (*mfxVPPParams).vpp.Out.Width         = params->dstWidth;
    (*mfxVPPParams).vpp.Out.Height        = params->dstHeight;
    if (params->dstFourCC == MFX_FOURCC_P010) // enable ms10bit
        (*mfxVPPParams).vpp.Out.Shift = 1;

    (*mfxVPPParams).IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
}

void PrintVppParams(mfxVideoParam* mfxVPPParams) {
    std::cout << "CodecId=" << mfxVPPParams->mfx.CodecId << " | ";
    std::cout << "CodecProfile=" << mfxVPPParams->mfx.CodecProfile << " | ";
    std::cout << "CodecLevel=" << mfxVPPParams->mfx.CodecLevel << " | ";
    std::cout << "GopPicSize=" << mfxVPPParams->mfx.GopPicSize << " | ";
    std::cout << "JPEGChromaFormat=" << mfxVPPParams->mfx.JPEGChromaFormat << " | ";
    std::cout << "Rotation=" << mfxVPPParams->mfx.Rotation << " | ";
    std::cout << "JPEGColorFormat=" << mfxVPPParams->mfx.JPEGColorFormat << " | " << std::endl;
    std::cout << "PicStruct=" << mfxVPPParams->mfx.FrameInfo.PicStruct << " | ";
    std::cout << "BitDepthLuma=" << mfxVPPParams->mfx.FrameInfo.BitDepthLuma << " | ";
    std::cout << "BitDepthChroma=" << mfxVPPParams->mfx.FrameInfo.BitDepthChroma << " | ";
    std::cout << "ChromaFormat=" << mfxVPPParams->mfx.FrameInfo.ChromaFormat << " | ";
    std::cout << "FourCC=" << mfxVPPParams->mfx.FrameInfo.FourCC << " | ";
    std::cout << "Width=" << mfxVPPParams->mfx.FrameInfo.Width << " | ";
    std::cout << "Height=" << mfxVPPParams->mfx.FrameInfo.Height << " | " << std::endl;
    std::cout << "CropX=" << mfxVPPParams->mfx.FrameInfo.CropX << " | ";
    std::cout << "CropY=" << mfxVPPParams->mfx.FrameInfo.CropY << " | ";
    std::cout << "CropH=" << mfxVPPParams->mfx.FrameInfo.CropH << " | ";
    std::cout << "CropW=" << mfxVPPParams->mfx.FrameInfo.CropW << " | ";
    std::cout << "AspectRatioH=" << mfxVPPParams->mfx.FrameInfo.AspectRatioH << " | ";
    std::cout << "AspectRatioW=" << mfxVPPParams->mfx.FrameInfo.AspectRatioW << " | ";
    std::cout << "FrameRateExtD=" << mfxVPPParams->mfx.FrameInfo.FrameRateExtD << " | ";
    std::cout << "FrameRateExtN=" << mfxVPPParams->mfx.FrameInfo.FrameRateExtN << " | "
              << std::endl;
    std::cout << "Quality=" << mfxVPPParams->mfx.Quality << " | " << std::endl;
}
