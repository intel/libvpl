# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [2023.1.0] - 2022-11-10

### Added
- oneVPL API 2.8 support
- New infer sample
- xdg_shell for weston10

### Fixed
- NOT_ENOUGH_BUFFER error when HRD off in ExtBRC
- Legacy tools using x86/x86_64 specific assembler code
- Pkg-config files not installing in correct location in cross compilation
  scenarios

## [2023.0.0] - 2022-10-14

### Changed
- Query thread limit to be enabled for Linux only
- Query sessions to be restricted to two threads

### Removed
- C++ previews
- Python previews
- decvpp_tool

### Fixed
- Typos in tools
- Examples returning 0 even if they fail

## [2022.2.4] - 2022-09-23

### Added
- Parallel encoding support
- Dynamic bitstream buffer reallocation
- Additional color description preset

## [2022.2.3] - 2022-09-16

### Fixed
- Slow mfxBitstream buffer allocation
- Missing UYVY VA-FOURCC causing encode failure

## [2022.2.1] - 2022-08-05

### Added
 - Dockerfile for dpcpp-blur sample

### Fixed
- dpcpp-blur missing API version check

## [2022.2.0] - 2022-07-22

### Added
- oneVPL API 2.7 support
- Experimental DeviceCopy option for GPU-accelerated surface copying
- Ability to select host or device responsible for the memory copy between host
  and device
- `MFX_FOURCC_XYUV` FourCC for non-alpha packed 4:4:4 format
- Notice to `mfxFrameSurfaceInterface::OnComplete` to clarify when library can
  call this callback
- Pass through extension buffer to mfxInitializationParam via config filter property
- Interface to get statistics after encode to Experimental API
- Support for Alder Lake N and Intel(R) Data Center GPU Flex Series (formerly Arctic Sound-M)
- Linux system_analyzer tool for improved runtime environment visibility

### Changed
- sample_multi_transcode to support HDR 3DLUT, SDR->HDR, VDSFC color conversion,
  improved tracer and latency measurement, AV1 temporal layers, HVS denoise

### Depreciated
- Support for Microsoft Visual Studio* 2017
- Current C++/Python preview APIs. (A different direction will be taken in future releases.)

### Fixed
- Multiple SYCL deprecation warnings when compiling dpcpp-blur
- Sample readmes out of sync with current cmake and docker
- Not turning off tools/examples build when `BUILD_DISPATCHER_ONLY=ON`
- Tools failing to build on SLES if `ENABLE_WAYLAND=ON`
- Build fails against libva installed at custom location
- Unclear error when advanced-decvpp-infer runs on unsupported platform
- Incomplete Linux instructions for dpcpp-blur sample
- Outdated OpenVINO path in interop Samples README
- dpcpp-blur sample not returning clear error when build attempted on
  unsupported operating systems
- `MFXEnumImplementation` not returning `MFX_ERR_NOT_FOUND` on non-Intel systems
- Sample_multi_transcode reporting error when multiline parfile uses -sw flag
- Legacy tools requiring libva-drm.so.2 when run in SW mode
- Inability to enable experimental features for legacy tools on Windows*
- Non-determinism in build behavior depending on machine configuration. Note
  that building tools from source will now require additional dependencies
  instead of quietly proceeding with indeterminate results.

    For Ubuntu:
```
    libx11-dev libx11-xcb-dev libxcb-present-dev libxcb-dri3-dev wayland-protocols libva-dev libdrm-dev
```
    For CentOS/RedHat:
```
    libX11-devel libpciaccess-devel libXext-devel libXfixes-devel wayland-devel  wayland-protocols-devel wayland-devel libdrm-devel libva libdrm-devel
```

## [2022.1.0] - 2022-03-18

### Added
- oneVPL API 2.6 support
- `ONEVPL_PRIORITY_PATH` environment variable for runtime loading
- Multi-adapter support for most combinations of GPU hardware that works with
  Intel(R) Media SDK and oneVPL
- Documentation on working with multiple adapters using 2.x API
- Support for extended device ID on legacy GPUs
- Support for Intel(R) Ar(TM) A Series Graphics
- Rocky Linux* support
- Windows* 11 support
- Windows* Server 2022 support
- Microsoft Visual Studio* 2022 support

### Changed
- Runtime loading to use Intel(R) Media SDK when DX9 is requested

### Deprecated
- Support for Microsoft Visual Studio* 2017

### Fixed
- MFXCloneSession is not functional on legacy GPU systems
- hello-encode not working on DG2 due to default input resolution


## [2022.0.6] - 2022-03-04

## [2022.0.5] - 2022-02-18

## [2022.0.4] - 2022-02-04

## [2022.0.3] - 2022-01-21

## [2022.0.2] - 2021-12-16

## [2022.0.0] - 2021-12-06

### Added
- Support for new GPU features in Sample* tools
- AV1 extension buffer support in C++ preview
- New property interface in C++ preview

