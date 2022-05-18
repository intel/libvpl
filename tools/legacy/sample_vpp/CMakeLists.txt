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

add_executable(${TARGET} ${SOURCES})
target_include_directories(${TARGET}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${TARGET} PRIVATE sample_common)

if(MSVC)
  target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                  COMPONENT dev)
