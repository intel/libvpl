# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

cmake_minimum_required(VERSION 3.13.0)

set(VPL_API_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED ENV{ONEVPL_API_HEADER_DIRECTORY})
  set(VPL_API_INCLUDE_DIR $ENV{ONEVPL_API_HEADER_DIRECTORY})
  message(STATUS "Using custom API header directory: ${VPL_API_INCLUDE_DIR}")
endif()

# get API version
file(READ ${VPL_API_INCLUDE_DIR}/vpl/mfxdefs.h mfxdefs)

string(REGEX MATCH "MFX_VERSION_MAJOR ([0-9]*)" _ ${mfxdefs})
set(API_VERSION_MAJOR ${CMAKE_MATCH_1})
set(API_VERSION_MAJOR
    ${API_VERSION_MAJOR}
    PARENT_SCOPE)

string(REGEX MATCH "MFX_VERSION_MINOR ([0-9]*)" _ ${mfxdefs})
set(API_VERSION_MINOR ${CMAKE_MATCH_1})
set(API_VERSION_MINOR
    ${API_VERSION_MINOR}
    PARENT_SCOPE)

message(STATUS "API version: ${API_VERSION_MAJOR}.${API_VERSION_MINOR}")
add_library(vpl-api INTERFACE)

target_include_directories(vpl-api INTERFACE ${VPL_API_INCLUDE_DIR})

if(INSTALL_DEV)
  install(
    DIRECTORY ${VPL_API_INCLUDE_DIR}/vpl
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT ${VPL_COMPONENT_DEV})
endif()
