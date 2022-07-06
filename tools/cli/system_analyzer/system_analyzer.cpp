/*############################################################################
  # Copyright (C) Intel Corporation
  #
  # SPDX-License-Identifier: MIT
  ############################################################################*/

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>
#include <array>
#include <fstream>
#include <string>
#include <vector>
#include "vpl/mfx.h"

#ifdef __linux__
    #include <fcntl.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include "dlfcn.h"
    #include "va/va.h"
    #include "va/va_drm.h"
#endif

typedef struct gpuinfo {
    unsigned int deviceid;
    std::array<char, 128> name;
    std::array<char, 8> arch;
    std::array<char, 32> codename;
} gpuinfo;

gpuinfo get_gpuinfo(unsigned int deviceid) {
    gpuinfo result;
    gpuinfo gpunames[] = { { 0x5694, "Intel® Arc(TM) A370M Graphics", "Xe HPG", "DG2" },
                           { 0x5693, "Intel® Arc(TM) A350M Graphics", "Xe HPG", "DG2" },
                           { 0x4905, "Intel® Iris® Xe MAX Graphics", "Xe MAX", "DG1" },
                           { 0x9A60, "Intel® UHD Graphics GT1", "Xe", "Tiger Lake" },
                           { 0x9A68, "Intel® UHD Graphics GT1", "Xe", "Tiger Lake" },
                           { 0x9A70, "Intel® UHD Graphics GT1", "Xe", "Tiger Lake" },
                           { 0x9A40, "Intel® Iris® Xe Graphics GT2", "Xe", "Tiger Lake" },
                           { 0x9A49, "Intel® Iris® Xe Graphics GT2", "Xe", "Tiger Lake" },
                           { 0x9A78, "Intel® UHD Graphics GT2", "Xe", "Tiger Lake" },
                           { 0x9AC0, "Intel® UHD Graphics GT2", "Xe", "Tiger Lake" },
                           { 0x9AC9, "Intel® UHD Graphics GT2", "Xe", "Tiger Lake" },
                           { 0x9AD9, "Intel® UHD Graphics GT2", "Xe", "Tiger Lake" },
                           { 0x9AF8, "Intel® UHD Graphics GT2", "Xe", "Tiger Lake" } };

    gpuinfo unknown = { 0x0000, "unknown", "na", "na" };

    uint32_t n    = (sizeof(gpunames[0]) > 0) ? (sizeof(gpunames) / sizeof(gpunames[0])) : 1;
    int resultidx = -1;

    for (uint32_t i = 0; i < n; i++) {
        if (gpunames[i].deviceid == deviceid) {
            resultidx = i;
            break;
        }
    }

    if (resultidx >= 0) {
        result = gpunames[resultidx];
    }
    else {
        result = unknown;
    }

    return result;
}

#define STRING_OPTION(x) \
    case x:              \
        return #x

const char *_print_Impl(mfxIMPL impl) {
    switch (impl) {
        STRING_OPTION(MFX_IMPL_TYPE_SOFTWARE);
        STRING_OPTION(MFX_IMPL_TYPE_HARDWARE);
    }

    return "<unknown implementation>";
}

const char *_print_AccelMode(mfxAccelerationMode mode) {
    switch (mode) {
        STRING_OPTION(MFX_ACCEL_MODE_NA);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_D3D9);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_D3D11);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_DRM_MODESET);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_GLX);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_X11);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_VAAPI_WAYLAND);
        STRING_OPTION(MFX_ACCEL_MODE_VIA_HDDLUNITE);
    }

    return "<unknown acceleration mode>";
}

const char *_print_MediaAdapterType(mfxMediaAdapterType type) {
    switch (type) {
        STRING_OPTION(MFX_MEDIA_UNKNOWN);
        STRING_OPTION(MFX_MEDIA_INTEGRATED);
        STRING_OPTION(MFX_MEDIA_DISCRETE);
    }

    return "<unknown media adapter type>";
}

