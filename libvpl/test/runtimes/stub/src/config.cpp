/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <iostream>
#include <map>
#include <ostream>
#include <string>

#include "src/caps.h"
#include "src/config.h"

// the auto-generated capabilities structs
// only include one time in this library
#ifdef ENABLE_STUB_1X
    #include "src/caps_dec_none.h"
    #include "src/caps_enc_none.h"
    #include "src/caps_vpp_none.h"
#else
    // include some actual stub caps
    #include "src/caps_dec.h"
    #include "src/caps_enc.h"
    #include "src/caps_vpp.h"
#endif

#include "src/caps_surface.h"

#define DEFAULT_SESSION_HANDLE_1X 0x01
#define DEFAULT_SESSION_HANDLE_2X 0x02

#define DEFAULT_CLONE_SESSION_HANDLE 0x08

// print messages to be parsed in unit tests to stdout, and other errors to stderr
static void StubRTLogMessage(const char *msg, ...) {
    std::cout << "[STUB RT]: message -- ";

    char s[1024] = "";
    va_list args;
    va_start(args, msg);
    vsprintf_s(s, sizeof(s) - 1, msg, args);
    va_end(args);

    std::cout << s << std::endl;
}

static void StubRTLogError(const char *msg, ...) {
    std::cout << "[STUB RT]: ERROR -- ";

    char s[1024] = "";
    va_list args;
    va_start(args, msg);
    vsprintf_s(s, sizeof(s) - 1, msg, args);
    va_end(args);

    std::cerr << s << std::endl;
}

static mfxStatus ValidateExtBuf(const char *strInitFunc, mfxExtBuffer *extBuf) {
    if (extBuf == nullptr) {
        StubRTLogError("%s -- extBuf is NULL\n", strInitFunc);
        return MFX_ERR_NULL_PTR;
    }

    // in practice most of these EXTBUFF types would not be relevant at init time,
    //   but we define some 'valid' ones in the stub RT's in order to validate dispatcher logic
    if (extBuf->BufferId == MFX_EXTBUFF_THREADS_PARAM) {
        if (extBuf->BufferSz != sizeof(mfxExtThreadsParam)) {
            StubRTLogError("%s -- invalid extBuf size (expected = %d, actual = %d)\n",
                           strInitFunc,
                           sizeof(mfxExtThreadsParam),
                           extBuf->BufferSz);
            return MFX_ERR_UNSUPPORTED;
        }

        // valid buffer
        mfxExtThreadsParam *extBufThreads = (mfxExtThreadsParam *)extBuf;
        StubRTLogMessage("%s -- extBuf enabled -- NumThread (%d)",
                         strInitFunc,
                         extBufThreads->NumThread);
    }
    else if (extBuf->BufferId == MFX_EXTBUFF_VPP_PROCAMP) {
        if (extBuf->BufferSz != sizeof(mfxExtVPPProcAmp)) {
            StubRTLogError("%s -- invalid extBuf size (expected = %d, actual = %d)\n",
                           strInitFunc,
                           sizeof(mfxExtVPPProcAmp),
                           extBuf->BufferSz);
            return MFX_ERR_UNSUPPORTED;
        }

        // valid buffer
        mfxExtVPPProcAmp *extVPPProcAmp = (mfxExtVPPProcAmp *)extBuf;
        StubRTLogMessage("%s -- extBuf enabled -- ProcAmp (%.1f %.2f %.1f %.2f)",
                         strInitFunc,
                         extVPPProcAmp->Contrast,
                         extVPPProcAmp->Brightness,
                         extVPPProcAmp->Hue,
                         extVPPProcAmp->Saturation);
    }
    else {
        return MFX_ERR_UNSUPPORTED;
    }

    // unsupported extBuf type
    return MFX_ERR_NONE;
}

