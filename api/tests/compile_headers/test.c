/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include <stddef.h>

#include "./removed_api.h"

#include "sdk_headers.inc"

/* Stubs */

#define UNUSED_PARAM(x) (void)(x)

mfxLoader MFX_CDECL MFXLoad()
{
    return (mfxLoader)1;
}

mfxConfig MFX_CDECL MFXCreateConfig(mfxLoader loader)
{
    UNUSED_PARAM(loader);
    return (mfxConfig)2;
}

mfxStatus MFX_CDECL MFXSetConfigFilterProperty(mfxConfig config, const mfxU8* name, mfxVariant value)
{
    UNUSED_PARAM(config);
    UNUSED_PARAM(name);
    UNUSED_PARAM(value);
    return MFX_ERR_NONE;
}

int main()
{
    /* Checkout dispatcher's macroses*/
    mfxLoader loader = NULL;

    MFX_ADD_PROPERTY_U32(loader, "mfxImplDescription.mfxVPPDescription.filter.FilterFourCC", (mfxU32)MFX_EXTBUFF_VPP_SCALING)
    MFX_ADD_PROPERTY_PTR(loader, "mfxImplDescription.mfxVPPDescription.filter.FilterFourCC", "something_amazing")
    MFX_ADD_PROPERTY_U16(loader, "mfxImplDescription.Impl", MFX_IMPL_TYPE_SOFTWARE)

    {
      mfxConfig cfg = NULL;

      MFX_UPDATE_PROPERTY_U16(loader, cfg, "mfxImplDescription.Impl", MFX_IMPL_TYPE_SOFTWARE)
      MFX_UPDATE_PROPERTY_U32(loader, cfg, "mfxImplDescription.Impl", MFX_IMPL_TYPE_SOFTWARE)
      MFX_UPDATE_PROPERTY_PTR(loader, cfg, "mfxImplDescription.Impl", "just a string")
    }

#ifdef ONEVPL_EXPERIMENTAL
  {
    mfxExtendedDeviceId *devinfo = NULL;
    mfxU8* uuid = NULL;
    mfxU8 sub_device_id = 0;
    MFX_UUID_COMPUTE_DEVICE_ID(devinfo, sub_device_id, uuid)
  }
#endif

    return 0;
}

#pragma message ("end of C test")
