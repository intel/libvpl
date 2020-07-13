# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
  message(WARNING "Doxygen not found. Documentation will not be generated")
  return()
endif()

set(DOXYGEN_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/developer-reference)

set(DOXYGEN_STRIP_FROM_PATH ${PROJECT_SOURCE_DIR})
set(DOXYGEN_MULTILINE_CPP_IS_BRIEF "YES")
set(DOXYGEN_QUIET "YES")
set(DOXYGEN_WARN_IF_UNDOCUMENTED "YES")
set(DOXYGEN_WARN_AS_ERROR "NO")
set(DOXYGEN_EXAMPLE_PATH ${PROJECT_SOURCE_DIR}/examples)
set(DOXYGEN_EXAMPLE_RECURSIVE "YES")

doxygen_add_docs(doc ${PROJECT_SOURCE_DIR}/api
                 ${PROJECT_SOURCE_DIR}/doc/developer-reference)
