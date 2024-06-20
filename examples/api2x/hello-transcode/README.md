# `hello-transcode` Sample

This sample shows how to use the Intel® Video Processing Library (Intel® VPL) 2.X API to
perform simple video transcode.

| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04; Windows* 10
| Hardware         | Compatible with Intel® VPL GPU implementation, which can be found at https://github.com/intel/vpl-gpu-rt
| What You Will Learn | How to use Intel® VPL to transcode a MJPEG encoded video file to H.265 encoded video file
| Time to Complete | 5 minutes

## Purpose

This sample is a command line application that takes a file containing a JPEG video elementary stream as an argument, decodes it, and encodes the output with Intel® VPL and writes the encoded output to the file `out.h265` in H.265 format.


## Key Implementation details

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | GPU
| Input format      | MJPEG video elementary stream
| Output format     | H.265 video elementary stream
| Output resolution | same as input


## License

Code samples are licensed under the MIT license.


## Building the `hello-transcode` Program

### Include Files

The Intel® VPL include folder is located at these locations on your development system:

 - Windows:<vpl_install_dir>\include
 - Linux:<vpl_install_dir>/include


### On a Linux* System

Perform the following steps:

1. Install the prerequisite software. To build and run the sample you need to
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
   Here <vpl_install_dir> represents the root folder of your Intel® VPL installation. If you customized the installation folder, it is in your custom location.

3. Build the program using the following commands:
   ```
   mkdir build
   cd build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake --build .
   ```

4. Run the program with default arguments using the following command:
   ```
   ./hello-transcode -i ../../../content/cars_320x240.mjpeg
   ```

### On a Windows* System Using Visual Studio* Version 2017 or Newer

#### Building the program using CMake

1. Install prerequisites. To build and run the sample you need to install prerequisite software and set up your environment:

   - Follow the steps in [install.md](https://github.com/intel/libvpl/blob/master/INSTALL.md) to install Intel® VPL package.
   - Visual Studio 2022
   - [CMake](https://cmake.org)

2. Set up your environment using the following command.
   ```
   <vpl_install_dir>\etc\vpl\vars.bat
   ```
   Here <vpl_install_dir> represents the root folder of your Intel® VPL installation. If you customized the installation folder, the vars.bat is in your custom location. 
   Note that if a compiler is not installed you should run in a Visual Studio 64-bit command prompt.

3. Build the program with default arguments using the following commands:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

4. Run the program using the following command:
   ```
   Release\hello-transcode -i ..\..\..\content\cars_320x240.mjpeg
   ```


## Running the Sample

### Example of Output

```
Implementation details:
  ApiVersion:           2.8  
  Implementation type:  HW
  AccelerationMode via: VAAPI
  DeviceID:             56a0/0
  Path: /usr/lib/x86_64-linux-gnu/libmfx-gen.so.1.2.8

Transcoding ../../../content/cars_320x240.mjpeg -> out.h265
Transcoded 30 frames
```

You can find the output file `out.h265` in the build directory.

You can display the output with a video player that supports raw streams such as
FFplay. You can use the following command to display the output with FFplay:

```
ffplay out.h265
```
