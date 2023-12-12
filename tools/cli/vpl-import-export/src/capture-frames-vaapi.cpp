//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./capture-frames.h"

mfxStatus CaptureCtxVAAPI::CaptureInit(DevCtx *devCtx) {
#ifdef TOOLS_ENABLE_X11
    m_devCtx      = devCtx;
    m_pX11Display = m_devCtx->GetX11Display();
    if (!m_pX11Display)
        return MFX_ERR_DEVICE_FAILED;

    int x11ScreenNumber = DefaultScreen(m_pX11Display);
    if (x11ScreenNumber == -1)
        return MFX_ERR_DEVICE_FAILED;

    m_x11Window = RootWindow(m_pX11Display, x11ScreenNumber);
    if (m_x11Window == None)
        return MFX_ERR_DEVICE_FAILED;

    m_dispWidth  = (mfxU32)XDisplayWidth(m_pX11Display, x11ScreenNumber);
    m_dispHeight = (mfxU32)XDisplayHeight(m_pX11Display, x11ScreenNumber);

    printf("CaptureCtxVAAPI::CaptureInit() - success - resolution = [%d x %d]\n", m_dispWidth, m_dispHeight);

    return MFX_ERR_NONE;
#else
    printf("CaptureCtxVAAPI::CaptureInit() - ERROR - TOOLS_ENABLE_X11 not defined\n");
    return MFX_ERR_UNSUPPORTED;
#endif
}

mfxStatus CaptureCtxVAAPI::CaptureFrame(VASurfaceID *pvaSurfaceID) {
#ifdef TOOLS_ENABLE_X11
    XImage *imgScreen;
    imgScreen = XGetImage(m_pX11Display, m_x11Window, 0, 0, m_dispWidth, m_dispHeight, AllPlanes, ZPixmap);
    if (imgScreen == NULL) {
        return MFX_ERR_NULL_PTR;
    }

    VADisplay vaDisplay = m_devCtx->GetVADisplay();

    VASurfaceID scrVASurfaceID;
    vaCreateSurfaces(vaDisplay, VA_RT_FORMAT_YUV420, m_dispWidth, m_dispHeight, &scrVASurfaceID, 1, NULL, 0);

    VAImage vaImage;
    vaDeriveImage(vaDisplay, scrVASurfaceID, &vaImage);

    unsigned char *va_image_data;
    vaMapBuffer(vaDisplay, vaImage.buf, reinterpret_cast<void **>(&va_image_data));

    unsigned char *y_plane  = va_image_data + vaImage.offsets[0];
    unsigned char *uv_plane = va_image_data + vaImage.offsets[1];

    int y_w                  = vaImage.pitches[0];
    int u_w                  = vaImage.pitches[1];
    unsigned char *src_rgb32 = reinterpret_cast<unsigned char *>(imgScreen->data);

    for (int j = 0; j < imgScreen->height; ++j) {
        for (int i = 0; i < imgScreen->width; ++i) {
            int rgb_i = (j * imgScreen->width + i) * 4;
            int y_i   = j * y_w + i;
            int uv_i  = ((j >> 1) * (u_w >> 1) + (i >> 1)) << 1;

            uint8_t b = src_rgb32[rgb_i];
            uint8_t g = src_rgb32[rgb_i + 1];
            uint8_t r = src_rgb32[rgb_i + 2];

            uint8_t y = static_cast<uint8_t>((0.257 * r) + (0.504 * g) + (0.098 * b) + 16);
            uint8_t u = static_cast<uint8_t>((-0.148 * r) - (0.291 * g) + (0.439 * b) + 128);
            uint8_t v = static_cast<uint8_t>((0.439 * r) - (0.368 * g) - (0.071 * b) + 128);

            y_plane[y_i] = y;

            if (j % 2 == 0 && i % 2 == 0) {
                uv_plane[uv_i]     = u;
                uv_plane[uv_i + 1] = v;
            }
        }
    }

    vaUnmapBuffer(vaDisplay, vaImage.buf);

    vaDestroyImage(vaDisplay, vaImage.image_id);
    XDestroyImage(imgScreen);

    *pvaSurfaceID = scrVASurfaceID;

    m_lastCapturedSurfaceID = scrVASurfaceID; // for release

    return MFX_ERR_NONE;
#else
    return MFX_ERR_UNSUPPORTED;
#endif
}

mfxStatus CaptureCtxVAAPI::ReleaseFrame() {
#ifdef TOOLS_ENABLE_X11
    VADisplay vaDisplay = m_devCtx->GetVADisplay();
    vaDestroySurfaces(vaDisplay, &m_lastCapturedSurfaceID, 1);

    return MFX_ERR_NONE;
#else
    return MFX_ERR_UNSUPPORTED;
#endif
}
