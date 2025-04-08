# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

This project is part of the Intel® Video Processing Library (Intel® VPL), a
programming interface that provides access to hardware accelerated video decode,
encode, and frame processing capabilities on Intel® GPUs to support AI visual
inference, media delivery, cloud gaming, and virtual desktop infrastructure use
cases.

> [!IMPORTANT]
>
> Intel® VPL Tools are no longer in this repository. They have all been moved to
> https://github.com/intel/libvpl-tools

## [Unreleased]

## [2.14.0] - 2025-04-11

### Added
- Intel® VPL API 2.15 support, including new property-based capabilities query
  interface, extended decoder and encoder capabilities reporting, and
  definitions for VVC Main 10 Still Picture profile and level 6.3.
- Explicit `INSTALL_EXAMPLES` build option to control installation of example
  source code and content

### Changed
- Model demonstrated interop example to a vehicle detection model

### Fixed
- `BUILD_EXAMPLES` build option requiring `INSTALL_DEV` to have any effect

### Changed
- Default Ubuntu build to 24.04

### Removed
- Outdated Dockerfiles provided with examples

## [2.14.0] - 2024-12-13

### Added
- Intel® VPL API 2.14 support, including new quality and speed settings for AI
  based video frame interpolation, new algorithm and mode selection options for
  AI based super resolution, and HEVC level 8.5 decode support
- Improved support for Python 3.12 development environments

### Fixed
- Bootstrap to support Debian distributions that do not define `ID_LIKE`

## [2.13.0] - 2024-08-30

### Added
- Intel® VPL API 2.13 support, including new APIs for AV1 screen content tools,
  encoded picture quality information, alpha channel encoding, AI-based frame
  interpolation, AI-based super resolution, and Battlemage platform
- hello-encode-jpeg example

## [2.12.0] - 2024-06-28

### Added
- Intel® VPL API 2.12 support, including new APIs for AI-based frame
  interpolation and Vulkan zero-copy surface sharing

## [2.11.0] - 2024-04-26

### Added
- Intel® VPL API 2.11 support

