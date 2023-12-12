//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef TOOLS_CLI_VAL_SURFACE_SHARING_SRC_DECODE_H_
#define TOOLS_CLI_VAL_SURFACE_SHARING_SRC_DECODE_H_

#include <vector>
#include "./hw-device.h"
#include "./util.h"

#ifdef TOOLS_ENABLE_OPENCL
    #include "./process-frames-ocl.h"
#endif

class CDecodeTest : public CTest {
public:
    CDecodeTest();
    virtual ~CDecodeTest();

    mfxStatus Init(int tIndex, Options *opts, std::vector<mfxU32> *adapterNumbers = nullptr);
    mfxStatus Run();

private:
    int m_tIndex;
    mfxLoader m_loader;
    mfxSession m_session;

    DevCtx *m_pDevCtx;
#ifdef TOOLS_ENABLE_OPENCL
    OpenCLCtx *m_pOclCtx;
#endif
    mfxVideoParam m_decParams;
    Options *m_pOpts;
    FrameInfo m_frameInfo;
    FileInfo m_fileInfo;
    std::vector<mfxU8 *> m_bsInput;

    mfxStatus CreateVPLSession();
    mfxStatus ProcessStreamDecode();
};

#endif // TOOLS_CLI_VAL_SURFACE_SHARING_SRC_DECODE_H_
