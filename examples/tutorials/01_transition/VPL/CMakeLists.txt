# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)
project(01_transition_vpl)

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

set(TARGET 01_transition_vpl)
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
endif()

option(USE_VPL2X_INIT "Compile with MFXLoad instead of MFXInit" OFF)

find_package(VPL REQUIRED)
target_link_libraries(${TARGET} VPL::dispatcher)
target_compile_definitions(${TARGET} PUBLIC -DUSE_VPL_INCLUDE)
if(USE_VPL2X_INIT)
  target_compile_definitions(${TARGET} PUBLIC -DUSE_VPL2X_INIT)
else()
  target_compile_definitions(${TARGET} PUBLIC -DMFX_DEPRECATED_OFF)
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
