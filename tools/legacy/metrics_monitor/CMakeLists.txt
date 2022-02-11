# ##############################################################################
# Copyright (C) 2017 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

if(NOT CMAKE_SYSTEM_NAME MATCHES Linux)
  return()
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(PKGConfig_LIBDRM libdrm>=2.4.91 IMPORTED_TARGET)

# cttmetrics
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

set(sources
    include/cttmetrics.h include/cttmetrics_utils.h src/cttmetrics.cpp
    src/cttmetrics_i915_custom.cpp src/cttmetrics_i915_pmu.cpp
    src/cttmetrics_utils.cpp)

file(GLOB_RECURSE srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(APPEND sources ${srcs})

if(PKGConfig_LIBDRM_FOUND)
  set(DEPENDENCIES libdrm)

  add_library(cttmetrics SHARED ${sources})

  target_include_directories(cttmetrics PUBLIC include)
  target_link_libraries(cttmetrics PRIVATE PkgConfig::PKGConfig_LIBDRM)

  target_compile_definitions(cttmetrics PRIVATE LIBVA_DRM_SUPPORT LIBVA_SUPPORT)

  # cttmetrics_static
  add_library(cttmetrics_static STATIC ${sources})

  target_include_directories(cttmetrics_static PUBLIC include)
  target_link_libraries(cttmetrics_static PUBLIC PkgConfig::PKGConfig_LIBDRM)

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

else()
  message(WARNING "libdrm not found: skipping build of metrics monitor")
endif()

# test_monitor

pkg_check_modules(PKG_PCIACCESS pciaccess)

if(PKG_PCIACCESS_FOUND AND BUILD_TESTS)

  set(test_srcs test/device_info.h test/i915_pciids.h test/igt_load.c
                test/igt_load.h test/cttmetrics_gtest.cpp test/device_info.c)

  add_executable(test_monitor ${test_srcs})

  if(NOT CMAKE_C_COMPILER_ID MATCHES Intel)
    set_property(TARGET test_monitor
                 PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
  endif()

  target_include_directories(test_monitor PRIVATE ./include)
  target_link_libraries(test_monitor PRIVATE pthread rt gtest cttmetrics
                                             PkgConfig::PKGConfig_LIBDRM)

else()

  if(NOT PKG_PCIACCESS_FOUND)
    message(
      STATUS
        "pciaccess was not found (optional), the following will not be built: test_monitor."
    )
  endif()

endif()
