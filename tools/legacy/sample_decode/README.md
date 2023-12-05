sample_decode is a Intel® Video Processing Library (Intel® VPL) decode application
that takes a file containing an encoded video elementary stream as an argument,
performs decoding of various video compression formats, and writes the decoded
output to the file in raw native format (NV12 for GPU).

Command Line format:  
```sample_decode h264|h265|mpeg2|mvc|jpeg -i in.bit -o out.yuv```  
Sample Command Line:  
```sample_decode h265 -i ../../../content/cars_320x240.h265 -o out.yuv```  
Sample Output:  
```
Loaded Library configuration:  
    Version: 2.7  
    ImplName: mfx-gen  
    Adapter number : 0  
    Adapter type: integrated  
    DRMRenderNodeNum: 128  
Used implementation number: 0  
Decoding started  
Frame number:   30, fps: 742.868, fread_fps: 0.000, fwrite_fps: 0.000358  
Decoding finished
```
