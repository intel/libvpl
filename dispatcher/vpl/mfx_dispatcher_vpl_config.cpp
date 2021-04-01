/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "vpl/mfx_dispatcher_vpl.h"

#include <assert.h>

// implementation of config context (mfxConfig)
// each loader instance can have one or more configs
//   associated with it - used for filtering implementations
//   based on what they support (codec types, etc.)
ConfigCtxVPL::ConfigCtxVPL()
        : m_propName(),
          m_propValue(),
          m_propIdx(),
          m_propParsedString(),
          m_propRange32U(),
          m_implName(),
          m_implLicense(),
          m_implKeywords(),
          m_implFunctionName() {
    m_propValue.Version.Version = MFX_VARIANT_VERSION;
    m_propValue.Type            = MFX_VARIANT_TYPE_UNSET;
    m_parentLoader              = nullptr;
    return;
}

ConfigCtxVPL::~ConfigCtxVPL() {
    return;
}

struct PropVariant {
    const char *Name;
    mfxVariantType Type;
};

enum PropIdx {
    // settable config properties for mfxImplDescription
    ePropMain_Impl = 0,
    ePropMain_AccelerationMode,
    ePropMain_ApiVersion,
    ePropMain_ApiVersion_Major,
    ePropMain_ApiVersion_Minor,
    ePropMain_ImplName,
    ePropMain_License,
    ePropMain_Keywords,
    ePropMain_VendorID,
    ePropMain_VendorImplID,

    // settable config properties for mfxDeviceDescription
    ePropDevice_DeviceID,

    // settable config properties for mfxDecoderDescription
    ePropDec_CodecID,
    ePropDec_MaxcodecLevel,
    ePropDec_Profile,
    ePropDec_MemHandleType,
    ePropDec_Width,
    ePropDec_Height,
    ePropDec_ColorFormats,

    // settable config properties for mfxEncoderDescription
    ePropEnc_CodecID,
    ePropEnc_MaxcodecLevel,
    ePropEnc_BiDirectionalPrediction,
    ePropEnc_Profile,
    ePropEnc_MemHandleType,
    ePropEnc_Width,
    ePropEnc_Height,
    ePropEnc_ColorFormats,

    // settable config properties for mfxVPPDescription
    ePropVPP_FilterFourCC,
    ePropVPP_MaxDelayInFrames,
    ePropVPP_MemHandleType,
    ePropVPP_Width,
    ePropVPP_Height,
    ePropVPP_InFormat,
    ePropVPP_OutFormat,

    // special properties not part of description struct
    ePropSpecial_HandleType,
    ePropSpecial_Handle,

    // functions which must report as implemented
    ePropFunc_FunctionName,

    // number of entries (always last)
    eProp_TotalProps
};

// leave table formatting alone
// clang-format off

// order must align exactly with PropIdx list
// to avoid mismatches, this should be automated (e.g. pre-processor step)
static const PropVariant PropIdxTab[] = {
    { "ePropMain_Impl",                     MFX_VARIANT_TYPE_U32 },
    { "ePropMain_AccelerationMode",         MFX_VARIANT_TYPE_U32 },
    { "ePropMain_ApiVersion",               MFX_VARIANT_TYPE_U32 },
    { "ePropMain_ApiVersion_Major",         MFX_VARIANT_TYPE_U16 },
    { "ePropMain_ApiVersion_Minor",         MFX_VARIANT_TYPE_U16 },
    { "ePropMain_ImplName",                 MFX_VARIANT_TYPE_PTR },
    { "ePropMain_License",                  MFX_VARIANT_TYPE_PTR },
    { "ePropMain_Keywords",                 MFX_VARIANT_TYPE_PTR },
    { "ePropMain_VendorID",                 MFX_VARIANT_TYPE_U32 },
    { "ePropMain_VendorImplID",             MFX_VARIANT_TYPE_U32 },

    { "ePropDevice_DeviceID",                 MFX_VARIANT_TYPE_U16 },

    { "ePropDec_CodecID",                   MFX_VARIANT_TYPE_U32 },
    { "ePropDec_MaxcodecLevel",             MFX_VARIANT_TYPE_U16 },
    { "ePropDec_Profile",                   MFX_VARIANT_TYPE_U32 },
    { "ePropDec_MemHandleType",             MFX_VARIANT_TYPE_U32 },
    { "ePropDec_Width",                     MFX_VARIANT_TYPE_PTR },
    { "ePropDec_Height",                    MFX_VARIANT_TYPE_PTR },
    { "ePropDec_ColorFormats",              MFX_VARIANT_TYPE_U32 },

    { "ePropEnc_CodecID",                   MFX_VARIANT_TYPE_U32 },
    { "ePropEnc_MaxcodecLevel",             MFX_VARIANT_TYPE_U16 },
    { "ePropEnc_BiDirectionalPrediction",   MFX_VARIANT_TYPE_U16 },
    { "ePropEnc_Profile",                   MFX_VARIANT_TYPE_U32 },
    { "ePropEnc_MemHandleType",             MFX_VARIANT_TYPE_U32 },
    { "ePropEnc_Width",                     MFX_VARIANT_TYPE_PTR },
    { "ePropEnc_Height",                    MFX_VARIANT_TYPE_PTR },
    { "ePropEnc_ColorFormats",              MFX_VARIANT_TYPE_U32 },

    { "ePropVPP_FilterFourCC",              MFX_VARIANT_TYPE_U32 },
    { "ePropVPP_MaxDelayInFrames",          MFX_VARIANT_TYPE_U16 },
    { "ePropVPP_MemHandleType",             MFX_VARIANT_TYPE_U32 },
    { "ePropVPP_Width",                     MFX_VARIANT_TYPE_PTR },
    { "ePropVPP_Height",                    MFX_VARIANT_TYPE_PTR },
    { "ePropVPP_InFormat",                  MFX_VARIANT_TYPE_U32 },
    { "ePropVPP_OutFormat",                 MFX_VARIANT_TYPE_U32 },

    { "ePropSpecial_HandleType",            MFX_VARIANT_TYPE_U32 },
    { "ePropSpecial_Handle",                MFX_VARIANT_TYPE_PTR },

    { "ePropFunc_FunctionName",             MFX_VARIANT_TYPE_PTR },
};

