/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <algorithm>

#include "vpl/mfx_dispatcher_vpl.h"

// new functions for API >= 2.0
static const VPLFunctionDesc FunctionDesc2[NumVPLFunctions] = {
    { "MFXQueryImplsDescription", { { 0, 2 } } },
    { "MFXReleaseImplDescription", { { 0, 2 } } },
    { "MFXMemory_GetSurfaceForVPP", { { 0, 2 } } },
    { "MFXMemory_GetSurfaceForEncode", { { 0, 2 } } },
    { "MFXMemory_GetSurfaceForDecode", { { 0, 2 } } },
};

// implementation of loader context (mfxLoader)
// each loader instance will build a list of valid runtimes and allow
// application to create sessions with them
LoaderCtxVPL::LoaderCtxVPL()
        : m_libInfoList(),
          m_implInfoList(),
          m_configCtxList(),
          m_implIdxNext(0),
          m_userSearchDirs(),
          m_packageSearchDirs(),
          m_vplPackageDir() {
    return;
}

LoaderCtxVPL::~LoaderCtxVPL() {
    return;
}

// creates ordered list of user-specified directories to search
mfxU32 LoaderCtxVPL::ParseEnvSearchPaths(const CHAR_TYPE* envVarName,
                                         std::list<STRING_TYPE>& searchDirs) {
    searchDirs.clear();

#if defined(_WIN32) || defined(_WIN64)
    DWORD err;
    CHAR_TYPE envVar[MAX_VPL_SEARCH_PATH] = { L"" };
    err = GetEnvironmentVariableW(envVarName, envVar, MAX_VPL_SEARCH_PATH);
    if (!err)
        return 0; // environment variable not defined

    // parse env variable into individual directories
    std::wstringstream envPath((CHAR_TYPE*)envVar);
    STRING_TYPE s;
    while (std::getline(envPath, s, L';')) {
        searchDirs.push_back(s);
    }
#else
    CHAR_TYPE* envVar = getenv(envVarName);
    if (!envVar)
        return 0; // environment variable not defined

    // parse env variable into individual directories
    std::stringstream envPath((CHAR_TYPE*)envVar);
    STRING_TYPE s;
    while (std::getline(envPath, s, ':')) {
        searchDirs.push_back(s);
    }
#endif

    return (mfxU32)searchDirs.size();
}

mfxStatus LoaderCtxVPL::SearchDirForLibs(STRING_TYPE searchDir,
                                         std::list<LibInfo*>& libInfoList,
                                         mfxU32 priority) {
    // okay to call with empty searchDir
    if (searchDir.empty())
        return MFX_ERR_NONE;

#if defined(_WIN32) || defined(_WIN64)
    HANDLE hTestFile = nullptr;
    WIN32_FIND_DATAW testFileData;
    DWORD err;
    STRING_TYPE testFileName = searchDir + MAKE_STRING("/*.dll");

    CHAR_TYPE currDir[MAX_VPL_SEARCH_PATH] = L"";
    if (GetCurrentDirectoryW(MAX_VPL_SEARCH_PATH, currDir))
        SetCurrentDirectoryW(searchDir.c_str());

    // iterate over all candidate files in directory
    hTestFile = FindFirstFileW(testFileName.c_str(), &testFileData);
    if (hTestFile != INVALID_HANDLE_VALUE) {
        do {
            wchar_t libNameFull[MAX_VPL_SEARCH_PATH];
            wchar_t* libNameBase;

            LibInfo* libInfo = new LibInfo;
            if (!libInfo)
                return MFX_ERR_MEMORY_ALLOC;

            err = GetFullPathNameW(testFileData.cFileName,
                                   MAX_VPL_SEARCH_PATH,
                                   libNameFull,
                                   &libNameBase);
            if (!err) {
                // unknown error - skip it and move on to next file
                delete libInfo;
                continue;
            }
            libInfo->libNameFull = libNameFull;
            libInfo->libPriority = priority;

            // add to list
            libInfoList.push_back(libInfo);
        } while (FindNextFileW(hTestFile, &testFileData));

        FindClose(hTestFile);
    }

    // restore current directory
    if (currDir[0])
        SetCurrentDirectoryW(currDir);

#else
    DIR* pSearchDir;
    struct dirent* currFile;

    pSearchDir = opendir(searchDir.c_str());
    if (pSearchDir) {
        while (1) {
            currFile = readdir(pSearchDir);
            if (!currFile)
                break;

            // save files with ".so" (including .so.1, etc.)
            if (strstr(currFile->d_name, ".so")) {
                char filePathC[MAX_VPL_SEARCH_PATH];

                LibInfo* libInfo = new LibInfo;
                if (!libInfo)
                    return MFX_ERR_MEMORY_ALLOC;

                // get full path to found library
                snprintf(filePathC,
                         MAX_VPL_SEARCH_PATH,
                         "%s/%s",
                         searchDir.c_str(),
                         currFile->d_name);
                char* fullPath = realpath(filePathC, NULL);
                if (!fullPath) {
                    // unknown error - skip it and move on to next file
                    delete libInfo;
                    continue;
                }

                libInfo->libNameFull = fullPath;
                libInfo->libPriority = priority;
                free(fullPath);

                // add to list
                libInfoList.push_back(libInfo);
            }
        }
    }
#endif

    return MFX_ERR_NONE;
}

