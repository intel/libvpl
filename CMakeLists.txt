# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) main build script
if(MSVC)
  cmake_minimum_required(VERSION 3.15)
  cmake_policy(SET CMP0091 NEW)
else()
  cmake_minimum_required(VERSION 3.10.2)
endif()

file(STRINGS "version.txt" version_txt)
project(vpl VERSION ${version_txt})

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#
# Project configuration options
#

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Default CMAKE_BUILD_TYPE not set using Release")
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE
        STRING
        "Choose build type from: None Debug Release RelWithDebInfo MinSizeRel"
        FORCE)
endif()

# More code hardening on Windows
if(MSVC)
  if(${use_control_flow_guard})
    message("Setting CONTROL FLOW GUARD")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /guard:cf")
    set(CMAKE_EXE_LINKER_FLAGS "/guard:cf /DYNAMICBASE")
  endif()
  if(${use_qspectre})
    message("Setting QSPECTRE")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Qspectre")
  endif()
endif()

# Project options
option(BUILD_SHARED_LIBS "Build shared instead of static libraries." ON)
option(BUILD_TESTS "Build tests." OFF)

set(BUILD_DEV
    ON
    CACHE BOOL "Build files that are only needed for development.")
set(BUILD_DISPATCHER
    ON
    CACHE BOOL "Build dispatcher.")
set(BUILD_TOOLS
    ON
    CACHE BOOL "Build tools.")
set(BUILD_EXAMPLES
    OFF
    CACHE BOOL "Build examples when building dev package.")
set(BUILD_PREVIEW
    ON
    CACHE BOOL "Build preview features.")
set(INSTALL_EXAMPLE_CODE
    ON
    CACHE BOOL "Install examples when installing dev package.")

set(BUILD_DISPATCHER_ONEVPL_EXPERIMENTAL
    ON
    CACHE BOOL "Build dispatcher with ONEVPL_EXPERIMENTAL APIs.")

set(BUILD_TOOLS_ONEVPL_EXPERIMENTAL
    ON
    CACHE BOOL "Build tools with ONEVPL_EXPERIMENTAL APIs.")

option(BUILD_DISPATCHER_ONLY "Build dispatcher only." OFF)
option(BUILD_DEV_ONLY "Build only developer package." OFF)

if(BUILD_DISPATCHER_ONLY)
  set(BUILD_DEV OFF)
  set(BUILD_PREVIEW OFF)
  set(BUILD_TOOLS OFF)
  set(BUILD_TOOLS_ONEVPL_EXPERIMENTAL OFF)
  set(INSTALL_EXAMPLE_CODE OFF)
  set(BUILD_DISPATCHER ON)

endif()
if(BUILD_DEV_ONLY)
  set(BUILD_DISPATCHER OFF)
  set(BUILD_TOOLS OFF)
endif()

option(USE_MSVC_STATIC_RUNTIME
       "Link MSVC runtime statically to all components." OFF)

if(USE_MSVC_STATIC_RUNTIME)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

option(USE_ONEAPI_INSTALL_LAYOUT "Use oneAPI install layout instead of FHS" OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

include(cmake/oneAPIInstallDirs.cmake)

# Set POSITION_INDEPENDENT_CODE property for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE true)

# Keep executable bits on shared objects when installing regardless of distro
set(CMAKE_INSTALL_SO_NO_EXE 0)

include(cmake/CompileOptions.cmake)

if(BUILD_TESTS)
  include(Dart)
  enable_testing()
endif()

add_subdirectory(api)
add_subdirectory(dispatcher)
if(BUILD_TOOLS)
  add_subdirectory(tools)
endif()
if(BUILD_DEV)
  add_subdirectory(env)
  add_subdirectory(modulefiles)
  add_subdirectory(examples)
endif()

install(
  FILES third-party-programs.txt
  DESTINATION ${ONEAPI_INSTALL_LICENSEDIR}
  COMPONENT license)

install(
  FILES LICENSE
  RENAME license.txt
  DESTINATION ${ONEAPI_INSTALL_LICENSEDIR}
  COMPONENT license)

if(USE_ONEAPI_INSTALL_LAYOUT)
  install(
    FILES LICENSE
    RENAME license.txt
    DESTINATION "licensing"
    COMPONENT license)
endif()

include(cmake/PackageTarget.cmake)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT runtime)
include(InstallRequiredSystemLibraries)

