/*############################################################################
  # Copyright (C) 2020 Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include "src/libmfxvplsw_caps.h"

template <typename T>
static T *AllocDescObject(mfxU32 NumObjects) {
    T *descObject;

    try {
        descObject = new T[NumObjects];
    }
    catch (...) {
        return nullptr;
    }

    memset(descObject, 0, sizeof(T) * NumObjects);

    return descObject;
}

template <typename T>
static void FreeDescObject(T *descObject) {
    if (descObject)
        delete[] descObject;
}

static const mfxStructVersion DefaultStructVersion = {
    DEF_STRUCT_VERSION_MINOR,
    DEF_STRUCT_VERSION_MAJOR
};

static const mfxRange32U DefaultRange = { DEF_RANGE_MIN,
                                          DEF_RANGE_MAX,
                                          DEF_RANGE_STEP };

// Start - Device Description

mfxStatus InitDeviceDescription(mfxDeviceDescription *Dev) {
    // initially empty
    memset(Dev, 0, sizeof(mfxDeviceDescription));

    Dev->Version.Major = DefaultStructVersion.Major;
    Dev->Version.Minor = DefaultStructVersion.Minor;

    strncpy_s((char *)Dev->DeviceID,
              sizeof(Dev->DeviceID),
              "CPU",
              sizeof(Dev->DeviceID) - 1);

    // CPU should report 0
    Dev->NumSubDevices = 0;

    return MFX_ERR_NONE;
}

void FreeDeviceDescription(mfxDeviceDescription *Dev) {
    // no dynamically allocated memory
    return;
}

// End - Device Description

// Start - Decoder Caps

static DecCodec *AddDecoderDescription(mfxDecoderDescription *Dec,
                                       mfxStructVersion Version,
                                       mfxU16 NumCodecs) {
    // must specify at least one codec
    if (NumCodecs == 0)
        return nullptr;

    Dec->Version.Version = MFX_DECODERDESCRIPTION_VERSION;
    Dec->NumCodecs       = NumCodecs;

    // allocate structures for supported codecs
    Dec->Codecs = AllocDescObject<DecCodec>(NumCodecs);

    return Dec->Codecs;
}

static DecProfile *AddDecoderCodec(DecCodec *Codecs,
                                   mfxU32 CodecIdx,
                                   mfxU32 CodecID,
                                   mfxU16 MaxcodecLevel,
                                   mfxU16 NumProfiles) {
    // must specify at least one profile
    if (NumProfiles == 0)
        return nullptr;

    // fill in info about this decoder
    Codecs[CodecIdx].CodecID       = CodecID;
    Codecs[CodecIdx].MaxcodecLevel = MaxcodecLevel;
    Codecs[CodecIdx].NumProfiles   = NumProfiles;

    // allocate structures for supported profiles
    Codecs[CodecIdx].Profiles = AllocDescObject<DecProfile>(NumProfiles);

    return Codecs[CodecIdx].Profiles;
}

static DecMemDesc *AddDecoderProfile(DecProfile *Profiles,
                                     mfxU32 ProfileIdx,
                                     mfxU32 Profile,
                                     mfxU16 NumMemTypes) {
    // must specify at least one memory type
    if (NumMemTypes == 0)
        return nullptr;

    // fill in info about this profile
    Profiles[ProfileIdx].Profile     = Profile;
    Profiles[ProfileIdx].NumMemTypes = NumMemTypes;

    // allocate structures for supported memory type
    Profiles[ProfileIdx].MemDesc = AllocDescObject<DecMemDesc>(NumMemTypes);

    return Profiles[ProfileIdx].MemDesc;
}

static mfxU32 *AddDecoderMemDesc(DecMemDesc *MemDesc,
                                 mfxU32 MemDescIdx,
                                 mfxResourceType MemHandleType,
                                 mfxRange32U Width,
                                 mfxRange32U Height,
                                 mfxU16 NumColorFormats) {
    // must specify at least one color format
    if (NumColorFormats == 0)
        return nullptr;

    // fill in info about this memory type
    MemDesc[MemDescIdx].MemHandleType   = MemHandleType;
    MemDesc[MemDescIdx].Width           = Width;
    MemDesc[MemDescIdx].Height          = Height;
    MemDesc[MemDescIdx].NumColorFormats = NumColorFormats;

    // allocate structures for supported color formats
    MemDesc[MemDescIdx].ColorFormats = AllocDescObject<mfxU32>(NumColorFormats);

    return MemDesc[MemDescIdx].ColorFormats;
}

mfxStatus InitDecoderCaps(mfxDecoderDescription *Dec) {
    DecCodec *Codecs;
    DecProfile *Profiles;
    DecMemDesc *MemDesc;
    mfxU32 *ColorFormats;

    // initially empty
    memset(Dec, 0, sizeof(mfxDecoderDescription));

    // fill decoder caps
    Codecs = AddDecoderDescription(Dec, DefaultStructVersion, 4);
    if (Codecs == nullptr)
        return MFX_ERR_MEMORY_ALLOC;

    // add codec type HEVC
    Profiles = AddDecoderCodec(Codecs, 0, MFX_CODEC_HEVC, MFX_LEVEL_HEVC_51, 2);

    // profile[0] - HEVC_MAIN
    MemDesc         = AddDecoderProfile(Profiles, 0, MFX_PROFILE_HEVC_MAIN, 1);
    ColorFormats    = AddDecoderMemDesc(MemDesc,
                                     0,
                                     MFX_RESOURCE_SYSTEM_SURFACE,
                                     DefaultRange,
                                     DefaultRange,
                                     1);
    ColorFormats[0] = MFX_FOURCC_I420;

    // profile[1] - HEVC_MAIN10
    MemDesc      = AddDecoderProfile(Profiles, 1, MFX_PROFILE_HEVC_MAIN10, 1);
    ColorFormats = AddDecoderMemDesc(MemDesc,
                                     0,
                                     MFX_RESOURCE_SYSTEM_SURFACE,
                                     DefaultRange,
                                     DefaultRange,
                                     1);
    ColorFormats[0] = MFX_FOURCC_I010;

    // add codec type AV1
    Profiles = AddDecoderCodec(Codecs, 1, MFX_CODEC_AV1, MFX_LEVEL_UNKNOWN, 1);

    // profile[0] - MFX_PROFILE_UNKNOWN
    MemDesc         = AddDecoderProfile(Profiles, 0, MFX_PROFILE_UNKNOWN, 1);
    ColorFormats    = AddDecoderMemDesc(MemDesc,
                                     0,
                                     MFX_RESOURCE_SYSTEM_SURFACE,
                                     DefaultRange,
                                     DefaultRange,
                                     2);
    ColorFormats[0] = MFX_FOURCC_I420;
    ColorFormats[1] = MFX_FOURCC_I010;

    // add codec type AVC
    Profiles = AddDecoderCodec(Codecs, 2, MFX_CODEC_AVC, MFX_LEVEL_UNKNOWN, 1);

    // profile[0] - MFX_PROFILE_AVC_HIGH
    MemDesc         = AddDecoderProfile(Profiles, 0, MFX_PROFILE_AVC_HIGH, 1);
    ColorFormats    = AddDecoderMemDesc(MemDesc,
                                     0,
                                     MFX_RESOURCE_SYSTEM_SURFACE,
                                     DefaultRange,
                                     DefaultRange,
                                     1);
    ColorFormats[0] = MFX_FOURCC_I420;

    // add codec type JPEG
    Profiles = AddDecoderCodec(Codecs, 3, MFX_CODEC_JPEG, MFX_LEVEL_UNKNOWN, 1);

    // profile[0] - MFX_PROFILE_AVC_HIGH
    MemDesc      = AddDecoderProfile(Profiles, 0, MFX_PROFILE_JPEG_BASELINE, 1);
    ColorFormats = AddDecoderMemDesc(MemDesc,
                                     0,
                                     MFX_RESOURCE_SYSTEM_SURFACE,
                                     DefaultRange,
                                     DefaultRange,
                                     1);
    ColorFormats[0] = MFX_FOURCC_I420;

    return MFX_ERR_NONE;
}

void FreeDecoderCaps(mfxDecoderDescription *decoder) {
    DecCodec *codec;
    DecProfile *profile;
    DecMemDesc *memdesc;
    mfxU32 c, p, m;

    for (c = 0; c < decoder->NumCodecs; c++) {
        codec = &(decoder->Codecs[c]);
        for (p = 0; p < codec->NumProfiles; p++) {
            profile = &(codec->Profiles[p]);
            for (m = 0; m < profile->NumMemTypes; m++) {
                memdesc = &(profile->MemDesc[m]);
                FreeDescObject<mfxU32>(memdesc->ColorFormats);
            }
            FreeDescObject<DecMemDesc>(profile->MemDesc);
        }
        FreeDescObject<DecProfile>(codec->Profiles);
    }
    FreeDescObject<DecCodec>(decoder->Codecs);

    memset(decoder, 0, sizeof(mfxImplDescription::Dec));
}

// End - Decoder Caps

// Start - Encoder Caps

static EncCodec *AddEncoderDescription(mfxEncoderDescription *Enc,
                                       mfxStructVersion Version,
                                       mfxU16 NumCodecs) {
    // must specify at least one codec
    if (NumCodecs == 0)
        return nullptr;

    Enc->Version.Version = MFX_ENCODERDESCRIPTION_VERSION;
    Enc->NumCodecs       = NumCodecs;

    // allocate structures for supported codecs
    Enc->Codecs = AllocDescObject<EncCodec>(NumCodecs);

    return Enc->Codecs;
}

static EncProfile *AddEncoderCodec(EncCodec *Codecs,
                                   mfxU32 CodecIdx,
                                   mfxU32 CodecID,
                                   mfxU16 MaxcodecLevel,
                                   mfxU16 BiDirectionalPrediction,
                                   mfxU16 NumProfiles) {
    // must specify at least one profile
    if (NumProfiles == 0)
        return nullptr;

    // fill in info about this encoder
    Codecs[CodecIdx].CodecID                 = CodecID;
    Codecs[CodecIdx].MaxcodecLevel           = MaxcodecLevel;
    Codecs[CodecIdx].BiDirectionalPrediction = BiDirectionalPrediction;
    Codecs[CodecIdx].NumProfiles             = NumProfiles;

    // allocate structures for supported profiles
    Codecs[CodecIdx].Profiles = AllocDescObject<EncProfile>(NumProfiles);

    return Codecs[CodecIdx].Profiles;
}

static EncMemDesc *AddEncoderProfile(EncProfile *Profiles,
                                     mfxU32 ProfileIdx,
                                     mfxU32 Profile,
                                     mfxU16 NumMemTypes) {
    // must specify at least one memory type
    if (NumMemTypes == 0)
        return nullptr;

    // fill in info about this profile
    Profiles[ProfileIdx].Profile     = Profile;
    Profiles[ProfileIdx].NumMemTypes = NumMemTypes;

    // allocate structures for supported memory type
    Profiles[ProfileIdx].MemDesc = AllocDescObject<EncMemDesc>(NumMemTypes);

    return Profiles[ProfileIdx].MemDesc;
}

static mfxU32 *AddEncoderMemDesc(EncMemDesc *MemDesc,
                                 mfxU32 MemDescIdx,
                                 mfxResourceType MemHandleType,
                                 mfxRange32U Width,
                                 mfxRange32U Height,
                                 mfxU16 NumColorFormats) {
    // must specify at least one color format
    if (NumColorFormats == 0)
        return nullptr;

    // fill in info about this memory type
    MemDesc[MemDescIdx].MemHandleType   = MemHandleType;
    MemDesc[MemDescIdx].Width           = Width;
    MemDesc[MemDescIdx].Height          = Height;
    MemDesc[MemDescIdx].NumColorFormats = NumColorFormats;

    // allocate structures for supported color formats
    MemDesc[MemDescIdx].ColorFormats = AllocDescObject<mfxU32>(NumColorFormats);

    return MemDesc[MemDescIdx].ColorFormats;
}

mfxStatus InitEncoderCaps(mfxEncoderDescription *Enc) {
    EncCodec *Codecs;
    EncProfile *Profiles;
    EncMemDesc *MemDesc;
    mfxU32 *ColorFormats;

    // initially empty
    memset(Enc, 0, sizeof(mfxEncoderDescription));

    // fill encoder caps
    Codecs = AddEncoderDescription(Enc, DefaultStructVersion, 3);
    if (Codecs == nullptr)
        return MFX_ERR_MEMORY_ALLOC;

    // add codec type HEVC
    Profiles =
        AddEncoderCodec(Codecs, 0, MFX_CODEC_HEVC, MFX_LEVEL_HEVC_51, 1, 2);

    // profile[0] - HEVC_MAIN
    MemDesc         = AddEncoderProfile(Profiles, 0, MFX_PROFILE_HEVC_MAIN, 1);
    ColorFormats    = AddEncoderMemDesc(MemDesc,
                                     0,
                                     MFX_RESOURCE_SYSTEM_SURFACE,
                                     DefaultRange,
                                     DefaultRange,
                                     1);
    ColorFormats[0] = MFX_FOURCC_I420;

    // profile[1] - HEVC_MAIN10
    MemDesc      = AddEncoderProfile(Profiles, 1, MFX_PROFILE_HEVC_MAIN10, 1);
    ColorFormats = AddEncoderMemDesc(MemDesc,
                                     0,
                                     MFX_RESOURCE_SYSTEM_SURFACE,
                                     DefaultRange,
                                     DefaultRange,
                                     1);
    ColorFormats[0] = MFX_FOURCC_I010;

    // add codec type AV1
    Profiles =
        AddEncoderCodec(Codecs, 1, MFX_CODEC_AV1, MFX_LEVEL_UNKNOWN, 1, 1);

    // profile[0] - MFX_PROFILE_UNKNOWN
    MemDesc         = AddEncoderProfile(Profiles, 0, MFX_PROFILE_UNKNOWN, 1);
    ColorFormats    = AddEncoderMemDesc(MemDesc,
                                     0,
                                     MFX_RESOURCE_SYSTEM_SURFACE,
                                     DefaultRange,
                                     DefaultRange,
                                     2);
    ColorFormats[0] = MFX_FOURCC_I420;
    ColorFormats[1] = MFX_FOURCC_I010;

    // add codec type JPEG
    Profiles =
        AddEncoderCodec(Codecs, 2, MFX_CODEC_JPEG, MFX_LEVEL_UNKNOWN, 0, 1);

    // profile[0] - MFX_PROFILE_UNKNOWN
    MemDesc      = AddEncoderProfile(Profiles, 0, MFX_PROFILE_JPEG_BASELINE, 1);
    ColorFormats = AddEncoderMemDesc(MemDesc,
                                     0,
                                     MFX_RESOURCE_SYSTEM_SURFACE,
                                     DefaultRange,
                                     DefaultRange,
                                     1);
    ColorFormats[0] = MFX_FOURCC_I420;

    return MFX_ERR_NONE;
}

void FreeEncoderCaps(mfxEncoderDescription *encoder) {
    EncCodec *codec;
    EncProfile *profile;
    EncMemDesc *memdesc;
    mfxU32 c, p, m;

    for (c = 0; c < encoder->NumCodecs; c++) {
        codec = &(encoder->Codecs[c]);
        for (p = 0; p < codec->NumProfiles; p++) {
            profile = &(codec->Profiles[p]);
            for (m = 0; m < profile->NumMemTypes; m++) {
                memdesc = &(profile->MemDesc[m]);
                FreeDescObject<mfxU32>(memdesc->ColorFormats);
            }
            FreeDescObject<EncMemDesc>(profile->MemDesc);
        }
        FreeDescObject<EncProfile>(codec->Profiles);
    }
    FreeDescObject<EncCodec>(encoder->Codecs);

    memset(encoder, 0, sizeof(mfxImplDescription::Enc));
}

// End - Encoder Caps

// Start - VPP caps

static VPPFilter *AddVPPDescription(mfxVPPDescription *VPP,
                                    mfxStructVersion Version,
                                    mfxU16 NumFilters) {
    // must specify at least one filter
    if (NumFilters == 0)
        return nullptr;

    VPP->Version.Version = MFX_VPPDESCRIPTION_VERSION;
    VPP->NumFilters      = NumFilters;

    // allocate structures for supported codecs
    VPP->Filters = AllocDescObject<VPPFilter>(NumFilters);

    return VPP->Filters;
}

static VPPMemDesc *AddVPPFilter(VPPFilter *Filters,
                                mfxU32 FilterIdx,
                                mfxU32 FilterFourCC,
                                mfxU16 MaxDelayInFrames,
                                mfxU16 NumMemTypes) {
    // must specify at least one memory type
    if (NumMemTypes == 0)
        return nullptr;

    // fill in info about this profile
    Filters[FilterIdx].FilterFourCC     = FilterFourCC;
    Filters[FilterIdx].MaxDelayInFrames = MaxDelayInFrames;
    Filters[FilterIdx].NumMemTypes      = NumMemTypes;

    // allocate structures for supported memory type
    Filters[FilterIdx].MemDesc = AllocDescObject<VPPMemDesc>(NumMemTypes);

    return Filters[FilterIdx].MemDesc;
}

static VPPFormat *AddVPPMemDesc(VPPMemDesc *MemDesc,
                                mfxU32 MemDescIdx,
                                mfxResourceType MemHandleType,
                                mfxRange32U Width,
                                mfxRange32U Height,
                                mfxU16 NumInFormats) {
    // must specify at least one input color format
    if (NumInFormats == 0)
        return nullptr;

    // fill in info about this memory type
    MemDesc[MemDescIdx].MemHandleType = MemHandleType;
    MemDesc[MemDescIdx].Width         = Width;
    MemDesc[MemDescIdx].Height        = Height;
    MemDesc[MemDescIdx].NumInFormats  = NumInFormats;

    // allocate structures for supported color formats
    MemDesc[MemDescIdx].Formats = AllocDescObject<VPPFormat>(NumInFormats);

    return MemDesc[MemDescIdx].Formats;
}

static mfxU32 *AddVPPInFormats(VPPFormat *Formats,
                               mfxU32 FormatIdx,
                               mfxU32 InFormat,
                               mfxU16 NumOutFormat) {
    // must specify at least one output color format
    if (NumOutFormat == 0)
        return nullptr;

    // fill in info about this memory type
    Formats[FormatIdx].InFormat     = InFormat;
    Formats[FormatIdx].NumOutFormat = NumOutFormat;

    // allocate structures for supported color formats
    Formats[FormatIdx].OutFormats = AllocDescObject<mfxU32>(NumOutFormat);

    return Formats[FormatIdx].OutFormats;
}

mfxStatus InitVPPCaps(mfxVPPDescription *VPP) {
    VPPFilter *Filters;
    VPPMemDesc *MemDesc;
    VPPFormat *InFormats;
    mfxU32 *OutFormats;

    // initially empty
    memset(VPP, 0, sizeof(mfxVPPDescription));

    // fill VPP caps
    Filters = AddVPPDescription(VPP, DefaultStructVersion, 2);
    if (Filters == nullptr)
        return MFX_ERR_MEMORY_ALLOC;

    // add VPP filter CSC
    MemDesc = AddVPPFilter(Filters, 0, MFX_EXTBUFF_VPP_COLOR_CONVERSION, 1, 1);

    InFormats = AddVPPMemDesc(MemDesc,
                              0,
                              MFX_RESOURCE_SYSTEM_SURFACE,
                              DefaultRange,
                              DefaultRange,
                              3);

    // add input format MFX_FOURCC_I420
    OutFormats    = AddVPPInFormats(InFormats, 0, MFX_FOURCC_I420, 3);
    OutFormats[0] = MFX_FOURCC_I420;
    OutFormats[1] = MFX_FOURCC_I010;
    OutFormats[2] = MFX_FOURCC_RGB4;

    // add input format MFX_FOURCC_I010
    OutFormats    = AddVPPInFormats(InFormats, 1, MFX_FOURCC_I010, 3);
    OutFormats[0] = MFX_FOURCC_I420;
    OutFormats[1] = MFX_FOURCC_I010;
    OutFormats[2] = MFX_FOURCC_RGB4;

    // add input format MFX_FOURCC_RGB4
    OutFormats    = AddVPPInFormats(InFormats, 2, MFX_FOURCC_RGB4, 3);
    OutFormats[0] = MFX_FOURCC_I420;
    OutFormats[1] = MFX_FOURCC_I010;
    OutFormats[2] = MFX_FOURCC_RGB4;

    // add VPP filter resize
    MemDesc = AddVPPFilter(Filters, 1, MFX_EXTBUFF_VPP_SCALING, 1, 1);

    InFormats = AddVPPMemDesc(MemDesc,
                              0,
                              MFX_RESOURCE_SYSTEM_SURFACE,
                              DefaultRange,
                              DefaultRange,
                              3);

    // add input format MFX_FOURCC_I420
    OutFormats    = AddVPPInFormats(InFormats, 0, MFX_FOURCC_I420, 3);
    OutFormats[0] = MFX_FOURCC_I420;
    OutFormats[1] = MFX_FOURCC_I010;
    OutFormats[2] = MFX_FOURCC_RGB4;

    // add input format MFX_FOURCC_I010
    OutFormats    = AddVPPInFormats(InFormats, 1, MFX_FOURCC_I010, 3);
    OutFormats[0] = MFX_FOURCC_I420;
    OutFormats[1] = MFX_FOURCC_I010;
    OutFormats[2] = MFX_FOURCC_RGB4;

    // add input format MFX_FOURCC_RGB4
    OutFormats    = AddVPPInFormats(InFormats, 2, MFX_FOURCC_RGB4, 3);
    OutFormats[0] = MFX_FOURCC_I420;
    OutFormats[1] = MFX_FOURCC_I010;
    OutFormats[2] = MFX_FOURCC_RGB4;

    return MFX_ERR_NONE;
}

void FreeVPPCaps(mfxVPPDescription *vpp) {
    VPPFilter *filter;
    VPPMemDesc *memdesc;
    VPPFormat *format;
    mfxU32 f, m, t;

    for (f = 0; f < vpp->NumFilters; f++) {
        filter = &(vpp->Filters[f]);
        for (m = 0; m < filter->NumMemTypes; m++) {
            memdesc = &(filter->MemDesc[m]);
            for (t = 0; t < memdesc->NumInFormats; t++) {
                format = &(memdesc->Formats[t]);
                FreeDescObject<mfxU32>(format->OutFormats);
            }
            FreeDescObject<VPPFormat>(memdesc->Formats);
        }
        FreeDescObject<VPPMemDesc>(filter->MemDesc);
    }
    FreeDescObject<VPPFilter>(vpp->Filters);

    memset(vpp, 0, sizeof(mfxImplDescription::VPP));
}

// End - VPP Caps