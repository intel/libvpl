# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)
project(vpl-infer)

# Default install places 64 bit runtimes in the environment, so we want to do a
# 64 bit build by default.
if(WIN32)
  if(NOT DEFINED CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM
        x64
        CACHE STRING "")
    message(STATUS "Generator Platform set to ${CMAKE_GENERATOR_PLATFORM}")
  endif()

  if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    if(NOT DEFINED ENV{VSCMD_VER})
      set(CMAKE_MSVCIDE_RUN_PATH $ENV{PATH})
    endif()
  endif()
endif()

set(TARGET vpl-infer)
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

# Set default build type to RelWithDebInfo if not specified
if(NOT CMAKE_BUILD_TYPE)
  message(
    STATUS "Default CMAKE_BUILD_TYPE not set using Release with Debug Info")
  set(CMAKE_BUILD_TYPE
      "RelWithDebInfo"
      CACHE
        STRING
        "Choose build type from: None Debug Release RelWithDebInfo MinSizeRel"
        FORCE)
endif()

add_executable(${TARGET} ${SOURCES})

find_package(VPL REQUIRED)
target_link_libraries(${TARGET} PRIVATE VPL::dispatcher)

find_package(OpenVINO COMPONENTS Runtime REQUIRED)
target_link_libraries(${TARGET} PRIVATE openvino::runtime)

if(WIN32)
  find_package(OpenCL CONFIG)
else()
  find_package(OpenCL)
endif()
if(OpenCL_FOUND)
  target_link_libraries(${TARGET} PRIVATE OpenCL::OpenCL)
  add_definitions(-DZEROCOPY)
  message(STATUS "Building ${TARGET} with zerocopy support")
else()
  message(STATUS "Building ${TARGET} WITHOUT zerocopy support")
endif()

if(UNIX)
  include(CheckIncludeFileCXX)
  set(LIBVA_SUPPORT
      ON
      CACHE BOOL "Enable hardware support.")
  if(LIBVA_SUPPORT)
    find_package(PkgConfig REQUIRED)
    # note: pkg-config version for libva is *API* version

    pkg_check_modules(PKG_LIBVA libva>=1.2)
    if(PKG_LIBVA_FOUND)
      target_compile_definitions(${TARGET} PUBLIC -DLIBVA_SUPPORT)
      target_link_libraries(${TARGET} PRIVATE ${PKG_LIBVA_LIBRARIES})
      target_include_directories(${TARGET} PUBLIC ${PKG_LIBVA_INCLUDEDIR})
    else()
      message(FATAL_ERROR "libva not found")
    endif()

    pkg_check_modules(PKG_LIBVADRM libva-drm>=1.2)
    if(PKG_LIBVADRM_FOUND)
      target_compile_definitions(${TARGET} PUBLIC -DLIBVA_SUPPORT)
      target_link_libraries(${TARGET} PRIVATE ${PKG_LIBVADRM_LIBRARIES})
      target_include_directories(${TARGET} PUBLIC ${PKG_LIBVADRM_INCLUDEDIR})
    else()
      message(FATAL_ERROR "libva-drm not found")
    endif()
  else()
    message(STATUS "Building ${TARGET} without hardware support")
  endif()
endif()

# copy dependent dlls to target location
if(WIN32)
  if(${CMAKE_VERSION} VERSION_LESS "3.26")
    message(
      STATUS
        "CMake Version less than 3.26, unable to copy dependent DLLs to target location"
    )
  else()
    add_custom_command(
      TARGET ${TARGET}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${TARGET}>
              $<TARGET_RUNTIME_DLLS:${TARGET}>
      COMMAND ${CMAKE_COMMAND} -E remove
              $<TARGET_FILE_DIR:${TARGET}>/openvino.dll
      COMMAND_EXPAND_LISTS)
  endif()
endif()

include(CTest)

set(VPL_EXAMPLE_MODEL
    "vehicle-detection-0200.xml"
    CACHE PATH "Model to use.")
set(VPL_CONTENT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../content
    CACHE PATH "Path to content.")

add_test(
  NAME downloadModelXml
  COMMAND
    curl -LJO
    "https://storage.openvinotoolkit.org/repositories/open_model_zoo/2023.0/models_bin/1/vehicle-detection-0200/FP16/vehicle-detection-0200.xml"
)
add_test(
  NAME downloadModelBin
  COMMAND
    curl -LJO
    "https://storage.openvinotoolkit.org/repositories/open_model_zoo/2023.0/models_bin/1/vehicle-detection-0200/FP16/vehicle-detection-0200.bin"
)
set_tests_properties(downloadModelXml PROPERTIES FIXTURES_SETUP DL)
set_tests_properties(downloadModelBin PROPERTIES FIXTURES_SETUP DL)

if("${TEST_ZERO_COPY}")
  add_test(NAME ${TARGET}-zerocopy-test
           COMMAND ${TARGET} -i "${VPL_CONTENT_DIR}/cars_320x240.h265"
                   -zerocopy -m "${VPL_EXAMPLE_MODEL}")

  set_tests_properties(${TARGET}-zerocopy-test PROPERTIES FIXTURES_REQUIRED DL)
endif()
if("${TEST_LEGACY_API}")
  add_test(NAME ${TARGET}-legacyapi-test
           COMMAND ${TARGET} -i "${VPL_CONTENT_DIR}/cars_320x240.h265" -legacy
                   -m "${VPL_EXAMPLE_MODEL}")

  set_tests_properties(${TARGET}-legacyapi-test PROPERTIES FIXTURES_REQUIRED DL)
endif()
