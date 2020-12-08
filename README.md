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

## Build and Installation

Build the project with the following commands:

```
mkdir _build
cd _build
cmake .. -DCMAKE_INSTALL_PREFIX=<vpl-install-location>
cmake --build . --config Release
cmake --build . --config Release --target install
```

You can find the build output in `<vpl-install-location>`.

## Getting an Implementation

To use oneVPL for video processing you need to install at least one
implementation. Current implementations:

- [oneVPL-cpu](https://github.com/oneapi-src/oneVPL-cpu), the CPU reference
  implementation

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for more information.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file
for details.

## Security

See the [Intel Security Center](https://www.intel.com/content/www/us/en/security-center/default.html) for information on how to report a potential
security issue or vulnerability.
