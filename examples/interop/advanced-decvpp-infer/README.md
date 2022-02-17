# `Advanced-decvpp-infer` Sample

This sample shows how to use the oneAPI Video Processing Library (oneVPL) to
perform a simple video decode and resize and inference using OpenVINO to show the device surface sharing (zero copy)

| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04
| Hardware         | Compatible with Intel® oneAPI Video Processing Library(oneVPL) GPU implementation, which can be found at https://github.com/oneapi-src/oneVPL-intel-gpu 
| Software         | Intel® oneAPI Video Processing Library(oneVPL) CPU implementation and Intel® OpenVINO
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


## Building the `advanced-decvpp-infer` Program

The first step is to set up a build environment with prerequisites installed.  
This can be set up in a bare metal Ubuntu 20.04 system or with Docker. 

### On a bare metal Linux* System

Install the prerequisite software:

   - Intel® oneAPI Base Toolkit for Linux*
   - Intel® OpenVINO for Linux*
   - [Python](http://python.org)
   - [CMake](https://cmake.org)
   - OpenCL headers: 'sudo apt install -y opencl-headers' 

### In a docker container

```
docker build -t openvino_vpl_environment docker
```

Start the container, mounting the examples directory
```
cd ../../..
docker run -it --rm --privileged -v `pwd`/examples:`pwd`/examples -w `pwd`/examples  openvino_vpl_environment
```
In the container, cd back to interop/advanced-decvpp-infer.  


### Common steps
Continue with the rest of these steps in your bare metal shell or in the container shell.

1. Set up your environment using the following commands.

```
source /opt/intel/oneapi/setvars.sh
source /opt/intel/openvino_2021/bin/setupvars.sh
```

Note: /opt/intel is the default location.  If you installed oneAPI and/or OpenVINO
to custom locations use them instead. 
 

2. Build the program using the following commands:

```
mkdir build
cd build
cmake ..
cmake --build .
```

3. Download the Mobilenet-ssd object detection model from OpenVINO model zoo

```
pip3 install -r /opt/intel/openvino_2021/deployment_tools/model_optimizer/requirements.txt
pip3 install -r /opt/intel/openvino_2021/deployment_tools/open_model_zoo/tools/downloader/requirements.in
/opt/intel/openvino_2021/deployment_tools/open_model_zoo/tools/downloader/downloader.py --output_dir ../../content --precisions FP32 --name mobilenet-ssd
/opt/intel/openvino_2021/deployment_tools/open_model_zoo/tools/downloader/converter.py --download_dir ../../content --name mobilenet-ssd
```

4. Run the program with defaults using the following command:

```
cmake --build . --target run
```




## Running the Sample

### Application Parameters

The instructions given above run the sample executable with the argument
`-i ../../../content/cars_128x96.h265 -m ../../../content/public/mobilenet-ssd/FP32/mobilenet-ssd.xml`.


### Example of Output

```
Implementation details:
  ApiVersion:           2.3  
  Implementation type:  HW
  AccelerationMode via: VAAPI
  Path: /usr/lib/x86_64-linux-gnu/libmfx-gen.so.1.2.3

libva info: VA-API version 1.12.0
libva info: Trying to open /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
libva info: Found init function __vaDriverInit_1_12
libva info: va_openDriver() returns 0
Decoding VPP, and infering /home/jeff/innersource_oneVPL/frameworks.media.onevpl.dispatcher/examples/interop/advanced-decvpp-infer/../../content/cars_128x96.h265 with /home/jeff/innersource_oneVPL/frameworks.media.onevpl.dispatcher/examples/interop/advanced-decvpp-infer/../../content/public/mobilenet-ssd/FP32/mobilenet-ssd.xml
[0,7] element, prob = 0.998121    (34,43)-(80,88) batch id : 0 WILL BE PRINTED!
[1,7] element, prob = 0.996414    (82,18)-(118,58) batch id : 0 WILL BE PRINTED!
[2,7] element, prob = 0.970787    (14,17)-(46,53) batch id : 0 WILL BE PRINTED!
[3,7] element, prob = 0.682925    (32,29)-(62,53) batch id : 0 WILL BE PRINTED!
...
Decoded 60 frames
```
