//==============================================================================
// Copyright (C) 2020 Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// A minimal oneAPI Video Processing Library (oneVPL) encode application.
///
/// @file

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "vpl/mfxvideo.h"

#define MAX_PATH    260
#define OUTPUT_FILE "out.h265"
#define MAX_WIDTH   3840
#define MAX_HEIGHT  2160

mfxStatus LoadRawFrame(mfxFrameSurface1 *pSurface, FILE *fSource);
void WriteEncodedStream(mfxU8 *data, mfxU32 length, FILE *f);
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height);
mfxI32 GetFreeSurfaceIndex(const std::vector<mfxFrameSurface1> &pSurfacesPool);
char *ValidateFileName(char *in);
void Usage(void);

int main(int argc, char *argv[]) {
    mfxU32 codecID = MFX_CODEC_HEVC;
    mfxU32 fourCC  = MFX_FOURCC_IYUV;

    if (argc != 4) {
        Usage();
        return 1;
    }

    char *in_filename = NULL;

    in_filename = ValidateFileName(argv[1]);
    if (!in_filename) {
        printf("Input filename is not valid\n");
        Usage();
        return 1;
    }

    FILE *fSource = fopen(in_filename, "rb");
    if (!fSource) {
        printf("could not open input file, \"%s\"\n", in_filename);
        return 1;
    }
    FILE *fSink = fopen(OUTPUT_FILE, "wb");
    if (!fSink) {
        fclose(fSource);
        printf("could not open output file, %s\n", OUTPUT_FILE);
        return 1;
    }
    mfxI32 isize = strtol(argv[2], NULL, 10);
    if (isize <= 0 || isize > MAX_WIDTH) {
        fclose(fSource);
        fclose(fSink);
        puts("input size is not valid\n");
        return 1;
    }
    mfxI32 inputWidth = isize;

    isize = strtol(argv[3], NULL, 10);
    if (isize <= 0 || isize > MAX_HEIGHT) {
        fclose(fSource);
        fclose(fSink);
        puts("input size is not valid\n");
        return 1;
    }
    mfxI32 inputHeight = isize;

    // initialize  session
    mfxInitParam initPar   = { 0 };
    initPar.Version.Major  = 1;
    initPar.Version.Minor  = 35;
    initPar.Implementation = MFX_IMPL_SOFTWARE;

    mfxSession session;
    mfxStatus sts = MFXInitEx(initPar, &session);
    if (sts != MFX_ERR_NONE) {
        fclose(fSource);
        fclose(fSink);
        puts("MFXInitEx error.  Could not initialize session");
        return 1;
    }

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
    mfxEncParams.mfx.FrameInfo.Width =
        (((inputWidth + 15) >> 4) << 4); // 16 bytes alignment
    // Height must be a multiple of 16 in case of frame picture and a multiple
    // of 32 in case of field picture
    mfxEncParams.mfx.FrameInfo.Height = (((inputHeight + 15) >> 4) << 4);

    mfxEncParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

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

    // Allocate surfaces for encoder - Frame surface array keeps pointers all
    // surface planes and general frame info
    mfxU32 surfaceSize = GetSurfaceSize(fourCC, inputWidth, inputHeight);
    if (surfaceSize == 0) {
        fclose(fSource);
        fclose(fSink);
        puts("Surface size is wrong");
        return 1;
    }

    std::vector<mfxU8> surfaceBuffersData(surfaceSize * nEncSurfNum);
    mfxU8 *surfaceBuffers = surfaceBuffersData.data();

    // Allocate surface headers (mfxFrameSurface1) for encoder
    std::vector<mfxFrameSurface1> pEncSurfaces(nEncSurfNum);
    for (mfxI32 i = 0; i < nEncSurfNum; i++) {
        memset(&pEncSurfaces[i], 0, sizeof(mfxFrameSurface1));
        pEncSurfaces[i].Info   = mfxEncParams.mfx.FrameInfo;
        pEncSurfaces[i].Data.Y = &surfaceBuffers[surfaceSize * i];

        pEncSurfaces[i].Data.U =
            pEncSurfaces[i].Data.Y + inputWidth * inputHeight;
        pEncSurfaces[i].Data.V =
            pEncSurfaces[i].Data.U + ((inputWidth / 2) * (inputHeight / 2));
        pEncSurfaces[i].Data.Pitch = inputWidth;
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

    // Start encoding the frames
    mfxI32 nEncSurfIdx = 0;
    mfxSyncPoint syncp;
    mfxU32 framenum = 0;

    printf("Encoding %s -> %s\n", in_filename, OUTPUT_FILE);

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
            // Encode a frame asynchronously (returns immediately)
            sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                                  NULL,
                                                  &pEncSurfaces[nEncSurfIdx],
                                                  &mfxBS,
                                                  &syncp);

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
            sts = MFXVideoCORE_SyncOperation(
                session,
                syncp,
                60000); // Synchronize. Wait until encoded frame is ready
            ++framenum;
            WriteEncodedStream(mfxBS.Data + mfxBS.DataOffset,
                               mfxBS.DataLength,

                               fSink);
            mfxBS.DataLength = 0;
        }
    }

    sts = MFX_ERR_NONE;

    // Stage 2: Retrieve the buffered encoded frames
    while (MFX_ERR_NONE <= sts) {
        for (;;) {
            // Encode a frame asychronously (returns immediately)
            sts = MFXVideoENCODE_EncodeFrameAsync(session,
                                                  NULL,
                                                  NULL,
                                                  &mfxBS,
                                                  &syncp);
            if (MFX_ERR_NONE < sts && syncp) {
                sts = MFX_ERR_NONE; // Ignore warnings if output is available
                break;
            }
            else {
                break;
            }
        }

        if (MFX_ERR_NONE == sts) {
            sts = MFXVideoCORE_SyncOperation(
                session,
                syncp,
                60000); // Synchronize. Wait until encoded frame is ready

            ++framenum;
            WriteEncodedStream(mfxBS.Data + mfxBS.DataOffset,
                               mfxBS.DataLength,

                               fSink);

            mfxBS.DataLength = 0;
        }
    }

    printf("Encoded %d frames\n", framenum);

    // Clean up resources - It is recommended to close Media SDK components
    // first, before releasing allocated surfaces, since some surfaces may still
    // be locked by internal Media SDK resources.
    MFXVideoENCODE_Close(session);

    fclose(fSource);
    fclose(fSink);

    return 0;
}

