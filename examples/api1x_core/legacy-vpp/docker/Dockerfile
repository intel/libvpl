FROM ubuntu:20.04
ARG DEBIAN_FRONTEND=noninteractive


#Install Graphics runtime package
RUN apt-get update && \
    apt-get install --no-install-recommends -q -y gnupg wget software-properties-common && \
    wget --no-verbose https://repositories.intel.com/graphics/intel-graphics.key && \
    apt-key add intel-graphics.key && \
    apt-add-repository 'deb [arch=amd64] https://repositories.intel.com/graphics/ubuntu focal main' && \
    apt-get update && \
    apt-get install --no-install-recommends -q -y libmfxgen1 intel-media-va-driver-non-free libmfx1 libva-dev libva-drm2 vainfo && \
    apt-get install --no-install-recommends -q -y cmake build-essential pkg-config && rm -rf /var/lib/apt/lists/*

# Use non-root user
ARG GROUP_ID=10000
ARG USER_ID=10001
RUN addgroup --gid ${GROUP_ID} --system appgroup \
 && adduser --uid ${USER_ID} --system --gid ${GROUP_ID} appuser
USER appuser

HEALTHCHECK CMD cmake --version || exit 1
