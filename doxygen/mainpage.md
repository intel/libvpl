# oneVPL C++ API

The oneAPI C++ Video Processing Library is a programming interface for video decoding, encoding, and processing to build portable media pipelines on CPUs, GPUs, and other accelerators. It provides device discovery and selection in media centric and video analytics workloads and API primitives for zero-copy buffer sharing. oneVPL is backwards and cross-architecture compatible to ensure optimal execution on current and next generation hardware without source code changes.

## Architecture

oneVPL functions fall into the following categories

- DECODE Functions that decode compressed video streams into raw video frames
- ENCODE Functions that encode raw video frames into compressed bitstreams
- VPP Functions that perform video processing on raw video frames

oneVPL functions are named after their functioning domain and category

## Programming Guide

This chapter describes the concepts used in programming with oneVPL.

The application must use the include file vpl.hpp for C++ programming.

Include these files:

```cpp
#include "vpl.hpp"    /* oneVPL include file */
```

Alternatively the vpl headers can be referenced directly as seen in the hello decode sample

### oneVPL Session

Before calling any oneVPL functions, the application must initialize the library and create a oneVPL session. A oneVPL session maintains context for the use of any vpl functions

#### Decode Session

##### The simplest initialization for a decode session

The simplest initialization for a decode session is one that specifies the target device and target codec. In the snippet below, the decode session is set to run on a CPU implementation and it will decode HEVC

```cpp
    // Default implementation selector. Selects first impl based on option list.
	oneapi::vpl::cpu_selector cpu;
    
    // File reader
    oneapi::vpl::bitstream_file_reader fr(source);

    // Load session and initialize decoder
	oneapi::vpl::decode_session decoder(cpu, (mfxU32)oneapi::vpl::codec_format_fourcc::hevc, &fr);
```

##### Checking to see if a codec is supported

To check if there is a implementation supports a needed codec,
the codec can be passed as a param to a selector which will try to match the codec to an implementation that can support it

```cpp
    // Create property descriptor. Please notice that this is constant expression!
    oneapi::vpl::property_name p1;
    oneapi::vpl::property opt(p1 / "mfxImplDescription" / "mfxDecoderDescription" / "decoder" / "CodecID", (uint32_t)vpl::codec_format_fourcc::hevc);

    // Default implementation selector. Selects first impl based on option list.
	oneapi::vpl::default_selector sel({opt});
    
    // File reader
    oneapi::vpl::bitstream_file_reader fr(source);

    // Load session and initialize decoder
	oneapi::vpl::decode_session decoder(sel, MFX_CODEC_HEVC, fr);
```

#### Encode Session

##### The simplest initialization for an encode session

The simplest initialization for an encode session is one that specifies the target device and target codec. In the snippet below, the encode session is set to run on a CPU implementation and it will decode HEVC

```cpp
    // Default implementation selector. Selects first impl based on option list.
    oneapi::vpl::cpu_selector cpu;
    
    // Frame source reader
    oneapi::vpl::frame_source_reader fr(source);

    // Load session and initialize encoder
    oneapi::vpl::encode_session encoder(cpu, fr);
```

#### Custom Source Readers

The `source_reader` interface can be subclassed if the default provided readers do not fit your application. Readers deliver data upon request and an end-of-stream flag. Readers should block if data is requested and pending or send an end-of-stream flag.

### Decoding Procedures

To decode frames

```cpp
    std::shared_ptr<oneapi::vpl::frame_surface> dec_surface_out = std::make_shared<oneapi::vpl::frame_surface>();

    decoder.decode_frame(dec_surface_out,{});
```

DecodeFrame is an asynchronous call and will start to decode the frame into the shared ptr.

Once the decoded surface is ready, it may be in non-system memory. The decoded surface will need to be converted to a system buffer

```cpp
    // sync on surface
    std::chrono::duration<int, std::milli> waitduration(WAIT_100_MILLSECONDS);
    st = dec_surface_out->wait_for(waitduration);

    if (oneapi::vpl::async_op_status::ready == st) {
        oneapi::vpl::frame_surface frame = dec_surface_out->get();
        auto [info, data] = frame.Map(MFX_MAP_READ);
        // Your code that interacts with the frame data
        frame.Unmap();
    }
```

#### 2nd argument to DecodeFrame

The 2nd argument to DecodeFrame is a list of extension buffers (in the example above, its empty). These extension buffers ask the decoder to do something more or get more information from it.

Each extension buffer is represented by it's corresponding class. Create an object of that class and add to the list.

Example, create an object of DecodeErrorReport extension buffer to get an error report of DecodeFrame

```cpp
    oneapi::vpl::ExtDecodeErrorReport err_report;
    std::shared_ptr<oneapi::vpl::frame_surface> dec_surface_out = std::make_shared<oneapi::vpl::frame_surface>();
    wrn = decoder.decode_frame(dec_surface_out,{&err_report});

    // Get error report
    // Before this step, verify decoder processed (due to async behavior),
    // otherwise err_report might not be updated
    std::cout << "error type = " << err_report.getRef().ErrorTypes << std::endl;
```

### Encoding Procedures

To encode frames

```cpp
    std::shared_ptr<bitstream_as_dst> bs = std::make_shared<oneapi::vpl::bitstream_as_dst>();

    encoder.encode_frame(bs,{});
```

EncodeFrame is an asynchronous call and will start to encode the frame into the shared ptr.

## FAQ

### What if I will pass encoder's extension buffer to the decoder?

Don't worry. For that case we have set of classes inherited from `buffer_list` class. Thanks to SFINAE we are verifying that you gave us the right extension buffer in the right place.

### Cool. What if something goes wrong?

1. If something really goes wrong (negative mfxStatus) - we will throw the exception. We have our lovely class to catch it.
2. Warnings (positive mfxStatus) are delivered as the method's return status for now. We know that this is bad but we raised it on oneapi level.
