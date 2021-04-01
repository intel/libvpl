# `dpcpp-blur` Sample

This sample shows how to use a DPC++ kernel together with
oneAPI Video Processing Library to perform a simple video content blur.

| Optimized for   | Description
|---------------- | ----------------------------------------
| OS              | Ubuntu* 20.04; Windows* 10
| Hardware        | Intel® Processor Graphics GEN9 or newer
| Software        | Intel® oneAPI Video Processing Library (oneVPL)
| What You Will Learn | How to use oneVPL and DPC++ to convert I420 raw video file in to BGRA and blur each frame.
| Time to Complete | 5 minutes

* I420: YUV color planes
* BGRA: BGRA color planes

## Purpose

This sample is a command line application that takes a file containing a raw
frame input video as an argument, converts it to BGRA with oneVPL and blurs each
frame with DPC++ by using SYCL kernel, and writes the decoded output to `out.raw` 
in BGRA format.

If the oneAPI DPC++ Compiler is not found the blur operation will be disabled.



## Key Implementation details

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | CPU
| Input format      | I420
| Output format     | BGRA raw video elementary stream
| Output resolution | 256x192


## License

This code sample is licensed under MIT license.


## Building the `dpcpp-blur` Program

### On a Linux* System

Perform the following steps:

1. Install the prerequisite software. To build and run the sample you need to
   install prerequisite software and set up your environment:

   - Intel® oneAPI Base Toolkit for Linux*
   - [CMake](https://cmake.org)

2. Set up your environment using the following command.
   ```
   source <oneapi_install_dir>/setvars.sh
   ```
   Here `<oneapi_install_dir>` represents the root folder of your oneAPI
   installation, which is `/opt/intel/oneapi/` when installed as root, and
   `~/intel/oneapi/` when installed as a normal user.  If you customized the
   installation folder, it is in your custom location.

3. Build the program using the following commands:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

4. Run the program using the following command:
   ```
   cmake --build . --target run
   ```


### On a Windows* System Using Visual Studio* Version 2017 or Newer

#### Building the program using CMake

1. Install the prerequisite software. To build and run the sample you need to
   install prerequisite software and set up your environment:

   - Intel® oneAPI Base Toolkit for Windows*
   - [CMake](https://cmake.org)

2. Set up your environment using the following command.
   ```
   <oneapi_install_dir>\setvars.bat
   ```
   Here `<oneapi_install_dir>` represents the root folder of your oneAPI
   installation, which is which is `C:\Program Files (x86)\Intel\oneAPI\`
   when installed using default options. If you customized the installation
   folder, the `setvars.bat` is in your custom location.  Note that if a
   compiler is not part of your oneAPI installation you should run in a Visual
   Studio 64-bit command prompt.

3. Build the program using the following commands:
   ```
   mkdir build
   cd build
   cmake .. -T "Intel(R) oneAPI DPC++ Compiler"
   cmake --build . --config Release
   ```

4. Run the program using the following command:
   ```
   cmake --build . --target run --config Release
   ```


## Running the Sample

### Application Parameters

The instructions given above run the sample executable with the argument
`-i <sample_dir>/content/cars_128x96.i420 -w 128 -h 96`.


### Example of Output

```
Processing ../content/cars_128x96.i420 -> out.raw
Processed 60 frames
```

You can find the output file ``out.raw`` in the build directory.

You can display the output with a video player that supports raw streams such as
FFplay. You can use the following command to display the output with FFplay:

```
ffplay -video_size [256]x[192] -pixel_format bgra -f rawvideo out.raw
```

## Using with USM mapping

Build and install level-zero from https://github.com/oneapi-src/level-zero

dpcpp ../src/dpcpp-blur.cpp -I../../ -lvpl -lva -lva-drm -lpthread -DBUILD_DPCPP -DUSE_VPL_USM_MAP  -lze_loader -I/usr/local/include
