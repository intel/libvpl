//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
// Example using Intel® Video Processing Library (Intel® VPL)

///
/// Utility library header file for sample code
///
/// @file

#ifndef EXAMPLES_UTIL_CPP_
#define EXAMPLES_UTIL_CPP_

#include <util.h>

char *ValidateFileName(char *in) {
    if (in) {
        if (strnlen(in, MAX_PATH) > MAX_PATH)
            return NULL;
    }

    return in;
}

bool ValidateSize(char *in, mfxU16 *vsize, mfxU32 vmax) {
    if (in) {
        *vsize = static_cast<mfxU16>(strtol(in, NULL, 10));
        if (vmax && *vsize > vmax) {
            *vsize = 0;
            printf("Value %d exceeds %d\n", *vsize, vmax);
            return false;
        }
        if (*vsize % 2 != 0) {
            printf("Value %d not divisible by 2\n", *vsize);
            *vsize = 0;
            return false;
        }
    }

    return true;
}

bool ParseArgsAndValidate(int argc, char *argv[], Params *params, ParamGroup group) {
    int idx;
    char *s;

    // init all params to 0
    *params = {};

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
            if (!params->infileName)
                return false;
        }

        else if (IS_ARG_EQ(s, "w")) {
            if (!ValidateSize(argv[idx++], &params->srcWidth, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "h")) {
            if (!ValidateSize(argv[idx++], &params->srcHeight, MAX_HEIGHT))
                return false;
        }
    }

    // input file required by all except createsession
    if ((group != PARAMS_CREATESESSION) && (!params->infileName)) {
        printf("ERROR - input file name (-i) is required\n");
        return false;
    }

    // VPP and encode samples require an input resolution
    if ((PARAMS_VPP == group) || (PARAMS_ENCODE == group)) {
        if ((!params->srcWidth) || (!params->srcHeight)) {
            printf("ERROR - source width/height required\n");
            return false;
        }
    }

    return true;
}

// Show implementation info for Media SDK or Intel® VPL
mfxVersion ShowImplInfo(mfxSession session) {
    mfxIMPL impl;
    mfxVersion version = { 0, 1 };

    mfxStatus sts = MFXQueryIMPL(session, &impl);
    if (sts != MFX_ERR_NONE)
        return version;

    sts = MFXQueryVersion(session, &version);
    if (sts != MFX_ERR_NONE)
        return version;

    printf("Session loaded: ApiVersion = %d.%d \timpl= ", version.Major, version.Minor);

    switch (impl) {
        case MFX_IMPL_SOFTWARE:
            puts("Software");
            break;
        case MFX_IMPL_HARDWARE | MFX_IMPL_VIA_VAAPI:
            puts("Hardware:VAAPI");
            break;
        case MFX_IMPL_HARDWARE | MFX_IMPL_VIA_D3D11:
            puts("Hardware:D3D11");
            break;
        case MFX_IMPL_HARDWARE | MFX_IMPL_VIA_D3D9:
            puts("Hardware:D3D9");
            break;
        default:
            puts("Unknown");
            break;
    }

    return version;
}

