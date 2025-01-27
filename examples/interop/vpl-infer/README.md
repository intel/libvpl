# `vpl-infer` Sample

## Intro

This sample shows how to use the Intel® Video Processing Library (Intel® VPL)
and Intel® Distribution of OpenVINO™ Toolkit together to perform a simple
inference pipeline

```mermaid
graph LR;
    decode-->resize-->infer;
```


| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 22.04/24.04; Windows* 10/11
| Hardware runtime | Intel® VPL GPU implementation, which can be found at https://github.com/intel/vpl-gpu-rt
| What You Will Learn | How to combine Intel® VPL and Intel® Distribution of OpenVINO™ Toolkit
| Time to Complete | 15 minutes


## Purpose

This sample is a command line application that takes a file containing an H.265
video elementary stream and network model as an argument, decodes and resize it
with Intel® VPL and performs object detection on each frame using the OpenVINO™
toolkit.


## Key Implementation Details

| Configuration          | Default setting
| ---------------------- | ----------------------------------
| Target device          | GPU
| Input video format     | H.265 video elementary stream
| Input IR network model | Object detection
| Output                 | Class ID, Bounding Box Location, and Confidence Score


## Command Line Options

| Option    | Description                       | Note
| --------- | --------------------------------- | -----------------
| -i        | H.265 video elementary stream     |
| -m        | Object detection network model    |
| -legacy   | Run using 1.x API for portability |
| -zerocopy | Process data without copying between Intel® VPL and the OpenVINO™ toolkit in hardware implementation mode | Not compatible with legacy API


## License

This code sample is licensed under MIT license.


## Building and Executing the `vpl-infer` Program

The first step is to set up a build environment with prerequisites installed.
This can be set up on a Ubuntu or Windows system.


### On a Linux* System

