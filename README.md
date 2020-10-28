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
See **Contributing** below for details about submitting issues and pull
requests.

oneAPI Specification: 
https://spec.oneapi.com

oneVPL Specification: 
https://spec.oneapi.com/versions/latest/elements/oneVPL/source/index.html

The version of the oneVPL API is listed in the file
[mfxdefs.h](./api/vpl/mfxdefs.h).


## Building and Installing

### Prerequisites

- Compiler with C++11 support
- CMake 3.10 or newer


### General

```
mkdir _build
cd _build
cmake .. -DCMAKE_INSTALL_PREFIX=<vpl-install-location>
cmake --build . --config Release
cmake --build . --config Release --target install
```

You can find the build output in `<vpl-install-location>`.


## Getting an Implementation

To use oneVPL for video processing to you need to install at least one
implementation:

- [oneVPL-cpu](https://github.com/oneapi-src/oneVPL-cpu) - CPU reference
  implementation


## Contributing

Please see the [CONTRIBUTING.md](CONTRIBUTING.md) file for details.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file
for details.
