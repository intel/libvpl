FROM ubuntu:20.04 as vpl_install
ARG DEBIAN_FRONTEND=noninteractive

#Install oneVPL runtime package from oneAPI
RUN apt update && \
    apt install --no-install-recommends -q -y gnupg wget software-properties-common && \
    wget https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB && \
    apt-key add GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB && \
    echo "deb https://apt.repos.intel.com/oneapi all main" | tee /etc/apt/sources.list.d/oneAPI.list && \
    apt update && \
    apt install --no-install-recommends -q -y intel-oneapi-onevpl intel-oneapi-onevpl-devel && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

FROM ubuntu:20.04
ARG DEBIAN_FRONTEND=noninteractive
LABEL Description="oneVPL Runtime"
LABEL Vendor="Intel Corporation"

# Copy oneVPL 
COPY --from=vpl_install /opt/intel/oneapi/vpl/latest/lib/ /lib
COPY --from=vpl_install /opt/intel/oneapi/vpl/latest/bin/vpl-inspect /bin/vpl-inspect

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
