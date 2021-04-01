/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "vpl/mfx_dispatcher_vpl.h"

#ifdef __linux__
    #include <pthread.h>
    #define strncpy_s(dst, size, src, cnt) strcpy((dst), (src)) // NOLINT
#endif

// leave table formatting alone
// clang-format off

static const mfxChar strImplName[MFX_IMPL_NAME_LEN] = "MSDK RT (compatibility mode)";
static const mfxChar strLicense[MFX_STRFIELD_LEN]   = "MIT";

#if defined _M_IX86
static const mfxChar strKeywords[MFX_STRFIELD_LEN] = "MSDK,x86";
#else
static const mfxChar strKeywords[MFX_STRFIELD_LEN] = "MSDK,x64";
#endif

static const mfxAccelerationMode MSDKAccelModes[] = {
#ifdef __linux__
    MFX_ACCEL_MODE_VIA_VAAPI,
#else
    MFX_ACCEL_MODE_VIA_D3D9,
    MFX_ACCEL_MODE_VIA_D3D11,
#endif
};

// 1.x function names should match list in enum eFunc
static const mfxChar* msdkImplFuncsNames[] = {
    "MFXInit",
    "MFXClose",
    "MFXQueryIMPL",
    "MFXQueryVersion",
    "MFXJoinSession",
    "MFXDisjoinSession",
    "MFXCloneSession",
    "MFXSetPriority",
    "MFXGetPriority",
    "MFXInitEx",
    "MFXVideoCORE_SetFrameAllocator",
    "MFXVideoCORE_SetHandle",
    "MFXVideoCORE_GetHandle",
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
    "MFXVideoCORE_QueryPlatform",
};

static const mfxImplementedFunctions msdkImplFuncs = {
    sizeof(msdkImplFuncsNames) / sizeof(mfxChar*),
    (mfxChar**)msdkImplFuncsNames
};

// end table formatting
// clang-format on

static const mfxIMPL hwImplTypes[] = {
    MFX_IMPL_HARDWARE,
    MFX_IMPL_HARDWARE2,
    MFX_IMPL_HARDWARE3,
    MFX_IMPL_HARDWARE4,
};

LoaderCtxMSDK::LoaderCtxMSDK()
        : msdkAdapter(),
          m_libNameFull(),
          m_id(),
          m_accelMode(),
          m_loaderDeviceID(0) {}

LoaderCtxMSDK::~LoaderCtxMSDK() {}

mfxStatus LoaderCtxMSDK::OpenSession(mfxSession* session,
                                     STRING_TYPE libNameFull,
                                     mfxAccelerationMode accelMode,
                                     mfxIMPL hwImpl) {
    // require API 1.0 or later (both MFXInit and MFXInitEx supported)
    mfxVersion reqVersion;
    reqVersion.Major = MSDK_MIN_VERSION_MAJOR;
    reqVersion.Minor = MSDK_MIN_VERSION_MINOR;

    // set acceleration mode - will be mapped to 1.x API
    mfxInitializationParam vplParam = {};
    vplParam.AccelerationMode       = accelMode;

    return MFXInitEx2(reqVersion,
                      vplParam,
                      hwImpl,
                      session,
                      &m_loaderDeviceID,
                      (CHAR_TYPE*)libNameFull.c_str());
}

// safe to call more than once (sets/checks for null session)
void LoaderCtxMSDK::CloseSession(mfxSession* session) {
    if (*session)
        MFXClose(*session);

    *session = nullptr;
}

// map mfxIMPL (1.x) to mfxAccelerationMode (2.x)
mfxAccelerationMode LoaderCtxMSDK::CvtAccelType(mfxIMPL implType, mfxIMPL implMethod) {
    if (implType == MFX_IMPL_HARDWARE) {
        switch (implMethod) {
            case MFX_IMPL_VIA_D3D9:
                return MFX_ACCEL_MODE_VIA_D3D9;
            case MFX_IMPL_VIA_D3D11:
                return MFX_ACCEL_MODE_VIA_D3D11;
            case MFX_IMPL_VIA_VAAPI:
                return MFX_ACCEL_MODE_VIA_VAAPI;
        }
    }

    return MFX_ACCEL_MODE_NA;
}

