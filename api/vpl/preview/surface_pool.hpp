/*############################################################################
  # Copyright Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include <cinttypes>
#include "vpl/mfxsurfacepool.h"
#include "extension_buffer.hpp"
namespace oneapi {
namespace vpl {

enum class vppl_pool_type : uint32_t {
    input_pool,
    output_pool,
};

REGISTER_TRIVIAL_EXT_BUFFER(ExtAllocationHints,
                            mfxExtAllocationHints,
                            MFX_EXTBUFF_ALLOCATION_HINTS)

} // namespace vpl
} // namespace oneapi
