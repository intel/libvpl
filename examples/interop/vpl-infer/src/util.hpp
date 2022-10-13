//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================

///
/// Utility library header file for sample code
///
/// @file

#ifndef EXAMPLES_UTIL_H_
#define EXAMPLES_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ostream>
#include <string>
#include <vector>

#ifdef USE_MEDIASDK1
    #include "mfxvideo.h"
enum {
    MFX_FOURCC_I420 = MFX_FOURCC_IYUV /*!< Alias for the IYUV color format. */
};
#else
    #include "vpl/mfxjpeg.h"
    #include "vpl/mfxvideo.h"
#endif

#if (MFX_VERSION >= 2000)
    #include "vpl/mfxdispatcher.h"
#endif

#ifdef __linux__
    #include <fcntl.h>
    #include <unistd.h>
#endif

#ifdef LIBVA_SUPPORT
    #include "va/va.h"
    #include "va/va_drm.h"
#endif

#define MAX_PATH        260
#define IS_ARG_EQ(a, b) (!strcmp((a), (b)))

#define ALIGN16(value)           (((value + 15) >> 4) << 4)
#define ALIGN32(X)               (((mfxU32)((X) + 31)) & (~(mfxU32)31))
#define VPLVERSION(major, minor) (major << 16 | minor)

enum ExampleParams { PARAM_IMPL = 0, PARAM_INFILE, PARAM_INRES, PARAM_COUNT };
enum ParamGroup {
    PARAMS_CREATESESSION = 0,
    PARAMS_DECODE,
    PARAMS_ENCODE,
    PARAMS_VPP,
    PARAMS_TRANSCODE,
    PARAMS_DECVPP
};

typedef struct _Params {
    mfxIMPL impl;
    mfxVariant implValue;

    char *infileName;
    char *inmodelName;
} Params;

char *ValidateFileName(char *in) {
    if (in) {
        if (strnlen(in, MAX_PATH) > MAX_PATH)
            return NULL;
    }

    return in;
}

bool ParseArgsAndValidate(int argc, char *argv[], Params *params, ParamGroup group) {
    int idx;
    char *s;

    // init all params to 0
    *params = {};

    params->impl               = MFX_IMPL_HARDWARE;
    params->implValue.Type     = MFX_VARIANT_TYPE_U32;
    params->implValue.Data.U32 = MFX_IMPL_TYPE_HARDWARE;

    for (idx = 1; idx < argc;) {
        // all switches must start with '-'
        if (argv[idx][0] != '-') {
            printf("ERROR: invalid argument: %s\n", argv[idx]);
            return false;
        }

        // switch string, starting after the '-'
        s = &argv[idx][1];
        idx++;

        // search for match
        if (IS_ARG_EQ(s, "i")) {
            params->infileName = ValidateFileName(argv[idx++]);
            if (!params->infileName) {
                return false;
            }
        }
        else if (IS_ARG_EQ(s, "m")) {
            params->inmodelName = ValidateFileName(argv[idx++]);
            if (!params->inmodelName) {
                return false;
            }
        }
        else if (IS_ARG_EQ(s, "hw")) {
            params->impl               = MFX_IMPL_HARDWARE;
            params->implValue.Data.U32 = MFX_IMPL_TYPE_HARDWARE;
        }
    }

    if (!params->inmodelName) {
        printf("ERROR: network model name (-m) is required\n");
        return false;
    }
    // input file required by all except createsession
    if ((group != PARAMS_CREATESESSION) && (!params->infileName)) {
        printf("ERROR: input file name (-i) is required\n");
        return false;
    }

    return true;
}

