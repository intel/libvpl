# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10)

# set the project name
project(hello-decode-cpp)
set(TARGET hello-decode-cpp)

find_package(VPL REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

# if (MSVC) # warning level 4 and all warnings as errors add_compile_options(/W4
# /WX) else() # lots of warnings and all warnings as errors
# add_compile_options(-Wall -Wextra -Werror) endif()

# C++ API based decoder sample

add_executable(${TARGET} src/hello-decode.cpp)

target_link_libraries(${TARGET} PRIVATE VPL::dispatcher)
if(WIN32)
  cmake_policy(SET CMP0079 NEW)
  target_link_libraries(${TARGET} PRIVATE d3d11 dxgi)
endif()
