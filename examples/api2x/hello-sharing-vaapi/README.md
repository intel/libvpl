# `hello-sharing-vaapi` Sample

This sample shows how to use the Intel® Video Processing Library (Intel® VPL) 2.X common API to :
- Perform video encode for a VA-API surface with surface sharing.
- Perform video decode and export mfxFrameSurface1 from Intel® VPL to a VA-API surface with surface sharing.

| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04/22.04;
| Hardware         | Compatible with Intel® VPL GPU implementation, which can be found at https://github.com/intel/vpl-gpu-rt 
| What You Will Learn | How to use Intel® VPL to encode a VA-API surface to H.265 and decode an H.265 encoded video file and save raw frame using VA-API surface sharing
| Time to Complete | 10 minutes


## Purpose

This sample has two command line applications:
- `hello-sharing-vaapi-import`: takes a file containing a raw format video elementary stream as an argument. The application creates a VA-API surface based on the input video stream to be able to illustrate VAAPI surface sharing encoding. Using Intel® VPL, the application encodes the VA-API surface and writes the encoded output to `a out.h265` in H.265 format. Native raw frame input format: GPU=NV12.
- `hello-sharing-vaapi-export`: takes a file containing an H.265
video elementary stream as an argument. Using Intel® VPL, the application decodes, exports mfxFrameSurface1 to a VA-API surface and writes the decoded surface to a file `out.raw` in raw format. Native raw frame output format: GPU=NV12.

## Key Implementation details

### `hello-sharing-vaapi-import`

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | GPU
| Input format      | NV12
| Output format     | H.265 video elementary stream
| Output resolution | same as the input

### `hello-sharing-vaapi-export`

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | GPU
| Input format      | H.265 video elementary stream
| Output format     | NV12
| Output resolution | same as the input

## License

Code samples are licensed under the MIT license.

## Building the `hello-sharing-vaapi` Project

### Include Files
The Intel® VPL include folder is located at these locations on your development system:
 - Windows: <vpl_install_dir>\include 


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
   cmake .. -DBUILD_EXAMPLES=ON
   cmake --build . --config Release
   ```
> Note: To build just one of the applications, specify `-t` followed by the name of the executable. For instance to build the import application: ```cmake --build . -t hello-sharing-vaapi-import```

## Running the Sample

1. For import, run the program using the following command:
   ```
   ./hello-sharing-vaapi-import -i ../../../content/cars_320x240.nv12 -w 320 -h 240
   ```
   and sample output is shown below:
   ```
   Implementation details:
   ApiVersion:           2.9
   Implementation type:  HW
   AccelerationMode via: VAAPI
   DeviceID:             4682/0
   Path:  <Path>/_build/__bin/Debug/libmfx-gen.so.1.2.9
   
   Encoding ../examples/../content/cars_320x240.nv12-> out.h265
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
   ./hello-sharing-vaapi-export -i ../../../content/cars_320x240.h265
   ```
   and sample output is shown below:
   ```
   Implementation details:
   ApiVersion:           2.9
   Implementation type:  HW
   AccelerationMode via: VAAPI
   DeviceID:             4682/0
   Path: <Path>/_build/__bin/Debug/libmfx-gen.so.1.2.9
   
   Decoding ../examples/../content/cars_320x240.h265 -> out.raw
   Output colorspace: NV12
   Decoded 30 frames
   ```
   
   You can find the output file `out.raw` in the build directory.
   
   You can display the output with a video player that supports raw streams such as
   FFplay. You can use the following command to display the output with FFplay:
   
   ```
   ffplay -video_size 320x240 -pixel_format nv12 -f rawvideo out.raw
   ```

