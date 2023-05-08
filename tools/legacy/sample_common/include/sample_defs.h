/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __SAMPLE_DEFS_H__
#define __SAMPLE_DEFS_H__

#include <memory.h>
#include <algorithm>
#include <iostream>

#include "vm/file_defs.h"
#include "vm/strings_defs.h"
#include "vm/time_defs.h"
#include "vpl/mfxdefs.h"

inline const char* path_to_name(const char* path, char delim) {
    const char* last_delim = strrchr(path, '\\');
    if (last_delim && *last_delim) {
        return last_delim + 1;
    }
    else {
        return path;
    }
}

#if defined(WIN32) || defined(WIN64)
    #define __FILENAME__ path_to_name(__FILE__, '\\')
#else
    #define __FILENAME__ path_to_name(__FILE__, '/')
#endif

// Run-time HSBC
#if (MFX_VERSION >= MFX_VERSION_NEXT)
    #define ENABLE_VPP_RUNTIME_HSBC
#endif

#define ENABLE_MCTF
enum { MCTF_BITRATE_MULTIPLIER = 100000 };

#if defined(_WIN32) || defined(_WIN64)

enum {
    MFX_HANDLE_DEVICEWINDOW = 0x101 /* A handle to the render window */
}; //mfxHandleType

    #ifndef D3D_SURFACES_SUPPORT
        #define D3D_SURFACES_SUPPORT 1
    #endif

    #if defined(_WIN32) && !defined(MFX_D3D11_SUPPORT)
        #include <sdkddkver.h>
        #if (NTDDI_VERSION >= NTDDI_VERSION_FROM_WIN32_WINNT2(0x0602)) // >= _WIN32_WINNT_WIN8
            #define MFX_D3D11_SUPPORT 1 // Enable D3D11 support if SDK allows
        #else
            #define MFX_D3D11_SUPPORT 0
        #endif
    #endif // #if defined(_WIN32) && !defined(MFX_D3D11_SUPPORT)
#endif // #if defined(_WIN32) || defined(_WIN64)

enum {
#define __DECLARE(type) MFX_MONITOR_##type
    __DECLARE(Unknown) = 0,
    __DECLARE(AUTO)    = __DECLARE(Unknown),
    __DECLARE(VGA),
    __DECLARE(DVII),
    __DECLARE(DVID),
    __DECLARE(DVIA),
    __DECLARE(Composite),
    __DECLARE(SVIDEO),
    __DECLARE(LVDS),
    __DECLARE(Component),
    __DECLARE(9PinDIN),
    __DECLARE(HDMIA),
    __DECLARE(HDMIB),
    __DECLARE(eDP),
    __DECLARE(TV),
    __DECLARE(DisplayPort),
#if defined(DRM_MODE_CONNECTOR_VIRTUAL) // from libdrm 2.4.59
    __DECLARE(VIRTUAL),
#endif
#if defined(DRM_MODE_CONNECTOR_DSI) // from libdrm 2.4.59
    __DECLARE(DSI),
#endif
    __DECLARE(MAXNUMBER)
#undef __DECLARE
};

#if defined(LIBVA_SUPPORT)

enum LibVABackend {
    MFX_LIBVA_AUTO,
    MFX_LIBVA_DRM,
    MFX_LIBVA_DRM_RENDERNODE = MFX_LIBVA_DRM,
    MFX_LIBVA_DRM_MODESET,
    MFX_LIBVA_X11,
    MFX_LIBVA_WAYLAND
};

#endif

//affects win32 winnt version macro
#include "sample_utils.h"
#include "vm/time_defs.h"

#define MSDK_DEC_WAIT_INTERVAL     300000
#define MSDK_ENC_WAIT_INTERVAL     300000
#define MSDK_VPP_WAIT_INTERVAL     300000
#define MSDK_SURFACE_WAIT_INTERVAL 300000
#define MSDK_WAIT_INTERVAL                                 \
    (MSDK_DEC_WAIT_INTERVAL + 3 * MSDK_VPP_WAIT_INTERVAL + \
     MSDK_ENC_WAIT_INTERVAL) // an estimate for the longest pipeline we have in samples

#define MSDK_INVALID_SURF_IDX 0xFFFF

#define MSDK_MAX_FILENAME_LEN            1024
#define MSDK_MAX_USER_DATA_UNREG_SEI_LEN 80

