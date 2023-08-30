## Cascade scaling

Some transcoding pipelines can greatly benefit from so called cascade scaling. SMT sample demonstrates how to implement and use cascade scaling. 

Let’s look at the transcoding case showed on the diagram below. Here one decoder feeds 8 encoders with different resolutions, frame rates and picture structures.

![original pipeline](./images/cs_org_pipeline.jpg)
 

As can be seen, deinterlacing and downscaling from original HD resolution is performed 6 times. Because deinterlacing is slow operation and downscaling from original resolution consumes a lot of memory bandwidth, this pipeline may be bottlenecked by VPP performance. To remove this bottleneck cascade scaling may be used as shown on the next diagram.

![CS pipeline](./images/cs_cs_pipeline.jpg)
 

Here, number of deinterlacing operations was reduce to two and just three downscaling operations are performed on original HD resolution. With growing number of channels and growing resolution ratio between decoder and encoder channels, benefits of cascade scaling will also grow.

To enable cascade scaling, new command line option was introduced “-cs”. It should be used in parameter file to define cascade scaling configuration. Adding this option to the channel description instructs sample to use output from the previous channel VPP instead of direct decoder output. Option is “sticky”. All subsequent channels specified in parameter file will use the same VPP output until new “-cs” option will be encountered. 

This is example of parameter file for the pipeline that is shown above.
```
-i::h264 in.h264 -async 3 -o::sink -join -trace
-i::source -join     -w 1920 -h 1080                                  -async 3 -b  8000 -o::h264 out101_di.264 
-i::source -join     -w  720 -h  480                                  -async 3 -b  4000 -o::h264 out102_di.264 
-i::source -join     -w 1280 -h  720 -FRC::PT -f 60 -deinterlace::ADI -async 3 -b  8000 -o::h264 out103_di.264 
-i::source -join -cs -w 1920 -h 1080 -FRC::PT -f 30 -deinterlace::ADI -async 3 -b  8000 -o::h264 out104_di.264
-i::source -join -cs -w 1280 -h  720                                  -async 3 -b  4000 -o::h264 out105_di.264 
-i::source -join -cs -w  640 -h  360                                  -async 3 -b  2000 -o::h264 out106_di.264
-i::source -join     -w  352 -h  288                                  -async 3 -b  1000 -o::h264 out107_di.264
-i::source -join     -w  240 -h  180                                  -async 3 -b  1000 -o::h264 out108_di.264
```


Cascade scaling performance strongly depends on HW capability, used VPP filters and input / output resolution ratio. To facilitate performance optimization, tracing capabilities were added to the sample. See smt-tracer-readme.md for more details how to enable tracing and use it to tune cascade scaling performance.
