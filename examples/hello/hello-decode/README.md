# `hello-decode` Sample

This sample shows how to use the oneAPI Video Processing Library (oneVPL) to
perform a simple video decode.

| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04; Windows* 10
| Hardware         | Compatible with Intel® oneAPI Video Processing Library(oneVPL) GPU implementation, which can be found at https://github.com/oneapi-src/oneVPL-intel-gpu 
|                  | and Intel® Media SDK GPU implementation, which can be found at https://github.com/Intel-Media-SDK/MediaSDK
| Software         | Intel® oneAPI Video Processing Library(oneVPL)
| What You Will Learn | How to use oneVPL to decode an H.265 encoded video file
| Time to Complete | 5 minutes


## Purpose

This sample is a command line application that takes a file containing an H.265
video elementary stream as an argument. Using oneVPL, the application decodes 
and writes the decoded output to a file `out.raw` in raw format.

Native raw frame output format: CPU=I420, GPU=NV12.

## Key Implementation details

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | CPU
| Input format      | H.265 video elementary stream
| Output format     | I420
| Output resolution | same as the input


## License

Code samples are licensed under the MIT license. See
[License.txt](https://github.com/oneapi-src/oneAPI-samples/blob/master/License.txt) for details.


## Building the `hello-decode` Program

### Include Files
The oneVPL include folder is located at these locations on your development system:
 - Windows: %ONEAPI_ROOT%\vpl\latest\include 
 - Linux: $ONEAPI_ROOT/vpl/latest/include


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

4. Run the program with default arguments using the following command:
   ```
   cmake --build . --target run
   ```

### On a Windows* System Using Visual Studio* Version 2017 or Newer

#### Building the program using CMake

1. These instructions assume you can read and write to the location 
   the examples are stored. If the examples have been installed in a
   protected folder such as "Program Files" copy the entire `examples`
   folder to a location with Read/Write access such as the Desktop
   (%USERPROFILE%\Desktop) and resume these instruictions from that copy.

2. Install the prerequisite software. To build and run the sample you need to
   install prerequisite software and set up your environment:

   - Intel® oneAPI Base Toolkit for Windows*
   - [CMake](https://cmake.org)

3. Set up your environment using the following command.
   ```
   <oneapi_install_dir>\setvars.bat
   ```
   Here `<oneapi_install_dir>` represents the root folder of your oneAPI
   installation, which is which is `C:\Program Files (x86)\Intel\oneAPI\`
   when installed using default options. If you customized the installation
   folder, the `setvars.bat` is in your custom location.  Note that if a
   compiler is not part of your oneAPI installation you should run in a Visual
   Studio 64-bit command prompt.

4. Build the program with default arguments using the following commands:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

5. Run the program using the following command:
   ```
   cmake --build . --target run
   ```


## Running the Sample

### Application Parameters

The instructions given above run the sample executable with the argument
`-i ${CONTENTPATH}/cars_128x96.h265`.


### Example Output

```
Implementation details:
  ApiVersion:           2.5  
  Implementation type:  SW
  AccelerationMode via: NA 
  Path: /opt/intel/oneapi/vpl/2021.6.0/lib/libvplswref64.so.1

Decoding /home/test/intel_innersource/frameworks.media.onevpl.dispatcher/examples/hello/hello-decode/content/cars_128x96.h265 -> out.raw
Output colorspace: I420 (aka yuv420p)
Decoded 60 frames
```

You can find the output file `out.raw` in the build directory.

You can display the output with a video player that supports raw streams such as
FFplay. You can use the following command to display the output with FFplay:

```
ffplay -video_size 128x96 -pixel_format yuv420p -f rawvideo out.raw
```

Use nv12 for pixel_format for GPU output.
