# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

if(UNIX)
  install(
    PROGRAMS vpl
    DESTINATION ${ONEAPI_INSTALL_MODFILEDIR}
    COMPONENT dev)
endif()
