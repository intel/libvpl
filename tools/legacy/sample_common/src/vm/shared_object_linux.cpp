/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#if !defined(_WIN32) && !defined(_WIN64)

    #include <dlfcn.h>
    #include "vm/so_defs.h"

msdk_so_handle msdk_so_load(const char* file_name) {
    if (!file_name)
        return NULL;
    return (msdk_so_handle)dlopen(file_name, RTLD_LAZY);
}

msdk_func_pointer msdk_so_get_addr(msdk_so_handle handle, const char* func_name) {
    if (!handle)
        return NULL;
    return (msdk_func_pointer)dlsym(handle, func_name);
}

void msdk_so_free(msdk_so_handle handle) {
    if (!handle)
        return;
    dlclose(handle);
}

#endif // #if !defined(_WIN32) && !defined(_WIN64)
