# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)
project(val-surface-sharing)

if(NOT BUILD_DISPATCHER_ONEVPL_EXPERIMENTAL)
  message(
    WARNING
      "BUILD_DISPATCHER_ONEVPL_EXPERIMENTAL is set to OFF. val-surface-sharing will not be built."
  )
  return()
endif()

# Default install places 64 bit runtimes in the environment, so we want to do a
# 64 bit build by default.
if(WIN32)
  if(NOT DEFINED CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM
        x64
        CACHE STRING "")
    message(STATUS "Generator Platform set to ${CMAKE_GENERATOR_PLATFORM}")
  endif()
endif()

set(TARGET val-surface-sharing)

add_executable(${TARGET})

target_sources(${TARGET} PRIVATE src/val-surface-sharing.cpp src/decode.cpp
                                 src/vpp.cpp src/encode.cpp src/util.cpp)

if(WIN32)
  target_sources(${TARGET} PRIVATE src/hw-device-d3d11.cpp)
else()
  target_sources(${TARGET} PRIVATE src/hw-device-vaapi.cpp)
endif()

# if toolchain environment is not configured, disable it and print warning
if(TOOLS_ENABLE_OPENCL)
  set(OPENCL_IS_READY TRUE)

  if(DEFINED ENV{OPENCL_INCLUDE_DIR})
    set(OPENCL_INCLUDE_DIR $ENV{OPENCL_INCLUDE_DIR})
  else()
    message(
      WARNING
        "*** OPENCL_INCLUDE_DIR not set - OpenCL surface sharing will be disabled."
    )
    set(OPENCL_IS_READY FALSE)
  endif()

  if(OPENCL_IS_READY AND DEFINED ENV{OPENCL_LIB_DIR})
    set(OPENCL_LIB_DIR $ENV{OPENCL_LIB_DIR})
  else()
    message(
      WARNING
        "*** OPENCL_LIB_DIR not set - OpenCL surface sharing will be disabled.")
    set(OPENCL_IS_READY FALSE)
  endif()

  if(OPENCL_IS_READY)
    target_compile_definitions(${TARGET} PRIVATE -DTOOLS_ENABLE_OPENCL)

    if(WIN32)
      target_sources(${TARGET} PRIVATE src/ src/process-frames-ocl-d3d11.cpp)
    endif()

    target_include_directories(${TARGET} PUBLIC ${OPENCL_INCLUDE_DIR})
    target_link_directories(${TARGET} PUBLIC ${OPENCL_LIB_DIR})
    target_link_libraries(${TARGET} PRIVATE OpenCL)
  endif()
endif()

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Default CMAKE_BUILD_TYPE not set using Release")
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE
        STRING
        "Choose build type from: None Debug Release RelWithDebInfo MinSizeRel"
        FORCE)
endif()

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  if(NOT DEFINED ENV{VSCMD_VER})
    set(CMAKE_MSVCIDE_RUN_PATH $ENV{PATH})
  endif()
endif()

# link VPL import library
find_package(VPL REQUIRED)
target_link_libraries(${TARGET} PRIVATE VPL::dispatcher)

if(WIN32)
  # Windows dependencies
  target_link_libraries(${TARGET} PRIVATE dxgi d3d11)
else()
  # Linux dependencies
  find_package(PkgConfig REQUIRED)

  find_package(Threads REQUIRED)
  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)

  pkg_check_modules(PKG_LIBVA IMPORTED_TARGET libva>=1.2)
  pkg_check_modules(PKG_LIBVA_DRM IMPORTED_TARGET libva-drm>=1.2)

  if(PKG_LIBVA_FOUND)
    target_include_directories(${TARGET} PUBLIC ${PKG_LIBVA_INCLUDE_DIRS})

    target_link_libraries(${TARGET} PRIVATE Threads::Threads)
    target_link_libraries(${TARGET} PRIVATE PkgConfig::PKG_LIBVA
                                            PkgConfig::PKG_LIBVA_DRM)
  else()
    message(SEND_ERROR "Missing packages: libva, libva_drm, x11 are required.")
  endif()
endif()

install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                  COMPONENT dev)
