FROM ubuntu:20.04
ENV DEBIAN_FRONTEND=noninteractive
 
# Install a few minimal prerequisites 
RUN apt update && apt install -y --no-install-recommends \
      software-properties-common ca-certificates gnupg wget apt-utils \
      libva-drm2 vainfo  lsb-release \
      libva-dev libdrm2 libdrm-dev libpciaccess0 pciutils \
      build-essential cmake pkg-config strace
      

# Enable Agama package repository for Ubuntu 20.04
RUN wget -qO - https://repositories.intel.com/graphics/intel-graphics.key | apt-key add - && \
  echo "deb [arch=amd64] https://repositories.intel.com/graphics/ubuntu focal main" > /etc/apt/sources.list.d/intel-graphics.list

# Enable oneAPI package repository for Ubuntu 20.04
RUN wget -qO - https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2023.PUB | apt-key add - && \
  add-apt-repository "deb https://apt.repos.intel.com/oneapi all main"

# Install MSDK GPU media accelerator runtime 
RUN apt-get update && apt-get install -y --no-install-recommends \
      intel-media-va-driver-non-free \
      libmfx1 && \
  echo 'LIBVA_DRIVERS_PATH=/usr/lib/x86_64-linux-gnu/dri' >> /etc/environment && \ 
  echo 'LIBVA_DRIVER_NAME=iHD' >> /etc/environment && \ 
  ldconfig

# Install MSDK Developer Package
RUN apt-get update && apt-get install -y --no-install-recommends \
      libmfx-tools \
      libmfx-dev

# Install oneVPL Developer Package     
RUN apt-get update && apt-get install -y --no-install-recommends \
      intel-oneapi-onevpl-devel
