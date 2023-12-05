/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef LIBVPL_SRC_WINDOWS_MFX_LOAD_DLL_H_
#define LIBVPL_SRC_WINDOWS_MFX_LOAD_DLL_H_

#include "src/windows/mfx_dispatcher.h"

namespace MFX {

//
// declare DLL loading routines
//

mfxStatus mfx_get_rt_dll_name(wchar_t *pPath, size_t pathSize);
mfxStatus mfx_get_default_dll_name(wchar_t *pPath, size_t pathSize, eMfxImplType implType);
mfxStatus mfx_get_default_vpl_dll_name(wchar_t *pPath, size_t pathSize);
mfxStatus mfx_get_default_plugin_name(wchar_t *pPath, size_t pathSize, eMfxImplType implType);
#if defined(MEDIASDK_UWP_DISPATCHER)
mfxStatus mfx_get_default_intel_gfx_api_dll_name(wchar_t *pPath, size_t pathSize);
#endif

mfxStatus mfx_get_default_audio_dll_name(wchar_t *pPath, size_t pathSize, eMfxImplType implType);

mfxModuleHandle mfx_dll_load(const wchar_t *file_name);
//increments reference counter
mfxModuleHandle mfx_get_dll_handle(const wchar_t *file_name);
mfxFunctionPointer mfx_dll_get_addr(mfxModuleHandle handle, const char *func_name);
bool mfx_dll_free(mfxModuleHandle handle);

} // namespace MFX

#endif // LIBVPL_SRC_WINDOWS_MFX_LOAD_DLL_H_