mfxStatus LoaderCtxMSDK::GetDefaultAccelType(mfxU32 adapterID, mfxIMPL* implDefault) {
#ifdef __linux__
    // VAAPI only
    *implDefault = MFX_IMPL_VIA_VAAPI;
    return MFX_ERR_NONE;
#else
    // get default acceleration modes
    mfxU32 VendorID = 0, DeviceID = 0;
    mfxIMPL implTest;
    mfxStatus sts;

    implTest = MFX_IMPL_VIA_ANY;
    sts      = MFX::SelectImplementationType(adapterID, &implTest, &VendorID, &DeviceID);

    if (sts != MFX_ERR_NONE || VendorID != 0x8086) {
        implTest = MFX_IMPL_UNSUPPORTED;
        return MFX_ERR_UNSUPPORTED;
    }

    *implDefault = implTest;

    return MFX_ERR_NONE;
#endif
}

mfxStatus LoaderCtxMSDK::CheckAccelType(mfxU32 adapterID, mfxIMPL implTest) {
#ifdef __linux__
    // VAAPI only
    if (implTest != MFX_IMPL_VIA_VAAPI)
        return MFX_ERR_UNSUPPORTED;

    return MFX_ERR_NONE;
#else
    // get default acceleration modes
    mfxU32 VendorID = 0, DeviceID = 0;
    mfxStatus sts;

    sts = MFX::SelectImplementationType(adapterID, &implTest, &VendorID, &DeviceID);

    if (sts != MFX_ERR_NONE || VendorID != 0x8086)
        return MFX_ERR_UNSUPPORTED;

    return MFX_ERR_NONE;
#endif
}