// Show implementation info with Intel® VPL
void ShowImplementationInfo(mfxLoader loader, mfxU32 implnum) {
    mfxImplDescription *idesc = nullptr;
    mfxStatus sts;
    //Loads info about implementation at specified list location
    sts = MFXEnumImplementations(loader, implnum, MFX_IMPLCAPS_IMPLDESCSTRUCTURE, (mfxHDL *)&idesc);
    if (!idesc || (sts != MFX_ERR_NONE))
        return;

    printf("Implementation details:\n");
    printf("  ApiVersion:           %hu.%hu  \n", idesc->ApiVersion.Major, idesc->ApiVersion.Minor);
    printf("  Implementation type: HW\n");
    printf("  AccelerationMode via: ");
    switch (idesc->AccelerationMode) {
        case MFX_ACCEL_MODE_NA:
            printf("NA \n");
            break;
        case MFX_ACCEL_MODE_VIA_D3D9:
            printf("D3D9\n");
            break;
        case MFX_ACCEL_MODE_VIA_D3D11:
            printf("D3D11\n");
            break;
        case MFX_ACCEL_MODE_VIA_VAAPI:
            printf("VAAPI\n");
            break;
        case MFX_ACCEL_MODE_VIA_VAAPI_DRM_MODESET:
            printf("VAAPI_DRM_MODESET\n");
            break;
        case MFX_ACCEL_MODE_VIA_VAAPI_GLX:
            printf("VAAPI_GLX\n");
            break;
        case MFX_ACCEL_MODE_VIA_VAAPI_X11:
            printf("VAAPI_X11\n");
            break;
        case MFX_ACCEL_MODE_VIA_VAAPI_WAYLAND:
            printf("VAAPI_WAYLAND\n");
            break;
        case MFX_ACCEL_MODE_VIA_HDDLUNITE:
            printf("HDDLUNITE\n");
            break;
        default:
            printf("unknown\n");
            break;
    }
    printf("  DeviceID:             %s \n", idesc->Dev.DeviceID);
    MFXDispReleaseImplDescription(loader, idesc);

#if (MFX_VERSION >= 2004)
    //Show implementation path, added in 2.4 API
    mfxHDL implPath = nullptr;
    sts             = MFXEnumImplementations(loader, implnum, MFX_IMPLCAPS_IMPLPATH, &implPath);
    if (!implPath || (sts != MFX_ERR_NONE))
        return;

    printf("  Path: %s\n\n", reinterpret_cast<mfxChar *>(implPath));
    MFXDispReleaseImplDescription(loader, implPath);
#endif
}

// Write encoded stream to file
void WriteEncodedStream(mfxBitstream &bs, FILE *f) {
    fwrite(bs.Data + bs.DataOffset, 1, bs.DataLength, f);
    bs.DataLength = 0;
    return;
}

mfxStatus ReadRawFrameCPU_NV12(CPUFrameInfo_NV12 *cpuFrameInfo, FILE *f) {
    mfxU16 i;
    size_t bytes_read;

    mfxU8 *Y     = cpuFrameInfo->Y;
    mfxU8 *UV    = cpuFrameInfo->UV;
    mfxU16 w     = cpuFrameInfo->width;
    mfxU16 h     = cpuFrameInfo->height;
    mfxU16 pitch = cpuFrameInfo->pitch;

    // Y
    for (i = 0; i < h; i++) {
        bytes_read = (mfxU32)fread((char *)(Y + i * pitch), 1, w, f);
        if (w != bytes_read)
            return MFX_ERR_MORE_DATA;
    }

    // UV
    for (i = 0; i < h / 2; i++) {
        bytes_read = (mfxU32)fread((char *)(UV + i * pitch), 1, w, f);
        if (w != bytes_read)
            return MFX_ERR_MORE_DATA;
    }

    return MFX_ERR_NONE;
}

// Read encoded stream from file
mfxStatus ReadEncodedStream(mfxBitstream &bs, FILE *f) {
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
    bs.DataLength += (mfxU32)fread(bs.Data + bs.DataLength, 1, bs.MaxLength - bs.DataLength, f);
    if (bs.DataLength == 0)
        return MFX_ERR_MORE_DATA;

    return MFX_ERR_NONE;
}

mfxStatus WriteRawFrameCPU_NV12(CPUFrameInfo_NV12 *cpuFrameInfo, FILE *f) {
    mfxU8 *Y     = cpuFrameInfo->Y;
    mfxU8 *UV    = cpuFrameInfo->UV;
    mfxU16 w     = cpuFrameInfo->width;
    mfxU16 h     = cpuFrameInfo->height;
    mfxU16 pitch = cpuFrameInfo->pitch;

    // write the output to disk
    for (int i = 0; i < h; i++) {
        fwrite((char *)(Y + i * pitch), 1, w, f);
    }
    // UV
    h /= 2;
    for (int i = 0; i < h; i++) {
        fwrite((char *)(UV + i * pitch), 1, w, f);
    }

    return MFX_ERR_NONE;
}

#endif //EXAMPLES_UTIL_CPP_
