# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)
project(legacy-decode-preview)

if(NOT TARGET vpl-example-util)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../util
                   ${CMAKE_CURRENT_BINARY_DIR}/util)
endif()

# Default install places 64 bit runtimes in the environment, so we want to do a
# 64 bit build by default.
if(WIN32)
  if(NOT DEFINED CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM
        x64
        CACHE STRING "")
    message(STATUS "Generator Platform set to ${CMAKE_GENERATOR_PLATFORM}")
  endif()
endif()

set(TARGET legacy-decode)
set(SOURCES src/legacy-decode.cpp)
set(CONTENTPATH ${CMAKE_CURRENT_SOURCE_DIR}/../../content)
set(RUNARGS -sw -i ${CONTENTPATH}/cars_128x96.h265)

# Set default build type to RelWithDebInfo if not specified
if(NOT CMAKE_BUILD_TYPE)
  message(
    STATUS "Default CMAKE_BUILD_TYPE not set using Release with Debug Info")
  set(CMAKE_BUILD_TYPE
      "RelWithDebInfo"
      CACHE
        STRING
        "Choose build type from: None Debug Release RelWithDebInfo MinSizeRel"
        FORCE)
endif()

add_executable(${TARGET} ${SOURCES})

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  if(NOT DEFINED ENV{VSCMD_VER})
    set(CMAKE_MSVCIDE_RUN_PATH $ENV{PATH})
  endif()
endif()

if(POLICY CMP0074)
  # ignore warning of VPL_ROOT in find_package search path
  cmake_policy(SET CMP0074 OLD)
endif()

find_package(VPL REQUIRED)
target_link_libraries(${TARGET} VPL::dispatcher)
find_path(VPL_UTIL_PATH util/util.h vpl-example-util
          ${CMAKE_CURRENT_SOURCE_DIR}/../../util)
target_include_directories(${TARGET} PRIVATE . ${VPL_UTIL_PATH})

get_directory_property(has_parent PARENT_DIRECTORY)
if(NOT has_parent)
  # only make run target available for stand-alone build
  add_custom_target(run ${TARGET} ${RUNARGS})
else()
  install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                    COMPONENT dev)
endif()
