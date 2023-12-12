//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef TOOLS_CLI_VAL_SURFACE_SHARING_SRC_DEFS_H_
#define TOOLS_CLI_VAL_SURFACE_SHARING_SRC_DEFS_H_

#include <atomic>
#include <fstream>
#include <iostream>
#include <iterator>
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

#define VERIFY(x, y, sts)  \
    if (!(x)) {            \
        printf("%s\n", y); \
        return sts;        \
    }

#define VERIFY2(x, y)                       \
    if (!(x)) {                             \
        printf("%s\n", y);                  \
        return MFX_ERR_INVALID_VIDEO_PARAM; \
    }

// true if mask value b is set in mask m
#define DBG_MASK(m, b) ((m) & (b))

#define STRING_OPTION(x) \
    case x:              \
        return #x

enum DebugMask {
    DBG_MASK_NATIVE_SURFACE_DESC = 0x0001,
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

    SURFACE_MODE_COPY   = 1,
    SURFACE_MODE_SHARED = 2,
};

struct Options {
    TestMode testMode;

    std::string infileName;
    std::string outfileName;

    mfxU32 codecID;
    mfxU32 inFourCC;
    mfxU32 outFourCC;

    mfxU32 surfaceMode;
    mfxU32 surfaceFlag;
    mfxSurfaceType surfaceType;
    mfxSurfaceComponent surfaceComponent;

    mfxU16 srcWidth;
    mfxU16 srcHeight;
    mfxU16 dstWidth;
    mfxU16 dstHeight;

    bool bTestMultiSession;
    bool bNoSSA; // don't use surface sharing api
    bool bShowAdaptersInfo; // display all the adapter information

    // adapter information of GPUs, for multi-session test
    std::vector<mfxU32> adapterNumbers;
    mfxU32 adapterToRun; // set adapter number to be activated in each thread
};

struct FrameInfo {
    mfxU32 fourcc;
    mfxU16 width;
    mfxU16 height;
    mfxU16 pitch;
};

struct FileInfo {
    std::ifstream infile;
    std::ofstream outfile;
    mfxBitstream bitstream;
};

struct CPUFrame {
    mfxU8 *Y;
    mfxU8 *UV;
    mfxU8 *BGRA;
};

class CTest {
public:
    virtual mfxStatus Init(int tIndex,
                           Options *pOpts,
                           std::vector<mfxU32> *adapterNumbers = nullptr) = 0;
    virtual mfxStatus Run()                                               = 0;
};

#endif // TOOLS_CLI_VAL_SURFACE_SHARING_SRC_DEFS_H_
