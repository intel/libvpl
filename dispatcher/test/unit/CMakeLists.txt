# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

project(${PROJECT_NAME}Tests LANGUAGES CXX)

set(test_sources
    src/session-test.cpp
    src/legacycpp-session-test-1x.cpp
    src/legacycpp-session-test-2x.cpp
    src/main.cpp
    src/dispatcher_common.cpp
    src/dispatcher_common_multiprop.cpp
    src/dispatcher_enum_impls.cpp
    src/dispatcher_gpu.cpp
    src/dispatcher_low_latency.cpp
    src/dispatcher_stub.cpp
    src/dispatcher_sw.cpp
    src/dispatcher_sw_multiprop.cpp
    src/dispatcher_util.cpp
    src/experimental_api.cpp)
add_executable(${PROJECT_NAME} ${test_sources})

find_package(VPL REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC GTest::gtest VPL::dispatcher)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PUBLIC shlwapi.lib)
endif()

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME} PROPERTIES ENVIRONMENT
                     ONEVPL_SEARCH_PATH=$<TARGET_FILE_DIR:vplstubrt>)
