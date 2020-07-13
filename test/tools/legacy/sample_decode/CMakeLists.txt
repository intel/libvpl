# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET sample_decode)

set(SOURCES src/pipeline_decode.cpp src/sample_decode.cpp)

add_executable(${TARGET} ${SOURCES})

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                                             ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${TARGET} sample_common mfx)

# todo: revisit this later -- this should really be pushed to the library
if(UNIX)
  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  target_link_libraries(${TARGET} Threads::Threads)
endif()

target_compile_definitions(${TARGET} PRIVATE -DDISABLE_NON_VPL)
