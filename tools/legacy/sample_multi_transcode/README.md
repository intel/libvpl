sample_multi_transcode performs the transcoding (decoding and encoding) of a
video stream of one compressed video format to another, with optional video
processing (resizing) of uncompressed video prior to encoding. The application
supports multiple input and output streams meaning it can execute multiple
transcoding sessions concurrently.

Command Line Format:  
```
Format-1: sample_multi_transcode [options] [--] pipeline-description  
Format-2: sample_multi_transcode [options] -par ParFile  
```
ParFile is extension of what can be achieved by setting pipeline in the command line.  
Sample Command Line: 
```
sample_multi_transcode -hw -i::h265 ../../../content/cars_320x240.h265  -o::mpeg2 out.mpeg2  
```
This command line transcodes a h265 video file to mpeg2 video format and writes it to out.mpeg2.  
Sample Output: 
```
Session 0:  
Loaded Library configuration:  
    Version: 2.7  
    ImplName: mfx-gen  
    Adapter number : 0  
    Adapter type: integrated  
    DRMRenderNodeNum: 128  
Used implementation number: 0  
Input  video: HEVC  
Output video: MPG2  
Session 0 was NOT joined with other sessions  
Transcoding started  
Transcoding finished  
Common transcoding time is 0.0288 sec  
-------------------------------------------------------------------------------  
*** session 0 [0x295d730] PASSED (MFX_ERR_NONE) 0.0287292 sec, 30 frames, 1044.235 fps  
-hw -i::h265 ../../../content/cars_320x240.h265 -o::mpeg2 out.mpeg2  
```
