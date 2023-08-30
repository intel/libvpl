# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)
set(DLL_PREFIX "lib")
file(STRINGS "version.txt" version_txt)
project(vplstubrt VERSION ${version_txt})

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

target_sources(${PROJECT_NAME} PRIVATE src/stubs.cpp src/config.cpp)

if(WIN32)
  target_sources(${PROJECT_NAME} PRIVATE src/windows/libvplminrt.def)
endif()

find_package(VPL 2.2 REQUIRED COMPONENTS api)
message(STATUS "Found VPL (version ${VPL_VERSION})")
target_link_libraries(${PROJECT_NAME} PUBLIC VPL::api)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
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

# install( TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
# COMPONENT runtime RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT
# runtime)