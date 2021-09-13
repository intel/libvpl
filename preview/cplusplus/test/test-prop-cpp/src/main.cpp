//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

#include <fstream>
#include <iostream>
#include <sstream>

#include "vpl/preview/vpl.hpp"

namespace vpl = oneapi::vpl;

const char *PropStrings[] = {
    "mfxImplDescription.Impl",
    "mfxImplDescription.AccelerationMode",
    "mfxImplDescription.ApiVersion.Version",
    "mfxImplDescription.ApiVersion.Major",
    "mfxImplDescription.ApiVersion.Minor",
    "mfxImplDescription.ImplName",
    "mfxImplDescription.License",
    "mfxImplDescription.Keywords",
    "mfxImplDescription.VendorID",
    "mfxImplDescription.VendorImplID",
    "mfxImplDescription.mfxDeviceDescription.device.DeviceID",
    "mfxImplDescription.mfxDecoderDescription.decoder.CodecID",
    "mfxImplDescription.mfxDecoderDescription.decoder.MaxcodecLevel",
    "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.Profile",
    "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.MemHandleType",
    "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.Width",
    "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.Height",
    "mfxImplDescription.mfxDecoderDescription.decoder.decprofile.decmemdesc.ColorFormats",
    "mfxImplDescription.mfxEncoderDescription.encoder.CodecID",
    "mfxImplDescription.mfxEncoderDescription.encoder.MaxcodecLevel",
    "mfxImplDescription.mfxEncoderDescription.encoder.BiDirectionalPrediction",
    "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.Profile",
    "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.MemHandleType",
    "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.Width",
    "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.Height",
    "mfxImplDescription.mfxEncoderDescription.encoder.encprofile.encmemdesc.ColorFormats",
    "mfxImplDescription.mfxVPPDescription.filter.FilterFourCC",
    "mfxImplDescription.mfxVPPDescription.filter.MaxDelayInFrames",
    "mfxImplDescription.mfxVPPDescription.filter.memdesc.MemHandleType",
    "mfxImplDescription.mfxVPPDescription.filter.memdesc.Width",
    "mfxImplDescription.mfxVPPDescription.filter.memdesc.Height",
    "mfxImplDescription.mfxVPPDescription.filter.memdesc.format.InFormat",
    "mfxImplDescription.mfxVPPDescription.filter.memdesc.format.OutFormats",

    "mfxImplementedFunctions.FunctionsName",

    // non-filtering properties (parameters passing only)
    "DXGIAdapterIndex",
    "mfxHandleType",
    "mfxHDL",
};

#define NUM_PROP_STRINGS (sizeof(PropStrings) / sizeof(char *))

static int TestProperty(const char *name) {
    std::stringstream prop(name);
    std::string s;
    vpl::property_name p;

    printf("Checking prop string: %s", name);

    try {
        while (getline(prop, s, '.')) {
            p = p / s.c_str();
        }
        vpl::property p2(p, (uint32_t)0);
    }
    catch (...) {
        printf("\n   Error!\n");
        return -1;
    }

    printf(" ... OK\n");

    return 0;
}

int main(int argc, char *argv[]) {
    int res;

    res = 0;
    for (unsigned int i = 0; i < NUM_PROP_STRINGS; i++) {
        res |= TestProperty(PropStrings[i]);
    }

    if (res)
        printf("\nErrors in property string parsing\n");
    else
        printf("\nSuccess!\n");

    return res;
}