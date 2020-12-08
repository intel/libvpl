# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(vpl-encode vpl-encode.cpp vpl-new-dispatcher.cpp)
add_executable(vpl-decode vpl-decode.cpp vpl-new-dispatcher.cpp)
add_executable(vpl-vpp vpl-vpp.cpp vpl-new-dispatcher.cpp)

if(UNIX)
  find_path(VA_INCLUDE_DIR va/va.h HINTS /usr/include)
  if(VA_INCLUDE_DIR)
    add_definitions(-DENABLE_VAAPI)
    target_link_libraries(vpl-encode VPL va va-drm pthread)
    target_link_libraries(vpl-decode VPL va va-drm pthread)
    target_link_libraries(vpl-vpp VPL va va-drm pthread)
    message(STATUS "VAAPI detected, enabling -hw for cli tools")
  else()
    target_link_libraries(vpl-encode VPL)
    target_link_libraries(vpl-decode VPL)
    target_link_libraries(vpl-vpp VPL)
    message(STATUS "No VAAPI, compiling without -hw for cli tools")
  endif()
else()
  target_link_libraries(vpl-encode VPL)
  target_link_libraries(vpl-decode VPL)
  target_link_libraries(vpl-vpp VPL)
endif()

add_executable(vpl-vppenc vpl-vppenc.cpp vpl-new-dispatcher.cpp)
target_link_libraries(vpl-vppenc VPL)

add_executable(vpl-decenc vpl-decenc.cpp vpl-new-dispatcher.cpp)
target_link_libraries(vpl-decenc VPL)

add_executable(vpl-decvpp vpl-decvpp.cpp vpl-new-dispatcher.cpp)
target_link_libraries(vpl-decvpp VPL)

add_executable(vpl-inspect vpl-inspect.cpp)
target_link_libraries(vpl-inspect VPL)

install(
  TARGETS vpl-encode
          vpl-decode
          vpl-vpp
          vpl-inspect
          vpl-vppenc
          vpl-decenc
          vpl-decvpp
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime)
