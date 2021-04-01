# `hello-decvpp` Sample

This sample shows how to use the oneAPI Video Processing Library (oneVPL) to
perform a simple video decode and vpp for separate output.

| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04; Windows* 10
| Hardware         | Intel® Processor Graphics GEN9 or newer
| Software         | Intel® oneAPI Video Processing Library(oneVPL)
| What You Will Learn | How to use oneVPL to decode an H.265 encoded video file and vpp for additional output
| Time to Complete | 5 minutes


## Purpose

This sample is a command line application that takes a file containing an H.265
video elementary stream as an argument, decodes it and vpp the  output with oneVPL, and writes the decode output to a the file `dec_out.raw` in raw native format and the two vpp outputs to "vpp_320x240_out.raw" in raw native format and "vpp_128x96_out.raw" in raw BGRA format.

Native raw frame format: CPU=I420, GPU=NV12.

## Key Implementation details

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | CPU
| Input format      | H.265 video elementary stream
| Output format     | I420, BGRA
| Output resolution | decode: same as input stream, vpp: 320x240, 128x96


## License

This code sample is licensed under MIT license.


## Building the `hello-decvpp` Program

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
   cmake ..
   cmake --build .
   ```

4. Run the program using the following command:
   ```
   cmake --build . --target run
   ```


## Running the Sample

### Application Parameters

The instructions given above run the sample executable with the argument
`-i examples/content/cars_128x96.h265`.


### Example of Output

```
Implementation info
      version = 2.2
      impl = Software
Decoding hello-decvpp/../content/cars_128x96.h265 -> dec_out.raw and vpp_320x240_out.raw, vpp_128x96_out.raw
Decoded 60 frames
```

You can find the output file `dec_out.raw`, `vpp_320x240_out.raw`, and `vpp_128x96_out.raw` in the build directory.

You can display the output with a video player that supports raw streams such as
FFplay. You can use the following command to display the output with FFplay:

```
ffplay -video_size 128x96 -pixel_format yuv420p -f rawvideo dec_out.raw
ffplay -video_size 320x240 -pixel_format yuv420p -f rawvideo vpp_320x240_out.raw
ffplay -video_size 128x96 -pixel_format bgra -f rawvideo vpp_128x96_out.raw
```

Use nv12 for pixel_format for GPU output.
