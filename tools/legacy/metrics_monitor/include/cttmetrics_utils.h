/*############################################################################
  # Copyright (C) 2018 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef __CTTMETRICS_UTILS_H__
#define __CTTMETRICS_UTILS_H__

#include <stdio.h>

#include "cttmetrics.h"

static const char DRM_DIR[]                   = "/sys/class/drm";
static const int PCI_DISPLAY_CONTROLLER_CLASS = 0x03;
static const int INTEL_VENDOR_ID              = 0x8086;

extern char CARD_N[16];
extern char CARD[32];

extern int path_gen(char* pdst,
                    size_t sz,
                    const char delim,
                    const char* a,
                    const char* b,
                    const char* c);
extern cttStatus discover_path_to_gpu();

extern int read_freq(int fd);

#endif // #ifndef __CTTMETRICS_UTILS_H__
