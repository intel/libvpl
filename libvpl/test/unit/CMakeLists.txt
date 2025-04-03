# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)

set(TARGET vpl-tests)
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
    src/dispatcher_gpu_stringapi.cpp
    src/dispatcher_stub_stringapi.cpp
    src/dispatcher_stub_propquery.cpp
    src/experimental_api.cpp)
add_executable(${TARGET} ${test_sources})

find_package(VPL REQUIRED)
target_link_libraries(${TARGET} PUBLIC GTest::gtest VPL::dispatcher)

# Include the stub runtime src dir so we can load the stub RT capabilities to
# check test results. Because of how relative paths are used in the caps*.h
# files, we can't just hardcode this path in the test code.
target_include_directories(
  ${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../runtimes/stub)

if(WIN32)
  target_link_libraries(${TARGET} PUBLIC shlwapi.lib)
endif()

include(GoogleTest)
# note that RESOURCE_LOCK prevents running any discoveded tests in parallel
gtest_discover_tests(
  ${TARGET} PROPERTIES ENVIRONMENT
  ONEVPL_SEARCH_PATH=$<TARGET_FILE_DIR:vplstubrt> RESOURCE_LOCK
  DISPATCHER_LOG_FILE)