// preferred entrypoint for 2.0 implementations (instead of MFXInitEx)
mfxStatus MFXInitialize(mfxInitializationParam par, mfxSession *session) {
    if (!session)
        return MFX_ERR_NULL_PTR;

    // check for valid extBufs
    if (par.NumExtParam > 0 && par.ExtParam == nullptr) {
        StubRTLogError("MFXInitialize -- ExtParam base ptr is NULL\n");
        return MFX_ERR_NULL_PTR;
    }

    for (mfxU32 idx = 0; idx < par.NumExtParam; idx++) {
        mfxStatus sts = MFX_ERR_NONE;

        // ValidateExtBuf checks for supported BufferId, returns MFX_ERR_UNSUPPORTED if not
        sts = ValidateExtBuf("MFXInitialize", par.ExtParam[idx]);

        if (sts != MFX_ERR_NONE)
            return sts;
    }

#if ONEVPL_EXPERIMENTAL
    if (par.DeviceCopy) {
        if (par.DeviceCopy == MFX_GPUCOPY_ON || par.DeviceCopy == MFX_GPUCOPY_OFF) {
            StubRTLogMessage("MFXInitialize -- DeviceCopy set (%d)", par.DeviceCopy);
        }
        else {
            StubRTLogMessage("MFXInitialize -- DeviceCopy set to invalid value (%d)",
                             par.DeviceCopy);
            return MFX_ERR_UNSUPPORTED;
        }
    }
#endif

    _mfxSession *stubSession = nullptr;
    try {
        stubSession = new _mfxSession;
    }
    catch (...) {
        return MFX_ERR_MEMORY_ALLOC;
    }

    stubSession->handleType = DEFAULT_SESSION_HANDLE_2X;

    *session = (mfxSession)stubSession;

    return MFX_ERR_NONE;
}

mfxStatus MFXInitEx(mfxInitParam par, mfxSession *session) {
    if (!session)
        return MFX_ERR_NULL_PTR;

    // check for valid extBufs
    if (par.NumExtParam > 0 && par.ExtParam == nullptr) {
        StubRTLogError("MFXInitEx -- ExtParam base ptr is NULL\n");
        return MFX_ERR_NULL_PTR;
    }

    for (mfxU32 idx = 0; idx < par.NumExtParam; idx++) {
        mfxStatus sts = MFX_ERR_NONE;

        // ValidateExtBuf checks for supported BufferId, returns MFX_ERR_UNSUPPORTED if not
        sts = ValidateExtBuf("MFXInitEx", par.ExtParam[idx]);

        if (sts != MFX_ERR_NONE)
            return sts;
    }

#ifdef ONEVPL_EXPERIMENTAL
    if (par.GPUCopy) {
        if (par.GPUCopy == MFX_GPUCOPY_ON || par.GPUCopy == MFX_GPUCOPY_OFF) {
            StubRTLogMessage("MFXInitEx -- GPUCopy set (%d)", par.GPUCopy);
        }
        else {
            StubRTLogMessage("MFXInitEx -- GPUCopy set to invalid value (%d)", par.GPUCopy);
            return MFX_ERR_UNSUPPORTED;
        }
    }
#endif

    _mfxSession *stubSession = nullptr;
    try {
        stubSession = new _mfxSession;
    }
    catch (...) {
        return MFX_ERR_MEMORY_ALLOC;
    }

    stubSession->handleType = DEFAULT_SESSION_HANDLE_1X;

    *session = (mfxSession)stubSession;

    return MFX_ERR_NONE;
}

mfxStatus MFXCloneSession(mfxSession session, mfxSession *clone) {
    if (!session)
        return MFX_ERR_INVALID_HANDLE;

    _mfxSession *stubSession = (_mfxSession *)session;
    if (stubSession->handleType != DEFAULT_SESSION_HANDLE_1X &&
        stubSession->handleType != DEFAULT_SESSION_HANDLE_2X)
        return MFX_ERR_INVALID_HANDLE;

    // create a clone session
    _mfxSession *cloneSession = nullptr;
    try {
        cloneSession = new _mfxSession;
    }
    catch (...) {
        return MFX_ERR_MEMORY_ALLOC;
    }

    cloneSession->handleType = DEFAULT_CLONE_SESSION_HANDLE;

    *clone = (mfxSession)cloneSession;

    return MFX_ERR_NONE;
}

mfxStatus MFXDisjoinSession(mfxSession session) {
    if (!session)
        return MFX_ERR_INVALID_HANDLE;

    _mfxSession *stubSession = (_mfxSession *)session;
    if (stubSession->handleType != DEFAULT_CLONE_SESSION_HANDLE)
        return MFX_ERR_INVALID_HANDLE;

    return MFX_ERR_NONE;
}

mfxStatus MFXClose(mfxSession session) {
    if (!session)
        return MFX_ERR_INVALID_HANDLE;

    _mfxSession *stubSession = (_mfxSession *)session;
    if (stubSession->handleType != DEFAULT_SESSION_HANDLE_1X &&
        stubSession->handleType != DEFAULT_SESSION_HANDLE_2X &&
        stubSession->handleType != DEFAULT_CLONE_SESSION_HANDLE)
        return MFX_ERR_INVALID_HANDLE;

    delete stubSession;

    return MFX_ERR_NONE;
}

#define NUM_CPU_IMPLS 1

