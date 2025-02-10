# ==============================================================================
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================
ARG BASE_IMAGE
FROM ${BASE_IMAGE}

USER root
WORKDIR /setup
COPY MSPerf.py /opt/intel/samples/bin/performance/MSPerf.py
COPY performance.sh performance.sh

ARG GROUP_ID
ARG USER_ID

RUN if [ -z "$GROUP_ID" ]; then echo \
    "GROUP_ID is not set for container. Use --build-arg GROUP_ID=$(id -g)"; \
    exit 1; fi
RUN if [ -z "$USER_ID" ]; then echo \
    "USER_ID is not set for container. Use --build-arg USER_ID=$(id -u)"; \
    exit 1; fi

# Use non-root user
RUN groupadd --system --gid ${GROUP_ID} appgroup \
 && useradd --system --create-home --uid ${USER_ID} --gid ${GROUP_ID} appuser
USER appuser

HEALTHCHECK CMD python3 --version || exit 1
