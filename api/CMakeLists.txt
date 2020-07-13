# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) API build script
cmake_minimum_required(VERSION 3.10.2)

add_library(vpl-api INTERFACE)

target_include_directories(vpl-api INTERFACE .)

install(
  DIRECTORY vpl
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT dev)

set(DEST_DIR "${CMAKE_INSTALL_PREFIX}")
configure_file("vpl.pc.in" "vpl.pc" @ONLY)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/vpl.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  COMPONENT dev)
