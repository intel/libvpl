# **Intel® oneVPL Multi-Adapter Guide**

- [Overview](#overview)
- [Developer guide](#developer-guide)
  * [Feature API](#feature-api)

- [Release Notes](#release-notes)
  * [Supported configurations](#supported-configurations)
  * [Known limitations](#known-limitations)

# Overview

Intel(R) oneVPL can access multiple adapters in Linux and Windows with features added in API 2.6.

# Developer guide

## Feature API

For multi-adapter scenarios, the implementation corresponding to the target device is chosen 
as part of the implementation selection process during session initialization.

For Windows, DXGIAdapterIndex allows implementations to be filtered by adapter number.
The code below shows how to filter in Windows for the implementation using adapter index 0:

```c++
  mfxVariant cfgVal;
  mfxConfig cfg = MFXCreateConfig(loader);
  cfgVal.Type = MFX_VARIANT_TYPE_U32;
  cfgVal.Data.U32 = 0;
  MFXSetConfigFilterProperty(
      cfg, (mfxU8 *)"DXGIAdapterIndex", cfgVal);
```

For more information on DXGIAdapterIndex, see https://spec.oneapi.io/onevpl/latest/programming_guide/VPL_prg_session.html.

Starting with API 2.6, Linux implementation selection is supported with the addition of mfxExtendedDeviceID.
The code below shows how to filter in Linux for the implementation using /dev/dri/renderD128:
 
```c++
  mfxVariant cfgVal;
  mfxConfig cfg = MFXCreateConfig(loader);
  cfgVal.Type = MFX_VARIANT_TYPE_U32;
  cfgVal.Data.U32 = 128;
  MFXSetConfigFilterProperty(
      cfg, (mfxU8 *)"mfxExtendedDeviceId.DRMRenderNodeNum", cfgVal);
```
For more information on MFXExtendedDeviceId, see
https://spec.oneapi.io/onevpl/latest/API_ref/VPL_disp_api_struct.html?highlight=mfxextendeddeviceid.


## Running sample_* tools with oneVPL runtime


## How to prepare the system

Multi-adapter features can be used with any implementation.  However, they are most relevant for GPUs.
Media SDK and oneVPL GPU hardware can be targeted.  


# Release Notes

## Known limitations

Media SDK only reports a subset of the info available from the VPL implementation, so it may not be possible to use all of the 
parameters available in mfxExtendedDeviceId for devices supported by Media SDK.


# Example code.

The code below is a quick illustration of how an implementation corresponding to a render node in Linux can be chosen by oneVPL multi-adapter features.

```c++

#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include "vpl/mfx.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
#ifdef _WIN32
        printf("usage: hello-multiadapter AdapterNum\n");
#else
        printf("usage: hello-multiadapter DRMRenderNodeNum\n");
#endif
        return -1;
    }
    mfxSession session = NULL;
    mfxConfig cfg[2];
    mfxVariant cfgVal[2];

    mfxLoader loader   = MFXLoad();
    cfg[0]             = MFXCreateConfig(loader);
    cfgVal[0].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[0].Data.U32 = MFX_IMPL_TYPE_HARDWARE;
    mfxStatus sts =
        MFXSetConfigFilterProperty(cfg[0], (mfxU8 *)"mfxImplDescription.Impl", cfgVal[0]);
    if (MFX_ERR_NONE != sts)
        printf("MFXSetConfigFilterProperty 0 error=%d\n", sts);

#ifdef _WIN32
    cfg[1]             = MFXCreateConfig(loader);
    cfgVal[1].Type     = MFX_VARIANT_TYPE_U32;
    cfgVal[1].Data.U32 = atoi(argv[1]);
    sts                = MFXSetConfigFilterProperty(cfg[1], (mfxU8 *)"DXGIAdapterIndex", cfgVal[1]);
    if (MFX_ERR_NONE != sts)
        printf("MFXSetConfigFilterProperty 1 error=%d\n", sts);
#else
    cfg[1] = MFXCreateConfig(loader);
    cfgVal[1].Type = MFX_VARIANT_TYPE_U32;
    cfgVal[1].Data.U32 = atoi(argv[1]);
    sts = MFXSetConfigFilterProperty(cfg[1],
                                     (mfxU8 *)"mfxExtendedDeviceId.DRMRenderNodeNum",
                                     cfgVal[1]);
    if (MFX_ERR_NONE != sts)
        printf("MFXSetConfigFilterProperty 1 error=%d\n", sts);
#endif

    sts = MFXCreateSession(loader, 0, &session);
    if (MFX_ERR_NONE != sts) {
        printf("MFXCreateSession error=%d\n", sts);
        if (MFX_ERR_NOT_FOUND == sts) {
            printf("No implementations could be found meeting criteria specified\n");
        }
    }
    else {
        mfxImplDescription *iDesc = nullptr;

        // Loads info about implementation at specified list location
        sts = MFXEnumImplementations(loader, 0, MFX_IMPLCAPS_IMPLDESCSTRUCTURE, (mfxHDL *)&iDesc);
        if (MFX_ERR_NONE != sts)
            printf("MFXEnumImplementations error=%d\n", sts);

        printf("Implementation used to create session:\n");
        printf("  ApiVersion:           %hu.%hu  \n",
               iDesc->ApiVersion.Major,
               iDesc->ApiVersion.Minor);
        printf("  Implementation type:  %s\n",
               (iDesc->Impl == MFX_IMPL_TYPE_SOFTWARE) ? "SW" : "HW");

        MFXDispReleaseImplDescription(loader, iDesc);

        mfxHDL implPath = nullptr;
        sts             = MFXEnumImplementations(loader, 0, MFX_IMPLCAPS_IMPLPATH, &implPath);
        if (MFX_ERR_NONE != sts)
            printf("MFXEnumImplementations error=%d\n", sts);

        printf("  Path: %s\n\n", reinterpret_cast<mfxChar *>(implPath));
        MFXDispReleaseImplDescription(loader, implPath);

        mfxExtendedDeviceId *idescDevice;

        sts = MFXEnumImplementations(loader,
                                     0,
                                     MFX_IMPLCAPS_DEVICE_ID_EXTENDED,
                                     reinterpret_cast<mfxHDL *>(&idescDevice));
        if (MFX_ERR_NONE != sts) {
            printf("MFXEnumImplementations MFX_IMPLCAPS_DEVICE_ID_EXTENDED error=%d\n", sts);
        }
        else {
            printf("  DeviceID: 0x%04X\n", idescDevice->DeviceID);
#ifndef _WIN32
            printf("  DRMRenderNodeNum: %d\n", idescDevice->DRMRenderNodeNum);
#endif
        }

        MFXDispReleaseImplDescription(loader, idescDevice);

        MFXClose(session);
    }

    if (loader)
        MFXUnload(loader);

    return 0;
}

```

To compile (Linux):

```
g++ -o hello-multiadapter hello-multiadapter.cpp -lvpl -DONEVPL_EXPERIMENTAL
```

To run (Linux):

hello-multiadapter accepts a render node number as an argument in Linux.  For example, to access the device at /dev/dri/renderD128:

```
./hello-multiadapter 128
```


Expected output (Linux, VPL implementation):

```
$ ./hello-multiadapter 128

Implementation used to create session:
  ApiVersion:           2.6
  Implementation type:  HW
  Path: /usr/lib/x86_64-linux-gnu/libmfx-gen.so.1.2.6

  DeviceID: 0x9A49
  DRMRenderNodeNum: 128

```

To run (Windows):

hello-multiadapter accepts an adapter number in Windows.  For example, to access the device corresponding to adapter 0:

```
./hello-multiadapter 0
```

Expected output (Windows, Media SDK implementation not upgraded to API 1.35):

```
./hello-multiadapter.exe 0
Implementation used to create session:
  ApiVersion:           1.34
  Implementation type:  HW
  Path: C:\WINDOWS\System32\DriverStore\FileRepository\iigd_dch.inf_amd64_90af7db2c816ac7b\libmfxhw64.dll

  DeviceID: 0x3EA0
```