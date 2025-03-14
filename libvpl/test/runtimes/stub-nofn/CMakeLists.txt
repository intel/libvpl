# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# Build stub runtime for testing Intel® Video Processing Library (Intel® VPL)
cmake_minimum_required(VERSION 3.13.0)
set(DLL_PREFIX "lib")
file(STRINGS "../stub/version.txt" version_txt)
project(vplstubrt-nofn VERSION ${version_txt})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(OUTPUT_NAME ${PROJECT_NAME}64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(OUTPUT_NAME ${PROJECT_NAME}32)
endif()

# add lib/<arch> to find_package path on windows
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(CMAKE_LIBRARY_ARCHITECTURE x86)
endif()

add_library(${PROJECT_NAME} SHARED "")

add_definitions(-DSKIP_NEW_FUNCTIONS)

if(WIN32)
  # force libxxx style sharedlib name on Windows
  set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX ${DLL_PREFIX})
  set(SHLIB_FILE_NAME ${DLL_PREFIX}${OUTPUT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(SHLIB_FILE_NAME_DEBUG
      ${DLL_PREFIX}${OUTPUT_NAME}${CMAKE_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}
  )
else()
  set(SHLIB_FILE_NAME
      ${CMAKE_SHARED_LIBRARY_PREFIX}${OUTPUT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}.${PROJECT_VERSION_MAJOR}
  )
endif()

set(IMPLIB_FILE_NAME
    ${CMAKE_IMPORT_LIBRARY_PREFIX}${OUTPUT_NAME}${CMAKE_IMPORT_LIBRARY_SUFFIX})
set(IMPLIB_FILE_NAME_DEBUG
    ${CMAKE_IMPORT_LIBRARY_PREFIX}${OUTPUT_NAME}${CMAKE_DEBUG_POSTFIX}${CMAKE_IMPORT_LIBRARY_SUFFIX}
)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES OUTPUT_NAME ${OUTPUT_NAME} SOVERSION ${PROJECT_VERSION_MAJOR}
             VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

target_sources(${PROJECT_NAME} PRIVATE ../stub/src/stubs.cpp
                                       ../stub/src/config.cpp)

# use .def file without new (experimental) functions exported
if(WIN32)
  target_sources(${PROJECT_NAME} PRIVATE ./src/windows/libvplminrt-nofn.def)
endif()

find_package(VPL 2.2 REQUIRED COMPONENTS api)
message(STATUS "Found Intel® VPL (version ${VPL_VERSION})")
target_link_libraries(${PROJECT_NAME} PUBLIC VPL::api)

target_include_directories(${PROJECT_NAME} PRIVATE ../stub
                                                   ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE -DVERSION_MAJOR=${PROJECT_VERSION_MAJOR}
          -DVERSION_MINOR=${PROJECT_VERSION_MINOR}
          -DVERSION_PATCH=${PROJECT_VERSION_PATCH})

if(UNIX)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS
                                                   -Wl,-Bsymbolic,-z,defs)
endif()
