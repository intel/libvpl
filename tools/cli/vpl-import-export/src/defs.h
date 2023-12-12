//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_DEFS_H_
#define TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_DEFS_H_

#include <atomic>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "vpl/mfx.h"

#define DEF_DST_WIDTH  640
#define DEF_DST_HEIGHT 480

#define MAX_WIDTH  3840
#define MAX_HEIGHT 2160

#define WAIT_100_MILLISECONDS  100
#define MAX_PATH               260
#define BITSTREAM_BUFFER_SIZE  2000000
#define MAX_NUM_CAPTURE_FRAMES 300

#define IS_ARG_EQ(a, b) (!strcmp((a), (b)))

#define ALIGN16(value)           (((value + 15) >> 4) << 4)
#define ALIGN32(X)               (((mfxU32)((X) + 31)) & (~(mfxU32)31))
#define VPLVERSION(major, minor) (major << 16 | minor)

#define MAJOR_API_VERSION_REQUIRED 2
#define MINOR_API_VERSION_REQUIRED 9

#define VERIFY(x, y)       \
    if (!(x)) {            \
        printf("%s\n", y); \
        return -1;         \
    }

#define VERIFY2(x, y)                       \
    if (!(x)) {                             \
        printf("%s\n", y);                  \
        return MFX_ERR_INVALID_VIDEO_PARAM; \
    }

// true if mask value b is set in mask m
#define DBG_MASK(m, b) ((m) & (b))

enum DebugMask {
    DBG_MASK_NATIVE_SURFACE_DESC  = 0x0001,
    DBG_MASK_ACTUAL_SURFACE_FLAGS = 0x0002,
};

enum TestMode {
    TEST_MODE_UNKNOWN = 0,

    TEST_MODE_RENDER = 1,
    TEST_MODE_CAPTURE,
    TEST_MODE_DECVPP_FILE,
    TEST_MODE_ENC_FILE,
};

enum SurfaceMode {
    SURFACE_MODE_UNKNOWN = 0,

    SURFACE_MODE_SHARED = 1,
    SURFACE_MODE_COPY   = 2,
};

struct Params {
    TestMode testMode;

    std::string infileName;
    std::string outfileName;
    std::string openCLfileName;

    mfxU32 outFourCC;
    mfxU32 surfaceMode;
    mfxU32 dbgMask;

    mfxU16 srcWidth;
    mfxU16 srcHeight;
    mfxU16 dstWidth;
    mfxU16 dstHeight;

    bool bEnableOpenCL;
    bool bEnableTiming;
};

struct FrameInfo {
    mfxU16 width;
    mfxU16 height;
    mfxU16 pitch;
};

struct FileInfo {
    std::ifstream infile;
    std::ofstream outfile;
    mfxBitstream bitstream;
};

struct CPUFrameInfo_NV12 {
    mfxU8 *Y;
    mfxU8 *UV;

    mfxU16 width;
    mfxU16 height;
    mfxU16 pitch;
};

struct SurfaceCaps {
    mfxSurfaceType SurfaceType;
    mfxSurfaceComponent SurfaceComponent;
    mfxU32 SurfaceFlags;
};

struct VPLSession {
    mfxLoader m_loader;
    mfxSession m_session;

    VPLSession() : m_loader(nullptr), m_session(nullptr) {}

    mfxStatus Open(std::list<SurfaceCaps> *surfaceCapsList);

    void Close() {
        if (m_session)
            MFXClose(m_session);
        m_session = nullptr;

        if (m_loader)
            MFXUnload(m_loader);
        m_loader = nullptr;
    }

    mfxLoader GetLoader() {
        return m_loader;
    }

    mfxSession GetSession() {
        return m_session;
    }

    ~VPLSession() {
        Close();
    }
};

#endif // TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_DEFS_H_
