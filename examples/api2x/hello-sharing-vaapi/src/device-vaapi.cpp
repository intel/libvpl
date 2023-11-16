//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "device-vaapi.h"
#include <cstring>
#include <iostream>
#include <string>

mfxStatus DevCtxVAAPI::InitDevice(mfxU32 nAdapterNum, mfxHandleType *pHandleType, mfxHDL *pHandle) {
    std::string renderPath = "/dev/dri/renderD";
    renderPath += std::to_string(128 + nAdapterNum);

    m_vaDRMfd = open(renderPath.c_str(), O_RDWR);
    if (m_vaDRMfd < 0)
        return MFX_ERR_DEVICE_FAILED;

    m_vaDisplay = vaGetDisplayDRM(m_vaDRMfd);
    if (!m_vaDisplay)
        return MFX_ERR_DEVICE_FAILED;

    int va_ver_major = 0, va_ver_minor = 0;
    VAStatus vaSts = vaInitialize(m_vaDisplay, &va_ver_major, &va_ver_minor);
    if (vaSts != VA_STATUS_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    *pHandleType = MFX_HANDLE_VA_DISPLAY;
    *pHandle     = (mfxHDL)m_vaDisplay;

    return MFX_ERR_NONE;
}

VASurfaceID DevCtxVAAPI::CreateSurfaceToShare(mfxU16 width,
                                              mfxU16 height,
                                              mfxU8 *data,
                                              bool brender) {
    VAStatus vaSts          = VA_STATUS_SUCCESS;
    VASurfaceID vaSurfaceID = VA_INVALID_ID;
    VAImage vaImg           = {};
    VASurfaceAttrib attrib  = {};
    attrib.type             = VASurfaceAttribPixelFormat;
    attrib.flags            = VA_SURFACE_ATTRIB_SETTABLE;
    attrib.value.type       = VAGenericValueTypeInteger;
    attrib.value.value.i    = VA_FOURCC_NV12;

    vaSts = vaCreateSurfaces(m_vaDisplay,
                             VA_RT_FORMAT_YUV420,
                             width,
                             height,
                             &vaSurfaceID,
                             1,
                             &attrib,
                             1);
    if (vaSts != VA_STATUS_SUCCESS)
        return VA_INVALID_ID;

    vaSts = vaDeriveImage(m_vaDisplay, vaSurfaceID, &vaImg);
    if (vaSts != VA_STATUS_SUCCESS)
        return VA_INVALID_ID;

    mfxU8 *p_buffer = nullptr;
    vaSts           = vaMapBuffer(m_vaDisplay, vaImg.buf, (void **)&p_buffer);
    if (vaSts != VA_STATUS_SUCCESS)
        return VA_INVALID_ID;

    int i;
    mfxU16 sw = width;
    mfxU16 sh = height;

    mfxU8 *src = data;
    mfxU8 *dst = (mfxU8 *)p_buffer + vaImg.offsets[0];

    // y
    for (i = 0; i < sh; i++) {
        memcpy(dst, src, sw);
        src += sw;
        dst += vaImg.pitches[0];
    }

    dst = (mfxU8 *)p_buffer + vaImg.offsets[1];
    // uv
    for (i = 0; i < sh / 2; i++) {
        memcpy(dst, src, sw);
        src += sw;
        dst += vaImg.pitches[1];
    }

    vaSts = vaUnmapBuffer(m_vaDisplay, vaImg.buf);
    if (vaSts != VA_STATUS_SUCCESS)
        return VA_INVALID_ID;

    vaSts = vaDestroyImage(m_vaDisplay, vaImg.image_id);
    if (vaSts != VA_STATUS_SUCCESS)
        return VA_INVALID_ID;

    return vaSurfaceID;
}
