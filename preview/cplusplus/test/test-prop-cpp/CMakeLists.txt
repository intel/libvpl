# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10)

# set the project name
project(test-prop-cpp)
set(TARGET test-prop-cpp)

find_package(VPL REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${TARGET} src/main.cpp)

target_link_libraries(${TARGET} PRIVATE VPL::dispatcher)
if(WIN32)
  cmake_policy(SET CMP0079 NEW)
  target_link_libraries(${TARGET} PRIVATE d3d11 dxgi)
endif()
