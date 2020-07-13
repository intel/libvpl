# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) main build script
cmake_minimum_required(VERSION 3.10.2)

file(STRINGS "version.txt" version_txt)
project(oneVPL VERSION ${version_txt})

#
# Project configuration options
#

# Project options
option(OPTION_BUILD_TESTS "Build tests." ON)
option(OPTION_BUILD_DOCS "Build documentation." OFF)
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
add_subdirectory(src)
add_subdirectory(env)
add_subdirectory(doc)
add_subdirectory(test/tools/cli)
add_subdirectory(examples/hello-decode)
add_subdirectory(examples/hello-encode)
add_subdirectory(test/tools/legacy)

install(
  FILES third-party-programs.txt LICENSE
  DESTINATION ${CMAKE_INSTALL_LICENSEDIR}
  COMPONENT runtime)

execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  OUTPUT_FILE "VPL-ID.txt")

install(
  FILES ${PROJECT_BINARY_DIR}/VPL-ID.txt
  DESTINATION ${CMAKE_INSTALL_DOCDIR}
  COMPONENT runtime)

include(cmake/PackageTarget.cmake)
