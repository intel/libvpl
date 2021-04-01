# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET sample_vpp)
set(SOURCES "")
list(
  APPEND
  SOURCES
  src/sample_vpp.cpp
  src/sample_vpp_config.cpp
  src/sample_vpp_frc.cpp
  src/sample_vpp_frc_adv.cpp
  src/sample_vpp_parser.cpp
  src/sample_vpp_pts.cpp
  src/sample_vpp_roi.cpp
  src/sample_vpp_utils.cpp)

add_executable(${TARGET} ${SOURCES})

if(MSVC)
  target_compile_definitions(${TARGET} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

if(POLICY CMP0074)
  # ignore warning of VPL_ROOT in find_package search path
  cmake_policy(SET CMP0074 OLD)
endif()

target_include_directories(${TARGET}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(VPL REQUIRED)
target_link_libraries(${TARGET} VPL::dispatcher)
target_link_libraries(${TARGET} sample_common)

install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                  COMPONENT runtime)
