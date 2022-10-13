# `vpl-infer` Sample

This sample shows how to use the oneAPI Video Processing Library (oneVPL) to
perform a simple video decode and resize and inference using OpenVINO to show the device surface sharing (zero copy)

| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04
| Hardware         | Compatible with Intel® oneAPI Video Processing Library(oneVPL) GPU implementation, which can be found at https://github.com/oneapi-src/oneVPL-intel-gpu 
| Software         | oneAPI Video Processing Library (oneVPL), Intel® OpenVINO
| What You Will Learn | How to use oneVPL to decode an H.265 encoded video file and resize and perform per-frame object detection inference
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


## Building the `vpl-infer` Program

The first step is to set up a build environment with prerequisites installed.  
This can be set up in a bare metal Ubuntu 20.04 system or with Docker. 

### On a bare metal Linux* System

1. Install the prerequisite software:

   - Intel® oneAPI Base Toolkit for Linux*
   - Intel® OpenVINO for Linux*
   - [Python](http://python.org)
   - [CMake](https://cmake.org)
   - OpenCL headers: 'sudo apt install -y opencl-headers' 


2. Install Intel general purpose GPU (GPGPU) software packages:

   Follow steps in 
   https://dgpu-docs.intel.com/installation-guides/index.html#


3. Download the Mobilenet-ssd object detection model from OpenVINO model Zoo and covert it to IR model

* Download OpenVINO development tools
```
apt install python3 python3.8-venv
python3 -m venv openvino_env
source openvino_env/bin/activate
python3 -m pip install --upgrade pip
pip install openvino-dev[caffe]==2022.2.0
```
* Download mobilenet-ssd from virtual environment
```
(openvino_env) .. $ omz_downloader --name mobilenet-ssd
```
mobilenet-ssd caffe model will be downloaded at `./public/mobilenet-ssd`
```
(openvino_env) .. $ ls ./public/mobilenet-ssd
mobilenet-ssd.caffemodel  mobilenet-ssd.prototxt
```
* Convert caffemodel to OpenVINO IR model
```
(openvino_env) .. $ omz_converter --name mobilenet-ssd --precision FP32 --download_dir . --output_dir .
```
`./public` is the default input/output directory\
mobilenet-ssd IR model will be generated at `./public/mobilenet-ssd/FP32` 
```
(openvino_env) .. $ ls ./public/mobilenet-ssd/FP32
mobilenet-ssd.bin  mobilenet-ssd.xml  mobilenet-ssd.mapping
```
(openvino_env) .. $ deactivate


4. Set up your environment using the following commands.

```
source /opt/intel/oneapi/setvars.sh
source /opt/intel/openvino_2022/setupvars.sh
```

Note: /opt/intel is the default location.  If you installed oneAPI and/or OpenVINO
to custom locations use them instead. 
 

5. Build the program using the following commands:

```
mkdir build && cd build
cmake .. && cmake --build . --config release
```


6. Run the program with defaults using the following command:

```
./vpl-infer -i cars_320x240.h265 -m mobilenet-ssd.xml -hw
```


### Using a docker container

Check groups of current user
Let's say your username is `user1`
```
groups
user1 adm sudo render docker
```
User1 should be in `render` group.
If you don't see `render` from `groups` command, then you should add user1 to `render` group.
```
usermod -a -G render user1
newgrp render
```

Go to vpl-infer/docker and build docker image

```
docker build -t onevpl_openvino .
```
If there’re proxy servers, then you might need to pass that information with using “—build-arg”,
```
docker build -t onevpl_openvino $(env | grep -E '(_proxy=|_PROXY)' | sed 's/^/--build-arg /') .
```
Start the container, mounting the examples directory\
Read render group id (it might be different by system configuration)
```
stat -c "%g" /dev/dri/render*
109
```
```
docker run -u 0 -it -v `pwd`/../../../../examples:/oneVPL/examples --device /dev/dri --group-add 109 onevpl_openvino

```
This `"groups: cannot find name for group ID 109"` message is expected.

In the container

```
cd /oneVPL/examples/interop/vpl-infer
mkdir build && cd build
cmake .. && cmake --build . --config release
./vpl-infer -hw -i /oneVPL/examples/content/cars_320x240.h265 -m /oneVPL/nm/mobilenet-ssd/FP32/mobilenet-ssd.xml
```

### Example of Output

```
OpenVINO Runtime
    Version : 2022.2.0
    Build   : 2022.2.0-7713-af16ea1d79a-releases/2022/2

Loading network model files: /oneVPL/nm/mobilenet-ssd/FP32/mobilenet-ssd.xml
    Model name: MobileNet-SSD
    Inputs
        Input name: data
        Input type: f32
        Input shape: {1, 3, 300, 300}
    Outputs
        Output name: detection_out
        Output type: f32
        Output shape: {1, 1, 100, 7}

libva info: VA-API version 1.15.0
libva info: Trying to open /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
libva info: Found init function __vaDriverInit_1_15
libva info: va_openDriver() returns 0
libva info: VA-API version 1.15.0
libva info: Trying to open /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
libva info: Found init function __vaDriverInit_1_15
libva info: va_openDriver() returns 0
libva info: VA-API version 1.15.0
libva info: Trying to open /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
libva info: Found init function __vaDriverInit_1_15
libva info: va_openDriver() returns 0
libva info: VA-API version 1.15.0
libva info: Trying to open /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
libva info: Found init function __vaDriverInit_1_15
libva info: va_openDriver() returns 0

OneVPL Implementation details:
    ApiVersion:           2.7
    Implementation type:  HW
    AccelerationMode via: VAAPI
    Path: /usr/lib/x86_64-linux-gnu/libmfx-gen.so.1.2.7

libva info: VA-API version 1.15.0
libva info: Trying to open /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
libva info: Found init function __vaDriverInit_1_15
libva info: va_openDriver() returns 0
Decoding VPP, and inferring /oneVPL/examples/content/cars_320x240.h265 with /oneVPL/nm/mobilenet-ssd/FP32/mobilenet-ssd.xml
Result:
  bbox 92, 112, 201, 217, confidence = 0.998823
  bbox 207, 50, 296, 144, confidence = 0.997168
  bbox 35, 43, 120, 134, confidence = 0.994219
  bbox 73, 82, 168, 171, confidence = 0.936172
  bbox 168, 199, 274, 238, confidence = 0.577672

  ...

Result:
  bbox 64, 68, 161, 178, confidence = 0.997325
  bbox 116, 133, 224, 238, confidence = 0.934097
  bbox 266, 81, 319, 190, confidence = 0.838117
  bbox 17, 44, 71, 93, confidence = 0.753288

Decoded 30 frames and detected objects

```
