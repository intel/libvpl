/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "mfx_samples_config.h"

#if defined(_WIN32) || defined(_WIN64)

    #include "vm/so_defs.h"

    #include <windows.h>

msdk_so_handle msdk_so_load(const char* file_name) {
    if (!file_name)
        return NULL;
    return (msdk_so_handle)LoadLibrary((LPCTSTR)file_name);
}

msdk_func_pointer msdk_so_get_addr(msdk_so_handle handle, const char* func_name) {
    if (!handle)
        return NULL;
    return (msdk_func_pointer)GetProcAddress((HMODULE)handle, /*(LPCSTR)*/ func_name);
}

void msdk_so_free(msdk_so_handle handle) {
    if (!handle)
        return;
    FreeLibrary((HMODULE)handle);
}

#endif // #if defined(_WIN32) || defined(_WIN64)
