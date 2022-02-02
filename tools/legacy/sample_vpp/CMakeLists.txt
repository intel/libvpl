# ##############################################################################
# Copyright (C) 2005 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

set(TARGET sample_vpp)
set(SOURCES "")
list(
  APPEND
  SOURCES
  src/sample_vpp.cpp
  src/sample_vpp_config.cpp
  src/sample_vpp_frc.cpp
  src/sample_vpp_frc_adv.cpp
  src/sample_vpp_parser.cpp
  src/sample_vpp_pts.cpp
  src/sample_vpp_roi.cpp
  src/sample_vpp_utils.cpp)

find_package(VPL REQUIRED)

if(POLICY CMP0074)
  # ignore warning of VPL_ROOT in find_package search path
  cmake_policy(SET CMP0074 OLD)
endif()

if(CMAKE_SYSTEM_NAME MATCHES Linux)
  if(NOT
     (PKG_LIBDRM_FOUND
      AND PKG_LIBVA_FOUND
      AND PKG_LIBVA_DRM_FOUND))
    if(NOT PKG_LIBDRM_FOUND)
      message("libdrm not found: skipping build of sample_encode")
    endif()

    if(NOT PKG_LIBVA_FOUND)
      message("libva not found: skipping build of sample_encode")
    endif()

    if(NOT PKG_LIBVA_DRM_FOUND)
      message("libvadrm not found: skipping build of sample_encode")
    endif()
    return()
  endif()
endif()

add_executable(${TARGET} ${SOURCES})
target_include_directories(${TARGET}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${TARGET} PRIVATE sample_common)

if(MSVC)
  target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                  COMPONENT dev)
