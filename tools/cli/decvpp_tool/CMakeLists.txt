# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)
project(decvpp_tool)

if(UNIX)
  find_package(PkgConfig REQUIRED)
  # note: pkg-config version for libva is *API* version
  pkg_check_modules(PKG_LIBVA libva>=1.2 IMPORTED_TARGET GLOBAL)
  pkg_check_modules(PKG_LIBVA_DRM libva-drm>=1.2 IMPORTED_TARGET GLOBAL)
  # Set defaults for ENABLE_VA
  if(PKG_LIBVA_FOUND AND PKG_LIBVA_DRM_FOUND)
    set(VA_DEFAULT ON)
  else()
    set(VA_DEFAULT OFF)
  endif()
  set(ENABLE_VA
      ${VA_DEFAULT}
      CACHE BOOL "Enable VA support in tools.")
endif()

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

set(TARGET decvpp_tool)
set(SOURCES decvpp_tool.cpp)

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

if(BUILD_TOOLS_ONEVPL_EXPERIMENTAL)
  target_compile_definitions(${TARGET} PRIVATE -DONEVPL_EXPERIMENTAL)
endif()

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  if(NOT DEFINED ENV{VSCMD_VER})
    set(CMAKE_MSVCIDE_RUN_PATH $ENV{PATH})
  endif()
endif()

find_package(VPL REQUIRED)
target_link_libraries(${TARGET} VPL::dispatcher)

if(UNIX)
  if(NOT ENABLE_VA)
    message(STATUS "Building ${TARGET} without VA support")
  else()
    if(PKG_LIBVA_FOUND)
      target_compile_definitions(${TARGET} PUBLIC -DLIBVA_SUPPORT)
      set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
      set(THREADS_PREFER_PTHREAD_FLAG TRUE)
      find_package(Threads REQUIRED)
      target_link_libraries(${TARGET} ${PKG_LIBVA_LIBRARIES}
                            ${PKG_LIBVA_DRM_LIBRARIES} ${PKG_THREAD_LIBRARIES})
      target_include_directories(${TARGET} PUBLIC ${PKG_LIBVA_INCLUDE_DIRS})
    else()
      message(
        SEND_ERROR
          "libva not found: set ENABLE_VA=OFF to build tools with VA disabled")
    endif()
  endif()
elseif(WIN32)
  cmake_policy(SET CMP0079 NEW)
  target_link_libraries(${TARGET} d3d11 dxgi)
endif()

install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                  COMPONENT dev)
