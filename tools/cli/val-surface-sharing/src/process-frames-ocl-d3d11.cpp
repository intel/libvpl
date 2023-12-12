//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifdef TOOLS_ENABLE_OPENCL
    #include "./process-frames-ocl.h"

// following is from MSDK samples (sample_plugin_opencl) - see opencl_filter_dx11.cpp

int OpenCLCtxD3D11::readFile(const std::string oclFileName, std::string &oclProgStr) {
    std::ifstream input(oclFileName.c_str(), std::ios::in | std::ios::binary);
    if (!input)
        return -1;

    input.seekg(0, std::ios::end);
    std::vector<char> program_source(static_cast<int>(input.tellg()));
    input.seekg(0);

    input.read(&program_source[0], program_source.size());

    oclProgStr = std::string(program_source.begin(), program_source.end());

    return 0;
}

// In multi-threaded situation,
// if clGetImageInfo is not called, then error -1073740791 (STATUS_STACK_BUFFER_OVERRUN) occurs.
// if memory is allocated/deallocated from local function, then error -1073740940 (STATUS_HEAP_CORRUPTION) occurs.
//
// Write NV12 raw frame to file from mfxSurfaceOpenCLImg2D
mfxStatus OpenCLCtxD3D11::WriteRawFrame_NV12(mfxSurfaceOpenCLImg2D *pImg2D, std::ofstream &f) {
    cl_mem d3d11Objects[] = { (cl_mem)pImg2D->ocl_image[0], (cl_mem)pImg2D->ocl_image[1] };
    cl_int error =
        m_pfn_clEnqueueAcquireD3D11ObjectsKHR((cl_command_queue)pImg2D->ocl_command_queue,
                                              pImg2D->ocl_image_num,
                                              d3d11Objects,
                                              0,
                                              nullptr,
                                              nullptr);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    size_t y_width, y_height;
    size_t uv_width, uv_height;

    // Y
    error |= clGetImageInfo((cl_mem)(pImg2D->ocl_image[0]),
                            CL_IMAGE_WIDTH,
                            sizeof(size_t),
                            &y_width,
                            NULL);
    error |= clGetImageInfo((cl_mem)(pImg2D->ocl_image[0]),
                            CL_IMAGE_HEIGHT,
                            sizeof(size_t),
                            &y_height,
                            NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    // UV
    error |= clGetImageInfo((cl_mem)(pImg2D->ocl_image[1]),
                            CL_IMAGE_WIDTH,
                            sizeof(size_t),
                            &uv_width,
                            NULL);
    error |= clGetImageInfo((cl_mem)(pImg2D->ocl_image[1]),
                            CL_IMAGE_HEIGHT,
                            sizeof(size_t),
                            &uv_height,
                            NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    // if width/height is different from original, then update info and reallocate cpu frame
    if (y_width != m_pframeInfoExported->width || y_height != m_pframeInfoExported->height ||
        (uv_width != (m_pframeInfoExported->width / 2)) ||
        (uv_height != (m_pframeInfoExported->height / 2))) {
        m_pframeInfoExported->width  = (mfxU16)y_width;
        m_pframeInfoExported->height = (mfxU16)y_height;

        mfxStatus sts = AllocCpuFrameForExport(m_pframeInfoExported);
        if (sts != MFX_ERR_NONE)
            return sts;
    }

    size_t origin[3]   = { 0, 0, 0 };
    size_t regionY[3]  = { y_width, y_height, 1 };
    size_t regionUV[3] = { uv_width, uv_height, 1 };

    cl_event readEventY;
    error = clEnqueueReadImage((cl_command_queue)pImg2D->ocl_command_queue,
                               (cl_mem)(pImg2D->ocl_image[0]),
                               CL_FALSE,
                               origin,
                               regionY,
                               0,
                               0,
                               m_cpuFrame.Y,
                               0,
                               NULL,
                               &readEventY);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    clWaitForEvents(1, &readEventY);
    clReleaseEvent(readEventY);

    // UV
    cl_event readEventUV;
    error = clEnqueueReadImage((cl_command_queue)pImg2D->ocl_command_queue,
                               (cl_mem)(pImg2D->ocl_image[1]),
                               CL_FALSE,
                               origin,
                               regionUV,
                               0,
                               0,
                               m_cpuFrame.UV,
                               0,
                               NULL,
                               &readEventUV);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    clWaitForEvents(1, &readEventUV);
    clReleaseEvent(readEventUV);

    error |= m_pfn_clEnqueueReleaseD3D11ObjectsKHR((cl_command_queue)pImg2D->ocl_command_queue,
                                                   pImg2D->ocl_image_num,
                                                   d3d11Objects,
                                                   0,
                                                   nullptr,
                                                   nullptr);

    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    // Y
    for (int i = 0; i < y_height; i++)
        f.write((char *)(m_cpuFrame.Y + i * y_width), y_width);

    // UV
    size_t pitch = uv_width * 2;
    for (int i = 0; i < uv_height; i++)
        f.write((char *)(m_cpuFrame.UV + i * pitch), pitch);

    return MFX_ERR_NONE;
}

// Write BGRA raw frame to file from mfxSurfaceOpenCLImg2D
mfxStatus OpenCLCtxD3D11::WriteRawFrame_BGRA(mfxSurfaceOpenCLImg2D *pImg2D, std::ofstream &f) {
    cl_mem d3d11Objects[] = { (cl_mem)pImg2D->ocl_image[0] };
    cl_int error =
        m_pfn_clEnqueueAcquireD3D11ObjectsKHR((cl_command_queue)pImg2D->ocl_command_queue,
                                              pImg2D->ocl_image_num,
                                              d3d11Objects,
                                              0,
                                              nullptr,
                                              nullptr);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    size_t width;
    size_t height;

    error |= clGetImageInfo((cl_mem)(pImg2D->ocl_image[0]),
                            CL_IMAGE_WIDTH,
                            sizeof(size_t),
                            &width,
                            NULL);
    error |= clGetImageInfo((cl_mem)(pImg2D->ocl_image[0]),
                            CL_IMAGE_HEIGHT,
                            sizeof(size_t),
                            &height,
                            NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    // if width/height is different from original, then update info and reallocate cpu frame
    if (width != m_pframeInfoExported->width || height != m_pframeInfoExported->height) {
        m_pframeInfoExported->width  = (mfxU16)width;
        m_pframeInfoExported->height = (mfxU16)height;

        mfxStatus sts = AllocCpuFrameForExport(m_pframeInfoExported);
        if (sts != MFX_ERR_NONE)
            return sts;
    }

    size_t origin[3] = { 0, 0, 0 };
    size_t region[3] = { width, height, 1 };

    cl_event readEvent;
    error = clEnqueueReadImage((cl_command_queue)pImg2D->ocl_command_queue,
                               (cl_mem)(pImg2D->ocl_image[0]),
                               CL_FALSE,
                               origin,
                               region,
                               0,
                               0,
                               m_cpuFrame.BGRA,
                               0,
                               NULL,
                               &readEvent);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    clWaitForEvents(1, &readEvent);
    clReleaseEvent(readEvent);

    error |= m_pfn_clEnqueueReleaseD3D11ObjectsKHR((cl_command_queue)pImg2D->ocl_command_queue,
                                                   pImg2D->ocl_image_num,
                                                   d3d11Objects,
                                                   0,
                                                   nullptr,
                                                   nullptr);

    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    size_t pitch = width * 4;
    for (int i = 0; i < height; i++)
        f.write((char *)(m_cpuFrame.BGRA + i * pitch), pitch);

    return MFX_ERR_NONE;
}

cl_int OpenCLCtxD3D11::InitPlatform() {
    cl_int error = CL_SUCCESS;

    // Determine the number of installed OpenCL platforms
    cl_uint num_platforms = 0;
    error                 = clGetPlatformIDs(0, NULL, &num_platforms);
    if (error)
        return error;

    // Get all of the handles to the installed OpenCL platforms
    std::vector<cl_platform_id> platforms(num_platforms);
    error = clGetPlatformIDs(num_platforms, &platforms[0], &num_platforms);
    if (error)
        return error;

    for (const auto &platform : platforms) {
        cl_uint num_devices = 0;

        error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
        if (error)
            continue;

        std::vector<cl_device_id> devices(num_devices);

        error =
            clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, (cl_uint)devices.size(), &devices[0], 0);
        if (error)
            continue;

        for (const auto &device : devices) {
            cl_uint deviceVendorId = 0;

            error = clGetDeviceInfo(device,
                                    CL_DEVICE_VENDOR_ID,
                                    sizeof(deviceVendorId),
                                    &deviceVendorId,
                                    NULL);

            if (error)
                continue;

            // Skip non-Intel devices
            if (deviceVendorId != 0x8086) {
                continue;
            }

            size_t num_extensions = 0;

            error = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 0, NULL, &num_extensions);
            if (error)
                continue;

            std::vector<char> extensions(num_extensions);

            error = clGetDeviceInfo(device,
                                    CL_DEVICE_EXTENSIONS,
                                    extensions.size(),
                                    extensions.data(),
                                    0);
            if (error)
                continue;

            bool isDeviceAppropriate = true;
            isDeviceAppropriate      = isDeviceAppropriate &&
                                  (std::strstr(extensions.data(), "cl_khr_d3d11_sharing") != NULL);

            if (isDeviceAppropriate) {
                m_clplatform = platform;
                return CL_SUCCESS;
            }
        }
    }

    if (0 == m_clplatform)
        return CL_INVALID_PLATFORM;

    return error;
}

    #define INIT_CL_EXT_FUNC(platform_id, func_name) \
        (m_pfn_##func_name =                         \
             (func_name##_fn)clGetExtensionFunctionAddressForPlatform(platform_id, #func_name))

cl_int OpenCLCtxD3D11::InitSurfaceSharingExtension() {
    if (!INIT_CL_EXT_FUNC(m_clplatform, clGetDeviceIDsFromD3D11KHR))
        return CL_INVALID_PLATFORM;

    if (!INIT_CL_EXT_FUNC(m_clplatform, clCreateFromD3D11Texture2DKHR))
        return CL_INVALID_PLATFORM;

    if (!INIT_CL_EXT_FUNC(m_clplatform, clEnqueueAcquireD3D11ObjectsKHR))
        return CL_INVALID_PLATFORM;

    if (!INIT_CL_EXT_FUNC(m_clplatform, clEnqueueReleaseD3D11ObjectsKHR))
        return CL_INVALID_PLATFORM;

    return CL_SUCCESS;
}

cl_int OpenCLCtxD3D11::InitDevice() {
    cl_int error = CL_SUCCESS;

    cl_uint numDevices = 0;
    error              = m_pfn_clGetDeviceIDsFromD3D11KHR(m_clplatform,
                                             CL_D3D11_DEVICE_KHR,
                                             m_pDevice,
                                             CL_PREFERRED_DEVICES_FOR_D3D11_KHR,
                                             1,
                                             &m_cldevice,
                                             &numDevices);

    if (error)
        return error;

    const cl_context_properties props[] = { CL_CONTEXT_D3D11_DEVICE_KHR,
                                            (cl_context_properties)m_pDevice,
                                            CL_CONTEXT_INTEROP_USER_SYNC,
                                            CL_FALSE,
                                            NULL };
    m_clcontext = clCreateContext(props, 1, &m_cldevice, NULL, NULL, &error);
    if (error)
        return error;

    return error;
}

mfxStatus OpenCLCtxD3D11::InitKernels(std::string oclFileName) {
    mfxStatus sts = MFX_ERR_NONE;
    std::string oclProgStr;

    // read program from file
    int err = readFile(oclFileName, oclProgStr);
    if (err)
        return MFX_ERR_ABORTED;

    // AddKernel()
    m_program_source    = std::string(oclProgStr.c_str());
    m_kernelY_FuncName  = std::string("rotate_Y");
    m_kernelUV_FuncName = std::string("rotate_UV");

    cl_int error = BuildKernels();
    if (error)
        return MFX_ERR_DEVICE_FAILED;

    // Work sizes for Y plane
    m_GlobalWorkSizeY[0] = m_pframeInfo->width;
    m_GlobalWorkSizeY[1] = m_pframeInfo->height;
    m_LocalWorkSizeY[0]  = chooseLocalSize(m_GlobalWorkSizeY[0], 8);
    m_LocalWorkSizeY[1]  = chooseLocalSize(m_GlobalWorkSizeY[1], 8);
    m_GlobalWorkSizeY[0] = m_LocalWorkSizeY[0] * (m_GlobalWorkSizeY[0] / m_LocalWorkSizeY[0]);
    m_GlobalWorkSizeY[1] = m_LocalWorkSizeY[1] * (m_GlobalWorkSizeY[1] / m_LocalWorkSizeY[1]);

    // Work size for UV plane
    m_GlobalWorkSizeUV[0] = m_pframeInfo->width / 2;
    m_GlobalWorkSizeUV[1] = m_pframeInfo->height / 2;
    m_LocalWorkSizeUV[0]  = chooseLocalSize(m_GlobalWorkSizeUV[0], 8);
    m_LocalWorkSizeUV[1]  = chooseLocalSize(m_GlobalWorkSizeUV[1], 8);
    m_GlobalWorkSizeUV[0] = m_LocalWorkSizeUV[0] * (m_GlobalWorkSizeUV[0] / m_LocalWorkSizeUV[0]);
    m_GlobalWorkSizeUV[1] = m_LocalWorkSizeUV[1] * (m_GlobalWorkSizeUV[1] / m_LocalWorkSizeUV[1]);

    return MFX_ERR_NONE;
}

cl_int OpenCLCtxD3D11::BuildKernels() {
    cl_int error = CL_SUCCESS;

    char buildOptions[] = "-I. -Werror -cl-fast-relaxed-math";

    // Create a program object from the source file
    const char *program_source_buf = m_program_source.c_str();

    m_clprogram = clCreateProgramWithSource(m_clcontext, 1, &program_source_buf, NULL, &error);
    if (error)
        return error;

    // Build OCL kernel
    error = clBuildProgram(m_clprogram, 1, &m_cldevice, buildOptions, NULL, NULL);
    if (error == CL_BUILD_PROGRAM_FAILURE) {
        size_t buildLogSize = 0;
        cl_int logStatus    = clGetProgramBuildInfo(m_clprogram,
                                                 m_cldevice,
                                                 CL_PROGRAM_BUILD_LOG,
                                                 0,
                                                 NULL,
                                                 &buildLogSize);
        std::vector<char> buildLog(buildLogSize + 1);
        logStatus = clGetProgramBuildInfo(m_clprogram,
                                          m_cldevice,
                                          CL_PROGRAM_BUILD_LOG,
                                          buildLogSize,
                                          &buildLog[0],
                                          NULL);
        return error;
    }
    else if (error)
        return error;

    // Create the kernel objects
    m_clkernelY = clCreateKernel(m_clprogram, m_kernelY_FuncName.c_str(), &error);
    if (error)
        return error;

    m_clkernelUV = clCreateKernel(m_clprogram, m_kernelUV_FuncName.c_str(), &error);
    if (error)
        return error;

    return error;
}

mfxStatus OpenCLCtxD3D11::OpenCLInit(mfxHandleType handleType, mfxHDL handle) {
    if (handleType != MFX_HANDLE_D3D11_DEVICE)
        return MFX_ERR_INVALID_HANDLE;

    if (!handle)
        return MFX_ERR_NULL_PTR;

    ID3D11Device *device = reinterpret_cast<ID3D11Device *>(handle);
    cl_int error         = CL_SUCCESS;

    if (!device)
        return MFX_ERR_DEVICE_FAILED;
    m_pDevice = device;

    error = InitPlatform();
    if (error)
        return MFX_ERR_DEVICE_FAILED;

    error = InitSurfaceSharingExtension();
    if (error)
        return MFX_ERR_DEVICE_FAILED;

    error = InitDevice();
    if (error)
        return MFX_ERR_DEVICE_FAILED;

    // Create a command queue
    m_clqueue = clCreateCommandQueueWithProperties(m_clcontext, m_cldevice, 0, &error);
    if (error)
        return MFX_ERR_DEVICE_FAILED;

    return MFX_ERR_NONE;
}

size_t OpenCLCtxD3D11::chooseLocalSize(size_t globalSize, size_t preferred) {
    size_t ret = 1;
    while ((globalSize % ret == 0) && ret <= preferred) {
        ret <<= 1;
    }

    return ret >> 1;
}

bool OpenCLCtxD3D11::EnqueueAcquireSurfaces(cl_mem *surfaces, int nSurfaces) {
    cl_int error =
        m_pfn_clEnqueueAcquireD3D11ObjectsKHR(m_clqueue, nSurfaces, surfaces, 0, NULL, NULL);
    if (error)
        return error;

    return true;
}

bool OpenCLCtxD3D11::EnqueueReleaseSurfaces(cl_mem *surfaces, int nSurfaces) {
    cl_int error =
        m_pfn_clEnqueueReleaseD3D11ObjectsKHR(m_clqueue, nSurfaces, surfaces, 0, NULL, NULL);
    if (error)
        return error;

    return true;
}

cl_int OpenCLCtxD3D11::ProcessSurface(cl_kernel clkernelY,
                                      cl_kernel clkernelUV,
                                      cl_mem clBufInY,
                                      cl_mem clBufInUV,
                                      cl_mem clBufOutY,
                                      cl_mem clBufOutUV) {
    cl_int error = CL_SUCCESS;

    // set kernelY parameters
    error = clSetKernelArg(clkernelY, 0, sizeof(cl_mem), &clBufInY);
    if (error)
        return error;

    error = clSetKernelArg(clkernelY, 1, sizeof(cl_mem), &clBufOutY);
    if (error)
        return error;

    // set kernelUV parameters
    error = clSetKernelArg(clkernelUV, 0, sizeof(cl_mem), &clBufInUV);
    if (error)
        return error;

    error = clSetKernelArg(clkernelUV, 1, sizeof(cl_mem), &clBufOutUV);
    if (error)
        return error;

    // enqueue kernels
    error = clEnqueueNDRangeKernel(m_clqueue,
                                   m_clkernelY,
                                   2,
                                   NULL,
                                   m_GlobalWorkSizeY,
                                   m_LocalWorkSizeY,
                                   0,
                                   NULL,
                                   NULL);
    if (error)
        return error;

    error = clEnqueueNDRangeKernel(m_clqueue,
                                   m_clkernelUV,
                                   2,
                                   NULL,
                                   m_GlobalWorkSizeUV,
                                   m_LocalWorkSizeUV,
                                   0,
                                   NULL,
                                   NULL);
    if (error)
        return error;

    // flush & finish the command queue
    error = clFlush(m_clqueue);
    if (error)
        return error;

    error = clFinish(m_clqueue);
    if (error)
        return error;
    return error;
}

mfxStatus OpenCLCtxD3D11::OpenCLProcessSurface(cl_mem clBufInY,
                                               cl_mem clBufInUV,
                                               cl_mem clBufOutY,
                                               cl_mem clBufOutUV) {
    cl_int error = CL_SUCCESS;

    error = ProcessSurface(m_clkernelY, m_clkernelUV, clBufInY, clBufInUV, clBufOutY, clBufOutUV);
    if (error)
        return MFX_ERR_DEVICE_FAILED;

    return MFX_ERR_NONE;
}

void OpenCLCtxD3D11::SetOCLSurfaceFrameInfo(FrameInfo *frameInfo) {
    m_pframeInfo = frameInfo;
    return;
}

mfxStatus OpenCLCtxD3D11::AllocCpuFrameForExport(FrameInfo *frameInfo) {
    m_pframeInfoExported = frameInfo;

    if (frameInfo->fourcc == MFX_FOURCC_NV12) {
        if (m_cpuFrame.Y) {
            free(m_cpuFrame.Y);
        }
        m_cpuFrame.Y = (unsigned char *)malloc(frameInfo->width * frameInfo->height);
        if (!m_cpuFrame.Y)
            return MFX_ERR_NULL_PTR;
        if (m_cpuFrame.UV) {
            free(m_cpuFrame.UV);
        }
        m_cpuFrame.UV = (unsigned char *)malloc(frameInfo->width / 2 * frameInfo->height / 2 * 2);
        if (!m_cpuFrame.UV)
            return MFX_ERR_NULL_PTR;
    }
    else if (frameInfo->fourcc == MFX_FOURCC_RGB4) {
        if (m_cpuFrame.BGRA) {
            free(m_cpuFrame.BGRA);
        }
        m_cpuFrame.BGRA = (unsigned char *)malloc(frameInfo->width * frameInfo->height * 4);
        if (!m_cpuFrame.BGRA)
            return MFX_ERR_NULL_PTR;
    }
    return MFX_ERR_NONE;
}

mfxStatus OpenCLCtxD3D11::CopySurfaceSystemToOCL(CPUFrame *cpuFrame, cl_mem *surfaces) {
    cl_int error = CL_SUCCESS;

    size_t region[3]     = { 0, 0, 1 };
    size_t src_origin[3] = { 0, 0, 0 };

    // copy Y (nv12), or BGRA (rgb4) data from CPU into OCL surface
    error = CL_SUCCESS;
    error |=
        clGetImageInfo((cl_mem)(surfaces[0]), CL_IMAGE_WIDTH, sizeof(size_t), &region[0], NULL);
    error |=
        clGetImageInfo((cl_mem)(surfaces[0]), CL_IMAGE_HEIGHT, sizeof(size_t), &region[1], NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    error = clEnqueueWriteImage(
        m_clqueue,
        (cl_mem)(surfaces[0]),
        CL_TRUE,
        src_origin,
        region,
        0,
        0,
        (m_pframeInfo->fourcc == MFX_FOURCC_NV12) ? cpuFrame->Y : cpuFrame->BGRA,
        0,
        NULL,
        NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    if (m_pframeInfo->fourcc == MFX_FOURCC_NV12) {
        // copy UV data from CPU into OCL surface
        error = CL_SUCCESS;
        error |=
            clGetImageInfo((cl_mem)(surfaces[1]), CL_IMAGE_WIDTH, sizeof(size_t), &region[0], NULL);
        error |= clGetImageInfo((cl_mem)(surfaces[1]),
                                CL_IMAGE_HEIGHT,
                                sizeof(size_t),
                                &region[1],
                                NULL);
        if (error != CL_SUCCESS)
            return MFX_ERR_DEVICE_FAILED;

        error = clEnqueueWriteImage(m_clqueue,
                                    (cl_mem)(surfaces[1]),
                                    CL_TRUE,
                                    src_origin,
                                    region,
                                    0,
                                    0,
                                    cpuFrame->UV,
                                    0,
                                    NULL,
                                    NULL);
        if (error != CL_SUCCESS)
            return MFX_ERR_DEVICE_FAILED;
    }

    return MFX_ERR_NONE;
}

mfxStatus OpenCLCtxD3D11::CopySurfaceOCLToSystem(cl_mem *surfaces, CPUFrame *cpuFrame) {
    cl_int error = CL_SUCCESS;

    // create shared OpenCL 2D image from D3D11 2D texture
    size_t region[3]     = { 0, 0, 1 };
    size_t src_origin[3] = { 0, 0, 0 };

    // copy Y (nv12), or BGRA (rgb4) data from CPU into OCL surface
    error = CL_SUCCESS;
    error |=
        clGetImageInfo((cl_mem)(surfaces[0]), CL_IMAGE_WIDTH, sizeof(size_t), &region[0], NULL);
    error |=
        clGetImageInfo((cl_mem)(surfaces[0]), CL_IMAGE_HEIGHT, sizeof(size_t), &region[1], NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    // set blocking_read to true
    error =
        clEnqueueReadImage(m_clqueue,
                           (cl_mem)(surfaces[0]),
                           CL_TRUE,
                           src_origin,
                           region,
                           0,
                           0,
                           (m_pframeInfo->fourcc == MFX_FOURCC_NV12) ? cpuFrame->Y : cpuFrame->BGRA,
                           0,
                           NULL,
                           NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    if (m_pframeInfo->fourcc == MFX_FOURCC_NV12) {
        // copy UV data from CPU into OCL surface
        error = CL_SUCCESS;
        error |=
            clGetImageInfo((cl_mem)(surfaces[1]), CL_IMAGE_WIDTH, sizeof(size_t), &region[0], NULL);
        error |= clGetImageInfo((cl_mem)(surfaces[1]),
                                CL_IMAGE_HEIGHT,
                                sizeof(size_t),
                                &region[1],
                                NULL);
        if (error != CL_SUCCESS)
            return MFX_ERR_DEVICE_FAILED;

        // set blocking_read to true
        error = clEnqueueReadImage(m_clqueue,
                                   (cl_mem)(surfaces[1]),
                                   CL_TRUE,
                                   src_origin,
                                   region,
                                   0,
                                   0,
                                   cpuFrame->UV,
                                   0,
                                   NULL,
                                   NULL);
        if (error != CL_SUCCESS)
            return MFX_ERR_DEVICE_FAILED;
    }

    return MFX_ERR_NONE;
}

// call ReleaseOCLSurface() when finished with this surface for proper refcounting (and in the case of mapped surfaces, to release from exclusive OCL access)
mfxStatus OpenCLCtxD3D11::GetOCLInputSurface(cl_mem *surfaces,
                                             bool bSharedD3D11,
                                             ID3D11Texture2D *pSurfIn) {
    mfxStatus sts = MFX_ERR_NONE;
    int nSurfaces = 0;

    if (m_pframeInfo->fourcc == MFX_FOURCC_NV12) {
        nSurfaces = 2;

        if (!m_clBufWritableY[0] || !m_clBufWritableUV[0]) {
            cl_mem nv12_buffers[2] = {};
            nv12_buffers[0]        = m_clBufWritableY[0];
            nv12_buffers[1]        = m_clBufWritableUV[0];

            sts = AllocateOCLSurface(nv12_buffers, bSharedD3D11, pSurfIn);
            if (sts != MFX_ERR_NONE)
                return sts;

            m_clBufWritableY[0]  = nv12_buffers[0];
            m_clBufWritableUV[0] = nv12_buffers[1];
        }

        surfaces[0] = m_clBufWritableY[0];
        surfaces[1] = m_clBufWritableUV[0];
    }
    else if (m_pframeInfo->fourcc == MFX_FOURCC_RGB4) {
        nSurfaces = 1;

        if (!m_clBufWritableBGRA[0]) {
            sts = AllocateOCLSurface(&m_clBufWritableBGRA[0], bSharedD3D11, pSurfIn);
            if (sts != MFX_ERR_NONE)
                return sts;
        }

        surfaces[0] = m_clBufWritableBGRA[0];
    }
    else
        return MFX_ERR_UNSUPPORTED;

    sts = AddRefOCLSurface(surfaces, nSurfaces, bSharedD3D11);
    if (sts != MFX_ERR_NONE)
        return sts;

    return MFX_ERR_NONE;
}

// call ReleaseOCLSurface() when finished with this surface for proper refcounting (and in the case of mapped surfaces, to release from exclusive OCL access)
mfxStatus OpenCLCtxD3D11::GetOCLOutputSurface(cl_mem *surfaces,
                                              bool bSharedD3D11,
                                              ID3D11Texture2D *pSurfIn) {
    mfxStatus sts = MFX_ERR_NONE;
    int nSurfaces = 0;

    if (m_pframeInfo->fourcc == MFX_FOURCC_NV12) {
        nSurfaces = 2;

        if (!m_clBufWritableY[1] || !m_clBufWritableUV[1]) {
            cl_mem nv12_buffers[2] = {};
            nv12_buffers[0]        = m_clBufWritableY[1];
            nv12_buffers[1]        = m_clBufWritableUV[1];

            sts = AllocateOCLSurface(nv12_buffers, bSharedD3D11, pSurfIn);
            if (sts != MFX_ERR_NONE)
                return sts;

            m_clBufWritableY[1]  = nv12_buffers[0];
            m_clBufWritableUV[1] = nv12_buffers[1];
        }

        surfaces[0] = m_clBufWritableY[1];
        surfaces[1] = m_clBufWritableUV[1];
    }
    else if (m_pframeInfo->fourcc == MFX_FOURCC_RGB4) {
        nSurfaces = 1;

        if (!m_clBufWritableBGRA[1]) {
            sts = AllocateOCLSurface(&m_clBufWritableBGRA[1], bSharedD3D11, pSurfIn);
            if (sts != MFX_ERR_NONE)
                return sts;
        }

        surfaces[0] = m_clBufWritableBGRA[1];
    }
    else
        return MFX_ERR_UNSUPPORTED;

    sts = AddRefOCLSurface(surfaces, nSurfaces, bSharedD3D11);
    if (sts != MFX_ERR_NONE)
        return sts;

    return MFX_ERR_NONE;
}

// Request an OCL surface for writing
// - if bSharedD3D11 is false (default), a new OCL surface is allocated
// - if bSharedD3D11 is true, then pSurfIn is mapped to an OCL surface instead (no new surface is allocated)
//
// This function does NOT update surface refcounts or lock surfaces. It is the caller's responsibility to call either
//   clEnqueueAcquireD3D11ObjectsKHR (for mapped surfaces) or clRetainMemObject (for native OCL surfaces)
//
// According to OCL spec, the OCL surface created from a shared D3D11 resource remains valid as long as the actual D3D11 resource
//   is not deleted, so we don't adjust the refcount of the cl_mem itself on each subsequent access. However, we still need to call
//   clReleaseMemObject() during teardown to avoid resource leak. (Determined via testing, docs are not very clear on this though).
mfxStatus OpenCLCtxD3D11::AllocateOCLSurface(cl_mem *surfaces,
                                             bool bSharedD3D11,
                                             ID3D11Texture2D *pSurfIn) {
    cl_int error = CL_SUCCESS;

    // Y or BGRA
    cl_channel_order clChannelOrder = (m_pframeInfo->fourcc == MFX_FOURCC_NV12) ? CL_R : CL_BGRA;

    surfaces[0]      = nullptr;
    cl_mem alloc_mem = nullptr;

    if (bSharedD3D11) {
        // create mapped surface using D3D11 input
        alloc_mem =
            m_pfn_clCreateFromD3D11Texture2DKHR(m_clcontext, CL_MEM_READ_WRITE, pSurfIn, 0, &error);
        if (error || (alloc_mem == nullptr))
            return MFX_ERR_DEVICE_FAILED;
    }
    else {
        // allocate a new OCL surface
        cl_image_format src_format   = {};
        cl_image_desc src_image_desc = {};

        src_format.image_channel_order     = clChannelOrder;
        src_format.image_channel_data_type = CL_UNORM_INT8;

        src_image_desc.image_width  = m_pframeInfo->width;
        src_image_desc.image_height = m_pframeInfo->height;
        src_image_desc.image_type   = CL_MEM_OBJECT_IMAGE2D;

        alloc_mem = clCreateImage(m_clcontext,
                                  CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                                  &src_format,
                                  &src_image_desc,
                                  0,
                                  &error);
        if (error || (alloc_mem == nullptr))
            return MFX_ERR_DEVICE_FAILED;
    }

    surfaces[0] = alloc_mem;

    // UV
    if (m_pframeInfo->fourcc == MFX_FOURCC_NV12) {
        clChannelOrder = CL_RG;

        surfaces[1]        = nullptr;
        cl_mem alloc_memUV = nullptr;

        if (bSharedD3D11) {
            // create mapped surface using D3D11 input
            alloc_memUV = m_pfn_clCreateFromD3D11Texture2DKHR(m_clcontext,
                                                              CL_MEM_READ_WRITE,
                                                              pSurfIn,
                                                              1,
                                                              &error);
            if (error || (alloc_memUV == nullptr))
                return MFX_ERR_DEVICE_FAILED;
        }
        else {
            // allocate a new OCL surface
            cl_image_format src_format   = {};
            cl_image_desc src_image_desc = {};

            src_format.image_channel_order     = clChannelOrder;
            src_format.image_channel_data_type = CL_UNORM_INT8;

            src_image_desc.image_width  = m_pframeInfo->width / 2;
            src_image_desc.image_height = m_pframeInfo->height / 2;
            src_image_desc.image_type   = CL_MEM_OBJECT_IMAGE2D;

            alloc_memUV = clCreateImage(m_clcontext,
                                        CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                                        &src_format,
                                        &src_image_desc,
                                        0,
                                        &error);
            if (error || (alloc_memUV == nullptr))
                return MFX_ERR_DEVICE_FAILED;
        }

        surfaces[1] = alloc_memUV;
    }

    return MFX_ERR_NONE;
}

// increase refcount (or sharing lock) for OCL surface (internal only, not to be called directly by application)
mfxStatus OpenCLCtxD3D11::AddRefOCLSurface(cl_mem *surfaces, int nSurfaces, bool bSharedD3D11) {
    cl_int error = CL_SUCCESS;

    if (bSharedD3D11) {
        // Lock the surfaces for exclusive OCL access. When finished, call EnqueueReleaseSurfaces()
        if (!EnqueueAcquireSurfaces(surfaces, nSurfaces))
            return MFX_ERR_DEVICE_FAILED;
    }
    else {
        // increase ref count so that this surface is not destroyed while the application is using it
        for (int i = 0; i < nSurfaces; i++)
            error |= clRetainMemObject(surfaces[i]);

        if (error)
            return MFX_ERR_DEVICE_FAILED;
    }

    return MFX_ERR_NONE;
}

mfxStatus OpenCLCtxD3D11::ReleaseOCLSurface(cl_mem *surfaces, bool bSharedD3D11) {
    int nSurfaces = (m_pframeInfo->fourcc == MFX_FOURCC_NV12) ? 2 : 1;
    if (bSharedD3D11) {
        // Release shared surfaces from exclusive OCL access.
        if (!EnqueueReleaseSurfaces(surfaces, nSurfaces))
            return MFX_ERR_DEVICE_FAILED;
    }
    else {
        cl_int error = CL_SUCCESS;
        for (int i = 0; i < nSurfaces; i++) {
            error |= clReleaseMemObject(surfaces[i]);
        }

        if (error)
            return MFX_ERR_DEVICE_FAILED;

        // For debugging - get refcount
        // cl_uint refCount = 0xF0;
        // clGetMemObjectInfo(memY, CL_MEM_REFERENCE_COUNT, sizeof(cl_uint), &refCount, NULL);
        // printf("*** refcount memY = %d\n", refCount);
    }

    return MFX_ERR_NONE;
}

    #if defined _DEBUG
void OpenCLCtxD3D11::DebugDumpSurfaceRefcount() {
    cl_uint refCount = 0xFEFE;
    cl_int err       = 0;

    printf(
        "DEBUG: refCount of allocated OpenCL surfaces in dtor (expect refCount=1 for allocated surfaces):\n");

    printf("  m_clBufWritableY[0]  ... ");
    if (m_clBufWritableY[0]) {
        err = clGetMemObjectInfo(m_clBufWritableY[0],
                                 CL_MEM_REFERENCE_COUNT,
                                 sizeof(cl_uint),
                                 &refCount,
                                 NULL);
        printf("error = %d, refCount = %d\n", err, refCount);
    }
    else {
        printf("not allocated\n");
    }

    printf("  m_clBufWritableUV[0] ... ");
    if (m_clBufWritableY[0]) {
        err = clGetMemObjectInfo(m_clBufWritableUV[0],
                                 CL_MEM_REFERENCE_COUNT,
                                 sizeof(cl_uint),
                                 &refCount,
                                 NULL);
        printf("error = %d, refCount = %d\n", err, refCount);
    }
    else {
        printf("not allocated\n");
    }

    printf("  m_clBufWritableY[1]  ... ");
    if (m_clBufWritableY[1]) {
        err = clGetMemObjectInfo(m_clBufWritableY[1],
                                 CL_MEM_REFERENCE_COUNT,
                                 sizeof(cl_uint),
                                 &refCount,
                                 NULL);
        printf("error = %d, refCount = %d\n", err, refCount);
    }
    else {
        printf("not allocated\n");
    }

    printf("  m_clBufWritableUV[1] ... ");
    if (m_clBufWritableY[1]) {
        err = clGetMemObjectInfo(m_clBufWritableUV[1],
                                 CL_MEM_REFERENCE_COUNT,
                                 sizeof(cl_uint),
                                 &refCount,
                                 NULL);
        printf("error = %d, refCount = %d\n", err, refCount);
    }
    else {
        printf("not allocated\n");
    }
}
    #endif

#endif // TOOLS_ENABLE_OPENCL