// optional wrapper for dynamicall-allocated caps tables which may be modified
// on Release, the pointer at handle - 8 will point to this wrapper object, which allows us to free the memory
//   allocated for this implIdx
// when the final implIdx object is released, we can free the implArray
typedef struct {
    mfxU8 *basePtr; // base pointer to buffer for this implIdx

    mfxU32 implFormat;
    mfxU32 numImpls; // number of impls in implArray
    mfxU32 implIdx; // implementation index this mfxCapsWrapper describes

    // pointer to the implArray containing the handles for all impls
    // on Release each element is set to null after freeing the memory
    // when the last one is freed, the implArray is released
    mfxHDL *implArray;
} mfxCapsWrapper;

// optional wrapper for read-only caps tables which are not copied and modified
// on Release, if pointer at handle - 8 (i.e. emptyBasePtr) is null then we can just return (nothing to free)
template <typename T>
struct mfxCapsWrapperReadonly {
    mfxU64 emptyBasePtr; // set to 0
    T capsPtr; // the actual caps structure
};

#define NUM_ACCELERATION_MODES_CPU 1

static const mfxAccelerationMode AccelerationMode[NUM_ACCELERATION_MODES_CPU] = {
    MFX_ACCEL_MODE_NA,
};

#define NUM_POOL_POLICIES_CPU 3

static const mfxPoolAllocationPolicy PoolPolicies[NUM_POOL_POLICIES_CPU] = {
    MFX_ALLOCATION_OPTIMAL,
    MFX_ALLOCATION_UNLIMITED,
    MFX_ALLOCATION_LIMITED,
};

// leave table formatting alone
// clang-format off

//
// MFX_IMPLCAPS_IMPLDESCSTRUCTURE
//

// read-only description, may be copied and modified based on caps query type
static const mfxImplDescription minImplDesc = {
    {{ 2, 1 }},                                     // struct Version
    MFX_IMPL_TYPE_SOFTWARE,                         // Impl
    MFX_ACCEL_MODE_NA,                              // AccelerationMode
#ifdef ENABLE_STUB_1X
    {{ 99, 1}},                                     // ApiVersion
    "Stub Implementation 1X",                       // ImplName
#elif SKIP_NEW_FUNCTIONS
    {{ MFX_VERSION_MINOR, MFX_VERSION_MAJOR }},     // ApiVersion
    "Stub Implementation - no fn",                  // ImplName
#else
    {{ MFX_VERSION_MINOR, MFX_VERSION_MAJOR }},     // ApiVersion
    "Stub Implementation",                          // ImplName
#endif

    "MIT",                                          // License

#if defined _M_IX86
    "VPL,Stub,x86",                                 // Keywords
#else
    "VPL,Stub,x64",                                 // Keywords
#endif

    0x8086,                                         // VendorID
    0xFFFF,                                         // VendorImplID

    // mfxDeviceDescription Dev
    {
        {{ 1, 1 }},        // struct Version
        {},                // reserved
        MFX_MEDIA_UNKNOWN, // MediaAdapterType
        "0000",            // DeviceID
        0,                 // NumSubDevices
        {},                // SubDevices
    },

    // mfxDecoderDescription Dec
    {
        {{ decoderDesc.Version.Minor, decoderDesc.Version.Major }},
        {},
        decoderDesc.NumCodecs,
        (DecCodec *)decoderDesc.Codecs,  // null for 1.x stub, may be valid for 2.x stub
    },

    // mfxEncoderDescription Enc
    {
        {{ encoderDesc.Version.Minor, encoderDesc.Version.Major }},
        {},
        encoderDesc.NumCodecs,
        (EncCodec *)encoderDesc.Codecs,
    },

    // mfxVPPDescription VPP
    {
        {{ vppDesc.Version.Minor, vppDesc.Version.Major }},
        {},
        vppDesc.NumFilters,
        (VPPFilter *)vppDesc.Filters,
    },

    // union { mfxAccelerationModeDescription AccelerationModeDescription }
    { {
        {{ 0, 1 }},
        {},
        NUM_ACCELERATION_MODES_CPU,
        (mfxAccelerationMode *)AccelerationMode,
    } },

    {
        {{ 0, 1 }},
        {},
        NUM_POOL_POLICIES_CPU,
        (mfxPoolAllocationPolicy *)PoolPolicies,
    },

    {},     // reserved
    0,      // NumExtParam
    {},     // ExtParams
};

static const struct mfxCapsWrapperReadonly<mfxImplDescription> minImplDescWrapper = {
    0,              // empty base ptr (8 bytes of zeros)
    minImplDesc,   // caps structure
};

static const mfxImplDescription *minImplDescArray[NUM_CPU_IMPLS] = {
    &(minImplDescWrapper.capsPtr),
};

//
// MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS
//

