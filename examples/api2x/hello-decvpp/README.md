# `hello-decvpp` Sample

This sample shows how to use the Intel® Video Processing Library (Intel® VPL) 2.X common API to
perform simple video decode with fused VPP.

| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04/22.04; Windows* 10
| Hardware         | Compatible with Intel® VPL GPU implementation, which can be found at https://github.com/intel/vpl-gpu-rt 
|                  | and Intel® Media SDK GPU implementation, which can be found at https://github.com/Intel-Media-SDK/MediaSDK
| What You Will Learn | How to use Intel® VPL to perform fused decode + frame processing from an H.265 encoded video file
| Time to Complete | 5 minutes


## Purpose

This sample is a command line application that takes a file containing an H.265
video elementary stream as an argument, decodes it and vpp the  output with Intel® VPL, and writes the decode output to a the file `dec_out.raw` in raw native format and the two vpp outputs to "vpp_640x480_out.raw" in raw native format and "vpp_128x96_out.raw" in raw BGRA format.

Native raw frame format: GPU=NV12.

## Key Implementation details

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | GPU
| Input format      | H.265 video elementary stream
| Output format     | NV12 BGRA
| Output resolution | decode: same as input stream, vpp: 640x480, 128x96


## License

Code samples are licensed under the MIT license.

## Building the `hello-decvpp` Program

### Include Files
The Intel® VPL include folder is located at these locations on your development system:
 - Windows: <vpl_install_dir>\include 
 - Linux: <vpl_install_dir>/include


### On a Linux* System

Perform the following steps:

1. Install prerequisites. To build and run the sample you need to
   install prerequisite software and set up your environment:

   - Follow the steps in [install.md](https://github.com/intel/libvpl/blob/master/INSTALL.md) or install libvpl-dev. 
   - Follow the steps in [dgpu-docs](https://dgpu-docs.intel.com/) according to your GPU.
   - Install the packages using following commands:
   ```
   apt update
   apt install -y cmake build-essential pkg-config libva-dev libva-drm2 vainfo
   ```

2. Set up your environment using the following command.
   ```
   source <vpl_install_dir>/etc/vpl/vars.sh
   ```
   Here `<vpl_install_dir>` represents the root folder of your Intel® VPL
   installation.  If you customized the
   installation folder, it is in your custom location.

3. Build the program using the following commands:
   ```
   mkdir build
   cd build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake --build .
   ```

4. Run the program using the following command:
   ```
   ./hello-decvpp -i ../../../content/cars_320x240.h265
   ```

### On a Windows* System Using Visual Studio* Version 2017 or Newer

#### Building the program using CMake

1. Install prerequisites. To build and run the sample you need to
   install prerequisite software and set up your environment:

   - Follow the steps in [install.md](https://github.com/intel/libvpl/blob/master/INSTALL.md) to install Intel® VPL package.
   - Visual Studio 2022
   - [CMake](https://cmake.org)

2. Set up your environment using the following command.
   ```
   <vpl_install_dir>\etc\vpl\vars.bat
   ```
   Here `<vpl_install_dir>` represents the root folder of your Intel® VPL
   installation. If you customized the installation
   folder, the `vars.bat` is in your custom location.  Note that if a
   compiler is not installed you should run in a Visual
   Studio 64-bit command prompt.

3. Build the program with default arguments using the following commands:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

4. Run the program using the following command:
   ```
   Release\hello-decvpp -i ..\..\..\content\cars_320x240.h265
   ```


## Running the Sample

### Example Output

```
Implementation details:
  ApiVersion:           2.8
  Implementation type:  HW
  AccelerationMode via: D3D11
  DeviceID:             56a6/0
  Path: C:\Windows\System32\DriverStore\FileRepository\iigd_dch.inf_amd64_a35f92e9f7f89b10\libmfx64-gen.dll

Output colorspace: NV12
Decoding and VPP ..\..\..\content\cars_320x240.h265 -> dec_out.raw and vpp_640x480_out.raw, vpp_128x96_out.raw
Decode and VPP processed 30 frames
```

You can find the output file `dec_out.raw`, `vpp_640x480_out.raw`, and `vpp_128x96_out.raw` in the build directory.

You can display the output with a video player that supports raw streams such as
FFplay. You can use the following command to display the output with FFplay:

```
ffplay -video_size 320x240 -pixel_format yuv420p -f rawvideo dec_out.raw
ffplay -video_size 640x480 -pixel_format yuv420p -f rawvideo vpp_640x480_out.raw
ffplay -video_size 128x96 -pixel_format bgra -f rawvideo vpp_128x96_out.raw
```

Use nv12 for pixel_format for GPU output.
