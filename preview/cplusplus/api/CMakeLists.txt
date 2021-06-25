# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# cmake_minimum_required(VERSION 3.10.2)

# set(TARGET VPLCPP) set(VPLCPP_API_INCLUDE_DIR
# ${CMAKE_CURRENT_SOURCE_DIR}/include)

# if(DEFINED ENV{ONEVPL_API_HEADER_DIRECTORY}) set(VPLCPP_API_INCLUDE_DIR
# $ENV{ONEVPL_API_HEADER_DIRECTORY}) message(STATUS "Using custom API header
# directory: ${VPLCPP_API_INCLUDE_DIR}") endif()

# add_library(vpl-cpp-api INTERFACE)

# target_include_directories(vpl-cpp-api INTERFACE ${VPLCPP_API_INCLUDE_DIR})

# if(BUILD_DEV) install( DIRECTORY ${VPLCPP_API_INCLUDE_DIR}/vpl DESTINATION
# ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT dev) endif()
