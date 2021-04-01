#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

set -e
BASEDIR=$(dirname "$0")
docker build -f ${BASEDIR}/Dockerfile-rhel-8 -t vpl:rhel8 --build-arg DOCKER_REGISTRY=${1:-"registry.access.redhat.com/ubi8/"} ${BASEDIR}/..
docker build -f ${BASEDIR}/Dockerfile-centos-7 -t vpl:centos7 --build-arg DOCKER_REGISTRY=$1 ${BASEDIR}/..
docker build -f ${BASEDIR}/Dockerfile-centos-8 -t vpl:centos8 --build-arg DOCKER_REGISTRY=$1 ${BASEDIR}/..
docker build -f ${BASEDIR}/Dockerfile-ubuntu-18.04 -t vpl:ubuntu18.04 --build-arg DOCKER_REGISTRY=$1 ${BASEDIR}/..
docker build -f ${BASEDIR}/Dockerfile-ubuntu-19.10 -t vpl:ubuntu19.10 --build-arg DOCKER_REGISTRY=$1 ${BASEDIR}/..
docker build -f ${BASEDIR}/Dockerfile-ubuntu-20.04 -t vpl:ubuntu20.04 --build-arg DOCKER_REGISTRY=$1 ${BASEDIR}/..
