# `Advanced-decvpp-infer` Sample

This sample shows how to use the oneAPI Video Processing Library (oneVPL) to
perform a simple video decode and resize and inference using OpenVINO to show the device surface sharing (zerocopy)

| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04
| Hardware         | Intel® Processor Graphics GEN9 or newer
| Software         | Intel® oneAPI Video Processing Library(oneVPL) and Intel® OpenVINO
| What You Will Learn | How to use oneVPL to decode an H.265 encoded video file and resize and perform per-frame objection detection inference
| Time to Complete | 5 minutes


## Purpose

This sample is a command line application that takes a file containing an H.265
video elementary stream and network model as an argument, decodes and resize it with oneVPL and perform 
object detection on each frame using OpenVINO.


## Key Implementation details

| Configuration       | Default setting
| ------------------  | ----------------------------------
| Target device       | GPU
| Input format        | H.265 video elementary stream
| Input network model | object detection network model

## License

This code sample is licensed under MIT license.


## Building the `advanced-decvpp-infer` Program

### On a Linux* System

Perform the following steps:

1. Install the prerequisite software. To build and run the sample you need to
   install prerequisite software and set up your environment:

   - Intel® oneAPI Base Toolkit for Linux*
   - Intel® OpenVINO 2021.2 for Linux*
   - [Python](http://python.org)
   - [CMake](https://cmake.org)

2. Set up your environment using the following command.
   ```
   source <oneapi_install_dir>/setvars.sh
   source <openvino_install_dir>/bin/setupvars.sh
   ```
   Here `<oneapi_install_dir>` represents the root folder of your oneAPI
   installation, which is `/opt/intel/oneapi/` when installed as root, and
   `~/intel/oneapi/` when installed as a normal user.  `<openvino_install_dir>` 
   represents the root folder of your OpenVINO installation, which is 
   `/opt/intel/openvino/` when installed as root.  If you customized the
   installation folders, it is in your custom location.

3. Build the program using the following commands:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

4. Download the Mobilenet-ssd object detection model from OpenVINO model zoo
   ```
   pip3 install -r <openvino_install_dir>/deployment_tools/model_optimizer/requirements.txt
   pip3 install -r <openvino_install_dir>/deployment_tools/open_model_zoo/tools/downloader/requirements.in
   <openvino_install_dir>/deployment_tools/open_model_zoo/tools/downloader/downloader.py --output_dir ../../content --precisions FP32 --name mobilenet-ssd
   <openvino_install_dir>/deployment_tools/open_model_zoo/tools/downloader/converter.py --download_dir ../../content --name mobilenet-ssd
   ```

5. Run the program using the following command:
   ```
   cmake --build . --target run
   ```




## Running the Sample

### Application Parameters

The instructions given above run the sample executable with the argument
`examples/content/cars_128x96.h265` and `examples/content/public/mobilenet-ssd/FP32/mobilenet-ssd.xml`.


### Example of Output

```
Implementation details:
  ApiVersion:           2.3
  Implementation type:  HW
  AccelerationMode via: VAAPI
  Path: /opt/intel/mediasdk/lib64/libmfx-gen.so.1.2.3

libva info: VA-API version 1.12.0
libva info: Trying to open /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
libva info: Found init function __vaDriverInit_1_12
libva info: va_openDriver() returns 0
Decoding VPP, and infering /home/gta/peter/videos/cars_128x96.h265 with /home/gta/peter/FP32/mobilenet-ssd.xml
[0,7] element, prob = 0.999335    (354,323)-(821,657) batch id : 0 WILL BE PRINTED!
[1,7] element, prob = 0.998945    (831,152)-(1184,434) batch id : 0 WILL BE PRINTED!
[2,7] element, prob = 0.991549    (131,130)-(497,401) batch id : 0 WILL BE PRINTED!
[3,7] element, prob = 0.911602    (253,222)-(622,449) batch id : 0 WILL BE PRINTED!
...
Decoded 60 frames
```
