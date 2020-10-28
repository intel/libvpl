/*############################################################################
  # Copyright (C) 2017-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <assert.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <list>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "vpl/mfxvideo.h"

#include "linux/mfxloader.h"

namespace MFX {

#if defined(__i386__)
    #define LIBMFXSW "libvplswref32.so.1"
    #define LIBMFXHW "libmfxhw32.so.1"
#elif defined(__x86_64__)
    #define LIBMFXSW "libvplswref64.so.1"
    #define LIBMFXHW "libmfxhw64.so.1"
#else
    #error Unsupported architecture
#endif

#undef FUNCTION
#define FUNCTION(return_value, func_name, formal_param_list, actual_param_list) e##func_name,

enum Function {
    eMFXInit,
    eMFXInitEx,
    eMFXClose,
    eMFXJoinSession,
#include "linux/mfxvideo_functions.h"
    eFunctionsNum,
    eNoMoreFunctions = eFunctionsNum
};

// new functions for API 2.0
enum Function2 {
    eMFXQueryImplsDescription = 0,
    eMFXReleaseImplDescription,
    eMFXMemory_GetSurfaceForVPP,
    eMFXMemory_GetSurfaceForEncode,
    eMFXMemory_GetSurfaceForDecode,
    eMFXInitialize,

    eFunctionsNum2,
};

struct FunctionsTable {
    Function id;
    const char* name;
    mfxVersion version;
};

struct FunctionsTable2 {
    Function2 id;
    const char* name;
    mfxVersion version;
};

#define VERSION(major, minor) \
    {                         \
        { minor, major }      \
    }

#undef FUNCTION
#define FUNCTION(return_value, func_name, formal_param_list, actual_param_list) \
    { e##func_name, #func_name, API_VERSION },

static const FunctionsTable g_mfxFuncTable[] = {
    { eMFXInit, "MFXInit", VERSION(1, 0) },
    { eMFXInitEx, "MFXInitEx", VERSION(1, 14) },
    { eMFXClose, "MFXClose", VERSION(1, 0) },
    { eMFXJoinSession, "MFXJoinSession", VERSION(1, 1) },
#include "linux/mfxvideo_functions.h" // NOLINT(build/include)
    { eNoMoreFunctions }
};

static const FunctionsTable2 g_mfxFuncTable2[] = {
    { eMFXQueryImplsDescription, "MFXQueryImplsDescription", VERSION(0, 2) },
    { eMFXReleaseImplDescription, "MFXReleaseImplDescription", VERSION(0, 2) },
    { eMFXMemory_GetSurfaceForVPP, "MFXMemory_GetSurfaceForVPP", VERSION(0, 2) },
    { eMFXMemory_GetSurfaceForEncode, "MFXMemory_GetSurfaceForEncode", VERSION(0, 2) },
    { eMFXMemory_GetSurfaceForDecode, "MFXMemory_GetSurfaceForDecode", VERSION(0, 2) },
    { eMFXInitialize, "MFXInitialize", VERSION(0, 2) },
};

class LoaderCtx {
public:
    mfxStatus Init(mfxInitParam& par, char* dllName);
    mfxStatus Close();

    inline void* getFunction(Function func) const {
        return m_table[func];
    }

    inline void* getFunction2(Function2 func) const {
        return m_table2[func];
    }

    inline mfxSession getSession() const {
        return m_session;
    }

    inline mfxIMPL getImpl() const {
        return m_implementation;
    }

    inline mfxVersion getVersion() const {
        return m_version;
    }

private:
    std::shared_ptr<void> m_dlh;
    mfxVersion m_version{};
    mfxIMPL m_implementation{};
    mfxSession m_session = nullptr;
    void* m_table[eFunctionsNum]{};
    void* m_table2[eFunctionsNum2]{};
};

std::shared_ptr<void> make_dlopen(const char* filename, int flags) {
    return std::shared_ptr<void>(dlopen(filename, flags), [](void* handle) {
        if (handle)
            dlclose(handle);
    });
}

mfxStatus LoaderCtx::Init(mfxInitParam& par, char* dllName) {
    std::vector<std::string> libs;

    if (dllName) {
        // attempt to load only this DLL, fail if unsuccessful
        std::string libToLoad(dllName);
        libs.emplace_back(libToLoad);
    }
    else if (MFX_IMPL_BASETYPE(par.Implementation) == MFX_IMPL_AUTO ||
             MFX_IMPL_BASETYPE(par.Implementation) == MFX_IMPL_AUTO_ANY) {
        libs.emplace_back(LIBMFXHW);
        libs.emplace_back(MFX_MODULES_DIR "/" LIBMFXHW);
        libs.emplace_back(LIBMFXSW);
        libs.emplace_back(MFX_MODULES_DIR "/" LIBMFXSW);
    }
    else if (par.Implementation & MFX_IMPL_HARDWARE || par.Implementation & MFX_IMPL_HARDWARE_ANY) {
        libs.emplace_back(LIBMFXHW);
        libs.emplace_back(MFX_MODULES_DIR "/" LIBMFXHW);
    }
    else if (par.Implementation & MFX_IMPL_SOFTWARE) {
        libs.emplace_back(LIBMFXSW);
        libs.emplace_back(MFX_MODULES_DIR "/" LIBMFXSW);
    }
    else {
        return MFX_ERR_UNSUPPORTED;
    }

    mfxStatus mfx_res = MFX_ERR_UNSUPPORTED;

    for (auto& lib : libs) {
        std::shared_ptr<void> hdl = make_dlopen(lib.c_str(), RTLD_LOCAL | RTLD_NOW);
        if (hdl) {
            do {
                /* Loading functions table */
                bool wrong_version = false;
                for (int i = 0; i < eFunctionsNum; ++i) {
                    assert(i == g_mfxFuncTable[i].id);
                    m_table[i] = dlsym(hdl.get(), g_mfxFuncTable[i].name);
                    if (!m_table[i] &&
                        ((g_mfxFuncTable[i].version <= par.Version) ||
                         (g_mfxFuncTable[i].version <= mfxVersion(VERSION(1, 14))))) {
                        // this version of dispatcher requires MFXInitEx which appeared
                        // in Media SDK API 1.14
                        wrong_version = true;
                        break;
                    }
                }

                // if version >= 2.0, load these functions as well
                if (par.Version.Major >= 2) {
                    for (int i = 0; i < eFunctionsNum2; ++i) {
                        assert(i == g_mfxFuncTable2[i].id);
                        m_table2[i] = dlsym(hdl.get(), g_mfxFuncTable2[i].name);
                        if (!m_table2[i]) {
                            wrong_version = true;
                            break;
                        }
                    }
                }

                if (wrong_version) {
                    mfx_res = MFX_ERR_UNSUPPORTED;
                    break;
                }

                if (par.Version.Major >= 2) {
                    // for API >= 2.0 call MFXInitialize instead of MFXInitEx
                    mfxInitializationParam initPar2 = {};

                    if (par.Implementation & MFX_IMPL_SOFTWARE) {
                        // software
                        initPar2.AccelerationMode = MFX_ACCEL_MODE_NA;
                    }
                    else {
                        // hardware - VAAPI
                        initPar2.AccelerationMode = MFX_ACCEL_MODE_VIA_VAAPI;
                    }

                    mfx_res =
                        ((decltype(MFXInitialize)*)m_table2[eMFXInitialize])(initPar2, &m_session);
                }
                else {
                    /* Initializing loaded library */
                    mfx_res = ((decltype(MFXInitEx)*)m_table[eMFXInitEx])(par, &m_session);
                }

                if (MFX_ERR_NONE != mfx_res) {
                    break;
                }

                // Below we just get some data and double check that we got what we have expected
                // to get. Some of these checks are done inside mediasdk init function
                mfx_res =
                    ((decltype(MFXQueryVersion)*)m_table[eMFXQueryVersion])(m_session, &m_version);
                if (MFX_ERR_NONE != mfx_res) {
                    break;
                }

                if (m_version < par.Version) {
                    mfx_res = MFX_ERR_UNSUPPORTED;
                    break;
                }

                mfx_res =
                    ((decltype(MFXQueryIMPL)*)m_table[eMFXQueryIMPL])(m_session, &m_implementation);
                if (MFX_ERR_NONE != mfx_res) {
                    mfx_res = MFX_ERR_UNSUPPORTED;
                    break;
                }
            } while (false);

            if (MFX_ERR_NONE == mfx_res) {
                m_dlh = std::move(hdl);
                break;
            }
            else {
                Close();
            }
        }
    }

    return mfx_res;
}

