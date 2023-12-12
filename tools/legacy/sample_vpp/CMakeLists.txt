# ##############################################################################
# Copyright (C) 2005 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

set(TARGET sample_vpp)
set(SOURCES "")
list(
  APPEND
  SOURCES
  src/main.cpp
  src/sample_vpp.cpp
  src/sample_vpp_config.cpp
  src/sample_vpp_frc.cpp
  src/sample_vpp_frc_adv.cpp
  src/sample_vpp_parser.cpp
  src/sample_vpp_pts.cpp
  src/sample_vpp_roi.cpp
  src/sample_vpp_utils.cpp)

find_package(VPL REQUIRED)

add_executable(${TARGET} ${SOURCES})
target_include_directories(${TARGET}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${TARGET} PRIVATE sample_common)

if(MSVC)
  target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
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

  add_executable(sample_vpp_test)

  target_sources(
    sample_vpp_test
    PRIVATE test/test_main.cpp
            src/sample_vpp.cpp
            src/sample_vpp_config.cpp
            src/sample_vpp_frc.cpp
            src/sample_vpp_frc_adv.cpp
            src/sample_vpp_parser.cpp
            src/sample_vpp_pts.cpp
            src/sample_vpp_roi.cpp
            src/sample_vpp_utils.cpp)

  target_link_libraries(sample_vpp_test PUBLIC GTest::gtest)
  target_link_libraries(sample_vpp_test PRIVATE sample_common)
  target_include_directories(
    sample_vpp_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                            ${CMAKE_SOURCE_DIR}/api/vpl)

  if(BUILD_TOOLS_ONEVPL_EXPERIMENTAL)
    target_compile_definitions(sample_vpp_test PRIVATE -DONEVPL_EXPERIMENTAL)
  endif()

  target_compile_definitions(sample_vpp_test PRIVATE MFX_ONEVPL)

  include(GoogleTest)
  gtest_discover_tests(sample_vpp_test)

endif()