// end table formatting
// clang-format on

// sanity check - property table and indexes must have same number of entries
static_assert((sizeof(PropIdxTab) / sizeof(PropVariant)) == eProp_TotalProps,
              "PropIdx and PropIdxTab are misaligned");

mfxStatus ConfigCtxVPL::ValidateAndSetProp(mfxI32 idx, mfxVariant value) {
    if (idx < 0 || idx >= eProp_TotalProps)
        return MFX_ERR_NOT_FOUND;

    if (value.Type != PropIdxTab[idx].Type)
        return MFX_ERR_UNSUPPORTED;

    m_propIdx        = idx;
    m_propValue.Type = value.Type;

    if (value.Type == MFX_VARIANT_TYPE_PTR) {
        if (value.Data.Ptr == nullptr)
            return MFX_ERR_NULL_PTR;

        // save copy of data passed by pointer, into object of the appropriate type
        switch (m_propIdx) {
            case ePropDec_Width:
            case ePropDec_Height:
            case ePropEnc_Width:
            case ePropEnc_Height:
            case ePropVPP_Width:
            case ePropVPP_Height:
                m_propRange32U       = *((mfxRange32U *)(value.Data.Ptr));
                m_propValue.Data.Ptr = &(m_propRange32U);
                break;
            case ePropSpecial_Handle:
                m_propValue.Data.Ptr = (mfxHDL)(value.Data.Ptr);
                break;
            case ePropMain_ImplName:
                m_implName           = (char *)(value.Data.Ptr);
                m_propValue.Data.Ptr = &(m_implName);
                break;
            case ePropMain_License:
                m_implLicense        = (char *)(value.Data.Ptr);
                m_propValue.Data.Ptr = &(m_implLicense);
                break;
            case ePropMain_Keywords:
                m_implKeywords       = (char *)(value.Data.Ptr);
                m_propValue.Data.Ptr = &(m_implKeywords);
                break;
            case ePropFunc_FunctionName:
                // no need to save Data.Ptr - parsed in main loop
                m_implFunctionName = (char *)(value.Data.Ptr);
                break;
            default:
                break;
        }
    }
    else {
        m_propValue.Data = value.Data;
    }

    return MFX_ERR_NONE;
}

mfxStatus ConfigCtxVPL::SetFilterPropertyDec(mfxVariant value) {
    std::string nextProp;

    nextProp = GetNextProp(&m_propParsedString);

    // no settable top-level members
    if (nextProp != "decoder")
        return MFX_ERR_NOT_FOUND;

    // parse 'decoder'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "CodecID") {
        return ValidateAndSetProp(ePropDec_CodecID, value);
    }
    else if (nextProp == "MaxcodecLevel") {
        return ValidateAndSetProp(ePropDec_MaxcodecLevel, value);
    }
    else if (nextProp != "decprofile") {
        return MFX_ERR_NOT_FOUND;
    }

    // parse 'decprofile'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "Profile") {
        return ValidateAndSetProp(ePropDec_Profile, value);
    }
    else if (nextProp != "decmemdesc") {
        return MFX_ERR_NOT_FOUND;
    }

    // parse 'decmemdesc'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "MemHandleType") {
        return ValidateAndSetProp(ePropDec_MemHandleType, value);
    }
    else if (nextProp == "Width") {
        return ValidateAndSetProp(ePropDec_Width, value);
    }
    else if (nextProp == "Height") {
        return ValidateAndSetProp(ePropDec_Height, value);
    }
    else if (nextProp == "ColorFormat" || nextProp == "ColorFormats") {
        return ValidateAndSetProp(ePropDec_ColorFormats, value);
    }

    // end of mfxDecoderDescription options
    return MFX_ERR_NOT_FOUND;
}

