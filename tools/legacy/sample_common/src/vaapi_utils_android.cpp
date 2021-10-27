/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifdef LIBVA_ANDROID_SUPPORT
    #ifdef ANDROID

        #include "vaapi_utils_android.h"

CLibVA* CreateLibVA(int) {
    return new AndroidLibVA;
}

/*------------------------------------------------------------------------------*/

typedef unsigned int vaapiAndroidDisplay;

        #define VAAPI_ANDROID_DEFAULT_DISPLAY 0x18c34078

AndroidLibVA::AndroidLibVA(void) : CLibVA(MFX_LIBVA_AUTO), m_display(NULL) {
    VAStatus va_res   = VA_STATUS_SUCCESS;
    mfxStatus sts     = MFX_ERR_NONE;
    int major_version = 0, minor_version = 0;
    vaapiAndroidDisplay* display = NULL;

    m_display = display = (vaapiAndroidDisplay*)malloc(sizeof(vaapiAndroidDisplay));
    if (NULL == m_display)
        sts = MFX_ERR_NOT_INITIALIZED;
    else
        *display = VAAPI_ANDROID_DEFAULT_DISPLAY;

    if (MFX_ERR_NONE == sts) {
        m_va_dpy = vaGetDisplay(m_display);
        if (!m_va_dpy) {
            free(m_display);
            sts = MFX_ERR_NULL_PTR;
        }
    }
    if (MFX_ERR_NONE == sts) {
        va_res = vaInitialize(m_va_dpy, &major_version, &minor_version);
        sts    = va_to_mfx_status(va_res);
        if (MFX_ERR_NONE != sts) {
            free(display);
            m_display = NULL;
        }
    }
    if (MFX_ERR_NONE != sts)
        throw std::bad_alloc();
}

AndroidLibVA::~AndroidLibVA(void) {
    if (m_va_dpy) {
        vaTerminate(m_va_dpy);
    }
    if (m_display) {
        free(m_display);
    }
}

    #endif // #ifdef ANDROID
#endif // #ifdef LIBVA_ANDROID_SUPPORT