// search for implementations of oneAPI Video Processing Library (oneVPL)
//   according to the rules in the spec
mfxStatus LoaderCtxVPL::BuildListOfCandidateLibs() {
    mfxStatus sts = MFX_ERR_NONE;

    STRING_TYPE emptyPath; // default construction = empty
    std::list<STRING_TYPE>::iterator it;

    // first priority: user-defined directories in environment variable
    ParseEnvSearchPaths(ENV_ONEVPL_SEARCH_PATH, m_userSearchDirs);
    it = m_userSearchDirs.begin();
    while (it != m_userSearchDirs.end()) {
        STRING_TYPE nextDir = (*it);
        sts =
            SearchDirForLibs(nextDir, m_libInfoList, LIB_PRIORITY_USER_DEFINED);
        it++;
    }

    // second priority: oneVPL package
    ParseEnvSearchPaths(ENV_ONEVPL_PACKAGE_PATH, m_packageSearchDirs);
    it = m_packageSearchDirs.begin();
    while (it != m_packageSearchDirs.end()) {
        STRING_TYPE nextDir = (*it);
        sts =
            SearchDirForLibs(nextDir, m_libInfoList, LIB_PRIORITY_VPL_PACKAGE);
        it++;
    }

    // third priority: OS-specific PATH / LD_LIBRARY_PATH
    ParseEnvSearchPaths(ENV_OS_PATH, m_pathSearchDirs);
    it = m_pathSearchDirs.begin();
    while (it != m_pathSearchDirs.end()) {
        STRING_TYPE nextDir = (*it);
        sts = SearchDirForLibs(nextDir, m_libInfoList, LIB_PRIORITY_OS_PATH);
        it++;
    }

    // fourth priority: default system folders (current location for now)
    m_vplPackageDir = MAKE_STRING("./");
    sts             = SearchDirForLibs(m_vplPackageDir,
                           m_libInfoList,
                           LIB_PRIORITY_SYS_DEFAULT);

    // fifth priority: standalone MSDK/driver installation
    sts = SearchDirForLibs(emptyPath, m_libInfoList, LIB_PRIORITY_MSDK_PACKAGE);

    return sts;
}