message(
  STATUS "---------------- Configuration summary ------------------------------"
)
message(STATUS "CMake:")
message(STATUS "  CMAKE_VERSION                   : ${CMAKE_VERSION}")
message(STATUS "  CMAKE_GENERATOR                 : ${CMAKE_GENERATOR}")
message(STATUS "  CMAKE_BUILD_TOOL                : ${CMAKE_BUILD_TOOL}")
message(STATUS "Target:")
message(STATUS "  CMAKE_SYSTEM_NAME               : ${CMAKE_SYSTEM_NAME}")
message(STATUS "  CMAKE_SYSTEM_VERSION            : ${CMAKE_SYSTEM_VERSION}")
message(STATUS "  CMAKE_SYSTEM_PROCESSOR          : ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "General:")
message(STATUS "  CMAKE_BUILD_TYPE                : ${CMAKE_BUILD_TYPE}")
message(STATUS "  CMAKE_TOOLCHAIN_FILE            : ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "  CMAKE_C_COMPILER                : ${CMAKE_C_COMPILER}")
message(STATUS "  CMAKE_CXX_COMPILER              : ${CMAKE_CXX_COMPILER}")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(STATUS "  Build architecture              : 64-bit")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(STATUS "  Build architecture              : 32-bit")
endif()
message(STATUS "Flags:")
message(STATUS "  CMAKE_C_FLAGS                   : ${CMAKE_C_FLAGS}")
message(STATUS "  CMAKE_CXX_FLAGS                 : ${CMAKE_CXX_FLAGS}")
message(STATUS "Release flags:")
message(STATUS "  CMAKE_C_FLAGS_RELEASE           : ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "  CMAKE_CXX_FLAGS_RELEASE         : ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "Debug flags:")
message(STATUS "  CMAKE_C_FLAGS_DEBUG             : ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "  CMAKE_CXX_FLAGS_DEBUG           : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "RelWithDebRT flags:")
message(
  STATUS "  CMAKE_C_FLAGS_RELWITHDEBRT      : ${CMAKE_C_FLAGS_RELWITHDEBRT}")
message(
  STATUS "  CMAKE_CXX_FLAGS_RELWITHDEBRT    : ${CMAKE_CXX_FLAGS_RELWITHDEBRT}")
message(STATUS "Install:")
message(STATUS "  CMAKE_INSTALL_PREFIX            : ${CMAKE_INSTALL_PREFIX}")
message(
  STATUS "  CMAKE_INSTALL_FULL_BINDIR       : ${CMAKE_INSTALL_FULL_BINDIR}")
message(
  STATUS "  CMAKE_INSTALL_FULL_INCLUDEDIR   : ${CMAKE_INSTALL_FULL_INCLUDEDIR}")
message(
  STATUS "  CMAKE_INSTALL_FULL_LIBDIR       : ${CMAKE_INSTALL_FULL_LIBDIR}")
message(
  STATUS "  CMAKE_INSTALL_FULL_DOCDIR       : ${CMAKE_INSTALL_FULL_DOCDIR}")
message(
  STATUS
    "  ONEAPI_INSTALL_FULL_PKGCONFIGDIR: ${ONEAPI_INSTALL_FULL_PKGCONFIGDIR}")
message(
  STATUS "  ONEAPI_INSTALL_FULL_CMAKEDIR    : ${ONEAPI_INSTALL_FULL_CMAKEDIR}")
message(
  STATUS "  ONEAPI_INSTALL_FULL_ENVDIR      : ${ONEAPI_INSTALL_FULL_ENVDIR}")
message(
  STATUS "  ONEAPI_INSTALL_FULL_MODFILEDIR  : ${ONEAPI_INSTALL_FULL_MODFILEDIR}"
)
message(
  STATUS "  ONEAPI_INSTALL_FULL_EXAMPLEDIR  : ${ONEAPI_INSTALL_FULL_EXAMPLEDIR}"
)
message(
  STATUS "  ONEAPI_INSTALL_FULL_LICENSEDIR  : ${ONEAPI_INSTALL_FULL_LICENSEDIR}"
)
message(
  STATUS "  ONEAPI_INSTALL_FULL_PYTHONDIR   : ${ONEAPI_INSTALL_FULL_PYTHONDIR}")
message(STATUS "Build:")
message(
  STATUS "  BUILD_DISPATCHER_ONLY                : ${BUILD_DISPATCHER_ONLY}")
message(STATUS "  BUILD_DEV                            : ${BUILD_DEV}")
message(STATUS "  BUILD_DISPATCHER                     : ${BUILD_DISPATCHER}")
message(STATUS "  BUILD_TOOLS                          : ${BUILD_TOOLS}")
if(CMAKE_SYSTEM_NAME MATCHES Linux)
  message(STATUS "  ENABLE_VA                            : ${ENABLE_VA}")
  message(STATUS "  ENABLE_DRM                           : ${ENABLE_DRM}")
  message(STATUS "  ENABLE_WAYLAND                       : ${ENABLE_WAYLAND}")
  message(STATUS "  ENABLE_X11                           : ${ENABLE_X11}")
  message(STATUS "  ENABLE_DRI3                          : ${ENABLE_DRI3}")
endif()
message(STATUS "  BUILD_SHARED_LIBS                    : ${BUILD_SHARED_LIBS}")
message(STATUS "  BUILD_TESTS                          : ${BUILD_TESTS}")
message(STATUS "  BUILD_EXAMPLES                       : ${BUILD_EXAMPLES}")
message(STATUS "  BUILD_PREVIEW                        : ${BUILD_PREVIEW}")
message(
  STATUS
    "  BUILD_DISPATCHER_ONEVPL_EXPERIMENTAL : ${BUILD_DISPATCHER_ONEVPL_EXPERIMENTAL}"
)
message(
  STATUS
    "  BUILD_TOOLS_ONEVPL_EXPERIMENTAL      : ${BUILD_TOOLS_ONEVPL_EXPERIMENTAL}"
)
message(
  STATUS "  INSTALL_EXAMPLE_CODE                 : ${INSTALL_EXAMPLE_CODE}")

if(MSVC)
  message(
    STATUS "  USE_MSVC_STATIC_RUNTIME              : ${USE_MSVC_STATIC_RUNTIME}"
  )
endif()
