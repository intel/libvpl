# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) main build script
cmake_minimum_required(VERSION 3.10.2)

file(STRINGS "version.txt" version_txt)
project(oneVPL VERSION ${version_txt})
if(NOT "${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  message(FATAL_ERROR "Unsupported architecture: only 64-bit supported")
endif()

#
# Project configuration options
#

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Default CMAKE_BUILD_TYPE not set using Release")
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE
        STRING
        "Choose build type from: None Debug Release RelWithDebInfo MinSizeRel"
        FORCE)
endif()

# Project options
option(OPTION_BUILD_EXAMPLES "Build examples." OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

include(cmake/oneAPIInstallDirs.cmake)

# Keep executable bits on shared objects when installing regardless of distro
set(CMAKE_INSTALL_SO_NO_EXE 0)

include(cmake/CompileOptions.cmake)

add_subdirectory(api)
add_subdirectory(dispatcher)
add_subdirectory(env)
add_subdirectory(modulefiles)
add_subdirectory(tools/cli)
add_subdirectory(tools/legacy)
add_subdirectory(examples/hello-decode)
add_subdirectory(examples/hello-encode)
add_subdirectory(examples/hello-vpp)
add_subdirectory(examples/dpcpp-blur)

install(
  FILES third-party-programs.txt
  DESTINATION ${CMAKE_INSTALL_LICENSEDIR}
  COMPONENT license)

install(
  FILES LICENSE
  RENAME license.txt
  DESTINATION ${CMAKE_INSTALL_LICENSEDIR}
  COMPONENT license)

include(cmake/PackageTarget.cmake)
