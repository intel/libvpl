# ==============================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

ARG DEBIAN_FRONTEND=noninteractive
ARG BASE_IMAGE
FROM ${BASE_IMAGE}

LABEL Description="This is the image for Intel® oneVPL and OpenVINO interop testing based on Intel® Deep Learning Streamer Pipeline Framework (Intel® DL Streamer Pipeline Framework) image for Ubuntu 20.04 LTS"
LABEL Vendor="Intel Corporation"

USER root

# clean up previous package list, no need anymore and to prevent any regression from the later changes
# ex: OpenVINO
RUN rm /etc/apt/sources.list.d/*.list

# install oneVPL
RUN apt-get install --no-install-recommends -q -y gnupg wget software-properties-common && \
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
    apt-get install --no-install-recommends -q -y libmfxgen1 intel-media-va-driver-non-free libmfx1 libva-drm2 vainfo

# create work directory
RUN mkdir -p /oneVPL

# install OpenVINO runtime package
## remove old version which dlstreamer installed in /opt/intel because new version of omz_converter fails
RUN rm -r /opt/intel/openvino*
RUN echo "deb https://apt.repos.intel.com/openvino/2022 focal main" | sudo tee /etc/apt/sources.list.d/intel-openvino-2022.list
RUN apt-get update && \
    apt-get install -y python openvino-2022.3.0

# install OpenVINO dev package to download network model for vpl-infer
RUN python3 -m venv ./openvino_env
ENV PATH="openvino_env/bin:$PATH"
RUN python -m pip install --upgrade pip
RUN pip install openvino-dev[caffe]==2022.3.0

# download mobilenet caffe model and convert it to OpenVINO IR model
RUN omz_downloader --name mobilenet-ssd
RUN omz_converter --name mobilenet-ssd --precision FP32 --download_dir . --output_dir .

RUN mv ./public /oneVPL/nm

RUN apt-get purge -y gnupg wget software-properties-common && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

CMD ["/bin/bash"]
