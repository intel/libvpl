/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#if defined(LIBVA_X11_SUPPORT)

    #include "vaapi_utils_x11.h"
    #include "sample_defs.h"

    #include <dlfcn.h>
    #if defined(X11_DRI3_SUPPORT)
        #include <fcntl.h>
    #endif
    #include <sys/ioctl.h>

    #define VAAPI_X_DEFAULT_DISPLAY ":0.0"

const char* MFX_X11_NODE_RENDER          = "/dev/dri/renderD";
const char* MFX_X11_DRIVER_NAME          = "i915";
constexpr mfxU32 MFX_X11_DRIVER_NAME_LEN = 4;
constexpr mfxU32 MFX_X11_NODE_INDEX      = 128;
constexpr mfxU32 MFX_X11_MAX_NODES       = 16;

X11LibVA::X11LibVA(void)
        : CLibVA(MFX_LIBVA_X11),
          m_display(0),
          m_configID(VA_INVALID_ID),
          m_contextID(VA_INVALID_ID) {
    char* currentDisplay = getenv("DISPLAY");
    fd                   = -1;

    m_display = (currentDisplay) ? m_x11lib.XOpenDisplay(currentDisplay)
                                 : m_x11lib.XOpenDisplay(VAAPI_X_DEFAULT_DISPLAY);

    if (!m_display) {
        printf("Failed to open X Display: try to check/set DISPLAY environment variable.\n");
        throw std::bad_alloc();
    }

    #if defined(X11_DRI3_SUPPORT)
    for (mfxU32 i = 0; i < MFX_X11_MAX_NODES; ++i) {
        std::string devPath = MFX_X11_NODE_RENDER + std::to_string(MFX_X11_NODE_INDEX + i);
        fd                  = open(devPath.c_str(), O_RDWR);
        if (fd < 0)
            continue;

        char driverName[MFX_X11_DRIVER_NAME_LEN + 1] = {};
        drm_version_t version                        = {};
        version.name_len                             = MFX_X11_DRIVER_NAME_LEN;
        version.name                                 = driverName;

        if (!ioctl(fd, DRM_IOWR(0, drm_version), &version) &&
            msdk_match(driverName, MFX_X11_DRIVER_NAME)) {
            break;
        }
        close(fd);
        fd = -1;
    }

    if (fd < 0) {
        m_x11lib.XCloseDisplay(m_display);
        printf("Failed to open adapter\n");
        throw std::bad_alloc();
    }

    m_va_dpy = m_vadrmlib.vaGetDisplayDRM(fd);
    #else
    m_va_dpy = m_vax11lib.vaGetDisplay(m_display);
    #endif

    if (!m_va_dpy) {
        m_x11lib.XCloseDisplay(m_display);
        printf("Failed to get VA Display\n");
        throw std::bad_alloc();
    }

    int major_version = 0, minor_version = 0;
    VAStatus sts = m_libva.vaInitialize(m_va_dpy, &major_version, &minor_version);

    if (VA_STATUS_SUCCESS != sts) {
        m_x11lib.XCloseDisplay(m_display);
        printf("Failed to initialize VAAPI: %d\n", (int)sts);
        throw std::bad_alloc();
    }

    #if !defined(X11_DRI3_SUPPORT)
    VAConfigAttrib cfgAttrib{};
    if (VA_STATUS_SUCCESS == sts) {
        cfgAttrib.type = VAConfigAttribRTFormat;
        sts            = m_libva.vaGetConfigAttributes(m_va_dpy,
                                            VAProfileNone,
                                            VAEntrypointVideoProc,
                                            &cfgAttrib,
                                            1);
    }
    if (VA_STATUS_SUCCESS == sts) {
        sts = m_libva.vaCreateConfig(m_va_dpy,
                                     VAProfileNone,
                                     VAEntrypointVideoProc,
                                     &cfgAttrib,
                                     1,
                                     &m_configID);
    }
    if (VA_STATUS_SUCCESS == sts) {
        sts =
            m_libva.vaCreateContext(m_va_dpy, m_configID, 0, 0, VA_PROGRESSIVE, 0, 0, &m_contextID);
    }
    if (VA_STATUS_SUCCESS != sts) {
        Close();
        printf("Failed to initialize VP: %d\n", sts);
        throw std::bad_alloc();
    }
    #endif // X11_DRI3_SUPPORT
}

X11LibVA::~X11LibVA(void) {
    Close();
}

void X11LibVA::Close() {
    VAStatus sts;
    if (m_contextID != VA_INVALID_ID) {
        sts = m_libva.vaDestroyContext(m_va_dpy, m_contextID);
        if (sts != VA_STATUS_SUCCESS)
            printf("Failed to destroy VA context: %d\n", (int)sts);
    }
    if (m_configID != VA_INVALID_ID) {
        sts = m_libva.vaDestroyConfig(m_va_dpy, m_configID);
        if (sts != VA_STATUS_SUCCESS)
            printf("Failed to destroy VA config: %d\n", (int)sts);
    }
    sts = m_libva.vaTerminate(m_va_dpy);
    if (sts != VA_STATUS_SUCCESS)
        printf("Failed to close VAAPI library: %d\n", (int)sts);

    m_x11lib.XCloseDisplay(m_display);
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

#endif // #if defined(LIBVA_X11_SUPPORT)
