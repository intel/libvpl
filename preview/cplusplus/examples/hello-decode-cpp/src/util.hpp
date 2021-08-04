//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// Utility library header file for sample code
///
/// @file

#ifndef PREVIEW_CPLUSPLUS_EXAMPLES_COMMON_UTIL_UTIL_HPP_
#define PREVIEW_CPLUSPLUS_EXAMPLES_COMMON_UTIL_UTIL_HPP_

#include <string.h>
#include <map>

#include "vpl/preview/vpl.hpp"

#if (MFX_VERSION >= 2000)
    #include "vpl/mfxdispatcher.h"
#endif

#ifdef __linux__
    #include <fcntl.h>
    #include <unistd.h>
#endif

#if defined(_WIN32) || defined(_WIN64)

    #include <atlbase.h>
    #include <d3d11.h>
    #include <dxgi1_2.h>
    #include <windows.h>

CComPtr<ID3D11Device> g_pD3D11Device;
CComPtr<ID3D11DeviceContext> g_pD3D11Ctx;
CComPtr<IDXGIFactory2> g_pDXGIFactory;
IDXGIAdapter *g_pAdapter;

std::map<mfxMemId *, void *> allocResponses;
std::map<void *, mfxFrameAllocResponse> allocDecodeResponses;
std::map<void *, int> allocDecodeRefCount;

typedef struct {
    mfxMemId memId;
    mfxMemId memIdStage;
    uint16_t rw;
} CustomMemId;

const struct {
    mfxIMPL impl; // actual implementation
    uint32_t adapterID; // device adapter number
} implTypes[] = { { MFX_IMPL_HARDWARE, 0 },
                  { MFX_IMPL_HARDWARE2, 1 },
                  { MFX_IMPL_HARDWARE3, 2 },
                  { MFX_IMPL_HARDWARE4, 3 } };

    #define MSDK_SAFE_RELEASE(X) \
        {                        \
            if (X) {             \
                X->Release();    \
                X = NULL;        \
            }                    \
        }
#elif defined(__linux__)
    #ifdef LIBVA_SUPPORT
        #include "va/va.h"
        #include "va/va_drm.h"
    #endif
#endif

#define WAIT_100_MILLISECONDS 100
#define MAX_PATH              260
#define MAX_WIDTH             3840
#define MAX_HEIGHT            2160
#define IS_ARG_EQ(a, b)       (!strcmp((a), (b)))

#define VERIFY(x, y)       \
    if (!(x)) {            \
        printf("%s\n", y); \
        goto end;          \
    }

#define ALIGN16(value) (((value + 15) >> 4) << 4)
#define ALIGN32(X)     (((uint32_t)((X) + 31)) & (~(uint32_t)31))

enum ExampleParams { PARAM_IMPL = 0, PARAM_INFILE, PARAM_INRES, PARAM_COUNT };
enum ParamGroup {
    PARAMS_CREATESESSION = 0,
    PARAMS_DECODE,
    PARAMS_ENCODE,
    PARAMS_VPP,
    PARAMS_TRANSCODE
};

typedef struct _Params {
    mfxIMPL impl;
#if (MFX_VERSION >= 2000)
    mfxVariant implValue;
#endif

    char *infileName;
    char *inmodelName;

    uint16_t srcWidth;
    uint16_t srcHeight;

    bool useVideoMemory;
} Params;

char *ValidateFileName(char *in) {
    if (in) {
        if (strnlen(in, MAX_PATH) > MAX_PATH)
            return NULL;
    }

    return in;
}

bool ValidateSize(char *in, uint16_t *vsize, uint32_t vmax) {
    if (in) {
        *vsize = static_cast<uint16_t>(strtol(in, NULL, 10));
        if (*vsize <= vmax)
            return true;
    }

    *vsize = 0;
    return false;
}

