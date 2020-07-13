/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "./cpu_workstream.h"
#include "vpl/mfxvideo.h"

#include "vpl/mfxdispatcher.h"

mfxStatus MFXInit(mfxIMPL implParam, mfxVersion *ver, mfxSession *session) {
    mfxInitParam par = {};

    par.Implementation = implParam;
    if (ver) {
        par.Version = *ver;
    }
    else {
        par.Version.Major = MFX_VERSION_MAJOR;
        par.Version.Minor = MFX_VERSION_MINOR;
    }
    par.ExternalThreads = 0;

    return MFXInitEx(par, session);
}

mfxStatus MFXInitEx(mfxInitParam par, mfxSession *session) {
    mfxIMPL impl = par.Implementation & (MFX_IMPL_VIA_ANY - 1);

    // check the library version
    if ((MFX_VERSION_MAJOR < par.Version.Major) ||
        (MFX_VERSION_MAJOR == par.Version.Major &&
         MFX_VERSION_MINOR < par.Version.Minor)) {
        return MFX_ERR_UNSUPPORTED;
    }

    // SW plugin only
    if ((MFX_IMPL_AUTO != impl) && (MFX_IMPL_AUTO_ANY != impl) &&
        (MFX_IMPL_SOFTWARE != impl)) {
        return MFX_ERR_UNSUPPORTED;
    }

    // create CPU workstream
    CpuWorkstream *ws = new CpuWorkstream;

    if (!ws) {
        return MFX_ERR_UNSUPPORTED;
    }

    // save the handle
    *session = (mfxSession)(ws);

    return MFX_ERR_NONE;
}

mfxStatus MFXClose(mfxSession session) {
    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }

    CpuWorkstream *ws = reinterpret_cast<CpuWorkstream *>(session);

    delete ws;
    ws = nullptr;

    return MFX_ERR_NONE;
}

mfxStatus MFXQueryIMPL(mfxSession session, mfxIMPL *impl) {
    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == impl) {
        return MFX_ERR_NULL_PTR;
    }

    *impl = MFX_IMPL_SOFTWARE;

    return MFX_ERR_NONE;
}

mfxStatus MFXQueryVersion(mfxSession session, mfxVersion *pVersion) {
    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == pVersion) {
        return MFX_ERR_NULL_PTR;
    }

    // set the library's version
    pVersion->Major = MFX_VERSION_MAJOR;
    pVersion->Minor = MFX_VERSION_MINOR;

    return MFX_ERR_NONE;
}

// stubs
mfxStatus MFXJoinSession(mfxSession session, mfxSession child) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXDisjoinSession(mfxSession session) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXCloneSession(mfxSession session, mfxSession *clone) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXSetPriority(mfxSession session, mfxPriority priority) {
    return MFX_ERR_UNSUPPORTED;
}
mfxStatus MFXGetPriority(mfxSession session, mfxPriority *priority) {
    return MFX_ERR_UNSUPPORTED;
}

// DLL entry point

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID lpReserved) {
    return TRUE;
} // BOOL APIENTRY DllMain(HMODULE hModule,
#else // #if defined(_WIN32) || defined(_WIN64)
void __attribute__((constructor)) dll_init(void) {}
#endif // #if defined(_WIN32) || defined(_WIN64)
