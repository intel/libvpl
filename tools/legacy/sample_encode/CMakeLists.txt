# ##############################################################################
# Copyright (C) 2005 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

find_package(VPL REQUIRED)

add_executable(sample_encode)

target_sources(
  sample_encode PRIVATE src/pipeline_encode.cpp src/pipeline_region_encode.cpp
                        src/pipeline_user.cpp src/sample_encode.cpp)

target_include_directories(sample_encode
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(sample_encode PRIVATE sample_common)

if(BUILD_TOOLS_ONEVPL_EXPERIMENTAL)
  target_compile_definitions(sample_encode PRIVATE -DONEVPL_EXPERIMENTAL)
endif()

if(MSVC)
  target_compile_definitions(sample_encode PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS sample_encode RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                      COMPONENT dev)