### Removed
- Command line tools. They have been moved to a separate repository
  (https://github.com/intel/libvpl-tools)

## [2.10.2] - 2024-02-21

### Fixed
- "®" symbol in pkg-config file breaking downstream build tools
- CMake not installing metrics monitor and cttmetrics library

## [2.10.1] - 2023-12-22

### Added
- Intel® VPL specification and associated build scripts

### Changed
- Project name to Intel® Video Processing Library (Intel® VPL)
- Project versioning scheme to align with library and API version

## [2023.4.0] - 2023-12-01

### Added
- Intel® VPL API 2.10 support
- New command line parameters to sample_multi_transcode for setting
  B Frame QP Offsets based on Pyramid Level
- New command line parameters to the sample_vpp tool to enable video signal
  information
- New APIs for importing and exporting shared surfaces
- New APIs for string-based parameter configuration

### Removed
- Environment module support
- Unused files and folders

### Fixed
- NV12 wayland render for MTL
- Buffer starvation when wayland window is moved
- Missing profile strings in vpl-inspect tool
- Rdrm to use card node instead of render node
- Rdrm tile4 selection based DRM/KMS query
- Rdrm NV12 and P010 render for MTL

### Changed
- Directory layout and name of dispatcher directory
- Location of image files used by documentation
- Runtime library search path to remove current directory

## [2023.3.1] - 2023-07-21

### Added
- Three new command line parameters to the sample_multi_transcode tool to enable
  video quality tuning for different use cases.
- New command line parameter to the sample_multi_transcode tool to enable
  adaptiveCQM control

### Removed
- Outdated docker files

### Fixed
- Unsafe calling of SetCurrentDirectory during MFXEnumImplementations
- Parameter parsing in sample_vpp tool

## [2023.3.0] - 2023-05-31

### Removed
- USE_ONEAPI_INSTALL_LAYOUT build shortcut
- '-sw' option from examples

### Added
- New MFXSetConfigFilterProperty property: `mfxExtendedDeviceId.RevisionID`

### Fixed
- Fix HDR luminance value
- Handling of special cases of CMAKE_INSTALL_PREFIX
- X11 rendering corruption issue
- Intel® Distribution of OpenVINO™ Toolkit installation steps in vpl-infer README
- Intermittent Sample_multi_transcode segfault on wayland

### Changed
- quick-start scripts to be more simple and easy to use

## [2023.2.1] - 2023-04-14

### Added
- Documentation on security policy and vulnerability reporting process

### Removed
- Fix for VPP processing for YUV input

## [2023.2.0] - 2023-04-07

### Added
- Intel® VPL API 2.9 support
- Perceptual encoding prefilter option to sample_multi_transcode

### Fixed
- VPP processing for YUV input
- Sample_multi_transcode segfault on wayland
- Missing prerequisites in vpl-infer README

### Changed
- vpl-infer Dockerfile and Linux setup steps to better align with https://dgpu-docs.intel.com/

### Removed
- dpcpp-blur example

## [2023.1.3] - 2023-03-03

### Added
 - New tutorial on transitioning from Media SDK to Intel® VPL replaces hello-createsession
 - More logs in sample tools to inform correct parameters for hyper encode mode

### Fixed
- vpl-infer README.md to specify python version supported by OpenVINO™ toolkit
- Printing incorrect library information of sample tools
- Issues discovered from static analysis

### Changed
- README.md and INSTALL.md to clarify setup steps

## [2023.1.2] - 2023-01-27

### Added
- Instructions on how to use vpl-infer with official Intel® Deep Learning
  Streamer (Intel® DL Streamer) docker images by platform
- Support for zero-copy to vpl-infer example on Windows

### Fixed
- vpl-infer Dockerfile failing to work in iGPU (Gen12) and to work with recent
  OpenVINO™ toolkit repository label change
- Warnings reported by Clang
- Hardening flags being incorrectly set on Linux executables

### Changed
- Example directory names to clarify API level used
- vpl-infer to support OpenVINO™ toolkit 2022.3.0
- Version of GoogleTest to 1.12.1
- Compilation flags to enable Control Flow Guard on Windows

## [2023.1.1] - 2022-12-23

### Added
- Help screen listing valid options for vpl-inspect
- YUV400 option for JPEG encoding with sample_encode
- Build option --disable_experimental to build with ONEVPL_EXPERIMENTAL disabled

### Changed
- Session creation example to request a minimum API version

## [2023.1.0] - 2022-11-10

### Added
- Intel® VPL API 2.8 support
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
- Intel® VPL API 2.7 support
- Experimental DeviceCopy option for GPU-accelerated surface copying
- Ability to select host or device responsible for the memory copy between host
  and device
- `MFX_FOURCC_XYUV` FourCC for non-alpha packed 4:4:4 format
- Notice to `mfxFrameSurfaceInterface::OnComplete` to clarify when library can
  call this callback
- Pass through extension buffer to mfxInitializationParam via config filter property
- Interface to get statistics after encode to Experimental API
- Support for Alder Lake N and Intel® Data Center GPU Flex Series (formerly Arctic Sound-M)
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
- Outdated OpenVINO™ toolkit path in interop Samples README
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
- Intel® VPL API 2.6 support
- `ONEVPL_PRIORITY_PATH` environment variable for runtime loading
- Multi-adapter support for most combinations of GPU hardware that works with
  Intel® Media SDK and Intel® VPL
- Documentation on working with multiple adapters using 2.x API
- Support for extended device ID on legacy GPUs
- Support for Intel® Arc™ A Series Graphics
- Rocky Linux* support
- Windows* 11 support
- Windows* Server 2022 support
- Microsoft Visual Studio* 2022 support

### Changed
- Runtime loading to use Intel® Media SDK when DX9 is requested

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
- Sample* tools to select Intel® VPL 2.x APIs by default
- Python previews to target Python 3.7
- Documentation and build for OpenVINO™ toolkit interop sample

### Removed
- Previously depreciated libmfx.dll and libmfx.so.2021.1.11 libraries

## [2021.6.0] - 2021-09-13

### Added
- Intel® VPL API 2.4 support
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

- Intel® VPL API 2.4 support
- Advanced OpenVINO™ toolkit interop sample w/ remoteBlob support (Linux only)
- C++ API and Samples (Preview)
- Python API and Samples (Preview)
- Dispatcher logging capability
- sample_multi_transcode tool
- decvpp_tool
- Debug libraries in Windows package

## [2021.2.2] - 2021-04-01

### Added
- Intel® VPL API 2.2 support
- Legacy Intel® Media SDK test tools (with CPU and GPU support)
- Intel® Media SDK to Intel® VPL migration guide
- pkg-config support
- 32-bit Windows* support
- Legacy mfxvideo++ header
- New Preview Sample: Intel® VPL OpenVINO™ toolkit Interop Sample
- Ubuntu 20.10 support

## [2021.1] - 2020-12-08

### Added
- Intel® VPL dispatcher implementation supporting Intel® VPL API 2.0
- Frame memory interface with access to internally allocated buffers
- hello-decode sample
- hello-encode sample
- hello-vpp sample
- dpcpp-blur sample
- hello-transcode sample


[Unreleased]: https://github.com/intel/libvpl/compare/v2.15.0...HEAD
[2.15.0]: https://github.com/intel/libvpl/compare/v2.14.0...v2.15.0
[2.14.0]: https://github.com/intel/libvpl/compare/v2.13.0...v2.14.0
[2.13.0]: https://github.com/intel/libvpl/compare/v2.12.0...v2.13.0
[2.12.0]: https://github.com/intel/libvpl/compare/v2.11.0...v2.12.0
[2.11.0]: https://github.com/intel/libvpl/compare/v2.10.2...v2.11.0
[2.10.2]: https://github.com/intel/libvpl/compare/v2.10.1...v2.10.2
[2.10.1]: https://github.com/intel/libvpl/compare/v2023.4.0...v2.10.1
[2023.4.0]: https://github.com/intel/libvpl/compare/v2023.3.1...v2023.4.0
[2023.3.1]: https://github.com/intel/libvpl/compare/v2023.3.0...v2023.3.1
[2023.3.0]: https://github.com/intel/libvpl/compare/v2023.2.1...v2023.3.0
[2023.2.1]: https://github.com/intel/libvpl/compare/v2023.2.0...v2023.2.1
[2023.2.0]: https://github.com/intel/libvpl/compare/v2023.1.3...v2023.2.0
[2023.1.3]: https://github.com/intel/libvpl/compare/v2023.1.2...v2023.1.3
[2023.1.2]: https://github.com/intel/libvpl/compare/v2023.1.1...v2023.1.2
[2023.1.1]: https://github.com/intel/libvpl/compare/v2023.1.0...v2023.1.1
[2023.1.0]: https://github.com/intel/libvpl/compare/v2023.0.0...v2023.1.0
[2023.0.0]: https://github.com/intel/libvpl/compare/v2022.2.4...v2023.0.0
[2022.2.4]: https://github.com/intel/libvpl/compare/v2022.2.3...v2022.2.4
[2022.2.3]: https://github.com/intel/libvpl/compare/v2022.2.2...v2022.2.3
[2022.2.2]: https://github.com/intel/libvpl/compare/v2022.2.1...v2022.2.2
[2022.2.1]: https://github.com/intel/libvpl/compare/v2022.2.0...v2022.2.1
[2022.2.0]: https://github.com/intel/libvpl/compare/v2022.1.6...v2022.2.0
[2022.1.6]: https://github.com/intel/libvpl/compare/v2022.1.5...v2022.1.6
[2022.1.5]: https://github.com/intel/libvpl/compare/v2022.1.4...v2022.1.5
[2022.1.4]: https://github.com/intel/libvpl/compare/v2022.1.3...v2022.1.4
[2022.1.3]: https://github.com/intel/libvpl/compare/v2022.1.2...v2022.1.3
[2022.1.2]: https://github.com/intel/libvpl/compare/v2022.1.1...v2022.1.2
[2022.1.1]: https://github.com/intel/libvpl/compare/v2022.1.0...v2022.1.1
[2022.1.0]: https://github.com/intel/libvpl/compare/v2022.0.6...v2022.1.0
[2022.0.6]: https://github.com/intel/libvpl/compare/v2022.0.5...v2022.0.6
[2022.0.5]: https://github.com/intel/libvpl/compare/v2022.0.4...v2022.0.5
[2022.0.4]: https://github.com/intel/libvpl/compare/v2022.0.3...v2022.0.4
[2022.0.3]: https://github.com/intel/libvpl/compare/v2022.0.2...v2022.0.3
[2022.0.2]: https://github.com/intel/libvpl/compare/v2022.0.0...v2022.0.2
[2022.0.0]: https://github.com/intel/libvpl/compare/v2021.6.0...v2022.0.0
[2021.6.0]: https://github.com/intel/libvpl/compare/v2021.5.0...v2021.6.0
[2021.5.0]: https://github.com/intel/libvpl/compare/v2021.4.0...v2021.5.0
[2021.4.0]: https://github.com/intel/libvpl/compare/v2021.2.2...v2021.4.0
[2021.2.2]: https://github.com/intel/libvpl/compare/v2021.1]:...v2021.2.2
[2021.1]: https://github.com/intel/libvpl/releases/tag/v2021.1
