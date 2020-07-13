/*############################################################################
  # Copyright (C) 2012-2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#ifndef SRC_DISPATCHER_WINDOWS_MFX_DISPATCHER_H_
#define SRC_DISPATCHER_WINDOWS_MFX_DISPATCHER_H_

#include <vpl/mfxvideo.h>

#include <vpl/mfxdispatcher.h>

#include <stddef.h>
#include "windows/mfx_dispatcher_defs.h"

#define INTEL_VENDOR_ID 0x8086

mfxStatus MFXQueryVersion(mfxSession session, mfxVersion *version);

enum {
    // to avoid code changing versions are just inherited
    // from the API header file.
    DEFAULT_API_VERSION_MAJOR = MFX_VERSION_MAJOR,
    DEFAULT_API_VERSION_MINOR = MFX_VERSION_MINOR
};

enum { VPL_MINIMUM_VERSION_MAJOR = 2, VPL_MINIMUM_VERSION_MINOR = 0 };

//
// declare functions' integer identifiers.
//

#undef FUNCTION
#define FUNCTION(return_value,      \
                 func_name,         \
                 formal_param_list, \
                 actual_param_list) \
    e##func_name,

enum eFunc {
    eMFXInit,
    eMFXClose,
    eMFXQueryIMPL,
    eMFXQueryVersion,
    eMFXJoinSession,
    eMFXDisjoinSession,
    eMFXCloneSession,
    eMFXSetPriority,
    eMFXGetPriority,
    eMFXInitEx,
#include "windows/mfx_exposed_functions_list.h"
    eVideoFuncTotal
};

enum ePluginFunc {
    eMFXVideoUSER_Load,
    eMFXVideoUSER_LoadByPath,
    eMFXVideoUSER_UnLoad,
    eMFXAudioUSER_Load,
    eMFXAudioUSER_UnLoad,
    ePluginFuncTotal
};

enum eVideoFunc2 {
    eMFXQueryImplDescription,
    eMFXReleaseImplDescription,
    eMFXMemory_GetSurfaceForVPP,
    eMFXMemory_GetSurfaceForEncode,
    eMFXMemory_GetSurfaceForDecode,

    eVideoFunc2Total
};

// declare max buffer length for regsitry key name
enum { MFX_MAX_REGISTRY_KEY_NAME = 256 };

// declare the maximum DLL path
enum { MFX_MAX_DLL_PATH = 1024 };

// declare library's implementation types
enum eMfxImplType {
    MFX_LIB_HARDWARE = 0,
    MFX_LIB_SOFTWARE = 1,
    MFX_LIB_PSEUDO   = 2,

    MFX_LIB_IMPL_TYPES
};

// declare dispatcher's version
enum { MFX_DISPATCHER_VERSION_MAJOR = 1, MFX_DISPATCHER_VERSION_MINOR = 2 };

struct _mfxSession {
    // A real handle from MFX engine passed to a called function
    mfxSession session;

    mfxFunctionPointer callTable[eVideoFuncTotal]; // NOLINT(runtime/arrays)
    mfxFunctionPointer
        callPlugInsTable[ePluginFuncTotal]; // NOLINT(runtime/arrays)
    mfxFunctionPointer
        callVideoTable2[eVideoFunc2Total]; // NOLINT(runtime/arrays)

    // Current library's implementation (exact implementation)
    mfxIMPL impl;
};

// declare a dispatcher's handle
struct MFX_DISP_HANDLE : public _mfxSession {
    // Default constructor
    explicit MFX_DISP_HANDLE(const mfxVersion requiredVersion);
    // Destructor
    ~MFX_DISP_HANDLE(void);

    // Load the library's module
    mfxStatus LoadSelectedDLL(const wchar_t *pPath,
                              eMfxImplType implType,
                              mfxIMPL impl,
                              mfxIMPL implInterface,
                              mfxInitParam &par);
    // Unload the library's module
    mfxStatus UnLoadSelectedDLL(void);

    // Close the handle
    mfxStatus Close(void);

    // NOTE: changing order of struct's members can make different version of
    // dispatchers incompatible. Think of different modules (e.g. MFT filters)
    // within a single application.

    // Library's implementation type (hardware or software)
    eMfxImplType implType;
    // Current library's VIA interface
    mfxIMPL implInterface;
    // Dispatcher's version. If version is 1.1 or lower, then old dispatcher's
    // architecture is used. Otherwise it means current dispatcher's version.
    mfxVersion dispVersion;
    // Required API version of session initialized
    const mfxVersion apiVersion;
    // Actual library API version
    mfxVersion actualApiVersion;
    // Status of loaded dll
    mfxStatus loadStatus;
    // Resgistry subkey name for windows version
    wchar_t subkeyName[MFX_MAX_REGISTRY_KEY_NAME];
    // Storage ID for windows version
    int storageID;

    // Library's module handle
    mfxModuleHandle hModule;

private:
    // Declare assignment operator and copy constructor to prevent occasional assignment
    MFX_DISP_HANDLE(const MFX_DISP_HANDLE &);
    MFX_DISP_HANDLE &operator=(const MFX_DISP_HANDLE &);
};

// declare comparison operator
inline bool operator==(const mfxVersion &one, const mfxVersion &two) {
    return (one.Version == two.Version);
}

inline bool operator<(const mfxVersion &one, const mfxVersion &two) {
    return (one.Major < two.Major) ||
           ((one.Major == two.Major) && (one.Minor < two.Minor));
}

inline bool operator<=(const mfxVersion &one, const mfxVersion &two) {
    return (one == two) || (one < two);
}

//
// declare a table with functions descriptions
//

typedef struct FUNCTION_DESCRIPTION {
    // Literal function's name
    const char *pName;
    // API version when function appeared first time
    mfxVersion apiVersion;
} FUNCTION_DESCRIPTION;

extern const FUNCTION_DESCRIPTION APIFunc[eVideoFuncTotal];

extern const FUNCTION_DESCRIPTION APIVideoFunc2[eVideoFunc2Total];

#endif // SRC_DISPATCHER_WINDOWS_MFX_DISPATCHER_H_
