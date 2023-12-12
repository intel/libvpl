//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef TOOLS_CLI_VAL_SURFACE_SHARING_SRC_PROCESS_FRAMES_OCL_H_
#define TOOLS_CLI_VAL_SURFACE_SHARING_SRC_PROCESS_FRAMES_OCL_H_

#include <string>

#include "./defs.h"

// OpenCL headers
#ifndef CL_TARGET_OPENCL_VERSION
    #define CL_TARGET_OPENCL_VERSION 300
#endif

#include "CL/cl.h"
#include "CL/cl_d3d11.h"

// support max one input and one output surface
// (could be generalized into surface pool)
#define MAX_NUM_OCL_SURFACES 2

struct OpenCLCtxD3D11 {
    OpenCLCtxD3D11()
            : m_clplatform(),

              m_pfn_clGetDeviceIDsFromD3D11KHR(),
              m_pfn_clCreateFromD3D11Texture2DKHR(),
              m_pfn_clEnqueueAcquireD3D11ObjectsKHR(),
              m_pfn_clEnqueueReleaseD3D11ObjectsKHR(),

              m_pDevice(),
              m_pSharedSurfaces(),
              m_cldevice(),
              m_clcontext(),
              m_program_source(),
              m_kernelY_FuncName(),
              m_kernelUV_FuncName(),
              m_clprogram(),
              m_clkernelY(),
              m_clkernelUV(),
              m_clqueue(),

              m_clBufWritableY(),
              m_clBufWritableUV(),
              m_clBufWritableBGRA(),

              m_GlobalWorkSizeY(),
              m_GlobalWorkSizeUV(),
              m_LocalWorkSizeY(),
              m_LocalWorkSizeUV(),
              m_cpuFrame() {}

    ~OpenCLCtxD3D11() {
#if defined _DEBUG
        if (m_clcontext) {
            // for debugging - print reference counts of surfaces at teardown
            // expect any allocated surface to have refcount of 1 at this point
            DebugDumpSurfaceRefcount();
        }
#endif
        for (int i = 0; i < MAX_NUM_OCL_SURFACES; i++) {
            if (m_clBufWritableY[i])
                clReleaseMemObject(m_clBufWritableY[i]);

            if (m_clBufWritableUV[i])
                clReleaseMemObject(m_clBufWritableUV[i]);

            if (m_clBufWritableBGRA[i])
                clReleaseMemObject(m_clBufWritableBGRA[i]);
        }

        if (m_clqueue)
            clReleaseCommandQueue(m_clqueue);

        if (m_clkernelY)
            clReleaseKernel(m_clkernelY);

        if (m_clkernelUV)
            clReleaseKernel(m_clkernelUV);

        if (m_clprogram)
            clReleaseProgram(m_clprogram);

        if (m_clcontext)
            clReleaseContext(m_clcontext);

        if (m_cpuFrame.Y) {
            free(m_cpuFrame.Y);
        }

        if (m_cpuFrame.UV) {
            free(m_cpuFrame.UV);
        }

        if (m_cpuFrame.BGRA) {
            free(m_cpuFrame.BGRA);
        }
    }

    mfxStatus OpenCLInit(mfxHandleType handleType, mfxHDL handle);

    mfxStatus OpenCLProcessSurface(cl_mem clBufInY,
                                   cl_mem clBufInUV,
                                   cl_mem clBufOutY,
                                   cl_mem clBufOutUV);

    // set ocl exported surface frame information
    void SetOCLSurfaceFrameInfo(FrameInfo *frameInfo);

    // prepare frames memory on system for data copy
    mfxStatus AllocCpuFrameForExport(FrameInfo *frameInfo);

    // copy a system memory buffer into an OCL surface
    mfxStatus CopySurfaceSystemToOCL(CPUFrame *cpuFrame, cl_mem *surfaces);

    // copy an OCL surface into a system memory buffer
    mfxStatus CopySurfaceOCLToSystem(cl_mem *surfaces, CPUFrame *cpuFrame);

    // request OCL surface for input to kernel
    mfxStatus GetOCLInputSurface(cl_mem *surfaces,
                                 bool bSharedD3D11        = false,
                                 ID3D11Texture2D *pSurfIn = nullptr);

    // request OCL surface for output from kernel
    mfxStatus GetOCLOutputSurface(cl_mem *surfaces,
                                  bool bSharedD3D11        = false,
                                  ID3D11Texture2D *pSurfIn = nullptr);