// should match libvplsw.def (unless any are not actually implemented, of course)
static const mfxChar *minImplFuncsNames[] = {
    "MFXInit",
    "MFXClose",
    "MFXQueryIMPL",
    "MFXQueryVersion",
    "MFXJoinSession",
    "MFXDisjoinSession",
    "MFXCloneSession",
    "MFXSetPriority",
    "MFXGetPriority",
    "MFXVideoCORE_SetFrameAllocator",
    "MFXVideoCORE_SetHandle",
    "MFXVideoCORE_GetHandle",
    "MFXVideoCORE_QueryPlatform",
    "MFXVideoCORE_SyncOperation",
    "MFXVideoENCODE_Query",
    "MFXVideoENCODE_QueryIOSurf",
    "MFXVideoENCODE_Init",
    "MFXVideoENCODE_Reset",
    "MFXVideoENCODE_Close",
    "MFXVideoENCODE_GetVideoParam",
    "MFXVideoENCODE_GetEncodeStat",
    "MFXVideoENCODE_EncodeFrameAsync",
    "MFXVideoDECODE_Query",
    "MFXVideoDECODE_DecodeHeader",
    "MFXVideoDECODE_QueryIOSurf",
    "MFXVideoDECODE_Init",
    "MFXVideoDECODE_Reset",
    "MFXVideoDECODE_Close",
    "MFXVideoDECODE_GetVideoParam",
    "MFXVideoDECODE_GetDecodeStat",
    "MFXVideoDECODE_SetSkipMode",
    "MFXVideoDECODE_GetPayload",
    "MFXVideoDECODE_DecodeFrameAsync",
    "MFXVideoVPP_Query",
    "MFXVideoVPP_QueryIOSurf",
    "MFXVideoVPP_Init",
    "MFXVideoVPP_Reset",
    "MFXVideoVPP_Close",
    "MFXVideoVPP_GetVideoParam",
    "MFXVideoVPP_GetVPPStat",
    "MFXVideoVPP_RunFrameVPPAsync",
    "MFXInitEx",
    "MFXQueryImplsDescription",
    "MFXReleaseImplDescription",
    "MFXMemory_GetSurfaceForVPP",
    "MFXMemory_GetSurfaceForEncode",
    "MFXMemory_GetSurfaceForDecode",
    "MFXInitialize",
    "MFXMemory_GetSurfaceForVPPOut",
    "MFXVideoDECODE_VPP_Init",
    "MFXVideoDECODE_VPP_DecodeFrameAsync",
    "MFXVideoDECODE_VPP_Reset",
    "MFXVideoDECODE_VPP_GetChannelParam",
    "MFXVideoDECODE_VPP_Close",
    "MFXVideoVPP_ProcessFrameAsync",

#ifndef SKIP_NEW_FUNCTIONS
    "MFXQueryImplsProperties",
#endif
};

static const mfxImplementedFunctions minImplFuncs = {
    sizeof(minImplFuncsNames) / sizeof(mfxChar *),
    (mfxChar**)minImplFuncsNames
};

static const struct mfxCapsWrapperReadonly<mfxImplementedFunctions> minImplFuncsWrapper = {
    0,              // empty base ptr (8 bytes of zeros)
    minImplFuncs,   // caps structure
};

static const mfxImplementedFunctions *minImplFuncsArray[NUM_CPU_IMPLS] = {
    &(minImplFuncsWrapper.capsPtr),
};

#ifndef ENABLE_STUB_1X
//
// MFX_IMPLCAPS_DEVICE_ID_EXTENDED
//

static const mfxExtendedDeviceId minExtDeviceID = {
    {{0, 1}},       // Version

    0x8086,         // VendorID
    0x1595,         // DeviceID
    
    1,              // PCIDomain
    3,              // PCIBus
    7,              // PCIDevice
    9,              // PCIFunction
    
    { 0x4f, 0x59, 0x2e, 0xa1, 0x33, 0x78, 0xdb, 0x29 }, // DeviceLUID
    
    0x0000AEAE,     // LUIDDeviceNodeMask
    1,              // LUIDValid

    130,            // DRMRenderNodeNum
    2,              // DRMPrimaryNodeNum
    0x1034,         // RevisionID

    {},             // reserved1

    "stub-extDev",  // DeviceName
};

static const struct mfxCapsWrapperReadonly<mfxExtendedDeviceId> minExtDeviceIDWrapper = {
    0,              // empty base ptr (8 bytes of zeros)
    minExtDeviceID,   // caps structure
};

static const mfxExtendedDeviceId *minExtDeviceIDArray[NUM_CPU_IMPLS] = {
    &(minExtDeviceIDWrapper.capsPtr),
};

#ifdef ONEVPL_EXPERIMENTAL
//
// MFX_IMPLCAPS_SURFACE_TYPES
//

