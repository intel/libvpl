# ##############################################################################
# Copyright (C) 2005 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

find_package(VPL REQUIRED)

add_executable(sample_multi_transcode)

target_sources(
  sample_multi_transcode
  PRIVATE src/pipeline_transcode.cpp src/sample_multi_transcode.cpp
          src/transcode_utils.cpp src/smt_tracer.cpp)

target_link_libraries(sample_multi_transcode PRIVATE sample_common)

target_include_directories(
  sample_multi_transcode PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                                 ${CMAKE_SOURCE_DIR}/api/vpl)

if(BUILD_TOOLS_ONEVPL_EXPERIMENTAL)
  target_compile_definitions(sample_multi_transcode
                             PRIVATE -DONEVPL_EXPERIMENTAL)
endif()

target_compile_definitions(sample_multi_transcode PRIVATE MFX_ONEVPL)

install(TARGETS sample_multi_transcode
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT dev)
