/*############################################################################
  # Copyright (C) 2021 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <stdlib.h>
#include <stdio.h>

#include "mfx.h"

/* These macro required for code compilation. */
#define INFINITE 0x7FFFFFFF
#define UNUSED_PARAM(x) (void)(x)

mfxSession session, sessionSW, sessionHW;

void prg_dispatcher1 () {
/*beg1*/
mfxLoader loader = MFXLoad();
MFXCreateSession(loader,0,&session);
/*end1*/
}

void prg_dispatcher2 () {
/*beg2*/
mfxLoader loader = MFXLoad();
mfxConfig cfg = MFXCreateConfig(loader);
mfxVariant ImplValue;
ImplValue.Type = MFX_VARIANT_TYPE_U32;
ImplValue.Data.U32 = MFX_IMPL_TYPE_HARDWARE;
MFXSetConfigFilterProperty(cfg,(const mfxU8 *)"mfxImplDescription.Impl",ImplValue);
MFXCreateSession(loader,0,&session);
/*end2*/
}

void prg_dispatcher3 () {
/*beg3*/
// Create session with software based implementation
mfxLoader loader1 = MFXLoad();
mfxConfig cfg1 = MFXCreateConfig(loader1);
mfxVariant ImplValueSW;
ImplValueSW.Type = MFX_VARIANT_TYPE_U32;
ImplValueSW.Data.U32 = MFX_IMPL_TYPE_SOFTWARE;
MFXSetConfigFilterProperty(cfg1,(const mfxU8 *)"mfxImplDescription.Impl",ImplValueSW);
MFXCreateSession(loader1,0,&sessionSW);

// Create session with hardware based implementation
mfxLoader loader2 = MFXLoad();
mfxConfig cfg2 = MFXCreateConfig(loader2);
mfxVariant ImplValueHW;
ImplValueHW.Type = MFX_VARIANT_TYPE_U32;
ImplValueHW.Data.U32 = MFX_IMPL_TYPE_HARDWARE;
MFXSetConfigFilterProperty(cfg2,(const mfxU8 *)"mfxImplDescription.Impl",ImplValueHW);
MFXCreateSession(loader2,0,&sessionHW);

// use both sessionSW and sessionHW
// ...
// Close everything
MFXClose(sessionSW);
MFXClose(sessionHW);
MFXUnload(loader1); // cfg1 will be destroyed here.
MFXUnload(loader2); // cfg2 will be destroyed here.
/*end3*/
}

void prg_dispatcher4 () {
/*beg4*/
mfxLoader loader = MFXLoad();

// We want to have AVC decoder supported.
mfxConfig cfg1 = MFXCreateConfig(loader);
mfxVariant ImplValue;
ImplValue.Type = MFX_VARIANT_TYPE_U32;
ImplValue.Data.U32 = MFX_CODEC_AVC;
MFXSetConfigFilterProperty(cfg1,
            (const mfxU8 *)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID",ImplValue);

// And we want to have HEVC encoder supported by the same implementation.
mfxConfig cfg2 = MFXCreateConfig(loader);
ImplValue.Type = MFX_VARIANT_TYPE_U32;
ImplValue.Data.U32 = MFX_CODEC_HEVC;
MFXSetConfigFilterProperty(cfg2,
            (const mfxU8 *)"mfxImplDescription.mfxEncoderDescription.encoder.CodecID",ImplValue);

// To create single session with both capabilities.
MFXCreateSession(loader,0,&session);
/*end4*/
}
