# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET sample_encode)

set(SOURCES src/pipeline_encode.cpp src/pipeline_user.cpp src/sample_encode.cpp)

add_executable(${TARGET} ${SOURCES})

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                                             ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${TARGET} sample_common mfx)

if(UNIX)
  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  target_link_libraries(${TARGET} Threads::Threads)
endif()

target_compile_definitions(${TARGET} PRIVATE -DDISABLE_NON_VPL)

install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                  COMPONENT runtime)
