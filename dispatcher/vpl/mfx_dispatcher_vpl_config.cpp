/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "vpl/mfx_dispatcher_vpl.h"

// implementation of config context (mfxConfig)
// each loader instance can have one or more configs
//   associated with it - used for filtering implementations
//   based on what they support (codec types, etc.)
ConfigCtxVPL::ConfigCtxVPL()
        : m_propName(),
          m_propValue(),
          m_propIdx(),
          m_propParsedString(),
          m_propRange32U() {
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
    ePropMain_VendorID,
    ePropMain_VendorImplID,

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

    // number of entries (always last)
    eProp_TotalProps
};

// order must align exactly with PropIdx list
// to avoid mismatches, this should be automated (e.g. pre-processor step)
static const PropVariant PropIdxTab[] = {
    { "ePropMain_Impl", MFX_VARIANT_TYPE_U32 },
    { "ePropMain_AccelerationMode", MFX_VARIANT_TYPE_U16 },
    { "ePropMain_VendorID", MFX_VARIANT_TYPE_U32 },
    { "ePropMain_VendorImplID", MFX_VARIANT_TYPE_U32 },

    { "ePropDec_CodecID", MFX_VARIANT_TYPE_U32 },
    { "ePropDec_MaxcodecLevel", MFX_VARIANT_TYPE_U16 },
    { "ePropDec_Profile", MFX_VARIANT_TYPE_U32 },
    { "ePropDec_MemHandleType", MFX_VARIANT_TYPE_U32 },
    { "ePropDec_Width", MFX_VARIANT_TYPE_PTR },
    { "ePropDec_Height", MFX_VARIANT_TYPE_PTR },
    { "ePropDec_ColorFormats", MFX_VARIANT_TYPE_U32 },

    { "ePropEnc_CodecID", MFX_VARIANT_TYPE_U32 },
    { "ePropEnc_MaxcodecLevel", MFX_VARIANT_TYPE_U16 },
    { "ePropEnc_BiDirectionalPrediction", MFX_VARIANT_TYPE_U16 },
    { "ePropEnc_Profile", MFX_VARIANT_TYPE_U32 },
    { "ePropEnc_MemHandleType", MFX_VARIANT_TYPE_U32 },
    { "ePropEnc_Width", MFX_VARIANT_TYPE_PTR },
    { "ePropEnc_Height", MFX_VARIANT_TYPE_PTR },
    { "ePropEnc_ColorFormats", MFX_VARIANT_TYPE_U32 },

    { "ePropVPP_FilterFourCC", MFX_VARIANT_TYPE_U32 },
    { "ePropVPP_MaxDelayInFrames", MFX_VARIANT_TYPE_U16 },
    { "ePropVPP_MemHandleType", MFX_VARIANT_TYPE_U32 },
    { "ePropVPP_Width", MFX_VARIANT_TYPE_PTR },
    { "ePropVPP_Height", MFX_VARIANT_TYPE_PTR },
    { "ePropVPP_InFormat", MFX_VARIANT_TYPE_U32 },
    { "ePropVPP_OutFormat", MFX_VARIANT_TYPE_U32 },
};

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

    // get first property descriptor - must be "mfxImplDescription"
    std::string nextProp = GetNextProp(&m_propParsedString);
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
    else if (nextProp == "VendorID") {
        return ValidateAndSetProp(ePropMain_VendorID, value);
    }
    else if (nextProp == "VendorImplID") {
        return ValidateAndSetProp(ePropMain_VendorImplID, value);
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
    CHECK_PROP(ePropMain_AccelerationMode, U16, libImplDesc->AccelerationMode);
    CHECK_PROP(ePropMain_VendorID, U32, libImplDesc->VendorID);
    CHECK_PROP(ePropMain_VendorImplID, U32, libImplDesc->VendorImplID);

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

mfxStatus ConfigCtxVPL::ValidateConfig(mfxImplDescription *libImplDesc,
                                       std::list<ConfigCtxVPL *> configCtxList) {
    mfxU32 idx;
    mfxStatus sts     = MFX_ERR_NONE;
    bool decRequested = false;
    bool encRequested = false;
    bool vppRequested = false;

    std::list<DecConfig> decConfigList;
    std::list<EncConfig> encConfigList;
    std::list<VPPConfig> vppConfigList;

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

    // generate "flat" descriptions of each combination
    //   (e.g. multiple profiles from the same codec)
    GetFlatDescriptionsDec(libImplDesc, decConfigList);
    GetFlatDescriptionsEnc(libImplDesc, encConfigList);
    GetFlatDescriptionsVPP(libImplDesc, vppConfigList);

    sts = CheckPropsGeneral(cfgPropsAll, libImplDesc);
    if (sts)
        return sts;

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
