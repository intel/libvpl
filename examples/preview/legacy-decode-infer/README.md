# `legacy-decode-infer` Sample

This sample shows how to use the oneAPI Video Processing Library (oneVPL) to
perform a simple video decode using 1.x common APIs, and inference using OpenVINO

| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04; Windows* 10
| Hardware         | Intel® Processor Graphics GEN9 or newer
| Software         | Intel® oneAPI Video Processing Library(oneVPL) and Intel® OpenVINO
| What You Will Learn | How to use oneVPL to decode an H.265 encoded video file and perform objection classification inference
| Time to Complete | 5 minutes


## Purpose

This sample is a command line application that takes a file containing an H.265
video elementary stream as an argument, decodes it with oneVPL and perform 
object classification on each frame using OpenVINO.


## Key Implementation details

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | CPU
| Input format      | H.265 video elementary stream


## License

This code sample is licensed under MIT license.


## Building the `legacy-decode-infer` Program

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

4. Download the Alexnet classification model from OpenVINO model zoo
   ```
   pip3 install -r <openvino_install_dir>/deployment_tools/model_optimizer/requirements.txt
   pip3 install -r <openvino_install_dir>/deployment_tools/open_model_zoo/tools/downloader/requirements.in
   <openvino_install_dir>/deployment_tools/open_model_zoo/tools/downloader/downloader.py --output_dir ../../content --precisions FP32 --name alexnet
   <openvino_install_dir>/deployment_tools/open_model_zoo/tools/downloader/converter.py --download_dir ../../content --name alexnet
   ```

5. Run the program using the following command:
   ```
   cmake --build . --target run
   ```


### On a Windows* System Using Visual Studio* Version 2017 or Newer

#### Building the program using CMake

1. Install the prerequisite software. To build and run the sample you need to
   install prerequisite software and set up your environment:

   - Intel® oneAPI Base Toolkit for Windows*
   - Intel® OpenVINO 2021.2 for Windows*
   - [Python 3.8 or earlier](http://python.org)
   - [CMake](https://cmake.org)

2. Set up your environment using the following command.
   ```
   <oneapi_install_dir>\setvars.bat
   <openvino_install_dir>\bin\setupvars.bat
   ```
   Here `<oneapi_install_dir>` represents the root folder of your oneAPI
   installation, which is which is `C:\Program Files (x86)\Intel\oneAPI\`
   when installed using default options. `<openvino_install_dir>` represents 
   the root folder of your OpenVINO installation, which is 
   `C:\Program Files (x86)\Intel\openvino_2021\` when installed using default options. 
   If you customized the installation folders, the `setvars.bat` and `setupvars.bat` 
   are in your custom locations.  Note that if a compiler is not part of your 
   oneAPI installation you should run in a Visual Studio 64-bit command prompt.

3. Build the program using the following commands:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

4. Download the Alexnet classification model from OpenVINO model zoo
   ```
   <openvino_install_dir>\deployment_tools\model_optimizer\install_prerequisites\install_prerequisites.bat
   pip install -r <openvino_install_dir>\deployment_tools\open_model_zoo\tools\downloader\requirements.in
   <openvino_install_dir>\deployment_tools\open_model_zoo\tools\downloader\downloader.py --output_dir ..\..\content --precisions FP32 --name alexnet
   <openvino_install_dir>\deployment_tools\open_model_zoo\tools\downloader\converter.py --download_dir ..\..\content --name alexnet
   ```

5. Run the program using the following command:
   ```
   cmake --build . --target run
   ```


## Running the Sample

### Application Parameters

The instructions given above run the sample executable with the argument
`examples/content/cars_128x96.h265` and `examples/content/public/alexnet/FP32/alexnet.xml`.


### Example of Output

```
Implementation info
      version = 2.2
      impl = Software
Decoding and inferring legacy-decode-infer/../content/cars_128x96.h265 with legacy-decode-infer/../content/public/alexnet/FP32/alexnet.xml
Top 5 results for video frame:
classid probability
------- -----------
817 0.429395
511 0.159974
751 0.044094
627 0.042601
468 0.039582
Decoded 60 frames
```
