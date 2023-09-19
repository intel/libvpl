/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <stdlib.h>
#include <stdio.h>

#include <vector>
#include <algorithm>

#include "mfxdefs.h"
#include "mfxvideo.h"
#include "mfxadapter.h"
#include "mfxdispatcher.h"

/* These macro required for code compilation. */

#define MSDK_CHECK_STATUS(X, MSG)                {if ((X) < MFX_ERR_NONE) {printf("%s\n",MSG);}}

/* end of internal stuff */


static void* vaDisplay;


static int prg_hw1 () {
/*beg1*/
mfxU32 num_adapters_available;
mfxIMPL impl;

// Query number of graphics adapters available on system
mfxStatus sts = MFXQueryAdaptersNumber(&num_adapters_available);
MSDK_CHECK_STATUS(sts, "MFXQueryAdaptersNumber failed");

// Allocate memory for response
std::vector<mfxAdapterInfo> displays_data(num_adapters_available);
mfxAdaptersInfo adapters = { displays_data.data(), mfxU32(displays_data.size()), 0u, {0} };

// Query information about all adapters (mind that first parameter is NULL)
sts = MFXQueryAdapters(nullptr, &adapters);
MSDK_CHECK_STATUS(sts, "MFXQueryAdapters failed");

// Find dGfx adapter in list of adapters
auto idx_d = std::find_if(adapters.Adapters, adapters.Adapters + adapters.NumActual,
    [](const mfxAdapterInfo info)
{
   return info.Platform.MediaAdapterType == mfxMediaAdapterType::MFX_MEDIA_DISCRETE;
});

// No dGfx in list
if (idx_d == adapters.Adapters + adapters.NumActual)
{
   printf("Warning: No dGfx detected on machine\n");
   return -1;
}

mfxU32 idx = static_cast<mfxU32>(std::distance(adapters.Adapters, idx_d));

// Choose correct implementation for discrete adapter
switch (adapters.Adapters[idx].Number)
{
case 0:
   impl = MFX_IMPL_HARDWARE;
   break;
case 1:
   impl = MFX_IMPL_HARDWARE2;
   break;
case 2:
   impl = MFX_IMPL_HARDWARE3;
   break;
case 3:
   impl = MFX_IMPL_HARDWARE4;
   break;

default:
   // Try searching on all display adapters
   impl = MFX_IMPL_HARDWARE_ANY;
   break;
}
printf("Choosen implementation: %d\n", impl);
// Initialize mfxSession in regular way with obtained implementation.
/*end1*/
return 0;
}

static int prg_hw2 () {
/*beg2*/
mfxU32 num_adapters_available;
mfxIMPL impl;
mfxVideoParam Encode_mfxVideoParam;

// Query number of graphics adapters available on system
mfxStatus sts = MFXQueryAdaptersNumber(&num_adapters_available);
MSDK_CHECK_STATUS(sts, "MFXQueryAdaptersNumber failed");

// Allocate memory for response
std::vector<mfxAdapterInfo> displays_data(num_adapters_available);
mfxAdaptersInfo adapters = { displays_data.data(), mfxU32(displays_data.size()), 0u, {0} };

// Fill description of Encode workload
mfxComponentInfo interface_request = { MFX_COMPONENT_ENCODE, Encode_mfxVideoParam, {0} };

// Query information about suitable adapters for Encode workload described by Encode_mfxVideoParam
sts = MFXQueryAdapters(&interface_request, &adapters);

if (sts == MFX_ERR_NOT_FOUND)
{
   printf("Error: No adapters on machine capable to process desired workload\n");
   return -1;
}

MSDK_CHECK_STATUS(sts, "MFXQueryAdapters failed");

// Choose correct implementation for discrete adapter. Mind usage of index 0, this is best suitable adapter from MSDK perspective
switch (adapters.Adapters[0].Number)
{
case 0:
   impl = MFX_IMPL_HARDWARE;
   break;
case 1:
   impl = MFX_IMPL_HARDWARE2;
   break;
case 2:
   impl = MFX_IMPL_HARDWARE3;
   break;
case 3:
   impl = MFX_IMPL_HARDWARE4;
   break;

default:
   // Try searching on all display adapters
   impl = MFX_IMPL_HARDWARE_ANY;
   break;
}

printf("Choosen implementation: %d\n", impl);

// Initialize mfxSession in regular way with obtained implementation
/*end2*/
return 0;
}

static int prg_hw3 () {
/*beg3*/
mfxLoader loader = MFXLoad();
mfxConfig config1 = MFXCreateConfig(loader);
mfxConfig config2 = MFXCreateConfig(loader);
mfxSession session;

mfxVariant HandleType;
HandleType.Type = MFX_VARIANT_TYPE_U32;
HandleType.Data.U32 = MFX_HANDLE_VA_DISPLAY;
MFXSetConfigFilterProperty(config1, (mfxU8*)"mfxHandleType", HandleType);

mfxVariant DisplayHandle;
DisplayHandle.Type = MFX_VARIANT_TYPE_PTR;
HandleType.Data.Ptr = vaDisplay;
MFXSetConfigFilterProperty(config2, (mfxU8*)"mfxHDL", DisplayHandle);

MFXCreateSession(loader, 0, &session);

/*end3*/
return 0;
}

