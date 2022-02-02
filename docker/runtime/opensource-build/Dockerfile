# ==============================================================================
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

FROM ubuntu:20.04 as vpl_build_env
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        git \
        pkg-config \
        ca-certificates \
        dh-autoreconf \
        libdrm-dev \
        cmake \
        nasm \
        python3 \
        meson \
        && \
    rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/intel/libva.git && \
    cd libva && \
    git fetch --tags && \
    git checkout $(git describe --tags `git rev-list --tags --max-count=1`) && \
    ./autogen.sh && \
    make -j $(nproc --all) && \
    make install

FROM vpl_build_env as vpl_build
ENV VPL_INSTALL_DIR=/onevpl_install
RUN git clone https://github.com/oneapi-src/oneVPL.git /onevpl && \
    git clone https://github.com/oneapi-src/oneVPL-cpu.git /onevpl-cpu && \
    mkdir /onevpl_install && \
    /onevpl/script/build && \
    /onevpl/script/install && \
    /onevpl-cpu/script/build --bootstrap && \
    /onevpl-cpu/script/install

FROM ubuntu:20.04
ARG DEBIAN_FRONTEND=noninteractive
LABEL Description="oneVPL Runtime"
LABEL Vendor="Intel Corporation"
# Copy oneVPL 
COPY --from=vpl_build /onevpl_install/lib /lib
COPY --from=vpl_build /onevpl_install/bin/vpl-inspect /bin/vpl-inspect

#Install Graphics runtime package
RUN apt update && \
    apt install --no-install-recommends -q -y gnupg wget software-properties-common && \
    wget https://repositories.intel.com/graphics/intel-graphics.key && \
    apt-key add intel-graphics.key && \
    apt-add-repository 'deb [arch=amd64] https://repositories.intel.com/graphics/ubuntu focal main' && \
    apt update && \
    apt install --no-install-recommends -q -y libmfxgen1 intel-media-va-driver-non-free libmfx1 libva-drm2 vainfo && \
    apt purge -y gnupg wget software-properties-common && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*
