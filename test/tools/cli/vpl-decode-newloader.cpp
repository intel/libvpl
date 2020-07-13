/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/
// sample demonstrating use of oneAPI Video Processing Library loader

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo.h"

#define MAX_PATH 260

int GetFreeSurfaceIndex(mfxFrameSurface1 *SurfacesPool, mfxU16 nPoolSize) {
    for (mfxU16 i = 0; i < nPoolSize; i++) {
        if (0 == SurfacesPool[i].Data.Locked)
            return i;
    }
    return MFX_ERR_NOT_FOUND;
}

// check if this implementation can decode our stream
bool CheckImplCaps(mfxImplDescription *implDesc, mfxU32 codecID) {
    mfxU32 i;

    for (i = 0; i < implDesc->Dec.NumCodecs; i++) {
        mfxDecoderDescription::decoder *currDec = &(implDesc->Dec.Codecs[i]);

        if (currDec->CodecID == codecID) {
            return true;
        }
    }

    return false;
}

char *ValidateFileName(char *in) {
    if (in) {
        if (strlen(in) > MAX_PATH)
            return NULL;
    }

    return in;
}

#define TEST_CFG(type, dType, val)                                           \
    cfg                  = MFXCreateConfig(loader);                          \
    ImplValue.Type       = (type);                                           \
    ImplValue.Data.dType = (val);                                            \
    sts                  = MFXSetConfigFilterProperty(cfg, name, ImplValue); \
    printf("Test config: sts = %d, name = %s\n", sts, name);

static void TestCfgPropsMain(mfxLoader loader) {
    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;
    const mfxU8 *name;

    name = (const mfxU8 *)"mfxImplDescription.Impl";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_IMPL_SOFTWARE);

    name = (const mfxU8 *)"mfxImplDescription.accelerationMode";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 3);

    name = (const mfxU8 *)"mfxImplDescription.VendorID";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, 0xabcd);

    name = (const mfxU8 *)"mfxImplDescription.VendorImplID";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, 0x1234);
}

static void TestCfgPropsDec(mfxLoader loader) {
    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;
    const mfxU8 *name;

    name = (const mfxU8
                *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_CODEC_HEVC);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.MaxcodecLevel";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 54);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, 150);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.MemHandleType";
    TEST_CFG(MFX_VARIANT_TYPE_I32, I32, MFX_RESOURCE_SYSTEM_SURFACE);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.ColorFormats";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_FOURCC_IYUV);
}

static void TestCfgPropsEnc(mfxLoader loader) {
    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;
    const mfxU8 *name;

    name = (const mfxU8
                *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_CODEC_HEVC);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.MaxcodecLevel";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 54);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.BiDirectionalPrediction";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 1);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.encprofile.Profile";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, 150);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.MemHandleType";
    TEST_CFG(MFX_VARIANT_TYPE_I32, I32, MFX_RESOURCE_SYSTEM_SURFACE);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.ColorFormats";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_FOURCC_IYUV);
}

static void TestCfgPropsVPP(mfxLoader loader) {
    mfxStatus sts;
    mfxConfig cfg;
    mfxVariant ImplValue;
    const mfxU8 *name;

    name = (const mfxU8
                *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_CODEC_HEVC);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxVPPDescription.filter.MaxDelayInFrames";
    TEST_CFG(MFX_VARIANT_TYPE_U16, U16, 3);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxVPPDescription.filter.memdesc.MemHandleType";
    TEST_CFG(MFX_VARIANT_TYPE_I32, I32, MFX_RESOURCE_SYSTEM_SURFACE);

    name =
        (const mfxU8
             *)"mfxImplDescription.mfxVPPDescription.filter.memdesc.format.OutFormats";
    TEST_CFG(MFX_VARIANT_TYPE_U32, U32, MFX_FOURCC_IYUV);
}

