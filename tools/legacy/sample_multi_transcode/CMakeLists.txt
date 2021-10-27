# ##############################################################################
# Copyright (C) 2005 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

if(POLICY CMP0074)
  # ignore warning of VPL_ROOT in find_package search path
  cmake_policy(SET CMP0074 OLD)
endif()

find_package(VPL REQUIRED)

if(CMAKE_SYSTEM_NAME MATCHES Linux)
  if(NOT
     (PKG_LIBVA_FOUND
      AND PKG_LIBVA_DRM_FOUND
      AND (PKG_LIBDRM_FOUND
           OR PKG_X11_FOUND
           OR PKG_LIBWAYLAND_FOUND)))
    message(
      "Skipping sample_multi_transcode build.  Requires libva + DRM, X11, or Wayland backend."
    )
    return()
  endif()
endif()

add_executable(sample_multi_transcode)

target_sources(
  sample_multi_transcode
  PRIVATE src/pipeline_transcode.cpp src/sample_multi_transcode.cpp
          src/transcode_utils.cpp)

target_link_libraries(sample_multi_transcode PRIVATE sample_common)

target_include_directories(
  sample_multi_transcode PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                                 ${CMAKE_SOURCE_DIR}/api/vpl)

target_compile_definitions(sample_multi_transcode PRIVATE MFX_ONEVPL)

install(TARGETS sample_multi_transcode
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT dev)
