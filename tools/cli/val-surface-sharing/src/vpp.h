//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef TOOLS_CLI_VAL_SURFACE_SHARING_SRC_VPP_H_
#define TOOLS_CLI_VAL_SURFACE_SHARING_SRC_VPP_H_

#include <memory>
#include <vector>
#include "./hw-device.h"
#include "./util.h"

#ifdef TOOLS_ENABLE_OPENCL
    #include "./process-frames-ocl.h"
#endif

class CVPPTest : public CTest {
public:
    CVPPTest();
    virtual ~CVPPTest();

    mfxStatus Init(int tIndex,
                   Options *opts,
                   std::vector<mfxU32> *adapterNumbers = nullptr) override;
    mfxStatus Run() override;

private:
    int m_tIndex;
    mfxLoader m_loader;
    mfxSession m_session;

    DevCtx *m_pDevCtx;
#ifdef TOOLS_ENABLE_OPENCL
    OpenCLCtx *m_pOclCtx;
#endif
    mfxVideoParam m_vppParams;
    Options *m_pOpts;
    FrameInfo m_frameInfo;
    FileInfo m_fileInfo;
    mfxMemoryInterface *m_memoryInterface;

    mfxStatus CreateVPLSession();
    mfxStatus ProcessStreamVPP();
};

#endif // TOOLS_CLI_VAL_SURFACE_SHARING_SRC_VPP_H_
