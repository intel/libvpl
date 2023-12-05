/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

///
/// Example of a minimal Intel® Video Processing Library (Intel® VPL) application.
///
/// @file

#include <gtest/gtest.h>

#include "vpl/mfxdispatcher.h"

TEST(CreateSession, SucceedsWithStubImpl) {
    mfxLoader loader = MFXLoad();
    ASSERT_NE(loader, nullptr) << "MFXLoad() returned null - no libraries found ";
    mfxConfig cfg = MFXCreateConfig(loader);

    mfxVariant vendor_id;
    vendor_id.Type     = MFX_VARIANT_TYPE_U32;
    vendor_id.Data.U32 = 0x8086;
    mfxVariant vendor_impl_id;
    vendor_impl_id.Type     = MFX_VARIANT_TYPE_U32;
    vendor_impl_id.Data.U32 = 0xFFFF;
    mfxStatus sts           = MFX_ERR_NONE;
    sts = MFXSetConfigFilterProperty(cfg, (const mfxU8 *)"mfxImplDescription.VendorID", vendor_id);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "MFXSetConfigFilterProperty failed with code " << sts;
    sts = MFXSetConfigFilterProperty(cfg,
                                     (const mfxU8 *)"mfxImplDescription.VendorImplID",
                                     vendor_impl_id);
    ASSERT_EQ(sts, MFX_ERR_NONE) << "MFXSetConfigFilterProperty failed with code " << sts;

    mfxSession session = NULL;
    sts                = MFXCreateSession(loader, 0, &session);
    EXPECT_EQ(sts, MFX_ERR_NONE) << "MFXCreateSession failed with code " << sts;

    sts = MFXClose(session);
    EXPECT_EQ(sts, MFX_ERR_NONE) << "MFXClose failed with code " << sts;

    MFXUnload(loader);
}
