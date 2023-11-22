# ==============================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

FROM ubuntu:20.04 

# Prerequisites to enable access to repositories.intel.com
# and make package installs noninteractive
ARG DEBIAN_FRONTEND=noninteractive
ARG TZ=America/Los_Angeles
RUN ln -fs /usr/share/zoneinfo/America/Los_Angeles /etc/localtime && \
  apt-get update && apt-get install --no-install-recommends -q -y gpg-agent wget tzdata && \
  dpkg-reconfigure --frontend noninteractive tzdata && rm -rf /var/lib/apt/lists/*


# Set up access to repositories.intel.com 
SHELL ["/bin/bash", "-o", "pipefail", "-c"]
RUN wget --no-verbose -O - https://repositories.intel.com/graphics/intel-graphics.key | gpg --dearmor --output /usr/share/keyrings/intel-graphics.gpg && \
 echo 'deb [arch=amd64 signed-by=/usr/share/keyrings/intel-graphics.gpg] https://repositories.intel.com/graphics/ubuntu focal-legacy main' | tee  /etc/apt/sources.list.d/intel.gpu.focal.list

# Install Intel media graphics stack libraries (except VPL dispatcher)
RUN apt-get update && apt-get install --no-install-recommends -q -y \
  intel-opencl-icd \
  intel-level-zero-gpu level-zero \
  intel-media-va-driver-non-free libmfx1 libmfxgen1 \
  libigc-dev \
  intel-igc-cm \
  libigdfcl-dev \
  libigfxcmrt-dev \
  level-zero-dev \
  cmake build-essential pkg-config libva-dev libva-drm2 && rm -rf /var/lib/apt/lists/* && \
  python3 python3-pip opencl-headers libpugixml-dev libtbb-dev libtbb2 curl vainfo


# Uncomment line below to install VPL dispatcher and development libraries, 
# Use below if you will use apt-get packages instead of steps in INSTALL.md
#RUN apt-get install --no-install-recommends -q -y libvpl-dev && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/intel
# install OpenVINO
RUN wget --no-verbose https://storage.openvinotoolkit.org/repositories/openvino/packages/2022.3/linux/l_openvino_toolkit_ubuntu20_2022.3.0.9052.9752fafe8eb_x86_64.tgz -O l_openvino_toolkit_ubuntu20_2022.3.0.tgz && \
  tar -xf l_openvino_toolkit_ubuntu20_2022.3.0.tgz && \
  mv l_openvino_toolkit_ubuntu20_2022.3.0.9052.9752fafe8eb_x86_64 openvino_2022.3.0 && \
  ln -s /opt/intel/openvino_2022.3.0 /opt/intel/openvino_2022

WORKDIR /OpenVINO
# download mobilenet caffe model and convert it to OpenVINO IR model
RUN python3 -m pip install --no-cache-dir --upgrade pip && \
  pip3 install --no-cache-dir "openvino-dev[caffe]"==2022.3.0 && \
  omz_downloader --name mobilenet-ssd && \
  omz_converter --name mobilenet-ssd --precision FP32 --download_dir . --output_dir .


