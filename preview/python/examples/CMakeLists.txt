# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) Python examples

cmake_minimum_required(VERSION 3.10.2)

install(
  DIRECTORY hello-decode-py hello-encode-py hello-vpp-py
  DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}/preview/python
  COMPONENT dev)
