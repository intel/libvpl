//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./util.h" //NOLINT

void Usage(void) {
    printf("\n");
    printf("Usage: vpl-import-export [testmode] [options]\n\n");
    printf("   Test mode (specify exactly one)\n");
    printf("     -r             decode+VPP infile and render to screen\n");
    printf("     -c             capture desktop, encode, and save to outfile\n");
    printf("     -d             decode+VPP infile and save to outfile\n");
    printf("     -e             encode raw video from infile and save to outfile\n");
    printf("\n");
    printf("   Other options\n");
    printf("     -? -help       print help screen\n");
    printf("     -i [infile]    input file name (HEVC elementary stream)\n");
    printf("     -o [outfile]   output file name (disables render to screen)\n");
    printf("     -sw [width]    source width, required for encode\n");
    printf("     -sh [height]   source height, required for encode\n");
    printf("     -dw [width]    VPP output width (default = %d)\n", DEF_DST_WIDTH);
    printf("     -dh [height]   VPP output height (default = %d)\n", DEF_DST_HEIGHT);
    printf("     -nv12          NV12 output (default)\n");
    printf("     -rgb           RGB output (MFX_FOURCC_BGR4)\n");
    printf("     -opencl file   enable OpenCL, read program from file (file is required)\n");
    printf("     -timing        print timing info (may modify other options)\n");
    printf("     -mode [mode]   select surface import/export mode, options = [copy, shared]\n");
    printf("     -dbg [mask]    enable debugging options according to bitmask in mask\n");
    printf("\n");
    printf("   Debug bitmask options\n");
    printf("      0x0001        DBG_MASK_NATIVE_SURFACE_DESC\n");
    printf("      0x0002        DBG_MASK_ACTUAL_SURFACE_FLAGS\n");
    return;
}

static bool ValidateFileName(char *in, std::string &inStr) {
    if (in && strnlen(in, MAX_PATH) < MAX_PATH) {
        inStr = in;
        return true;
    }

    return false;
}

static bool ValidateSize(char *in, mfxU16 *vsize, mfxU32 vmax) {
    if (in) {
        *vsize = static_cast<mfxU16>(strtol(in, NULL, 10));
        if (*vsize <= vmax)
            return true;
    }

    *vsize = 0;
    return false;
}

