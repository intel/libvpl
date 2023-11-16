//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#ifndef DISPATCHER_HELLO_VAAPI_SHARING_IMPORT_SRC_HW_DEVICE_H_
#define DISPATCHER_HELLO_VAAPI_SHARING_IMPORT_SRC_HW_DEVICE_H_
#ifdef __linux__
    #include <fcntl.h>
    #include <unistd.h>
    #include "va/va.h"
    #include "va/va_drm.h"
    #include "va/va_drmcommon.h"

    #include "vpl/mfx.h"
struct DevCtxVAAPI {
public:
    DevCtxVAAPI() : m_vaDRMfd(-1), m_vaDisplay() {}

    ~DevCtxVAAPI() {
        if (m_vaDisplay) {
            vaTerminate(m_vaDisplay);
            m_vaDisplay = nullptr;
        }

        if (m_vaDRMfd >= 0) {
            close(m_vaDRMfd);
            m_vaDRMfd = -1;
        }
    }

    mfxStatus InitDevice(mfxU32 nAdapterNum, mfxHandleType *pHandleType, mfxHDL *pHandle);

    VADisplay GetVADisplay(void) {
        return m_vaDisplay;
    }

    VASurfaceID CreateSurfaceToShare(mfxU16 width,
                                     mfxU16 height,
                                     mfxU8 *data,
                                     bool brender = false);

private:
    int m_vaDRMfd;
    VADisplay m_vaDisplay;
};

#endif //Linux

#endif // DISPATCHER__HELLO_VAAPI_SHARING_IMPORT_SRC_HW_DEVICE_H_
