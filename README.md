# oneAPI Video Processing Library

The oneAPI Video Processing Library (oneVPL) provides a single video processing
API for encode, decode, and video processing that works across a wide range of
accelerators.

This repository contains the following components of oneVPL:

- Copies of the oneVPL Specification API header files
- oneVPL dispatcher
- Examples demonstrating API usage
- oneVPL command line tools

This project is part of the larger [oneAPI](https://www.oneapi.com/) project.
See the [oneAPI Specification](https://spec.oneapi.com) and the
[oneVPL Specification](https://spec.oneapi.com/versions/latest/elements/oneVPL/source/index.html) for additional information.

The version of the oneVPL API is listed in the
[mfxdefs.h](./api/vpl/mfxdefs.h) file.

## Prerequisites

To build this project you will need:

- A compiler with C++11 support
- CMake 3.10 or newer

## Build and Installation of this package

Build the project with the following commands:

```
cd <vpl-repo-clone-location>

mkdir _build
cd _build
cmake .. -DCMAKE_INSTALL_PREFIX=<vpl-install-location>
cmake --build . --config Release
# optional, create a portable zip file from the build
# cmake --build . --config Release --target package
cmake --build . --config Release --target install
```

You can find the build output in `<vpl-install-location>`.

Helper scripts are avalible in the `script` folder.

- bootstrap : builds any dependencies (none at this time). Built dependencies will be placed in the location stored in VPL_BUILD_DEPENDENCIES if the variable is defined.
- clean : removes intermediate build results
- build : builds the package (all but the last step above)
- install : installs the package (the last step above). Package will be installed in the location stored in VPL_INSTALL_DIR if the variable is defined.
- test : run any smoke testing on the installed package

You can build the project using the scripts with the follwoing commands
```
cd <vpl-repo-clone-location>
script/bootstrap
script/build
script/install
```

Optionaly the environment variable VPL_INSTALL_DIR can be set to specify a location for the scripts to install the project.

## Getting an Implementation

To use oneVPL for video processing you need to install at least one
implementation. Current implementations:

- [oneVPL-cpu](https://github.com/oneapi-src/oneVPL-cpu), the CPU reference
  implementation

## Building Base and CPU Implementations

Normally you don't just want to build the base package, which is limited to the dispatcher and samples. You want to
be able to do actual work like decoding video. To do this you need an implemetnation. As described above the CPU
implementation is delivered alongside the Base package.

You can extend the project build commands above as follows to also build the CPU implementation.

```
cd <vpl-repo-clone-location>

mkdir _build
cd _build
cmake .. -DCMAKE_INSTALL_PREFIX=<vpl-install-location>
cmake --build . --config Release
cmake --build . --config Release --target package
cmake --build . --config Release --target install

cd <vpl-cpu-repo-clone-location>

# optional, specify an external cache folder to store dependencies
export VPL_BUILD_DEPENDENCIES=<dependencies-cache>

# Build 3rd party dependencies.
source script/bootstrap

mkdir _build
cd _build
cmake .. -DCMAKE_INSTALL_PREFIX=<vpl-install-location>
cmake --build . --config Release
cmake --build . --config Release --target package
cmake --build . --config Release --target install
```

You can find the build output in `<vpl-install-location>`.

Helper scripts to build Base and CPU implemenation are avalible in the `script/e2e` folder.

- bootstrap : builds any dependencies.
- clean : removes intermediate build results
- build : builds and installs the package
- test : run any smoke testing on the installed package

These scripts assume oneVPL and oneVPL-cpu are cloned into the same parent folder

Note: `.bat` versions are also provided for Windows.

### Using End-To-End Scripts

You can build oneVPL base and CPU implementation with the End-To-End scripts using the following commands:

```
cd <vpl-root>
oneVPL/script/e2e/bootstrap
oneVPL/script/e2e/build
```

Optionaly you may set the environment variables VPL_INSTALL_DIR and
VPL_BUILD_DEPENDENCIES to specify a location to install the project and to
store the built dependencies respectivly.


```
cd <vpl-root>

export VPL_BUILD_DEPENDENCIES=<dependencies-cache>
export VPL_INSTALL_DIR=<vpl-install-location>

oneVPL/script/e2e/bootstrap
oneVPL/script/e2e/build
```

You can also clear build results by calling the clean script.
Note, if VPL_INSTALL_DIR is set it will be cleared too.


```
cd <vpl-root>

oneVPL/script/e2e/clean
```

A complete rebuild can be forced by cleaning and then rebuilding.

```
cd <vpl-root>

export VPL_BUILD_DEPENDENCIES=<dependencies-cache>
export VPL_INSTALL_DIR=<vpl-install-location>

oneVPL/script/e2e/clean
oneVPL/script/e2e/bootstrap
oneVPL/script/e2e/build
```

## Developer Usage

### Configure the Environment

If you did not install to standard system locations, you need to set up the
environment, so tools like CMake and pkg-config can find the library and
headers.

For Linux:
```
source <vpl-install-location>/env/vars.sh
```

For Windows:
```
<vpl-install-location>\env\vars.bat
```


### Link to oneVPL with CMake

Add the following code to your CMakeLists, assuming TARGET is defined as the
component that wants to use oneVPL:

```
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
