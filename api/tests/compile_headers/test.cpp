/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include <stddef.h>

// force packing to 1 byte, which is never used by any real struct in our
// headers the definition of each structure should include setting the desired
// pack value (push) then immediately restoring the original value (pop)
#if defined(_WIN32) || defined(_WIN64)
__pragma(pack(push, 1))
#else
_Pragma("pack(push, 1)")
#endif

typedef struct {
    char a;
    int b;
} PackStart;

static_assert(offsetof(PackStart, a) == 0, "Error - offset of PackStart.a should be 0");
static_assert(offsetof(PackStart, b) == 1, "Error - offset of PackStart.b should be 1");

#include "./removed_api.h"

#include "./mfxvideo++.h"

#include "sdk_headers.inc"

// if every pack(push) was paired with exactly one pack(pop), then
//   packing should again be 1 byte at this point
typedef struct {
    char a;
    int b;
} PackEnd;

// if this test fails at build time, check that every struct definition is wrapped
//   with both MFX_PACK_BEGIN_XXX() and MFX_PACK_END()
static_assert(offsetof(PackEnd, a) == 0, "Error - offset of PackEnd.a should be 0");
static_assert(offsetof(PackEnd, b) == 1, "Error - offset of PackEnd.b should be 1");

#if defined(_WIN32) || defined(_WIN64)
__pragma(pack(pop))
#else
_Pragma("pack(pop)")
#endif

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
    mfxLoader loader = nullptr;

    MFX_ADD_PROPERTY_U32(loader, "mfxImplDescription.mfxVPPDescription.filter.FilterFourCC", 1)
    MFX_ADD_PROPERTY_PTR(loader, "mfxImplDescription.mfxVPPDescription.filter.FilterFourCC", "something_amazing")
    MFX_ADD_PROPERTY_U16(loader, "mfxImplDescription.Impl", MFX_IMPL_TYPE_SOFTWARE)

    {
      mfxConfig cfg = nullptr;

      MFX_UPDATE_PROPERTY_U16(loader, cfg, "mfxImplDescription.Impl", MFX_IMPL_TYPE_SOFTWARE)
      MFX_UPDATE_PROPERTY_U32(loader, cfg, "mfxImplDescription.Impl", MFX_IMPL_TYPE_SOFTWARE)
      MFX_UPDATE_PROPERTY_PTR(loader, cfg, "mfxImplDescription.Impl", "just a string")
    }

#ifdef ONEVPL_EXPERIMENTAL
  {
    mfxExtendedDeviceId *devinfo = nullptr;
    mfxU8* uuid = nullptr;
    mfxU8 sub_device_id = 0;
    MFX_UUID_COMPUTE_DEVICE_ID(devinfo, sub_device_id, uuid)
  }
#endif

    return 0;
}

#pragma message ("end of CPP test")