    // release surface obtained with GetOCLInputSurface() or GetOCLOutputSurface()
    mfxStatus ReleaseOCLSurface(cl_mem *surfaces, bool bSharedD3D11 = false);

    cl_context GetOpenCLContext() {
        return m_clcontext;
    }

    cl_command_queue GetOpenCLCommandQueue() {
        return m_clqueue;
    }

    mfxStatus WriteRawFrame_NV12(mfxSurfaceOpenCLImg2D *pImg2D, std::ofstream &f);
    mfxStatus WriteRawFrame_BGRA(mfxSurfaceOpenCLImg2D *pImg2D, std::ofstream &f);

private:
    int readFile(const std::string oclFileName, std::string &oclProgStr);
    cl_int InitPlatform();
    cl_int InitSurfaceSharingExtension();
    cl_int InitDevice();
    mfxStatus InitKernels(std::string oclFileName);
    cl_int BuildKernels();
    size_t chooseLocalSize(size_t globalSize, size_t preferred);
    bool EnqueueAcquireSurfaces(cl_mem *surfaces, int nSurfaces);
    bool EnqueueReleaseSurfaces(cl_mem *surfaces, int nSurfaces);
    cl_int ProcessSurface(cl_kernel clkernelY,
                          cl_kernel clkernelUV,
                          cl_mem clBufInY,
                          cl_mem clBufInUV,
                          cl_mem clBufOutY,
                          cl_mem clBufOutUV);
    cl_int ReleaseResources(bool bFreeInBufs = true, bool bFreeOutBufs = true);

    // allocate a new OCL surface
    mfxStatus AllocateOCLSurface(cl_mem *surfaces,
                                 bool bSharedD3D11        = false,
                                 ID3D11Texture2D *pSurfIn = nullptr);

    // increase refcount (or sharing lock) for OCL surface (internal only, not to be called directly by application)
    mfxStatus OpenCLCtxD3D11::AddRefOCLSurface(cl_mem *surfaces, int nSurface, bool bSharedD3D11);

    static cl_int SetKernelArgs(cl_kernel clkernelY,
                                cl_kernel clkernelUV,
                                cl_mem clBufInY,
                                cl_mem clBufInUV,
                                cl_mem clBufOutY,
                                cl_mem clBufOutUV);

#if defined _DEBUG
    void DebugDumpSurfaceRefcount();
#endif
    cl_platform_id m_clplatform;

    clGetDeviceIDsFromD3D11KHR_fn m_pfn_clGetDeviceIDsFromD3D11KHR;
    clCreateFromD3D11Texture2DKHR_fn m_pfn_clCreateFromD3D11Texture2DKHR;
    clEnqueueAcquireD3D11ObjectsKHR_fn m_pfn_clEnqueueAcquireD3D11ObjectsKHR;
    clEnqueueReleaseD3D11ObjectsKHR_fn m_pfn_clEnqueueReleaseD3D11ObjectsKHR;

    ID3D11Device *m_pDevice;
    ID3D11Texture2D *m_pSharedSurfaces[2];
    cl_device_id m_cldevice;
    cl_context m_clcontext;

    std::string m_program_source;
    std::string m_kernelY_FuncName;
    std::string m_kernelUV_FuncName;

    cl_program m_clprogram = 0;
    cl_kernel m_clkernelY  = 0;
    cl_kernel m_clkernelUV = 0;

    cl_command_queue m_clqueue;

    FrameInfo *m_pframeInfo;
    FrameInfo *m_pframeInfoExported;

    cl_mem m_clBufWritableY[MAX_NUM_OCL_SURFACES];
    cl_mem m_clBufWritableUV[MAX_NUM_OCL_SURFACES];

    cl_mem m_clBufWritableBGRA[MAX_NUM_OCL_SURFACES];

    size_t m_GlobalWorkSizeY[2];
    size_t m_GlobalWorkSizeUV[2];
    size_t m_LocalWorkSizeY[2];
    size_t m_LocalWorkSizeUV[2];

    // especially for multi-session use case
    // frequent memory alloc/dealloc causes heap corruption sometimes
    CPUFrame m_cpuFrame;
};

struct OpenCLCtx
#if defined(_WIN32) || defined(_WIN64)
        : OpenCLCtxD3D11
#else

#endif
{
public:
    OpenCLCtx() {}
    ~OpenCLCtx() {}
};

#endif // TOOLS_CLI_VAL_SURFACE_SHARING_SRC_PROCESS_FRAMES_OCL_H_
