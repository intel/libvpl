/*############################################################################
  # Copyright (C) 2017 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __IGT_LOAD_H__
#define __IGT_LOAD_H__

#include <fcntl.h>
#include <i915_drm.h>
#include <pciaccess.h>
#include "i915_pciids.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct igt_spin igt_spin_t;

_Bool gem_has_bsd2(int fd);
uint16_t intel_get_drm_devid();
igt_spin_t* igt_spin_batch_new(int fd, uint32_t ctx, unsigned engine, uint32_t dep);
void igt_spin_batch_free(int fd, igt_spin_t* spin);

#ifdef __cplusplus
}
#endif

#endif /* __IGT_LOAD_H__ */
