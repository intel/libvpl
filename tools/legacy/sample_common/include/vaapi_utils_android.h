/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __VAAPI_UTILS_ANDROID_H__
#define __VAAPI_UTILS_ANDROID_H__

#if defined(LIBVA_ANDROID_SUPPORT)

    #include <va/va_android.h>
    #include "vaapi_utils.h"

class AndroidLibVA : public CLibVA {
public:
    AndroidLibVA(void);
    virtual ~AndroidLibVA(void);

protected:
    void* m_display;

private:
    DISALLOW_COPY_AND_ASSIGN(AndroidLibVA);
};

#endif // #if defined(LIBVA_ANDROID_SUPPORT)

#endif // #ifndef __VAAPI_UTILS_ANDROID_H__
