//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./util.h" //NOLINT
#include <algorithm>
void Usage(void) {
    printf("\n");
    printf("Usage: val-surface-sharing [options]\n\n");
    printf("     -? -help              print help messages\n");
    printf("     -c <h264, h265>       codec name\n");
    printf("     -i input file         input raw video file name\n");
    printf("     -o output file        output file name to store encoded stream\n");
    printf("     -surface_component <encode, decode, vpp_in, vpp_out>\n");
    printf("                           select surface component\n");
    printf("     -mode <shared, copy>  select surface import/export mode\n");
    printf("                           if '-mode' isn't set: process w/o surface sharing api\n");
    printf("     -surface_type <d3d11, vaapi, opencl>\n");
    printf("                           select surface type\n");
    printf("     -sc <nv12, rgb4>      source color format\n");
    printf("     -dc <nv12, rgb4>      dest color format\n");
    printf("     -sw width             source width, required for encode\n");
    printf("     -sh height            source height, required for encode\n");
    printf("     -adapters             show adapters supported surface sharing API\n");
    printf("     -multi_session        run configured test case as multi threads\n");
    printf("Example:\n");
    printf(
        "     encode: val-surface-sharing -c h265 -i cars_320x240.nv12 -o out.h265 -sw 320 -sh 240 -sc nv12 -surface_component encode -surface_type d3d11 -mode shared\n");
    printf(
        "     decode: val-surface-sharing -c h265 -i cars_320x240.h265 -o out.nv12 -surface_component decode -surface_type d3d11 -mode shared\n");

    printf("\n");
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

bool ParseArgsAndValidate(int argc, char *argv[], Options *opts) {
    int idx;
    char *s;

    // init all opts to 0
    *opts = {};

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
            if (!ValidateFileName(argv[idx++], opts->infileName))
                return false;
        }
        else if (IS_ARG_EQ(s, "o")) {
            if (!ValidateFileName(argv[idx++], opts->outfileName))
                return false;
        }
        else if (IS_ARG_EQ(s, "c")) {
            std::string codecID(argv[idx++]);
            if (codecID == "h265" || codecID == "hevc")
                opts->codecID = MFX_CODEC_HEVC;
            else if (codecID == "h264" || codecID == "avc")
                opts->codecID = MFX_CODEC_AVC;
            else
                return false;
        }
        else if (IS_ARG_EQ(s, "mode")) {
            std::string surfaceMode(argv[idx++]);
            if (surfaceMode == "copy")
                opts->surfaceMode = SURFACE_MODE_COPY;
            else if (surfaceMode == "shared")
                opts->surfaceMode = SURFACE_MODE_SHARED;
            else
                return false;
        }
        else if (IS_ARG_EQ(s, "surface_component")) {
            std::string surfaceComponent(argv[idx++]);
            if (surfaceComponent == "encode")
                opts->surfaceComponent = MFX_SURFACE_COMPONENT_ENCODE;
            else if (surfaceComponent == "decode")
                opts->surfaceComponent = MFX_SURFACE_COMPONENT_DECODE;
            else if (surfaceComponent == "vpp_in")
                opts->surfaceComponent = MFX_SURFACE_COMPONENT_VPP_INPUT;
            else if (surfaceComponent == "vpp_out")
                opts->surfaceComponent = MFX_SURFACE_COMPONENT_VPP_OUTPUT;
            else
                return false;
        }
        else if (IS_ARG_EQ(s, "surface_type")) {
            std::string surfaceType(argv[idx++]);
            if (surfaceType == "d3d11")
                opts->surfaceType = MFX_SURFACE_TYPE_D3D11_TEX2D;
            else if (surfaceType == "vaapi")
                opts->surfaceType = MFX_SURFACE_TYPE_VAAPI;
            else if (surfaceType == "opencl")
                opts->surfaceType = MFX_SURFACE_TYPE_OPENCL_IMG2D;
            else
                return false;
        }
        else if (IS_ARG_EQ(s, "multi_session")) {
            opts->bTestMultiSession = true;
        }
        else if (IS_ARG_EQ(s, "sc")) {
            std::string fourCC(argv[idx++]);
            if (fourCC == "nv12")
                opts->inFourCC = MFX_FOURCC_NV12;
            else if (fourCC == "rgb4")
                opts->inFourCC = MFX_FOURCC_RGB4;
            else
                return false;
        }
        else if (IS_ARG_EQ(s, "dc")) {
            std::string fourCC(argv[idx++]);
            if (fourCC == "nv12")
                opts->outFourCC = MFX_FOURCC_NV12;
            else if (fourCC == "rgb4")
                opts->outFourCC = MFX_FOURCC_RGB4;
            else
                return false;
        }
        else if (IS_ARG_EQ(s, "sw")) {
            if (!ValidateSize(argv[idx++], &opts->srcWidth, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "sh")) {
            if (!ValidateSize(argv[idx++], &opts->srcHeight, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "dw")) {
            if (!ValidateSize(argv[idx++], &opts->dstWidth, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "dh")) {
            if (!ValidateSize(argv[idx++], &opts->dstHeight, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "adapters")) {
            opts->bShowAdaptersInfo = true;
            return true;
        }
        else {
            printf("\nERROR: '-%s' is not supported\n", s);
            return false;
        }
    }

    // validate required parameters
    if (opts->infileName.empty()) {
        printf("ERROR: input file name (-i) is required\n");
        return false;
    }

    if (opts->outfileName.empty()) {
        printf("ERROR: out file name (-o) is required\n");
        return false;
    }

    if (opts->surfaceComponent == MFX_SURFACE_COMPONENT_UNKNOWN) {
        printf("ERROR: '-surface_component' is required\n");
        opts->outfileName.clear();
        return false;
    }

    if ((!opts->srcWidth || !opts->srcHeight || !opts->inFourCC) &&
        (opts->surfaceComponent == MFX_SURFACE_COMPONENT_ENCODE ||
         opts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_INPUT ||
         opts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_OUTPUT)) {
        printf("ERROR: '-sw', '-sh', '-sc' are required\n");
        opts->outfileName.clear();
        return false;
    }

    if ((!opts->dstWidth || !opts->dstHeight || !opts->outFourCC) &&
        (opts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_INPUT ||
         opts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_OUTPUT)) {
        printf("ERROR: '-dw', '-dh', '-dc' are required\n");
        opts->outfileName.clear();
        return false;
    }

    if (opts->surfaceMode != SURFACE_MODE_UNKNOWN &&
        opts->surfaceType == MFX_SURFACE_TYPE_UNKNOWN) {
        printf("ERROR: '-surface_type' is required for surface sharing mode\n");
        opts->outfileName.clear();
        return false;
    }

    // encode, vpp in -> import
    // decode, vpp out -> export
    if (opts->surfaceComponent == MFX_SURFACE_COMPONENT_ENCODE ||
        opts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_INPUT) {
        if (opts->surfaceMode == SURFACE_MODE_COPY)
            opts->surfaceFlag = MFX_SURFACE_FLAG_IMPORT_COPY;
        else if (opts->surfaceMode == SURFACE_MODE_SHARED)
            opts->surfaceFlag = MFX_SURFACE_FLAG_IMPORT_SHARED;
    }
    else if (opts->surfaceComponent == MFX_SURFACE_COMPONENT_DECODE ||
             opts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_OUTPUT) {
        if (opts->surfaceMode == SURFACE_MODE_COPY)
            opts->surfaceFlag = MFX_SURFACE_FLAG_EXPORT_COPY;
        else if (opts->surfaceMode == SURFACE_MODE_SHARED)
            opts->surfaceFlag = MFX_SURFACE_FLAG_EXPORT_SHARED;
    }

    if (opts->surfaceMode == SURFACE_MODE_UNKNOWN &&
        opts->surfaceComponent != MFX_SURFACE_COMPONENT_UNKNOWN) {
        opts->bNoSSA = true;
    }

    return true;
}

const char *FourCCToString(mfxU32 fourCC) {
    switch (fourCC) {
        case MFX_FOURCC_NV12:
            return (const char *)"NV12";
        case MFX_FOURCC_BGR4:
            return (const char *)"BGR4";
        case MFX_FOURCC_RGB4:
            return (const char *)"RGB4";
        default:
            return (const char *)"unknown";
    }
}

void ShowTestInfo(Options *opts) {
    printf("Running:\n");

    printf("\tSurface Component = ");
    if (opts->surfaceComponent == MFX_SURFACE_COMPONENT_ENCODE)
        printf("Encode\n");
    else if (opts->surfaceComponent == MFX_SURFACE_COMPONENT_DECODE)
        printf("Decode\n");
    else if (opts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_INPUT)
        printf("VPP In\n");
    else if (opts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_OUTPUT)
        printf("VPP Out\n");

    if (opts->bNoSSA == true) {
        printf("\tMode              = Not use surface sharing api\n");
    }
    else {
        printf("\tSurface Type      = ");
        if (opts->surfaceType == MFX_SURFACE_TYPE_D3D11_TEX2D)
            printf("D3D11 Tex2d\n");
        else if (opts->surfaceType == MFX_SURFACE_TYPE_VAAPI)
            printf("VAAPI\n");
        else if (opts->surfaceType == MFX_SURFACE_TYPE_OPENCL_IMG2D)
            printf("Opencl Img2d\n");

        printf("\tSurface Flag      = ");
        if (opts->surfaceComponent == MFX_SURFACE_COMPONENT_ENCODE ||
            opts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_INPUT)
            printf("Import");
        else if (opts->surfaceComponent == MFX_SURFACE_COMPONENT_DECODE ||
                 opts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_OUTPUT)
            printf("Export");
        printf(".");
        if (opts->surfaceMode == SURFACE_MODE_COPY)
            printf("Copy\n");
        else if (opts->surfaceMode == SURFACE_MODE_SHARED)
            printf("Shared\n");
    }

    if (opts->surfaceComponent == MFX_SURFACE_COMPONENT_ENCODE ||
        opts->surfaceComponent == MFX_SURFACE_COMPONENT_DECODE)
        printf("\tCodec             = %s\n", opts->codecID == MFX_CODEC_AVC ? "H.264" : "H.265");

    printf("\tInput File        = %s\n", opts->infileName.c_str());
    printf("\tOutput File       = %s\n",
           opts->outfileName.empty() ? "None" : opts->outfileName.c_str());

    printf("\tInput Colorspace  = %s\n", FourCCToString(opts->inFourCC));
    printf("\tInput Resolution  = %d x %d\n", opts->srcWidth, opts->srcHeight);

    if (opts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_INPUT ||
        opts->surfaceComponent == MFX_SURFACE_COMPONENT_VPP_OUTPUT) {
        printf("\tOutput Colorspace = %s\n", FourCCToString(opts->outFourCC));
        printf("\tOutput Resolution = %d x %d\n", opts->dstWidth, opts->dstHeight);
    }
}

const char *_print_Impl(mfxIMPL impl) {
    switch (impl) {
        STRING_OPTION(MFX_IMPL_TYPE_SOFTWARE);
        STRING_OPTION(MFX_IMPL_TYPE_HARDWARE);
    }

    return "<unknown implementation>";
}

const char *_print_AccelMode(mfxAccelerationMode mode) {
    switch (mode) {
        STRING_OPTION(MFX_ACCEL_MODE_NA);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_D3D9);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_D3D11);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_DRM_MODESET);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_GLX);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_X11);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_WAYLAND);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_HDDLUNITE);
    }

    return "<unknown acceleration mode>";
}

