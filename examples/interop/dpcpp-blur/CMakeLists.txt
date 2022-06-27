# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

if(WIN32)
  message(FATAL_ERROR "This sample is not supported in Windows")
endif()

# This sample must be compiled and executed within the oneAPI product enviroment
# with the dpcpp compiler to use dpcpp features
set(DPCPP_COMPILER_NAME "dpcpp")
find_program(
  DPCPP_COMPILER
  NAMES ${DPCPP_COMPILER_NAME}
  HINTS /opt/intel/oneapi)

if(DPCPP_COMPILER)
  set(CMAKE_CXX_COMPILER ${DPCPP_COMPILER})
  message(STATUS "DPCPP compiler found, sample enabled")

  project(dpcpp-blur LANGUAGES CXX)

  set(CMAKE_CXX_STANDARD 17)

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

else()
  message(STATUS "DPCPP compiler not found, dpcpp-blur build disabled")
endif()
