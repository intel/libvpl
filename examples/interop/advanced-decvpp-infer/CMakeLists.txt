# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)
project(advanced-decvpp-infer)

# Default install places 64 bit runtimes in the environment, so we want to do a
# 64 bit build by default.
if(WIN32)
  message(FATAL_ERROR "Windows not supported by this sample.")
endif()

set(TARGET advanced-decvpp-infer)
set(SOURCES src/advanced-decvpp-infer.cpp src/util.h)
set(CONTENTPATH ${CMAKE_CURRENT_SOURCE_DIR}/../../content)
set(RUNARGS -i ${CONTENTPATH}/cars_320x240.h265 -m
            ${CONTENTPATH}/public/mobilenet-ssd/FP32/mobilenet-ssd.xml)

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

    pkg_check_modules(PKG_LIBVA libva>=1.2)
    if(PKG_LIBVA_FOUND)
      target_compile_definitions(${TARGET} PUBLIC -DLIBVA_SUPPORT)
      target_link_libraries(${TARGET} ${PKG_LIBVA_LIBRARIES})
      target_include_directories(${TARGET} PUBLIC ${PKG_LIBVA_INCLUDEDIR})
    else()
      message(
        SEND_ERROR
          "libva not found: set LIBVA_SUPPORT=OFF to build ${TARGET} without libva support"
      )
    endif()

    pkg_check_modules(PKG_LIBVADRM libva-drm>=1.2)
    if(PKG_LIBVADRM_FOUND)
      target_compile_definitions(${TARGET} PUBLIC -DLIBVA_SUPPORT)
      target_link_libraries(${TARGET} ${PKG_LIBVADRM_LIBRARIES})
      target_include_directories(${TARGET} PUBLIC ${PKG_LIBVADRM_INCLUDEDIR})
    else()
      message(
        SEND_ERROR
          "libva-drm not found: set LIBVA_SUPPORT=OFF to build ${TARGET} without libva support"
      )
    else()
      message(STATUS "Building ${TARGET} without hardware support")
    endif()
  endif()

  find_package(InferenceEngine REQUIRED)
  if(InferenceEngine_FOUND)
    target_link_libraries(${TARGET} ${InferenceEngine_LIBRARIES})
    target_include_directories(${TARGET} PUBLIC ${InferenceEngine_INCLUDE_DIRS})
  else()
    message(SEND_ERROR "InferenceEngine not found")
  endif()

  find_package(OpenCL)
  if(OpenCL_FOUND)
    target_include_directories(${TARGET} PUBLIC ${OpenCL_INCLUDE_DIRS})
  else()
    message(
      "OpenCL not found with find_package(OpenCL), using backup approach to find OpenCL library."
    )
    find_path(
      OpenCL_LIBRARY_PATH libOpenCL.so.1
      PATHS $ENV{ONEAPI_ROOT}/compiler/latest/linux/lib
            /usr/lib/x86_64-linux-gnu
      NO_DEFAULT_PATH)
    if(OpenCL_LIBRARY_PATH)
      set(OpenCL_LIBRARIES ${OpenCL_LIBRARY_PATH}/libOpenCL.so.1)
      set(OpenCL_FOUND true)
    endif()

  endif()

  if(OpenCL_FOUND)
    message("using OpenCL library ${OpenCL_LIBRARIES}")
    target_link_libraries(${TARGET} ${OpenCL_LIBRARIES})
  else()
    message(SEND_ERROR "OpenCL not found")
  endif()

endif()

get_directory_property(has_parent PARENT_DIRECTORY)
if(NOT has_parent)
  # only make run target available for stand-alone build
  add_custom_target(run ${TARGET} ${RUNARGS})
endif()