mfxStatus LoaderCtx::Close() {
    auto proc         = (decltype(MFXClose)*)m_table[eMFXClose];
    mfxStatus mfx_res = (proc) ? (*proc)(m_session) : MFX_ERR_NONE;

    m_implementation = {};
    m_version        = {};
    m_session        = nullptr;
    std::fill(std::begin(m_table), std::end(m_table), nullptr);
    return mfx_res;
}

} // namespace MFX

// internal function - load a specific DLL, return unsupported if it fails
mfxStatus MFXInitEx2(mfxInitParam par, mfxSession* session, char* dllName) {
    if (!session)
        return MFX_ERR_NULL_PTR;

    try {
        std::unique_ptr<MFX::LoaderCtx> loader;

        loader.reset(new MFX::LoaderCtx{});

        mfxStatus mfx_res = loader->Init(par, dllName);
        if (MFX_ERR_NONE == mfx_res) {
            *session = (mfxSession)loader.release();
        }
        else {
            *session = nullptr;
        }

        return mfx_res;
    }
    catch (...) {
        return MFX_ERR_MEMORY_ALLOC;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

mfxStatus MFXInit(mfxIMPL impl, mfxVersion* ver, mfxSession* session) {
    mfxInitParam par{};

    par.Implementation = impl;
    if (ver) {
        par.Version = *ver;
    }
    else {
        par.Version = VERSION(MFX_VERSION_MAJOR, MFX_VERSION_MINOR);
    }

    return MFXInitEx(par, session);
}

mfxStatus MFXInitEx(mfxInitParam par, mfxSession* session) {
    if (!session)
        return MFX_ERR_NULL_PTR;

    try {
        std::unique_ptr<MFX::LoaderCtx> loader;

        loader.reset(new MFX::LoaderCtx{});

        mfxStatus mfx_res = loader->Init(par, nullptr);
        if (MFX_ERR_NONE == mfx_res) {
            *session = (mfxSession)loader.release();
        }
        else {
            *session = nullptr;
        }

        return mfx_res;
    }
    catch (...) {
        return MFX_ERR_MEMORY_ALLOC;
    }
}

mfxStatus MFXClose(mfxSession session) {
    if (!session)
        return MFX_ERR_INVALID_HANDLE;

    try {
        std::unique_ptr<MFX::LoaderCtx> loader((MFX::LoaderCtx*)session);
        mfxStatus mfx_res = loader->Close();

        if (mfx_res == MFX_ERR_UNDEFINED_BEHAVIOR) {
            // It is possible, that there is an active child session.
            // Can't unload library in this case.
            loader.release();
        }
        return mfx_res;
    }
    catch (...) {
        return MFX_ERR_MEMORY_ALLOC;
    }
}

// passthrough functions to implementation
mfxStatus MFXMemory_GetSurfaceForVPP(mfxSession session, mfxFrameSurface1** surface) {
    if (!session)
        return MFX_ERR_INVALID_HANDLE;
    if (!surface)
        return MFX_ERR_NULL_PTR;

    MFX::LoaderCtx* loader = (MFX::LoaderCtx*)session;

    auto proc = (decltype(MFXMemory_GetSurfaceForVPP)*)loader->getFunction2(
        MFX::eMFXMemory_GetSurfaceForVPP);
    if (!proc) {
        return MFX_ERR_INVALID_HANDLE;
    }

    return (*proc)(loader->getSession(), surface);
}

mfxStatus MFXMemory_GetSurfaceForEncode(mfxSession session, mfxFrameSurface1** surface) {
    if (!session)
        return MFX_ERR_INVALID_HANDLE;
    if (!surface)
        return MFX_ERR_NULL_PTR;

    MFX::LoaderCtx* loader = (MFX::LoaderCtx*)session;

    auto proc = (decltype(MFXMemory_GetSurfaceForEncode)*)loader->getFunction2(
        MFX::eMFXMemory_GetSurfaceForEncode);
    if (!proc) {
        return MFX_ERR_INVALID_HANDLE;
    }

    return (*proc)(loader->getSession(), surface);
}

mfxStatus MFXMemory_GetSurfaceForDecode(mfxSession session, mfxFrameSurface1** surface) {
    if (!session)
        return MFX_ERR_INVALID_HANDLE;
    if (!surface)
        return MFX_ERR_NULL_PTR;

    MFX::LoaderCtx* loader = (MFX::LoaderCtx*)session;

    auto proc = (decltype(MFXMemory_GetSurfaceForDecode)*)loader->getFunction2(
        MFX::eMFXMemory_GetSurfaceForDecode);
    if (!proc) {
        return MFX_ERR_INVALID_HANDLE;
    }

    return (*proc)(loader->getSession(), surface);
}

mfxStatus MFXJoinSession(mfxSession session, mfxSession child_session) {
    if (!session || !child_session) {
        return MFX_ERR_INVALID_HANDLE;
    }

    MFX::LoaderCtx* loader       = (MFX::LoaderCtx*)session;
    MFX::LoaderCtx* child_loader = (MFX::LoaderCtx*)child_session;

    if (loader->getVersion().Version != child_loader->getVersion().Version) {
        return MFX_ERR_INVALID_HANDLE;
    }

    auto proc = (decltype(MFXJoinSession)*)loader->getFunction(MFX::eMFXJoinSession);
    if (!proc) {
        return MFX_ERR_INVALID_HANDLE;
    }

    return (*proc)(loader->getSession(), child_loader->getSession());
}

mfxStatus MFXCloneSession(mfxSession session, mfxSession* clone) {
    if (!session)
        return MFX_ERR_INVALID_HANDLE;

    MFX::LoaderCtx* loader = (MFX::LoaderCtx*)session;
    // initialize the clone session
    mfxVersion version = loader->getVersion();
    mfxStatus mfx_res  = MFXInit(loader->getImpl(), &version, clone);
    if (MFX_ERR_NONE != mfx_res) {
        return mfx_res;
    }

    // join the sessions
    mfx_res = MFXJoinSession(session, *clone);
    if (MFX_ERR_NONE != mfx_res) {
        MFXClose(*clone);
        *clone = nullptr;
        return mfx_res;
    }

    return MFX_ERR_NONE;
}

#undef FUNCTION
#define FUNCTION(return_value, func_name, formal_param_list, actual_param_list)   \
    return_value MFX_CDECL func_name formal_param_list {                          \
        /* get the function's address and make a call */                          \
        if (!session)                                                             \
            return MFX_ERR_INVALID_HANDLE;                                        \
                                                                                  \
        MFX::LoaderCtx* loader = (MFX::LoaderCtx*)session;                        \
                                                                                  \
        auto proc = (decltype(func_name)*)loader->getFunction(MFX::e##func_name); \
        if (!proc)                                                                \
            return MFX_ERR_INVALID_HANDLE;                                        \
                                                                                  \
        /* get the real session pointer */                                        \
        session = loader->getSession();                                           \
        /* pass down the call */                                                  \
        return (*proc)actual_param_list;                                          \
    }

#include "linux/mfxvideo_functions.h" // NOLINT(build/include)

#ifdef __cplusplus
}
#endif
