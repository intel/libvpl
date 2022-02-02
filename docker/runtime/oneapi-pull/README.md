# Docker file support   

The files in this folder demonstrate a oneAPI Video Processing Library (oneVPL)
runtime environment created using packages published via Intel(R) oneAPI.

The Dockerfile will:

* Create a container into which full oneVPL packages will be pulled.
This will include the development package so that vpl-inspect can be included for
basic smoke testing of the final container.
* Create a runtime container
* Copy oneVPL dispatcher and CPU runtime files
* Use `apt-get` to pull in GPU runtime files

# Running Container

To run a container first you must build the image.

```bash
$ docker build -t onevpl-runtime:1.0 .
```

After building the image you can create a container.

```bash
$ docker container run  --device /dev/dri onevpl-runtime:1.0
```

Alternatively you may use the following command to launch an
interactive bash shell in a temporary container

```bash
$ docker container run --rm -it onevpl-runtime:1.0 bash
```

## Accessing Hardware Resources

Docker requires elevated access to access the GPU device(es). (A more detailed discussion can be found in the OpenVINO documentation https://github.com/openvinotoolkit/docker_ci/blob/master/configure_gpu_ubuntu20.md). This is only needed
for hardware access. Using the CPU implementation does not require this step.

* Add your user to the `render` group 
* start the docker container using the `--device /dev/dri` option to give access to the driver.

# Running Using Docker Compose

To run a container using docker compose you can simply launch using docker-compose.

```bash
$ docker-compose up
```

You can open an interactive shell in the container using

```bash
$ docker-compose run onevpl bash
```

# Notice

**LEGAL NOTICE:  By downloading and using any scripts (the “Software Package”) and the included software or software made available for download, you agree to the terms and conditions of the software license agreements for the Software Package, which may also include notices, disclaimers, or license terms for third party software (together, the “Agreements”) included in this README file.
If the Software Package is installed through a silent install, your download and use of the Software Package indicates your acceptance of the Agreements.**

Docker files will also pull in the following: 

* [Ubuntu 20.04](https://hub.docker.com/layers/ubuntu/library/ubuntu/20.04/images/sha256-42d5c74d24685935e6167271ebb74c5898c5adf273dae80a82f9e39e8ae0dab4?context=explore)
* [gnupg](https://gnupg.org/)
* [wget](https://www.gnu.org/software/wget/)
* [software-properties-common](https://packages.ubuntu.com/focal/admin/software-properties-common)
* [Intel(R) Media SDK](https://github.com/Intel-Media-SDK/MediaSDK)
** libmfxgen1
** intel-media-va-driver-non-free
** libmfx1
** [Video Acceleration (VA) API for Linux -- DRM runtime (libva-drm2)](https://01.org/linuxmedia/vaapi)
** [Libva](https://github.com/intel/libva.git)
* [git](https://git-scm.com)
* [build-essential](https://packages.debian.org/sid/build-essential)
* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)
* [ca-certificates](https://packages.ubuntu.com/source/hirsute/ca-certificates)
* [dh-autoeconf](https://salsa.debian.org/debian/dh-autoreconf)
* [libdrm-dev](https://cgit.freedesktop.org/mesa/drm/)
* [cmake](https://cmake.org/)
* [libtool](https://www.gnu.org/software/libtool/)
* [make](https://www.gnu.org/software/make/)
* [pkgconfig](https://github.com/matze/pkgconfig)
* [devtoolset-7](https://www.softwarecollections.org/en/scls/rhscl/devtoolset-7/)
* [gcc](https://gcc.gnu.org/)
* [gcc-c++](https://gcc.gnu.org/)
* [libarchive](https://www.libarchive.org/)
* [Libva](https://github.com/intel/libva.git)

# Known issues

If running behind a proxy you may need to add proxy settings to the image.
For details see the [Docker documentation](https://docs.docker.com/network/proxy/)