mfxStatus LoaderCtxMSDK::QueryMSDKCaps(STRING_TYPE libNameFull,
                                       mfxImplDescription** implDesc,
                                       mfxImplementedFunctions** implFuncs,
                                       mfxIMPL* msdkAdapter) {
    mfxStatus sts;
    mfxSession session;

    m_libNameFull = libNameFull;

    mfxIMPL msdkImplType = MFX_IMPL_UNSUPPORTED;
    *msdkAdapter         = MFX_IMPL_UNSUPPORTED;

#ifdef __linux__
    // require pthreads to be linked in for MSDK RT to load
    pthread_key_t pkey;
    if (pthread_key_create(&pkey, NULL) == 0) {
        pthread_key_delete(pkey);
    }
#endif

    // try HW session, default acceleration mode, search for first supported device
    mfxIMPL hwImpl      = MFX_IMPL_AUTO;
    mfxIMPL implDefault = MFX_IMPL_UNSUPPORTED;
    for (mfxU32 i = 0; i < TAB_SIZE(mfxIMPL, hwImplTypes); i++) {
        hwImpl = hwImplTypes[i];

        sts = GetDefaultAccelType(i, &implDefault);
        if (sts != MFX_ERR_NONE) {
            // unsupported
            continue;
        }

        sts =
            OpenSession(&session,
                        m_libNameFull,
                        (mfxAccelerationMode)CvtAccelType(MFX_IMPL_HARDWARE, implDefault & 0xFF00),
                        hwImpl);

        if (sts == MFX_ERR_NONE) {
            // hwImpl now indicates which adapter succeeded
            msdkImplType = MFX_IMPL_HARDWARE;
            break;
        }
    }

#ifdef ENABLE_MSDK_SW_FALLBACK
    // if HW failed, try SW session
    if (msdkImplType == MFX_IMPL_UNSUPPORTED) {
        sts = OpenSession(&session, m_libNameFull, MFX_ACCEL_MODE_NA, MFX_IMPL_SOFTWARE);
        if (sts == MFX_ERR_NONE) {
            msdkImplType = MFX_IMPL_SOFTWARE;
        }
    }
#endif

    // failed to initialize with any impl type
    if (msdkImplType == MFX_IMPL_UNSUPPORTED)
        return MFX_ERR_UNSUPPORTED;

    // failed to initialize with any impl type
    if (sts != MFX_ERR_NONE)
        return sts;

    // return list of implemented functions
    *implFuncs = (mfxImplementedFunctions*)(&msdkImplFuncs);

    // clear new 2.0 style description struct
    memset(&m_id, 0, sizeof(mfxImplDescription));
    *implDesc = &m_id;

    // fill in top-level capabilities
    m_id.Version.Version = MFX_IMPLDESCRIPTION_VERSION;

    // query API version
    sts = MFXQueryVersion(session, &m_id.ApiVersion);
    if (sts != MFX_ERR_NONE)
        return sts;

    // fill in acceleration description struct
    mfxAccelerationModeDescription* accelDesc = &(m_id.AccelerationModeDescription);
    accelDesc->Version.Version                = MFX_ACCELERATIONMODESCRIPTION_VERSION;
    accelDesc->NumAccelerationModes           = 0;
    accelDesc->Mode                           = m_accelMode;

    if (msdkImplType == MFX_IMPL_HARDWARE) {
        // query actual implementation for this session
        mfxIMPL actualImpl = {};
        sts                = MFXQueryIMPL(session, &actualImpl);
        if (sts != MFX_ERR_NONE)
            return sts;

        // return HW accelerator - required by MFXCreateSession
        *msdkAdapter = hwImpl;

        // map MFX HW number to VendorImplID
        m_id.VendorImplID = 0;
        switch (hwImpl) {
            case MFX_IMPL_HARDWARE:
                m_id.VendorImplID = 0;
                break;
            case MFX_IMPL_HARDWARE2:
                m_id.VendorImplID = 1;
                break;
            case MFX_IMPL_HARDWARE3:
                m_id.VendorImplID = 2;
                break;
            case MFX_IMPL_HARDWARE4:
                m_id.VendorImplID = 3;
                break;
        }

        // set default acceleration mode
        m_id.AccelerationMode = CvtAccelType(MFX_IMPL_HARDWARE, implDefault & 0xFF00);

        CloseSession(&session);

        // hardware - test for all supported acceleration modes
        m_id.Impl = MFX_IMPL_TYPE_HARDWARE;
        for (mfxU32 i = 0; i < TAB_SIZE(mfxAccelerationMode, MSDKAccelModes); i++) {
            // check if this accelerator type is supported
            if (CheckAccelType(m_id.VendorImplID, MSDKAccelModes[i]) != MFX_ERR_NONE)
                continue;

            sts = OpenSession(&session, m_libNameFull, MSDKAccelModes[i], hwImpl);

            if (sts == MFX_ERR_NONE) {
                mfxU16 m = accelDesc->NumAccelerationModes;

                accelDesc->Mode[m] = MSDKAccelModes[i];
                accelDesc->NumAccelerationModes++;

                CloseSession(&session);
            }
        }

        // at least one acceleration mode must be supported
        if (accelDesc->NumAccelerationModes == 0)
            return MFX_ERR_UNSUPPORTED;

        // if QueryIMPL did not set VIA flag, set default to first supported mode
        if (!m_id.AccelerationMode)
            m_id.AccelerationMode = accelDesc->Mode[0];

        // reopen session with default mode
        sts = OpenSession(&session, m_libNameFull, m_id.AccelerationMode, hwImpl);
        if (sts != MFX_ERR_NONE)
            return sts;
    }
    else {
        // software - set default accel mode
        m_id.Impl             = MFX_IMPL_TYPE_SOFTWARE;
        m_id.AccelerationMode = MFX_ACCEL_MODE_NA;
        m_id.VendorImplID     = 0;

        accelDesc->NumAccelerationModes = 1;
        accelDesc->Mode[0]              = MFX_ACCEL_MODE_NA;
    }

    // fill in strings
    strncpy_s(m_id.ImplName, sizeof(m_id.ImplName), strImplName, sizeof(strImplName));
    strncpy_s(m_id.License, sizeof(m_id.License), strLicense, sizeof(strLicense));
    strncpy_s(m_id.Keywords, sizeof(m_id.Keywords), strKeywords, sizeof(strKeywords));

    m_id.VendorID    = 0x8086;
    m_id.NumExtParam = 0;

    // fill in device description
    mfxDeviceDescription* Dev = &(m_id.Dev);
    memset(Dev, 0, sizeof(mfxDeviceDescription)); // initially empty

    // query for underlying deviceID (requires API >= 1.19)
    mfxU16 deviceID = 0x0000;
    if (IsVersionSupported(MAKE_MFX_VERSION(1, 19), m_id.ApiVersion)) {
        mfxPlatform platform = {};

        sts = MFXVideoCORE_QueryPlatform(session, &platform);
        if (sts == MFX_ERR_NONE)
            deviceID = platform.DeviceId;
    }

    // if QueryPlatform did not return deviceID, we may have received
    //   it from the loader (MFXInitEx2)
    if (deviceID == 0)
        deviceID = m_loaderDeviceID;

    Dev->Version.Version = MFX_DEVICEDESCRIPTION_VERSION;
    snprintf(Dev->DeviceID, sizeof(Dev->DeviceID), "%04x", deviceID);
    Dev->NumSubDevices = 0;

    CloseSession(&session);

    return MFX_ERR_NONE;
}
