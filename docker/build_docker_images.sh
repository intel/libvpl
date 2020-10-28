#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

set -e
BASEDIR=$(dirname "$0")
docker build -f ${BASEDIR}/Dockerfile-rhel-8 -t vpl-rhel:8 ${BASEDIR}/..
docker build -f ${BASEDIR}/Dockerfile-centos-7 -t vpl-centos:7 ${BASEDIR}/..
docker build -f ${BASEDIR}/Dockerfile-centos-8 -t vpl-centos:8 ${BASEDIR}/..
docker build -f ${BASEDIR}/Dockerfile-ubuntu-18.04 -t vpl-ubuntu:18.04 ${BASEDIR}/..
docker build -f ${BASEDIR}/Dockerfile-ubuntu-20.04 -t vpl-ubuntu:20.04 ${BASEDIR}/..
