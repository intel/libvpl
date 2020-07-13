/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>
#include "vpl/mfxvideo.h"

#define ALIGN_UP(addr, size) \
    (((addr) + ((size)-1)) & (~((decltype(addr))(size)-1)))

// IVF container helper functions and definitions
#define AV1_FOURCC 0x31305641
#define MAX_WIDTH  3840
#define MAX_HEIGHT 2160
#define MAX_PATH   260

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
mfxStatus LoadRawFrame(mfxFrameSurface1* pSurface, FILE* fSource);
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height);
mfxI32 GetFreeSurfaceIndex(const std::vector<mfxFrameSurface1>& pSurfacesPool);
char* ValidateFileName(char* in);

void Usage(void);

int main(int argc, char* argv[]) {
    if (argc < 6) {
        Usage();
        return 1; // return 1 as error code
    }

    char* in_filename  = NULL;
    char* out_filename = NULL;
    mfxU32 codecID;
    if (strncmp("h265", argv[1], 4) == 0) {
        codecID = MFX_CODEC_HEVC;
        puts("h265 encoding");
    }
    else if (strncmp("av1", argv[1], 3) == 0) {
        codecID = MFX_CODEC_AV1;
        puts("av1 encoding");
    }
    else {
        printf("%s is not supported\n", argv[1]);
        Usage();
        return 1;
    }

    in_filename = ValidateFileName(argv[2]);
    if (!in_filename) {
        printf("Input filename is not valid\n");
        Usage();
        return 1;
    }

    out_filename = ValidateFileName(argv[3]);
    if (!out_filename) {
        printf("Output filename is not valid\n");
        Usage();
        return 1;
    }

    printf("opening %s\n", in_filename);
    FILE* fSource = fopen(in_filename, "rb");
    if (!fSource) {
        printf("could not open input file, %s\n", in_filename);
        return 1;
    }

    FILE* fSink = fopen(out_filename, "wb");
    if (!fSink) {
        fclose(fSource);
        printf("could not create output file, %s\n", out_filename);
        return 1;
    }

    mfxI32 isize = strtol(argv[4], NULL, 10);
    if (isize <= 0 || isize > MAX_WIDTH) {
        fclose(fSource);
        fclose(fSink);
        puts("input size is not valid\n");
        return 1;
    }
    mfxI32 inputWidth = isize;

    isize = strtol(argv[5], NULL, 10);
    if (isize <= 0 || isize > MAX_HEIGHT) {
        fclose(fSource);
        fclose(fSink);
        puts("input size is not valid\n");
        return 1;
    }
    mfxI32 inputHeight = isize;
    mfxU32 fourCC;

    if (argv[6]) {
        if (strncmp("10", argv[6], 2) == 0) {
            fourCC = MFX_FOURCC_I010;
            puts("10bit input");
        }
        else {
            Usage();
            fclose(fSource);
            fclose(fSink);
            return 1;
        }
    }
    else {
        fourCC = MFX_FOURCC_IYUV;
        puts("8bit input");
    }

    // Initialize Media SDK session
    mfxInitParam initPar   = { 0 };
    initPar.Version.Major  = 1;
    initPar.Version.Minor  = 35;
    initPar.Implementation = MFX_IMPL_SOFTWARE;

    mfxSession session;
    mfxStatus sts = MFXInitEx(initPar, &session);
    if (sts != MFX_ERR_NONE) {
        puts("MFXInitEx error. could not initialize session");
        fclose(fSource);
        fclose(fSink);
        return 1;
    }

    puts("library initialized");

    // Initialize encoder parameters
    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));
    mfxEncParams.mfx.CodecId                 = codecID;
    mfxEncParams.mfx.TargetUsage             = MFX_TARGETUSAGE_BALANCED;
    mfxEncParams.mfx.TargetKbps              = 4000;
    mfxEncParams.mfx.RateControlMethod       = MFX_RATECONTROL_VBR;
    mfxEncParams.mfx.FrameInfo.FrameRateExtN = 30;
    mfxEncParams.mfx.FrameInfo.FrameRateExtD = 1;
    mfxEncParams.mfx.FrameInfo.FourCC        = fourCC;
    mfxEncParams.mfx.FrameInfo.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    mfxEncParams.mfx.FrameInfo.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    mfxEncParams.mfx.FrameInfo.CropX         = 0;
    mfxEncParams.mfx.FrameInfo.CropY         = 0;
    mfxEncParams.mfx.FrameInfo.CropW         = inputWidth;
    mfxEncParams.mfx.FrameInfo.CropH         = inputHeight;
    // Width must be a multiple of 16
    // Height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
    mfxEncParams.mfx.FrameInfo.Width  = ALIGN_UP(inputWidth, 16);
    mfxEncParams.mfx.FrameInfo.Height = ALIGN_UP(inputHeight, 16);

    if (fourCC == MFX_FOURCC_I010) {
        mfxEncParams.mfx.FrameInfo.BitDepthLuma   = 10;
        mfxEncParams.mfx.FrameInfo.BitDepthChroma = 10;
        mfxEncParams.mfx.FrameInfo.Shift          = 1;
        if (codecID == MFX_CODEC_HEVC) {
            mfxEncParams.mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN10;
            mfxEncParams.mfx.CodecLevel   = MFX_LEVEL_HEVC_51;
        }
    }

    mfxEncParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    if (codecID == MFX_CODEC_AV1) {
        g_conf                      = new AV1EncConfig;
        g_conf->width               = inputWidth;
        g_conf->height              = inputHeight;
        g_conf->framerate_numerator = mfxEncParams.mfx.FrameInfo.FrameRateExtN;
        g_conf->framerate_denominator =
            mfxEncParams.mfx.FrameInfo.FrameRateExtD;
    }
    else {
        g_conf = NULL;
    }

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
    mfxU16 nEncSurfNum = EncRequest.NumFrameSuggested;

    // Allocate surfaces for encoder
    // - Frame surface array keeps pointers all surface planes and general frame info
    mfxU32 surfaceSize = GetSurfaceSize(fourCC, inputWidth, inputHeight);
    if (surfaceSize == 0) {
        fclose(fSource);
        fclose(fSink);
        puts("Surface size is wrong");
        return 1;
    }

    std::vector<mfxU8> surfaceBuffersData(surfaceSize * nEncSurfNum);
    mfxU8* surfaceBuffers = surfaceBuffersData.data();

    mfxU16 surfW = (fourCC == MFX_FOURCC_I010) ? inputWidth * 2 : inputWidth;
    mfxU16 surfH = inputHeight;

    // Allocate surface headers (mfxFrameSurface1) for encoder
    std::vector<mfxFrameSurface1> pEncSurfaces(nEncSurfNum);
    for (mfxI32 i = 0; i < nEncSurfNum; i++) {
        memset(&pEncSurfaces[i], 0, sizeof(mfxFrameSurface1));
        pEncSurfaces[i].Info   = mfxEncParams.mfx.FrameInfo;
        pEncSurfaces[i].Data.Y = &surfaceBuffers[surfaceSize * i];

        pEncSurfaces[i].Data.U = pEncSurfaces[i].Data.Y + surfW * surfH;
        pEncSurfaces[i].Data.V =
            pEncSurfaces[i].Data.U + ((surfW / 2) * (surfH / 2));
        pEncSurfaces[i].Data.Pitch = surfW;
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
    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = 2000000;
    std::vector<mfxU8> bstData(mfxBS.MaxLength);
    mfxBS.Data = bstData.data();

    double encode_time = 0;
    double sync_time   = 0;

    // Start encoding the frames
    mfxI32 nEncSurfIdx = 0;
    mfxSyncPoint syncp;
    mfxU32 framenum = 0;

    puts("start encoding");

    // Stage 1: Main encoding loop
    while (MFX_ERR_NONE <= sts || MFX_ERR_MORE_DATA == sts) {
        nEncSurfIdx =
            GetFreeSurfaceIndex(pEncSurfaces); // Find free frame surface
        if (nEncSurfIdx == MFX_ERR_NOT_FOUND) {
            fclose(fSource);
            fclose(fSink);
            puts("no available surface");
            return 1;
        }

        sts = LoadRawFrame(&pEncSurfaces[nEncSurfIdx], fSource);
        if (sts != MFX_ERR_NONE)
            break;

        for (;;) {
            // Encode a frame asychronously (returns immediately)
            auto t0 = std::chrono::high_resolution_clock::now();
            sts     = MFXVideoENCODE_EncodeFrameAsync(session,
                                                  NULL,
                                                  &pEncSurfaces[nEncSurfIdx],
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
                               codecID,
                               fSink);
            mfxBS.DataLength = 0;
        }
    }

    sts = MFX_ERR_NONE;

    // Stage 2: Retrieve the buffered encoded frames
    while (MFX_ERR_NONE <= sts) {
        for (;;) {
            // Encode a frame asychronously (returns immediately)
            auto t0 = std::chrono::high_resolution_clock::now();
            sts     = MFXVideoENCODE_EncodeFrameAsync(session,
                                                  NULL,
                                                  NULL,
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
            else {
                break;
            }
        }

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
                               codecID,
                               fSink);

            mfxBS.DataLength = 0;
        }
    }

    if (g_conf)
        delete g_conf;

    // Clean up resources
    //  - It is recommended to close Media SDK components first, before releasing allocated surfaces, since
    //    some surfaces may still be locked by internal Media SDK resources.
    MFXVideoENCODE_Close(session);

    fclose(fSource);
    fclose(fSink);

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
        WriteIVF_FrameHeader(length, nframe, f);
        fwrite(data, 1, length, f);
    }
    else {
        fwrite(data, 1, length, f);
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
        case MFX_FOURCC_IYUV:
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
        case MFX_FOURCC_IYUV:
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

char* ValidateFileName(char* in) {
    if (in) {
        if (strlen(in) > MAX_PATH)
            return NULL;
    }

    return in;
}

void Usage(void) {
    printf(
        "Usage: hello_encode [encoder] [input filename] [out filename] [width] [height] [10]\n\n");
    printf("\t[encoder]         : h265|av1\n");
    printf("\t[input filename]  : raw video file (i420 only)\n");
    printf("\t[out filename]    : filename to store the output\n");
    printf("\t[width]           : width of input video\n");
    printf("\t[height]          : height of input video\n");
    printf("\t[10]              : 10 bit input (option)\n\n");
    printf("In case of AV1, output will be contained with IVF headers.\n");
    printf("To view:\n");
    printf(" ffplay [out filename]\n");
    return;
}
