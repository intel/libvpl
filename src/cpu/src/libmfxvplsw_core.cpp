/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"
#include "vpl/mfxvideo.h"

// SetFrameAllocator not implemented in CPU reference implementation
mfxStatus MFXVideoCORE_SetFrameAllocator(mfxSession session,
                                         mfxFrameAllocator *allocator) {
    RET_IF_FALSE(session, MFX_ERR_INVALID_HANDLE);
    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    return ws->SetFrameAllocator(allocator);
}

// SetHandle accepts a handle.  This holds a void* which is
// not used by the CPU reference implementation.
mfxStatus MFXVideoCORE_SetHandle(mfxSession session,
                                 mfxHandleType type,
                                 mfxHDL hdl) {
    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);

    //Handle should not be redefined.  If the handle is null,
    //set it.  Otherwise return UNDEFINED_BEHAVIOR.
    if (!ws->GetHandle(type)) {
        ws->SetHandle(type, hdl);
        return MFX_ERR_NONE;
    }
    else {
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }
}

// Return handle set by SetHandle
mfxStatus MFXVideoCORE_GetHandle(mfxSession session,
                                 mfxHandleType type,
                                 mfxHDL *hdl) {
    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    hdl               = ws->GetHandle(type);
    return MFX_ERR_NONE;
}

// QueryPlatform not implemented in CPU reference implementation
mfxStatus MFXVideoCORE_QueryPlatform(mfxSession session,
                                     mfxPlatform *platform) {
    return MFX_ERR_NOT_IMPLEMENTED;
}

// Sync operation is implemented as a simple passthrough
// until asynchronous operation is added
mfxStatus MFXVideoCORE_SyncOperation(mfxSession session,
                                     mfxSyncPoint syncp,
                                     mfxU32 wait) {
    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);
    return ws->Sync(syncp, wait);
}
