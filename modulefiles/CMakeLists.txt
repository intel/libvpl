# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

if(UNIX)
  install(
    PROGRAMS vpl
    DESTINATION ${CMAKE_INSTALL_MODDIR}
    COMPONENT dev)
endif()
