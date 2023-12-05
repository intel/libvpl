sample_encode is a Intel® Video Processing Library (Intel® VPL) application that
performs preprocessing and encoding of an uncompressed video stream of raw
native format (NV12 for GPU) according to a specific video compression standard
and can write the encoded video stream to a specified file.

Command Line format:    
```
sample_encode h264|h265|mpeg2|mvc|jpeg -i InputYUVFile -o OutputEncodedFile -w width -h height -angle 180 -opencl
```  
Sample Command Line:  
```
sample_encode h265 -hw -i ../../../content/cars_320x240.nv12 -w 320 -h 240 -o output.h265
```   
Sample Output:  
```
Loaded Library configuration:  
    Version: 2.7  
    ImplName: mfx-gen  
    Adapter number : 0  
    Adapter type: integrated  
    DRMRenderNodeNum: 128  
Used implementation number: 0  

Input file format       YUV420  
Output video            HEVC  
Source picture:  
        Resolution      320x240  
        Crop X,Y,W,H    0,0,320,240  
Destination picture:  
        Resolution      320x240  
        Crop X,Y,W,H    0,0,320,240  
Frame rate      30.00  
Bit rate(Kbps)  432  
Gop size        0  
Ref dist        0  
Ref number      0  
Idr Interval    0  
Target usage    balanced  
Memory type     system  
Media SDK impl          hw  
Media SDK version       2.7  
Processing started  
Frame number: 30  
Encoding fps: 994  
Processing finished
```  

