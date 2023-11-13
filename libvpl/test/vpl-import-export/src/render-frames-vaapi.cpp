//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./render-frames.h"

mfxStatus RenderCtxVAAPI::RenderInit(DevCtx *devCtx, mfxU32 width, mfxU32 height, std::string infileName) {
#ifdef TOOLS_ENABLE_X11
    m_devCtx      = devCtx;
    m_pX11Display = m_devCtx->GetX11Display();
    if (!m_pX11Display)
        return MFX_ERR_DEVICE_FAILED;

    int x11ScreenNumber = DefaultScreen(m_pX11Display);
    if (x11ScreenNumber == -1)
        return MFX_ERR_DEVICE_FAILED;

    unsigned long x11BG     = BlackPixel(m_pX11Display, x11ScreenNumber); //NOLINT
    unsigned long x11Border = WhitePixel(m_pX11Display, x11ScreenNumber); //NOLINT

    Window x11RootWindow = DefaultRootWindow(m_pX11Display);
    m_x11Window          = XCreateSimpleWindow(m_pX11Display, x11RootWindow, 0, 0, width, height, 0, x11Border, x11BG);

    if (m_x11Window == None)
        return MFX_ERR_DEVICE_FAILED;

    XMapWindow(m_pX11Display, m_x11Window);
    XSync(m_pX11Display, False);

    printf("RenderCtxVAAPI::RenderInit() - success\n");

    return MFX_ERR_NONE;
#else
    printf("RenderCtxVAAPI::RenderInit() - ERROR - TOOLS_ENABLE_X11 not defined\n");
    return MFX_ERR_UNSUPPORTED;
#endif
}

mfxStatus RenderCtxVAAPI::RenderFrame(mfxHDL extSurface) {
#ifdef TOOLS_ENABLE_X11
    mfxSurfaceVAAPI *es = reinterpret_cast<mfxSurfaceVAAPI *>(extSurface);
    if (!es || es->SurfaceInterface.Header.SurfaceType != MFX_SURFACE_TYPE_VAAPI)
        return MFX_ERR_INVALID_HANDLE;

    VASurfaceID surfID = es->vaSurfaceID;

    VADisplay vaDisplay = m_devCtx->GetVADisplay();

    VAStatus vaRes;
    VAImage vaImage         = {};
    VABufferInfo bufferInfo = {};

    vaRes = vaDeriveImage(vaDisplay, surfID, &vaImage);
    if (vaRes != VA_STATUS_SUCCESS)
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    vaRes = vaAcquireBufferHandle(vaDisplay, vaImage.buf, &bufferInfo);
    if (vaRes != VA_STATUS_SUCCESS) {
        vaDestroyImage(vaDisplay, vaImage.image_id);
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }

    VASurfaceAttribExternalBuffers vaExtBuf = {};
    uintptr_t vaBufferHandles[1]            = {};
    VASurfaceAttrib attribs[2]              = {};

    vaExtBuf.pixel_format = vaImage.format.fourcc;
    vaExtBuf.width        = vaImage.width;
    vaExtBuf.height       = vaImage.height;
    vaExtBuf.data_size    = vaImage.data_size;
    vaExtBuf.num_planes   = vaImage.num_planes;

    for (int i = 0; i < (int)vaExtBuf.num_planes; ++i) {
        vaExtBuf.pitches[i] = vaImage.pitches[i];
        vaExtBuf.offsets[i] = vaImage.offsets[i];
    }

    vaExtBuf.buffers     = vaBufferHandles;
    vaExtBuf.buffers[0]  = bufferInfo.handle;
    vaExtBuf.num_buffers = 1;

    vaExtBuf.flags = VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;

    attribs[0].type          = (VASurfaceAttribType)VASurfaceAttribMemoryType;
    attribs[0].flags         = VA_SURFACE_ATTRIB_SETTABLE;
    attribs[0].value.type    = VAGenericValueTypeInteger;
    attribs[0].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;

    attribs[1].type          = (VASurfaceAttribType)VASurfaceAttribExternalBufferDescriptor;
    attribs[1].flags         = VA_SURFACE_ATTRIB_SETTABLE;
    attribs[1].value.type    = VAGenericValueTypePointer;
    attribs[1].value.value.p = &vaExtBuf;

    VASurfaceID renderVASurfaceID = {};

    vaRes = vaCreateSurfaces(vaDisplay, VA_RT_FORMAT_YUV420, vaExtBuf.width, vaExtBuf.height, &renderVASurfaceID, 1, attribs, 2);
    if (vaRes != VA_STATUS_SUCCESS) {
        printf("vaCreateSurfaces returned %d\n", vaRes);
        return MFX_ERR_DEVICE_FAILED;
    }

    vaRes = vaPutSurface(vaDisplay, renderVASurfaceID, m_x11Window, 0, 0, vaExtBuf.width, vaExtBuf.height, 0, 0, vaExtBuf.width, vaExtBuf.height, NULL, 0, VA_FRAME_PICTURE);
    if (vaRes != VA_STATUS_SUCCESS) {
        printf("vaPutSurface returned %d\n", vaRes);
        return MFX_ERR_DEVICE_FAILED;
    }

    vaReleaseBufferHandle(vaDisplay, vaImage.buf);
    vaDestroySurfaces(vaDisplay, &renderVASurfaceID, 1);

    return MFX_ERR_NONE;
#else
    return MFX_ERR_UNSUPPORTED;
#endif
}
