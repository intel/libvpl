/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/mfx_config_interface/mfx_config_interface.h"

namespace MFX_CONFIG_INTERFACE {

// leave table formatting alone
// clang-format off

// Dispatcher-level implementation of mfxConfigInterface.
// Current implementation is stateless (string API only) so set Context to a special key.
// We could replace Context with pointer to mfxSession (or some object within mfxSession)
//   if future extensions require access to the session state.
// The application is not permitted to modify or dereference Context (see struct definition)
//   so we can set this to whatever we need.
const mfxConfigInterface g_dispatcher_mfxConfigInterface = {
    MFX_CONFIG_INTERFACE_CONTEXT,               // Context
    { { 0, 1 } },                               // Version

    MFX_CONFIG_INTERFACE::ExtSetParameter,      // SetParameter (callback function)

    {},                                         // reserved
};

// end table formatting
// clang-format on

// callback function - set mfxConfigInterface::SetParameter to this
// this has to be static since it's returned as function pointer, so cast session to _mfxSession
// to access session-specific state (current implementation is stateless)
mfxStatus ExtSetParameter(struct mfxConfigInterface *config_interface,
                          const mfxU8 *key,
                          const mfxU8 *value,
                          mfxStructureType struct_type,
                          mfxHDL structure,
                          mfxExtBuffer *ext_buffer) {
    if (struct_type == MFX_STRUCTURE_TYPE_VIDEO_PARAM) {
        return SetParameter(key, value, (mfxVideoParam *)structure, ext_buffer);
    }

    return MFX_ERR_UNSUPPORTED;
}

// validate key and value input strings
mfxStatus ValidateKVPair(const mfxU8 *key, const mfxU8 *value, KVPair &kvStr) {
    mfxU32 lengthKey, lengthValue;

    // manually check length of key string
    for (lengthKey = 0; lengthKey < MAX_PARAM_STRING_LENGTH; lengthKey++) {
        if (!key[lengthKey])
            break;
    }
    if (lengthKey == 0 || lengthKey == MAX_PARAM_STRING_LENGTH)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    // manually check length of value string
    for (lengthValue = 0; lengthValue < MAX_PARAM_STRING_LENGTH; lengthValue++) {
        if (!value[lengthValue])
            break;
    }
    if (lengthValue == 0 || lengthValue == MAX_PARAM_STRING_LENGTH)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    kvStr.first  = (const char *)key;
    kvStr.second = (const char *)value;

    return MFX_ERR_NONE;
}

mfxStatus SetParameter(const mfxU8 *key, const mfxU8 *value, mfxVideoParam *videoParam, mfxExtBuffer *extBuf) {
    if (!key || !value || !videoParam || !extBuf)
        return MFX_ERR_NULL_PTR;

    *extBuf = {}; // clear extBuf, will be filled in if new extBuf is required from caller

    mfxStatus sts = MFX_ERR_NOT_FOUND; // default if no valid parameters

    // validate C-style key and value strings
    // output is a pair of std::string's in kvStr
    KVPair kvStr;
    sts = ValidateKVPair(key, value, kvStr);
    if (sts != MFX_ERR_NONE)
        return sts;

    if (IsExtBuf(kvStr)) {
        mfxExtBuffer extBufRequired = {};

        // key maps to a parameter in an extension buffer
        sts = UpdateExtBufParam(kvStr, videoParam, &extBufRequired);

        // if required extension buffer is not attached, we request the app to allocate it
        // and then call the SetParameter function again
        if (sts == MFX_ERR_MORE_EXTBUFFER) {
            extBuf->BufferId = extBufRequired.BufferId;
            extBuf->BufferSz = extBufRequired.BufferSz;
        }

        return sts;
    }

    // key maps to a parameter in mfxVideoParam
    sts = UpdateVideoParam(kvStr, videoParam);
    return sts;
}

} // namespace MFX_CONFIG_INTERFACE
