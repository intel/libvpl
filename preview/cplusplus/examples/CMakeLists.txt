# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) C++ examples

cmake_minimum_required(VERSION 3.10.2)

if(BUILD_EXAMPLES)
  add_subdirectory(hello-decode-cpp)
  add_subdirectory(hello-encode-cpp)
endif()

if(INSTALL_EXAMPLE_CODE)
  install(
    DIRECTORY hello-decode-cpp hello-encode-cpp
    DESTINATION ${ONEAPI_INSTALL_EXAMPLEDIR}/preview/cplusplus
    COMPONENT dev)
endif()
