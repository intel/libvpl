# ##############################################################################
# Copyright (C) 2005 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

if(NOT PKG_WAYLAND_CLIENT_FOUND)
  message(
    SEND_ERROR
      "wayland-client not found: set BUILD_TOOLS=OFF to skip building tools")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(PKGConfig_LIBDRM libdrm>=2.4.91 IMPORTED_TARGET)

if(PKGConfig_LIBDRM_FOUND)
  add_library(vpl_wayland SHARED)

  target_include_directories(
    vpl_wayland PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                        ${CMAKE_SOURCE_DIR}/api/vpl)

  target_sources(
    vpl_wayland
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/class_wayland.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/listener_wayland.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/wayland-drm-protocol.c)

  if(WAYLAND_SCANNER_BIN
     AND PKG_WAYLAND_PROTCOLS_FOUND
     AND WAYLAND_LINUX_DMABUF_XML_PATH)
    execute_process(
      COMMAND
        "${WAYLAND_SCANNER_BIN_PATH}\/${WAYLAND_SCANNER_BIN}" "client-header"
        "${WAYLAND_LINUX_DMABUF_XML_PATH}"
        "tools/legacy/sample_misc/wayland/linux-dmabuf-unstable-v1.h"
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
      RESULT_VARIABLE WAYLAND_SCANNER_RESULT)
    if(WAYLAND_SCANNER_RESULT)
      message(FATAL_ERROR "Failed to generate linux-dmabuf-unstable-v1.h")
    endif()

    execute_process(
      COMMAND
        "${WAYLAND_SCANNER_BIN_PATH}\/${WAYLAND_SCANNER_BIN}" "private-code"
        "${WAYLAND_LINUX_DMABUF_XML_PATH}"
        "tools/legacy/sample_misc/wayland/linux-dmabuf-unstable-v1.c"
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
      RESULT_VARIABLE WAYLAND_SCANNER_RESULT)
    if(WAYLAND_SCANNER_RESULT)
      message(FATAL_ERROR "Failed to generate linux-dmabuf-unstable-v1.c")
    endif()

    target_include_directories(
      vpl_wayland PRIVATE ${CMAKE_BINARY_DIR}/tools/legacy/sample_misc/wayland)
    target_sources(
      vpl_wayland
      PRIVATE
        ${CMAKE_BINARY_DIR}/tools/legacy/sample_misc/wayland/linux-dmabuf-unstable-v1.c
    )
  else()
    message(
      SEND_ERROR
        "Wayland modules not found: set BUILD_TOOLS=OFF to skip building tools")
  endif()

  target_link_libraries(vpl_wayland PRIVATE sample_common wayland-client va drm
                                            drm_intel)

  install(TARGETS vpl_wayland LIBRARY DESTINATION ${_TOOLS_LIB_PATH})
else()
  message(
    SEND_ERROR "libdrm not found: set BUILD_TOOLS=OFF to skip building tools")
endif()
