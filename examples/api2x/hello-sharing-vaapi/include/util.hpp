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

#ifndef EXAMPLES_UTIL_HPP_
#define EXAMPLES_UTIL_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include "vpl/mfx.h"

#if (MFX_VERSION >= 2009)
    #include "vpl/mfxdispatcher.h"
#endif

#define WAIT_100_MILLISECONDS 100
#define MAX_PATH              260
#define MAX_WIDTH             3840
#define MAX_HEIGHT            2160
#define IS_ARG_EQ(a, b)       (!strcmp((a), (b)))

#define VERIFY(x, y)       \
    if (!(x)) {            \
        printf("%s\n", y); \
        exit(-1);          \
    }

#define ALIGN16(value)           (((value + 15) >> 4) << 4)
#define ALIGN32(X)               (((mfxU32)((X) + 31)) & (~(mfxU32)31))
#define VPLVERSION(major, minor) (major << 16 | minor)

enum ExampleParams { PARAM_IMPL = 0, PARAM_INFILE, PARAM_INRES, PARAM_COUNT };
enum ParamGroup {
    PARAMS_CREATESESSION = 0,
    PARAMS_DECODE,
    PARAMS_ENCODE,
    PARAMS_VPP,
    PARAMS_TRANSCODE
};

typedef struct _Params {
    char *infileName;
    char *inmodelName;

    mfxU16 srcWidth;
    mfxU16 srcHeight;
} Params;

struct CPUFrameInfo {
    mfxU8 *data;
    mfxU16 width;
    mfxU16 height;
    mfxU16 pitch;
};

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
        else if (IS_ARG_EQ(s, "m")) {
            params->inmodelName = ValidateFileName(argv[idx++]);
            if (!params->inmodelName)
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

//Shows implementation info for Media SDK or Intel® VPL
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

// Shows implementation info with Intel® VPL
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

mfxStatus ReadRawFrameNV12(const CPUFrameInfo &frameInfo, FILE *f) {
    mfxU16 w     = frameInfo.width;
    mfxU16 h     = frameInfo.height;
    mfxU16 pitch = frameInfo.pitch;
    mfxU8 *Y     = frameInfo.data;
    mfxU8 *UV    = frameInfo.data + pitch * h;

    size_t bytes_read;
    // Y
    for (int i = 0; i < h; i++) {
        bytes_read = (mfxU32)fread((char *)(Y + pitch * i), 1, w, f);
        if (pitch != bytes_read)
            return MFX_ERR_MORE_DATA;
    }
    // UV
    h /= 2;
    for (int i = 0; i < h; i++) {
        bytes_read = (mfxU32)fread((char *)(UV + pitch * i), 1, w, f);
        if (pitch != bytes_read)
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

// Write raw frame to file from vaSurface
mfxStatus WriteRawFrame(VADisplay vaDisplay, VASurfaceID vaSurfaceID, FILE *f) {
    VAStatus vaSts = VA_STATUS_SUCCESS;
    VAImage vaImg  = {};

    vaSts = vaDeriveImage(vaDisplay, vaSurfaceID, &vaImg);
    if (vaSts != VA_STATUS_SUCCESS)
        return MFX_ERR_LOCK_MEMORY;

    void *pImgBuff = nullptr;
    vaSts          = vaMapBuffer(vaDisplay, vaImg.buf, &pImgBuff);
    if (vaSts != VA_STATUS_SUCCESS)
        return MFX_ERR_LOCK_MEMORY;

    int i;
    mfxU16 w     = vaImg.width;
    mfxU16 h     = vaImg.height;
    mfxU16 pitch = vaImg.pitches[0];

    mfxU8 *data = (mfxU8 *)pImgBuff;

    mfxU8 *y  = data + vaImg.offsets[0];
    mfxU8 *uv = data + vaImg.offsets[1];

    // Y
    for (i = 0; i < h; i++) {
        fwrite((char *)(y + i * pitch), w, 1, f);
    }
    // UV
    h /= 2;
    for (i = 0; i < h; i++) {
        fwrite((char *)(uv + i * pitch), w, 1, f);
    }

    vaSts = vaUnmapBuffer(vaDisplay, vaImg.buf);
    if (vaSts != VA_STATUS_SUCCESS)
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    vaSts = vaDestroyImage(vaDisplay, vaImg.image_id);
    if (vaSts != VA_STATUS_SUCCESS)
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    return MFX_ERR_NONE;
}

#endif //EXAMPLES_UTIL_HPP_
