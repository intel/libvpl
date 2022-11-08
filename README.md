# ![oneAPI](assets/oneapi-logo.png "oneAPI") Video Processing Library

The oneAPI Video Processing Library (oneVPL) is a programming interface for video decoding, encoding, 
and processing to build portable media pipelines on CPUs, GPUs, and other accelerators.

It provides device discovery
and selection in media centric and video analytics workloads and API primitives for zero-copy buffer sharing. oneVPL is backwards
and cross-architecture compatible to ensure optimal execution on current and next generation hardware without source code changes.

See the [oneVPL Specification](https://spec.oneapi.io/versions/latest/elements/oneVPL/source/index.html) for additional information. This is part of the [oneAPI specification](https://www.oneapi.io/spec/).

This repository contains the following components of oneVPL:

- Copies of the oneVPL Specification API header files. The version of the oneVPL API is listed in the
[mfxdefs.h](./api/vpl/mfxdefs.h) file.
- oneVPL Dispatcher
- Examples demonstrating API usage
- oneVPL command line tools

To use oneVPL for video processing you need to install at least one implementation. Here is a list of current implementations.

- [oneVPL-cpu](https://github.com/oneapi-src/oneVPL-cpu) for use on CPU
- [oneVPL-intel-gpu](https://github.com/oneapi-src/oneVPL-intel-gpu) for use on Intel Xe graphics and newer
- [Media SDK](https://github.com/Intel-Media-SDK/MediaSDK) for use on legacy Intel graphics

## oneVPL Architecture
```mermaid
graph TD;
    VPL[oneVPL Dispatcher]-->oneVPL-cpu;
    VPL[oneVPL Dispatcher]-->oneVPL-intel-gpu;
    VPL[oneVPL Dispatcher]-->MediaSDK;
    VPL[oneVPL Dispatcher]-->Future1;
    VPL[oneVPL Dispatcher]-->Future2;
```

As shown in this diagram, the oneVPL Dispatcher dispatches the application to use either the VPL CPU runtime, VPL GPU runtime, or the Media SDK GPU Runtime. We may support more implementations in the future.

## oneVPL Dispatcher behavior when targeting Intel GPUs
Runtime loaded by oneVPL Dispatcher and their Microsoft* DirectX* support:


| GPU                                        | Media SDK        | oneVPL           | Microsoft* DirectX* Support |
|--------------------------------------------|------------------|------------------|-----------------------------|
| Earlier platforms, back to BDW (Broadwell) |:heavy_check_mark:|                  | DX9/DX11                    |
| ICL (Ice Lake)                             |:heavy_check_mark:|                  | DX9/DX11                    |
| JSL (Jasper Lake)                          |:heavy_check_mark:|                  | DX9/DX11                    |
| EHL (Elkhart Lake)                         |:heavy_check_mark:|                  | DX9/DX11                    |
| SG1                                        |:heavy_check_mark:|                  | DX9/DX11                    |
| TGL (Tiger Lake)                           |:heavy_check_mark:|:heavy_check_mark:| DX9/DX11*                   |
| DG1 (Iris® Xe MAX)                         |:heavy_check_mark:|:heavy_check_mark:| DX11*                       |
| RKL (Rocket Lake)                          |                  |:heavy_check_mark:| DX11                        |
| ADL-S (Alder Lake S)                       |                  |:heavy_check_mark:| DX11                        |
| ADL-P (Alder Lake P)                       |                  |:heavy_check_mark:| DX11                        |
| Future platforms...                        |                  |:heavy_check_mark:| DX11                        |

For TGL and DG1, if both oneVPL and Intel(R) Media SDK runtime are installed then the oneVPL Dispatcher will prefer oneVPL runtime unless the application requests D3D9 by setting the oneVPL Dispatcher filter property "mfxImplDescription.AccelerationMode" to MFX_ACCEL_MODE_VIA_D3D9.

## Installation

You can install oneVPL:

- from [oneVPL home page](https://software.intel.com/content/www/us/en/develop/tools/oneapi/components/onevpl.html) as a part of Intel&reg; oneAPI Base Toolkit.
- from source code.  See [Installation from Sources](INSTALL.md) for details.

For more details on installation options and procedures, see the [Intel® oneAPI Video Processing Library Installation Guide](https://software.intel.com/content/www/us/en/develop/articles/onevpl-installation-guide.html).

## Developer Usage

### Configure the Environment

If you did not install to standard system locations, you need to set up the
environment, so tools like CMake and pkg-config can find the library and
headers.

For Linux:
```
source <vpl-install-location>/etc/vpl/vars.sh
```

For Windows:
```
<vpl-install-location>\etc\vpl\vars.bat
```

### Link to oneVPL with CMake

Add the following code to your CMakeLists, assuming TARGET is defined as the
component that wants to use oneVPL:

```
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
 set(CMAKE_LIBRARY_ARCHITECTURE x86)
endif()
find_package(VPL REQUIRED)
target_link_libraries(${TARGET} VPL::dispatcher)
```


### Link to oneVPL from Bash with pkg-config

The following command line illustrates how to link a simple program to oneVPL
using pkg-config.

```
gcc program.cpp `pkg-config --cflags --libs vpl`
```


## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for more information.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file
for details.

## Security

See the [Intel Security Center](https://www.intel.com/content/www/us/en/security-center/default.html) for information on how to report a potential
security issue or vulnerability.