static const struct mfxCapsWrapperReadonly<mfxSurfaceTypesSupported> minSurfTypesSupportedWrapper = {
    0,                       // empty base ptr (8 bytes of zeros)
    minSurfTypesSupported,   // caps structure - defined in caps_surface.h
};

static const mfxSurfaceTypesSupported *minSurfTypesSupportedArray[NUM_CPU_IMPLS] = {
    &(minSurfTypesSupportedWrapper.capsPtr),
};
#endif

#endif

// query and release are independent of session - called during
//   caps query and config stage using Intel® Video Processing Library (Intel® VPL) extensions
mfxHDL *MFXQueryImplsDescription(mfxImplCapsDeliveryFormat format, mfxU32 *num_impls) {
    if (!num_impls)
        return nullptr;

    *num_impls = NUM_CPU_IMPLS;

    if (format == MFX_IMPLCAPS_IMPLDESCSTRUCTURE) {
        return (mfxHDL *)(minImplDescArray);
    }
    else if (format == MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS) {
        return (mfxHDL *)(minImplFuncsArray);
    }
    else if (format == MFX_IMPLCAPS_DEVICE_ID_EXTENDED) {
#ifdef ENABLE_STUB_1X
        return nullptr;
#else
        return (mfxHDL *)(minExtDeviceIDArray);
#endif
    }
#ifdef ONEVPL_EXPERIMENTAL
    else if (format == MFX_IMPLCAPS_SURFACE_TYPES) {
    #ifdef ENABLE_STUB_1X
        return nullptr;
    #else
        return (mfxHDL *)(minSurfTypesSupportedArray);
    #endif
    }
#endif
    else {
        return nullptr;
    }
}

static void ReleaseImplDesc(mfxHDL hdl) {
    if (!hdl)
        return;

    mfxU8 *capsPtr              = (mfxU8 *)hdl;
    mfxCapsWrapper *capsWrapper = *(mfxCapsWrapper **)(capsPtr - 8);

    // for read-only IMPLCAPS objects, there is nothing to release so we did not even create a mfxCapsWrapper
    // instead the handle was wrapped with mfxCapsWrapperReadonly (see definition above)
    if (!capsWrapper)
        return;

    mfxU32 implIdx    = capsWrapper->implIdx;
    mfxHDL *implArray = capsWrapper->implArray;

    if (capsWrapper->implFormat == MFX_IMPLCAPS_IMPLDESCSTRUCTURE) {
        // free any caps format specific objects
        mfxImplDescription *localImplDesc = (mfxImplDescription *)implArray[implIdx];
        if (localImplDesc) {
            if (localImplDesc->Dec.Codecs)
                delete[] localImplDesc->Dec.Codecs;
            if (localImplDesc->Enc.Codecs)
                delete[] localImplDesc->Enc.Codecs;
            if (localImplDesc->VPP.Filters)
                delete[] localImplDesc->VPP.Filters;
        }
    }

    if (capsWrapper->basePtr)
        delete[] capsWrapper->basePtr;

    implArray[implIdx] = nullptr; // mark this impl as freed
    mfxU32 idx;
    for (idx = 0; idx < capsWrapper->numImpls; idx++) {
        if (implArray[idx])
            break;
    }

    // the last impl has been freed, so now delete the implArray
    if (idx == capsWrapper->numImpls) {
        delete[] capsWrapper->implArray;
    }

    delete capsWrapper;
}

#ifndef SKIP_NEW_FUNCTIONS
    #ifdef ONEVPL_EXPERIMENTAL

static std::map<mfxU32, mfxU32> CapsSizeMap = {
    { MFX_IMPLCAPS_IMPLDESCSTRUCTURE,    static_cast<mfxU32>(sizeof(mfxImplDescription))       },
    { MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS, static_cast<mfxU32>(sizeof(mfxImplementedFunctions))  },
    { MFX_IMPLCAPS_DEVICE_ID_EXTENDED,   static_cast<mfxU32>(sizeof(mfxExtendedDeviceId))      },
    { MFX_IMPLCAPS_SURFACE_TYPES,        static_cast<mfxU32>(sizeof(mfxSurfaceTypesSupported)) },
};

// end table formatting
// clang-format on

