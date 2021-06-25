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
  src/plugin_utils.cpp
  src/preset_manager.cpp
  src/parameters_dumper.cpp
  src/vpl_implementation_loader.cpp
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
    src/vaapi_utils_x11.cpp
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
target_link_libraries(${TARGET} VPL::dispatcher media_sdk_compatibility_headers)

if(UNIX)
  find_package(PkgConfig REQUIRED)
  # note: pkg-config version for libva is *API* version
  pkg_check_modules(PKG_LIBVA libva>=1.2 libva-drm>=1.2)
  pkg_check_modules(PKG_LIBDRM libdrm)
  pkg_check_modules(PKG_X11 x11)
  pkg_check_modules(PKG_LIBVA_X11 libva-x11>=1.10.0)
  if(PKG_LIBVA_FOUND)
    target_compile_definitions(${TARGET} PUBLIC -DLIBVA_SUPPORT)
    target_compile_definitions(${TARGET} PUBLIC -DLIBVA_DRM_SUPPORT)
    target_include_directories(${TARGET} PUBLIC ${PKG_LIBVA_INCLUDE_DIRS})
  else()
    message(WARNING "libva not found: building without libVA support")
  endif()
  if(PKG_LIBDRM_FOUND)
    target_include_directories(${TARGET} PUBLIC ${PKG_LIBDRM_INCLUDE_DIRS})
  else()
    message(WARNING "libva-drm not found: building without libVA DRM support")
  endif()
  if(PKG_X11_FOUND)
    target_compile_definitions(${TARGET} PUBLIC -DLIBVA_X11_SUPPORT)
    target_include_directories(${TARGET} PUBLIC ${PKG_X11_LIBRARY_DIRS})
  else()
    message(WARNING "x11 modules not found: building without libVA X11 support")
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
