/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "vpl/mfx.h"

#include "src/caps.h"

// the auto-generated capabilities structs
// only include one time in this library
#include "src/caps_dec_none.h"
#include "src/caps_enc_none.h"
#include "src/caps_vpp_none.h"

// preferred entrypoint for 2.0 implementations (instead of MFXInitEx)
mfxStatus MFXInitialize(mfxInitializationParam par, mfxSession *session) {
    if (!session)
        return MFX_ERR_NULL_PTR;

    *session = (mfxSession)0x01;

    return MFX_ERR_NONE;
}

mfxStatus MFXClose(mfxSession session) {
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
    { MFX_VERSION_MINOR, MFX_VERSION_MAJOR },       // ApiVersion

    "Stub Implementation",                          // ImplName

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
