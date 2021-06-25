# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET sample_decode)
set(SOURCES "")
list(APPEND SOURCES src/pipeline_decode.cpp src/sample_decode.cpp)

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
target_link_libraries(${TARGET} sample_common)

install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                  COMPONENT dev)
