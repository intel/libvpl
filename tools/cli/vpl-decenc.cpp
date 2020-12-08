/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./vpl-common.h"

#define AV1_FOURCC             0x31305641
#define MAX_LENGTH             260
#define MAX_WIDTH              3840
#define MAX_HEIGHT             2160
#define MAX_BS_BUFFER_SIZE     64 * 1024 * 1024
#define DEFAULT_BS_BUFFER_SIZE 2 * 1024 * 1024

typedef struct {
    mfxU32 width;
    mfxU32 height;
    mfxU32 framerate_numerator;
    mfxU32 framerate_denominator;

} AV1EncConfig;

AV1EncConfig* g_conf = NULL;
mfxU32 repeatCount   = 0;
bool g_read_streamheader;

mfxStatus ReadStreamInfo(mfxSession session, FILE* f, mfxBitstream* bs, mfxVideoParam* param);
mfxStatus AllocateExternalMemorySurface(std::vector<mfxU8>* dec_buf,
                                        mfxFrameSurface1* surfpool,
                                        mfxFrameInfo* frame_info,
                                        mfxU16 surfnum);
mfxStatus ReadEncodedStream(mfxBitstream& bs, mfxU32 codecid, FILE* f, mfxU32 repeat);
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
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height);
int GetFreeSurfaceIndex(mfxFrameSurface1* SurfacesPool, mfxU16 nPoolSize);
char** ValidateInput(int cnt, char* in[]);
void str_upper(char* str, int l);
char* ValidateFileName(char* in);
bool ValidateSize(char* in, mfxU32* vsize, mfxU32 vmax);
bool ValidateParams(Params* params);
bool ParseArgsAndValidate(int argc, char* argv[], Params* params);
void Usage(void);
mfxStatus InitializeSession(Params* params, mfxSession* session);
void InitializeEncodeParams(Params* params,
                            mfxVideoParam* mfxDecParam,
                            mfxVideoParam* mfxEncParams);
