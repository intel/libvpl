# ==============================================================================
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================
ARG BASE_IMAGE
FROM ${BASE_IMAGE}

ARG DEBIAN_FRONTEND=noninteractive
ENV PIP_ROOT_USER_ACTION=ignore
USER root
WORKDIR /setup
COPY MSPerf.py /opt/intel/samples/bin/performance/MSPerf.py
COPY performance.sh performance.sh

# Use non-root user
ARG GROUP_ID=10000
ARG USER_ID=10001
RUN addgroup --gid ${GROUP_ID} --system appgroup \
 && adduser --uid ${USER_ID} --system --gid ${GROUP_ID} appuser
USER appuser

HEALTHCHECK CMD python3 --version || exit 1