const char *_print_MediaAdapterType(mfxMediaAdapterType type) {
    switch (type) {
        STRING_OPTION(MFX_MEDIA_UNKNOWN);
        STRING_OPTION(MFX_MEDIA_INTEGRATED);
        STRING_OPTION(MFX_MEDIA_DISCRETE);
    }

    return "<unknown media adapter type>";
}

mfxStatus GetAdaptersInfo(Options *opts, bool bPrint) {
    mfxStatus sts    = MFX_ERR_NONE;
    mfxLoader loader = MFXLoad();
    VERIFY(loader != nullptr,
           "Error - MFXLoad() returned null - no libraries found",
           MFX_ERR_NULL_PTR);

    mfxConfig cfg[3] = {};
    mfxVariant cfgVal;

    // Implementation used must provide API version 2.10 or newer
    cfg[0]          = MFXCreateConfig(loader);
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = VPLVERSION(MAJOR_API_VERSION_REQUIRED, MINOR_API_VERSION_REQUIRED);
    sts             = MFXSetConfigFilterProperty(cfg[0],
                                     (mfxU8 *)"mfxImplDescription.ApiVersion.Version",
                                     cfgVal);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for Impl", sts);

    // Implementation used must be the hardware implementation
    cfg[1]          = MFXCreateConfig(loader);
    cfgVal.Type     = MFX_VARIANT_TYPE_U32;
    cfgVal.Data.U32 = MFX_IMPL_TYPE_HARDWARE;

    sts = MFXSetConfigFilterProperty(cfg[1], (mfxU8 *)"mfxImplDescription.Impl", cfgVal);
    VERIFY(MFX_ERR_NONE == sts, "ERROR: MFXSetConfigFilterProperty failed for Impl", sts);

    // Implementation used must be d3d11 acceleration mode
    cfg[2]      = MFXCreateConfig(loader);
    cfgVal.Type = MFX_VARIANT_TYPE_U32;
#ifdef _WIN32
    cfgVal.Data.U32 = MFX_ACCEL_MODE_VIA_D3D11;
#else
    cfgVal.Data.U32 = MFX_ACCEL_MODE_VIA_VAAPI;
#endif
    sts =
        MFXSetConfigFilterProperty(cfg[2], (mfxU8 *)"mfxImplDescription.AccelerationMode", cfgVal);
    VERIFY(MFX_ERR_NONE == sts,
           "ERROR: MFXSetConfigFilterProperty failed for acceleration mode",
           sts);

    int i                     = 0;
    mfxImplDescription *idesc = nullptr;
    while (1) {
        sts = MFXEnumImplementations(loader,
                                     i,
                                     MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                     reinterpret_cast<mfxHDL *>(&idesc));
        if (sts == MFX_ERR_NOT_FOUND) {
            // no more implementation, status is not a failure
            sts = MFX_ERR_NONE;
            break;
        }

        if (sts != MFX_ERR_NONE) {
            printf("\nMFXEnumImplementations(description) failure: %d\n", sts);
            break;
        }

        printf("\nImplementation #%d: %s\n", i, idesc->ImplName);

        // get path if supported (available starting with API 2.4)
        mfxHDL hImplPath = nullptr;
        sts              = MFXEnumImplementations(loader, i, MFX_IMPLCAPS_IMPLPATH, &hImplPath);
        if (sts != MFX_ERR_NONE || hImplPath == nullptr) {
            printf("\nMFXEnumImplementations(implpath) failure: %d\n", sts);
            break;
        }

        printf("%2sLibrary path: %s\n", "", reinterpret_cast<mfxChar *>(hImplPath));
        MFXDispReleaseImplDescription(loader, hImplPath);

        printf("%2sAccelerationMode: %s\n", "", _print_AccelMode(idesc->AccelerationMode));
        printf("%2sApiVersion: %hu.%hu\n", "", idesc->ApiVersion.Major, idesc->ApiVersion.Minor);
        printf("%2sImpl: %s\n", "", _print_Impl(idesc->Impl));
        printf("%2sVendorImplID: 0x%04X\n", "", idesc->VendorImplID);
        printf("%2sVendorID: 0x%04X\n", "", idesc->VendorID);

        /* mfxDeviceDescription */
        mfxDeviceDescription *dev = &idesc->Dev;
        printf("%2sDeviceID: %s\n", "", dev->DeviceID);

        mfxU32 adapterNum = 0;
        std::string str   = dev->DeviceID;
        std::size_t pos   = str.find('/');
        if (pos != std::string::npos) {
            std::string strAdapterNum = str.substr(pos + 1);
            adapterNum                = std::stoul(strAdapterNum);
        }
        else {
            printf("Can't find adapter number from device id (%s)\n", dev->DeviceID);
            sts = MFX_ERR_NOT_FOUND;
            break;
        }

        if (dev->Version.Version >= MFX_STRUCT_VERSION(1, 1)) {
            mfxMediaAdapterType eMA = (mfxMediaAdapterType)dev->MediaAdapterType;
            if (eMA != MFX_MEDIA_UNKNOWN) {
#ifdef __linux__
                mfxExtendedDeviceId *idescDevice = nullptr;
                sts                              = MFXEnumImplementations(loader,
                                             i,
                                             MFX_IMPLCAPS_DEVICE_ID_EXTENDED,
                                             reinterpret_cast<mfxHDL *>(&idescDevice));
                if (sts == MFX_ERR_NONE) {
                    // In Linux, use drm render node number for multi-session testing
                    adapterNum = idescDevice->DRMRenderNodeNum;
                    MFXDispReleaseImplDescription(loader, idescDevice);
                }
                else {
                    printf("Can't get device information\n");
                    break;
                }
#endif
                auto it = std::find_if(opts->adapterNumbers.begin(),
                                       opts->adapterNumbers.end(),
                                       [&adapterNum](const mfxU32 &existingElement) {
                                           return existingElement == adapterNum;
                                       });

                if (it == opts->adapterNumbers.end()) {
                    opts->adapterNumbers.push_back(adapterNum);
                }
            }

            printf("%2sMediaAdapterType: %s\n", "", _print_MediaAdapterType(eMA));
        }

        MFXDispReleaseImplDescription(loader, idesc);

        i++;
    }

    if (idesc)
        MFXDispReleaseImplDescription(loader, idesc);

    if (sts != MFX_ERR_NONE)
        return sts;

    if (i == 0)
        printf("\nWarning - no implementations found by MFXEnumImplementations()\n");
    else {
        printf("\nTotal number of implementations found = %d\n", i);
        printf("\nAvailable adapter numbers for multi-session test = %d\n",
               (int)opts->adapterNumbers.size());
        for (size_t j = 0; j < opts->adapterNumbers.size(); j++) {
            printf("  [%d] adapter number: %d\n", (int)j, opts->adapterNumbers[j]);
        }
    }

    MFXUnload(loader);

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

#ifdef _WIN32
// Write raw frame to file from ID3D11Texture2D
mfxStatus WriteRawFrame(ID3D11Device *pDevice, ID3D11Texture2D *pTex2D, std::ofstream &f) {
    CComPtr<ID3D11DeviceContext> deviceContext;
    D3D11_TEXTURE2D_DESC desc = {};
    CComPtr<ID3D11Texture2D> tex2DToSave;

    pDevice->GetImmediateContext(&deviceContext);
    pTex2D->GetDesc(&desc);

    if (desc.Usage != D3D11_USAGE_STAGING || (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ) == 0) {
        D3D11_TEXTURE2D_DESC sdsc = desc;
        sdsc.Usage                = D3D11_USAGE_STAGING;
        sdsc.BindFlags            = 0;
        sdsc.CPUAccessFlags       = D3D11_CPU_ACCESS_READ;
        sdsc.MiscFlags            = 0;

        HRESULT hr = pDevice->CreateTexture2D(&sdsc, nullptr, &tex2DToSave);

        if (FAILED(hr))
            return MFX_ERR_UNKNOWN;

        deviceContext->CopyResource(tex2DToSave, pTex2D);
    }
    else
        tex2DToSave = pTex2D;

    D3D11_MAPPED_SUBRESOURCE mdata;
    deviceContext->Map(tex2DToSave, 0, D3D11_MAP_READ, 0, &mdata);
    int i;
    mfxU16 w     = desc.Width;
    mfxU16 h     = desc.Height;
    mfxU16 pitch = mdata.RowPitch;

    if (desc.Format == DXGI_FORMAT_NV12) {
        mfxU8 *y  = reinterpret_cast<mfxU8 *>(mdata.pData);
        mfxU8 *uv = y + pitch * h;

        // Y
        for (i = 0; i < h; i++) {
            f.write((char *)(y + i * pitch), w);
        }
        // UV
        h /= 2;
        for (i = 0; i < h; i++) {
            f.write((char *)(uv + i * pitch), w);
        }
    }
    else if (desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM) {
        mfxU8 *data = reinterpret_cast<mfxU8 *>(mdata.pData);
        for (i = 0; i < h; i++) {
            f.write((char *)(data + i * pitch), pitch);
        }
    }
    else {
        return MFX_ERR_UNSUPPORTED;
    }

    deviceContext->Unmap(tex2DToSave, 0);

    return MFX_ERR_NONE;
}
#else
mfxStatus WriteRawFrame(VADisplay vaDisplay, VASurfaceID vaSurfaceID, std::ofstream &f) {
    VAStatus vaSts = VA_STATUS_SUCCESS;
    VAImage vaImg = {};

    vaSts = vaDeriveImage(vaDisplay, vaSurfaceID, &vaImg);
    if (vaSts != VA_STATUS_SUCCESS)
        return MFX_ERR_LOCK_MEMORY;

    void *pImgBuff = nullptr;
    vaSts = vaMapBuffer(vaDisplay, vaImg.buf, &pImgBuff);
    if (vaSts != VA_STATUS_SUCCESS)
        return MFX_ERR_LOCK_MEMORY;

    int i;
    mfxU16 w = vaImg.width;
    mfxU16 h = vaImg.height;
    mfxU16 pitch = vaImg.pitches[0];

    mfxU8 *data = (mfxU8 *)pImgBuff;

    if (vaImg.format.fourcc == VA_FOURCC_NV12) {
        mfxU8 *y = data + vaImg.offsets[0];
        mfxU8 *uv = data + vaImg.offsets[1];

        // Y
        for (i = 0; i < h; i++) {
            f.write((char *)(y + i * pitch), w);
        }
        // UV
        h /= 2;
        for (i = 0; i < h; i++) {
            f.write((char *)(uv + i * pitch), w);
        }
    }
    else {
        for (i = 0; i < h; i++) {
            f.write((char *)(data + i * pitch), pitch);
        }
    }

    vaSts = vaUnmapBuffer(vaDisplay, vaImg.buf);
    if (vaSts != VA_STATUS_SUCCESS)
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    vaSts = vaDestroyImage(vaDisplay, vaImg.image_id);
    if (vaSts != VA_STATUS_SUCCESS)
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    return MFX_ERR_NONE;
}
#endif

// Write raw frame to file from mfxFrameSurface1
mfxStatus WriteRawFrame(mfxFrameSurface1 *surface, std::ofstream &f) {
    mfxStatus sts = surface->FrameInterface->Map(surface, MFX_MAP_READ);
    if (sts != MFX_ERR_NONE) {
        printf("mfxFrameSurfaceInterface->Map failed (%d)\n", sts);
        return sts;
    }

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

    sts = surface->FrameInterface->Unmap(surface);
    if (sts != MFX_ERR_NONE) {
        printf("mfxFrameSurfaceInterface->Unmap failed (%d)\n", sts);
        return sts;
    }

    return MFX_ERR_NONE;
}

mfxStatus ReadRawFrame(FrameInfo fi, mfxU8 *bs, std::ifstream &f) {
    mfxU16 w     = fi.width;
    mfxU16 h     = fi.height;
    mfxU16 pitch = fi.pitch;

    size_t bytes_read;

    if (fi.fourcc == MFX_FOURCC_NV12) {
        // Y
        mfxU8 *y = bs;
        for (int i = 0; i < h; i++) {
            f.read((char *)(y + pitch * i), w);
            bytes_read = (mfxU32)f.gcount();
            if (pitch != bytes_read)
                return MFX_ERR_MORE_DATA;
        }
        // UV
        mfxU8 *uv = bs + pitch * h;
        h /= 2;
        for (int i = 0; i < h; i++) {
            f.read((char *)(uv + pitch * i), w);
            bytes_read = (mfxU32)f.gcount();
            if (pitch != bytes_read)
                return MFX_ERR_MORE_DATA;
        }
    }
    else if (fi.fourcc == MFX_FOURCC_RGB4) {
        for (int i = 0; i < h; i++) {
            f.read((char *)(bs + pitch * i), pitch);
            bytes_read = (mfxU32)f.gcount();
            if (pitch != bytes_read)
                return MFX_ERR_MORE_DATA;
        }
    }

    return MFX_ERR_NONE;
}

// Load raw I420 frames to mfxFrameSurface
mfxStatus ReadRawFrame(mfxFrameSurface1 *surface, std::ifstream &f) {
    mfxStatus sts = surface->FrameInterface->Map(surface, MFX_MAP_WRITE);
    if (sts != MFX_ERR_NONE) {
        printf("mfxFrameSurfaceInterface->Map failed (%d)\n", sts);
        return sts;
    }

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
                f.read((char *)(data->B + i * pitch), pitch);
                bytes_read = (mfxU32)f.gcount();
                if (pitch != bytes_read)
                    return MFX_ERR_MORE_DATA;
            }
            break;
        default:
            printf("Unsupported FourCC code, skip LoadRawFrame\n");
            break;
    }

    sts = surface->FrameInterface->Unmap(surface);
    if (sts != MFX_ERR_NONE) {
        printf("mfxFrameSurfaceInterface->Unmap failed (%d)\n", sts);
        return sts;
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
