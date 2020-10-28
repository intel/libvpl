/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "vpl/mfx_dispatcher_vpl.h"

// exported functions for API >= 2.0

// create unique loader context
mfxLoader MFXLoad() {
    LoaderCtxVPL* loaderCtx;

    try {
        std::unique_ptr<LoaderCtxVPL> pLoaderCtx;
        pLoaderCtx.reset(new LoaderCtxVPL{});
        loaderCtx = (LoaderCtxVPL*)pLoaderCtx.release();
    }
    catch (...) {
        return nullptr;
    }

    // search directories for candidate implementations based on search order in
    // spec
    mfxStatus sts = loaderCtx->BuildListOfCandidateLibs();
    if (MFX_ERR_NONE != sts) {
        return nullptr;
    }

    // prune libraries which are not actually implementations, filling function
    // ptr table for each library which is
    mfxU32 numLibs = loaderCtx->CheckValidLibraries();
    if (numLibs == 0) {
        return nullptr;
    }

    // query capabilities of each implementation
    // may be more than one implementation per library
    sts = loaderCtx->QueryLibraryCaps();

    return (mfxLoader)loaderCtx;
}

// unload libraries, destroy all created mfxConfig objects, free other memory
void MFXUnload(mfxLoader loader) {
    if (loader) {
        LoaderCtxVPL* loaderCtx = (LoaderCtxVPL*)loader;

        loaderCtx->UnloadAllLibraries();

        loaderCtx->FreeConfigFilters();

        delete loaderCtx;
    }

    return;
}

// create config context
// each loader may have more than one config context
mfxConfig MFXCreateConfig(mfxLoader loader) {
    if (!loader)
        return nullptr;

    LoaderCtxVPL* loaderCtx = (LoaderCtxVPL*)loader;
    ConfigCtxVPL* configCtx;

    try {
        std::unique_ptr<ConfigCtxVPL> pConfigCtx;
        pConfigCtx.reset(new ConfigCtxVPL{});
        configCtx = loaderCtx->AddConfigFilter();
    }
    catch (...) {
        return nullptr;
    }

    return (mfxConfig)(configCtx);
}

// set a config proprerty to use in enumerating implementations
// each config context may have only one property
mfxStatus MFXSetConfigFilterProperty(mfxConfig config, const mfxU8* name, mfxVariant value) {
    if (!config)
        return MFX_ERR_NULL_PTR;

    ConfigCtxVPL* configCtx = (ConfigCtxVPL*)config;

    mfxStatus sts = configCtx->SetFilterProperty(name, value);

    // update list of valid libraries based on updated set of
    //   mfxConfig properties
    LoaderCtxVPL* loaderCtx = configCtx->m_parentLoader;
    loaderCtx->UpdateValidImplList();

    return sts;
}

// iterate over available implementations
// capabilities are returned in idesc
mfxStatus MFXEnumImplementations(mfxLoader loader,
                                 mfxU32 i,
                                 mfxImplCapsDeliveryFormat format,
                                 mfxHDL* idesc) {
    if (!loader || !idesc)
        return MFX_ERR_NULL_PTR;

    LoaderCtxVPL* loaderCtx = (LoaderCtxVPL*)loader;

    mfxStatus sts = loaderCtx->QueryImpl(i, format, idesc);

    return sts;
}

// create a new session with implementation i
mfxStatus MFXCreateSession(mfxLoader loader, mfxU32 i, mfxSession* session) {
    if (!loader || !session)
        return MFX_ERR_NULL_PTR;

    LoaderCtxVPL* loaderCtx = (LoaderCtxVPL*)loader;

    mfxStatus sts = loaderCtx->CreateSession(i, session);

    return sts;
}

// release memory associated with implementation description hdl
mfxStatus MFXDispReleaseImplDescription(mfxLoader loader, mfxHDL hdl) {
    if (!loader)
        return MFX_ERR_NULL_PTR;

    LoaderCtxVPL* loaderCtx = (LoaderCtxVPL*)loader;

    mfxStatus sts = loaderCtx->ReleaseImpl(hdl);

    return sts;
}