void PrintEncParams(mfxVideoParam* mfxEncParams);
void PrintDecParams(mfxVideoParam* mfxDecParams);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        Usage();
        return 1; // return 1 as error code
    }

    bool is_draining_dec = false;
    bool is_draining_enc = false;
    bool is_stillgoing   = true;

    mfxBitstream bs_enc_out = { 0 };

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
        return sts;
    }

    printf("Dispatcher mode  = %s\n", DispatcherModeString[params.dispatcherMode]);
    printf("Memory mode      = %s\n", MemoryModeString[params.memoryMode]);
    printf("Library initialized\n");

    // prepare input bs_dec_in
    mfxBitstream bs_dec_in = { 0 };
    if (params.srcbsbufSize > 0 && params.srcbsbufSize <= MAX_BS_BUFFER_SIZE)
        bs_dec_in.MaxLength = params.srcbsbufSize;
    else {
        fclose(fSource);
        fclose(fSink);
        puts("Memory allocation error. bs_enc_out buffersize is not correct");
        return 1;
    }

    std::vector<mfxU8> input_buffer;
    input_buffer.resize(bs_dec_in.MaxLength);
    bs_dec_in.Data = input_buffer.data();

    // initialize decode parameters from stream header
    mfxVideoParam mfxDecParams = { 0 };
    mfxDecParams.mfx.CodecId   = params.srcFourCC;
    mfxDecParams.IOPattern     = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

    sts = ReadStreamInfo(session, fSource, &bs_dec_in, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        fclose(fSource);
        fclose(fSink);
        puts("Fail to read stream info");
        return sts;
    }

    // do lazy-init for AUTO mode
    if (params.memoryMode == MEM_MODE_AUTO) {
        bs_dec_in.CodecId = params.srcFourCC;
    }
    else {
        // input parameters finished, now initialize decode
        sts = MFXVideoDECODE_Init(session, &mfxDecParams);
        if (sts != MFX_ERR_NONE) {
            fclose(fSource);
            fclose(fSink);
            PrintDecParams(&mfxDecParams);
            puts("Could not initialize decode");
            return sts;
        }
    }

    mfxFrameAllocRequest DecRequest = { 0 };
    mfxU16 nSurfNumDecEnc           = 0;
    mfxFrameSurface1* surfDecEnc    = nullptr;
    int nIndex                      = -1;
    std::vector<mfxU8> DECoutbuf;

    // Initialize encoder parameters
    mfxVideoParam mfxEncParams = { 0 };
    InitializeEncodeParams(&params, &mfxDecParams, &mfxEncParams);

    // Query number of required surfaces for encode
    mfxFrameAllocRequest EncRequest = { 0 };

    if (params.memoryMode == MEM_MODE_EXTERNAL) {
        // Query number required surfaces for decoder
        DecRequest = { 0 };
        sts        = MFXVideoDECODE_QueryIOSurf(session, &mfxDecParams, &DecRequest);
        if (sts != MFX_ERR_NONE) {
            fclose(fSource);
            fclose(fSink);
            printf("[Decode] QueryIOSurf error\n");
            return sts;
        }

        sts = MFXVideoENCODE_QueryIOSurf(session, &mfxEncParams, &EncRequest);
        if (sts != MFX_ERR_NONE) {
            fclose(fSource);
            fclose(fSink);
            printf("[Encode] QueryIOSurf error\n");
            return sts;
        }

        nSurfNumDecEnc = DecRequest.NumFrameSuggested + EncRequest.NumFrameSuggested;

        surfDecEnc = new mfxFrameSurface1[nSurfNumDecEnc];
        sts        = AllocateExternalMemorySurface(&DECoutbuf,
                                            surfDecEnc,
                                            &mfxDecParams.mfx.FrameInfo,
                                            nSurfNumDecEnc);
        if (sts != MFX_ERR_NONE) {
            if (surfDecEnc)
                delete[] surfDecEnc;
            fclose(fSource);
            fclose(fSink);
            puts("External memory allocation error.");
            return sts;
        }
    }

    if (params.dstFourCC == MFX_CODEC_AV1) {
        g_conf                        = new AV1EncConfig;
        g_conf->width                 = params.srcWidth;
        g_conf->height                = params.srcHeight;
        g_conf->framerate_numerator   = mfxEncParams.mfx.FrameInfo.FrameRateExtN;
        g_conf->framerate_denominator = mfxEncParams.mfx.FrameInfo.FrameRateExtD;
    }
    else {
        g_conf = NULL;
    }

    // Initialize the encoder
    sts = MFXVideoENCODE_Query(session, &mfxEncParams, &mfxEncParams);
    if (sts != MFX_ERR_NONE) {
        fclose(fSource);
        fclose(fSink);
        delete[] surfDecEnc;
        PrintEncParams(&mfxEncParams);
        puts("could not query encode");
        return 1;
    }

    sts = MFXVideoENCODE_Init(session, &mfxEncParams);
    if (sts != MFX_ERR_NONE) {
        if (fSource) {
            fclose(fSource);
            fSource = NULL;
        }
        if (fSink) {
            fclose(fSink);
            fSink = NULL;
        }

        PrintEncParams(&mfxEncParams);
        puts("could not initialize encode");
        if (surfDecEnc)
            delete[] surfDecEnc;
        return 1;
    }

    // Prepare output bs_enc_out and start encoding
    bs_enc_out.MaxLength = DEFAULT_BS_BUFFER_SIZE;
    bs_enc_out.Data      = reinterpret_cast<mfxU8*>(malloc(bs_enc_out.MaxLength * sizeof(mfxU8)));

    // Start processing the frames
    int framenum                      = 0;
    mfxSyncPoint syncp                = { 0 };
    mfxFrameSurface1* pmfxWorkSurface = nullptr;
    mfxFrameSurface1* dec_surface_out = nullptr;

    printf("start decoding\n");

    while (is_stillgoing == true) {
        sts = MFX_ERR_NONE;

        if (params.memoryMode == MEM_MODE_EXTERNAL) {
            nIndex = GetFreeSurfaceIndex(surfDecEnc, nSurfNumDecEnc);
            if (nIndex < 0) {
                printf("There is no free surface: sts=%d\n", nIndex);
                exit(1);
            }
            pmfxWorkSurface = &surfDecEnc[nIndex];
        }
        else if (params.memoryMode == MEM_MODE_INTERNAL) {
            sts = MFXMemory_GetSurfaceForDecode(session, &pmfxWorkSurface);
            if (sts) {
                printf("Error in GetSurfaceForDecode: sts=%d\n", sts);
                exit(1);
            }
        }
        else if (params.memoryMode == MEM_MODE_AUTO) {
            pmfxWorkSurface = nullptr;
        }

        // Read input stream for decode
        if (is_draining_dec == false) {
            sts = ReadEncodedStream(bs_dec_in, params.srcFourCC, fSource, params.repeat);
            if (sts != MFX_ERR_NONE) // No more data to read, start decode draining mode
                is_draining_dec = true;
        }

        if (is_draining_enc == false) {
            sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                                  (is_draining_dec == true) ? nullptr : &bs_dec_in,
                                                  pmfxWorkSurface,
                                                  &dec_surface_out,
                                                  &syncp);
        }

        switch (sts) {
            case MFX_ERR_NONE: // Got 1 decoded frame
                break;
            case MFX_ERR_MORE_DATA: // The function requires more bs_enc_out at input before decoding can proceed
                if (params.memoryMode == MEM_MODE_INTERNAL)
                    pmfxWorkSurface->FrameInterface->Release(pmfxWorkSurface);

                if (is_draining_dec == true)
                    is_draining_enc =
                        true; // No more data to drain from decoder, start encode draining mode
                else
                    continue; // read more data
                break;
            case MFX_ERR_MORE_SURFACE: // feed a fresh surface to decode
                if (params.memoryMode == MEM_MODE_EXTERNAL) {
                    nIndex = GetFreeSurfaceIndex(surfDecEnc, nSurfNumDecEnc);
                }
                else {
                    printf(
                        "Error - MFX_ERR_MORE_SURFACE returned with internal memory allocation\n");
                    exit(1);
                }
                break;
            case MFX_ERR_INCOMPATIBLE_VIDEO_PARAM:
                if (params.memoryMode == MEM_MODE_EXTERNAL) {
                    MFXVideoDECODE_GetVideoParam(session, &mfxDecParams);
                    sts = AllocateExternalMemorySurface(&DECoutbuf,
                                                        surfDecEnc,
                                                        &mfxDecParams.mfx.FrameInfo,
                                                        nSurfNumDecEnc);
                    if (sts != MFX_ERR_NONE) {
                        if (fSource) {
                            fclose(fSource);
                        }
                        if (fSink) {
                            fclose(fSink);
                        }
                        if (bs_enc_out.Data) {
                            free(bs_enc_out.Data);
                        }
                        puts("External memory allocation error after resolution change.");
                        return sts;
                    }

                    nIndex          = GetFreeSurfaceIndex(surfDecEnc, nSurfNumDecEnc);
                    pmfxWorkSurface = &surfDecEnc[nIndex];

                    sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                                          nullptr,
                                                          pmfxWorkSurface,
                                                          &dec_surface_out,
                                                          &syncp);

                    break;
                }
                else {
                    printf("Error in DecodeFrameAsync: sts=%d\n", sts);
                    exit(1);
                }
                break;
            default:
                printf("unknown status %d\n", sts);
                is_stillgoing = false;
                continue;
        }

        sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                              NULL,
                                              (is_draining_enc == true) ? NULL : dec_surface_out,
                                              &bs_enc_out,
                                              &syncp);

        if (params.memoryMode == MEM_MODE_INTERNAL) {
            if (is_draining_enc == false && dec_surface_out) {
                mfxStatus sts_r = dec_surface_out->FrameInterface->Release(dec_surface_out);
                if (sts_r != MFX_ERR_NONE) {
                    printf("mfxFrameSurfaceInterface->Release failed\n");
                    exit(1);
                }
            }
        }

        switch (sts) {
            case MFX_ERR_NONE:
                // MFX_ERR_NONE and syncp_enc indicate output is available
                if (syncp) {
                    // Encode output is not available on CPU until sync operation completes
                    sts = MFXVideoCORE_SyncOperation(session, syncp, 60000);
                    if (sts) {
                        puts("MFXVideoCORE_SyncOperation error");
                        return 1;
                    }
                    ++framenum;
                    if (!IS_ARG_EQ(params.outfileName, "null")) {
                        WriteEncodedStream(framenum,
                                           g_conf,
                                           bs_enc_out.Data + bs_enc_out.DataOffset,
                                           bs_enc_out.DataLength,
                                           params.dstFourCC,
                                           fSink);
                    }
                    bs_enc_out.DataLength = 0;
                }
                break;
            case MFX_ERR_MORE_DATA:
                // The function requires more data to generate any output
                if (is_draining_enc == true)
                    is_stillgoing = false;
                else
                    continue;
                break;
            default:
                printf("unknown status %d\n", sts);
                is_stillgoing = false;
                break;
        }
    }

    printf("Processed %d frames\n", framenum);

    if (bs_enc_out.Data)
        free(bs_enc_out.Data);

    if (fSource) {
        fclose(fSource);
        fSource = NULL;
    }

    if (fSink) {
        fclose(fSink);
        fSink = NULL;
    }

    if (params.memoryMode == MEM_MODE_EXTERNAL) {
        if (surfDecEnc) {
            delete[] surfDecEnc;
        }
    }

    MFXVideoDECODE_Close(session);
    MFXVideoENCODE_Close(session);
    MFXClose(session);

    return 0;
}

