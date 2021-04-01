# Tutorial: Transition from Intel(R) Media SDK to Intel(R) oneAPI Video Processing Library (oneVPL)

This tutorial shows the first steps to upgrade Intel(R) Media SDK code to Intel(R) oneAPI Video Processing Library.

**Note**: This tutorial is based on Docker and assumes that you have already set up and configured Docker for your environment.

**Contents:**
- [Create an image for the tutorial](#create-an-image-for-the-tutorial)
- [Build examples using the same source for both toolkits](#build-examples-using-the-same-source-for-both-toolkits)
- [Run examples with Intel(R) Media SDK](#run-examples-with-intelr-media-sdk)
- [Run examples with oneVPL](#run-examples-with-onevpl)

## Create an image for the tutorial
To create an image with all prerequisites and start a container, run the `run.sh` script:

```bash
./run.sh
```

This image is based on Ubuntu 20.04.

Among other prerequisites for GPU use, this image installs:
* the MSDK developer package from Agama
* the oneVPL developer package from the Intel oneAPI program

## Build example

The example illustrated here shows the small changes needed to upgrade.

Review legacy-createsession code to see initialization changes:

Media SDK initialization:
```
    sts = MFXInit(cliParams.impl, &version, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session");
```

oneVPL initialization:
```
    mfxConfig cfg    = NULL;
    mfxLoader loader = NULL;

    loader = MFXLoad();
    VERIFY(NULL != loader, "MFXLoad failed -- is implementation in path?");

    cfg = MFXCreateConfig(loader);
    VERIFY(NULL != cfg, "MFXCreateConfig failed")

    sts = MFXSetConfigFilterProperty(
        cfg,
        reinterpret_cast<mfxU8 *>(const_cast<char *>("mfxImplDescription.Impl")),
        cliParams.implValue);
    VERIFY(MFX_ERR_NONE == sts, "MFXSetConfigFilterProperty failed");

    sts = MFXCreateSession(loader, 0, &session);
    VERIFY(MFX_ERR_NONE == sts, "Not able to create VPL session");
``` 

Build the example with Intel(R) Media SDK:

```bash
cd preview/legacy-createsession/src
g++ legacy-createsession.cpp -o hello_mediasdk -I/usr/include/mfx -I../../../util -lmfx -lva -lva-drm -lpthread -DUSE_MEDIASDK1
```

Build the example with oneVPL:

```bash
source /opt/intel/oneapi/vpl/latest/env/vars.sh
g++ legacy-createsession.cpp -o hello_onevpl -I/opt/intel/oneapi/vpl/latest/include/vpl -I../../../util -lmfx -lva -lva-drm -lpthread
```

**Note**: The oneVPL dispatcher library name was recently updated to `vpl`. Change `-lmfx` to `-lvpl` when compiling
after this `intel-oneapi-onevpl` change has propagated.

Other than API upgrades, the oneVPL repository adds the convention of adding the `vpl` directory name before include files.
I420 is added as a more recognizable name for the industry standard planar YUV 4:2:0.

These minor differences are shown at the top of `util.h`:

```c++
#ifdef USE_MEDIASDK1
    #include "mfxvideo.h"
    enum {
    MFX_FOURCC_I420         = MFX_FOURCC_IYUV                 /*!< Alias for the IYUV color format. */
    };
#else
    #include "vpl/mfxvideo.h"
    #include "vpl/mfxjpeg.h"
#endif
```

## Run examples with Intel(R) Media SDK

Run the example code as compiled with Intel(R) Media SDK:

```bash
./hello_mediasdk -hw
```

**Expected output:**

```bash
Implementation info
        version = 1.34
        impl = Hardware:VAAPI
```

Check that the legacy Intel(R) Media SDK dispatcher was used:

```bash
strace ./hello_mediasdk 2>&1 | grep 'libmfx'
```

**Expected output:**

```bash
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libmfx.so.1", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libmfxhw64.so.1", O_RDONLY|O_CLOEXEC) = 3
```

`libmfx.so.1` is the 1.x legacy Intel(R) Media SDK dispatcher.


## Run examples with oneVPL

Run the example code as compiled with oneVPL:

```bash
./hello_onevpl -hw
```

**Expected output:**

```bash
Implementation info
        version = 1.34
        impl = Hardware:VAAPI
```

Check that the oneVPL dispatcher was used:

```bash
strace ./hello_onevpl 2>&1 | grep 'libmfx'
```

**Expected output:**

```bash
openat(AT_FDCWD, "/opt/intel/oneapi/vpl/2021.1.1/lib/libmfx.so.2", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/opt/intel/oneapi/vpl/2021.1.1/lib/libmfxhw64.so.1", O_RDONLY|O_CLOEXEC) = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libmfxhw64.so.1", O_RDONLY|O_CLOEXEC) = 3
```

`libmfx.so.2` is the 2.x oneVPL dispatcher.