mfxStatus ConfigCtxVPL::SetFilterPropertyEnc(mfxVariant value) {
    std::string nextProp;

    nextProp = GetNextProp(&m_propParsedString);

    // no settable top-level members
    if (nextProp != "encoder")
        return MFX_ERR_NOT_FOUND;

    // parse 'encoder'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "CodecID") {
        return ValidateAndSetProp(ePropEnc_CodecID, value);
    }
    else if (nextProp == "MaxcodecLevel") {
        return ValidateAndSetProp(ePropEnc_MaxcodecLevel, value);
    }
    else if (nextProp == "BiDirectionalPrediction") {
        return ValidateAndSetProp(ePropEnc_BiDirectionalPrediction, value);
    }
    else if (nextProp != "encprofile") {
        return MFX_ERR_NOT_FOUND;
    }

    // parse 'encprofile'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "Profile") {
        return ValidateAndSetProp(ePropEnc_Profile, value);
    }
    else if (nextProp != "encmemdesc") {
        return MFX_ERR_NOT_FOUND;
    }

    // parse 'encmemdesc'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "MemHandleType") {
        return ValidateAndSetProp(ePropEnc_MemHandleType, value);
    }
    else if (nextProp == "Width") {
        return ValidateAndSetProp(ePropEnc_Width, value);
    }
    else if (nextProp == "Height") {
        return ValidateAndSetProp(ePropEnc_Height, value);
    }
    else if (nextProp == "ColorFormat" || nextProp == "ColorFormats") {
        return ValidateAndSetProp(ePropEnc_ColorFormats, value);
    }

    // end of mfxEncoderDescription options
    return MFX_ERR_NOT_FOUND;
}

mfxStatus ConfigCtxVPL::SetFilterPropertyVPP(mfxVariant value) {
    std::string nextProp;

    nextProp = GetNextProp(&m_propParsedString);

    // no settable top-level members
    if (nextProp != "filter")
        return MFX_ERR_NOT_FOUND;

    // parse 'filter'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "FilterFourCC") {
        return ValidateAndSetProp(ePropVPP_FilterFourCC, value);
    }
    else if (nextProp == "MaxDelayInFrames") {
        return ValidateAndSetProp(ePropVPP_MaxDelayInFrames, value);
    }
    else if (nextProp != "memdesc") {
        return MFX_ERR_NOT_FOUND;
    }

    // parse 'memdesc'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "MemHandleType") {
        return ValidateAndSetProp(ePropVPP_MemHandleType, value);
    }
    else if (nextProp == "Width") {
        return ValidateAndSetProp(ePropVPP_Width, value);
    }
    else if (nextProp == "Height") {
        return ValidateAndSetProp(ePropVPP_Height, value);
    }
    else if (nextProp != "format") {
        return MFX_ERR_NOT_FOUND;
    }

    // parse 'format'
    nextProp = GetNextProp(&m_propParsedString);
    if (nextProp == "InFormat") {
        return ValidateAndSetProp(ePropVPP_InFormat, value);
    }
    else if (nextProp == "OutFormat" || nextProp == "OutFormats") {
        return ValidateAndSetProp(ePropVPP_OutFormat, value);
    }

    // end of mfxVPPDescription options
    return MFX_ERR_NOT_FOUND;
}

// return codes (from spec):
//   MFX_ERR_NOT_FOUND - name contains unknown parameter name
//   MFX_ERR_UNSUPPORTED - value data type != parameter with provided name
mfxStatus ConfigCtxVPL::SetFilterProperty(const mfxU8 *name, mfxVariant value) {
    if (!name)
        return MFX_ERR_NULL_PTR;

    m_propName = std::string((char *)name);

    m_propValue.Version.Version = MFX_VARIANT_VERSION;

    // initially set Type = unset (invalid)
    // if valid property string and value are passed in,
    //   this will be updated
    // otherwise loader will ignore this cfg during EnumImplementations
    m_propValue.Type     = MFX_VARIANT_TYPE_UNSET;
    m_propValue.Data.U32 = 0;

    // parse property string into individual properties,
    //   separated by '.'
    std::stringstream prop((char *)name);
    std::string s;
    while (getline(prop, s, '.')) {
        m_propParsedString.push_back(s);
    }

    // get first property descriptor
    std::string nextProp = GetNextProp(&m_propParsedString);

    // check for special-case properties, not part of mfxImplDescription
    if (nextProp == "mfxHandleType") {
        return ValidateAndSetProp(ePropSpecial_HandleType, value);
    }
    else if (nextProp == "mfxHDL") {
        return ValidateAndSetProp(ePropSpecial_Handle, value);
    }

    // to require that a specific function is implemented, use the property name
    //   "mfxImplementedFunctions.FunctionsName"
    if (nextProp == "mfxImplementedFunctions") {
        nextProp = GetNextProp(&m_propParsedString);
        if (nextProp == "FunctionsName") {
            return ValidateAndSetProp(ePropFunc_FunctionName, value);
        }
        return MFX_ERR_NOT_FOUND;
    }

    // standard properties must begin with "mfxImplDescription"
    if (nextProp != "mfxImplDescription") {
        return MFX_ERR_NOT_FOUND;
    }

    // get next property descriptor
    nextProp = GetNextProp(&m_propParsedString);

    // property is a top-level member of mfxImplDescription
    if (nextProp == "Impl") {
        return ValidateAndSetProp(ePropMain_Impl, value);
    }
    else if (nextProp == "AccelerationMode") {
        return ValidateAndSetProp(ePropMain_AccelerationMode, value);
    }
    else if (nextProp == "ApiVersion") {
        // ApiVersion may be passed as single U32 (Version) or two U16's (Major, Minor)
        nextProp = GetNextProp(&m_propParsedString);
        if (nextProp == "Version")
            return ValidateAndSetProp(ePropMain_ApiVersion, value);
        else if (nextProp == "Major")
            return ValidateAndSetProp(ePropMain_ApiVersion_Major, value);
        else if (nextProp == "Minor")
            return ValidateAndSetProp(ePropMain_ApiVersion_Minor, value);
        else
            return MFX_ERR_NOT_FOUND;
    }
    else if (nextProp == "VendorID") {
        return ValidateAndSetProp(ePropMain_VendorID, value);
    }
    else if (nextProp == "ImplName") {
        return ValidateAndSetProp(ePropMain_ImplName, value);
    }
    else if (nextProp == "License") {
        return ValidateAndSetProp(ePropMain_License, value);
    }
    else if (nextProp == "Keywords") {
        return ValidateAndSetProp(ePropMain_Keywords, value);
    }
    else if (nextProp == "VendorImplID") {
        return ValidateAndSetProp(ePropMain_VendorImplID, value);
    }

    // property is a member of mfxDeviceDescription
    // currently only settable parameter is DeviceID
    if (nextProp == "mfxDeviceDescription") {
        nextProp = GetNextProp(&m_propParsedString);
        if (nextProp == "device") {
            nextProp = GetNextProp(&m_propParsedString);
            if (nextProp == "DeviceID") {
                return ValidateAndSetProp(ePropDevice_DeviceID, value);
            }
        }
        return MFX_ERR_NOT_FOUND;
    }

    // property is a member of mfxDecoderDescription
    if (nextProp == "mfxDecoderDescription") {
        return SetFilterPropertyDec(value);
    }

    if (nextProp == "mfxEncoderDescription") {
        return SetFilterPropertyEnc(value);
    }

    if (nextProp == "mfxVPPDescription") {
        return SetFilterPropertyVPP(value);
    }

    return MFX_ERR_NOT_FOUND;
}

