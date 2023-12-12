# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(vpl-inspect src/vpl-inspect.cpp)
find_package(VPL REQUIRED)
target_link_libraries(vpl-inspect PUBLIC VPL::dispatcher)
target_include_directories(vpl-inspect PRIVATE ${ONEVPL_API_HEADER_DIRECTORY})

if(BUILD_TOOLS_ONEVPL_EXPERIMENTAL)
  target_compile_definitions(vpl-inspect PRIVATE -DONEVPL_EXPERIMENTAL)
endif()

install(TARGETS vpl-inspect RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                    COMPONENT ${VPL_COMPONENT_TOOLS})
