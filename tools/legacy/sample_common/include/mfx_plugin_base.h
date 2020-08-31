/*###########################################################################
  # Copyright (C) 2005-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ###########################################################################*/

#pragma once

#include <mfxplugin++.h>

typedef MFXDecoderPlugin* (*mfxCreateDecoderPlugin)();
typedef MFXEncoderPlugin* (*mfxCreateEncoderPlugin)();
typedef MFXGenericPlugin* (*mfxCreateGenericPlugin)();