int main(int argc, char *argv[]) {
    mfxStatus sts      = MFX_ERR_NONE;
    mfxSession session = nullptr;

    if (argc != 2) {
        printf("usage: hello-decode-newloader in.h265\n\n");
        printf("decodes an h265/hevc file to out.raw\n");
        printf("to view:\n");
        printf(" ffplay -s 128x96 -pix_fmt yuv420p -f rawvideo out.raw\n");
        exit(1);
    }

    char *in_filename = NULL;
    in_filename       = ValidateFileName(argv[1]);
    if (!in_filename) {
        printf("Input filename is not valid\n");
        exit(1);
    }

    printf("opening %s\n", in_filename);
    FILE *fSource = fopen(in_filename, "rb");
    if (!fSource) {
        printf("could not open input file, %s\n", in_filename);
        exit(1);
    }
    FILE *fSink = fopen("out.raw", "wb");
    if (!fSink) {
        fclose(fSource);
        puts("could not open output file");
        exit(1);
    }

    mfxU32 inCodecFourCC = MFX_CODEC_HEVC;

    // init Media SDK
    mfxBitstream mfxBS = { 0 };
    mfxBS.MaxLength    = 2000000;
    mfxBS.Data         = new mfxU8[mfxBS.MaxLength];
    memset(mfxBS.Data, 0, mfxBS.MaxLength);

    mfxLoader loader = MFXLoad();

    mfxVariant ImplValue;
    mfxConfig cfg;

    // DBG
    //TestCfgPropsMain(loader);
    //TestCfgPropsDec(loader);
    //TestCfgPropsEnc(loader);
    //TestCfgPropsVPP(loader);

    cfg                = MFXCreateConfig(loader);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_IMPL_SOFTWARE;
    MFXSetConfigFilterProperty(cfg,
                               (const mfxU8 *)"mfxImplDescription.Impl",
                               ImplValue);

    cfg                = MFXCreateConfig(loader);
    ImplValue.Type     = MFX_VARIANT_TYPE_U32;
    ImplValue.Data.U32 = MFX_CODEC_HEVC;
    MFXSetConfigFilterProperty(
        cfg,
        (const mfxU8
             *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
        ImplValue);

    mfxU32 implIdx = 0;
    while (1) {
        mfxImplDescription *implDesc;
        sts = MFXEnumImplementations(loader,
                                     implIdx,
                                     MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                     reinterpret_cast<mfxHDL *>(&implDesc));

        // out of range - we've tested all implementations
        if (sts == MFX_ERR_NOT_FOUND)
            break;

        if (CheckImplCaps(implDesc, inCodecFourCC) == true) {
            // this implementation is capable of decoding the input stream
            sts = MFXCreateSession(loader, implIdx, &session);
            if (sts != MFX_ERR_NONE) {
                delete[] mfxBS.Data;
                fclose(fSink);
                fclose(fSource);
                printf("Error in MFXCreateSession, sts = %d", sts);
                return -1;
            }
            MFXDispReleaseImplDescription(loader, implDesc);
            break;
        }
        else {
            MFXDispReleaseImplDescription(loader, implDesc);
        }

        implIdx++;
    }

    if (session == nullptr || sts == MFX_ERR_NOT_FOUND) {
        delete[] mfxBS.Data;
        fclose(fSource);
        fclose(fSink);
        printf("Error - unable to create session");
        return -1;
    }

    // DBG - test dispatcher calls
    mfxFrameSurface1 *testSurface;
    MFXMemory_GetSurfaceForDecode(session, &testSurface);
    MFXMemory_GetSurfaceForEncode(session, &testSurface);
    MFXMemory_GetSurfaceForVPP(session, &testSurface);

    // set up input bitstream
    memmove(mfxBS.Data, mfxBS.Data + mfxBS.DataOffset, mfxBS.DataLength);
    mfxBS.DataLength =
        static_cast<mfxU32>(fread(mfxBS.Data, 1, mfxBS.MaxLength, fSource));

    // initialize decode parameters from stream header
    mfxVideoParam mfxDecParams;
    memset(&mfxDecParams, 0, sizeof(mfxDecParams));
    mfxDecParams.mfx.CodecId = inCodecFourCC;
    mfxDecParams.IOPattern   = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
    sts = MFXVideoDECODE_DecodeHeader(session, &mfxBS, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        printf("Problem decoding header.  DecodeHeader sts=%d\n", sts);
        exit(1);
    }

    // Query number required surfaces for decoder
    mfxFrameAllocRequest DecRequest = { 0 };
    MFXVideoDECODE_QueryIOSurf(session, &mfxDecParams, &DecRequest);

    // Determine the required number of surfaces for decoder output
    mfxU16 nSurfNumDec = DecRequest.NumFrameSuggested;

    mfxFrameSurface1 *decSurfaces = new mfxFrameSurface1[nSurfNumDec];

    // initialize surface pool for decode (I420 format)
    size_t framePoolBufSize = static_cast<size_t>(
        mfxDecParams.mfx.FrameInfo.Width * mfxDecParams.mfx.FrameInfo.Height *
        1.5 * nSurfNumDec);
    mfxU8 *DECoutbuf = new mfxU8[framePoolBufSize];

    for (int i = 0; i < nSurfNumDec; i++) {
        decSurfaces[i]      = { 0 };
        decSurfaces[i].Info = mfxDecParams.mfx.FrameInfo;
        size_t buf_offset =
            static_cast<size_t>(i * mfxDecParams.mfx.FrameInfo.Height *
                                mfxDecParams.mfx.FrameInfo.Width * 1.5);
        decSurfaces[i].Data.Y = DECoutbuf + buf_offset;
        decSurfaces[i].Data.U = DECoutbuf + buf_offset +
                                mfxDecParams.mfx.FrameInfo.Height *
                                    mfxDecParams.mfx.FrameInfo.Width;
        decSurfaces[i].Data.V =
            decSurfaces[i].Data.U + mfxDecParams.mfx.FrameInfo.Height / 2 *
                                        mfxDecParams.mfx.FrameInfo.Width / 2;
        decSurfaces[i].Data.Pitch = mfxDecParams.mfx.FrameInfo.Width;
    }

    // input parameters finished, now initialize decode
    sts = MFXVideoDECODE_Init(session, &mfxDecParams);
    if (sts != MFX_ERR_NONE) {
        puts("Could not initialize decode");
        exit(1);
    }
    // ------------------
    // main loop
    // ------------------
    int framenum  = 0;
    bool draining = false;
    bool finished = false;

    while (!finished) {
        mfxSyncPoint syncp               = { 0 };
        mfxFrameSurface1 *pmfxOutSurface = nullptr;

        bool stillgoing = true;
        int nIndex      = GetFreeSurfaceIndex(decSurfaces, nSurfNumDec);
        while (stillgoing) {
            // send null bitstream ptr to drain buffered frames at EOF
            mfxBitstream *pBS = (draining == true ? nullptr : &mfxBS);

            // submit async decode request
            sts = MFXVideoDECODE_DecodeFrameAsync(session,
                                                  pBS,
                                                  &decSurfaces[nIndex],
                                                  &pmfxOutSurface,
                                                  &syncp);

            // all finished
            if (draining == true && sts == MFX_ERR_MORE_DATA) {
                finished = true;
                break;
            }

            // next step actions provided by application
            switch (sts) {
                case MFX_ERR_MORE_DATA: // more data is needed to decode
                {
                    memmove(mfxBS.Data,
                            mfxBS.Data + mfxBS.DataOffset,
                            mfxBS.DataLength);
                    mfxBS.DataOffset = 0;
                    mfxBS.DataLength = static_cast<mfxU32>(
                        fread(mfxBS.Data + mfxBS.DataLength,
                              1,
                              mfxBS.MaxLength - mfxBS.DataLength,
                              fSource));
                    if (mfxBS.DataLength == 0) {
                        draining = true; // EOF - drain buffered frames
                    }
                } break;
                case MFX_ERR_MORE_SURFACE: // feed a fresh surface to decode
                    nIndex = GetFreeSurfaceIndex(decSurfaces, nSurfNumDec);
                    break;
                case MFX_ERR_NONE: // no more steps needed, exit loop
                    stillgoing = false;
                    break;
                default: // state is not one of the cases above
                    printf("Error in DecodeFrameAsync: sts=%d\n", sts);
                    exit(1);
                    break;
            }
        }

        if (sts < 0)
            break;

        // data available to app only after sync
        MFXVideoCORE_SyncOperation(session, syncp, 60000);

        // write output if output file specified
        if (fSink) {
            // write the output to disk
            //Y
            for (int r = 0; r < pmfxOutSurface->Info.CropH; r++) {
                fwrite(pmfxOutSurface->Data.Y + r * pmfxOutSurface->Data.Pitch,
                       1,
                       pmfxOutSurface->Info.CropW,
                       fSink);
            }
            //U
            for (int r = 0; r < pmfxOutSurface->Info.CropH / 2; r++) {
                fwrite(
                    pmfxOutSurface->Data.U + r * pmfxOutSurface->Data.Pitch / 2,
                    1,
                    pmfxOutSurface->Info.CropW / 2,
                    fSink);
            }
            //V
            for (int r = 0; r < pmfxOutSurface->Info.CropH / 2; r++) {
                fwrite(
                    pmfxOutSurface->Data.V + r * pmfxOutSurface->Data.Pitch / 2,
                    1,
                    pmfxOutSurface->Info.CropW / 2,
                    fSink);
            }
        }
        framenum++;
    }
    printf("read %d frames\n", framenum);

    fclose(fSink);
    fclose(fSource);
    MFXVideoDECODE_Close(session);
    delete[] mfxBS.Data;
    delete[] DECoutbuf;
    delete[] decSurfaces;

    MFXClose(session);
    MFXUnload(loader);

    return 0;
}
