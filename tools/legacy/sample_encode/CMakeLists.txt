# ##############################################################################
# Copyright (C) 2005 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

find_package(VPL REQUIRED)

if(POLICY CMP0074)
  # ignore warning of VPL_ROOT in find_package search path
  cmake_policy(SET CMP0074 OLD)
endif()

if(CMAKE_SYSTEM_NAME MATCHES Linux)
  if(NOT (PKG_LIBVA_FOUND AND PKG_LIBVA_DRM_FOUND))
    message("Skipping sample_encode build.")
    return()
  endif()
endif()

add_executable(sample_encode)

target_sources(
  sample_encode PRIVATE src/pipeline_encode.cpp src/pipeline_region_encode.cpp
                        src/pipeline_user.cpp src/sample_encode.cpp)

target_include_directories(sample_encode
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(sample_encode PRIVATE sample_common)

if(MSVC)
  target_compile_definitions(sample_encode PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS sample_encode RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                      COMPONENT dev)
