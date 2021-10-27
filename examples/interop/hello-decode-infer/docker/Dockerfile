FROM ubuntu:20.04
ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=America/Los_Angeles

# install environment package prerequisites
RUN apt update && \
    apt install --no-install-recommends -q -y software-properties-common gnupg wget sudo unzip libnss3-tools ncurses-term python3-pip


# Start with OpenVINO
RUN wget https://apt.repos.intel.com/openvino/2021/GPG-PUB-KEY-INTEL-OPENVINO-2021 && \
    apt-key add GPG-PUB-KEY-INTEL-OPENVINO-2021  && \
    echo "deb https://apt.repos.intel.com/openvino/2021 all main" | tee /etc/apt/sources.list.d/intel-openvino-2021.list && \
    apt update && apt install -y intel-openvino-dev-ubuntu20-2021.4.689


# Install Intel graphics stack packages from Agama
RUN wget https://repositories.intel.com/graphics/intel-graphics.key && \
    apt-key add intel-graphics.key && \
    apt-add-repository 'deb [arch=amd64] https://repositories.intel.com/graphics/ubuntu focal main' && \
    apt update && \
    apt install -y libmfx1 libmfxgen1 vainfo intel-opencl-icd intel-level-zero-gpu level-zero intel-media-va-driver-non-free python3.9


# Dev packages (optional for run-only environments)
RUN apt install -y cmake build-essential libva-dev libdrm-dev net-tools pkg-config libigc-dev intel-igc-cm libigdfcl-dev libigfxcmrt-dev level-zero-dev opencl-headers build-essential

#Install oneVPL devkit package from oneAPI
RUN wget https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB && \
    apt-key add GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB && \
    echo "deb https://apt.repos.intel.com/oneapi all main" | tee /etc/apt/sources.list.d/oneAPI.list && \
    apt update && apt install -y intel-oneapi-onevpl-devel  