mfxStatus AllocateExternalMemorySurface(std::vector<mfxU8>* dec_buf,
                                        mfxFrameSurface1* surfpool,
                                        mfxFrameInfo* frame_info,
                                        mfxU16 surfnum) {
    // initialize surface pool for decode (I420 format)
    mfxU32 surfaceSize = GetSurfaceSize(frame_info->FourCC, frame_info->Width, frame_info->Height);
    if (!surfaceSize)
        return MFX_ERR_MEMORY_ALLOC;

    size_t framePoolBufSize = static_cast<size_t>(surfaceSize) * surfnum;
    dec_buf->resize(framePoolBufSize);
    mfxU8* decout = dec_buf->data();

    mfxU16 surfW =
        (frame_info->FourCC == MFX_FOURCC_I010) ? frame_info->Width * 2 : frame_info->Width;
    mfxU16 surfH = frame_info->Height;

    for (mfxU32 i = 0; i < surfnum; i++) {
        surfpool[i]            = { 0 };
        surfpool[i].Info       = *frame_info;
        size_t buf_offset      = static_cast<size_t>(i) * surfaceSize;
        surfpool[i].Data.Y     = decout + buf_offset;
        surfpool[i].Data.U     = decout + buf_offset + (surfW * surfH);
        surfpool[i].Data.V     = surfpool[i].Data.U + ((surfW / 2) * (surfH / 2));
        surfpool[i].Data.Pitch = surfW;
    }

    return MFX_ERR_NONE;
}

mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height) {
    mfxU32 nbytes = 0;

    switch (FourCC) {
        case MFX_FOURCC_I420:
            nbytes = width * height + (width >> 1) * (height >> 1) + (width >> 1) * (height >> 1);
            break;

        case MFX_FOURCC_I010:
            nbytes = width * height + (width >> 1) * (height >> 1) + (width >> 1) * (height >> 1);
            nbytes *= 2;
            break;

        case MFX_FOURCC_RGB4:
            nbytes = width * height * 4;
        default:
            break;
    }

    return nbytes;
}

int GetFreeSurfaceIndex(mfxFrameSurface1* SurfacesPool, mfxU16 nPoolSize) {
    for (mfxU16 i = 0; i < nPoolSize; i++) {
        if (0 == SurfacesPool[i].Data.Locked)
            return i;
    }
    return MFX_ERR_NOT_FOUND;
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
        printf("ERROR - output file name (-i) is required\n");
        return false;
    }

    // input format (required)
    if (!params->infileFormat) {
        printf("ERROR - input format (-if) is required\n");
        return false;
    }

    // input format (required)
    if (strncmp(params->infileFormat, "H264", strlen("H264")) == 0) {
        params->srcFourCC = MFX_CODEC_AVC;
    }
    else if (strncmp(params->infileFormat, "H265", strlen("H265")) == 0) {
        params->srcFourCC = MFX_CODEC_HEVC;
    }
    else if (strncmp(params->infileFormat, "AV1", strlen("AV1")) == 0) {
        params->srcFourCC = MFX_CODEC_AV1;
    }
    else if (strncmp(params->infileFormat, "JPEG", strlen("JPEG")) == 0) {
        params->srcFourCC = MFX_CODEC_JPEG;
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

    // default bitstream buffer size (input)
    if (params->srcbsbufSize == 0) {
        params->srcbsbufSize = DEFAULT_BS_BUFFER_SIZE;
    }

    if (params->memoryMode == MEM_MODE_EXTERNAL) {
        puts("[external memory mode]");
    }
    else if (params->memoryMode == MEM_MODE_INTERNAL) {
        puts("[internal memory mode]");
    }
    else {
        puts("[auto memory mode]");
    }

    // if this param is set, it will be used for decode res-change testing.
    // and will select this resolution from decoded frame to save.
    if (params->outResolution) {
        char* position_ptr = NULL;
        position_ptr       = strchr(params->outResolution, 'X');
        if (position_ptr) {
            if (!ValidateSize(params->outResolution, &params->outWidth, MAX_WIDTH))
                return false;
            position_ptr++;
            if (!ValidateSize(position_ptr, &params->outHeight, MAX_HEIGHT))
                return false;

            printf("only %dx%d res of decoded frames will be saved\n",
                   params->outWidth,
                   params->outHeight);
        }
        else {
            puts("wrong separator for output resolution(-o_res)\n");
            return false;
        }
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
                      static_cast<int>(strlen(params->infileFormat))); // to upper case
        }
        else if (IS_ARG_EQ(s, "of")) {
            params->outfileFormat = argv[idx++];
            str_upper(params->outfileFormat,
                      static_cast<int>(strlen(params->outfileFormat))); // to upper case
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
        else {
            printf("ERROR - invalid argument: %s\n", argv[idx]);
            return false;
        }
    }

    // run basic parameter validation
    return ValidateParams(params);
}