#define CHECK_IDX(idxA, idxB, numB) \
    if ((idxB) == (numB)) {         \
        (idxA)++;                   \
        (idxB) = 0;                 \
        continue;                   \
    }

mfxStatus ConfigCtxVPL::GetFlatDescriptionsDec(mfxImplDescription *libImplDesc,
                                               std::list<DecConfig> &decConfigList) {
    mfxU32 codecIdx   = 0;
    mfxU32 profileIdx = 0;
    mfxU32 memIdx     = 0;
    mfxU32 outFmtIdx  = 0;

    DecCodec *decCodec     = nullptr;
    DecProfile *decProfile = nullptr;
    DecMemDesc *decMemDesc = nullptr;

    while (codecIdx < libImplDesc->Dec.NumCodecs) {
        DecConfig dc = {};

        decCodec         = &(libImplDesc->Dec.Codecs[codecIdx]);
        dc.CodecID       = decCodec->CodecID;
        dc.MaxcodecLevel = decCodec->MaxcodecLevel;
        CHECK_IDX(codecIdx, profileIdx, decCodec->NumProfiles);

        decProfile = &(decCodec->Profiles[profileIdx]);
        dc.Profile = decProfile->Profile;
        CHECK_IDX(profileIdx, memIdx, decProfile->NumMemTypes);

        decMemDesc       = &(decProfile->MemDesc[memIdx]);
        dc.MemHandleType = decMemDesc->MemHandleType;
        dc.Width         = decMemDesc->Width;
        dc.Height        = decMemDesc->Height;
        CHECK_IDX(memIdx, outFmtIdx, decMemDesc->NumColorFormats);

        dc.ColorFormat = decMemDesc->ColorFormats[outFmtIdx];
        outFmtIdx++;

        // we have a valid, unique description - add to list
        decConfigList.push_back(dc);
    }

    if (decConfigList.empty())
        return MFX_ERR_INVALID_VIDEO_PARAM;

    return MFX_ERR_NONE;
}

mfxStatus ConfigCtxVPL::GetFlatDescriptionsEnc(mfxImplDescription *libImplDesc,
                                               std::list<EncConfig> &encConfigList) {
    mfxU32 codecIdx   = 0;
    mfxU32 profileIdx = 0;
    mfxU32 memIdx     = 0;
    mfxU32 inFmtIdx   = 0;

    EncCodec *encCodec     = nullptr;
    EncProfile *encProfile = nullptr;
    EncMemDesc *encMemDesc = nullptr;

    while (codecIdx < libImplDesc->Enc.NumCodecs) {
        EncConfig ec = {};

        encCodec                   = &(libImplDesc->Enc.Codecs[codecIdx]);
        ec.CodecID                 = encCodec->CodecID;
        ec.MaxcodecLevel           = encCodec->MaxcodecLevel;
        ec.BiDirectionalPrediction = encCodec->BiDirectionalPrediction;
        CHECK_IDX(codecIdx, profileIdx, encCodec->NumProfiles);

        encProfile = &(encCodec->Profiles[profileIdx]);
        ec.Profile = encProfile->Profile;
        CHECK_IDX(profileIdx, memIdx, encProfile->NumMemTypes);

        encMemDesc       = &(encProfile->MemDesc[memIdx]);
        ec.MemHandleType = encMemDesc->MemHandleType;
        ec.Width         = encMemDesc->Width;
        ec.Height        = encMemDesc->Height;
        CHECK_IDX(memIdx, inFmtIdx, encMemDesc->NumColorFormats);

        ec.ColorFormat = encMemDesc->ColorFormats[inFmtIdx];
        inFmtIdx++;

        // we have a valid, unique description - add to list
        encConfigList.push_back(ec);
    }

    if (encConfigList.empty())
        return MFX_ERR_INVALID_VIDEO_PARAM;

    return MFX_ERR_NONE;
}

