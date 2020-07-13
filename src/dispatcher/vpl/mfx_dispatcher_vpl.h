/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_DISPATCHER_VPL_MFX_DISPATCHER_VPL_H_
#define SRC_DISPATCHER_VPL_MFX_DISPATCHER_VPL_H_

#include <list>
#include <memory>
#include <sstream>
#include <string>

#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>

    #include "windows/mfx_dispatcher.h"
    #include "windows/mfx_load_dll.h"

    // use wide char on Windows
    #define MAKE_STRING(x) L##x
typedef std::wstring STRING_TYPE;
typedef wchar_t CHAR_TYPE;

#else
    #include <dirent.h>
    #include <dlfcn.h>
    #include <string.h>

    // use standard char on Linux
    #define MAKE_STRING(x) x
typedef std::string STRING_TYPE;
typedef char CHAR_TYPE;
#endif

#define MAX_VPL_SEARCH_PATH 1024

// internal function to load dll by full path, fail if unsuccessful
mfxStatus MFXInitEx2(mfxInitParam par, mfxSession* session, CHAR_TYPE* dllName);

typedef void(MFX_CDECL* VPLFunctionPtr)(void);

enum VPLFunctionIdx {
    IdxMFXQueryImplDescription = 0,
    IdxMFXReleaseImplDescription,
    IdxMFXMemory_GetSurfaceForVPP,
    IdxMFXMemory_GetSurfaceForEncode,
    IdxMFXMemory_GetSurfaceForDecode,

    NumVPLFunctions
};

// both Windows and Linux use char* for function names
struct VPLFunctionDesc {
    const char* pName;
    mfxVersion apiVersion;
};

// priority of runtime loading, based on oneAPI-spec
enum LibPriority {
    LIB_PRIORITY_USE_DEFINED  = 1,
    LIB_PRIORITY_VPL_PACKAGE  = 2,
    LIB_PRIORITY_MSDK_PACKAGE = 3,
};

enum CfgPropState {
    CFG_PROP_STATE_NOT_SET = 0,
    CFG_PROP_STATE_SUPPORTED,
    CFG_PROP_STATE_UNSUPPORTED,
};

// config class implementation
class ConfigCtxVPL {
public:
    ConfigCtxVPL();
    ~ConfigCtxVPL();

    // set a single filter property (KV pair)
    mfxStatus SetFilterProperty(const mfxU8* name, mfxVariant value);

    // compare library caps vs. set of configuration filters
    static mfxStatus ValidateConfig(mfxImplDescription* libImplDesc,
                                    std::list<ConfigCtxVPL*> m_configCtxList);
    static CfgPropState CheckProp(mfxI32 propIdx,
                                  mfxVariant value,
                                  mfxImplDescription* libImplDesc);

private:
    __inline std::string GetNextProp(std::list<std::string>* s) {
        std::string t = s->front();
        s->pop_front();
        return t;
    }

    mfxStatus ValidateAndSetProp(mfxI32 idx, mfxVariant value);
    mfxStatus SetFilterPropertyDec(mfxVariant value);
    mfxStatus SetFilterPropertyEnc(mfxVariant value);
    mfxStatus SetFilterPropertyVPP(mfxVariant value);

    std::string m_propName;
    mfxVariant m_propValue;
    mfxI32 m_propIdx;
    std::list<std::string> m_propParsedString;
};

struct LibInfo {
    // during search store candidate file names
    //   and priority based on rules in spec
    STRING_TYPE libNameFull;
    STRING_TYPE libNameBase;
    mfxU32 libPriority;

    // if valid library, store file handle
    //   and table of exported functions
    void* hModuleVPL;
    VPLFunctionPtr vplFuncTable[NumVPLFunctions]; // NOLINT
    mfxHDL implDesc;

    // used for session initialization with this implementation
    mfxInitParam initPar;

    // assign unique index after validating library
    mfxU32 libIdx;

    // avoid warnings
    LibInfo()
            : libNameFull(),
              libNameBase(),
              libPriority(0),
              hModuleVPL(nullptr),
              vplFuncTable(),
              implDesc(),
              initPar(),
              libIdx(0) {}
};

// loader class implementation
class LoaderCtxVPL {
public:
    LoaderCtxVPL();
    ~LoaderCtxVPL();

    // manage library implementations
    mfxStatus BuildListOfCandidateLibs();
    mfxU32 CheckValidLibraries();
    mfxStatus UnloadAllLibraries();

    // query capabilities of each implementation
    mfxStatus QueryImpl(mfxU32 idx,
                        mfxImplCapsDeliveryFormat format,
                        mfxHDL* idesc);
    mfxStatus ReleaseImpl(mfxHDL idesc);

    // create mfxSession
    mfxStatus CreateSession(mfxU32 idx, mfxSession* session);

    // manage configuration filters
    ConfigCtxVPL* AddConfigFilter();
    mfxStatus FreeConfigFilters();

private:
    // helper functions
    mfxStatus SearchDirForLibs(STRING_TYPE searchDir,
                               std::list<LibInfo*>& libInfoList,
                               mfxU32 priority);
    LibInfo* GetLibInfo(std::list<LibInfo*> libInfoList, mfxU32 idx);

    std::list<LibInfo*> m_libInfoList;
    std::list<ConfigCtxVPL*> m_configCtxList;

    STRING_TYPE m_vplPackageDir;
};

#endif // SRC_DISPATCHER_VPL_MFX_DISPATCHER_VPL_H_
