# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  set(LIBS version)
endif()

add_executable(vpl-timing src/vpl-timing.cpp)
target_link_libraries(vpl-timing VPL ${LIBS})
target_include_directories(vpl-timing PRIVATE ${ONEVPL_API_HEADER_DIRECTORY})
