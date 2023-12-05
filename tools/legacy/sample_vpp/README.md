sample_vpp is a Intel® Video Processing Library (Intel® VPL) application that
performs video processing of raw video sequences.

Command Line Format:
```
sample_vpp [Options] -i InputFile -o OutputFile  
```
Sample Command Line: 
```
sample_vpp -sw 320 -sh 240  -dw 320 -dh 240 -i out.raw -o crop.raw   
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
VPP started  
Frame number: 30  
VPP finished  
Total frames 30  
Total time 0.01 sec  
Frames per second 2151.000 fps  
```
