# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
if(CMAKE_HOST_WIN32)
  # need at least CMake 3.23 for IntelLLVM support of IntelDPCPP package on
  # Windows
  cmake_minimum_required(VERSION 3.23)
else()
  # CMake 3.20.5 is the minimum recommended for IntelLLVM on Linux
  cmake_minimum_required(VERSION 3.20.5)
endif()

project(dpcpp-blur LANGUAGES CXX)

find_package(IntelDPCPP REQUIRED)
if(NOT IntelDPCPP_FOUND)
  message(STATUS "DPCPP compiler not found, dpcpp-blur build disabled")
  return()
endif()

# if(WIN32) message(FATAL_ERROR "This sample is not supported in Windows")
# endif()

# This sample must be compiled and executed within the oneAPI product enviroment
# with the dpcpp compiler to use dpcpp features

set(TARGET dpcpp-blur)
set(SOURCES src/dpcpp-blur.cpp)

# Set default build type to RelWithDebInfo if not specified
if(NOT CMAKE_BUILD_TYPE)
  message(
    STATUS "Default CMAKE_BUILD_TYPE not set using Release with Debug Info")
  set(CMAKE_BUILD_TYPE
      "RelWithDebInfo"
      CACHE
        STRING
        "Choose build type from: None Debug Release RelWithDebInfo MinSizeRel"
        FORCE)
endif()

add_executable(${TARGET} ${SOURCES})

find_package(VPL REQUIRED)
target_link_libraries(${TARGET} VPL::dispatcher)

if(UNIX)
  set(LIBVA_SUPPORT
      ON
      CACHE BOOL "Enable hardware support.")
  if(LIBVA_SUPPORT)
    find_package(PkgConfig REQUIRED)
    # note: pkg-config version for libva is *API* version
    pkg_check_modules(PKG_LIBVA libva>=1.2 libva-drm>=1.2)
    if(PKG_LIBVA_FOUND)
      target_compile_definitions(${TARGET} PUBLIC -DLIBVA_SUPPORT)
      set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
      set(THREADS_PREFER_PTHREAD_FLAG TRUE)
      find_package(Threads REQUIRED)
      target_link_libraries(${TARGET} ${PKG_LIBVA_LIBRARIES}
                            ${PKG_THREAD_LIBRARIES} ze_loader)
      target_include_directories(${TARGET} PUBLIC ${PKG_LIBVA_INCLUDE_DIRS})
    else()
      message(
        SEND_ERROR
          "libva not found: set LIBVA_SUPPORT=OFF to build ${TARGET} without libva support"
      )
    endif()
  else()
    message(STATUS "Building ${TARGET} without hardware support")
  endif()
  target_compile_options(${TARGET} PRIVATE -Wall -Wextra -pedantic)
endif()

include(CTest)
set(VPL_CONTENT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../content
    CACHE PATH "Path to content.")
if(IMPL_ARG EQUAL -hw)
  set(content_file cars_320x240.nv12)
else()
  set(content_file cars_320x240.i420)
endif()
add_test(NAME ${TARGET}-test
         COMMAND ${TARGET} ${IMPL_ARG} -i "${VPL_CONTENT_DIR}/${content_file}"
                 -w 320 -h 240)
