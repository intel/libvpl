# ##############################################################################
# Copyright (C) 2017 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

if(NOT CMAKE_SYSTEM_NAME MATCHES Linux)
  return()
endif()

find_package(PkgConfig REQUIRED)
# note: pkg-config version for libva is *API* version
pkg_check_modules(PKG_LIBVA libva>=1.2 IMPORTED_TARGET GLOBAL)
pkg_check_modules(PKG_LIBVA_DRM libva-drm>=1.2 IMPORTED_TARGET GLOBAL)
pkg_check_modules(PKG_LIBDRM libdrm>=2.4.91 IMPORTED_TARGET)
pkg_check_modules(PKG_PCIACCESS pciaccess)
# Set defaults for ENABLE_VA
if(PKG_LIBVA_FOUND AND PKG_LIBVA_DRM_FOUND)
  set(VA_DEFAULT ON)
else()
  set(VA_DEFAULT OFF)
endif()
set(ENABLE_VA
    ${VA_DEFAULT}
    CACHE BOOL "Enable VA support in tools.")

# Set default for ENABLE_DRM
if(VA_DEFAULT AND PKG_LIBDRM_FOUND)
  set(DRM_DEFAULT ON)
else()
  set(DRM_DEFAULT OFF)
endif()
set(ENABLE_DRM
    ${DRM_DEFAULT}
    CACHE BOOL "Enable VA/DRM in tools.")

# Error if options conflict
if(NOT ENABLE_VA AND ENABLE_DRM)
  message(
    SEND_ERROR "conflicting options: DRM backend requested but ENABLE_VA=OFF")
endif()

# cttmetrics
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

set(sources
    include/cttmetrics.h include/cttmetrics_utils.h src/cttmetrics.cpp
    src/cttmetrics_i915_custom.cpp src/cttmetrics_i915_pmu.cpp
    src/cttmetrics_utils.cpp)

file(GLOB_RECURSE srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(APPEND sources ${srcs})

if(PKG_LIBDRM_FOUND)
  set(DEPENDENCIES libdrm)

  add_library(cttmetrics SHARED ${sources})

  target_include_directories(cttmetrics PUBLIC include)
  target_link_libraries(cttmetrics PRIVATE PkgConfig::PKG_LIBDRM)

  target_compile_definitions(cttmetrics PRIVATE LIBVA_DRM_SUPPORT LIBVA_SUPPORT)

  # cttmetrics_static
  add_library(cttmetrics_static STATIC ${sources})

  target_include_directories(cttmetrics_static PUBLIC include)
  target_link_libraries(cttmetrics_static PUBLIC PkgConfig::PKG_LIBDRM)

  target_compile_definitions(cttmetrics_static PRIVATE LIBVA_DRM_SUPPORT
                                                       LIBVA_SUPPORT)

  # metrics_monitor
  set(sources sample/cttmetrics_sample.cpp)
  file(GLOB_RECURSE srcs "${CMAKE_CURRENT_SOURCE_DIR}/sample/*.cpp")

  add_executable(metrics_monitor ${sources})
  set_property(TARGET metrics_monitor PROPERTY FOLDER "samples")
  if(NOT CMAKE_C_COMPILER_ID MATCHES Intel)
    set_property(TARGET metrics_monitor
                 PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
  endif()

  target_link_libraries(metrics_monitor PUBLIC cttmetrics)

  target_compile_definitions(metrics_monitor PRIVATE LIBVA_DRM_SUPPORT
                                                     LIBVA_SUPPORT)

  # test_monitor
  if(BUILD_TESTS)
    if(PKG_PCIACCESS_FOUND)

      set(test_srcs
          test/device_info.h test/i915_pciids.h test/igt_load.c test/igt_load.h
          test/cttmetrics_gtest.cpp test/device_info.c)

      add_executable(test_monitor ${test_srcs})

      if(NOT CMAKE_C_COMPILER_ID MATCHES Intel)
        set_property(TARGET test_monitor
                     PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
      endif()

      target_include_directories(test_monitor PRIVATE ./include)
      target_link_libraries(test_monitor PRIVATE pthread rt gtest cttmetrics
                                                 PkgConfig::PKG_LIBDRM)

    else()
      message(
        SEND_ERROR
          "pciaccess not found: set BUILD_TESTS=OFF to skip building test_monitor"
      )
    endif()
  endif()
else()
  message(STATUS "libdrm not found: skipping build of metrics_monitor")
endif()