// return number of valid libraries found
mfxU32 LoaderCtxVPL::CheckValidLibraries() {
    // load all libraries
    std::list<LibInfo*>::iterator it = m_libInfoList.begin();
    while (it != m_libInfoList.end()) {
        mfxU32 i         = 0;
        LibInfo* libInfo = (*it);

#if defined(_WIN32) || defined(_WIN64)
        // load DLL
        libInfo->hModuleVPL = MFX::mfx_dll_load(libInfo->libNameFull.c_str());

        // load video functions: pointers to exposed functions
        if (libInfo->hModuleVPL) {
            for (i = 0; i < NumVPLFunctions; i += 1) {
                VPLFunctionPtr pProc = (VPLFunctionPtr)MFX::mfx_dll_get_addr(
                    libInfo->hModuleVPL,
                    FunctionDesc2[i].pName);
                if (pProc) {
                    libInfo->vplFuncTable[i] = pProc;
                }
                else {
                    MFX::mfx_dll_free(libInfo->hModuleVPL);
                    break;
                }
            }
        }
#else
        // Linux
        libInfo->hModuleVPL =
            dlopen(libInfo->libNameFull.c_str(), RTLD_LOCAL | RTLD_NOW);

        if (libInfo->hModuleVPL) {
            for (i = 0; i < NumVPLFunctions; i += 1) {
                VPLFunctionPtr pProc =
                    (VPLFunctionPtr)dlsym(libInfo->hModuleVPL,
                                          FunctionDesc2[i].pName);

                if (pProc) {
                    libInfo->vplFuncTable[i] = pProc;
                }
                else {
                    dlclose(libInfo->hModuleVPL);
                    break;
                }
            }
        }
#endif
        if (i == NumVPLFunctions) {
            it++;
        }
        else {
            // required function is missing from DLL
            // remove this library from the list of options
            delete libInfo;
            it = m_libInfoList.erase(it);
        }
    }

    // number of valid oneVPL libs
    return (mfxU32)m_libInfoList.size();
}

// iterate over all implementation runtimes
// unload DLL's and free memory
mfxStatus LoaderCtxVPL::UnloadAllLibraries() {
    std::list<LibInfo*>::iterator it = m_libInfoList.begin();
    while (it != m_libInfoList.end()) {
        LibInfo* libInfo = (*it);

        if (libInfo) {
#if defined(_WIN32) || defined(_WIN64)
            MFX::mfx_dll_free(libInfo->hModuleVPL);
#else
            dlclose(libInfo->hModuleVPL);
#endif
            delete libInfo;
        }
        it++;
    }

    std::list<ImplInfo*>::iterator it2 = m_implInfoList.begin();
    while (it2 != m_implInfoList.end()) {
        ImplInfo* implInfo = (*it2);

        if (implInfo) {
            delete implInfo;
        }
        it2++;
    }

    return MFX_ERR_NONE;
}

// query capabilities of all valid libraries
//   and add to list for future calls to EnumImplementations()
//   as well as filtering by functionality
// assume MFX_IMPLCAPS_IMPLDESCSTRUCTURE is the only format supported
mfxStatus LoaderCtxVPL::QueryLibraryCaps() {
    std::list<LibInfo*>::iterator it = m_libInfoList.begin();
    while (it != m_libInfoList.end()) {
        mfxU32 num_impls = 0;
        LibInfo* libInfo = (*it);

        VPLFunctionPtr pFunc =
            libInfo->vplFuncTable[IdxMFXQueryImplsDescription];

        // call MFXQueryImplsDescription() for this implementation
        // return handle to description in requested format
        mfxHDL* hImpl;
        hImpl = (*(mfxHDL * (MFX_CDECL*)(mfxImplCapsDeliveryFormat, mfxU32*))
                     pFunc)(MFX_IMPLCAPS_IMPLDESCSTRUCTURE, &num_impls);

        if (!hImpl)
            return MFX_ERR_UNSUPPORTED;

        for (mfxU32 i = 0; i < num_impls; i++) {
            ImplInfo* implInfo = new ImplInfo;
            if (!implInfo)
                return MFX_ERR_MEMORY_ALLOC;

            // library which contains this implementation
            implInfo->libInfo = libInfo;

            // implementation descriptor returned from runtime
            implInfo->implDesc = hImpl[i];

            // fill out mfxInitParam struct for when we call MFXInitEx
            //   in CreateSession()
            mfxImplDescription* implDesc =
                reinterpret_cast<mfxImplDescription*>(hImpl[i]);

            memset(&(implInfo->initPar), 0, sizeof(mfxInitParam));
            implInfo->initPar.Version        = implDesc->ApiVersion;
            implInfo->initPar.Implementation = implDesc->Impl;

            // save local index for this library
            implInfo->libImplIdx = i;

            // assign unique index to each implementation
            implInfo->vplImplIdx = m_implIdxNext++;

            // add implementation to overall list
            m_implInfoList.push_back(implInfo);
        }
        it++;
    }

    return MFX_ERR_NONE;
}