// Shows implementation info with oneVPL
void ShowImplementationInfo(mfxLoader loader, mfxU32 implnum) {
    mfxImplDescription *idesc = nullptr;
    mfxStatus sts;
    //Loads info about implementation at specified list location
    sts = MFXEnumImplementations(loader, implnum, MFX_IMPLCAPS_IMPLDESCSTRUCTURE, (mfxHDL *)&idesc);
    if (!idesc || (sts != MFX_ERR_NONE))
        return;

    printf("\nOneVPL Implementation details:\n");
    printf("    ApiVersion:           %hu.%hu  \n",
           idesc->ApiVersion.Major,
           idesc->ApiVersion.Minor);
    printf("    Implementation type:  %s\n", (idesc->Impl == MFX_IMPL_TYPE_SOFTWARE) ? "SW" : "HW");
    printf("    AccelerationMode via: ");
    switch (idesc->AccelerationMode) {
        case MFX_ACCEL_MODE_NA:
            printf("NA \n");
            break;
        case MFX_ACCEL_MODE_VIA_D3D9:
            printf("D3D9\n");
            break;
        case MFX_ACCEL_MODE_VIA_D3D11:
            printf("D3D11\n");
            break;
        case MFX_ACCEL_MODE_VIA_VAAPI:
            printf("VAAPI\n");
            break;
        case MFX_ACCEL_MODE_VIA_VAAPI_DRM_MODESET:
            printf("VAAPI_DRM_MODESET\n");
            break;
        case MFX_ACCEL_MODE_VIA_VAAPI_GLX:
            printf("VAAPI_GLX\n");
            break;
        case MFX_ACCEL_MODE_VIA_VAAPI_X11:
            printf("VAAPI_X11\n");
            break;
        case MFX_ACCEL_MODE_VIA_VAAPI_WAYLAND:
            printf("VAAPI_WAYLAND\n");
            break;
        case MFX_ACCEL_MODE_VIA_HDDLUNITE:
            printf("HDDLUNITE\n");
            break;
        default:
            printf("unknown\n");
            break;
    }
    MFXDispReleaseImplDescription(loader, idesc);

#if (MFX_VERSION >= 2004)
    //Show implementation path, added in 2.4 API
    mfxHDL implPath = nullptr;
    sts             = MFXEnumImplementations(loader, implnum, MFX_IMPLCAPS_IMPLPATH, &implPath);
    if (!implPath || (sts != MFX_ERR_NONE))
        return;

    printf("    Path: %s\n\n", reinterpret_cast<mfxChar *>(implPath));
    MFXDispReleaseImplDescription(loader, implPath);
#endif
}

// Read encoded stream from file
mfxStatus ReadEncodedStream(mfxBitstream &bs, FILE *f) {
    mfxU8 *p0 = bs.Data;
    mfxU8 *p1 = bs.Data + bs.DataOffset;
    if (bs.DataOffset > bs.MaxLength - 1) {
        return MFX_ERR_NOT_ENOUGH_BUFFER;
    }
    if (bs.DataLength + bs.DataOffset > bs.MaxLength) {
        return MFX_ERR_NOT_ENOUGH_BUFFER;
    }
    for (mfxU32 i = 0; i < bs.DataLength; i++) {
        *(p0++) = *(p1++);
    }
    bs.DataOffset = 0;
    bs.DataLength += (mfxU32)fread(bs.Data + bs.DataLength, 1, bs.MaxLength - bs.DataLength, f);
    if (bs.DataLength == 0)
        return MFX_ERR_MORE_DATA;

    return MFX_ERR_NONE;
}

#define TSTRING2STRING(tstr) tstr

void PrintInputAndOutputsInfo(const ov::Model &network) {
    std::cout << "    Model name: " << network.get_friendly_name() << std::endl;

    const std::vector<ov::Output<const ov::Node>> inputs = network.inputs();
    for (const ov::Output<const ov::Node> input : inputs) {
        std::cout << "    Inputs" << std::endl;

        const std::string name = input.get_names().empty() ? "NONE" : input.get_any_name();
        std::cout << "        Input name: " << name << std::endl;

        const ov::element::Type type = input.get_element_type();
        std::cout << "        Input type: " << type << std::endl;

        const ov::Shape shape = input.get_shape();
        std::cout << "        Input shape: " << shape << std::endl;
    }

    const std::vector<ov::Output<const ov::Node>> outputs = network.outputs();
    for (const ov::Output<const ov::Node> output : outputs) {
        std::cout << "    Outputs" << std::endl;

        const std::string name = output.get_names().empty() ? "NONE" : output.get_any_name();
        std::cout << "        Output name: " << name << std::endl;

        const ov::element::Type type = output.get_element_type();
        std::cout << "        Output type: " << type << std::endl;

        const ov::Shape shape = output.get_shape();
        std::cout << "        Output shape: " << shape << std::endl;
    }

    std::cout << std::endl;
}

#endif //EXAMPLES_UTIL_H_
