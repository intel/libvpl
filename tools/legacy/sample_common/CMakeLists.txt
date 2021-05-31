# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET sample_common)

set(sources "")
list(
  APPEND
  sources
  src/base_allocator.cpp
  src/decode_render.cpp
  src/mfx_buffering.cpp
  src/sysmem_allocator.cpp
  src/general_allocator.cpp
  src/sample_utils.cpp
  src/preset_manager.cpp
  src/parameters_dumper.cpp
  src/vm/atomic.cpp
  src/vm/time.cpp)

if(UNIX)
  list(
    APPEND
    sources
    src/vaapi_utils.cpp
    src/vaapi_device.cpp
    src/vaapi_allocator.cpp
    src/vm/time_linux.cpp
    src/vm/atomic_linux.cpp
    src/vm/thread_linux.cpp
    src/vaapi_utils_drm.cpp)
else()
  list(
    APPEND
    sources
    src/d3d11_allocator.cpp
    src/d3d11_device.cpp
    src/d3d_allocator.cpp
    src/vm/thread_windows.cpp
    src/d3d_device.cpp)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE true)
add_library(${TARGET} STATIC ${sources})
target_include_directories(
  ${TARGET} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_definitions(-DMFX_DEPRECATED_OFF)

if(POLICY CMP0074)
  # ignore warning of VPL_ROOT in find_package search path
  cmake_policy(SET CMP0074 OLD)
endif()

find_package(VPL REQUIRED)
target_link_libraries(${TARGET} VPL::dispatcher)

if(UNIX)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(PKG_LIBVA libva libva-drm)
  pkg_check_modules(PKG_LIBDRM libdrm)
  pkg_check_modules(PKG_WAYLAND_CLIENT wayland-client)
  if(PKG_LIBVA_FOUND)
    target_compile_definitions(${TARGET} PUBLIC -DLIBVA_SUPPORT)
    target_include_directories(${TARGET} PUBLIC ${PKG_LIBVA_INCLUDE_DIRS})
  else()
    message(WARNING "libva not found: building without libVA support")
  endif()
  if(PKG_LIBDRM_FOUND)
    target_compile_definitions(${TARGET} PUBLIC -DLIBVA_DRM_SUPPORT)
    target_include_directories(${TARGET} PUBLIC ${PKG_LIBDRM_INCLUDE_DIRS})
  else()
    message(WARNING "libva-drm not found: building without libVA DRM support")
  endif()
  if(PKG_WAYLAND_CLIENT_FOUND)
    target_compile_definitions(${TARGET} PUBLIC -DLIBVA_WAYLAND_SUPPORT)
    target_include_directories(${TARGET} PUBLIC ${PKG_WAYLAND_CLIENT_LIBRARY_DIRS})
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../sample_misc/wayland/include)
  else()
    message(WARNING "wayland modules not found: building without Wayland support")
  endif()

  target_link_libraries(${TARGET} ${CMAKE_DL_LIBS})

  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  target_link_libraries(${TARGET} Threads::Threads)

else()
  target_compile_definitions(${TARGET} PUBLIC -DMFX_D3D11_SUPPORT)
  target_link_libraries(${TARGET} DXGI D3D11 D3D9 DXVA2)
endif()
