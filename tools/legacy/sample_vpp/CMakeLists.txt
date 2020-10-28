# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET sample_vpp)

set(SOURCES
    src/sample_vpp.cpp
    src/sample_vpp_config.cpp
    src/sample_vpp_parser.cpp
    src/sample_vpp_utils.cpp
    src/sample_vpp_roi.cpp
    src/sample_vpp_pts.cpp
    src/sample_vpp_frc.cpp
    src/sample_vpp_frc_adv.cpp)

add_executable(${TARGET} ${SOURCES})

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                                             ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${TARGET} sample_common VPL)

if(UNIX)
  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  target_link_libraries(${TARGET} Threads::Threads)
endif()

target_compile_definitions(${TARGET} PRIVATE -DDISABLE_NON_VPL)

install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                  COMPONENT runtime)
