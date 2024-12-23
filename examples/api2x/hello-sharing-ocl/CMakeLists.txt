# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)
project(hello-sharing-ocl)

# Enable experimental mode
add_definitions(-DONEVPL_EXPERIMENTAL)

# Default install places 64 bit runtimes in the environment, so we want to do a
# 64 bit build by default.
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(CMAKE_LIBRARY_ARCHITECTURE x86)
endif()

if(WIN32)
  if(NOT DEFINED CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM
        x64
        CACHE STRING "")
    message(STATUS "Generator Platform set to ${CMAKE_GENERATOR_PLATFORM}")
  endif()
endif()

find_package(OpenCL CONFIG REQUIRED)

set(OCL_TARGETS hello-sharing-ocl-export hello-sharing-ocl-import)
foreach(target IN LISTS OCL_TARGETS)
  set(TARGET ${target})
  set(SOURCES src/${target}.cpp src/device-d3d11.cpp src/device-ocl.cpp
              src/util.cpp)
  add_executable(${target} ${SOURCES})
  target_include_directories(${target}
                             PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(${target} PRIVATE OpenCL::OpenCL)

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

  if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    if(NOT DEFINED ENV{VSCMD_VER})
      set(CMAKE_MSVCIDE_RUN_PATH $ENV{PATH})
    endif()
  endif()

  find_package(VPL REQUIRED)
  target_link_libraries(${target} PRIVATE VPL::dispatcher)
  if(WIN32)
    # Windows dependencies
    target_link_libraries(${target} PRIVATE dxgi d3d11)
  endif()

  if(UNIX)
    set(LIBVA_SUPPORT
        ON
        CACHE BOOL "Enable hardware support.")
    if(LIBVA_SUPPORT)
      find_package(PkgConfig REQUIRED)
      # note: pkg-config version for libva is *API* version
      pkg_check_modules(PKG_LIBVA IMPORTED_TARGET libva>=1.2)
      pkg_check_modules(PKG_LIBVA_DRM IMPORTED_TARGET libva-drm>=1.2)
      if(PKG_LIBVA_FOUND)
        target_compile_definitions(${target} PUBLIC -DLIBVA_SUPPORT)
        set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
        set(THREADS_PREFER_PTHREAD_FLAG TRUE)
        find_package(Threads REQUIRED)
        target_link_libraries(${target} PkgConfig::PKG_LIBVA
                              PkgConfig::PKG_LIBVA_DRM Threads::Threads)
        target_include_directories(${target} PUBLIC ${PKG_LIBVA_INCLUDE_DIRS})
      else()
        message(
          SEND_ERROR
            "libva not found: set LIBVA_SUPPORT=OFF to build ${TARGET} without libva support"
        )
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
        COMMAND_EXPAND_LISTS)
    endif()
  endif()
endforeach()

include(CTest)
set(VPL_CONTENT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../content
    CACHE PATH "Path to content.")
set(OCL_CONTENT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../libvpl/test/vpl-import-export/ocl_kernels
    CACHE PATH "Path to OCL content.")

# Import test
add_test(NAME hello-sharing-ocl-import-test
         COMMAND hello-sharing-ocl-import -i
                 "${VPL_CONTENT_DIR}/cars_320x240.nv12" -w 320 -h 240)
set(passRegex "Encoded 30 frames" "^All ok")
set_property(TEST hello-sharing-ocl-import-test PROPERTY PASS_REGULAR_EXPRESSION
                                                         "${passRegex}")

# Export test
add_test(NAME hello-sharing-ocl-export-test
         COMMAND hello-sharing-ocl-export -i
                 "${VPL_CONTENT_DIR}/cars_320x240.h265")
set(passRegex "Decoded 30 frames" "^All ok")
set_property(TEST hello-sharing-ocl-export-test PROPERTY PASS_REGULAR_EXPRESSION
                                                         "${passRegex}")