### Changed
- mfxvideo++.h to remove deprecation warnings
- Sample* tools to select oneVPL 2.x APIs by default
- Python previews to target Python 3.7
- Documentation and build for OpenVINO interop sample

### Removed
- Previously depreciated libmfx.dll and libmfx.so.2021.1.11 libraries

## [2021.6.0] - 2021-09-13

### Added
- oneVPL API 2.4 support
- Internal memory support for dpcpp-blur sample

## [2021.5.0] - 2021-08-03

### Added
- Option to build dispatcher as a static library
- Ability to build dispatcher under MinGW

### Fixed
- Sample builds incorrectly being aligned with release version instead of API
  version
- Numerous minor bugs


## [2021.4.0] - 2021-06-25

### Added

- oneVPL API 2.4 support
- Advanced OpenVINO interop sample w/ remoteBlob support (Linux only)
- C++ API and Samples (Preview)
- Python API and Samples (Preview)
- Dispatcher logging capability
- sample_multi_transcode tool
- decvpp_tool
- Debug libraries in Windows package

## [2021.2.2] - 2021-04-01

### Added
- oneVPL API 2.2 support
- Legacy Intel(R) Media SDK test tools (with CPU and GPU support)
- Intel(R) Media SDK to oneVPL migration guide
- pkg-config support
- 32-bit Windows* support
- Legacy mfxvideo++ header
- New Preview Sample: oneVPL-OpenVINO Interop Sample
- Ubuntu 20.10 support

## [2021.1] - 2020-12-08

### Added
- oneVPL dispatcher implementation supporting oneVPL API 2.0
- Frame memory interface with access to internally allocated buffers
- hello-decode sample
- hello-encode sample
- hello-vpp sample
- dpcpp-blur sample
- hello-transcode sample


[Unreleased]: https://github.com/oneapi-src/oneVPL/compare/v2023.1.0...HEAD
[2023.1.0]: https://github.com/oneapi-src/oneVPL/compare/v2023.0.0...v2023.1.0
[2023.0.0]: https://github.com/oneapi-src/oneVPL/compare/v2022.2.4...v2023.0.0
[2022.2.4]: https://github.com/oneapi-src/oneVPL/compare/v2022.2.3...v2022.2.4
[2022.2.3]: https://github.com/oneapi-src/oneVPL/compare/v2022.2.2...v2022.2.3
[2022.2.2]: https://github.com/oneapi-src/oneVPL/compare/v2022.2.1...v2022.2.2
[2022.2.1]: https://github.com/oneapi-src/oneVPL/compare/v2022.2.0...v2022.2.1
[2022.2.0]: https://github.com/oneapi-src/oneVPL/compare/v2022.1.6...v2022.2.0
[2022.1.6]: https://github.com/oneapi-src/oneVPL/compare/v2022.1.5...v2022.1.6
[2022.1.5]: https://github.com/oneapi-src/oneVPL/compare/v2022.1.4...v2022.1.5
[2022.1.4]: https://github.com/oneapi-src/oneVPL/compare/v2022.1.3...v2022.1.4
[2022.1.3]: https://github.com/oneapi-src/oneVPL/compare/v2022.1.2...v2022.1.3
[2022.1.2]: https://github.com/oneapi-src/oneVPL/compare/v2022.1.1...v2022.1.2
[2022.1.1]: https://github.com/oneapi-src/oneVPL/compare/v2022.1.0...v2022.1.1
[2022.1.0]: https://github.com/oneapi-src/oneVPL/compare/v2022.0.6...v2022.1.0
[2022.0.6]: https://github.com/oneapi-src/oneVPL/compare/v2022.0.5...v2022.0.6
[2022.0.5]: https://github.com/oneapi-src/oneVPL/compare/v2022.0.4...v2022.0.5
[2022.0.4]: https://github.com/oneapi-src/oneVPL/compare/v2022.0.3...v2022.0.4
[2022.0.3]: https://github.com/oneapi-src/oneVPL/compare/v2022.0.2...v2022.0.3
[2022.0.2]: https://github.com/oneapi-src/oneVPL/compare/v2022.0.0...v2022.0.2
[2022.0.0]: https://github.com/oneapi-src/oneVPL/compare/v2021.6.0...v2022.0.0
[2021.6.0]: https://github.com/oneapi-src/oneVPL/compare/v2021.5.0...v2021.6.0
[2021.5.0]: https://github.com/oneapi-src/oneVPL/compare/v2021.4.0...v2021.5.0
[2021.4.0]: https://github.com/oneapi-src/oneVPL/compare/v2021.2.2...v2021.4.0
[2021.2.2]: https://github.com/oneapi-src/oneVPL/compare/v2021.1]:...v2021.2.2
[2021.1]: https://github.com/oneapi-src/oneVPL/releases/tag/v2021.1
