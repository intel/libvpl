/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef DISPATCHER_VPL_MFX_DISPATCHER_VPL_H_
#define DISPATCHER_VPL_MFX_DISPATCHER_VPL_H_

#include <algorithm>
#include <list>
#include <memory>
#include <sstream>
#include <string>

#include "vpl/mfxdispatcher.h"
#include "vpl/mfxvideo.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>

    #include "windows/mfx_dispatcher.h"
    #include "windows/mfx_dispatcher_defs.h"
    #include "windows/mfx_library_iterator.h"
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

#define MSDK_MIN_VERSION_MAJOR 1
#define MSDK_MIN_VERSION_MINOR 0

#define MAX_MSDK_ACCEL_MODES 16 // see mfxcommon.h --> mfxAccelerationMode

#define MAX_WINDOWS_ADAPTER_ID 3 // check adapterID in range [0,3]

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

#define TAB_SIZE(type, tab) (sizeof(tab) / sizeof(type))
#define MAKE_MFX_VERSION(major, minor) \
    { (minor), (major) }

// internal flag for MSDK compatibility mode (loading via MFXLoad)
// do not specify accelerator mode (see MFXInitEx2 default path)
enum { MFX_ACCEL_MODE_VIA_HW_ANY = 0x7FFFFFFF };

// internal function to load dll by full path, fail if unsuccessful
mfxStatus MFXInitEx2(mfxVersion version,
                     mfxInitializationParam vplParam,
                     mfxIMPL hwImpl,
                     mfxSession* session,
                     mfxU16* deviceID,
                     CHAR_TYPE* dllName);

typedef void(MFX_CDECL* VPLFunctionPtr)(void);

enum LibType {
    LibTypeUnknown = -1,

    LibTypeVPL = 0,
    LibTypeMSDK,

    NumLibTypes
};

enum VPLFunctionIdx {
    // 2.0
    IdxMFXQueryImplsDescription = 0,
    IdxMFXReleaseImplDescription,
    IdxMFXMemory_GetSurfaceForVPP,
    IdxMFXMemory_GetSurfaceForEncode,
    IdxMFXMemory_GetSurfaceForDecode,
    IdxMFXInitialize,

    // 2.1
    IdxMFXMemory_GetSurfaceForVPPOut,
    IdxMFXVideoDECODE_VPP_Init,
    IdxMFXVideoDECODE_VPP_DecodeFrameAsync,
    IdxMFXVideoDECODE_VPP_Reset,
    IdxMFXVideoDECODE_VPP_GetChannelParam,
    IdxMFXVideoDECODE_VPP_Close,
    IdxMFXVideoVPP_ProcessFrameAsync,

    NumVPLFunctions
};

// select MSDK functions for 1.x style caps query
enum MSDKCompatFunctionIdx {
    IdxMFXInitEx = 0,
    IdxMFXClose,

    NumMSDKFunctions
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

// special props which are passed in via MFXSetConfigProperty()
// these are updated with every call to ValidateConfig() and may
//   be used in MFXCreateSession()
struct SpecialConfig {
    mfxHandleType deviceHandleType;
    mfxHDL deviceHandle;
    mfxAccelerationMode accelerationMode;
    mfxVersion ApiVersion;
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
                                    mfxImplementedFunctions* libImplFuncs,
                                    std::list<ConfigCtxVPL*> configCtxList,
                                    LibType libType,
                                    SpecialConfig* specialConfig);

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

    static mfxStatus CheckPropString(mfxChar* implString, std::string filtString);

    std::string m_propName;
    mfxVariant m_propValue;
    mfxI32 m_propIdx;
    std::list<std::string> m_propParsedString;

    // special containers for properties which are passed by pointer
    //   (save a copy of the whole object based on m_propName)
    mfxRange32U m_propRange32U;
    std::string m_implName;
    std::string m_implLicense;
    std::string m_implKeywords;
    std::string m_implFunctionName;
};

// MSDK compatibility loader implementation
class LoaderCtxMSDK {
public:
    LoaderCtxMSDK();
    ~LoaderCtxMSDK();

    // public function to be called by VPL dispatcher
    // do not allocate any new memory here, so no need for a matching Release functions
    mfxStatus QueryMSDKCaps(STRING_TYPE libNameFull,
                            mfxImplDescription** implDesc,
                            mfxImplementedFunctions** implFuncs,
                            mfxIMPL* msdkAdapter);

    // required by MFXCreateSession
    mfxIMPL msdkAdapter;

private:
    // session management
    mfxStatus OpenSession(mfxSession* session,
                          STRING_TYPE libNameFull,
                          mfxAccelerationMode accelMode,
                          mfxIMPL hwImpl);
    void CloseSession(mfxSession* session);