#define MSDK_PRINT_RET_MSG(ERR, MSG)                                                    \
    {                                                                                   \
        std::stringstream tmpStr1;                                                      \
        tmpStr1 << std::endl                                                            \
                << "[ERROR], sts=" << StatusToString(ERR) << "(" << ERR << ")"          \
                << ", " << __FUNCTION__ << ", " << MSG << " at " << __FILENAME__ << ":" \
                << __LINE__ << std::endl;                                               \
        std::cerr << tmpStr1.str();                                                     \
    }

#define MSDK_PRINT_WRN_MSG(WRN, MSG)                                                    \
    {                                                                                   \
        std::stringstream tmpStr1;                                                      \
        tmpStr1 << std::endl                                                            \
                << "[WARNING], sts=" << StatusToString(WRN) << "(" << WRN << ")"        \
                << ", " << __FUNCTION__ << ", " << MSG << " at " << __FILENAME__ << ":" \
                << __LINE__ << std::endl;                                               \
        std::cerr << tmpStr1.str();                                                     \
    }

#define MSDK_TRACE_LEVEL(level, ERR)                                                  \
    if (level <= msdk_trace_get_level()) {                                            \
        std::cerr << __FILENAME__ << " :" << __LINE__ << " [" << level << "] " << ERR \
                  << std::endl;                                                       \
    }

#define MSDK_TRACE_CRITICAL(ERR) MSDK_TRACE_LEVEL(MSDK_TRACE_LEVEL_CRITICAL, ERR)
#define MSDK_TRACE_ERROR(ERR)    MSDK_TRACE_LEVEL(MSDK_TRACE_LEVEL_ERROR, ERR)
#define MSDK_TRACE_WARNING(ERR)  MSDK_TRACE_LEVEL(MSDK_TRACE_LEVEL_WARNING, ERR)
#define MSDK_TRACE_INFO(ERR)     MSDK_TRACE_LEVEL(MSDK_TRACE_LEVEL_INFO, ERR)
#define MSDK_TRACE_DEBUG(ERR)    MSDK_TRACE_LEVEL(MSDK_TRACE_LEVEL_DEBUG, ERR)

#define MSDK_CHECK_ERROR(P, X, ERR)                         \
    {                                                       \
        if ((X) == (P)) {                                   \
            std::stringstream tmpStr2;                      \
            tmpStr2 << #X << "==" << #P << " error";        \
            MSDK_PRINT_RET_MSG(ERR, tmpStr2.str().c_str()); \
            return ERR;                                     \
        }                                                   \
    }

#define MSDK_CHECK_NOT_EQUAL(P, X, ERR)                     \
    {                                                       \
        if ((X) != (P)) {                                   \
            std::stringstream tmpStr3;                      \
            tmpStr3 << #X << "!=" << #P << " error";        \
            MSDK_PRINT_RET_MSG(ERR, tmpStr3.str().c_str()); \
            return ERR;                                     \
        }                                                   \
    }

#define MSDK_CHECK_STATUS(X, MSG)       \
    {                                   \
        if ((X) < MFX_ERR_NONE) {       \
            MSDK_PRINT_RET_MSG(X, MSG); \
            return X;                   \
        }                               \
    }
#define MSDK_CHECK_STATUS_NO_RET(X, MSG) \
    {                                    \
        if ((X) < MFX_ERR_NONE) {        \
            MSDK_PRINT_RET_MSG(X, MSG);  \
        }                                \
    }
#define MSDK_CHECK_WRN(X, MSG)          \
    {                                   \
        if ((X) > MFX_ERR_NONE) {       \
            MSDK_PRINT_WRN_MSG(X, MSG); \
        }                               \
    }
#define MSDK_CHECK_ERR_NONE_STATUS(X, ERR, MSG) \
    {                                           \
        if ((X) != MFX_ERR_NONE) {              \
            MSDK_PRINT_RET_MSG(X, MSG);         \
            return ERR;                         \
        }                                       \
    }
#define MSDK_CHECK_ERR_NONE_STATUS_NO_RET(X, MSG) \
    {                                             \
        if ((X) != MFX_ERR_NONE) {                \
            MSDK_PRINT_RET_MSG(X, MSG);           \
        }                                         \
    }
#define MSDK_CHECK_NOERROR_STATUS_NO_RET(X, MSG)                         \
    {                                                                    \
        if ((X) != MFX_ERR_NONE && (X) != MFX_ERR_NONE_PARTIAL_OUTPUT) { \
            MSDK_PRINT_RET_MSG(X, MSG);                                  \
        }                                                                \
    }
#define MSDK_CHECK_PARSE_RESULT(P, X, ERR) \
    {                                      \
        if ((X) > (P)) {                   \
            return ERR;                    \
        }                                  \
    }