mfxStatus ConfigCtxVPL::GetFlatDescriptionsVPP(mfxImplDescription *libImplDesc,
                                               std::list<VPPConfig> &vppConfigList) {
    mfxU32 filterIdx = 0;
    mfxU32 memIdx    = 0;
    mfxU32 inFmtIdx  = 0;
    mfxU32 outFmtIdx = 0;

    VPPFilter *vppFilter   = nullptr;
    VPPMemDesc *vppMemDesc = nullptr;
    VPPFormat *vppFormat   = nullptr;

    while (filterIdx < libImplDesc->VPP.NumFilters) {
        VPPConfig vc = {};

        vppFilter           = &(libImplDesc->VPP.Filters[filterIdx]);
        vc.FilterFourCC     = vppFilter->FilterFourCC;
        vc.MaxDelayInFrames = vppFilter->MaxDelayInFrames;
        CHECK_IDX(filterIdx, memIdx, vppFilter->NumMemTypes);

        vppMemDesc       = &(vppFilter->MemDesc[memIdx]);
        vc.MemHandleType = vppMemDesc->MemHandleType;
        vc.Width         = vppMemDesc->Width;
        vc.Height        = vppMemDesc->Height;
        CHECK_IDX(memIdx, inFmtIdx, vppMemDesc->NumInFormats);

        vppFormat   = &(vppMemDesc->Formats[inFmtIdx]);
        vc.InFormat = vppFormat->InFormat;
        CHECK_IDX(inFmtIdx, outFmtIdx, vppFormat->NumOutFormat);

        vc.OutFormat = vppFormat->OutFormats[outFmtIdx];
        outFmtIdx++;

        // we have a valid, unique description - add to list
        vppConfigList.push_back(vc);
    }

    if (vppConfigList.empty())
        return MFX_ERR_INVALID_VIDEO_PARAM;

    return MFX_ERR_NONE;
}

#define CHECK_PROP(idx, type, val)                             \
    if ((cfgPropsAll[(idx)].Type != MFX_VARIANT_TYPE_UNSET) && \
        (cfgPropsAll[(idx)].Data.type != val))                 \
        isCompatible = false;

mfxStatus ConfigCtxVPL::CheckPropsGeneral(mfxVariant cfgPropsAll[],
                                          mfxImplDescription *libImplDesc) {
    bool isCompatible = true;

    // check if this implementation includes
    //   all of the required top-level properties
    CHECK_PROP(ePropMain_Impl, U32, libImplDesc->Impl);
    CHECK_PROP(ePropMain_VendorID, U32, libImplDesc->VendorID);
    CHECK_PROP(ePropMain_VendorImplID, U32, libImplDesc->VendorImplID);

    // confirm that API version of this implementation is >= requested version
    mfxU32 versionRequested = 0;
    if (cfgPropsAll[ePropMain_ApiVersion].Type != MFX_VARIANT_TYPE_UNSET) {
        // version was passed as U32 = (Major | Minor)
        versionRequested = (mfxU32)(cfgPropsAll[ePropMain_ApiVersion].Data.U32);
    }
    else if (cfgPropsAll[ePropMain_ApiVersion_Major].Type != MFX_VARIANT_TYPE_UNSET &&
             cfgPropsAll[ePropMain_ApiVersion_Minor].Type != MFX_VARIANT_TYPE_UNSET) {
        // version was passed as 2x U16
        mfxVersion ver   = {};
        ver.Major        = (mfxU16)(cfgPropsAll[ePropMain_ApiVersion_Major].Data.U16);
        ver.Minor        = (mfxU16)(cfgPropsAll[ePropMain_ApiVersion_Minor].Data.U16);
        versionRequested = (mfxU32)ver.Version;
    }

    if (versionRequested) {
        if (libImplDesc->ApiVersion.Version < versionRequested)
            isCompatible = false;
    }

    if (libImplDesc->AccelerationModeDescription.NumAccelerationModes > 0) {
        if (cfgPropsAll[ePropMain_AccelerationMode].Type != MFX_VARIANT_TYPE_UNSET) {
            // check all supported modes if list is filled out
            mfxU16 numModes = libImplDesc->AccelerationModeDescription.NumAccelerationModes;
            mfxAccelerationMode modeRequested =
                (mfxAccelerationMode)(cfgPropsAll[ePropMain_AccelerationMode].Data.U32);
            auto *modeTab = libImplDesc->AccelerationModeDescription.Mode;

            auto *m = std::find(modeTab, modeTab + numModes, modeRequested);
            if (m == modeTab + numModes)
                isCompatible = false;
        }
    }
    else {
        // check default mode
        CHECK_PROP(ePropMain_AccelerationMode, U32, libImplDesc->AccelerationMode);
    }

    // check string: ImplName (string match)
    if (cfgPropsAll[ePropMain_ImplName].Type != MFX_VARIANT_TYPE_UNSET) {
        std::string filtName = *(std::string *)(cfgPropsAll[ePropMain_ImplName].Data.Ptr);
        std::string implName = libImplDesc->ImplName;
        if (filtName != implName)
            isCompatible = false;
    }

    // check string: License (tokenized)
    if (cfgPropsAll[ePropMain_License].Type != MFX_VARIANT_TYPE_UNSET) {
        std::string license = *(std::string *)(cfgPropsAll[ePropMain_License].Data.Ptr);
        if (CheckPropString(libImplDesc->License, license) != MFX_ERR_NONE)
            isCompatible = false;
    }

    // check string: Keywords (tokenized)
    if (cfgPropsAll[ePropMain_Keywords].Type != MFX_VARIANT_TYPE_UNSET) {
        std::string keywords = *(std::string *)(cfgPropsAll[ePropMain_Keywords].Data.Ptr);
        if (CheckPropString(libImplDesc->Keywords, keywords) != MFX_ERR_NONE)
            isCompatible = false;
    }

    // check DeviceID - stored as char*, but passed in for filtering as U16
    // convert both to unsigned ints and compare
    if (cfgPropsAll[ePropDevice_DeviceID].Type != MFX_VARIANT_TYPE_UNSET) {
        unsigned int implDeviceID = 0;
        try {
            implDeviceID = std::stoi(libImplDesc->Dev.DeviceID, 0, 16);
        }
        catch (...) {
            return MFX_ERR_UNSUPPORTED;
        }

        unsigned int filtDeviceID = (unsigned int)(cfgPropsAll[ePropDevice_DeviceID].Data.U16);
        if (implDeviceID != filtDeviceID)
            isCompatible = false;
    }

    if (isCompatible == true)
        return MFX_ERR_NONE;

    return MFX_ERR_UNSUPPORTED;
}

