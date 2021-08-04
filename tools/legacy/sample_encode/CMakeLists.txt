# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET sample_encode)
set(SOURCES "")
list(APPEND SOURCES src/pipeline_encode.cpp src/${TARGET}.cpp)

find_package(VPL REQUIRED)

if(POLICY CMP0074)
  # ignore warning of VPL_ROOT in find_package search path
  cmake_policy(SET CMP0074 OLD)
endif()

if(UNIX)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(PKG_LIBVA libva>=1.2 libva-drm>=1.2)
  pkg_check_modules(PKG_LIBDRM libdrm)
  pkg_check_modules(PKG_X11 x11)
  pkg_check_modules(PKG_LIBWAYLAND libwayland)

  if(NOT
     (PKG_LIBVA_FOUND
      AND (PKG_LIBDRM_FOUND
           OR PKG_X11_FOUND
           OR PKG_LIBWAYLAND_FOUND)))

    message(
      "Skipping sample_encode build.  Requires libva + DRM, X11, or Wayland backend."
    )
    return()
  endif()
endif()

add_executable(${TARGET} ${SOURCES})
target_include_directories(${TARGET}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${TARGET} sample_common)

if(MSVC)
  target_compile_definitions(${TARGET} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                  COMPONENT dev)