void Usage(void) {
    printf("\nOptions - VPP+Encode:\n");
    printf("  -i      inputFile     ... input file name\n");
    printf("  -o      outputFile    ... output file name ('null' for no output file)\n");
    printf("  -n      maxFrames     ... max frames to process\n");
    printf("  -rp     repeat        ... number of times to repeat encoding\n");
    printf("  -sbs    bsbufSize     ... source bitstream buffer size (bytes)\n");

    printf("  -if     inputFormat   ... [h264, h265, av1, jpeg]\n");
    printf("  -of     outputFormat  ... [h264, h265, av1, jpeg]\n");

    printf("  -tu     targetUsage   ... TU [1-7]\n");
    printf("  -fr     frameRate     ... frames per second\n");
    printf("  -br     bitRate       ... bitrate in kbps\n");
    printf("  -bm     brcMode       ... bitrate control [1=CBR, 2=VBR, 3=CQP]\n");
    printf("  -qu     quality       ... quality parameter for JPEG encoder\n");
    printf("  -qp     qp            ... quantization parameter for CQP bitrate control mode\n");
    printf("  -gs     gopSize       ... GOP size\n");

    printf("\nMemory model (default = -ext)\n");
    printf("  -ext  = external memory (1.0 style)\n");
    printf("  -int  = internal memory with MFXMemory_GetSurfaceForVPP\n");

    printf("\nDispatcher (default = -dsp1)\n");
    printf("  -dsp1 = legacy dispatcher (MSDK 1.x)\n");
    printf("  -dsp2 = smart dispatcher (API 2.0)\n");

    printf("\nIn case of AV1, output will be contained with IVF headers.\n");
    printf("To view:\n");
    printf(" ffplay [out filename]\n");
}