bool ParseArgsAndValidate(int argc, char *argv[], Params *params, ParamGroup group) {
    int idx;
    char *s;

    // init all params to 0
    *params      = {};
    params->impl = MFX_IMPL_SOFTWARE;
#if (MFX_VERSION >= 2000)
    params->implValue.Type     = MFX_VARIANT_TYPE_U32;
    params->implValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
#endif

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
        else if (IS_ARG_EQ(s, "m")) {
            params->inmodelName = ValidateFileName(argv[idx++]);
            if (!params->inmodelName) {
                return false;
            }
        }
        else if (IS_ARG_EQ(s, "w")) {
            if (!ValidateSize(argv[idx++], &params->srcWidth, MAX_WIDTH))
                return false;
        }
        else if (IS_ARG_EQ(s, "h")) {
            if (!ValidateSize(argv[idx++], &params->srcHeight, MAX_HEIGHT))
                return false;
        }
        else if (IS_ARG_EQ(s, "hw")) {
            params->impl = MFX_IMPL_HARDWARE;
#if (MFX_VERSION >= 2000)
            params->implValue.Data.U32 = MFX_IMPL_TYPE_HARDWARE;
#endif
        }
        else if (IS_ARG_EQ(s, "sw")) {
            params->impl = MFX_IMPL_SOFTWARE;
#if (MFX_VERSION >= 2000)
            params->implValue.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
#endif
        }
        else if (IS_ARG_EQ(s, "vmem")) {
            params->useVideoMemory = true;
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

#if defined(_WIN32) || defined(_WIN64)
IDXGIAdapter *GetIntelDeviceAdapterHandle(mfxIMPL impl) {
    uint32_t adapterNum = 0;
    mfxIMPL baseImpl    = MFX_IMPL_BASETYPE(impl); // Extract Media SDK base implementation type

    // get corresponding adapter number
    for (uint8_t i = 0; i < sizeof(implTypes) / sizeof(implTypes[0]); i++) {
        if (implTypes[i].impl == baseImpl) {
            adapterNum = implTypes[i].adapterID;
            break;
        }
    }

    HRESULT hres =
        CreateDXGIFactory(__uuidof(IDXGIFactory2), reinterpret_cast<void **>(&g_pDXGIFactory));
    if (FAILED(hres))
        return NULL;

    IDXGIAdapter *adapter;
    hres = g_pDXGIFactory->EnumAdapters(adapterNum, &adapter);
    if (FAILED(hres))
        return NULL;

    return adapter;
}
#endif

void PrepareFrameInfo(mfxFrameInfo *fi, uint32_t format, uint16_t w, uint16_t h) {
    // Video processing input data format
    fi->FourCC        = format;
    fi->ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    fi->CropX         = 0;
    fi->CropY         = 0;
    fi->CropW         = w;
    fi->CropH         = h;
    fi->PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    fi->FrameRateExtN = 30;
    fi->FrameRateExtD = 1;
    // width must be a multiple of 16
    // height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
    fi->Width = ALIGN16(fi->CropW);
    fi->Height =
        (MFX_PICSTRUCT_PROGRESSIVE == fi->PicStruct) ? ALIGN16(fi->CropH) : ALIGN32(fi->CropH);
}

uint32_t GetSurfaceSize(uint32_t FourCC, uint32_t width, uint32_t height) {
    uint32_t nbytes = 0;

    switch (FourCC) {
        case MFX_FOURCC_I420:
        case MFX_FOURCC_NV12:
            nbytes = width * height + (width >> 1) * (height >> 1) + (width >> 1) * (height >> 1);
            break;
        case MFX_FOURCC_I010:
        case MFX_FOURCC_P010:
            nbytes = width * height + (width >> 1) * (height >> 1) + (width >> 1) * (height >> 1);
            nbytes *= 2;
            break;
        case MFX_FOURCC_RGB4:
            nbytes = width * height * 4;
            break;
        default:
            break;
    }

    return nbytes;
}

int GetFreeSurfaceIndex(mfxFrameSurface1 *SurfacesPool, uint16_t nPoolSize) {
    for (uint16_t i = 0; i < nPoolSize; i++) {
        if (0 == SurfacesPool[i].Data.Locked)
            return i;
    }
    return MFX_ERR_NOT_FOUND;
}

#endif //PREVIEW_CPLUSPLUS_EXAMPLES_COMMON_UTIL_UTIL_HPP_