bool show_MFXLoad_info() {
    mfxLoader loader = MFXLoad();
    if (loader == NULL) {
        return false;
    }

    //Loop over implementations found by MFXLoad
    int i = 0;
    mfxImplDescription *idesc;
    while (MFX_ERR_NONE == MFXEnumImplementations(loader,
                                                  i,
                                                  MFX_IMPLCAPS_IMPLDESCSTRUCTURE,
                                                  reinterpret_cast<mfxHDL *>(&idesc))) {
        printf("\nImplementation #%d: %s\n", i, idesc->ImplName);

        // get path if supported (available starting with API 2.4)
        mfxHDL hImplPath = nullptr;
        if (MFX_ERR_NONE == MFXEnumImplementations(loader, i, MFX_IMPLCAPS_IMPLPATH, &hImplPath)) {
            if (hImplPath) {
                printf("%2sLibrary path: %s\n", "", reinterpret_cast<mfxChar *>(hImplPath));
                MFXDispReleaseImplDescription(loader, hImplPath);
            }
        }

        printf("  AccelerationMode: %s\n", _print_AccelMode(idesc->AccelerationMode));
        printf("  ApiVersion: %hu.%hu\n", idesc->ApiVersion.Major, idesc->ApiVersion.Minor);
        printf("  Impl: %s\n", _print_Impl(idesc->Impl));
        printf("  ImplName: %s\n", idesc->ImplName);

        /* mfxDeviceDescription */
        mfxDeviceDescription *dev = &idesc->Dev;

        printf("  MediaAdapterType: %s\n",
               _print_MediaAdapterType((mfxMediaAdapterType)dev->MediaAdapterType));

        for (int subdevice = 0; subdevice < dev->NumSubDevices; subdevice++) {
            printf("    Index: %u\n", dev->SubDevices[subdevice].Index);
            printf("    SubDeviceID: %s\n", dev->SubDevices[subdevice].SubDeviceID);
        }

        mfxExtendedDeviceId *idescDevice;

        mfxStatus sts = MFXEnumImplementations(loader,
                                               i,
                                               MFX_IMPLCAPS_DEVICE_ID_EXTENDED,
                                               reinterpret_cast<mfxHDL *>(&idescDevice));
        if (sts == MFX_ERR_NONE) {
            printf("  VendorID: 0x%04X\n", idescDevice->VendorID);
            printf("  DeviceID: 0x%04X \n", idescDevice->DeviceID);

            gpuinfo g = get_gpuinfo(idescDevice->DeviceID);
            std::string gpuname(std::begin(g.name), std::end(g.name));
            std::string gpuarch(std::begin(g.arch), std::end(g.arch));
            std::string gpucodename(std::begin(g.codename), std::end(g.codename));
            printf("  GPU name: %s (arch=%s codename=%s)\n",
                   gpuname.c_str(),
                   gpuarch.c_str(),
                   gpucodename.c_str());

            printf("  PCI BDF: %04X:%02X:%02X.%02X\n",
                   idescDevice->PCIDomain,
                   idescDevice->PCIBus,
                   idescDevice->PCIDevice,
                   idescDevice->PCIFunction);

            if (idescDevice->LUIDValid) {
                printf("  DeviceLUID: ");
                for (mfxU32 idx = 0; idx < 8; idx++) {
                    printf("%02x", idescDevice->DeviceLUID[7 - idx]);
                }
                printf("\n");
                printf("  LUIDDeviceNodeMask: 0x%04X\n", idescDevice->LUIDDeviceNodeMask);
            }

            printf("  DRMRenderNodeNum: %d\n", idescDevice->DRMRenderNodeNum);

            printf("DeviceName: %s\n", idescDevice->DeviceName);
            MFXDispReleaseImplDescription(loader, idescDevice);
        }

        i++;
    }

    MFXUnload(loader);

    return (i > 0) ? true : false;
}

int main() {
    bool check_gpu_caps = true;
    void *libva_handle  = dlopen("libva.so", RTLD_NOW | RTLD_GLOBAL);
    if (!libva_handle) {
        printf("could not open libva.so\n");
        check_gpu_caps = false;
    }
    void *libvadrm_handle = dlopen("libva-drm.so", RTLD_NOW | RTLD_GLOBAL);
    if (!libvadrm_handle) {
        printf("could not open libva-drm.so\n");
        check_gpu_caps = false;
    }

    if (check_gpu_caps) {
        printf("------------------------------------\n");
        printf("Looking for GPU interfaces available to OS...\n");
        int nGPUadapters             = 0;
        std::vector<std::string> rns = { "/dev/dri/renderD128", "/dev/dri/renderD129",
                                         "/dev/dri/renderD130", "/dev/dri/renderD131",
                                         "/dev/dri/renderD132", "/dev/dri/renderD133",
                                         "/dev/dri/renderD134", "/dev/dri/renderD135" };

        for (size_t i = 0; i < rns.size(); i++) {
            std::ifstream rn(rns[i].c_str());
            if (rn.good()) {
                printf("FOUND: %s\n", rns[i].c_str());
                nGPUadapters++;
            }
        }
        printf("GPU interfaces found: %d\n", nGPUadapters);
        if (!nGPUadapters) {
            printf("No GPU adapters found.  Possible reasons:\n");
            printf(" * No GPU HW available\n");
            printf(" * GPU not enabled in BIOS\n");
            printf(" * Unsupported kernel version.  Check 'uname -r'\n");
            printf(" * i915 module not started.  Check 'lsmod'\n");
            printf(" * problem initializing i915.  Check 'dmesg'\n");
        }
        printf("------------------------------------\n\n\n");
    }
    else {
        printf("------------------------------------\n");
        printf("Could not initialize libva. Is libva installed?\n");
        printf("GPU media capabilities cannot work without libva\n");
        printf("------------------------------------\n\n\n");
    }

    printf("------------------------------------\n");
    printf("Available implementation details:\n");
    if (!show_MFXLoad_info()) {
        printf("No oneVPL implementations found.  Is environment configured?\n");
    }
    printf("------------------------------------\n");

    if (libva_handle)
        dlclose(libva_handle);
    if (libvadrm_handle)
        dlclose(libvadrm_handle);

    return 0;
}
