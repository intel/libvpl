/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./vpl-common.h"

#define ALIGN_UP(addr, size) \
    (((addr) + ((size)-1)) & (~((decltype(addr))(size)-1)))

// IVF container helper functions and definitions
#define AV1_FOURCC 0x31305641
#define MAX_WIDTH  3840
#define MAX_HEIGHT 2160
#define MAX_LENGTH 260

typedef struct {
    mfxU32 width;
    mfxU32 height;
    mfxU32 framerate_numerator;
    mfxU32 framerate_denominator;

} AV1EncConfig;

AV1EncConfig* g_conf = NULL;

inline void mem_put_le16(void* vmem, mfxU32 val);
inline void mem_put_le32(void* vmem, mfxU32 val);
void WriteIVF_StreamHeader(const AV1EncConfig* conf, FILE* f);
void WriteIVF_FrameHeader(mfxU32 byte_count, mfxU64 pts, FILE* f);
void WriteEncodedStream(mfxU32 nframe,
                        void* conf,
                        mfxU8* data,
                        mfxU32 length,
                        mfxU32 codecID,
                        FILE* f);
void UpdateTotalNumberFrameInfo(FILE* f, mfxU32 total_frames);
mfxStatus LoadRawFrame(mfxFrameSurface1* pSurface, FILE* fSource);
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height);
mfxI32 GetFreeSurfaceIndex(const std::vector<mfxFrameSurface1>& pSurfacesPool);
char** ValidateInput(int cnt, char* in[]);
void str_upper(char* str, int l);
char* ValidateFileName(char* in);
bool ValidateSize(char* in, mfxU32* vsize, mfxU32 vmax);
bool ValidateParams(Params* params);
bool ParseArgsAndValidate(int argc, char* argv[], Params* params);
void Usage(void);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        Usage();
        return 1; // return 1 as error code
    }
    char** cmd_args;
    cmd_args = ValidateInput(argc, argv);
    if (cmd_args == NULL) {
        Usage();
        return 1;
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

    if (params.dispatcherMode == DISPATCHER_MODE_VPL_20) {
        sts = InitNewDispatcher(WSTYPE_ENCODE, &params, &session);
    }
    else if (params.dispatcherMode == DISPATCHER_MODE_LEGACY) {
        // initialize session
        mfxInitParam initPar   = { 0 };
        initPar.Version.Major  = 2;
        initPar.Version.Minor  = 0;
        initPar.Implementation = MFX_IMPL_SOFTWARE;

        sts = MFXInitEx(initPar, &session);
    }
    else {
        printf("invalid dispatcher mode %d\n", params.dispatcherMode);
    }

    if (sts != MFX_ERR_NONE) {
        puts("MFXInitEx error. could not initialize session");
        fclose(fSource);
        fclose(fSink);
        return 1;
    }

    printf("Dispatcher mode = %s\n",
           DispatcherModeString[params.dispatcherMode]);
    printf("Memory mode     = %s\n", MemoryModeString[params.memoryMode]);
    puts("library initialized");

    // Initialize encoder parameters
    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));
    mfxEncParams.mfx.CodecId     = params.dstFourCC;
    mfxEncParams.mfx.TargetUsage = params.targetUsage;
    mfxEncParams.mfx.TargetKbps  = params.bitRate;
    if (mfxEncParams.mfx.CodecId == MFX_CODEC_JPEG)
        mfxEncParams.mfx.Quality = params.quality;
    mfxEncParams.mfx.RateControlMethod = params.brcMode;
    if (mfxEncParams.mfx.RateControlMethod == MFX_RATECONTROL_CQP) {
        mfxEncParams.mfx.QPI = params.qp;
        mfxEncParams.mfx.QPB = params.qp;
        mfxEncParams.mfx.QPP = params.qp;
    }
    mfxEncParams.mfx.GopPicSize              = params.gopSize;
    mfxEncParams.mfx.GopRefDist              = params.keyFrameDist;
    mfxEncParams.mfx.FrameInfo.FrameRateExtN = params.frameRate;
    mfxEncParams.mfx.FrameInfo.FrameRateExtD = 1;
    mfxEncParams.mfx.FrameInfo.FourCC        = params.srcFourCC;
    mfxEncParams.mfx.FrameInfo.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxEncParams.mfx.FrameInfo.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    mfxEncParams.mfx.FrameInfo.CropX         = 0;
    mfxEncParams.mfx.FrameInfo.CropY         = 0;
    mfxEncParams.mfx.FrameInfo.CropW         = params.srcWidth;
    mfxEncParams.mfx.FrameInfo.CropH         = params.srcHeight;
    mfxEncParams.mfx.FrameInfo.Width         = params.srcWidth;
    mfxEncParams.mfx.FrameInfo.Height        = params.srcHeight;

    if (params.srcFourCC == MFX_FOURCC_I010) {
        mfxEncParams.mfx.FrameInfo.BitDepthLuma   = 10;
        mfxEncParams.mfx.FrameInfo.BitDepthChroma = 10;
        mfxEncParams.mfx.FrameInfo.Shift          = 1;
        if (params.dstFourCC == MFX_CODEC_HEVC) {
            mfxEncParams.mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN10;
            mfxEncParams.mfx.CodecLevel   = MFX_LEVEL_HEVC_51;
        }
        else if (params.dstFourCC == MFX_CODEC_AVC) {
            mfxEncParams.mfx.CodecProfile = MFX_PROFILE_AVC_HIGH10;
            mfxEncParams.mfx.CodecLevel   = MFX_LEVEL_AVC_1;
        }
    }

    mfxEncParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    if (params.dstFourCC == MFX_CODEC_AV1) {
        g_conf                      = new AV1EncConfig;
        g_conf->width               = params.srcWidth;
        g_conf->height              = params.srcHeight;
        g_conf->framerate_numerator = mfxEncParams.mfx.FrameInfo.FrameRateExtN;
        g_conf->framerate_denominator =
            mfxEncParams.mfx.FrameInfo.FrameRateExtD;
    }
    else {
        g_conf = NULL;
    }

    std::vector<mfxFrameSurface1> pEncSurfaces;
    std::vector<mfxU8> surfaceBuffersData;
    mfxU16 nEncSurfNum = 0;

    if (params.memoryMode == MEM_MODE_EXTERNAL) {
        // Query number required surfaces for encoder
        mfxFrameAllocRequest EncRequest = { 0 };
        sts = MFXVideoENCODE_QueryIOSurf(session, &mfxEncParams, &EncRequest);

        if (sts != MFX_ERR_NONE) {
            fclose(fSource);
            fclose(fSink);
            puts("QueryIOSurf error");
            return 1;
        }

        // Determine the required number of surfaces for encoder
        if (mfxEncParams.mfx.CodecId == MFX_CODEC_JPEG) {
            // QueryIOSurf returns always NumFrameSuggested value 3 regardless of any condition, it's not safe to MJPEG.
            // Need larger number of surfaces than suggested to avoid lack of surface until we improve.
            nEncSurfNum = 15;
        }
        else
            nEncSurfNum = EncRequest.NumFrameSuggested;

        // Allocate surfaces for encoder
        // - Frame surface array keeps pointers all surface planes and general frame info
        mfxU32 surfaceSize =
            GetSurfaceSize(params.srcFourCC, params.srcHeight, params.srcWidth);
        if (surfaceSize == 0) {
            fclose(fSource);
            fclose(fSink);
            puts("Surface size is wrong");
            return 1;
        }

        surfaceBuffersData.resize(surfaceSize * nEncSurfNum);
        mfxU8* surfaceBuffers = surfaceBuffersData.data();

        mfxU16 surfW = (params.srcFourCC == MFX_FOURCC_I010)
                           ? params.srcWidth * 2
                           : params.srcWidth;
        mfxU16 surfH = params.srcHeight;

        // Allocate surface headers (mfxFrameSurface1) for encoder
        pEncSurfaces.resize(nEncSurfNum);
        for (mfxI32 i = 0; i < nEncSurfNum; i++) {
            memset(&pEncSurfaces[i], 0, sizeof(mfxFrameSurface1));
            pEncSurfaces[i].Info   = mfxEncParams.mfx.FrameInfo;
            pEncSurfaces[i].Data.Y = &surfaceBuffers[surfaceSize * i];

            pEncSurfaces[i].Data.U = pEncSurfaces[i].Data.Y + surfW * surfH;
            pEncSurfaces[i].Data.V =
                pEncSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
            pEncSurfaces[i].Data.Pitch = surfW;
        }
    }

    // Initialize the Media SDK encoder
    sts = MFXVideoENCODE_Init(session, &mfxEncParams);
    if (sts != MFX_ERR_NONE) {
        fclose(fSource);
        fclose(fSink);
        puts("could not initialize encode");
        return 1;
    }

    // Prepare Media SDK bit stream buffer
    mfxBitstream mfxBS   = { 0 };
    mfxBS.MaxLength      = 2000000;
    mfxU8* output_buffer = new mfxU8[mfxBS.MaxLength];
    mfxBS.Data           = output_buffer;

    double encode_time = 0;
    double sync_time   = 0;

    // Start encoding the frames
    mfxI32 nEncSurfIdx = 0;
    mfxSyncPoint syncp;
    mfxU32 framenum = 0;

    puts("start encoding");

    // Stage 1: Main encoding loop
    bool isdraining = false;
    while (MFX_ERR_NONE <= sts || MFX_ERR_MORE_DATA == sts) {
        mfxFrameSurface1* pmfxWorkSurface = nullptr;

        if (!isdraining) {
            if (params.memoryMode == MEM_MODE_EXTERNAL) {
                nEncSurfIdx = GetFreeSurfaceIndex(
                    pEncSurfaces); // Find free frame surface

                if (nEncSurfIdx == MFX_ERR_NOT_FOUND) {
                    if (output_buffer)
                        delete[] output_buffer;
                    fclose(fSource);
                    fclose(fSink);
                    puts("no available surface");
                    return 1;
                }

                pmfxWorkSurface = &pEncSurfaces[nEncSurfIdx];
            }
            else if (params.memoryMode == MEM_MODE_INTERNAL) {
                sts = MFXMemory_GetSurfaceForEncode(session, &pmfxWorkSurface);
                if (sts) {
                    if (output_buffer)
                        delete[] output_buffer;
                    fclose(fSource);
                    fclose(fSink);
                    printf(
                        "Unknown error in MFXMemory_GetSurfaceForEncode, sts = %d\n",
                        sts);
                    return 1;
                }

                pmfxWorkSurface->FrameInterface->Map(pmfxWorkSurface,
                                                     MFX_MAP_READ);
            }

            if (pmfxWorkSurface)
                sts = LoadRawFrame(pmfxWorkSurface, fSource);
            else {
                sts = MFX_ERR_UNKNOWN;
            }

            if (sts == MFX_ERR_MORE_DATA) {
                isdraining = true;
            }
            else if (sts) {
                if (output_buffer)
                    delete[] output_buffer;
                fclose(fSource);
                fclose(fSink);
                printf("Unknown error in LoadRawFrame()\n");
                return 1;
            }
        }

        for (;;) {
            // Encode a frame asychronously (returns immediately)
            auto t0 = std::chrono::high_resolution_clock::now();
            sts     = MFXVideoENCODE_EncodeFrameAsync(
                session,
                NULL,
                (isdraining ? NULL : pmfxWorkSurface),
                &mfxBS,
                &syncp);
            auto t1 = std::chrono::high_resolution_clock::now();
            encode_time +=
                std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0)
                    .count();

            if (MFX_ERR_NONE < sts && syncp) {
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

        if (params.memoryMode == MEM_MODE_INTERNAL) {
            if (pmfxWorkSurface) {
                pmfxWorkSurface->FrameInterface->Unmap(pmfxWorkSurface);
                pmfxWorkSurface->FrameInterface->Release(pmfxWorkSurface);
            }
        }

        // all done
        if (MFX_ERR_MORE_DATA == sts && isdraining == true)
            break;

        if (MFX_ERR_NONE == sts) {
            auto t0 = std::chrono::high_resolution_clock::now();
            sts     = MFXVideoCORE_SyncOperation(
                session,
                syncp,
                60000); // Synchronize. Wait until encoded frame is ready
            auto t1 = std::chrono::high_resolution_clock::now();
            sync_time +=
                std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0)
                    .count();
            ++framenum;
            WriteEncodedStream(framenum,
                               g_conf,
                               mfxBS.Data + mfxBS.DataOffset,
                               mfxBS.DataLength,
                               params.dstFourCC,
                               fSink);
            mfxBS.DataLength = 0;
        }
    }

    if (params.dstFourCC == MFX_CODEC_AV1) {
        UpdateTotalNumberFrameInfo(fSink, framenum);
    }

    if (g_conf)
        delete g_conf;

    // Clean up resources
    //  - It is recommended to close Media SDK components first, before releasing allocated surfaces, since
    //    some surfaces may still be locked by internal Media SDK resources.
    MFXVideoENCODE_Close(session);
    MFXClose(session);

    fclose(fSource);
    fclose(fSink);

    delete[] output_buffer;

    printf("encoded %d frames\n", framenum);
    if (framenum) {
        printf("encode avg=%f usec, sync avg=%f usec\n",
               encode_time / framenum,
               sync_time / framenum);
    }

    return 0;
}

