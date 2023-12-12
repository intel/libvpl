# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)

set(sources "")
list(APPEND sources cttmetrics_sample.cpp)

add_executable(metrics_monitor ${sources})
target_link_libraries(metrics_monitor cttmetrics_static drm)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. /usr/include/drm)

install(TARGETS metrics_monitor RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                        COMPONENT ${VPL_COMPONENT_TOOLS})
