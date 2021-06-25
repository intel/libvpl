#!/bin/bash
# ==============================================================================
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

set -e
BASEDIR=$(dirname "$0")
echo "Building RHEL 8 Image"
docker build -f ${BASEDIR}/Dockerfile-rhel-8 -t vpl:rhel8 --build-arg DOCKER_REGISTRY=${1:-"registry.access.redhat.com/ubi8/"} ${BASEDIR}/..
echo "Building CentOS 7 Image"
docker build -f ${BASEDIR}/Dockerfile-centos-7 -t vpl:centos7 --build-arg DOCKER_REGISTRY=$1 ${BASEDIR}/..
echo "Building CentOS 8 Image"
docker build -f ${BASEDIR}/Dockerfile-centos-8 -t vpl:centos8 --build-arg DOCKER_REGISTRY=$1 ${BASEDIR}/..
echo "Building Ubuntu 18.04 Image"
docker build -f ${BASEDIR}/Dockerfile-ubuntu-18.04 -t vpl:ubuntu18.04 --build-arg DOCKER_REGISTRY=$1 ${BASEDIR}/..
echo "Building Ubuntu 20.04 Image"
docker build -f ${BASEDIR}/Dockerfile-ubuntu-20.04 -t vpl:ubuntu20.04 --build-arg DOCKER_REGISTRY=$1 ${BASEDIR}/..