inline void mem_put_le16(void* vmem, mfxU32 val) {
    mfxU8* mem = reinterpret_cast<mfxU8*>(vmem);

    mem[0] = (mfxU8)((val >> 0) & 0xff);
    mem[1] = (mfxU8)((val >> 8) & 0xff);
}

inline void mem_put_le32(void* vmem, mfxU32 val) {
    mfxU8* mem = reinterpret_cast<mfxU8*>(vmem);

    mem[0] = (mfxU8)((val >> 0) & 0xff);
    mem[1] = (mfxU8)((val >> 8) & 0xff);
    mem[2] = (mfxU8)((val >> 16) & 0xff);
    mem[3] = (mfxU8)((val >> 24) & 0xff);
}

void WriteIVF_StreamHeader(const AV1EncConfig* conf, FILE* f) {
    char header[32] = { 0 };

    header[0] = 'D';
    header[1] = 'K';
    header[2] = 'I';
    header[3] = 'F';

    mem_put_le16(header + 4, 0); // version
    mem_put_le16(header + 6, 32); // header size
    mem_put_le32(header + 8, AV1_FOURCC); // fourcc
    mem_put_le16(header + 12, conf->width); // width
    mem_put_le16(header + 14, conf->height); // height
    mem_put_le32(header + 16, conf->framerate_numerator); // rate
    mem_put_le32(header + 20, conf->framerate_denominator); // scale
    mem_put_le32(header + 24, 0); // length
    mem_put_le32(header + 28, 0); // unused

    fwrite(header, 1, 32, f);
    return;
}

