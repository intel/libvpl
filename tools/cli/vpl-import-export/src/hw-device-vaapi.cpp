//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include "./hw-device.h"

mfxStatus DevCtxVAAPI::InitDevice(mfxU32 nAdapterNum, mfxHandleType *pHandleType, mfxHDL *pHandle) {
    m_vaDisplay = nullptr;

#ifdef TOOLS_ENABLE_X11
    // if X11 is running, connect to server and get current display
    m_pX11Display = XOpenDisplay(NULL);
    if (m_pX11Display) {
        m_vaDisplay = vaGetDisplay(m_pX11Display);
        if (!m_vaDisplay) {
            XCloseDisplay(m_pX11Display);
            m_pX11Display = nullptr;
        }

        // rendering currently implements DRI2 path only
        if (!getenv("LIBVA_DRI3_DISABLE")) {
            printf("DevCtxVAAPI::InitDevice() - setting environment variable LIBVA_DRI3_DISABLE\n");
            setenv("LIBVA_DRI3_DISABLE", "1", 0);
        }
    }
#endif

    // if X11 is not running (e.g. SSH session) fallback to DRM
    // Rendering and screen capture will not be available
    if (!m_vaDisplay) {
        printf("*** WARNING -- DevCtxVAAPI -- X11 Display not supported, falling back to DRM\n");
        std::string renderPath = "/dev/dri/renderD";
        renderPath += std::to_string(128 + nAdapterNum);

        m_vaDRMfd = open(renderPath.c_str(), O_RDWR);
        if (m_vaDRMfd < 0)
            return MFX_ERR_DEVICE_FAILED;

        m_vaDisplay = vaGetDisplayDRM(m_vaDRMfd);
    }

    int va_ver_major = 0, va_ver_minor = 0;
    VAStatus vaSts = vaInitialize(m_vaDisplay, &va_ver_major, &va_ver_minor);
    if (vaSts != VA_STATUS_SUCCESS)
        return MFX_ERR_DEVICE_FAILED;

    *pHandleType = MFX_HANDLE_VA_DISPLAY;
    *pHandle     = (mfxHDL)m_vaDisplay;

    return MFX_ERR_NONE;
}
