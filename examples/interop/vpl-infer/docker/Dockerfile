# ==============================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

FROM intel/dlstreamer:2022.2.0-ubuntu20-gpu419.40-devel
ARG DEBIAN_FRONTEND=noninteractive

LABEL Description="This is the image for Intel® oneVPL and OpenVINO interop testing based on Intel® Deep Learning Streamer Pipeline Framework (Intel® DL Streamer Pipeline Framework) image for Ubuntu 20.04 LTS"
LABEL Vendor="Intel Corporation"

USER root

# install oneVPL
RUN apt-get update && \
    apt-get install --no-install-recommends -q -y gnupg wget software-properties-common && \
    wget https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB && \
    apt-key add GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB && \
    echo "deb https://apt.repos.intel.com/oneapi all main" | tee /etc/apt/sources.list.d/oneAPI.list

RUN apt-get update && \
    apt-get install --no-install-recommends -q -y intel-oneapi-onevpl intel-oneapi-onevpl-devel && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# install prerequisite for oneVPL
RUN wget https://repositories.intel.com/graphics/intel-graphics.key && \
    apt-key add intel-graphics.key && \
    apt-add-repository 'deb [arch=amd64] https://repositories.intel.com/graphics/ubuntu focal main'
    
RUN apt-get update && \
    apt-get install --no-install-recommends -q -y libmfxgen1 intel-media-va-driver-non-free libmfx1 libva-drm2 vainfo && \
    apt-get purge -y gnupg wget software-properties-common && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN mkdir -p /oneVPL

# download mobilenet caffe model and convert it to OpenVINO IR model
RUN omz_downloader --name mobilenet-ssd
RUN omz_converter --name mobilenet-ssd --precision FP32 --download_dir . --output_dir .
RUN mv ./public /oneVPL/nm

CMD ["/bin/bash"]
