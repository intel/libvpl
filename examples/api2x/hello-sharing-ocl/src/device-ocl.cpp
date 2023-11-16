//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "device-ocl.h"
#include <iostream>

// following is from MSDK samples (sample_plugin_opencl) - see opencl_filter_dx11.cpp

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

mfxStatus OpenCLCtxD3D11::OpenCLInit(mfxHandleType handleType,
                                     mfxHDL handle,
                                     mfxU32 width,
                                     mfxU32 height) {
    if (handleType != MFX_HANDLE_D3D11_DEVICE)
        return MFX_ERR_INVALID_HANDLE;

    if (!handle)
        return MFX_ERR_NULL_PTR;

    ID3D11Device *device = reinterpret_cast<ID3D11Device *>(handle);
    m_width              = width;
    m_height             = height;

    cl_int error = CL_SUCCESS;

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

mfxStatus OpenCLCtxD3D11::CopySurfaceSystemToOCL(CPUFrameInfo_NV12 *cpuFrameInfo,
                                                 cl_mem memY,
                                                 cl_mem memUV) {
    cl_int error = CL_SUCCESS;

    size_t region[3]     = { 0, 0, 1 };
    size_t src_origin[3] = { 0, 0, 0 };

    // copy Y data from CPU into OCL surface
    error = CL_SUCCESS;
    error |= clGetImageInfo((cl_mem)(memY), CL_IMAGE_WIDTH, sizeof(size_t), &region[0], NULL);
    error |= clGetImageInfo((cl_mem)(memY), CL_IMAGE_HEIGHT, sizeof(size_t), &region[1], NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    error = clEnqueueWriteImage(m_clqueue,
                                (cl_mem)(memY),
                                CL_TRUE,
                                src_origin,
                                region,
                                0,
                                0,
                                cpuFrameInfo->Y,
                                0,
                                NULL,
                                NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    // copy UV data from CPU into OCL surface
    error = CL_SUCCESS;
    error |= clGetImageInfo((cl_mem)(memUV), CL_IMAGE_WIDTH, sizeof(size_t), &region[0], NULL);
    error |= clGetImageInfo((cl_mem)(memUV), CL_IMAGE_HEIGHT, sizeof(size_t), &region[1], NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    error = clEnqueueWriteImage(m_clqueue,
                                (cl_mem)(memUV),
                                CL_TRUE,
                                src_origin,
                                region,
                                0,
                                0,
                                cpuFrameInfo->UV,
                                0,
                                NULL,
                                NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    return MFX_ERR_NONE;
}

mfxStatus OpenCLCtxD3D11::CopySurfaceOCLToSystem(cl_mem memY,
                                                 cl_mem memUV,
                                                 CPUFrameInfo_NV12 *cpuFrameInfo) {
    cl_int error = CL_SUCCESS;

    // create shared OpenCL 2D image from D3D11 2D texture
    size_t region[3]     = { 0, 0, 1 };
    size_t src_origin[3] = { 0, 0, 0 };

    // copy Y data from CPU into OCL surface
    error = CL_SUCCESS;
    error |= clGetImageInfo((cl_mem)(memY), CL_IMAGE_WIDTH, sizeof(size_t), &region[0], NULL);
    error |= clGetImageInfo((cl_mem)(memY), CL_IMAGE_HEIGHT, sizeof(size_t), &region[1], NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    // set blocking_read to true
    error = clEnqueueReadImage(m_clqueue,
                               (cl_mem)(memY),
                               CL_TRUE,
                               src_origin,
                               region,
                               0,
                               0,
                               cpuFrameInfo->Y,
                               0,
                               NULL,
                               NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    // copy UV data from CPU into OCL surface
    error = CL_SUCCESS;
    error |= clGetImageInfo((cl_mem)(memUV), CL_IMAGE_WIDTH, sizeof(size_t), &region[0], NULL);
    error |= clGetImageInfo((cl_mem)(memUV), CL_IMAGE_HEIGHT, sizeof(size_t), &region[1], NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    // set blocking_read to true
    error = clEnqueueReadImage(m_clqueue,
                               (cl_mem)(memUV),
                               CL_TRUE,
                               src_origin,
                               region,
                               0,
                               0,
                               cpuFrameInfo->UV,
                               0,
                               NULL,
                               NULL);
    if (error != CL_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    return MFX_ERR_NONE;
}

// call ReleaseOCLSurface() when finished with this surface for proper refcounting (and in the case of mapped surfaces, to release from exclusive OCL access)
mfxStatus OpenCLCtxD3D11::GetOCLInputSurface(cl_mem *memY,
                                             cl_mem *memUV,
                                             bool bSharedD3D11,
                                             ID3D11Texture2D *pSurfIn,
                                             mfxU32 width,
                                             mfxU32 height) {
    mfxStatus sts = MFX_ERR_NONE;

    if (!m_clBufWritableY[0] || !m_clBufWritableUV[0]) {
        sts = AllocateOCLSurface(&m_clBufWritableY[0],
                                 &m_clBufWritableUV[0],
                                 bSharedD3D11,
                                 pSurfIn,
                                 width,
                                 height);
        if (sts != MFX_ERR_NONE)
            return sts;
    }

    *memY  = m_clBufWritableY[0];
    *memUV = m_clBufWritableUV[0];

    sts = AddRefOCLSurface(*memY, *memUV, bSharedD3D11);
    if (sts != MFX_ERR_NONE)
        return sts;

    return MFX_ERR_NONE;
}

// call ReleaseOCLSurface() when finished with this surface for proper refcounting (and in the case of mapped surfaces, to release from exclusive OCL access)
mfxStatus OpenCLCtxD3D11::GetOCLOutputSurface(cl_mem *memY,
                                              cl_mem *memUV,
                                              bool bSharedD3D11,
                                              ID3D11Texture2D *pSurfIn,
                                              mfxU32 width,
                                              mfxU32 height) {
    mfxStatus sts = MFX_ERR_NONE;

    if (!m_clBufWritableY[1] || !m_clBufWritableUV[1]) {
        sts = AllocateOCLSurface(&m_clBufWritableY[1],
                                 &m_clBufWritableUV[1],
                                 bSharedD3D11,
                                 pSurfIn,
                                 width,
                                 height);
        if (sts != MFX_ERR_NONE)
            return sts;
    }

    *memY  = m_clBufWritableY[1];
    *memUV = m_clBufWritableUV[1];

    sts = AddRefOCLSurface(*memY, *memUV, bSharedD3D11);
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
mfxStatus OpenCLCtxD3D11::AllocateOCLSurface(cl_mem *memY,
                                             cl_mem *memUV,
                                             bool bSharedD3D11,
                                             ID3D11Texture2D *pSurfIn,
                                             mfxU32 width,
                                             mfxU32 height) {
    cl_int error = CL_SUCCESS;

    *memY  = nullptr;
    *memUV = nullptr;

    cl_mem alloc_memY  = nullptr;
    cl_mem alloc_memUV = nullptr;

    if (bSharedD3D11) {
        // create mapped surface using D3D11 input
        alloc_memY =
            m_pfn_clCreateFromD3D11Texture2DKHR(m_clcontext, CL_MEM_READ_WRITE, pSurfIn, 0, &error);
        if (error || (alloc_memY == nullptr))
            return MFX_ERR_DEVICE_FAILED;

        alloc_memUV =
            m_pfn_clCreateFromD3D11Texture2DKHR(m_clcontext, CL_MEM_READ_WRITE, pSurfIn, 1, &error);
        if (error || (alloc_memUV == nullptr))
            return MFX_ERR_DEVICE_FAILED;
    }
    else {
        // allocate a new OCL surface
        cl_image_format src_format   = {};
        cl_image_desc src_image_desc = {};

        if (width == 0)
            width = m_width;

        if (height == 0)
            height = m_height;

        // Y
        src_format.image_channel_order     = CL_R;
        src_format.image_channel_data_type = CL_UNORM_INT8;

        src_image_desc.image_width  = width;
        src_image_desc.image_height = height;
        src_image_desc.image_type   = CL_MEM_OBJECT_IMAGE2D;

        alloc_memY = clCreateImage(m_clcontext,
                                   CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                                   &src_format,
                                   &src_image_desc,
                                   0,
                                   &error);
        if (error || (alloc_memY == nullptr))
            return MFX_ERR_DEVICE_FAILED;

        // UV
        src_format.image_channel_order     = CL_RG;
        src_format.image_channel_data_type = CL_UNORM_INT8;
        src_image_desc.image_width /= 2;
        src_image_desc.image_height /= 2;

        alloc_memUV = clCreateImage(m_clcontext,
                                    CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                                    &src_format,
                                    &src_image_desc,
                                    0,
                                    &error);
        if (error || (alloc_memUV == nullptr))
            return MFX_ERR_DEVICE_FAILED;
    }

    *memY  = alloc_memY;
    *memUV = alloc_memUV;

    return MFX_ERR_NONE;
}

// increase refcount (or sharing lock) for OCL surface (internal only, not to be called directly by application)
mfxStatus OpenCLCtxD3D11::AddRefOCLSurface(cl_mem memY, cl_mem memUV, bool bSharedD3D11) {
    cl_int error = CL_SUCCESS;

    if (bSharedD3D11) {
        // Lock the surfaces for exclusive OCL access. When finished, call EnqueueReleaseSurfaces()
        cl_mem surfaces[2] = { memY, memUV };
        if (!EnqueueAcquireSurfaces(surfaces, sizeof(surfaces) / sizeof(cl_mem)))
            return MFX_ERR_DEVICE_FAILED;
    }
    else {
        // increase ref count so that this surface is not destroyed while the application is using it
        error = clRetainMemObject(memY);
        error |= clRetainMemObject(memUV);
        if (error)
            return MFX_ERR_DEVICE_FAILED;
    }

    return MFX_ERR_NONE;
}

mfxStatus OpenCLCtxD3D11::ReleaseOCLSurface(cl_mem memY, cl_mem memUV, bool bSharedD3D11) {
    if (bSharedD3D11) {
        // Release shared surfaces from exclusive OCL access.
        cl_mem surfaces[2] = { memY, memUV };
        if (!EnqueueReleaseSurfaces(surfaces, sizeof(surfaces) / sizeof(cl_mem)))
            return MFX_ERR_DEVICE_FAILED;
    }
    else {
        cl_int error = clReleaseMemObject(memY);
        error |= clReleaseMemObject(memUV);
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