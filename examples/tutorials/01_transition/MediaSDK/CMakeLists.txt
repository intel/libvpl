# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)
project(01_transition_mediasdk)

# Default install places 64 bit runtimes in the environment, so we want to do a
# 64 bit build by default.
if(WIN32)
  if(NOT DEFINED CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM
        x64
        CACHE STRING "")
    message(STATUS "Generator Platform set to ${CMAKE_GENERATOR_PLATFORM}")
  endif()
endif()

set(TARGET 01_transition_mediasdk)
set(SOURCES ../src/transition.cpp)

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

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  if(NOT DEFINED ENV{VSCMD_VER})
    set(CMAKE_MSVCIDE_RUN_PATH $ENV{PATH})
  endif()

  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
  find_package(MFX REQUIRED)
  target_link_libraries(${TARGET} ${MFX_LIBRARIES} legacy_stdio_definitions.lib)
  target_include_directories(${TARGET} PUBLIC ${MFX_INCLUDE_DIRS})
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(PKG_MFX REQUIRED mfx)
  target_link_libraries(${TARGET} ${PKG_MFX_LIBRARIES})
  target_include_directories(${TARGET} PUBLIC ${PKG_MFX_INCLUDE_DIRS})
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
      COMMAND_EXPAND_LISTS)
  endif()
endif()

include(CTest)
add_test(NAME ${TARGET}-test COMMAND ${TARGET})