#define MSDK_CHECK_STATUS_SAFE(X, FUNC, ADD) \
    {                                        \
        if ((X) < MFX_ERR_NONE) {            \
            ADD;                             \
            MSDK_PRINT_RET_MSG(X, FUNC);     \
            return X;                        \
        }                                    \
    }
#define MSDK_IGNORE_MFX_STS(P, X) \
    {                             \
        if ((X) == (P)) {         \
            P = MFX_ERR_NONE;     \
        }                         \
    }
#define MSDK_CHECK_POINTER(P, ...)                                       \
    {                                                                    \
        if (!(P)) {                                                      \
            std::stringstream tmpStr4;                                   \
            tmpStr4 << #P << " pointer is NULL";                         \
            MSDK_PRINT_RET_MSG(MFX_ERR_NULL_PTR, tmpStr4.str().c_str()); \
            return __VA_ARGS__;                                          \
        }                                                                \
    }
#define MSDK_CHECK_POINTER_NO_RET(P)                                     \
    {                                                                    \
        if (!(P)) {                                                      \
            std::stringstream tmpStr4;                                   \
            tmpStr4 << #P << " pointer is NULL";                         \
            MSDK_PRINT_RET_MSG(MFX_ERR_NULL_PTR, tmpStr4.str().c_str()); \
            return;                                                      \
        }                                                                \
    }
#define MSDK_CHECK_POINTER_SAFE(P, ERR, ADD) \
    {                                        \
        if (!(P)) {                          \
            ADD;                             \
            return ERR;                      \
        }                                    \
    }
#define MSDK_BREAK_ON_ERROR(P)   \
    {                            \
        if (MFX_ERR_NONE != (P)) \
            break;               \
    }
#define MSDK_SAFE_DELETE_ARRAY(P) \
    {                             \
        if (P) {                  \
            delete[] P;           \
            P = NULL;             \
        }                         \
    }
#define MSDK_SAFE_RELEASE(X) \
    {                        \
        if (X) {             \
            X->Release();    \
            X = NULL;        \
        }                    \
    }
#define MSDK_SAFE_FREE(X) \
    {                     \
        if (X) {          \
            free(X);      \
            X = NULL;     \
        }                 \
    }

#ifndef MSDK_SAFE_DELETE
    #define MSDK_SAFE_DELETE(P) \
        {                       \
            if (P) {            \
                delete P;       \
                P = NULL;       \
            }                   \
        }
#endif // MSDK_SAFE_DELETE

#define MSDK_ZERO_MEMORY(VAR) \
    { memset(&VAR, 0, sizeof(VAR)); }
#define MSDK_ALIGN16(value) (((value + 15) >> 4) << 4) // round up to a multiple of 16
#define MSDK_ALIGN32(value) (((value + 31) >> 5) << 5) // round up to a multiple of 32
#define MSDK_ALIGN(value, alignment) \
    (alignment) * ((value) / (alignment) + (((value) % (alignment)) ? 1 : 0))
#define MSDK_ARRAY_LEN(value) (sizeof(value) / sizeof(value[0]))

#ifndef UNREFERENCED_PARAMETER
    #define UNREFERENCED_PARAMETER(par) std::ignore = par;
#endif

#define MFX_IMPL_VIA_MASK(x) (0x0f00 & (x))

// Deprecated
#define MSDK_PRINT_RET_MSG_(ERR) \
    { printf("\nReturn on error: error code %d,\t%s\t%d\n\n", (int)ERR, __FILENAME__, __LINE__); }
#define MSDK_CHECK_RESULT(P, X, ERR)  \
    {                                 \
        if ((X) > (P)) {              \
            MSDK_PRINT_RET_MSG_(ERR); \
            return ERR;               \
        }                             \
    }
#define MSDK_CHECK_RESULT_SAFE(P, X, ERR, ADD) \
    {                                          \
        if ((X) > (P)) {                       \
            ADD;                               \
            MSDK_PRINT_RET_MSG_(ERR);          \
            return ERR;                        \
        }                                      \
    }

namespace mfx {
// TODO: switch to std::clamp when C++17 support will be enabled

// Clip value v to range [lo, hi]
template <class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return std::min(hi, std::max(v, lo));
}

// Comp is comparison function object with meaning of 'less' operator (i.e. std::less<> or operator<)
template <class T, class Compare>
constexpr const T& clamp(const T& v, const T& lo, const T& hi, Compare comp) {
    return comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}
} // namespace mfx

#endif //__SAMPLE_DEFS_H__
