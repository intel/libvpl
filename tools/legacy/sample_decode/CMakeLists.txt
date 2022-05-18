# ##############################################################################
# Copyright (C) 2005 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

find_package(VPL REQUIRED)

add_executable(sample_decode)

target_sources(sample_decode PRIVATE src/pipeline_decode.cpp
                                     src/sample_decode.cpp)

target_include_directories(sample_decode
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(sample_decode PRIVATE sample_common)

if(BUILD_TOOLS_ONEVPL_EXPERIMENTAL)
  target_compile_definitions(sample_decode PRIVATE -DONEVPL_EXPERIMENTAL)
endif()

if(MSVC)
  target_compile_definitions(sample_decode PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS sample_decode RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                      COMPONENT dev)
