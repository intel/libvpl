/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __VAAPI_UTILS_X11_H__
#define __VAAPI_UTILS_X11_H__

#if defined(LIBVA_X11_SUPPORT)

    #include <va/va_x11.h>
    #include "vaapi_utils.h"

class X11LibVA : public CLibVA {
public:
    X11LibVA(void);
    virtual ~X11LibVA(void);

    void* GetXDisplay(void) {
        return m_display;
    }

    MfxLoader::XLib_Proxy& GetX11() {
        return m_x11lib;
    }
    MfxLoader::VA_X11Proxy& GetVAX11() {
        return m_vax11lib;
    }
    #if defined(X11_DRI3_SUPPORT)
    MfxLoader::Xcb_Proxy& GetXcbX11() {
        return m_xcblib;
    }
    MfxLoader::X11_Xcb_Proxy& GetX11XcbX11() {
        return m_x11xcblib;
    }
    MfxLoader::XCB_Dri3_Proxy& GetXCBDri3X11() {
        return m_xcbdri3lib;
    }
    MfxLoader::Xcbpresent_Proxy& GetXcbpresentX11() {
        return m_xcbpresentlib;
    }
    MfxLoader::DrmIntel_Proxy& GetDrmIntelX11() {
        return m_drmintellib;
    }
    #endif // X11_DRI3_SUPPORT

protected:
    Display* m_display;
    VAConfigID m_configID;
    VAContextID m_contextID;
    MfxLoader::XLib_Proxy m_x11lib;
    MfxLoader::VA_X11Proxy m_vax11lib;
    #if defined(X11_DRI3_SUPPORT)
    MfxLoader::VA_DRMProxy m_vadrmlib;
    MfxLoader::Xcb_Proxy m_xcblib;
    MfxLoader::X11_Xcb_Proxy m_x11xcblib;
    MfxLoader::XCB_Dri3_Proxy m_xcbdri3lib;
    MfxLoader::Xcbpresent_Proxy m_xcbpresentlib;
    MfxLoader::DrmIntel_Proxy m_drmintellib;
    #endif // X11_DRI3_SUPPORT
    int fd;

private:
    void Close();

    DISALLOW_COPY_AND_ASSIGN(X11LibVA);
};

#endif // #if defined(LIBVA_X11_SUPPORT)

#endif // #ifndef __VAAPI_UTILS_X11_H__
