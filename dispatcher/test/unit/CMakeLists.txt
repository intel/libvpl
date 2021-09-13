# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

project(${PROJECT_NAME}Tests LANGUAGES CXX)

set(test_sources src/session-test.cpp src/low-latency.cpp)
add_executable(${PROJECT_NAME} ${test_sources})

find_package(VPL REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC GTest::gtest GTest::gtest_main
                                             VPL::dispatcher)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME} PROPERTIES ENVIRONMENT
                     ONEVPL_SEARCH_PATH=$<TARGET_FILE_DIR:vplstubrt>)