void WriteIVF_FrameHeader(mfxU32 byte_count, mfxU64 pts, FILE* f) {
    char header[12] = { 0 };

    mem_put_le32(header, (mfxU32)byte_count);
    mem_put_le32(header + 4, (mfxU32)(pts & 0xFFFFFFFF));
    mem_put_le32(header + 8, (mfxU32)(pts >> 32));

    fwrite(header, 1, 12, f);
}

void WriteEncodedStream(mfxU32 nframe,
                        void* conf,
                        mfxU8* data,
                        mfxU32 length,
                        mfxU32 codecID,
                        FILE* f) {
    if (codecID == MFX_CODEC_AV1) {
        if (nframe == 1) {
            WriteIVF_StreamHeader(reinterpret_cast<AV1EncConfig*>(conf), f);
        }
        WriteIVF_FrameHeader(length, nframe - 1, f); // pts starts from 0
        fwrite(data, 1, length, f);
    }
    else {
        fwrite(data, 1, length, f);
    }
}

void UpdateTotalNumberFrameInfo(FILE* f, mfxU32 total_frames) {
    if (f) {
        fseek(f, 24, SEEK_SET);
        fwrite(&total_frames, 1, sizeof(mfxU32), f);
    }
}

mfxStatus LoadRawFrame(mfxFrameSurface1* pSurface, FILE* fSource) {
    mfxU16 w, h, i, pitch;
    mfxU32 nBytesRead;
    mfxU8* ptr;
    mfxFrameInfo* pInfo = &pSurface->Info;
    mfxFrameData* pData = &pSurface->Data;

    w = pInfo->Width;
    h = pInfo->Height;

    switch (pInfo->FourCC) {
        case MFX_FOURCC_I420:
            // read luminance plane (Y)
            pitch = pData->Pitch;
            ptr   = pData->Y;
            for (i = 0; i < h; i++) {
                nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, fSource);
                if (w != nBytesRead)
                    return MFX_ERR_MORE_DATA;
            }

            // read chrominance (U, V)
            pitch /= 2;
            h /= 2;
            w /= 2;
            ptr = pData->U;
            for (i = 0; i < h; i++) {
                nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, fSource);
                if (w != nBytesRead)
                    return MFX_ERR_MORE_DATA;
            }

            ptr = pData->V;
            for (i = 0; i < h; i++) {
                nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, fSource);
                if (w != nBytesRead)
                    return MFX_ERR_MORE_DATA;
            }
            break;
        case MFX_FOURCC_I010:
            // read luminance plane (Y)
            pitch = pData->Pitch;
            w *= 2;
            ptr = pData->Y;
            for (i = 0; i < h; i++) {
                nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, fSource);
                if (w != nBytesRead)
                    return MFX_ERR_MORE_DATA;
            }

            // read chrominance (U, V)
            pitch /= 2;
            w /= 2;
            h /= 2;
            ptr = pData->U;
            for (i = 0; i < h; i++) {
                nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, fSource);
                if (w != nBytesRead)
                    return MFX_ERR_MORE_DATA;
            }

            ptr = pData->V;
            for (i = 0; i < h; i++) {
                nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, fSource);
                if (w != nBytesRead)
                    return MFX_ERR_MORE_DATA;
            }
            break;
        default:
            break;
    }

    return MFX_ERR_NONE;
}

mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height) {
    mfxU32 nbytes = 0;

    switch (FourCC) {
        case MFX_FOURCC_I420:
            nbytes = width * height + (width >> 1) * (height >> 1) +
                     (width >> 1) * (height >> 1);
            break;
        case MFX_FOURCC_I010:
            nbytes = width * height + (width >> 1) * (height >> 1) +
                     (width >> 1) * (height >> 1);
            nbytes *= 2;
            break;
        default:
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
        *vsize = static_cast<mfxU32>(strtol(in, NULL, 10));
        if (*vsize > vmax)
            return false;
        else
            return true;
    }

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

    if (!params->infileFormat) {
        printf("ERROR - input format (-if) is required\n");
        return false;
    }

    if (!strncmp(params->infileFormat, "I420", strlen("I420"))) {
        params->srcFourCC = MFX_FOURCC_I420;
    }
    else if (!strncmp(params->infileFormat, "I010", strlen("I010"))) {
        params->srcFourCC = MFX_FOURCC_I010;
    }
    else {
        printf("ERROR - unsupported input format %s\n", params->infileFormat);
        return false;
    }

    // output format (required)
    if (!params->outfileFormat) {
        printf("ERROR - output format (-of) is required\n");
        return false;
    }

    // output format (required)
    if (!strncmp(params->outfileFormat, "H264", strlen("H264"))) {
        params->dstFourCC = MFX_CODEC_AVC;
    }
    else if (!strncmp(params->outfileFormat, "H265", strlen("H265"))) {
        params->dstFourCC = MFX_CODEC_HEVC;
    }
    else if (!strncmp(params->outfileFormat, "AV1", strlen("AV1"))) {
        params->dstFourCC = MFX_CODEC_AV1;
    }
    else if (!strncmp(params->outfileFormat, "JPEG", strlen("JPEG"))) {
        params->dstFourCC = MFX_CODEC_JPEG;
    }
    else {
        printf("ERROR - unsupported output format %s\n", params->outfileFormat);
        return false;
    }

    // Source Height required
    if (!params->srcHeight) {
        printf("ERROR - srcHeight (-sh) is required\n");
        return false;
    }

    // Source Width required
    if (!params->srcWidth) {
        printf("ERROR - srcWidth (-sw) is required\n");
        return false;
    }

    return true;
}

