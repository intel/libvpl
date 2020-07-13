# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

if(WIN32)
  install(
    DIRECTORY cmd/
    DESTINATION ${CMAKE_INSTALL_ENVDIR}
    USE_SOURCE_PERMISSIONS
    COMPONENT dev)
else()
  install(
    DIRECTORY bash/
    DESTINATION ${CMAKE_INSTALL_ENVDIR}
    USE_SOURCE_PERMISSIONS
    COMPONENT dev)
endif()
