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
target_link_libraries(vpl-encode VPL)

add_executable(vpl-decode vpl-decode.cpp vpl-new-dispatcher.cpp)
target_link_libraries(vpl-decode VPL)

add_executable(vpl-vpp vpl-vpp.cpp vpl-new-dispatcher.cpp)
target_link_libraries(vpl-vpp VPL)

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
