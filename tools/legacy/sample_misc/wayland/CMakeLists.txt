# ##############################################################################
# Copyright (C) 2005 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

if(NOT PKG_WAYLAND_CLIENT_FOUND)
  return()
endif()

add_library(vpl_wayland SHARED)

target_include_directories(
  vpl_wayland PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                      ${CMAKE_SOURCE_DIR}/api/vpl)

target_sources(
  vpl_wayland
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/class_wayland.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/src/listener_wayland.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/src/wayland-drm-protocol.c)

target_link_libraries(vpl_wayland PRIVATE sample_common wayland-client va drm
                                          drm_intel)

install(TARGETS vpl_wayland LIBRARY DESTINATION ${_TOOLS_LIB_PATH})
