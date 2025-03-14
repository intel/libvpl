# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)

add_subdirectory(runtimes/stub)
add_subdirectory(runtimes/stub1x)
add_subdirectory(runtimes/stub-nofn)

# Build googletest
set(BUILD_SHARED_LIBS OFF)

set(BUILD_GMOCK
    OFF
    CACHE BOOL "" FORCE)
set(INSTALL_GTEST
    OFF
    CACHE BOOL "" FORCE)
set(gtest_disable_pthreads
    OFF
    CACHE BOOL "" FORCE)
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
set(gtest_hide_internal_symbols
    OFF
    CACHE BOOL "" FORCE)

#
add_subdirectory(ext/googletest)
add_library(GTest::gtest ALIAS gtest)
add_library(GTest::gtest_main ALIAS gtest_main)

add_subdirectory(diagnostic)
add_subdirectory(unit)
