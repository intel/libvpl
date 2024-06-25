# `vpl-infer` Sample

## Intro

This sample shows how to use the Intel® Video Processing Library (Intel® VPL)
and Intel® Distribution of OpenVINO™ Toolkit together to perform a simple inference pipeline

```mermaid
graph LR;
    decode-->resize-->infer;
```


| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04/22.04; Windows* 10/11
| Hardware runtime | Compatible with Intel® VPL GPU implementation, which can be found at https://github.com/intel/vpl-gpu-rt
|                  | and Intel® Media SDK GPU implementation, which can be found at [Intel® Media SDK Open Source](https://github.com/Intel-Media-SDK/MediaSDK)
| What You Will Learn | How to combine Intel® VPL and Intel® Distribution of OpenVINO™ Toolkit
| Time to Complete | 15 minutes


## Purpose

This sample is a command line application that takes a file containing an H.265
video elementary stream and network model as an argument, decodes and resize it with Intel® VPL and performs
object detection on each frame using the OpenVINO™ toolkit.


## Key Implementation Details

| Configuration          | Default setting
| ---------------------- | ----------------------------------
| Target device          | GPU
| Input video format     | H.265 video elementary stream
| Input IR network model | Object detection
| Output                 | Class ID, Bounding Box Location, and Confidence Score


## Command Line Options

| Option   | Description | Note
| -------  | -------------------------------| -----------------
| -i       | H.265 video elementary stream  |
| -m       | Object detection network model |
| -legacy  | Run sample using core 1.x API for portability |
| -zerocopy| Process data without copying between Intel® VPL and the OpenVINO™ toolkit in hardware implemenation mode | Not compatible with legacy API


## License

This code sample is licensed under MIT license.


## Building and Executing the `vpl-infer` Program

The first step is to set up a build environment with prerequisites installed.
This can be set up in a bare metal Ubuntu 22.04 system or with Docker for Linux, and in Windows.


### On a Linux* System

1. Install media and compute stack prerequisites.

   - Follow the steps in [dgpu-docs](https://dgpu-docs.intel.com/) according to your GPU.
   - Follow the steps in [install.md](https://github.com/intel/libvpl/blob/master/INSTALL.md) or install libvpl-dev.
   - Install these additional packages:

    ```
    apt-get update
    apt-get install -y cmake build-essential pkg-config libva-dev libva-drm2 vainfo
    ```

2. Install Intel® Distribution of OpenVINO™ Toolkit 2023.3.0 from archive

    ```
    curl -L https://storage.openvinotoolkit.org/repositories/openvino/packages/2023.3/linux/l_openvino_toolkit_ubuntu22_2023.3.0.13775.ceeafaf64f3_x86_64.tgz --output l_openvino_toolkit.tgz
    tar -xf l_openvino_toolkit.tgz
    mkdir -p /opt/intel
    mv l_openvino_toolkit_ubuntu22_2023.3.0.13775.ceeafaf64f3_x86_64 /opt/intel/openvino_2023.3.0
    ln -s /opt/intel/openvino_2023.3.0 /opt/intel/openvino
    ```


3. Download the Mobilenet-ssd object detection model from the Open Model Zoo for
   OpenVINO™ toolkit and covert it to an IR model:

    Start Python virtual environments from Command Prompt

    ```
    apt-get install python3 python3.10-venv
    python3 -m venv openvino_env
    source openvino_env/bin/activate
    ```

    From the Python virtual environment `(openvino_env) .. $`

    ```
    python -m pip install --upgrade pip
    pip install openvino-dev[caffe]==2023.3.0
    omz_downloader --name mobilenet-ssd
    omz_converter --name mobilenet-ssd --precision FP32 --download_dir . --output_dir .
    deactivate
    ```


4. Set up OpenVINO™ toolkit environment:

    ```
    source /opt/intel/openvino/setupvars.sh
    ```


5. Build and run the program:

    Install prerequisites and build

    ```
    apt-get install -y opencl-headers libpugixml-dev libtbb-dev libtbb2

    mkdir build && cd build
    cmake .. && cmake --build . --config release
    ```

    Provide path to video file and IR model in command line parameters

    To run with 2.x API zero copy on GPU

    ```
    ./vpl-infer -i cars_320x240.h265 -m mobilenet-ssd.xml -zerocopy
    ```

    To run with 1.x API (and extra copy) on GPU

    ```
    ./vpl-infer -i cars_320x240.h265 -m mobilenet-ssd.xml -legacy
    ```


### Using a Docker Container (Linux)

These instructions assume that Docker is already set up on your system.



1. Build docker image:

    These steps start from vpl-infer sample directory:
    ```
    examples/interop/vpl-infer
    ```
    Build Dockerfile

    ```
    docker build -t interop docker
    ```

    If needed, pass proxy information with “--build-arg”:

    ```
    docker build -t interop $(env | grep -E '(_proxy=|_PROXY)' | sed 's/^/--build-arg /') docker
    ```


2. Start the container

    Mount `examples` directory as `/work` directory, and current directory is `vpl-infer` example directory.
    ```
    docker run -it --rm --privileged -v `pwd`/../../:/work -w /work/interop/vpl-infer interop:latest
    ```


3. Build and run the program in the container:


    Build steps:

    ```
    source /opt/intel/openvino/setupvars.sh

    mkdir build && cd build
    cmake .. && cmake --build . --config release
    ```

    To run with 2.x API zero copy on GPU

    ```
    ./vpl-infer -i /work/content/cars_320x240.h265 -m /OpenVINO/public/mobilenet-ssd/FP32/mobilenet-ssd.xml -zerocopy
    ```

    To run with 1.x API (and extra copy) on GPU

    ```
    ./vpl-infer -i /work/content/cars_320x240.h265 -m /OpenVINO/public/mobilenet-ssd/FP32/mobilenet-ssd.xml -legacy
    ```

### On a Windows* System


1. Install prerequisites. To build and run the sample you need to
   install prerequisite software and set up your environment:

   - Follow the steps in
     [install.md](https://github.com/intel/libvpl/blob/master/INSTALL.md) to
     install Intel® VPL package.
   - Visual Studio 2022
   - [CMake](https://cmake.org)
   - [Python](http://python.org) (v3.7-v3.10)

2. Install Intel® Distribution of OpenVINO™ Toolkit 2024.2.0 from archive

    ```
    pushd %USERPROFILE%\Downloads
    curl -L -o OpenVINO.zip https://storage.openvinotoolkit.org/repositories/openvino/packages/2024.2/windows/w_openvino_toolkit_windows_2024.2.0.15519.5c0f38f83f6_x86_64.zip
    tar -xvf OpenVINO.zip
    ren w_openvino_toolkit_windows_2024.2.0.15519.5c0f38f83f6_x86_64 OpenVINO
    move OpenVINO C:\
    popd
    ```

2. Download the Mobilenet-ssd object detection model from the Open Model Zoo for
   OpenVINO™ toolkit and covert it to an IR model:

    Start Python virtual environments from Command Prompt

    ```
    python -m venv openvino_env
    openvino_env\Scripts\activate
    ```

    From the Python virtual environment `(openvino_env) .. >`

    ```
    python -m pip install --upgrade pip
    pip install openvino-dev[caffe]==2023.3.0
    omz_downloader --name mobilenet-ssd
    omz_converter --name mobilenet-ssd --precision FP32 --download_dir . --output_dir .
    deactivate
    ```
    mobilenet-ssd IR model will be generated in `.\public\mobilenet-ssd\FP32`

    ```
    mobilenet-ssd.bin  mobilenet-ssd.xml  mobilenet-ssd.mapping
    ```


3. Set up OpenVINO™ toolkit environment:

    ```
    "C:\OpenVINO\setupvars.bat"
    ```

5. Build and run the program:

    Go to `examples\interop\vpl-infer`
    (Make sure that your shell is configured with vars.bat)

    ```
    mkdir build && cd build
    cmake .. && cmake --build . --config release && cd release
    ```

    To run with 2.x API zero copy on GPU

    ```
    .\vpl-infer -i cars_320x240.h265 -m mobilenet-ssd.xml -zerocopy
    ```

    To run with 1.x API (and extra copy) on GPU

    ```
    .\vpl-infer -i cars_320x240.h265 -m mobilenet-ssd.xml -legacy
    ```

> [!NOTE]
>
> Building this example in debug mode is not currently supported. The model will
> fail to load if built in debug mode.


### Example of Output

This is the output from Linux, but the test result will be similar to Windows:

```
Version : 2023.2.0
Build : 2023.2.0-13089-cfd42bd2cb0-HEAD
Loading network model files: /OpenVINO/public/mobilenet-ssd/FP32/mobilenet-ssd.xml
Model name: MobileNet-SSD
Inputs
Input name: data
Input type: f32
Input shape: [1,3,300,300]
Outputs
Output name: detection_out
Output type: f32
Output shape: [1,1,100,7]

libva info: VA-API version 1.18.0
libva info: Trying to open /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
libva info: Found init function __vaDriverInit_1_18
libva info: va_openDriver() returns 0
libva info: VA-API version 1.18.0
libva info: Trying to open /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
libva info: Found init function __vaDriverInit_1_18
libva info: va_openDriver() returns 0

Intel® VPL Implementation details:
ApiVersion: 2.9
AccelerationMode via: VAAPI
DeviceID: 4680/0
Path: /usr/lib/x86_64-linux-gnu/libmfx-gen.so.1.2.9

libva info: VA-API version 1.18.0
libva info: Trying to open /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
libva info: Found init function __vaDriverInit_1_18
libva info: va_openDriver() returns 0
Decoding VPP, and inferring cars_320x240.h265 with /OpenVINO/public/mobilenet-ssd/FP32/mobilenet-ssd.xml
Result:
Class ID (7), BBox ( 92, 112, 201, 217), Confidence (0.999)
Class ID (7), BBox ( 207, 50, 296, 144), Confidence (0.997)
Class ID (7), BBox ( 35, 43, 120, 134), Confidence (0.994)
Class ID (7), BBox ( 73, 82, 168, 171), Confidence (0.936)
Class ID (7), BBox ( 168, 199, 274, 238), Confidence (0.563)

....

Result:
Class ID (7), BBox ( 64, 70, 160, 181), Confidence (0.997)
Class ID (7), BBox ( 116, 133, 224, 238), Confidence (0.937)
Class ID (7), BBox ( 266, 81, 319, 190), Confidence (0.837)
Class ID (7), BBox ( 17, 44, 71, 93), Confidence (0.760)

Decoded 30 frames and detected objects

```
This execution is the object detection use case with `mobilenet-ssd` network model.

`Class ID` is predicted class ID (1..20 - PASCAL VOC defined class ids).

Mapping to class names provided by `<omz_dir>/data/dataset_classes/voc_20cl_bkgr.txt` file, which is downloaded when you install the development version of the OpenVINO™ toolkit.

`7` is `car` from the list.