    // utility functions
    mfxAccelerationMode CvtAccelType(mfxIMPL implType, mfxIMPL implMethod);
    mfxStatus GetDefaultAccelType(mfxU32 adapterID, mfxIMPL* implDefault);
    mfxStatus CheckAccelType(mfxU32 adapterID, mfxIMPL implTest);

    // internal state variables
    STRING_TYPE m_libNameFull;
    mfxImplDescription m_id; // base description struct
    mfxAccelerationMode m_accelMode[MAX_MSDK_ACCEL_MODES];
    mfxU16 m_loaderDeviceID;

    __inline bool IsVersionSupported(mfxVersion reqVersion, mfxVersion actualVersion) {
        if (actualVersion.Major > reqVersion.Major) {
            return true;
        }
        else if ((actualVersion.Major == reqVersion.Major) &&
                 (actualVersion.Minor >= reqVersion.Minor)) {
            return true;
        }
        return false;
    }
};

struct LibInfo {
    // during search store candidate file names
    //   and priority based on rules in spec
    STRING_TYPE libNameFull;
    STRING_TYPE libNameBase;
    mfxU32 libPriority;
    LibType libType;

    // if valid library, store file handle
    //   and table of exported functions
    void* hModuleVPL;
    VPLFunctionPtr vplFuncTable[NumVPLFunctions]; // NOLINT

    // select MSDK functions for 1.x style caps query
    VPLFunctionPtr msdkFuncTable[NumMSDKFunctions]; // NOLINT
    class LoaderCtxMSDK* msdkCtx;

    // avoid warnings
    LibInfo()
            : libNameFull(),
              libNameBase(),
              libPriority(0),
              libType(LibTypeUnknown),
              hModuleVPL(nullptr),
              vplFuncTable(),
              msdkFuncTable(),
              msdkCtx() {}

    ~LibInfo() {
        if (msdkCtx)
            delete msdkCtx;
    }
};

struct ImplInfo {
    // library containing this implementation
    LibInfo* libInfo;

    // description of implementation
    mfxHDL implDesc;

    // list of implemented functions
    mfxHDL implFuncs;

    // used for session initialization with this implementation
    mfxInitializationParam vplParam;
    mfxVersion version;

    // local index for libraries with more than one implementation
    mfxU32 libImplIdx;

    // index of valid libraries - updates with every call to MFXSetConfigFilterProperty()
    mfxI32 validImplIdx;

    // avoid warnings
    ImplInfo()
            : libInfo(nullptr),
              implDesc(nullptr),
              implFuncs(nullptr),
              vplParam(),
              version(),
              libImplIdx(0),
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
    mfxStatus PrioritizeImplList(void);

    // create mfxSession
    mfxStatus CreateSession(mfxU32 idx, mfxSession* session);

    // manage configuration filters
    ConfigCtxVPL* AddConfigFilter();
    mfxStatus FreeConfigFilters();

private:
    // helper functions
    mfxStatus LoadSingleLibrary(LibInfo* libInfo);
    mfxStatus UnloadSingleLibrary(LibInfo* libInfo);
    mfxStatus UnloadSingleImplementation(ImplInfo* implInfo);
    VPLFunctionPtr GetFunctionAddr(void* hModuleVPL, const char* pName);

    mfxU32 ParseEnvSearchPaths(const CHAR_TYPE* envVarName, std::list<STRING_TYPE>& searchDirs);
    mfxU32 ParseLegacySearchPaths(std::list<STRING_TYPE>& searchDirs);

    mfxStatus SearchDirForLibs(STRING_TYPE searchDir,
                               std::list<LibInfo*>& libInfoList,
                               mfxU32 priority);

    mfxStatus ValidateAPIExports(VPLFunctionPtr* vplFuncTable, mfxVersion reportedVersion);

    std::list<LibInfo*> m_libInfoList;
    std::list<ImplInfo*> m_implInfoList;
    std::list<ConfigCtxVPL*> m_configCtxList;

    std::list<STRING_TYPE> m_userSearchDirs;
    std::list<STRING_TYPE> m_packageSearchDirs;
    std::list<STRING_TYPE> m_pathSearchDirs;
    std::list<STRING_TYPE> m_legacySearchDirs;
    STRING_TYPE m_vplPackageDir;
    STRING_TYPE m_driverStoreDir;
    SpecialConfig m_specialConfig;

    mfxU32 m_implIdxNext;
    bool m_bKeepCapsUntilUnload;
};

#endif // DISPATCHER_VPL_MFX_DISPATCHER_VPL_H_
