# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

file(STRINGS "version.txt" version_txt)
project(vpl VERSION ${version_txt})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(TARGET mfxsw64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(TARGET mfxsw32)
endif()

set(SOURCES
    src/cpu_common.cpp
    src/cpu_decode.cpp
    src/cpu_encode.cpp
    src/cpu_vpp.cpp
    src/libmfxvplsw.cpp
    src/libmfxvplsw_caps.cpp
    src/libmfxvplsw_core.cpp
    src/libmfxvplsw_decode.cpp
    src/libmfxvplsw_encode.cpp
    src/libmfxvplsw_vpl.cpp
    src/libmfxvplsw_vpp.cpp)

if(WIN32)
  list(APPEND SOURCES src/windows/libmfxsw.def)
endif()

add_library(${TARGET} SHARED "")

# force correct name on Windows
set_target_properties(${TARGET} PROPERTIES PREFIX "lib")

if(UNIX)
  set_target_properties(${TARGET} PROPERTIES SUFFIX ".so.1")
endif()

target_sources(${TARGET} PRIVATE ${SOURCES})

target_link_libraries(${TARGET} PUBLIC vpl-api)

add_subdirectory(ext/ffmpeg-svt)

target_link_libraries(${TARGET} PRIVATE ffmpeg-svt)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                             ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(
  ${TARGET}
  PRIVATE -DVPL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
          -DVPL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
          -DVPL_VERSION_PATCH=${PROJECT_VERSION_PATCH})

if(UNIX)
  set_target_properties(${TARGET} PROPERTIES LINK_FLAGS -Wl,-Bsymbolic,-z,defs)
endif()

install(
  TARGETS ${TARGET}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev)
