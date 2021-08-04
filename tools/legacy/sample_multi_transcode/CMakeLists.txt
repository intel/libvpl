# ##############################################################################
# Copyright (C) 2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET sample_multi_transcode)

if(POLICY CMP0074)
  # ignore warning of VPL_ROOT in find_package search path
  cmake_policy(SET CMP0074 OLD)
endif()

find_package(VPL REQUIRED)

if(UNIX)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(PKG_LIBVA libva>=1.2 libva-drm>=1.2)
  pkg_check_modules(PKG_LIBDRM libdrm)
  if(PKG_LIBVA_FOUND AND PKG_LIBDRM_FOUND)
    add_executable(
      ${TARGET} src/pipeline_transcode.cpp src/sample_multi_transcode.cpp
                src/transcode_utils.cpp)
    target_link_libraries(${TARGET} ${PKG_LIBVA_LIBRARIES} ${CMAKE_DL_LIBS}
                          sample_common media_sdk_compatibility_headers pthread)
    add_definitions(-DLIBVA_SUPPORT -DLIBVA_DRM_SUPPORT -DLINUX64 -DMFX_ONEVPL)
    target_include_directories(
      ${TARGET}
      PRIVATE ${PKG_LIBVA_INCLUDE_DIRS} ${PKG_LIBDRM_INCLUDE_DIRS}
              ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/api/vpl)
    install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                      COMPONENT dev)
  else()
    message("Skipping sample_multi_transcode build.  Requires libva/libva-drm.")
  endif()
else()
  add_executable(
    ${TARGET} src/pipeline_transcode.cpp src/sample_multi_transcode.cpp
              src/transcode_utils.cpp)
  target_include_directories(
    ${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                      ${CMAKE_SOURCE_DIR}/api/vpl)
  target_link_libraries(
    ${TARGET}
    DXGI
    D3D11
    D3D9
    DXVA2
    sample_common
    media_sdk_compatibility_headers)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -DMFX_D3D11_SUPPORT -DMFX_ONEVPL)

  install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                    COMPONENT dev)
endif()
