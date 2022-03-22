# `legacy-decode-infer` Sample

This sample shows how to use the oneAPI Video Processing Library (oneVPL) to
perform a simple video decode using 1.x common APIs, and inference using OpenVINO

| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04; Windows* 10
| Hardware         | Compatible with Intel® oneAPI Video Processing Library(oneVPL) GPU implementation, which can be found at https://github.com/oneapi-src/oneVPL-intel-gpu 
|                  | and Intel® Media SDK GPU implementation, which can be found at https://github.com/Intel-Media-SDK/MediaSDK
| Software         | Intel® oneAPI Video Processing Library(oneVPL) CPU implementation
| What You Will Learn | How to use oneVPL to decode an H.265 encoded video file and perform image classification inference
| Time to Complete | 5 minutes


## Purpose

This sample is a command line application that takes a file containing an H.265
video elementary stream and network model as an argument, decodes it with oneVPL and perform 
image classification on each frame using OpenVINO.


## Key Implementation details

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | CPU
| Input format      | H.265 video elementary stream
| Input network model | image classification network model

## License

This code sample is licensed under MIT license.


## Building the `legacy-decode-infer` Program

The first step is to set up a build environment with prerequisites installed.  
This can be set up in a bare metal Ubuntu 20.04 system or with Docker. 

### On a Linux system

#### On a bare metal Linux* System

Install the prerequisite software:

   - Intel® oneAPI Base Toolkit for Linux*
   - Intel® OpenVINO 2021.2 for Linux*
   - [Python](http://python.org)
   - [CMake](https://cmake.org)

#### In a docker container

   ```
docker build -t openvino_vpl_environment docker
   ```

Start the container, mounting the examples directory
```
cd ../../..
docker run -it --rm --privileged -v `pwd`/examples:`pwd`/examples -w `pwd`/examples  openvino_vpl_environment
```
In the container, cd back to interop/hello-decode-infer.  

#### Common steps
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

3. Download the Alexnet classification model from OpenVINO model zoo
   ```
pip3 install -r /opt/intel/openvino_2021/deployment_tools/model_optimizer/requirements.txt
pip3 install -r /opt/intel/openvino_2021/deployment_tools/open_model_zoo/tools/downloader/requirements.in
/opt/intel/openvino_2021/deployment_tools/open_model_zoo/tools/downloader/downloader.py --output_dir ../../content --precisions FP32 --name alexnet
/opt/intel/openvino_2021/deployment_tools/open_model_zoo/tools/downloader/converter.py --download_dir ../../content --name alexnet
   ```

4. Run the program with defaults using the following command:

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
   - Intel® OpenVINO 2021.2 for Windows*
   - [Python 3.8 or earlier](http://python.org)
   - [CMake](https://cmake.org)

3. Set up your environment using the following command.
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

4. Build the program using the following commands:
   ```
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

5. Download the Alexnet classification model from OpenVINO model zoo
   ```
   <openvino_install_dir>\deployment_tools\model_optimizer\install_prerequisites\install_prerequisites.bat
   pip install -r <openvino_install_dir>\deployment_tools\open_model_zoo\tools\downloader\requirements.in
   <openvino_install_dir>\deployment_tools\open_model_zoo\tools\downloader\downloader.py --output_dir ..\..\content --precisions FP32 --name alexnet
   <openvino_install_dir>\deployment_tools\open_model_zoo\tools\downloader\converter.py --download_dir ..\..\content --name alexnet
   ```

6. Run the program using the following command:
   ```
   cmake --build . --target run
   ```


## Running the Sample

### Application Parameters

The instructions given above run the sample executable with the argument
`-sw -i ../../../content/cars_320x240.h265 -m ../../..//content/public/alexnet/FP32/alexnet.xml`.


### Example of Output

```
Implementation info
      version = 2.2
      impl = Software
Decoding and inferring legacy-decode-infer/../content/cars_320x240.h265 with legacy-decode-infer/../content/public/alexnet/FP32/alexnet.xml
Top 5 results for video frame:
classid probability
------- -----------
817 0.429395
511 0.159974
751 0.044094
627 0.042601
468 0.039582
Decoded 30 frames
```