// allocate holder object with pointer just before the mfxHDL for the caps description
// layout the memory manually to avoid possible packing/alignment issues
// if we assumed C++17, this could be simplified with aligned "new" or std::aligned_malloc()
// the handle (void *) gets passed through the C API
static mfxHDL *AllocCapsHolder(mfxU32 implFormat, mfxU32 numImpls) {
    // get size of caps structure for this format
    auto it = CapsSizeMap.find(implFormat);
    if (it == CapsSizeMap.end())
        return nullptr; // invalid implFormat
    mfxU32 capsSize = it->second;

    mfxHDL *implArray = nullptr;
    try {
        // allocate space for array of handles
        implArray = new mfxHDL[numImpls]();

        for (mfxU32 implIdx = 0; implIdx < numImpls; implIdx++) {
            // 8 bytes  = pointer to holder description, sufficient for 32-bit or 64-bit)
            // capsSize = actual size of caps struct (for example, mfxImplDescription)
            // 7 bytes  = padding so that both holder pointer and caps struct can be aligned to 8 bytes
            mfxU32 holderSize = 8 + capsSize + 7;

            // basePtr may have any alignment, holderPtr and capsPtr are 8-byte aligned
            mfxU8 *basePtr   = new mfxU8[holderSize]();
            mfxHDL holderPtr = (mfxHDL)((mfxU64)(basePtr + 7) & (mfxU64)(~0x0007));
            mfxHDL capsPtr   = (mfxHDL)((mfxU8 *)holderPtr + 8);

            implArray[implIdx] = capsPtr;

            // allocated a mfxCapsWrapper and store its address at holderPtr
            mfxCapsWrapper *capsWrapper = new mfxCapsWrapper();
            *(mfxHDL *)(holderPtr)      = (mfxHDL)capsWrapper;

            capsWrapper->basePtr    = basePtr; // save base pointer so buffer can be freed
            capsWrapper->implFormat = implFormat;
            capsWrapper->numImpls   = numImpls;
            capsWrapper->implIdx    = implIdx;
            capsWrapper->implArray  = implArray;
        }
    }
    catch (...) {
        return nullptr; // memory alloc error
    }

    return implArray;
}

// allocate modifiable description struct in order to test property-based query
static mfxImplDescription **AllocDefaultImplDesc(mfxImplDescription *localImplDesc) {
    // allocate only the child structs which may be changed when using MFXQueryImplsProperties()
    // all other child structs can point to the read-only memory
    DecCodec *localDecoders;
    EncCodec *localEncoders;
    VPPFilter *localVPPFilters;

    try {
        // These are the child structs which may be changed when using MFXQueryImplsProperties()
        //   and all other child structs can keep the default values, including pointers to read-only memory.
        // Allocate enough codec/filter structs for the max number available (see minImplDesc).
        // For default query, all codec/filters are populated. For property-based query, only fill in the requested ones.
        localDecoders   = new DecCodec[minImplDesc.Dec.NumCodecs]();
        localEncoders   = new EncCodec[minImplDesc.Enc.NumCodecs]();
        localVPPFilters = new VPPFilter[minImplDesc.VPP.NumFilters]();
    }
    catch (...) {
        return nullptr;
    }

    // copy the contents of the read-only minImplDesc, including pointers to its child structs (some of which will be replaced)
    *localImplDesc = minImplDesc;

    // we have space for up to the maximum (default) of each codec/filter, but start with them all unset
    localImplDesc->Dec.NumCodecs = 0;
    localImplDesc->Dec.Codecs    = localDecoders;

    localImplDesc->Enc.NumCodecs = 0;
    localImplDesc->Enc.Codecs    = localEncoders;

    localImplDesc->VPP.NumFilters = 0;
    localImplDesc->VPP.Filters    = localVPPFilters;

    return nullptr;
}

// fill in prop with corresponding codec/filter description from minImplDesc
// srcIdx is the index of the codec/filter in minImplDesc

// DecCodec = mfxDecoderDescription::decoder
static mfxStatus AddPropByIdx(DecCodec &decCodec, mfxU32 srcIdx) {
    if (srcIdx >= minImplDesc.Dec.NumCodecs)
        return MFX_ERR_NOT_FOUND;

    decCodec = minImplDesc.Dec.Codecs[srcIdx];

    return MFX_ERR_NONE;
}

// EncCodec = mfxEncoderDescription::encoder
static mfxStatus AddPropByIdx(EncCodec &encCodec, mfxU32 srcIdx) {
    if (srcIdx >= minImplDesc.Enc.NumCodecs)
        return MFX_ERR_NOT_FOUND;

    encCodec = minImplDesc.Enc.Codecs[srcIdx];

    return MFX_ERR_NONE;
}

// VPPFilter = mfxVPPDescription::filter
static mfxStatus AddPropByIdx(VPPFilter &vppFilter, mfxU32 srcIdx) {
    if (srcIdx >= minImplDesc.VPP.NumFilters)
        return MFX_ERR_NOT_FOUND;

    vppFilter = minImplDesc.VPP.Filters[srcIdx];

    return MFX_ERR_NONE;
}

