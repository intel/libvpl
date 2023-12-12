# ##############################################################################
# Copyright (C) 2005 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

find_package(VPL REQUIRED)

add_executable(sample_multi_transcode)

target_sources(
  sample_multi_transcode
  PRIVATE src/pipeline_transcode.cpp src/sample_multi_transcode.cpp
          src/smt_cli.cpp src/smt_tracer.cpp src/main.cpp)

target_link_libraries(sample_multi_transcode PRIVATE sample_common)

target_include_directories(
  sample_multi_transcode PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                                 ${CMAKE_SOURCE_DIR}/api/vpl)

if(BUILD_TOOLS_ONEVPL_EXPERIMENTAL)
  target_compile_definitions(sample_multi_transcode
                             PRIVATE -DONEVPL_EXPERIMENTAL)
endif()

target_compile_definitions(sample_multi_transcode PRIVATE MFX_ONEVPL)

install(TARGETS sample_multi_transcode
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                COMPONENT ${VPL_COMPONENT_TOOLS})

if(BUILD_TESTS)
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

  add_executable(sample_multi_transcode_test)
  target_sources(
    sample_multi_transcode_test
    PRIVATE src/pipeline_transcode.cpp src/sample_multi_transcode.cpp
            src/smt_cli.cpp src/smt_tracer.cpp test/test_main.cpp)

  target_link_libraries(sample_multi_transcode_test PUBLIC GTest::gtest)
  target_link_libraries(sample_multi_transcode_test PRIVATE sample_common)
  target_include_directories(
    sample_multi_transcode_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                                        ${CMAKE_SOURCE_DIR}/api/vpl)

  if(BUILD_TOOLS_ONEVPL_EXPERIMENTAL)
    target_compile_definitions(sample_multi_transcode_test
                               PRIVATE -DONEVPL_EXPERIMENTAL)
  endif()

  target_compile_definitions(sample_multi_transcode_test PRIVATE MFX_ONEVPL)

  include(GoogleTest)
  gtest_discover_tests(sample_multi_transcode_test)

endif()
