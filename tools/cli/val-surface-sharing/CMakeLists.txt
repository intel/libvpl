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

target_compile_definitions(${TARGET} PRIVATE -DONEVPL_EXPERIMENTAL)

target_sources(${TARGET} PRIVATE src/val-surface-sharing.cpp src/decode.cpp
                                 src/vpp.cpp src/encode.cpp src/util.cpp)

if(WIN32)
  target_sources(${TARGET} PRIVATE src/hw-device-d3d11.cpp)
else()
  target_sources(${TARGET} PRIVATE src/hw-device-vaapi.cpp)
endif()

# if OpenCL is not configured, disable use of it in this tool and print warning
find_package(OpenCL CONFIG)
if(NOT OpenCL_FOUND)
  message(
    WARNING "*** OpenCL not found. OpenCL surface sharing will be disabled.")
  set(TOOLS_ENABLE_OPENCL OFF)
endif()

# OpenCL is currently supported on Windows only for this tool
if(TOOLS_ENABLE_OPENCL)
  if(WIN32)
    target_compile_definitions(${TARGET} PRIVATE -DTOOLS_ENABLE_OPENCL)
    target_sources(${TARGET} PRIVATE src/process-frames-ocl-d3d11.cpp)
    target_link_libraries(${TARGET} PRIVATE OpenCL::OpenCL)
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

# link import library
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
                                  COMPONENT ${VPL_COMPONENT_TOOLS})
