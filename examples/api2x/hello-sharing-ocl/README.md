# `hello-sharing-ocl` Sample

This sample shows how to use the Intel® Video Processing Library (Intel® VPL) 2.X common API to :
- Perform video encode for a 2D OpenCL image with OpenCL surface sharing.
- Perform video decode and export mfxFrameSurface1 from Intel® VPL to a 2D OpenCL image with surface sharing.

| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Windows* 10
| Hardware         | Compatible with Intel® VPL GPU implementation, which can be installed with the Graphics driver available at https://www.intel.com/content/www/us/en/download-center/home.html
| What You Will Learn | How to use Intel® VPL to encode a 2D OpenCL image to H.265 and decode an H.265 encoded video file with exporting to a 2D OpenCL image and save raw frames.
| Time to Complete | 10 minutes


## Purpose

This sample has two command line applications:
- `hello-sharing-ocl-import`:  takes a file containing a raw format video elementary stream as an argument. The application creates a 2D OpenCL image based on the input video stream to be able to illustrate OpenCL sharing encoding. Using Intel® VPL, the application encodes the 2D OpenCL image and writes the encoded output to `a out.h265` in H.265 format. Native raw frame input format: GPU=NV12.

- `hello-sharing-ocl-export`: takes a file containing an H.265
video elementary stream as an argument. Using Intel® VPL, the application decodes, exports mfxFrameSurface1 to a 2D OpenCL image and writes the decoded image to a file `out.raw` in raw format. Native raw frame output format: GPU=NV12.

## Key Implementation details

### `hello-sharing-ocl-import`

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | GPU
| Input format      | NV12
| Output format     | H.265 video elementary stream
| Output resolution | same as the input

### `hello-sharing-ocl-export`

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | GPU
| Input format      | H.265 video elementary stream
| Output format     | NV12
| Output resolution | same as the input

## License

Code samples are licensed under the MIT license.

## Building the `hello-sharing-ocl` Project

### Include Files
The Intel® VPL include folder is located at these locations on your development system:
 - Windows: <vpl_install_dir>\include


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

3. Install OpenCL dependencies

   Check the instructions in this OpenCL Guide
   ```
   https://github.com/KhronosGroup/OpenCL-Guide/blob/main/chapters/getting_started_windows.md
   ```
   
   Here are steps we tested:
   ```
   git clone --recursive https://github.com/KhronosGroup/OpenCL-SDK.git
   cmake -A x64 -T v143 -D OPENCL_SDK_BUILD_OPENGL_SAMPLES=OFF -B OpenCL-SDK\build -S OpenCL-SDK
   cmake --build  OpenCL-SDK\build --config Release
   cmake --install OpenCL-SDK\build --prefix OpenCL-SDK\install
   set OpenCL_DIR=%cd%\OpenCL-SDK\install
   ```

4. Build the program with default arguments using the following commands:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```
> Note: To build just one of the applications, specify `-t` followed by the name of the executable. For instance to build the import application: ```cmake --build . -t hello-sharing-ocl-import```

## Running the Sample

1. For import, run the program using the following command:
   ```
   Release\hello-sharing-ocl-import -i ..\..\..\content\cars_320x240.nv12 -w 320 -h 240
   ```
   and sample output is shown below:
   ```
   Implementation details:
   ApiVersion:           2.10
   Implementation type: HW
   AccelerationMode via: D3D11
   DeviceID:             46a6/0
   Path: C:\Windows\System32\DriverStore\FileRepository\iigd_dch.inf_amd64_a35f92e9f7f89b10\libmfx64-gen.dll

   Encoding ..\..\..\content\cars_320x240.nv12 -> out.h265
   Input colorspace: NV12
   Encoded 30 frames
   ```
   You can find the output file `out.h265` in the build directory.

   You can display the output with a video player that supports raw streams such as
   FFplay. You can use the following command to display the output with FFplay:

   ```
   ffplay out.h265
   ```

2. For export, run the program using the following command:
   ```
   Release\hello-sharing-ocl-export -i ..\..\..\content\cars_320x240.h265
   ```
   and sample output is shown below
   ```
   Implementation details:
   ApiVersion:           2.10
   Implementation type: HW
   AccelerationMode via: D3D11
   DeviceID:             46a6/0
   Path: C:\Windows\System32\DriverStore\FileRepository\iigd_dch.inf_amd64_a35f92e9f7f89b10\libmfx64-gen.dll

   Decoding ..\..\..\content\cars_320x240.h265 -> out.raw
   Output colorspace: NV12
   Decoded 30 frames
   ```

   You can find the output file `out.raw` in the build directory.

   You can display the output with a video player that supports raw streams such as
   FFplay. You can use the following command to display the output with FFplay:

   ```
   ffplay -video_size 320x240 -pixel_format nv12 -f rawvideo out.raw
   ```
