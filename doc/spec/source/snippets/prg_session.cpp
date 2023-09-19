/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "mfxdefs.h"
#include "mfxdispatcher.h"
#include "mfxstructures.h"

#include <iostream>
#include <algorithm>

mfxLoader loader = 0;
int idx = 0;

static void prg_session1 () {
/*beg1*/
mfxHDL h;
// request pointer to the list. Assume that implementation supports that.
// Assume that `loader` is configured before.
mfxStatus sts = MFXEnumImplementations(loader, idx, MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS, &h);
// break if no idx
if (sts != MFX_ERR_NOT_FOUND) {
  // Cast typeless handle to structure pointer
  mfxImplementedFunctions *implemented_functions = (mfxImplementedFunctions*)h;

  // print out list of functions' name
  std::for_each(implemented_functions->FunctionsName, implemented_functions->FunctionsName +
                                                      implemented_functions->NumFunctions,
          [](mfxChar*  functionName) {
              std::cout << functionName << " is implemented" << std::endl;
          });
  // Release resource
  MFXDispReleaseImplDescription(loader, h);
}
/*end1*/
}

static void prg_session2 () {
/*beg2*/
mfxSession session_handle;
loader = mfxLoader();

// We want to search for the implementation with Decode+VPP domain functions support.
// i.e we search for the MFXVideoDECODE_VPP_Init and MFXVideoDECODE_VPP_DecodeFrameAsync
// implemented functions
mfxConfig init_funct_prop = MFXCreateConfig(loader);
mfxVariant value;

// Filter property for the Init function
value.Type = mfxVariantType::MFX_VARIANT_TYPE_PTR;
value.Data.Ptr = (mfxHDL)"MFXVideoDECODE_VPP_Init";
MFXSetConfigFilterProperty(init_funct_prop, (const mfxU8*)"mfxImplementedFunctions.FunctionsName",
                           value);

// Filter property for the Process function
mfxConfig process_func_prop = MFXCreateConfig(loader);
value.Data.Ptr = (mfxHDL)"MFXVideoDECODE_VPP_DecodeFrameAsync";
MFXSetConfigFilterProperty(process_func_prop, (const mfxU8*)"mfxImplementedFunctions.FunctionsName",
                           value);

// create session from first matched implementation
MFXCreateSession(loader, 0, &session_handle);
/*end2*/
}

static void prg_session3 () {
/*beg3*/
mfxSession hevc_session_handle;
loader = mfxLoader();

// We want to search for the HEVC decoder implementation
mfxConfig hevc_decoder_config = MFXCreateConfig(loader);
mfxVariant value;

// Filter property for the implementations with HEVC decoder
value.Type     = MFX_VARIANT_TYPE_U32;
value.Data.U32 = MFX_CODEC_HEVC;

MFXSetConfigFilterProperty(hevc_decoder_config
      , (const mfxU8*)"mfxImplDescription.mfxDecoderDescription.decoder.CodecID"
      , value);

// create session from first matched implementation
MFXCreateSession(loader, 0, &hevc_session_handle);
/*end3*/
}

static void prg_session4 () {
/*beg4*/
mfxSession mirror_session_handle;
loader = mfxLoader();

// We want to search for the VPP mirror implementation
mfxConfig  mirror_flt_config = MFXCreateConfig(loader);
mfxVariant value;

// Filter property for the implementations with VPP mirror
value.Type     = MFX_VARIANT_TYPE_U32;
value.Data.U32 = MFX_EXTBUFF_VPP_MIRRORING;

MFXSetConfigFilterProperty(mirror_flt_config
      , (const mfxU8*)"mfxImplDescription.mfxVPPDescription.filter.FilterFourCC"
      , value);

// create session from first matched implementation
MFXCreateSession(loader, 0, &mirror_session_handle);
/*end4*/
}

static void prg_session5 () {
/*beg5*/
mfxHDL h;
mfxSession def_session;

loader = mfxLoader();

// Create session from the first available implementation.
// That's why we no any filters need to be set.
// First available implementation has index equal to the 0.
MFXCreateSession(loader, 0, &def_session);

// Get and print out OS path to the loaded shared library
// with the implementation. It is absolutely OK to call
// MFXEnumImplementations after session creation just need to make
// sure that the same index of implementation is provided to the
// function call.
MFXEnumImplementations(loader, 0, MFX_IMPLCAPS_IMPLPATH, &h);
mfxChar* path = reinterpret_cast<mfxChar*>(h);

// Print out the path
std::cout << "Loaded shared library: " << path << std::endl;

// Release the memory for the string with path.
MFXDispReleaseImplDescription(loader, h);
/*end5*/
}
