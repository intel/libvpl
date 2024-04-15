/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef LIBVPL_SRC_MFX_CONFIG_INTERFACE_MFX_CONFIG_INTERFACE_H_
#define LIBVPL_SRC_MFX_CONFIG_INTERFACE_MFX_CONFIG_INTERFACE_H_

#include <algorithm>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "vpl/mfxvideo.h"

// special key to indicate Context is not a real context, as it is a stateless interface
#define MFX_CONFIG_INTERFACE_CONTEXT ((mfxHDL)(-1))

namespace MFX_CONFIG_INTERFACE {

// dispatcher returns this interface from call to MFXVideoCORE_GetHandle(type = MFX_HANDLE_CONFIG_INTERFACE)
extern const mfxConfigInterface g_dispatcher_mfxConfigInterface;

// string K-V pairs, each key may only have a single value
typedef std::pair<std::string, std::string> KVPair;

mfxStatus MFX_CDECL ExtSetParameter(struct mfxConfigInterface *config_interface,
                                    const mfxU8 *key,
                                    const mfxU8 *value,
                                    mfxStructureType struct_type,
                                    mfxHDL structure,
                                    mfxExtBuffer *ext_buffer);

mfxStatus SetParameter(const mfxU8 *key, const mfxU8 *value, mfxVideoParam *videoParam, mfxExtBuffer *extBuf);

mfxStatus UpdateVideoParam(const KVPair &kvStr, mfxVideoParam *videoParam);
mfxStatus UpdateExtBufParam(const KVPair &kvStr, mfxVideoParam *videoParam, mfxExtBuffer *extBufRequired);
bool IsExtBuf(const KVPair &kvStr);

mfxStatus ValidateKVPair(const mfxU8 *key, const mfxU8 *value, KVPair &kvStr);
mfxStatus SetExtBufParam(mfxExtBuffer *extBufActual, KVPair &kvStrParsed);
mfxStatus GetExtBufType(const KVPair &kvStr, mfxExtBuffer *extBufHeader, KVPair &kvStrParsed);

}; // namespace MFX_CONFIG_INTERFACE

#endif // LIBVPL_SRC_MFX_CONFIG_INTERFACE_MFX_CONFIG_INTERFACE_H_
