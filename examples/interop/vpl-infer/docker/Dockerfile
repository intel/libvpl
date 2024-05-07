# ==============================================================================
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

ARG DOCKER_REGISTRY

FROM ${DOCKER_REGISTRY}ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

# Install test system prerequisites
RUN apt-get update && apt-get install -y --no-install-recommends \
    git \
    python3 \
    python3-pip \
    && \
    rm -rf /var/lib/apt/lists/*

# install xcb libs.
RUN apt-get update && apt-get install -y --no-install-recommends \
    libxcb-dri3-0 \
    libx11-xcb1 \
    libxcb-present0 \
    && \
    rm -rf /var/lib/apt/lists/*

# no intel-opencl-icd
RUN apt-get update && apt-get install -y --no-install-recommends \
    intel-media-va-driver-non-free \
    libigfxcmrt7 \
    && \
    rm -rf /var/lib/apt/lists/*

# Install Graphics runtime package
RUN apt-get update && apt-get install -y --no-install-recommends \
    gpg-agent wget gpg \
    && \
    rm -rf /var/lib/apt/lists/*

# Set up access to repositories.intel.com 
SHELL ["/bin/bash", "-o", "pipefail", "-c"]
RUN wget --no-verbose -e "no_proxy=" -O - https://repositories.intel.com/graphics/intel-graphics.key | gpg --dearmor --output /usr/share/keyrings/intel-graphics.gpg && \
    echo 'deb [arch=amd64 signed-by=/usr/share/keyrings/intel-graphics.gpg] https://repositories.intel.com/graphics/ubuntu jammy arc' | \
    tee  /etc/apt/sources.list.d/intel.gpu.jammy.list

# no intel-level-zero-gpu level-zero libvpl2 vainfo
RUN unset no_proxy && unset NO_PROXY && apt-get update && apt-get install -y --no-install-recommends \
    libmfx1 \
    libmfxgen1 \
    libvpl-dev \
    && \
    rm -rf /var/lib/apt/lists/*

RUN unset no_proxy && unset NO_PROXY && apt-get update && apt-get install -y --no-install-recommends \
    intel-opencl-icd intel-level-zero-gpu level-zero \
    intel-media-va-driver-non-free libmfx1 libmfxgen1 libvpl2 \
    libegl-mesa0 libegl1-mesa libegl1-mesa-dev libgbm1 libgl1-mesa-dev libgl1-mesa-dri \
    libglapi-mesa libgles2-mesa-dev libglx-mesa0 libigdgmm12 libxatracker2 mesa-va-drivers \
    mesa-vdpau-drivers mesa-vulkan-drivers va-driver-all \
    libigc-dev \
    intel-igc-cm \
    libigdfcl-dev \
    libigfxcmrt-dev \
    level-zero-dev \
    && \
    rm -rf /var/lib/apt/lists/*

# install build tools to test examples
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    build-essential \
    pkg-config \
    && \
    rm -rf /var/lib/apt/lists/*

# install libs for examples hw support
RUN unset no_proxy && unset NO_PROXY && apt-get update && apt-get install -y --no-install-recommends \
    libva-dev \
    && \
    rm -rf /var/lib/apt/lists/*

# install OpenVINO
RUN apt-get update && apt-get install -y --no-install-recommends \
    libpugixml-dev \
    libtbb-dev \
    libtbb2 \
    opencl-headers \
    && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /opt/intel
RUN wget --no-verbose https://storage.openvinotoolkit.org/repositories/openvino/packages/2023.2/linux/l_openvino_toolkit_ubuntu22_2023.2.0.13089.cfd42bd2cb0_x86_64.tgz -O openvino_2023.2.0.tgz && \
    tar -xf openvino_2023.2.0.tgz && \
    mv l_openvino_toolkit_ubuntu22_2023.2.0.13089.cfd42bd2cb0_x86_64 openvino_2023.2.0 && \
    ln -s /opt/intel/openvino_2023.2.0 /opt/intel/openvino

# download mobilenet caffe model and convert it to OpenVINO IR model
WORKDIR /OpenVINO
RUN python3 -m pip install --no-cache-dir --upgrade pip && \
    pip3 install --no-cache-dir 'openvino-dev[caffe]==2023.2.0' && \
    omz_downloader --name mobilenet-ssd && \
    omz_converter --name mobilenet-ssd --precision FP32 --download_dir . --output_dir .

# Use non-root user
ARG GROUP_ID=10000
ARG USER_ID=10001
RUN addgroup --gid ${GROUP_ID} --system appgroup \
 && adduser --uid ${USER_ID} --system --gid ${GROUP_ID} appuser
USER appuser

HEALTHCHECK CMD cmake --version || exit 1
