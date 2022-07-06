# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)
project(system_analyzer)

if(WIN32)
  message(STATUS "skipping system_analyzer, not supported on Windows")
  return()
endif()

if(NOT BUILD_TOOLS_ONEVPL_EXPERIMENTAL)
  message(
    WARNING "BUILD_TOOLS_ONEVPL_EXPERIMENTAL required to build system_analyzer")
  return()
endif()

set(TARGET system_analyzer)
set(SOURCES system_analyzer.cpp)

add_executable(${TARGET} ${SOURCES})
target_compile_definitions(${TARGET} PRIVATE -DONEVPL_EXPERIMENTAL)

find_package(VPL REQUIRED)
target_link_libraries(${TARGET} VPL::dispatcher)
target_link_libraries(${TARGET} dl pthread)
install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                  COMPONENT dev)