bool ParseArgsAndValidate(int argc, char *argv[], Params *params) {
    int idx;
    char *s;

    // init all params to 0
    *params = {};

    // defaults
    params->outFourCC = MFX_FOURCC_NV12;
    params->dstWidth  = DEF_DST_WIDTH;
    params->dstHeight = DEF_DST_HEIGHT;

    params->bEnableOpenCL = false;
    params->bEnableTiming = false;
    params->surfaceMode   = SURFACE_MODE_UNKNOWN;
    params->dbgMask       = 0;

    for (idx = 1; idx < argc;) {
        // all switches must start with '-'
        if (argv[idx][0] != '-') {
            printf("ERROR: invalid argument: %s\n", argv[idx]);
            return false;
        }

        // switch string, starting after the '-'
        s = &argv[idx][1];
        idx++;

        // search for match
        if (IS_ARG_EQ(s, "?") || IS_ARG_EQ(s, "help")) {
            return false; // just print help screen
        }
        else if (IS_ARG_EQ(s, "i")) {
            if (!ValidateFileName(argv[idx++], params->infileName))
                return false;
        }
        else if (IS_ARG_EQ(s, "o")) {
            if (!ValidateFileName(argv[idx++], params->outfileName))
                return false;
        }
        else if (IS_ARG_EQ(s, "opencl")) {
            if (!ValidateFileName(argv[idx++], params->openCLfileName))
                return false;
            params->bEnableOpenCL = true;
        }
        else if (IS_ARG_EQ(s, "mode")) {
            std::string surfaceMode(argv[idx++]);
            if (surfaceMode == "shared")
                params->surfaceMode = SURFACE_MODE_SHARED;
            else if (surfaceMode == "copy")
                params->surfaceMode = SURFACE_MODE_COPY;
            else
                return false;
        }
        else if (IS_ARG_EQ(s, "r")) {
            params->testMode = TEST_MODE_RENDER;
        }
        else if (IS_ARG_EQ(s, "c")) {
            params->testMode = TEST_MODE_CAPTURE;
        }
        else if (IS_ARG_EQ(s, "d")) {
            params->testMode = TEST_MODE_DECVPP_FILE;
        }
        else if (IS_ARG_EQ(s, "e")) {
            params->testMode = TEST_MODE_ENC_FILE;
        }
        else if (IS_ARG_EQ(s, "sw")) {
            if (!ValidateSize(argv[idx++], &params->srcWidth, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "sh")) {
            if (!ValidateSize(argv[idx++], &params->srcHeight, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "dw")) {
            if (!ValidateSize(argv[idx++], &params->dstWidth, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "dh")) {
            if (!ValidateSize(argv[idx++], &params->dstHeight, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "nv12")) {
            params->outFourCC = MFX_FOURCC_NV12;
        }
        else if (IS_ARG_EQ(s, "rgb")) {
            params->outFourCC = MFX_FOURCC_BGR4;
        }
        else if (IS_ARG_EQ(s, "timing")) {
            params->bEnableTiming = true;
        }
        else if (IS_ARG_EQ(s, "dbg")) {
            std::string dbgMask(argv[idx++]);
            try {
                params->dbgMask = std::stoi(dbgMask, 0, 16);
            }
            catch (...) {
                return false;
            }
        }
        else {
            printf("\nERROR: '-%s' is not supported\n", s);
            return false;
        }
    }

    // validate required parameters
    if (params->testMode == TEST_MODE_UNKNOWN) {
        printf("ERROR: test mode is required (choose one of: -r, -c, -d, -e)\n");
        return false;
    }

    if (params->infileName.empty() && params->testMode != TEST_MODE_CAPTURE) {
        printf("ERROR: input file name (-i) is required\n");
        return false;
    }

    if ((!params->srcWidth || !params->srcHeight) && params->testMode == TEST_MODE_ENC_FILE) {
        printf("ERROR: srcHeight and srcWidth are required for encoding\n");
        params->outfileName.clear();
    }

    if (!params->outfileName.empty() && params->testMode == TEST_MODE_RENDER) {
        printf("Warning - output file (-o) ignored when rendering\n");
        params->outfileName.clear();
    }

    return true;
}

const char *FourCCToString(mfxU32 fourCC) {
    switch (fourCC) {
        case MFX_FOURCC_NV12:
            return (const char *)"NV12";
        case MFX_FOURCC_BGR4:
            return (const char *)"BGR4";
        default:
            return (const char *)"unknown";
    }
}

mfxStatus VPLSession::Open(std::list<SurfaceCaps> *surfaceCapsList) {
    if (!surfaceCapsList)
        return MFX_ERR_NULL_PTR;

    mfxStatus sts = MFX_ERR_NONE;

    // variables used only in 2.x version
    mfxConfig cfg[4];
    mfxVariant cfgVal;

    //-- Create session
    m_loader = MFXLoad();
    VERIFY2(NULL != m_loader, "ERROR: MFXLoad failed -- is implementation in path?\n");

    // Implementation used must be the hardware implementation
    cfg[0] = MFXCreateConfig(m_loader);
    VERIFY2(NULL != cfg[0], "MFXCreateConfig failed");
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = MFX_IMPL_TYPE_HARDWARE;

    sts = MFXSetConfigFilterProperty(cfg[0], (mfxU8 *)"mfxImplDescription.Impl", cfgVal);
    VERIFY2(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for Impl");

    // Implementation must provide an HEVC decoder
    cfg[1] = MFXCreateConfig(m_loader);
    VERIFY2(NULL != cfg[1], "MFXCreateConfig failed");
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = MFX_CODEC_HEVC;
    sts             = MFXSetConfigFilterProperty(cfg[1], (mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID", cfgVal);
    VERIFY2(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for decoder CodecID");

    // Implementation used must have VPP scaling capability
    cfg[2] = MFXCreateConfig(m_loader);
    VERIFY2(NULL != cfg[2], "MFXCreateConfig failed");
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = MFX_EXTBUFF_VPP_SCALING;
    sts             = MFXSetConfigFilterProperty(cfg[2], (mfxU8 *)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC", cfgVal);
    VERIFY2(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for VPP scale");

    // Implementation used must provide API version 2.9 or newer
    cfg[3] = MFXCreateConfig(m_loader);
    VERIFY2(NULL != cfg[3], "MFXCreateConfig failed");
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = VPLVERSION(MAJOR_API_VERSION_REQUIRED, MINOR_API_VERSION_REQUIRED);
    sts             = MFXSetConfigFilterProperty(cfg[3], (mfxU8 *)"mfxImplDescription.ApiVersion.Version", cfgVal);
    VERIFY2(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for API version");

    // Add filters for required surface sharing capabilities
    for (auto surfaceCaps : *surfaceCapsList) {
        mfxConfig cfgSS = MFXCreateConfig(m_loader);
        VERIFY2(NULL != cfgSS, "MFXCreateConfig failed");

        cfgVal.Type     = MFX_VARIANT_TYPE_U32;
        cfgVal.Data.U32 = surfaceCaps.SurfaceType;
        sts             = MFXSetConfigFilterProperty(cfgSS, (mfxU8 *)"mfxSurfaceTypesSupported.surftype.SurfaceType", cfgVal);
        VERIFY2(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for SurfaceType");

        cfgVal.Type     = MFX_VARIANT_TYPE_U32;
        cfgVal.Data.U32 = surfaceCaps.SurfaceComponent;
        sts             = MFXSetConfigFilterProperty(cfgSS, (mfxU8 *)"mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceComponent", cfgVal);
        VERIFY2(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for SurfaceType");

        cfgVal.Type     = MFX_VARIANT_TYPE_U32;
        cfgVal.Data.U32 = surfaceCaps.SurfaceFlags;
        sts             = MFXSetConfigFilterProperty(cfgSS, (mfxU8 *)"mfxSurfaceTypesSupported.surftype.surfcomp.SurfaceFlags", cfgVal);
        VERIFY2(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for SurfaceType");
    }

    mfxHDL idesc_path;
    sts = MFXEnumImplementations(m_loader, 0, MFX_IMPLCAPS_IMPLPATH, &idesc_path);
    VERIFY2(MFX_ERR_NONE == sts, "ERROR: MFXEnumImplementations failed for implpath");

    sts = MFXCreateSession(m_loader, 0, &m_session);
    VERIFY2(MFX_ERR_NONE == sts, "ERROR: cannot create session -- no implementations meet selection criteria");

    printf("Created session with library: %s\n", (char *)idesc_path);

    return MFX_ERR_NONE;
}

// Read encoded stream from file
mfxStatus ReadEncodedStream(mfxBitstream &bs, std::ifstream &f) {
    mfxU8 *p0 = bs.Data;
    mfxU8 *p1 = bs.Data + bs.DataOffset;
    if (bs.DataOffset > bs.MaxLength - 1) {
        return MFX_ERR_NOT_ENOUGH_BUFFER;
    }
    if (bs.DataLength + bs.DataOffset > bs.MaxLength) {
        return MFX_ERR_NOT_ENOUGH_BUFFER;
    }
    for (mfxU32 i = 0; i < bs.DataLength; i++) {
        *(p0++) = *(p1++);
    }
    bs.DataOffset = 0;

    f.read((char *)bs.Data + bs.DataLength, bs.MaxLength - bs.DataLength);
    bs.DataLength += (mfxU32)f.gcount();

    if (bs.DataLength == 0)
        return MFX_ERR_MORE_DATA;

    return MFX_ERR_NONE;
}

// Write encoded stream to file
void WriteEncodedStream(mfxBitstream &bs, std::ofstream &f) {
    f.write((char *)(bs.Data + bs.DataOffset), bs.DataLength);
    bs.DataLength = 0;
    return;
}

// Write raw frame to file
mfxStatus WriteRawFrame(mfxFrameSurface1 *surface, std::ofstream &f) {
    mfxU16 w, h, i, pitch;
    mfxFrameInfo *info = &surface->Info;
    mfxFrameData *data = &surface->Data;

    w = info->CropW;
    h = info->CropH;

    // write the output to disk
    switch (info->FourCC) {
        case MFX_FOURCC_I420:
            // Y
            pitch = data->Pitch;
            for (i = 0; i < h; i++) {
                f.write((char *)(data->Y + i * pitch), w);
            }
            // U
            pitch /= 2;
            h /= 2;
            w /= 2;
            for (i = 0; i < h; i++) {
                f.write((char *)(data->U + i * pitch), w);
            }
            // V
            for (i = 0; i < h; i++) {
                f.write((char *)(data->V + i * pitch), w);
            }
            break;
        case MFX_FOURCC_NV12:
            // Y
            pitch = data->Pitch;
            for (i = 0; i < h; i++) {
                f.write((char *)(data->Y + i * pitch), w);
            }
            // UV
            h /= 2;
            for (i = 0; i < h; i++) {
                f.write((char *)(data->UV + i * pitch), w);
            }
            break;
        case MFX_FOURCC_RGB4:
            // Y
            pitch = data->Pitch;
            for (i = 0; i < h; i++) {
                f.write((char *)(data->B + i * pitch), 4 * w);
            }
            break;
        case MFX_FOURCC_BGR4:
            // Y
            pitch = data->Pitch;
            for (i = 0; i < h; i++) {
                f.write((char *)(data->R + i * pitch), 4 * w);
            }
            break;
        default:
            return MFX_ERR_UNSUPPORTED;
            break;
    }

    return MFX_ERR_NONE;
}

mfxStatus WriteRawFrameCPU_NV12(CPUFrameInfo_NV12 &cpuFrameInfo, std::ofstream &f) {
    mfxU8 *Y     = cpuFrameInfo.Y;
    mfxU8 *UV    = cpuFrameInfo.UV;
    mfxU16 w     = cpuFrameInfo.width;
    mfxU16 h     = cpuFrameInfo.height;
    mfxU16 pitch = cpuFrameInfo.pitch;

    // write the output to disk
    for (int i = 0; i < h; i++) {
        f.write((char *)(Y + i * pitch), w);
    }
    // UV
    h /= 2;
    for (int i = 0; i < h; i++) {
        f.write((char *)(UV + i * pitch), w);
    }

    return MFX_ERR_NONE;
}

mfxStatus ReadRawFrameCPU_NV12(CPUFrameInfo_NV12 &cpuFrameInfo, std::ifstream &f) {
    mfxU16 i;
    size_t bytes_read;

    mfxU8 *Y     = cpuFrameInfo.Y;
    mfxU8 *UV    = cpuFrameInfo.UV;
    mfxU16 w     = cpuFrameInfo.width;
    mfxU16 h     = cpuFrameInfo.height;
    mfxU16 pitch = cpuFrameInfo.pitch;

    // Y
    for (i = 0; i < h; i++) {
        f.read((char *)(Y + i * pitch), w);
        bytes_read = (mfxU32)f.gcount();
        if (w != bytes_read)
            return MFX_ERR_MORE_DATA;
    }

    // UV
    for (i = 0; i < h / 2; i++) {
        f.read((char *)(UV + i * pitch), w);
        bytes_read = (mfxU32)f.gcount();
        if (w != bytes_read)
            return MFX_ERR_MORE_DATA;
    }

    return MFX_ERR_NONE;
}

// Load raw I420 frames to mfxFrameSurface
mfxStatus ReadRawFrame(mfxFrameSurface1 *surface, std::ifstream &f) {
    mfxU16 w, h, i, pitch;
    size_t bytes_read;
    mfxU8 *ptr;
    mfxFrameInfo *info = &surface->Info;
    mfxFrameData *data = &surface->Data;

    w = info->CropW;
    h = info->CropH;

    switch (info->FourCC) {
        case MFX_FOURCC_I420:
            // read luminance plane (Y)
            pitch = data->Pitch;
            ptr   = data->Y;
            for (i = 0; i < h; i++) {
                f.read((char *)(ptr + i * pitch), w);
                bytes_read = (mfxU32)f.gcount();
                if (w != bytes_read)
                    return MFX_ERR_MORE_DATA;
            }

            // read chrominance (U, V)
            pitch /= 2;
            h /= 2;
            w /= 2;
            ptr = data->U;
            for (i = 0; i < h; i++) {
                f.read((char *)(ptr + i * pitch), w);
                bytes_read = (mfxU32)f.gcount();
                if (w != bytes_read)
                    return MFX_ERR_MORE_DATA;
            }

            ptr = data->V;
            for (i = 0; i < h; i++) {
                f.read((char *)(ptr + i * pitch), w);
                bytes_read = (mfxU32)f.gcount();
                if (w != bytes_read)
                    return MFX_ERR_MORE_DATA;
            }
            break;
        case MFX_FOURCC_NV12:
            // Y
            pitch = data->Pitch;
            for (i = 0; i < h; i++) {
                f.read((char *)(data->Y + i * pitch), w);
                bytes_read = (mfxU32)f.gcount();
                if (w != bytes_read)
                    return MFX_ERR_MORE_DATA;
            }
            // UV
            h /= 2;
            for (i = 0; i < h; i++) {
                f.read((char *)(data->UV + i * pitch), w);
                bytes_read = (mfxU32)f.gcount();
                if (w != bytes_read)
                    return MFX_ERR_MORE_DATA;
            }
            break;
        case MFX_FOURCC_RGB4:
            pitch = data->Pitch;
            for (i = 0; i < h; i++) {
                f.read((char *)(data->B + i * pitch), w);
                bytes_read = (mfxU32)f.gcount();
                if (pitch != bytes_read)
                    return MFX_ERR_MORE_DATA;
            }
            break;
        default:
            printf("Unsupported FourCC code, skip LoadRawFrame\n");
            break;
    }

    return MFX_ERR_NONE;
}

bool CheckKB_Quit(void) {
#if defined(_WIN32) || defined(_WIN64)
    while (_kbhit()) {
        int k = _getch();
        if (k == 0x1B || k == 'q' || k == 'Q')
            return true;
    }
    return false;
#else
    return false;
#endif
}