mfxStatus ConfigCtxVPL::CheckPropsDec(mfxVariant cfgPropsAll[],
                                      std::list<DecConfig> decConfigList) {
    auto it = decConfigList.begin();
    while (it != decConfigList.end()) {
        DecConfig dc      = (DecConfig)(*it);
        bool isCompatible = true;

        // check if this decode description includes
        //   all of the required decoder properties
        CHECK_PROP(ePropDec_CodecID, U32, dc.CodecID);
        CHECK_PROP(ePropDec_MaxcodecLevel, U16, dc.MaxcodecLevel);
        CHECK_PROP(ePropDec_Profile, U32, dc.Profile);
        CHECK_PROP(ePropDec_MemHandleType, U32, dc.MemHandleType);
        CHECK_PROP(ePropDec_ColorFormats, U32, dc.ColorFormat);

        // special handling for properties passed via pointer
        if (cfgPropsAll[ePropDec_Width].Type != MFX_VARIANT_TYPE_UNSET) {
            mfxRange32U width = {};
            if (cfgPropsAll[ePropDec_Width].Data.Ptr)
                width = *((mfxRange32U *)(cfgPropsAll[ePropDec_Width].Data.Ptr));

            if ((width.Max > dc.Width.Max) || (width.Min < dc.Width.Min) ||
                (width.Step < dc.Width.Step))
                isCompatible = false;
        }
        else if (cfgPropsAll[ePropDec_Height].Type != MFX_VARIANT_TYPE_UNSET) {
            mfxRange32U height = {};
            if (cfgPropsAll[ePropDec_Height].Data.Ptr)
                height = *((mfxRange32U *)(cfgPropsAll[ePropDec_Height].Data.Ptr));

            if ((height.Max > dc.Height.Max) || (height.Min < dc.Height.Min) ||
                (height.Step < dc.Height.Step))
                isCompatible = false;
        }

        if (isCompatible == true)
            return MFX_ERR_NONE;

        it++;
    }

    return MFX_ERR_UNSUPPORTED;
}

mfxStatus ConfigCtxVPL::CheckPropsEnc(mfxVariant cfgPropsAll[],
                                      std::list<EncConfig> encConfigList) {
    auto it = encConfigList.begin();
    while (it != encConfigList.end()) {
        EncConfig ec      = (EncConfig)(*it);
        bool isCompatible = true;

        // check if this encode description includes
        //   all of the required encoder properties
        CHECK_PROP(ePropEnc_CodecID, U32, ec.CodecID);
        CHECK_PROP(ePropEnc_MaxcodecLevel, U16, ec.MaxcodecLevel);
        CHECK_PROP(ePropEnc_BiDirectionalPrediction, U16, ec.BiDirectionalPrediction);
        CHECK_PROP(ePropEnc_Profile, U32, ec.Profile);
        CHECK_PROP(ePropEnc_MemHandleType, U32, ec.MemHandleType);
        CHECK_PROP(ePropEnc_ColorFormats, U32, ec.ColorFormat);

        // special handling for properties passed via pointer
        if (cfgPropsAll[ePropEnc_Width].Type != MFX_VARIANT_TYPE_UNSET) {
            mfxRange32U width = {};
            if (cfgPropsAll[ePropEnc_Width].Data.Ptr)
                width = *((mfxRange32U *)(cfgPropsAll[ePropEnc_Width].Data.Ptr));

            if ((width.Max > ec.Width.Max) || (width.Min < ec.Width.Min) ||
                (width.Step < ec.Width.Step))
                isCompatible = false;
        }
        else if (cfgPropsAll[ePropEnc_Height].Type != MFX_VARIANT_TYPE_UNSET) {
            mfxRange32U height = {};
            if (cfgPropsAll[ePropEnc_Height].Data.Ptr)
                height = *((mfxRange32U *)(cfgPropsAll[ePropEnc_Height].Data.Ptr));

            if ((height.Max > ec.Height.Max) || (height.Min < ec.Height.Min) ||
                (height.Step < ec.Height.Step))
                isCompatible = false;
        }

        if (isCompatible == true)
            return MFX_ERR_NONE;

        it++;
    }

    return MFX_ERR_UNSUPPORTED;
}

