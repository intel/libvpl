//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include <string.h>
#include "./hw-device.h"

mfxStatus DevCtxVAAPI::InitDevice(mfxU32 nAdapterNum, mfxHandleType *pHandleType, mfxHDL *pHandle) {
    m_vaDisplay            = nullptr;
    std::string renderPath = "/dev/dri/renderD";
    renderPath += std::to_string(128 + nAdapterNum);

    m_vaDRMfd = open(renderPath.c_str(), O_RDWR);
    if (m_vaDRMfd < 0)
        return MFX_ERR_DEVICE_FAILED;

    m_vaDisplay = vaGetDisplayDRM(m_vaDRMfd);

    int va_ver_major = 0, va_ver_minor = 0;
    VAStatus vaSts = vaInitialize(m_vaDisplay, &va_ver_major, &va_ver_minor);
    if (vaSts != VA_STATUS_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    *pHandleType = MFX_HANDLE_VA_DISPLAY;
    *pHandle     = (mfxHDL)m_vaDisplay;

    return MFX_ERR_NONE;
}

VASurfaceID DevCtxVAAPI::CreateSurfaceToShare(FrameInfo *pframeInfo, mfxU8 *data, bool brender) {
    VAStatus vaSts          = VA_STATUS_SUCCESS;
    VASurfaceID vaSurfaceID = VA_INVALID_ID;
    VAImage vaImg           = {};
    VASurfaceAttrib attrib  = {};
    attrib.type             = VASurfaceAttribPixelFormat;
    attrib.flags            = VA_SURFACE_ATTRIB_SETTABLE;
    attrib.value.type       = VAGenericValueTypeInteger;
    attrib.value.value.i =
        (pframeInfo->fourcc == MFX_FOURCC_NV12) ? VA_FOURCC_NV12 : VA_FOURCC_BGRA;

    vaSts = vaCreateSurfaces(
        m_vaDisplay,
        (pframeInfo->fourcc == MFX_FOURCC_NV12) ? VA_RT_FORMAT_YUV420 : VA_RT_FORMAT_RGB32,
        pframeInfo->width,
        pframeInfo->height,
        &vaSurfaceID,
        1,
        &attrib,
        1);
    if (vaSts != VA_STATUS_SUCCESS)
        return VA_INVALID_SURFACE;

    vaSts = vaDeriveImage(m_vaDisplay, vaSurfaceID, &vaImg);
    if (vaSts != VA_STATUS_SUCCESS)
        return VA_INVALID_SURFACE;

    void *pImgBuff = nullptr;
    vaSts          = vaMapBuffer(m_vaDisplay, vaImg.buf, &pImgBuff);
    if (vaSts != VA_STATUS_SUCCESS)
        return VA_INVALID_SURFACE;

    int i;
    mfxU16 sw     = pframeInfo->width;
    mfxU16 sh     = pframeInfo->height;
    mfxU16 spitch = pframeInfo->pitch;

    mfxU8 *src = data;

    if (pframeInfo->fourcc == MFX_FOURCC_NV12) {
        // y
        mfxU8 *dst = (mfxU8 *)pImgBuff + vaImg.offsets[0];
        for (i = 0; i < sh; i++) {
            memcpy(dst, src, sw);
            src += sw;
            dst += vaImg.pitches[0];
        }

        // uv
        // sometimes, uv plane is not followed by y plane
        // so, should check the offset for uv plane
        dst = (mfxU8 *)pImgBuff + vaImg.offsets[1];
        for (i = 0; i < sh / 2; i++) {
            memcpy(dst, src, sw);
            src += sw;
            dst += vaImg.pitches[1];
        }
    }
    else {
        mfxU8 *dst = (mfxU8 *)pImgBuff + vaImg.offsets[0];
        for (i = 0; i < sh; i++) {
            memcpy(dst, src, spitch);
            src += spitch;
            dst += vaImg.pitches[0];
        }
    }

    vaSts = vaUnmapBuffer(m_vaDisplay, vaImg.buf);
    if (vaSts != VA_STATUS_SUCCESS)
        return VA_INVALID_SURFACE;

    vaSts = vaDestroyImage(m_vaDisplay, vaImg.image_id);
    if (vaSts != VA_STATUS_SUCCESS)
        return VA_INVALID_SURFACE;

    return vaSurfaceID;
}