mfxStatus LoadRawFrame(mfxFrameSurface1 *pSurface, FILE *fSource) {
    mfxU16 w, h, i, pitch;
    mfxU32 nBytesRead;
    mfxU8 *ptr;
    mfxFrameInfo *pInfo = &pSurface->Info;
    mfxFrameData *pData = &pSurface->Data;

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
        default:
            break;
    }

    return MFX_ERR_NONE;
}

// Write encoded stream to file
void WriteEncodedStream(mfxU8 *data, mfxU32 length, FILE *f) {
    fwrite(data, 1, length, f);
}

// Return the surface size in bytes given format and dimensions
mfxU32 GetSurfaceSize(mfxU32 FourCC, mfxU32 width, mfxU32 height) {
    mfxU32 nbytes = 0;

    switch (FourCC) {
        case MFX_FOURCC_IYUV:
            nbytes = width * height + (width >> 1) * (height >> 1) +
                     (width >> 1) * (height >> 1);
            break;
        default:
            break;
    }

    return nbytes;
}

// Return index of free surface in given pool
mfxI32 GetFreeSurfaceIndex(const std::vector<mfxFrameSurface1> &pSurfacesPool) {
    auto it = std::find_if(pSurfacesPool.begin(),
                           pSurfacesPool.end(),
                           [](const mfxFrameSurface1 &surface) {
                               return 0 == surface.Data.Locked;
                           });

    if (it == pSurfacesPool.end())
        return MFX_ERR_NOT_FOUND;
    else
        return static_cast<mfxI32>(it - pSurfacesPool.begin());
}

char *ValidateFileName(char *in) {
    if (in) {
        if (strlen(in) > MAX_PATH)
            return NULL;
    }

    return in;
}

// Print usage message
void Usage(void) {
    printf("Usage: hello-encode SOURCE WIDTH HEIGHT\n\n"
           "Encode raw I420 video in SOURCE having dimensions WIDTH x HEIGHT "
           "to H265 in %s\n\n"
           "To view:\n"
           " ffplay %s\n",
           OUTPUT_FILE,
           OUTPUT_FILE);
}