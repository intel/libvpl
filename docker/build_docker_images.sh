#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

set -e
BASEDIR=$(dirname "$0")
docker build -f ${BASEDIR}/Dockerfile-ubuntu-18.04 -t vpl:18.04 ${BASEDIR}/..
docker build -f ${BASEDIR}/Dockerfile-ubuntu-20.04 -t vpl:20.04 ${BASEDIR}/..
