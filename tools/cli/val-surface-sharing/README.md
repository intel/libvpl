***
# `val-surface-sharing` tool
## Purpose
This tool is a command line application that validates Surface Sharing API
functionality of the Intel® Video Processing Library (Intel® VPL)

## Support
| Support          | Description
|----------------- | ----------------------------------------
| OS               | Windows* 10, Windows* 11, Ubuntu* 22.04
| Hardware         | Compatible with Intel® VPL GPU implementation, which can be found at https://github.com/oneapi-src/oneVPL-intel-gpu 

## Command line options
| Options          | Values     | Description
|----------------- | ---------- | ---------------
| -? -help         |            | 
| -c               | `h264`, `h265` | Select a codec name for Encode or Decode component test
| -i               | filename   | Input content file name. Raw video format or video elementary stream file
| -o               | filename   | Output content file name
| -adapters        |            | Display available adapters information
| -surface_component | `encode`, `decode`, `vpp_in`, `vpp_out` | Select a surface component to test
|                  |                                           | `encode`, `vpp_in`: Surface import testing
|                  |                                           | `decode`, `vpp_out` : Surface export testing
| -mode            | `shared`, `copy` | Select a surface sharing method
| -surface_type    | `d3d11`, `vaapi`, `opencl` | Select a surface type
|                  |                            | Support `d3d11` on Windows and `vaapi` on Linux for now
| -sw              | number | Input content resolution/ Width
| -sh              | number | Input content resolution/ Height
| -sc              | `nv12`, `rgb4` | Input content video format
| -dw              | number | Output content resolution/ Width
| -dh              | number | Output content resolution/ Height
| -dc              | `nv12`, `rgb4` | Output content video format
| -multi_session   |            | Execute a test case with multi threads

>**NOTE** If `-mode`, `-surface_type` is not set, then tool does not use Surface Sharing API and execute the process. This output is used as reference content in `val_surface_sharing_api.py` script.
>
> for example:
> ```
> val-surface-sharing.exe -surface_component encode -c h264 -i cars_320x240.nv12 -o out.h264 -sw 320 -sh 240 -sc nv12
> ```

## Command line examples
Display available adapters
```
val-surface-sharing.exe -adapters
```
Test D3D11 surface import, shared, and encode component
```
val-surface-sharing.exe -surface_component encode -surface_type d3d11 -mode shared -c h264 -i cars_320x240.nv12 -o out.h264 -sw 320 -sh 240 -sc nv12
```
Test D3D11 surface import, copy, and vpp_in component
```
val-surface-sharing.exe -surface_component vpp_in -surface_type d3d11 -mode copy -i cars_320x240.nv12 -o out.bgra -sw 320 -sh 240 -sc nv12 -dw 352 -dh 288 -dc rgb4
```
Test D3D11 surface export, share, and decode component
```
val-surface-sharing.exe -surface_component decode -surface_type d3d11 -mode shared -c h265 -i cars_320x240.h265 -o out.nv12
```
Test D3D11 surface export, copy, and vpp_out component
```
val-surface-sharing.exe -surface_component vpp_out -surface_type d3d11 -mode copy -i cars_320x240.nv12 -o out.bgra -sw 320 -sh 240 -sc nv12 -dw 352 -dh 288 -dc rgb4
```
For multi_session testing, add "-multi_session" to any test case
```
val-surface-sharing.exe -surface_component vpp_out -surface_type d3d11 -mode copy -i cars_320x240.nv12 -o out.bgra -sw 320 -sh 240 -sc nv12 -dw 352 -dh 288 -dc rgb4 -multi_session
```
> `multi_session` test will create 2 instances of test case defined from command line.
If there is only 1 adapter, then it will create 2 instances to the same adapter number and if there are more adapters (>=2), then create 2 instances to different 2 adapter numbers.

***
# `val_surface_sharing_api.py` script
## Purpose
This is automation script utilizing `val-surface-sharing` tool and validates Surface Sharing API functionalities.
All the test list is defined in the script which is composing the command line options.