mfxStatus ConfigCtxVPL::CheckPropsVPP(mfxVariant cfgPropsAll[],
                                      std::list<VPPConfig> vppConfigList) {
    auto it = vppConfigList.begin();
    while (it != vppConfigList.end()) {
        VPPConfig vc      = (VPPConfig)(*it);
        bool isCompatible = true;

        // check if this filter description includes
        //   all of the required VPP properties
        CHECK_PROP(ePropVPP_FilterFourCC, U32, vc.FilterFourCC);
        CHECK_PROP(ePropVPP_MaxDelayInFrames, U32, vc.MaxDelayInFrames);
        CHECK_PROP(ePropVPP_MemHandleType, U32, vc.MemHandleType);
        CHECK_PROP(ePropVPP_InFormat, U32, vc.InFormat);
        CHECK_PROP(ePropVPP_OutFormat, U32, vc.OutFormat);

        // special handling for properties passed via pointer
        if (cfgPropsAll[ePropVPP_Width].Type != MFX_VARIANT_TYPE_UNSET) {
            mfxRange32U width = {};
            if (cfgPropsAll[ePropVPP_Width].Data.Ptr)
                width = *((mfxRange32U *)(cfgPropsAll[ePropVPP_Width].Data.Ptr));

            if ((width.Max > vc.Width.Max) || (width.Min < vc.Width.Min) ||
                (width.Step < vc.Width.Step))
                isCompatible = false;
        }
        else if (cfgPropsAll[ePropVPP_Height].Type != MFX_VARIANT_TYPE_UNSET) {
            mfxRange32U height = {};
            if (cfgPropsAll[ePropVPP_Height].Data.Ptr)
                height = *((mfxRange32U *)(cfgPropsAll[ePropVPP_Height].Data.Ptr));

            if ((height.Max > vc.Height.Max) || (height.Min < vc.Height.Min) ||
                (height.Step < vc.Height.Step))
                isCompatible = false;
        }

        if (isCompatible == true)
            return MFX_ERR_NONE;

        it++;
    }

    return MFX_ERR_UNSUPPORTED;
}

// implString = string from implDesc - one or more comma-separated tokens
// filtString = string user is looking for - one or more comma-separated tokens
// we parse filtString into tokens, then check if all of them are present in implString
mfxStatus ConfigCtxVPL::CheckPropString(mfxChar *implString, std::string filtString) {
    std::list<std::string> tokenString;
    std::string s;

    // parse implString string into tokens, separated by ','
    std::stringstream implSS((char *)implString);
    while (getline(implSS, s, ',')) {
        tokenString.push_back(s);
    }

    // parse filtString string into tokens, separated by ','
    // check that each token is present in implString, otherwise return error
    std::stringstream filtSS(filtString);
    while (getline(filtSS, s, ',')) {
        if (std::find(tokenString.begin(), tokenString.end(), s) == tokenString.end())
            return MFX_ERR_UNSUPPORTED;
    }

    return MFX_ERR_NONE;
}

