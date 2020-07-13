/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "mfxdefs.h"

#ifndef __MFXIMPLCAPS_H__
#define __MFXIMPLCAPS_H__

#include "mfxstructures.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if (MFX_VERSION >= 2000)
/*!
   @brief
      This function delivers implementation capabilities in the requested format according to the format value.

   @param[in] format   Format in which capabilities must be delivered. See mfxImplCapsDeliveryFormat for more details.

   @return
      Handle to the capability report or NULL in case of unsupported format.
*/
mfxHDL MFX_CDECL MFXQueryImplDescription(mfxImplCapsDeliveryFormat format);

/*!
   @brief
      This function destoys handle allocated by MFXQueryImplCapabilities function.

   @param[in] hdl   Handle to destroy. Can be equal to NULL.

   @return
      MFX_ERR_NONE The function completed successfully.
*/
mfxStatus MFX_CDECL MFXReleaseImplDescription(mfxHDL hdl);
#endif


#ifdef __cplusplus
} // extern "C"
#endif

#endif // __MFXIMPLCAPS_H__