// add decoder CodecID if available
static mfxStatus AddPropByValue(mfxDecoderDescription &dec, mfxU32 CodecID) {
    mfxU32 idx;

    // first see if codec has already been added
    for (idx = 0; idx < dec.NumCodecs; idx++) {
        if (dec.Codecs[idx].CodecID == CodecID)
            return MFX_ERR_NONE;
    }

    // sanity check - should not happen (since there are no duplicates)
    if (dec.NumCodecs >= minImplDesc.Dec.NumCodecs)
        return MFX_ERR_UNSUPPORTED;

    // search for this codec in the full description, copy into local description if found
    for (idx = 0; idx < minImplDesc.Dec.NumCodecs; idx++) {
        if (minImplDesc.Dec.Codecs[idx].CodecID == CodecID) {
            dec.Codecs[dec.NumCodecs] = minImplDesc.Dec.Codecs[idx];
            dec.NumCodecs++;
            return MFX_ERR_NONE;
        }
    }

    // codec was not found in full description
    return MFX_ERR_NOT_FOUND;
}

// add encoder CodecID if available
static mfxStatus AddPropByValue(mfxEncoderDescription &enc, mfxU32 CodecID) {
    mfxU32 idx;

    // first see if codec has already been added
    for (idx = 0; idx < enc.NumCodecs; idx++) {
        if (enc.Codecs[idx].CodecID == CodecID)
            return MFX_ERR_NONE;
    }

    // sanity check - should not happen (since there are no duplicates)
    if (enc.NumCodecs >= minImplDesc.Enc.NumCodecs)
        return MFX_ERR_UNSUPPORTED;

    // search for this codec in the full description, copy into local description if found
    for (idx = 0; idx < minImplDesc.Enc.NumCodecs; idx++) {
        if (minImplDesc.Enc.Codecs[idx].CodecID == CodecID) {
            enc.Codecs[enc.NumCodecs] = minImplDesc.Enc.Codecs[idx];
            enc.NumCodecs++;
            return MFX_ERR_NONE;
        }
    }

    // codec was not found in full description
    return MFX_ERR_NOT_FOUND;
}

// add VPP FilterFourCC if available
static mfxStatus AddPropByValue(mfxVPPDescription &vpp, mfxU32 FilterFourCC) {
    mfxU32 idx;

    // first see if codec has already been added
    for (idx = 0; idx < vpp.NumFilters; idx++) {
        if (vpp.Filters[idx].FilterFourCC == FilterFourCC)
            return MFX_ERR_NONE;
    }

    // sanity check - should not happen (since there are no duplicates)
    if (vpp.NumFilters >= minImplDesc.VPP.NumFilters)
        return MFX_ERR_UNSUPPORTED;

    // search for this codec in the full description, copy into local description if found
    for (idx = 0; idx < minImplDesc.VPP.NumFilters; idx++) {
        if (minImplDesc.VPP.Filters[idx].FilterFourCC == FilterFourCC) {
            vpp.Filters[vpp.NumFilters] = minImplDesc.VPP.Filters[idx];
            vpp.NumFilters++;
            return MFX_ERR_NONE;
        }
    }

    // codec was not found in full description
    return MFX_ERR_NOT_FOUND;
}

// from RT implementation
typedef enum {
    // Only fill top-level shallow fields in mfxImplDescription, not query into decoder/encoder/vpp
    QueryProp_ImplDescription = 0,

    // Query any codec or set of codecs for decode/encoder
    // Query any VPP filter or set of filters​
    QueryProp_DecCodecID,
    QueryProp_EncCodecID,
    QueryProp_VPPFilterFourCC,

    // Query all decoder or encoder or VPP capabilities​
    QueryProp_DecAll,
    QueryProp_EncAll,
    QueryProp_VPPAll,

    // Query all and fill all fields in mfxImplDescription
    QueryProp_All,

    QueryProp_TotalProps
} QueryPropAction;

static const std::map<std::string, QueryPropAction> QueryPropMap = {
    { "mfxImplDescription", QueryProp_ImplDescription },

    { "mfxImplDescription.mfxDecoderDescription.decoder.CodecID", QueryProp_DecCodecID },
    { "mfxImplDescription.mfxEncoderDescription.encoder.CodecID", QueryProp_EncCodecID },
    { "mfxImplDescription.mfxVPPDescription.filter.FilterFourCC", QueryProp_VPPFilterFourCC },

    { "mfxImplDescription.mfxDecoderDescription", QueryProp_DecAll },
    { "mfxImplDescription.mfxEncoderDescription", QueryProp_EncAll },
    { "mfxImplDescription.mfxVPPDescription", QueryProp_VPPAll },
};