mfxStatus ConfigCtxVPL::ValidateConfig(mfxImplDescription *libImplDesc,
                                       mfxImplementedFunctions *libImplFuncs,
                                       std::list<ConfigCtxVPL *> configCtxList,
                                       LibType libType,
                                       SpecialConfig *specialConfig) {
    mfxU32 idx;
    mfxStatus sts     = MFX_ERR_NONE;
    bool decRequested = false;
    bool encRequested = false;
    bool vppRequested = false;

    if (!libImplDesc)
        return MFX_ERR_NULL_PTR;

    std::list<DecConfig> decConfigList;
    std::list<EncConfig> encConfigList;
    std::list<VPPConfig> vppConfigList;

    // list of functions required to be implemented
    std::list<std::string> implFunctionList;
    implFunctionList.clear();

    // save list when multiple filters apply to the the same property
    //   (e.g two mfxConfig objects with different values for CodecID)
    std::list<ConfigCtxVPL *> configCtxListDups;

    // initially all properties are unset
    mfxVariant cfgPropsAll[eProp_TotalProps] = {};
    for (idx = 0; idx < eProp_TotalProps; idx++) {
        cfgPropsAll[idx].Type = MFX_VARIANT_TYPE_UNSET;
    }

    // iterate through all filters and populate cfgPropsAll
    std::list<ConfigCtxVPL *>::iterator it = configCtxList.begin();
    while (it != configCtxList.end()) {
        ConfigCtxVPL *config = (*it);

        idx = config->m_propIdx;
        it++;

        //if index is invalid continue to the next filter
        if (idx >= eProp_TotalProps)
            continue;

        // if property is required function, add to list which will be checked below
        if (idx == ePropFunc_FunctionName) {
            implFunctionList.push_back(config->m_implFunctionName);
            continue;
        }

        // ignore config objects that were never assigned a property
        if (config->m_propValue.Type == MFX_VARIANT_TYPE_UNSET)
            continue;

        // save duplicates for check in second pass (prop has already been set)
        if (cfgPropsAll[idx].Type != MFX_VARIANT_TYPE_UNSET) {
            configCtxListDups.push_back(config);
            continue;
        }

        cfgPropsAll[idx].Type = config->m_propValue.Type;
        cfgPropsAll[idx].Data = config->m_propValue.Data;

        if (idx >= ePropDec_CodecID && idx <= ePropDec_ColorFormats)
            decRequested = true;
        else if (idx >= ePropEnc_CodecID && idx <= ePropEnc_ColorFormats)
            encRequested = true;
        else if (idx >= ePropVPP_FilterFourCC && idx <= ePropVPP_OutFormat)
            vppRequested = true;
    }

    // check whether required functions are implemented
    if (!implFunctionList.empty()) {
        if (!libImplFuncs) {
            // library did not provide list of implemented functions
            return MFX_ERR_UNSUPPORTED;
        }

        auto fn = implFunctionList.begin();
        while (fn != implFunctionList.end()) {
            std::string fnName = (*fn++);
            mfxU32 fnIdx;

            // search for fnName in list of implemented functions
            for (fnIdx = 0; fnIdx < libImplFuncs->NumFunctions; fnIdx++) {
                if (fnName == libImplFuncs->FunctionsName[fnIdx])
                    break;
            }

            if (fnIdx == libImplFuncs->NumFunctions)
                return MFX_ERR_UNSUPPORTED;
        }
    }

    // update any special (including non-filtering) properties, for use by caller
    if (cfgPropsAll[ePropSpecial_HandleType].Type != MFX_VARIANT_TYPE_UNSET)
        specialConfig->deviceHandleType =
            (mfxHandleType)cfgPropsAll[ePropSpecial_HandleType].Data.U32;

    if (cfgPropsAll[ePropSpecial_Handle].Type != MFX_VARIANT_TYPE_UNSET)
        specialConfig->deviceHandle = (mfxHDL)cfgPropsAll[ePropSpecial_Handle].Data.Ptr;

    if (cfgPropsAll[ePropMain_ApiVersion].Type != MFX_VARIANT_TYPE_UNSET) {
        specialConfig->ApiVersion.Version = (mfxU32)cfgPropsAll[ePropMain_ApiVersion].Data.U32;
    }
    else if (cfgPropsAll[ePropMain_ApiVersion_Major].Type != MFX_VARIANT_TYPE_UNSET &&
             cfgPropsAll[ePropMain_ApiVersion_Minor].Type != MFX_VARIANT_TYPE_UNSET) {
        specialConfig->ApiVersion.Major = cfgPropsAll[ePropMain_ApiVersion_Major].Data.U16;
        specialConfig->ApiVersion.Minor = cfgPropsAll[ePropMain_ApiVersion_Minor].Data.U16;
    }

    if (cfgPropsAll[ePropMain_AccelerationMode].Type != MFX_VARIANT_TYPE_UNSET)
        specialConfig->accelerationMode =
            (mfxAccelerationMode)cfgPropsAll[ePropMain_AccelerationMode].Data.U32;

    // generate "flat" descriptions of each combination
    //   (e.g. multiple profiles from the same codec)
    GetFlatDescriptionsDec(libImplDesc, decConfigList);
    GetFlatDescriptionsEnc(libImplDesc, encConfigList);
    GetFlatDescriptionsVPP(libImplDesc, vppConfigList);

    sts = CheckPropsGeneral(cfgPropsAll, libImplDesc);
    if (sts)
        return sts;

    // early exit - MSDK RT compatibility mode (1.x) does not provide Dec/Enc/VPP caps
    // ignore these filters if set (do not use them to _exclude_ the library)
    if (libType == LibTypeMSDK)
        return MFX_ERR_NONE;

    if (decRequested) {
        sts = CheckPropsDec(cfgPropsAll, decConfigList);
        if (sts)
            return sts;
    }

    if (encRequested) {
        sts = CheckPropsEnc(cfgPropsAll, encConfigList);
        if (sts)
            return sts;
    }

    if (vppRequested) {
        sts = CheckPropsVPP(cfgPropsAll, vppConfigList);
        if (sts)
            return sts;
    }

    // second pass - if multiple filters were set with the same property,
    //   confirm that implementation supports all of them
    //
    // NOTE: the order of calling MFXSetConfigFilterProperty() can affect
    //   the logic here if user does something unusual such as setting
    //   two CodecId's AND two Profiles - how do we know which profile
    //   is associated with which codec? (it is just some integer)
    // Probably need to add more information to the spec to clarify
    //   allowable combinations of filter properties.
    std::list<ConfigCtxVPL *>::iterator it2 = configCtxListDups.begin();
    while (it2 != configCtxListDups.end()) {
        ConfigCtxVPL *config = (*it2);

        idx = config->m_propIdx;
        sts = MFX_ERR_NONE;
        it2++;

        //if index is invalid continue to the next filter
        if (idx >= eProp_TotalProps)
            continue;

        cfgPropsAll[idx].Type = config->m_propValue.Type;
        cfgPropsAll[idx].Data = config->m_propValue.Data;

        // re-test based on the type of parameter (e.g. a second decoder
        //   type just requires checking the decoder configurations)
        // no need to test general (top-level) properties, as they can
        //   only have a single value
        if (idx >= ePropDec_CodecID && idx <= ePropDec_ColorFormats)
            sts = CheckPropsDec(cfgPropsAll, decConfigList);
        else if (idx >= ePropEnc_CodecID && idx <= ePropEnc_ColorFormats)
            sts = CheckPropsEnc(cfgPropsAll, encConfigList);
        else if (idx >= ePropVPP_FilterFourCC && idx <= ePropVPP_OutFormat)
            sts = CheckPropsVPP(cfgPropsAll, vppConfigList);

        if (sts)
            return sts;
    }

    return MFX_ERR_NONE;
}