1. Install media and compute stack prerequisites.

   - Follow the steps in [dgpu-docs](https://dgpu-docs.intel.com/) according to your GPU.
   - Follow the steps in
     [install.md](https://github.com/intel/libvpl/blob/master/INSTALL.md) or
     install libvpl-dev.

2. Install Intel® Distribution of OpenVINO™ Toolkit 2025.0.0:

    ```
    wget https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB
    sudo apt-key add GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB
    echo "deb https://apt.repos.intel.com/openvino/2025 ubuntu24 main" | sudo tee /etc/apt/sources.list.d/intel-openvino-2025.list
    sudo apt-get update
    sudo apt-get install openvino-2025.0.0
    ```

3. Install additional prerequisites for building examples:

    ```
    sudo apt-get install cmake build-essential pkg-config libva-dev ocl-icd-opencl-dev
    ```

3. Download the vehicle-detection-0200 model from the Open Model Zoo:

    ```
    curl -LJO "https://storage.openvinotoolkit.org/repositories/open_model_zoo/2023.0/models_bin/1/vehicle-detection-0200/FP16/vehicle-detection-0200.xml"
    curl -LJO "https://storage.openvinotoolkit.org/repositories/open_model_zoo/2023.0/models_bin/1/vehicle-detection-0200/FP16/vehicle-detection-0200.bin"
    ```

4. Build and run the program:

    Install prerequisites and build

    ```
    mkdir build && cd build
    cmake .. && cmake --build . --config release
    ```

    Provide path to video file and IR model in command line parameters

    To run with 2.x API zero copy on GPU

    ```
    ./vpl-infer -i cars_320x240.h265 -m vehicle-detection-0200.xml -zerocopy
    ```

    To run with 1.x API (and extra copy) on GPU

    ```
    ./vpl-infer -i cars_320x240.h265 -m vehicle-detection-0200.xml -legacy
    ```


### On a Windows* System


1. Install prerequisites. To build and run the sample you need to install
   prerequisite software and set up your environment:

   - Follow the steps in
     [install.md](https://github.com/intel/libvpl/blob/master/INSTALL.md) to
     install Intel® VPL package.
   - Visual Studio 2022
   - [CMake](https://cmake.org)

2. Install Intel® Distribution of OpenVINO™ Toolkit 2025.0.0 from archive

    ```
    pushd %USERPROFILE%\Downloads

    curl -L -o OpenVINO.zip https://storage.openvinotoolkit.org/repositories/openvino/packages/2025.0/windows/openvino_toolkit_windows_2025.0.0.17942.1f68be9f594_x86_64.zip
    tar -xvf OpenVINO.zip
    ren openvino_toolkit_windows_2025.0.0.17942.1f68be9f594_x86_64 OpenVINO
    move OpenVINO C:\
    popd
    ```

3. Download the vehicle-detection-0200 model from the Open Model Zoo:

    ```
    curl -LJO "https://storage.openvinotoolkit.org/repositories/open_model_zoo/2023.0/models_bin/1/vehicle-detection-0200/FP16/vehicle-detection-0200.xml"
    curl -LJO "https://storage.openvinotoolkit.org/repositories/open_model_zoo/2023.0/models_bin/1/vehicle-detection-0200/FP16/vehicle-detection-0200.bin"
    ```

4. Set up OpenVINO™ toolkit environment:

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
    .\vpl-infer -i cars_320x240.h265 -m vehicle-detection-0200.xml -zerocopy
    ```

    To run with 1.x API (and extra copy) on GPU

    ```
    .\vpl-infer -i cars_320x240.h265 -m vehicle-detection-0200.xml -legacy
    ```

> [!NOTE]
>
> Building this example in debug mode is not currently supported. The model will
> fail to load if built in debug mode.


### Example of Output

This is the output from Linux, but the test result will be similar to Windows:

```

Loading network model files: vehicle-detection-0200.xml
    Model name: torch-jit-export
    Inputs
        Input name: image
        Input type: f32
        Input shape: [1,3,256,256]
    Outputs
        Output name: detection_out
        Output type: f32
        Output shape: [1,1,200,7]

libva info: VA-API version 1.22.0
libva info: Trying to open /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
libva info: Found init function __vaDriverInit_1_22
libva info: va_openDriver() returns 0
libva info: VA-API version 1.22.0
libva info: Trying to open /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
libva info: Found init function __vaDriverInit_1_22
libva info: va_openDriver() returns 0
libva info: VA-API version 1.22.0
libva info: Trying to open /usr/lib/x86_64-linux-gnu/dri/iHD_drv_video.so
libva info: Found init function __vaDriverInit_1_22
libva info: va_openDriver() returns 0

Intel® VPL Implementation details:
    ApiVersion:           2.13
    AccelerationMode via: VAAPI
  DeviceID:             4680/0
    Path: /usr/lib/x86_64-linux-gnu/libmfx-gen.so.1.2.13

Decoding VPP, and inferring cars_320x240.h265 with vehicle-detection-0200.xml
Result:
    Class ID (0),  BBox ( 205,   49,  296,  144),  Confidence (0.998)
    Class ID (0),  BBox (  91,  115,  198,  221),  Confidence (0.996)
    Class ID (0),  BBox (  36,   44,  111,  134),  Confidence (0.984)
    Class ID (0),  BBox (  78,   72,  155,  164),  Confidence (0.975)

Result:
    Class ID (0),  BBox ( 207,   50,  299,  146),  Confidence (0.998)
    Class ID (0),  BBox (  93,  115,  200,  222),  Confidence (0.994)
    Class ID (0),  BBox (  84,   92,  178,  191),  Confidence (0.993)
    Class ID (0),  BBox (  37,   45,  113,  132),  Confidence (0.982)
    Class ID (0),  BBox (  75,   71,  154,  164),  Confidence (0.967)

...

Result:
    Class ID (0),  BBox ( 109,  126,  217,  243),  Confidence (0.998)
    Class ID (0),  BBox (  67,   69,  161,  177),  Confidence (0.997)
    Class ID (0),  BBox (  24,   53,   69,   97),  Confidence (0.949)
    Class ID (0),  BBox ( 265,   84,  319,  192),  Confidence (0.745)
    Class ID (0),  BBox ( 156,  179,  270,  242),  Confidence (0.671)

Decoded 30 frames and detected objects
```

This execution is the object detection use case with `vehicle-detection-0200`
network model.