// query implementation i
mfxStatus LoaderCtxVPL::QueryImpl(mfxU32 idx,
                                  mfxImplCapsDeliveryFormat format,
                                  mfxHDL* idesc) {
    if (format != MFX_IMPLCAPS_IMPLDESCSTRUCTURE)
        return MFX_ERR_UNSUPPORTED;

    std::list<ImplInfo*>::iterator it = m_implInfoList.begin();
    while (it != m_implInfoList.end()) {
        ImplInfo* implInfo = (*it);

        if (implInfo->vplImplIdx == idx) {
            *idesc = implInfo->implDesc;
            return MFX_ERR_NONE;
        }
        it++;
    }

    // invalid idx
    return MFX_ERR_NOT_FOUND;
}

mfxStatus LoaderCtxVPL::ReleaseImpl(mfxHDL idesc) {
    mfxStatus sts = MFX_ERR_NONE;

    if (idesc == nullptr)
        return MFX_ERR_NULL_PTR;

    // all we get from the application is a handle to the descriptor,
    //   not the implementation associated with it, so we search
    //   through the full list until we find a match
    std::list<ImplInfo*>::iterator it = m_implInfoList.begin();
    while (it != m_implInfoList.end()) {
        ImplInfo* implInfo = (*it);

        if (implInfo->implDesc == idesc) {
            LibInfo* libInfo = implInfo->libInfo;

            VPLFunctionPtr pFunc =
                libInfo->vplFuncTable[IdxMFXReleaseImplDescription];

            // call MFXReleaseImplDescription() for this implementation
            sts = (*(mfxStatus(MFX_CDECL*)(mfxHDL))pFunc)(implInfo->implDesc);

            implInfo->implDesc = nullptr; // no longer valid

            return MFX_ERR_NONE;
        }
        it++;
    }

    // did not find a matching handle - should not happen
    return MFX_ERR_INVALID_HANDLE;
}

mfxStatus LoaderCtxVPL::CreateSession(mfxU32 idx, mfxSession* session) {
    mfxStatus sts = MFX_ERR_NONE;

    // find library with given implementation index
    std::list<ImplInfo*>::iterator it = m_implInfoList.begin();
    while (it != m_implInfoList.end()) {
        ImplInfo* implInfo = (*it);

        if (implInfo->vplImplIdx == idx) {
            LibInfo* libInfo = implInfo->libInfo;

            // compare caps from this library vs. config filters
            sts = ConfigCtxVPL::ValidateConfig(
                (mfxImplDescription*)implInfo->implDesc,
                m_configCtxList);

            if (sts == MFX_ERR_NONE) {
                // initialize this library via MFXInitEx, or else fail
                //   (specify full path to library)
                sts = MFXInitEx2(implInfo->initPar,
                                 session,
                                 (CHAR_TYPE*)libInfo->libNameFull.c_str());
            }

            return sts;
        }
        it++;
    }

    // invalid idx
    return MFX_ERR_NOT_FOUND;
}

ConfigCtxVPL* LoaderCtxVPL::AddConfigFilter() {
    // create new config filter context and add
    //   to list associated with this loader
    std::unique_ptr<ConfigCtxVPL> configCtx;
    try {
        configCtx.reset(new ConfigCtxVPL{});
    }
    catch (...) {
        return nullptr;
    }

    ConfigCtxVPL* config = (ConfigCtxVPL*)(configCtx.release());

    m_configCtxList.push_back(config);

    return config;
}

mfxStatus LoaderCtxVPL::FreeConfigFilters() {
    std::list<ConfigCtxVPL*>::iterator it = m_configCtxList.begin();

    while (it != m_configCtxList.end()) {
        ConfigCtxVPL* config = (*it);
        if (config)
            delete config;
        it++;
    }

    return MFX_ERR_NONE;
}
