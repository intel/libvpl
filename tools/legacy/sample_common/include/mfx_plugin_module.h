/*############################################################################
  # Copyright (C) 2005 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#pragma once

#include "mfxplugin++.h"

struct PluginModuleTemplate {
    typedef MFXDecoderPlugin* (*fncCreateDecoderPlugin)();
    typedef MFXEncoderPlugin* (*fncCreateEncoderPlugin)();
    typedef MFXAudioDecoderPlugin* (*fncCreateAudioDecoderPlugin)();
    typedef MFXAudioEncoderPlugin* (*fncCreateAudioEncoderPlugin)();
    typedef MFXGenericPlugin* (*fncCreateGenericPlugin)();
    typedef mfxStatus(MFX_CDECL* CreatePluginPtr_t)(mfxPluginUID uid, mfxPlugin* plugin);

    fncCreateDecoderPlugin CreateDecoderPlugin;
    fncCreateEncoderPlugin CreateEncoderPlugin;
    fncCreateGenericPlugin CreateGenericPlugin;
    CreatePluginPtr_t CreatePlugin;
    fncCreateAudioDecoderPlugin CreateAudioDecoderPlugin;
    fncCreateAudioEncoderPlugin CreateAudioEncoderPlugin;
};

extern PluginModuleTemplate g_PluginModule;
