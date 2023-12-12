# ** Intel® Video Processing Library (Intel® VPL) Multi-Adapter Guide**

- [Overview](#overview)
- [Developer guide](#developer-guide)
  * [Feature API](#feature-api)

- [Release Notes](#release-notes)
  * [Supported configurations](#supported-configurations)
  * [Known limitations](#known-limitations)

# Overview

Intel® VPL can access multiple adapters in Linux and Windows with features added in API 2.6.

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
- For more information on DXGIAdapterIndex, see https://intel.github.io/libvpl/programming_guide/VPL_prg_session.html.

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
https://intel.github.io/libvpl/API_ref/VPL_disp_api_struct.html?highlight=mfxextendeddeviceid.


## Running sample_* tools with Intel® VPL runtime


## How to prepare the system

Multi-adapter features can be used with any implementation.  However, they are most relevant for GPUs.
Media SDK and Intel® VPL GPU hardware can be targeted.  


# Release Notes

## Known limitations

Media SDK only reports a subset of the info available from the Intel® VPL implementation, so it may not be possible to use all of the 
parameters available in mfxExtendedDeviceId for devices supported by Media SDK.


# Example code.

The code below is a quick illustration of how an implementation corresponding to a render node in Linux can be chosen by Intel® VPL multi-adapter features. Most of dependent functions are in  libvpl/src/mfx_dispatcher_vpl.cpp

```c++

#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include "vpl/mfx.h"
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
#ifdef _WIN32
        printf("usage: hello-multiadapter AdapterNum\n");
#else
        printf("usage: hello-multiadapter DRMRenderNodeNum\n");
#endif
        return -1;
    }

#ifdef ONEVPL_EXPERIMENTAL
	printf("Build ONEVPL_EXPERIMENTAL APIs.\n");
#endif

    mfxSession session = NULL;
    mfxConfig cfg[2];
    mfxVariant cfgVal[2];
    mfxLoader loader = MFXLoad();
	
    cfg[0] = MFXCreateConfig(loader);
    cfgVal[0].Type = MFX_VARIANT_TYPE_U32;
    cfgVal[0].Data.U32 = MFX_IMPL_TYPE_HARDWARE;
    mfxStatus sts =
        MFXSetConfigFilterProperty(cfg[0], (const mfxU8*)"mfxImplDescription.Impl", cfgVal[0]);
    if (MFX_ERR_NONE != sts)
        printf("MFXSetConfigFilterProperty 0 error=%d\n", sts);
	
    mfxU8* filter;
#ifdef _WIN32
    filter = (mfxU8*)"DXGIAdapterIndex";
	//or
	//filter = (mfxU8*)"mfxImplDescription.VendorImplID";
	printf("Filter hardware query by: %s\n", filter);
#else
    filter = (mfxU8*)"mfxImplDescription.DRMRenderNodeNum";
#endif
    cfg[1] = MFXCreateConfig(loader);
    cfgVal[1].Type = MFX_VARIANT_TYPE_U32;
    cfgVal[1].Data.U32 = atoi(argv[1]);
    sts = MFXSetConfigFilterProperty(cfg[1], filter, cfgVal[1]);
    if (MFX_ERR_NONE != sts)
        printf("MFXSetConfigFilterProperty 1 error=%d\n", sts);

    sts = MFXCreateSession(loader, 0, &session);
    if (MFX_ERR_NONE != sts) {
        printf("MFXCreateSession error=%d\n", sts);
        if (MFX_ERR_NOT_FOUND == sts) {
            printf("No implementations could be found meeting criteria specified\n");
        }
    }
    else {
        mfxImplDescription* iDesc = nullptr;

        // Loads info about implementation at specified list location
        sts = MFXEnumImplementations(loader, 0, MFX_IMPLCAPS_IMPLDESCSTRUCTURE, (mfxHDL*)&iDesc);
        if (MFX_ERR_NONE != sts)
            printf("MFXEnumImplementations error=%d\n", sts);

        printf("Implementation used to create session:\n");
        printf("  ApiVersion:           %hu.%hu  \n",
            iDesc->ApiVersion.Major,
            iDesc->ApiVersion.Minor);
        printf("  Implementation type:  %s\n",
            (iDesc->Impl == MFX_IMPL_TYPE_SOFTWARE) ? "SW" : "HW");
        printf("%2sApiVersion.Major: 0x%04X\n", "", iDesc->ApiVersion.Major);
		printf("%2sApiVersion.Minor: 0x%04X\n", "", iDesc->ApiVersion.Minor);
		printf("%2sImplementation Name: %s\n", "", iDesc->ImplName);
		printf("%2sLicense: %s\n", "", iDesc->License);
		printf("%2sKeywords: %s\n", "", iDesc->Keywords);
		printf("%2sVendorID: 0x%04X\n", "", iDesc->VendorID);
		printf("%2sVendorImplID: 0x%04X\n", "", iDesc->VendorImplID);
		
        MFXDispReleaseImplDescription(loader, iDesc);

        mfxHDL implPath = nullptr;
        sts = MFXEnumImplementations(loader, 0, MFX_IMPLCAPS_IMPLPATH, &implPath);
        if (MFX_ERR_NONE != sts)
            printf("MFXEnumImplementations error=%d\n", sts);

        printf("  Path: %s\n\n", reinterpret_cast<mfxChar*>(implPath));
        MFXDispReleaseImplDescription(loader, implPath);

#ifdef ONEVPL_EXPERIMENTAL
		printf("Enable mfxExtendedDeviceId with ONEVPL_EXPERIMENTAL APIs.\n");
        mfxExtendedDeviceId* idescDevice;

        sts = MFXEnumImplementations(loader,
            0,
            MFX_IMPLCAPS_DEVICE_ID_EXTENDED,
            reinterpret_cast<mfxHDL*>(&idescDevice));
        if (MFX_ERR_NONE != sts) {
            printf("MFXEnumImplementations MFX_IMPLCAPS_DEVICE_ID_EXTENDED error=%d\n", sts);
        }
        else {
            printf("%6sDeviceName: %s\n", "", idescDevice->DeviceName);
            printf("%6sExtended DeviceID's:\n", "");
            printf("%6sVendorID: 0x%04X\n", "", idescDevice->VendorID);
		    printf("%6sDeviceID: 0x%04X\n", "", idescDevice->DeviceID);
            printf("%6sPCIDomain: 0x%08X\n", "", idescDevice->PCIDomain);
            printf("%6sPCIBus: 0x%08X\n", "", idescDevice->PCIBus);
            printf("%6sPCIdevice: 0x%08X\n", "", idescDevice->PCIDevice);
            printf("%6sPCIFunction: 0x%08X\n", "", idescDevice->PCIFunction);
            printf("%6sDRMRenderNodeNum: %d\n", "", idescDevice->DRMRenderNodeNum);
            printf("%6sDRMPrimaryNodeNum: 0x%04X\n", "", idescDevice->DRMPrimaryNodeNum);
            printf("%6sLUIDValid: 0x%04X\n", "", idescDevice->LUIDValid);

			if (idescDevice->LUIDValid) {
                printf("%6sDeviceLUID: ", "");
                for (mfxU32 idx = 0; idx < 8; idx++) {
                    printf("%02x", idescDevice->DeviceLUID[7 - idx]);
                }
                printf("\n");
                printf("%6sLUIDDeviceNodeMask: 0x%04X\n", "", idescDevice->LUIDDeviceNodeMask);
            }
			
#ifndef _WIN32
            printf("  DRMRenderNodeNum: %d\n", idescDevice->DRMRenderNodeNum);
#endif
        }

        MFXDispReleaseImplDescription(loader, idescDevice);
#endif

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


Expected output (Linux, Intel® VPL implementation):

```
$ ./hello-multiadapter 128

Implementation used to create session:
  ApiVersion:           2.6
  Implementation type:  HW
  Path: /usr/lib/x86_64-linux-gnu/libmfx-gen.so.1.2.6

  DeviceID: 0x9A49
  DRMRenderNodeNum: 128

```
To Build in (Windows):

Create a hello-multiadapter directory, a src subdirectory, place the hello-multiadapter.cpp in the src, and the CMakeLists.txt (see below) in hello-multiadapter directory.
```
md build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
cd Release
```
To run (Windows):

hello-multiadapter accepts an adapter number in Windows.  For example, to access the device corresponding to adapter 0:

```
hello-multiadapter 0
```

Expected output (Windows, Media SDK implementation not upgraded to API 1.35):

```
Build ONEVPL_EXPERIMENTAL APIs.
Filter hardware query by: DXGIAdapterIndex
Implementation used to create session:
  ApiVersion:           2.6
  Implementation type:  HW
  ApiVersion.Major: 0x0002
  ApiVersion.Minor: 0x0006
  Implementation Name: mfx-gen
  License:
  Keywords:
  VendorID: 0x8086
  VendorImplID: 0x0000
  Path: C:\windows\System32\DriverStore\FileRepository\iigd_dch.inf_amd64_393549dac595e659\libmfx64-gen.dll

Enable mfxExtendedDeviceId with ONEVPL_EXPERIMENTAL APIs.
      DeviceName: mfx-gen
      Extended DeviceID's:
      VendorID: 0x8086
      DeviceID: 0x9A49
      PCIDomain: 0x00000000
      PCIBus: 0x00000000
      PCIdevice: 0x00000002
      PCIFunction: 0x00000000
      DRMRenderNodeNum: 0
      DRMPrimaryNodeNum: 0x7FFFFFFF
      LUIDValid: 0x0001
      DeviceLUID: 000000000002e94c
      LUIDDeviceNodeMask: 0x0001
```

The CMakeLists.txt:
```
# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)
project(hello-multiadapter)

# Default install places 64 bit runtimes in the environment, so we want to do a
# 64 bit build by default.
if(WIN32)
  if(NOT DEFINED CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM
        x64
        CACHE STRING "")
    message(STATUS "Generator Platform set to ${CMAKE_GENERATOR_PLATFORM}")
  endif()
endif()

set(TARGET hello-multiadapter)
set(SOURCES src/hello-multiadapter.cpp)
# set(CONTENTPATH ${CMAKE_CURRENT_SOURCE_DIR}/../../content)
# set(RUNARGS -sw -i ${CONTENTPATH}/cars_320x240.h265)

# Set default build type to RelWithDebInfo if not specified
if(NOT CMAKE_BUILD_TYPE)
  message(
    STATUS "Default CMAKE_BUILD_TYPE not set using Release with Debug Info")
  set(CMAKE_BUILD_TYPE
      "RelWithDebInfo"
      CACHE
        STRING
        "Choose build type from: None Debug Release RelWithDebInfo MinSizeRel"
        FORCE)
endif()

add_compile_definitions(ONEVPL_EXPERIMENTAL)

add_executable(${TARGET} ${SOURCES})

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  if(NOT DEFINED ENV{VSCMD_VER})
    set(CMAKE_MSVCIDE_RUN_PATH $ENV{PATH})
  endif()
endif()

find_package(VPL REQUIRED)
target_link_libraries(${TARGET} VPL::dispatcher)

if(UNIX)
  set(LIBVA_SUPPORT
      ON
      CACHE BOOL "Enable hardware support.")
  if(LIBVA_SUPPORT)
    find_package(PkgConfig REQUIRED)
    # note: pkg-config version for libva is *API* version
    pkg_check_modules(PKG_LIBVA libva>=1.2 libva-drm>=1.2)
    if(PKG_LIBVA_FOUND)
      target_compile_definitions(${TARGET} PUBLIC -DLIBVA_SUPPORT)
      set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
      set(THREADS_PREFER_PTHREAD_FLAG TRUE)
      find_package(Threads REQUIRED)
      target_link_libraries(${TARGET} ${PKG_LIBVA_LIBRARIES}
                            ${PKG_THREAD_LIBRARIES})
      target_include_directories(${TARGET} PUBLIC ${PKG_LIBVA_INCLUDE_DIRS})
    else()
      message(
        SEND_ERROR
          "libva not found: set LIBVA_SUPPORT=OFF to build ${TARGET} without libva support"
      )
    endif()
  else()
    message(STATUS "Building ${TARGET} without hardware support")
  endif()
endif()

```
