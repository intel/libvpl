/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef DISPATCHER_VPL_MFX_DISPATCHER_VPL_H_
#define DISPATCHER_VPL_MFX_DISPATCHER_VPL_H_

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

#define MAX_VPL_SEARCH_PATH 4096

// OS-specific environment variables for implementation
//   search paths as defined by spec
#if defined(_WIN32) || defined(_WIN64)
    #define ENV_ONEVPL_SEARCH_PATH  L"ONEVPL_SEARCH_PATH"
    #define ENV_ONEVPL_PACKAGE_PATH L"VPL_BIN"
    #define ENV_OS_PATH             L"PATH"
#else
    #define ENV_ONEVPL_SEARCH_PATH  "ONEVPL_SEARCH_PATH"
    #define ENV_ONEVPL_PACKAGE_PATH "VPL_BIN"
    #define ENV_OS_PATH             "LD_LIBRARY_PATH"
#endif

// internal function to load dll by full path, fail if unsuccessful
mfxStatus MFXInitEx2(mfxInitParam par, mfxSession* session, CHAR_TYPE* dllName);

typedef void(MFX_CDECL* VPLFunctionPtr)(void);

enum VPLFunctionIdx {
    IdxMFXQueryImplsDescription = 0,
    IdxMFXReleaseImplDescription,
    IdxMFXMemory_GetSurfaceForVPP,
    IdxMFXMemory_GetSurfaceForEncode,
    IdxMFXMemory_GetSurfaceForDecode,
    IdxMFXInitialize,

    NumVPLFunctions
};

// both Windows and Linux use char* for function names
struct VPLFunctionDesc {
    const char* pName;
    mfxVersion apiVersion;
};

// priority of runtime loading, based on oneAPI-spec
enum LibPriority {
    LIB_PRIORITY_USER_DEFINED = 1,
    LIB_PRIORITY_VPL_PACKAGE  = 2,
    LIB_PRIORITY_OS_PATH      = 3,
    LIB_PRIORITY_SYS_DEFAULT  = 4,
    LIB_PRIORITY_MSDK_PACKAGE = 5,
};

enum CfgPropState {
    CFG_PROP_STATE_NOT_SET = 0,
    CFG_PROP_STATE_SUPPORTED,
    CFG_PROP_STATE_UNSUPPORTED,
};

// typedef child structures for easier reading
typedef struct mfxDecoderDescription::decoder DecCodec;
typedef struct mfxDecoderDescription::decoder::decprofile DecProfile;
typedef struct mfxDecoderDescription::decoder::decprofile::decmemdesc DecMemDesc;

typedef struct mfxEncoderDescription::encoder EncCodec;
typedef struct mfxEncoderDescription::encoder::encprofile EncProfile;
typedef struct mfxEncoderDescription::encoder::encprofile::encmemdesc EncMemDesc;

typedef struct mfxVPPDescription::filter VPPFilter;
typedef struct mfxVPPDescription::filter::memdesc VPPMemDesc;
typedef struct mfxVPPDescription::filter::memdesc::format VPPFormat;

// flattened version of single enc/dec/vpp configs
// each struct contains all _settable_ props
//   i.e. not implied values like NumCodecs
struct DecConfig {
    mfxU32 CodecID;
    mfxU16 MaxcodecLevel;
    mfxU32 Profile;
    mfxResourceType MemHandleType;
    mfxRange32U Width;
    mfxRange32U Height;
    mfxU32 ColorFormat;
};

struct EncConfig {
    mfxU32 CodecID;
    mfxU16 MaxcodecLevel;
    mfxU16 BiDirectionalPrediction;
    mfxU32 Profile;
    mfxResourceType MemHandleType;
    mfxRange32U Width;
    mfxRange32U Height;
    mfxU32 ColorFormat;
};

struct VPPConfig {
    mfxU32 FilterFourCC;
    mfxU16 MaxDelayInFrames;
    mfxResourceType MemHandleType;
    mfxRange32U Width;
    mfxRange32U Height;
    mfxU32 InFormat;
    mfxU32 OutFormat;
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
                                    std::list<ConfigCtxVPL*> configCtxList);

    // loader object this config is associated with - needed to
    //   rebuild valid implementation list after each calling
    //   MFXSetConfigFilterProperty()
    class LoaderCtxVPL* m_parentLoader;

