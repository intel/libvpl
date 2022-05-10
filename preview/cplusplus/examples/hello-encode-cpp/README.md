# `hello-encode` Sample

This sample shows how to use the oneAPI Video Processing Library (oneVPL) to
perform a simple video encode using preview C++ APIs.

| Optimized for    | Description
|----------------- | ----------------------------------------
| OS               | Ubuntu* 20.04
| Hardware         | Compatible with Intel® oneAPI Video Processing Library(oneVPL) GPU implementation, which can be found at https://github.com/oneapi-src/oneVPL-intel-gpu 
| Software         | Intel® oneAPI Video Processing Library(oneVPL) CPU implementation
| What You Will Learn | How to use oneVPL to encode a raw video file to H.265
| Time to Complete | 5 minutes


## Purpose

This sample is a command line application that takes a file containing a raw
format video elementary stream as an argument.  Using oneVPL, the application encodes and
writes the encoded output to a file named `out.h265` in H.265 format.

## Key Implementation details

| Configuration     | Default setting
| ----------------- | ----------------------------------
| Target device     | CPU
| Input format      | I420
| Output format     | H.265 video elementary stream
| Output resolution | same as the input

Native raw frame input format: CPU=I420, GPU=NV12.

## License

Code samples are licensed under the MIT license. See
[License.txt](https://github.com/oneapi-src/oneAPI-samples/blob/master/License.txt) for details.

Third-party program Licenses can be found here: [third-party-programs.txt](https://github.com/oneapi-src/oneAPI-samples/blob/master/third-party-programs.txt)


## Building the `hello-encode-cpp` Program

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
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake --build .
   ```

4. Run the program using the following command:
   ```
   ./hello-encode-cpp -i ../../../content/cars_320x240.i420 -w 320 -h 240
   ```


## Running the Sample

### Application Parameters

The instructions given above run the sample executable with the arguments
`-i <sample_dir>/content/cars_320x240.i420 -w 320 -h 240`.


### Example of Output

```
./hello-encode-cpp -i ../../../content/cars_320x240.i420 -w 320 -h 240

    BitDepthLuma   = 0
    BitDepthChroma = 0
    Shift          = Not Specifyed
    Color Format   = IYUV
    Size [W,H]     = [320,240]
    ROI [X,Y,W,H]  = [0,0,320,240]
    FrameRate [N:D]= 30:1
    AspecRato [W,H]= [Unset]
    PicStruct      = Progressive Picture
    ChromaFormat   = 4:2:0

Init done
Encoding ../../../content/cars_320x240.i420 -> out.h265
EndOfStream Reached
Encoded 30 frames

-- Encode information --

Base:
    AllocId    = 0
    AsyncDepth = 1
    Protected  = 0
    IOPattern  = In System Memory
Codec:
    LowPower           = Unset
    BRCParamMultiplier = 0
    CodecId            = HEVC
    CodecProfile       = 1
    CodecLevel         = 0
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
    PicStruct      = Progressive Picture
    ChromaFormat   = 4:2:0

Encoder:
    TargetUsage       = 4
    GopPicSize        = Not Specifyed
    GopRefDist        = Not Specifyed
    GopOptFlag        = Value is out of possible values
    IdrInterval       = 0
    RateControlMethod = CQP
    InitialDelayInKB  = 0
    QPI               = 0
    Accuracy          = 0
    BufferSizeInKB    = 0
    TargetKbps        = 0
    QPP               = 0
    ICQQuality        = 0
    MaxKbps           = 0
    QPB               = 0
    Convergence       = 0
    NumSlice          = Not Specifyed
    NumRefFrame       = Not Specifyed
    EncodedOrder      = False

```

You can find the output file `out.h265` in the build directory.

You can display the output with a video player that supports raw streams such as
FFplay. You can use the following command to display the output with FFplay:

```
ffplay out.h265
```
