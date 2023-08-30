## Overview

This document describes how to configure and use SMT tracer for performance estimation and tuning. Tracer has been developed to tune complicated 1 to N transcoding pipelines, where one input stream is transcoded to several output streams with different resolutions, bitrates, and frame rates. It visualizes surface pool utilization, decode, VPP and encode task dependencies. It measures per frame End To End (E2E) and encoding (ENC) latencies.

Tracer supports conventional 1 to N and N to N pipelines and two major SMT performance tuning tools - cascade scaling and parallel encoding. See smt-cascade-scaling-readme.md and smt-parallel-encoding-readme.md for more details.

This document starts with general overview of tracer capabilities, then goes to E2E and ENC latencies tracing, and concludes by HOWTOs and known limitations.


## Tracing

Transcoding performance, e.g., cascade scaling performance, strongly depends on HW capability, used VPP filters and input / output resolution ratio. To facilitate performance optimization, tracing capabilities were added to the sample. To enable tracing add “-trace” command line option to the par file, see parameter file example above. Then, after execution, trace file with unique name will be generated.

In this trace file surface pools utilization will be showed. Picture below is an example for the pipeline described in smt-cascade-scaling-readme.md.

![surface pools utilization](./images/cs_surface_pool_utilization.jpg)

 
Eight pools are shown – decoder, three pools for cascade scaling (VPP pools) and four encoder pools (for 2nd, 3rd, 7th and 8th channels). As can be seen, decoder pool is completely utilized,  encoder pool for 2nd, 7th and 8th channels have optimal utilization, high enough, but with some spare frames in reserve and cascade scaling pools for 4th, 5th and 6th channels are underutilized. Reducing number of surfaces in these pools will reduce memory footprint in this particular case.

This trace file will also show general execution flow as illustrated by next three pictures. They are also captured for the pipeline described in previous section. 

![overall control flow](./images/cs_control_flow.jpg)
 

On this picture, from top to bottom. First line is decoding events, then three cascade scaling VPPs for 4th 5th and 6th channels, and then 8 encoders. In each channel task submission is shown, under “dec”, “csvpp”, “vpp” and “enc” names, “busy” wait, sync operation wait “syncp” and task dependencies (arrows). Note, that actual processing time (decoding, scaling or encoding) is not shown as a separate block on the diagram, but it can be deduced as a time interval between task submission and corresponded sync operation. Also note, that on high performance system duration of some events may be less than 1 microsecond. Such event is shown as zero duration event and its dependency may be incorrect. 

On this picture it can be clearly seen that third channel in this pipeline is the bottleneck. All channels except 3rd start VPP operation and encoding soon after decoding, 3rd channel only after several frames delay. It is also clearly visible from this traces that 3rd channel has two VPP and two encoder calls for each decoded frame, due to i60 to p60 frame rate conversion.

![cascade scaling control flow](./images/cs_control_flow_enc.jpg)
 

This is zoomed in area “1” of the previous picture. It shows relations between decoder, cascade scaling , VPPs and encoders in different channels. Note, that 1st channel depends only on decoder output, 2nd uses cascade scaler VPP but also runs its own VPP before encode, 4th uses cascade scaling output directly and so on.

![synchronization control flow](./images/cs_control_flow_sync.jpg)

 
This is zoomed-in area “2” of the previous picture. It shows sync point wait operations. Note, that all processing has been completed before this wait and wait finishes almost immediately for all channels. This is one more conformation that 3rd channel is the bottleneck in this case.


## E2E and ENC latencies

Depending on use case, we may need to tune pipeline for E2E or ENC latency. To facilitate this tuning, tracer measure these latencies and dumps them in *.csv file. Latencies are computed after transcoding is finished and this computation does not affect transcoding performance. That is why if tracer is enabled, it always computes and dumps latencies.

To measure E2E latency, we start timer just before MFXVideoDECODE_DecodeFrameAsync() call and stop it when MFXVideoCORE_SyncOperation() finishes wait for sync point returned from MFXVideoENCODE_EncodeFrameAsync() function. I.e., we measure latency from the moment we send bitstream to decoder till the moment we get bitstream from encoder. Note, that we don’t take into account auxiliary calls, like request for more bitstream data from decoder, because in this case we will also count time spent reading bitstream from disk. Picture below shows example of E2E latency measured for first encoder.

