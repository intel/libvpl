/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <iostream>
#include <ostream>

#include "src/caps.h"
#include "src/config.h"

// the auto-generated capabilities structs
// only include one time in this library
#include "src/caps_dec_none.h"
#include "src/caps_vpp_none.h"

#ifdef ENABLE_STUB_1X
    #include "src/caps_enc_none.h"
#else
    // include some actual stub encoder caps
    #include "src/caps_enc.h"
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

    _mfxSession *stubSession = new _mfxSession;
    if (!stubSession)
        return MFX_ERR_MEMORY_ALLOC;

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

    _mfxSession *stubSession = new _mfxSession;
    if (!stubSession)
        return MFX_ERR_MEMORY_ALLOC;

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
    _mfxSession *cloneSession = new _mfxSession;
    if (!cloneSession)
        return MFX_ERR_MEMORY_ALLOC;

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

static const mfxImplDescription minImplDesc = {
    {{ 2, 1 }},                                     // struct Version
    MFX_IMPL_TYPE_SOFTWARE,                         // Impl
    MFX_ACCEL_MODE_NA,                              // AccelerationMode
#ifdef ENABLE_STUB_1X
    {{ 99, 1}},                                     // ApiVersion
    "Stub Implementation 1X",                       // ImplName
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

static const mfxImplDescription *minImplDescArray[NUM_CPU_IMPLS] = {
    &minImplDesc,
};

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
};

static const mfxImplementedFunctions minImplFuncs = {
    sizeof(minImplFuncsNames) / sizeof(mfxChar *),
    (mfxChar**)minImplFuncsNames
};

static const mfxImplementedFunctions *minImplFuncsArray[NUM_CPU_IMPLS] = {
    &minImplFuncs,
};

#ifndef ENABLE_STUB_1X
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

static const mfxExtendedDeviceId *minExtDeviceIDArray[NUM_CPU_IMPLS] = {
    &minExtDeviceID,
};

#ifdef ONEVPL_EXPERIMENTAL
static const mfxSurfaceTypesSupported *minSurfTypesSupportedArray[NUM_CPU_IMPLS] = {
    &minSurfTypesSupported,
};
#endif

#endif


// end table formatting
// clang-format on

// query and release are independent of session - called during
//   caps query and config stage using Intel® Video Processing Library (Intel® VPL) extensions
mfxHDL *MFXQueryImplsDescription(mfxImplCapsDeliveryFormat format, mfxU32 *num_impls) {
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

// walk through implDesc and delete dynamically-allocated structs
mfxStatus MFXReleaseImplDescription(mfxHDL hdl) {
    if (!hdl)
        return MFX_ERR_NULL_PTR;

    // nothing to do - caps are stored in ROM table

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
