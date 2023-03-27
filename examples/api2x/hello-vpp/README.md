# `hello-vpp` Sample

This sample shows how to use the oneAPI Video Processing Library (oneVPL) 2.X common API to
perform simple video processing.

| Optimized for       | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04/22.04; Windows* 10
| Hardware         | Compatible with Intel® oneAPI Video Processing Library(oneVPL) GPU implementation, which can be found at https://github.com/oneapi-src/oneVPL-intel-gpu 
|                  | and Intel® Media SDK GPU implementation, which can be found at https://github.com/Intel-Media-SDK/MediaSDK
| What You Will Learn | How to use oneVPL to resize and change color format of a raw video file
| Time to Complete    | 5 minutes

Expected input/output formats:
* In: GPU=NV12 color planes
* Out: BGRA color planes

## Purpose

This sample is a command line application that takes a file containing a raw
format video elementary stream as an argument. Using oneVPL, the application
processes it and writes the resized output to `out.raw` in BGRA raw video format.

Native raw frame input format: GPU=NV12.

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | GPU
| Input format      | NV12
| Output format     | BGRA raw video elementary stream
| Output resolution | 640 x 480

## License

Code samples are licensed under the MIT license.

## Building the `hello-vpp` Program

### Include Files
The oneVPL include folder is located at these locations on your development system:
 - Windows: <vpl_install_dir>\include 
 - Linux: <vpl_install_dir>/include


### On a Linux* System

Perform the following steps:

1. Install prerequisites. To build and run the sample you need to
   install prerequisite software and set up your environment:

   - Follow the steps in [install.md](https://github.com/intel-innersource/frameworks.media.onevpl.dispatcher/blob/main/INSTALL.md) or install libvpl-dev. 
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
   Here `<vpl_install_dir>` represents the root folder of your oneVPL
   installation.  If you customized the
   installation folder, it is in your custom location.

3. Build the program using the following commands:
   ```
   mkdir build
   cd build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake --build .
   ```

4. Run the program with default arguments using the following command:
   ```
   ./hello-vpp -hw -i ../../../content/cars_320x240.i420 -w 320 -h 240
   ```

### On a Windows* System Using Visual Studio* Version 2017 or Newer

#### Building the program using CMake

1. Install prerequisites. To build and run the sample you need to
   install prerequisite software and set up your environment:

   - Follow the steps in [install.md](https://github.com/intel-innersource/frameworks.media.onevpl.dispatcher/blob/main/INSTALL.md) to install oneVPL package.
   - Visual Studio 2022
   - [CMake](https://cmake.org)

2. Set up your environment using the following command.
   ```
   <vpl_install_dir>\etc\vpl\vars.bat
   ```
   Here `<vpl_install_dir>` represents the root folder of your oneVPL
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
   Release\hello-vpp -hw -i ..\..\..\content\cars_320x240.i420 -w 320 -h 240
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

Processing ../../../content/cars_320x240.i420 -> out.raw
Processed 30 frames
```

You can find the 640x480 BGRA output file `out.raw` in the build directory.

You can display the output with a video player that supports raw streams such as
FFplay. You can use the following command to display the output with FFplay:

```
ffplay -video_size 640x480 -pixel_format bgra -f rawvideo out.raw
```