## Preparation
You need to prepare the environment from install directory, for example:

On Windows
```
etc\vpl\vars.bat
```
On Linux
```
source etc/vpl/vars.sh
```
## Execution
On Windows (`d3d11`)
```
py -3 val_surface_sharing_api.py
```
On Linux (`vaapi`)
```
python ./val_surface_sharing_api.py
```
## Result
Output from the execution on Windows (`d3d11`)
```
[Command] val-surface-sharing.exe -adapters

Implementation #0: mfx-gen
  Library path: <...>\libmfx64-gen.dll
  AccelerationMode: MFX_ACCEL_MODE_VIA_D3D11
  ApiVersion: 2.9
  Impl: MFX_IMPL_TYPE_HARDWARE
  VendorImplID: 0x0000
  VendorID: 0x8086
  DeviceID: 9a60/0
  MediaAdapterType: MFX_MEDIA_INTEGRATED
...

Total number of implementations found = 4

Available adapter numbers for multi-session test = 2
  [0] adapter number: 0
  [1] adapter number: 1

======================================================================
[Test] Encode Component
======================================================================

[Command] val-surface-sharing.exe -surface_component encode -c h264 -i cars_320x240.nv12 -o ref_nv12.h264 -sw 320 -sh 240 -sc nv12

Created session with library: <...>\libmfx64-gen.dll
Warning: MFXVideoENCODE_Query returned 5.
Running:
        Surface Component = Encode
        Mode              = Not use surface sharing api
        Codec             = H.264
        Input File        = cars_320x240.nv12
        Output File       = ref_nv12.h264
        Input Colorspace  = NV12
        Input Resolution  = 320 x 240
Encoded 30 frames
----------------------------------------------------------------------
  encode/h264/320x240,nv12                                      PASS
----------------------------------------------------------------------

...


[Command] val-surface-sharing.exe -surface_component vpp_out -surface_type d3d11 -mode copy -i cars_320x240.bgra -o out.nv12 -sw 320 -sh 240 -sc rgb4 -dw 352 -dh 288 -dc nv12 -multi_session

...

Available adapter numbers for multi-session test = 2
  [0] adapter number: 0
  [1] adapter number: 1

Create 2 threads for the processing
  1. adapter number: 0, output file name: out.nv12.1
  2. adapter number: 1, output file name: out.nv12.2

Created m_session with library: C:\Temp\gen\drivers.gpu.media.sdk.lib\_build\__bin\Release\libmfx64-gen.dll
Running:
        Surface Component = VPP Out
        Surface Type      = D3D11 Tex2d
        Surface Flag      = Export.Copy
        Input File        = cars_320x240.bgra
        Output File       = out.nv12.1
        Input Colorspace  = RGB4
        Input Resolution  = 320 x 240
        Output Colorspace = NV12
        Output Resolution = 352 x 288
VPP 30 frames
Created m_session with library: C:\Temp\gen\drivers.gpu.media.sdk.lib\_build\__bin\Release\libmfx64-gen.dll
Running:
        Surface Component = VPP Out
        Surface Type      = D3D11 Tex2d
        Surface Flag      = Export.Copy
        Input File        = cars_320x240.bgra
        Output File       = out.nv12.2
        Input Colorspace  = RGB4
        Input Resolution  = 320 x 240
        Output Colorspace = NV12
        Output Resolution = 352 x 288
VPP 30 frames
----------------------------------------------------------------------
  vpp_out, d3d11, copy/320x240,rgb4->352x288,nv12/multi_session PASS
----------------------------------------------------------------------
  vpp_out, d3d11, copy                                          PASS
  3 tests, 3 passed, 0 failed
======================================================================
  Result Final:
  Surface Sharing API functional Validation Test                PASS
  40 tests, 40 passed, 0 failed
======================================================================

```

>**Note** This tool and script are provided so that end users can test functionalities, and not intended for direct use in customer products.