bool ParseArgsAndValidate(int argc, char* argv[], Params* params) {
    int idx;
    char* s;

    // init all params to 0
    memset(params, 0, sizeof(Params));

    // set any non-zero defaults
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
        else if (IS_ARG_EQ(s, "if")) {
            params->infileFormat = argv[idx++];
            str_upper(params->infileFormat,
                      static_cast<int>(
                          strlen(params->infileFormat))); // to upper case
        }
        else if (IS_ARG_EQ(s, "of")) {
            params->outfileFormat = argv[idx++];
            str_upper(params->outfileFormat,
                      static_cast<int>(
                          strlen(params->outfileFormat))); // to upper case
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
        else if (IS_ARG_EQ(s, "qp")) {
            params->qp = atoi(argv[idx++]);
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
        else {
            printf("ERROR - invalid argument: %s\n", argv[idx]);
            return false;
        }
    }

    // run basic parameter validation
    return ValidateParams(params);
}

void Usage(void) {
    printf("\nOptions - Encode:\n");
    printf("  -i      inputFile     ... input file name\n");
    printf("  -o      outputFile    ... output file name\n");
    printf("  -n      maxFrames     ... max frames to decode\n");
    printf("  -if     inputFormat   ... [i420, i010]\n");
    printf("  -of     outputFormat  ... [h264, h265, av1, jpeg]\n");
    printf("  -sh     srcHeight     ... Source Height\n");
    printf("  -sw     srcWidth      ... Source Width\n");
    printf("  -tu     targetUsage   ... TU [1-7]\n");
    printf("  -fr     frameRate     ... frames per second\n");
    printf("  -br     bitRate       ... bitrate in kbps\n");
    printf(
        "  -bm     brcMode       ... bitrate control [1=CBR, 2=VBR, 3=CQP]\n");
    printf("  -qu     quality       ... quality parameter for JPEG encoder\n");
    printf(
        "  -qp     qp            ... quantization parameter for CQP bitrate control mode\n");
    printf("  -gs     gopSize       ... GOP size\n");

    printf("\nMemory model (default = -ext)\n");
    printf("  -ext  = external memory (1.0 style)\n");
    printf("  -int  = internal memory with MFXMemory_GetSurfaceForEncode\n");

    printf("\nDispatcher (default = -dsp1)\n");
    printf("  -dsp1 = legacy dispatcher (MSDK 1.x)\n");
    printf("  -dsp2 = smart dispatcher (API 2.0)\n");

    printf("\nIn case of AV1, output will be contained with IVF headers.\n");
    printf("To view:\n");
    printf(" ffplay [out filename]\n");
    return;
}
