# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET sample_common)
set(DEPENDENCIES mfx)

set(SOURCES
    src/base_allocator.cpp
    src/general_allocator.cpp
    src/mfx_buffering.cpp
    src/preset_manager.cpp
    src/sample_utils.cpp
    src/sysmem_allocator.cpp
    src/vpp_ex.cpp
    src/vm/atomic.cpp
    src/vm/shared_object.cpp
    src/vm/thread.cpp
    src/vm/thread_windows.cpp
    src/vm/time.cpp)

set(EXTRA_LINUX_SOURCES src/vm/atomic_linux.cpp src/vm/shared_object_linux.cpp
                        src/vm/thread_linux.cpp src/vm/time_linux.cpp)

add_library(${TARGET} STATIC "")

target_sources(${TARGET} PRIVATE ${SOURCES})

if(UNIX)
  target_sources(sample_common PRIVATE ${EXTRA_LINUX_SOURCES})
endif()

target_link_libraries(${TARGET} ${DEPENDENCIES})

target_include_directories(
  ${TARGET}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(${TARGET} PRIVATE -DDISABLE_NON_VPL)
