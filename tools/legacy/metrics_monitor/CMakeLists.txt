# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(sources "")
list(APPEND sources src/cttmetrics_i915_pmu.cpp src/cttmetrics.cpp
     src/cttmetrics_i915_custom.cpp src/cttmetrics_utils.cpp)

add_library(cttmetrics_static STATIC ${sources})
add_library(cttmetrics SHARED ${sources})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} /usr/include/drm)

install(TARGETS cttmetrics_static ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                                          COMPONENT dev)

install(TARGETS cttmetrics LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
                                   COMPONENT runtime)

add_subdirectory(sample)