mfxHDL *MFXQueryImplsProperties(mfxQueryProperty **properties,
                                mfxU32 num_properties,
                                mfxU32 *num_impls) {
    if (!properties || !num_impls)
        return nullptr;

    // scan through the properties list before allocating any memory, return error if any unknown props
    for (mfxU32 propIdx = 0; propIdx < num_properties; propIdx++) {
        mfxQueryProperty *prop = properties[propIdx];
        auto it                = QueryPropMap.find(std::string((char *)prop->PropName));
        if (it == QueryPropMap.end())
            return nullptr;
    }

    *num_impls = NUM_CPU_IMPLS;

    // currently only mfxImplDescription properties are supported, so allocate type MFX_IMPLCAPS_IMPLDESCSTRUCTURE
    // in the future other caps could be enabled, and the returned IMPLCAPS format would be based on the
    //    first part of the string (such as "mfxImplDescription.")
    // require that each call to MFXQueryImplsProperties() includes properties of the same IMPLCAPS type
    mfxHDL *localImplDescArray = AllocCapsHolder(MFX_IMPLCAPS_IMPLDESCSTRUCTURE, NUM_CPU_IMPLS);

    // only one implementation
    mfxImplDescription *localImplDesc = (mfxImplDescription *)localImplDescArray[0];
    AllocDefaultImplDesc(localImplDesc);

    for (mfxU32 propIdx = 0; propIdx < num_properties; propIdx++) {
        // we already validated property names above, so assume they are all some valid string
        mfxQueryProperty *prop     = properties[propIdx];
        auto it                    = QueryPropMap.find(std::string((char *)prop->PropName));
        QueryPropAction propAction = it->second;

        // top-level description is already filled in by AllocDefaultImplDesc
        if (propAction == QueryProp_ImplDescription) {
            continue;
        }

        // add all supported codecs/filters
        if (propAction == QueryProp_DecAll) {
            for (mfxU32 idx = 0; idx < minImplDesc.Dec.NumCodecs; idx++)
                AddPropByIdx(localImplDesc->Dec.Codecs[idx], idx);
            localImplDesc->Dec.NumCodecs = minImplDesc.Dec.NumCodecs;
        }
        else if (propAction == QueryProp_EncAll) {
            for (mfxU32 idx = 0; idx < minImplDesc.Enc.NumCodecs; idx++)
                AddPropByIdx(localImplDesc->Enc.Codecs[idx], idx);
            localImplDesc->Enc.NumCodecs = minImplDesc.Enc.NumCodecs;
        }
        else if (propAction == QueryProp_VPPAll) {
            for (mfxU32 idx = 0; idx < minImplDesc.VPP.NumFilters; idx++)
                AddPropByIdx(localImplDesc->VPP.Filters[idx], idx);
            localImplDesc->VPP.NumFilters = minImplDesc.VPP.NumFilters;
        }

        // add one codec/filter based on ID (FourCC)
        // if not found, AddPropByValue() returns an error but we just keep going rather than error out
        //   because this implementation will get filtered out when the requested property is not supported
        if (propAction == QueryProp_DecCodecID) {
            mfxU32 codecID = prop->PropVar.Data.U32;
            AddPropByValue(localImplDesc->Dec, codecID);
        }
        else if (propAction == QueryProp_EncCodecID) {
            mfxU32 codecID = prop->PropVar.Data.U32;
            AddPropByValue(localImplDesc->Enc, codecID);
        }
        else if (propAction == QueryProp_VPPFilterFourCC) {
            mfxU32 filterFourCC = prop->PropVar.Data.U32;
            AddPropByValue(localImplDesc->VPP, filterFourCC);
        }
    }

    return (mfxHDL *)(localImplDescArray);
}
    #else
// define dummy function to avoid link error if ONEVPL_EXPERIMENTAL is disabled (preprocessor does not apply to .def file)
mfxHDL *MFXQueryImplsProperties(void **properties, mfxU32 num_properties, mfxU32 *num_impls) {
    return nullptr;
}
    #endif
#endif

// walk through implDesc and delete dynamically-allocated structs
mfxStatus MFXReleaseImplDescription(mfxHDL hdl) {
    if (!hdl)
        return MFX_ERR_NULL_PTR;

    ReleaseImplDesc(hdl);

    return MFX_ERR_NONE;
}

// must be implemented else MFXCreateSession() will fail
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

mfxStatus MFXQueryIMPL(mfxSession session, mfxIMPL *impl) {
    if (0 == session) {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == impl) {
        return MFX_ERR_NULL_PTR;
    }

    *impl = DBG_VALID_IMPL_CFG_ALL;

    return MFX_ERR_NONE;
}
