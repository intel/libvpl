# ![oneAPI](assets/oneapi-logo.png "oneAPI") Video Processing Library

The oneAPI Video Processing Library (oneVPL) provides a single video processing
API for encode, decode, and video processing that works across a wide range of
accelerators.

This repository contains the following components of oneVPL:

- Copies of the oneVPL Specification API header files
- oneVPL dispatcher
- Examples demonstrating API usage
- oneVPL command line tools

This project is part of the larger [oneAPI](https://www.oneapi.io/) project.
See the [oneAPI Specification](https://spec.oneapi.io) and the
[oneVPL Specification](https://spec.oneapi.io/versions/latest/elements/oneVPL/source/index.html) for additional information.

The version of the oneVPL API is listed in the
[mfxdefs.h](./api/vpl/mfxdefs.h) file.

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
source <vpl-install-location>/share/oneVPL/env/vars.sh
```

For Windows:
```
<vpl-install-location>\share\oneVPL\env\vars.bat
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
