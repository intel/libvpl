# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(mfxinit-test main.cpp)
target_link_libraries(mfxinit-test VPL)
target_include_directories(mfxinit-test PRIVATE ${ONEVPL_API_HEADER_DIRECTORY})
target_compile_definitions(${TARGET} PUBLIC -DMFX_DEPRECATED_OFF)