![E2E latency](./images/cs_e2e.jpg)

 
For ENC latency, we start timer just before MFXVideoENCODE_EncodeFrameAsync() call (very small blue rectangle on the picture below) and stop it just after MFXVideoCORE_SyncOperation() call (long green rectangle on the picture below). I.e., we measure pure encoding time, from the moment we send surface to encoding, till the moment we get encoded bitstream.

![ENC latency](./images/cs_enc.jpg)

 
Important note. To correctly interpret measured latencies, we have to take into account frame reordering. If input or output bitstreams have reordered frames, then DecodeFrameAsync() may consume one frame as input, but return surface that corresponds to another frame as output. The same is true for encoder – it may consume one frame as input but return another as output in the same EncodeFrameAsync() call. That means that in this case E2E or ENC latencies don’t show timing for the same frame.

Another thing that we have to take into account during latency measurement is asynchronous nature of transcoding. Even for async depth equals 1, sample calls EncodeFrameAsync() before actual decoding is finished. That means that ENC latency includes not only encoding time but decoding also. To measure only encoding time use “-trace::ENC” command line option. This option adds additional synchronization point between decoder and encoder and ensures that encoder is called after decoding is finished and only encoding latency is measured. This is example of pipeline with “-trace::ENC” option.

![ENC latency](./images/trace_enc_option.jpg)

 
Similar issue exists for E2E latency. Even for async depth equals 1, decoder starts decoding of next frame before encoding of previous frame is finished and as a result decoding workload overlaps with encoding one and decoding time may be measured incorrectly (decoder may finish earlier and wait for encoder to start). To avoid this, use “-trace::E2E” command line option. In this case decoder starts decoding of next frame only after encoding of previous one has been finished in all channels. This is example of “-trace::E2E” option.

![E2E latency](./images/trace_e2e_option.jpg)

  
Note, that both these options “-trace::E2E” and “-trace::ENC” affect pipeline and reduce throughput by introducing additional synchronization points. They should be used only to simulate specific use case, e.g., real-time streaming, when we start processing of next frame only after previous one has been encoded and sent out.


## HOWTOs


### How to find tracer output

Look at the very end of the SMT output in console. File names are unique for each run.

![file names](./images/cs_bufer_usage.jpg)
 

### How to choose tracer buffer size

Tracer buffer size is fixed to avoid dynamic memory allocation that impacts performance. If workload is big enough or unusually high number of events occur during transcoding, then tracer runs out of buffer. In this case tracing stops and only part of the trace is saved. That also affects latency calculation - only first frames will have latency measured.

SMT outputs buffer usage in console after transcoding. See picture above. If usage is 100%, look at the .csv file and check how many frames were captured, then increase buffer size accordingly. E.g., if 30% of the frames were capture, then triple buffer size. Use “-trace_buffer_size X” command line option.  Default buffer size is 7 MBytes. It is usually enough to capture 1000 frames in 1 to 8 transcoding pipeline. Maximum size is 127 Mbytes.


### How to view .json file

Open Google Chrome (TM). Type “chrome://tracing/” in address bar. Drag and drop .json file or click “Load” button and select .json file to view. Use “A”, “S”, “D”, “W” keys or mouse to navigate through traces.


### How to view .csv file

Open file, for example, using Microsoft(R) Excel(R).

![csv file](./images/cs_lat_chart1.jpg)
 
First five lines are short description of file format. Then goes latency data. For each channel in 1 to N pipeline there are four columns. First one is frame number. Note, that channels may have different numbers of frames, e.g., due to frame rate conversion. Second column is time when processing of current frame started. It may be start of decoding or encoding depending on use case - E2E or ENC. This column shows so called “wall clock” time. It usually starts at the system boot time but depending on implementation it may be different moment of time. Third column is the same moment in time, but from different starting point. This one from SMT start. Looking at the first frame in this column we can estimate application initialization time. Last column is latency. All numbers are in milliseconds.

To plot latency chart, delete irrelevant columns and rows, then select data and click insert chart button.

![chart](./images/cs_lat_chart2.jpg)
 

![chart](./images/cs_lat_chart3.jpg)

 
To debug system wide behavior, we can combine several .csv files. To do so, leave wall clock and latency columns for required channels, then move them to the same file and plot the chart.


## Known limitations.

Tracer works only with par file.

Tracer supports only default (GENERAL_ALLOC) memory model.

All sessions used in transcoding should be joined, overwise event dependencies in trace may be shown incorrectly and some latencies may be computed incorrectly. Trace itself will still be correct.
