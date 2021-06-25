# Docker file support   

The following files enable building the oneAPI Video Processing Library (oneVPL)
API and samples as a Docker containers for Ubuntu 18.04, Ubuntu 20.04, CentOS 7, CentOS 8, and RHEL 8.1.

# Building

The following command will build all of the supported docker container images:
```bash
$ ./build_docker_images.sh
```


**LEGAL NOTICE:  By downloading and using any scripts (the “Software Package”) and the included software or software made available for download, you agree to the terms and conditions of the software license agreements for the Software Package, which may also include notices, disclaimers, or license terms for third party software (together, the “Agreements”) included in this README file.
If the Software Package is installed through a silent install, your download and use of the Software Package indicates your acceptance of the Agreements.**

Docker files will also pull in the following: 
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
* [CentOS 7](https://hub.docker.com/layers/centos/library/centos/7/images/sha256-b79613a1c63084399b6c21c97ab4e2816ef5e9c513842c1bd4dca46bdd921b31?context=explore)
* [CentOS 8](https://hub.docker.com/layers/centos/library/centos/centos8/images/sha256-7723d6b5d15b1c64d0a82ee6298c66cf8c27179e1c8a458e719041ffd08cd091?context=explore)
* [RedHat 8](https://catalog.redhat.com/software/containers/ubi8/ubi/5c359854d70cc534b3a3784e?tag=8.4&push_date=1622575047000&container-tabs=gti)
* [Ubuntu 18.04](https://hub.docker.com/layers/ubuntu/library/ubuntu/18.04/images/sha256-dce82ba9ee1bc3a515212bb17fa21c134102bffafb5234a25ac10747df25816b?context=explore)
* [Ubuntu 20.04](https://hub.docker.com/layers/ubuntu/library/ubuntu/20.04/images/sha256-42d5c74d24685935e6167271ebb74c5898c5adf273dae80a82f9e39e8ae0dab4?context=explore)
* [Libva](https://github.com/intel/libva.git)