private:
    __inline std::string GetNextProp(std::list<std::string>* s) {
        if (s->empty())
            return "";
        std::string t = s->front();
        s->pop_front();
        return t;
    }

    mfxStatus ValidateAndSetProp(mfxI32 idx, mfxVariant value);
    mfxStatus SetFilterPropertyDec(mfxVariant value);
    mfxStatus SetFilterPropertyEnc(mfxVariant value);
    mfxStatus SetFilterPropertyVPP(mfxVariant value);

    static mfxStatus GetFlatDescriptionsDec(mfxImplDescription* libImplDesc,
                                            std::list<DecConfig>& decConfigList);

    static mfxStatus GetFlatDescriptionsEnc(mfxImplDescription* libImplDesc,
                                            std::list<EncConfig>& encConfigList);

    static mfxStatus GetFlatDescriptionsVPP(mfxImplDescription* libImplDesc,
                                            std::list<VPPConfig>& vppConfigList);

    static mfxStatus CheckPropsGeneral(mfxVariant cfgPropsAll[], mfxImplDescription* libImplDesc);

    static mfxStatus CheckPropsDec(mfxVariant cfgPropsAll[], std::list<DecConfig> decConfigList);

    static mfxStatus CheckPropsEnc(mfxVariant cfgPropsAll[], std::list<EncConfig> encConfigList);

    static mfxStatus CheckPropsVPP(mfxVariant cfgPropsAll[], std::list<VPPConfig> vppConfigList);

    std::string m_propName;
    mfxVariant m_propValue;
    mfxI32 m_propIdx;
    std::list<std::string> m_propParsedString;

    // special containers for properties which are passed by pointer
    //   (save a copy of the whole object based on m_propName)
    mfxRange32U m_propRange32U;
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

    // avoid warnings
    LibInfo() : libNameFull(), libNameBase(), libPriority(0), hModuleVPL(nullptr), vplFuncTable() {}
};

struct ImplInfo {
    // library containing this implementation
    LibInfo* libInfo;

    // description of implementation
    mfxHDL implDesc;

    // used for session initialization with this implementation
    mfxInitParam initPar;

    // local index for libraries with more than one implementation
    mfxU32 libImplIdx;

    // globally unique index assigned after querying implementation
    mfxU32 vplImplIdx;

    // index of valid libraries - updates with every call to MFXSetConfigFilterProperty()
    mfxI32 validImplIdx;

    // avoid warnings
    ImplInfo()
            : libInfo(nullptr),
              implDesc(nullptr),
              initPar(),
              libImplIdx(0),
              vplImplIdx(0),
              validImplIdx(-1) {}
};

// loader class implementation
class LoaderCtxVPL {
public:
    LoaderCtxVPL();
    ~LoaderCtxVPL();

    // manage library implementations
    mfxStatus BuildListOfCandidateLibs();
    mfxU32 CheckValidLibraries();
    mfxStatus QueryLibraryCaps();
    mfxStatus UnloadAllLibraries();

    // query capabilities of each implementation
    mfxStatus QueryImpl(mfxU32 idx, mfxImplCapsDeliveryFormat format, mfxHDL* idesc);
    mfxStatus ReleaseImpl(mfxHDL idesc);

    // update list of valid implementations based on current filter props
    mfxStatus UpdateValidImplList(void);

    // create mfxSession
    mfxStatus CreateSession(mfxU32 idx, mfxSession* session);

    // manage configuration filters
    ConfigCtxVPL* AddConfigFilter();
    mfxStatus FreeConfigFilters();

private:
    // helper functions
    mfxU32 ParseEnvSearchPaths(const CHAR_TYPE* envVarName, std::list<STRING_TYPE>& searchDirs);
    mfxStatus SearchDirForLibs(STRING_TYPE searchDir,
                               std::list<LibInfo*>& libInfoList,
                               mfxU32 priority);

    std::list<LibInfo*> m_libInfoList;
    std::list<ImplInfo*> m_implInfoList;
    std::list<ConfigCtxVPL*> m_configCtxList;

    std::list<STRING_TYPE> m_userSearchDirs;
    std::list<STRING_TYPE> m_packageSearchDirs;
    std::list<STRING_TYPE> m_pathSearchDirs;
    STRING_TYPE m_vplPackageDir;

    mfxU32 m_implIdxNext;
};

#endif // DISPATCHER_VPL_MFX_DISPATCHER_VPL_H_