mfxStatus InitializeSession(Params* params, mfxSession* session) {
    std::string diagnoseText = "";
    mfxStatus sts            = MFX_ERR_UNDEFINED_BEHAVIOR;
    if (params->dispatcherMode == DISPATCHER_MODE_VPL_20) {
        diagnoseText = "InitNewDispatcher with WSTYPE_VPP while in DISPATCHER_MODE_VPL_20";
        sts          = InitNewDispatcher(WSTYPE_DECENC, params, *(&session));
    }
    else if (params->dispatcherMode == DISPATCHER_MODE_LEGACY) {
        diagnoseText = "MFXInitEx with MFX_IMPL_SOFTWARE while in DISPATCHER_MODE_LEGACY";
        // initialize session
        mfxInitParam initPar   = { 0 };
        initPar.Version.Major  = 2;
        initPar.Version.Minor  = 0;
        initPar.Implementation = MFX_IMPL_SOFTWARE;

        sts = MFXInitEx(initPar, *(&session));
    }
    else {
        diagnoseText = "Invalid dispatcher mode";
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

void InitializeEncodeParams(Params* params,
                            mfxVideoParam* mfxDecParams,
                            mfxVideoParam* mfxEncParams) {
    (*mfxEncParams).mfx.CodecId           = params->dstFourCC;
    (*mfxEncParams).mfx.TargetUsage       = params->targetUsage;
    (*mfxEncParams).mfx.TargetKbps        = params->bitRate;
    (*mfxEncParams).mfx.RateControlMethod = params->brcMode;
    if ((*mfxEncParams).mfx.RateControlMethod == MFX_RATECONTROL_CQP) {
        (*mfxEncParams).mfx.QPI = params->qp;
        (*mfxEncParams).mfx.QPB = params->qp;
        (*mfxEncParams).mfx.QPP = params->qp;
    }
    (*mfxEncParams).mfx.GopPicSize = params->gopSize;
    (*mfxEncParams).mfx.GopRefDist = params->keyFrameDist;
    if ((*mfxEncParams).mfx.CodecId == MFX_CODEC_JPEG)
        (*mfxEncParams).mfx.Quality = params->quality;
    (*mfxEncParams).mfx.FrameInfo.FrameRateExtN = (params->frameRate) ? params->frameRate : 30;
    (*mfxEncParams).mfx.FrameInfo.FrameRateExtD = 1;
    (*mfxEncParams).mfx.FrameInfo.FourCC        = mfxDecParams->mfx.FrameInfo.FourCC;
    (*mfxEncParams).mfx.FrameInfo.ChromaFormat  = (mfxDecParams->mfx.FrameInfo.ChromaFormat)
                                                     ? mfxDecParams->mfx.FrameInfo.ChromaFormat
                                                     : MFX_CHROMAFORMAT_YUV420;
    (*mfxEncParams).mfx.FrameInfo.PicStruct = (mfxDecParams->mfx.FrameInfo.PicStruct)
                                                  ? mfxDecParams->mfx.FrameInfo.PicStruct
                                                  : MFX_PICSTRUCT_PROGRESSIVE;
    (*mfxEncParams).mfx.FrameInfo.CropX  = 0;
    (*mfxEncParams).mfx.FrameInfo.CropY  = 0;
    (*mfxEncParams).mfx.FrameInfo.CropW  = mfxDecParams->mfx.FrameInfo.Width;
    (*mfxEncParams).mfx.FrameInfo.CropH  = mfxDecParams->mfx.FrameInfo.Height;
    (*mfxEncParams).mfx.FrameInfo.Width  = mfxDecParams->mfx.FrameInfo.Width;
    (*mfxEncParams).mfx.FrameInfo.Height = mfxDecParams->mfx.FrameInfo.Height;

    if (mfxDecParams->mfx.FrameInfo.FourCC == MFX_FOURCC_I010) {
        (*mfxEncParams).mfx.FrameInfo.BitDepthLuma   = 10;
        (*mfxEncParams).mfx.FrameInfo.BitDepthChroma = 10;
        (*mfxEncParams).mfx.FrameInfo.Shift          = 1;
        if (params->dstFourCC == MFX_CODEC_HEVC) {
            (*mfxEncParams).mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN10;
        }
        else if (params->dstFourCC == MFX_CODEC_AVC) {
            (*mfxEncParams).mfx.CodecProfile = MFX_PROFILE_AVC_HIGH10;
        }
    }

    (*mfxEncParams).IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
}

void PrintDecParams(mfxVideoParam* mfxDecParams) {
    std::cout << "CodecId=" << mfxDecParams->mfx.CodecId << " | ";
    std::cout << "TargetUsage=" << mfxDecParams->mfx.TargetUsage << " | ";
    std::cout << "TargetKbps=" << mfxDecParams->mfx.TargetKbps << " | ";
    std::cout << "RateControlMethod=" << mfxDecParams->mfx.RateControlMethod << " | ";
    std::cout << "GopPicSize=" << mfxDecParams->mfx.GopPicSize << " | ";
    std::cout << "GopRefDist=" << mfxDecParams->mfx.GopRefDist << " | " << std::endl;
    std::cout << "CodecId=" << mfxDecParams->mfx.CodecId << " | ";
    std::cout << "CodecProfile=" << mfxDecParams->mfx.CodecProfile << " | ";
    std::cout << "CodecLevel=" << mfxDecParams->mfx.CodecLevel << " | ";
    std::cout << "JPEGChromaFormat=" << mfxDecParams->mfx.JPEGChromaFormat << " | ";
    std::cout << "JPEGColorFormat=" << mfxDecParams->mfx.JPEGColorFormat << " | ";
    std::cout << "Rotation=" << mfxDecParams->mfx.Rotation << " | " << std::endl;
    std::cout << "PicStruct=" << mfxDecParams->mfx.FrameInfo.PicStruct << " | ";
    std::cout << "Shift=" << mfxDecParams->mfx.FrameInfo.Shift << " | ";
    std::cout << "BitDepthLuma=" << mfxDecParams->mfx.FrameInfo.BitDepthLuma << " | ";
    std::cout << "BitDepthChroma=" << mfxDecParams->mfx.FrameInfo.BitDepthChroma << " | ";
    std::cout << "ChromaFormat=" << mfxDecParams->mfx.FrameInfo.ChromaFormat << " | ";
    std::cout << "FourCC=" << mfxDecParams->mfx.FrameInfo.FourCC << " | " << std::endl;
    std::cout << "Width=" << mfxDecParams->mfx.FrameInfo.Width << " | ";
    std::cout << "Height=" << mfxDecParams->mfx.FrameInfo.Height << " | ";
    std::cout << "CropX=" << mfxDecParams->mfx.FrameInfo.CropX << " | ";
    std::cout << "CropY=" << mfxDecParams->mfx.FrameInfo.CropY << " | ";
    std::cout << "CropH=" << mfxDecParams->mfx.FrameInfo.CropH << " | ";
    std::cout << "CropW=" << mfxDecParams->mfx.FrameInfo.CropW << " | ";
    std::cout << "AspectRatioH=" << mfxDecParams->mfx.FrameInfo.AspectRatioH << " | ";
    std::cout << "AspectRatioW=" << mfxDecParams->mfx.FrameInfo.AspectRatioW << " | " << std::endl;
    std::cout << "FrameRateExtD=" << mfxDecParams->mfx.FrameInfo.FrameRateExtD << " | ";
    std::cout << "FrameRateExtN=" << mfxDecParams->mfx.FrameInfo.FrameRateExtN << " | "
              << std::endl;
    if (mfxDecParams->mfx.CodecId == MFX_CODEC_JPEG) {
        std::cout << "Quality=" << mfxDecParams->mfx.Quality << " | ";
    }
}

void PrintEncParams(mfxVideoParam* mfxEncParams) {
    std::cout << "CodecId=" << mfxEncParams->mfx.CodecId << " | ";
    std::cout << "TargetUsage=" << mfxEncParams->mfx.TargetUsage << " | ";
    std::cout << "TargetKbps=" << mfxEncParams->mfx.TargetKbps << " | ";
    std::cout << "RateControlMethod=" << mfxEncParams->mfx.RateControlMethod << " | ";
    std::cout << "GopPicSize=" << mfxEncParams->mfx.GopPicSize << " | ";
    std::cout << "GopRefDist=" << mfxEncParams->mfx.GopRefDist << " | " << std::endl;
    std::cout << "CodecId=" << mfxEncParams->mfx.CodecId << " | ";
    std::cout << "CodecProfile=" << mfxEncParams->mfx.CodecProfile << " | ";
    std::cout << "CodecLevel=" << mfxEncParams->mfx.CodecLevel << " | ";
    std::cout << "JPEGChromaFormat=" << mfxEncParams->mfx.JPEGChromaFormat << " | ";
    std::cout << "JPEGColorFormat=" << mfxEncParams->mfx.JPEGColorFormat << " | ";
    std::cout << "Rotation=" << mfxEncParams->mfx.Rotation << " | " << std::endl;
    std::cout << "PicStruct=" << mfxEncParams->mfx.FrameInfo.PicStruct << " | ";
    std::cout << "Shift=" << mfxEncParams->mfx.FrameInfo.Shift << " | ";
    std::cout << "BitDepthLuma=" << mfxEncParams->mfx.FrameInfo.BitDepthLuma << " | ";
    std::cout << "BitDepthChroma=" << mfxEncParams->mfx.FrameInfo.BitDepthChroma << " | ";
    std::cout << "ChromaFormat=" << mfxEncParams->mfx.FrameInfo.ChromaFormat << " | ";
    std::cout << "FourCC=" << mfxEncParams->mfx.FrameInfo.FourCC << " | " << std::endl;
    std::cout << "Width=" << mfxEncParams->mfx.FrameInfo.Width << " | ";
    std::cout << "Height=" << mfxEncParams->mfx.FrameInfo.Height << " | ";
    std::cout << "CropX=" << mfxEncParams->mfx.FrameInfo.CropX << " | ";
    std::cout << "CropY=" << mfxEncParams->mfx.FrameInfo.CropY << " | ";
    std::cout << "CropH=" << mfxEncParams->mfx.FrameInfo.CropH << " | ";
    std::cout << "CropW=" << mfxEncParams->mfx.FrameInfo.CropW << " | ";
    std::cout << "AspectRatioH=" << mfxEncParams->mfx.FrameInfo.AspectRatioH << " | ";
    std::cout << "AspectRatioW=" << mfxEncParams->mfx.FrameInfo.AspectRatioW << " | " << std::endl;
    std::cout << "FrameRateExtD=" << mfxEncParams->mfx.FrameInfo.FrameRateExtD << " | ";
    std::cout << "FrameRateExtN=" << mfxEncParams->mfx.FrameInfo.FrameRateExtN << " | "
              << std::endl;
    if (mfxEncParams->mfx.CodecId == MFX_CODEC_JPEG) {
        std::cout << "Quality=" << mfxEncParams->mfx.Quality << " | ";
    }
    if (mfxEncParams->mfx.RateControlMethod == MFX_RATECONTROL_CQP) {
        std::cout << "RateControlMethod=" << mfxEncParams->mfx.QPI << " | ";
        std::cout << "RateControlMethod=" << mfxEncParams->mfx.QPB << " | ";
        std::cout << "RateControlMethod=" << mfxEncParams->mfx.QPP << " | " << std::endl;
    }
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

mfxStatus ReadEncodedStream(mfxBitstream& bs, mfxU32 codecid, FILE* f, mfxU32 repeat) {
    memmove(bs.Data, bs.Data + bs.DataOffset, bs.DataLength);
    bs.DataOffset = 0;

    if (codecid == MFX_CODEC_AV1) {
        // spec for IVF headers
        // https://wiki.multimedia.cx/index.php/IVF

        // extract AV1 elementary stream from IVF file.
        // should remove stream header and frame header.
        // and use the frame size information to read AV1 frame.
        mfxU32 nBytesInFrame = 0;
        mfxU64 nTimeStamp    = 0;
        mfxU32 nBytesRead    = 0;

        // read stream header, only once at the beginning, 32 bytes
        if (g_read_streamheader == false) {
            mfxU8 header[32] = { 0 };
            nBytesRead       = (mfxU32)fread(header, 1, 32, f);
            if (nBytesRead == 0)
                return MFX_ERR_MORE_DATA;
            g_read_streamheader = true;
        }

        bs.DataLength = 0;

        // read frame header and parse frame data
        while (!feof(f)) {
            nBytesRead = (mfxU32)fread(&nBytesInFrame, 1, 4, f);
            if (nBytesInFrame == 0 || nBytesInFrame > bs.MaxLength)
                return MFX_ERR_ABORTED;
            if (nBytesRead == 0)
                break;

            // check whether buffer is over if we read this frame or not
            if (bs.DataLength + nBytesInFrame > bs.MaxLength) {
                // set file pointer location back to -4
                // so, the access pointer for next frame will be the "bytesinframe" location and break
                fseek(f, -4, SEEK_CUR);
                break;
            }

            nBytesRead = (mfxU32)fread(&nTimeStamp, 1, 8, f);
            if (nBytesRead == 0)
                return MFX_ERR_MORE_DATA;

            nBytesRead = (mfxU32)fread(bs.Data + bs.DataLength, 1, nBytesInFrame, f);
            if (nBytesRead == 0)
                return MFX_ERR_MORE_DATA;

            bs.DataLength += nBytesRead;
        }
    }
    else {
        bs.DataLength +=
            static_cast<mfxU32>(fread(bs.Data + bs.DataLength, 1, bs.MaxLength - bs.DataLength, f));

        while (feof(f) && repeat > 0 && repeatCount <= repeat) {
            if (repeatCount == repeat)
                return MFX_ERR_NONE;

            // The end-of-file and error internal indicators associated to the stream are cleared after a successful call to this function,
            // and all effects from previous calls to ungetc on this stream are dropped.
            rewind(f);

            repeatCount++;
        }
    }

    if (bs.DataLength == 0)
        return MFX_ERR_MORE_DATA;

    return MFX_ERR_NONE;
}

mfxStatus ReadStreamInfo(mfxSession session, FILE* f, mfxBitstream* bs, mfxVideoParam* param) {
    mfxStatus sts = MFX_ERR_NONE;
    // Decode few frames to get the basic stream information
    // Width and height of input stream will be set to vpp in
    ReadEncodedStream(*bs, param->mfx.CodecId, f, 0);
    sts = MFXVideoDECODE_DecodeHeader(session, bs, param);
    if (sts != MFX_ERR_NONE) {
        printf("MFXDecodeHeader failed\n");
        return sts;
    }
    else {
        bs->DataLength = 0;
        rewind(f);
        if (param->mfx.CodecId == MFX_CODEC_AV1)
            g_read_streamheader = false;
    }

    //only MFX_CHROMAFORMAT_YUV420 in I420 and I010 colorspaces allowed
    switch (param->mfx.FrameInfo.FourCC) {
        case MFX_FOURCC_I420:
            if (param->mfx.FrameInfo.BitDepthLuma && (param->mfx.FrameInfo.BitDepthLuma != 8)) {
                printf("Unsupported Luma Bit Depth for I420\n");
                return MFX_ERR_INVALID_VIDEO_PARAM;
            }

            if (param->mfx.FrameInfo.ChromaFormat &&
                (param->mfx.FrameInfo.ChromaFormat != MFX_CHROMAFORMAT_YUV420)) {
                printf("Unsupported chroma format for I420\n");
                return MFX_ERR_INVALID_VIDEO_PARAM;
            }

            break;
        case MFX_FOURCC_I010:
            if (param->mfx.FrameInfo.BitDepthLuma && (param->mfx.FrameInfo.BitDepthLuma != 10)) {
                printf("Unsupported Luma Bit Depth for I010\n");
                return MFX_ERR_INVALID_VIDEO_PARAM;
            }

            if (param->mfx.FrameInfo.ChromaFormat &&
                (param->mfx.FrameInfo.ChromaFormat != MFX_CHROMAFORMAT_YUV420)) {
                printf("Unsupported chroma format for I010\n");
                return MFX_ERR_INVALID_VIDEO_PARAM;
            }
            break;
        default:
            printf("Unsupported FourCC\n");
            return MFX_ERR_UNSUPPORTED;
    }

    if (sts != MFX_ERR_NONE) {
        printf("Problem decoding header.  DecodeHeader sts=%d\n", sts);
        return sts;
    }
    return sts;
}