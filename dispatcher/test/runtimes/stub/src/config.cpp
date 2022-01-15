/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <stdarg.h>
#include <stdio.h>

#include "vpl/mfx.h"

#include "src/caps.h"

// the auto-generated capabilities structs
// only include one time in this library
#include "src/caps_dec_none.h"
#include "src/caps_enc_none.h"
#include "src/caps_vpp_none.h"

#define DEFAULT_SESSION_HANDLE_1X 0x01
#define DEFAULT_SESSION_HANDLE_2X 0x02

// print messages to be parsed in unit tests to stdout, and other errors to stderr
static void StubRTLogMessage(const char *msg, ...) {
    fprintf(stdout, "[STUB RT]: message -- ");

    va_list args;
    va_start(args, msg);
    vfprintf(stdout, msg, args);
    va_end(args);
}

static void StubRTLogError(const char *msg, ...) {
    fprintf(stderr, "[STUB RT]: ERROR -- ");
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
}

static mfxStatus ValidateExtBuf(const char *strInitFunc,
                                mfxExtBuffer *extBuf,
                                mfxU32 expectedId,
                                mfxU32 expectedSz) {
    if (extBuf == nullptr) {
        StubRTLogError("%s -- extBuf is NULL\n", strInitFunc);
        return MFX_ERR_NULL_PTR;
    }

    if (extBuf->BufferId == expectedId) {
        if (extBuf->BufferSz != expectedSz) {
            StubRTLogError("%s -- invalid extBuf size (expected = %d, actual = %d)\n",
                           strInitFunc,
                           expectedSz,
                           extBuf->BufferSz);

            return MFX_ERR_UNSUPPORTED;
        }

        // valid buffer
        mfxExtThreadsParam *extBufThreads = (mfxExtThreadsParam *)extBuf;
        StubRTLogMessage("%s -- extBuf enabled -- NumThread (%d)",
                         strInitFunc,
                         extBufThreads->NumThread);

        return MFX_ERR_NONE;
    }

    // unsupported extBuf type
    return MFX_ERR_UNSUPPORTED;
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

        if (par.ExtParam[idx]->BufferId == MFX_EXTBUFF_THREADS_PARAM) {
            sts = ValidateExtBuf("MFXInitialize",
                                 par.ExtParam[idx],
                                 MFX_EXTBUFF_THREADS_PARAM,
                                 sizeof(mfxExtThreadsParam));
        }
        else {
            // unsupported BufferId
            sts = MFX_ERR_UNSUPPORTED;
        }

        if (sts != MFX_ERR_NONE)
            return sts;
    }

    *session = (mfxSession)DEFAULT_SESSION_HANDLE_2X;

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

        if (par.ExtParam[idx]->BufferId == MFX_EXTBUFF_THREADS_PARAM) {
            sts = ValidateExtBuf("MFXInitEx",
                                 par.ExtParam[idx],
                                 MFX_EXTBUFF_THREADS_PARAM,
                                 sizeof(mfxExtThreadsParam));
        }
        else {
            // unsupported BufferId
            sts = MFX_ERR_UNSUPPORTED;
        }

        if (sts != MFX_ERR_NONE)
            return sts;
    }

    *session = (mfxSession)DEFAULT_SESSION_HANDLE_1X;

    return MFX_ERR_NONE;
}

mfxStatus MFXClose(mfxSession session) {
    if (!session)
        return MFX_ERR_INVALID_HANDLE;

    mfxU64 s = (mfxU64)session;
    if (s != DEFAULT_SESSION_HANDLE_1X && s != DEFAULT_SESSION_HANDLE_2X)
        return MFX_ERR_INVALID_HANDLE;

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
    { 2, 1 },                                       // struct Version
    MFX_IMPL_TYPE_SOFTWARE,                         // Impl
    MFX_ACCEL_MODE_NA,                              // AccelerationMode
#ifdef ENABLE_STUB_1X
    { 99, 1},                                       // ApiVersion
    "Stub Implementation 1X",                       // ImplName
#else
    { MFX_VERSION_MINOR, MFX_VERSION_MAJOR },       // ApiVersion
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
        { 1, 1 },          // struct Version
        {},                // reserved
        MFX_MEDIA_UNKNOWN, // MediaAdapterType
        "0000",            // DeviceID
        0,                 // NumSubDevices
        {},                // SubDevices
    },

    // mfxDecoderDescription Dec
    {
        { decoderDesc.Version.Minor, decoderDesc.Version.Major },
        {},
        decoderDesc.NumCodecs,
        (DecCodec *)nullptr,
    },

    // mfxEncoderDescription Enc
    {
        { encoderDesc.Version.Minor, encoderDesc.Version.Major },
        {},
        encoderDesc.NumCodecs,
        (EncCodec *)nullptr,
    },

    // mfxVPPDescription VPP
    {
        { vppDesc.Version.Minor, vppDesc.Version.Major },
        {},
        vppDesc.NumFilters,
        (VPPFilter *)nullptr,
    },

    // union { mfxAccelerationModeDescription AccelerationModeDescription }
    { {
        { 0, 1 },
        {},
        NUM_ACCELERATION_MODES_CPU,
        (mfxAccelerationMode *)AccelerationMode,
    } },

    {
        { 0, 1 },
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

// end table formatting
// clang-format on

// query and release are independent of session - called during
//   caps query and config stage using oneVPL extensions
mfxHDL *MFXQueryImplsDescription(mfxImplCapsDeliveryFormat format, mfxU32 *num_impls) {
    *num_impls = NUM_CPU_IMPLS;

    if (format == MFX_IMPLCAPS_IMPLDESCSTRUCTURE) {
        return (mfxHDL *)(minImplDescArray);
    }
    else if (format == MFX_IMPLCAPS_IMPLEMENTEDFUNCTIONS) {
        return (mfxHDL *)(minImplFuncsArray);
    }
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
