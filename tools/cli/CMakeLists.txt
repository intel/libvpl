# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(system_analyzer)

add_executable(vpl-inspect vpl-inspect.cpp)
target_link_libraries(vpl-inspect VPL)
target_include_directories(vpl-inspect PRIVATE ${ONEVPL_API_HEADER_DIRECTORY})

if(BUILD_TOOLS_ONEVPL_EXPERIMENTAL)
  target_compile_definitions(vpl-inspect PRIVATE -DONEVPL_EXPERIMENTAL)
endif()

install(TARGETS vpl-inspect RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                    COMPONENT dev)
