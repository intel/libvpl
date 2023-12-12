//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_PROCESS_FRAMES_OCL_H_
#define TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_PROCESS_FRAMES_OCL_H_

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

#define VERIFY_OCL(err)   \
    {                     \
        if (err)          \
            return (err); \
    }

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
              m_width(0),
              m_height(0),

              m_clBufWritableY(),
              m_clBufWritableUV(),

              m_GlobalWorkSizeY(),
              m_GlobalWorkSizeUV(),
              m_LocalWorkSizeY(),
              m_LocalWorkSizeUV() {}

    ~OpenCLCtxD3D11() {
#if defined _DEBUG
        if (m_clcontext) {
            // for debugging - print reference counts of surfaces at teardown
            // expect any allocated surface to have refcount of 1 at this point
            DebugDumpSurfaceRefcount();
        }
#endif
        if (m_clBufWritableY[0])
            clReleaseMemObject(m_clBufWritableY[0]);

        if (m_clBufWritableUV[0])
            clReleaseMemObject(m_clBufWritableUV[0]);

        if (m_clBufWritableY[1])
            clReleaseMemObject(m_clBufWritableY[1]);

        if (m_clBufWritableUV[1])
            clReleaseMemObject(m_clBufWritableUV[1]);

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
    }

    __inline mfxStatus AquireOCLSharedSurface(cl_mem surface) {}

    // initialize OpenCL processing
    mfxStatus OpenCLInit(mfxHandleType handleType, mfxHDL handle, mfxU32 width, mfxU32 height, std::string oclFileName);

    // process surface with kernels read during OpenCLInit
    cl_int OpenCLProcessSurface(cl_mem clBufInY, cl_mem clBufInUV, cl_mem clBufOutY, cl_mem clBufOutUV);

    // copy a system memory buffer into an OCL surface
    cl_int CopySurfaceSystemToOCL(CPUFrameInfo_NV12 *cpuFrameInfo, cl_mem memY, cl_mem memUV);

    // copy an OCL surface into a system memory buffer
    cl_int CopySurfaceOCLToSystem(cl_mem memY, cl_mem memUV, CPUFrameInfo_NV12 *cpuFrameInfo);

    // request OCL surface for input to kernel
    cl_int GetOCLInputSurface(cl_mem *memY, cl_mem *memUV, bool bSharedD3D11 = false, ID3D11Texture2D *pSurfIn = nullptr, mfxU32 width = 0, mfxU32 height = 0);

    // request OCL surface for output from kernel
    cl_int GetOCLOutputSurface(cl_mem *memY, cl_mem *memUV, bool bSharedD3D11 = false, ID3D11Texture2D *pSurfIn = nullptr, mfxU32 width = 0, mfxU32 height = 0);

    // release surface obtained with GetOCLInputSurface() or GetOCLOutputSurface()
    cl_int ReleaseOCLSurface(cl_mem memY, cl_mem memUV, bool bSharedD3D11 = false);

    // lock shared D3D11 surface for OCL use
    cl_int EnqueueAcquireSurfaces(cl_mem memY, cl_mem memUV);

    // unlock shared D3D11 surface
    cl_int EnqueueReleaseSurfaces(cl_mem memY, cl_mem memUV);

    cl_context GetOpenCLContext() {
        return m_clcontext;
    }

    cl_command_queue GetOpenCLCommandQueue() {
        return m_clqueue;
    }

private:
    // initialization
    int ReadProgramFile(const std::string oclFileName, std::string &oclProgStr);
    cl_int InitPlatform();
    cl_int InitSurfaceSharingExtension();
    cl_int InitDevice();
    cl_int BuildKernels();
    size_t ChooseLocalSize(size_t globalSize, size_t preferred);

    // allocate a new OCL surface
    cl_int AllocateOCLSurface(cl_mem *memY, cl_mem *memUV, bool bSharedD3D11 = false, ID3D11Texture2D *pSurfIn = nullptr, mfxU32 width = 0, mfxU32 height = 0);

    static cl_int SetKernelArgs(cl_kernel clkernelY, cl_kernel clkernelUV, cl_mem clBufInY, cl_mem clBufInUV, cl_mem clBufOutY, cl_mem clBufOutUV);

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

    mfxU32 m_width;
    mfxU32 m_height;

    cl_mem m_clBufWritableY[MAX_NUM_OCL_SURFACES];
    cl_mem m_clBufWritableUV[MAX_NUM_OCL_SURFACES];

    size_t m_GlobalWorkSizeY[2];
    size_t m_GlobalWorkSizeUV[2];
    size_t m_LocalWorkSizeY[2];
    size_t m_LocalWorkSizeUV[2];
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

#endif // TOOLS_CLI_VPL_IMPORT_EXPORT_SRC_PROCESS_FRAMES_OCL_H_
