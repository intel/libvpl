# `hello-decode` Sample

This sample shows how to use the oneAPI Video Processing Library (oneVPL) to
perform a simple video decode using preview C++ APIs.

| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04
| Hardware         | Compatible with Intel® oneAPI Video Processing Library(oneVPL) GPU implementation, which can be found at https://github.com/oneapi-src/oneVPL-intel-gpu 
| Software         | Intel® oneAPI Video Processing Library(oneVPL) CPU implementation
| What You Will Learn | How to use oneVPL to decode an H.265 encoded video file
| Time to Complete | 5 minutes


## Purpose

This sample is a command line application that takes a file containing an H.265
video elementary stream as an argument. Using oneVPL, the application decodes 
and writes the decoded output to a file named `out.raw` in raw format.

## Key Implementation details

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | CPU
| Input format      | H.265 video elementary stream
| Output format     | I420
| Output resolution | same as the input

Native raw frame output format: CPU=I420, GPU=NV12.

## License

Code samples are licensed under the MIT license. See
[License.txt](https://github.com/oneapi-src/oneAPI-samples/blob/master/License.txt) for details.

Third-party program licenses can be found here: [third-party-programs.txt](https://github.com/oneapi-src/oneAPI-samples/blob/master/third-party-programs.txt)


## Building the `hello-decode-cpp` Program

Perform the following steps:

1. Install the prerequisite software. To build and run the sample, you need to
   install prerequisite software and set up your environment:

   - Intel® oneAPI Base Toolkit* 
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

4. Run the program using the following command:
   ```
    ./hello-decode-cpp -i ../../../content/cars_320x240.h265
   ```


## Running the Sample

### Application Parameters

The instructions given above run the sample executable with the argument
`-i <sample_dir>/content/cars_320x240.h265`.


### Example of Output

```
./hello-decode-cpp -i ../../../content/cars_320x240.h265

Decoding ../../../content/cars_320x240.h265 -> out.raw
Got not enough data.
All input data is processed.
Decoded 30 frames

-- Decode information --

Base:
    AllocId    = 0
    AsyncDepth = Not Specifyed
    Protected  = 0
    IOPattern  = Out Syatem Memory
Codec:
    LowPower           = Unset
    BRCParamMultiplier = 0
    CodecId            = HEVC
    CodecProfile       = 1
    CodecLevel         = 120
    NumThread          = 0
FrameInfo:
    BitDepthLuma   = 8
    BitDepthChroma = 8
    Shift          = Not Specifyed
    Color Format   = IYUV
    Size [W,H]     = [320,240]
    ROI [X,Y,W,H]  = [0,0,320,240]
    FrameRate [N:D]= 30:1
    AspecRato [W,H]= [1,1]
    PicStruct      = Unset
    ChromaFormat   = 4:2:0

Decoder:
    DecodedOrder         = False
    ExtendedPicStruct    = Unset
    TimeStampCalc        = Not specifyed
    SliceGroupsPresent   = False
    MaxDecFrameBuffering = Not Specifyed
    EnableReallocRequest = Unset

```

You can find the output file `out.raw` in the build directory.

You can display the output with a video player that supports raw streams such as
FFplay. You can use the following command to display the output with FFplay:

```
ffplay -video_size 320x240 -pixel_format yuv420p -f rawvideo out.raw
```

Use nv12 for pixel_format for GPU output.
