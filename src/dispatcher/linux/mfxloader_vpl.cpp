/*############################################################################
  # Copyright (C) 2019-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "linux/mfxloader_vpl.h"

mfxLoader MFXLoad() {
    return nullptr;
}

void MFXUnload(mfxLoader loader) {}

mfxConfig MFXCreateConfig(mfxLoader loader) {
    return nullptr;
}

mfxStatus MFXSetConfigFilterProperty(mfxConfig config,
                                     const mfxU8* name,
                                     mfxVariant value) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXEnumImplementations(mfxLoader loader,
                                 mfxU32 i,
                                 mfxImplCapsDeliveryFormat format,
                                 mfxHDL* idesc) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXCreateSession(mfxLoader loader, mfxU32 i, mfxSession* session) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXDispReleaseImplDescription(mfxLoader loader, mfxHDL hdl) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXMemory_GetSurfaceForVPP(mfxSession session,
                                     mfxFrameSurface1** surface) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXMemory_GetSurfaceForEncode(mfxSession session,
                                        mfxFrameSurface1** surface) {
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus MFXMemory_GetSurfaceForDecode(mfxSession session,
                                        mfxFrameSurface1** surface) {
    return MFX_ERR_UNSUPPORTED;
}
