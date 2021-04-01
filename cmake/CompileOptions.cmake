# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

#
# Set compilation options
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-D_DEBUG)
  if(UNIX)
    set(CMAKE_CXX_FLAGS "-O0 -g ${CMAKE_CXX_FLAGS}")
  endif(UNIX)
endif()

if(MSVC)
  add_link_options("/DYNAMICBASE")
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    add_link_options("/HIGHENTROPYVA")
  endif()
  add_link_options("/LARGEADDRESSAWARE")
  add_link_options("/NXCOMPAT")
  add_compile_options("/GS")
else()
  add_compile_options("-Wformat")
  add_compile_options("-Wformat-security")
  add_compile_options("-Werror=format-security")
  add_definitions("-D_FORTIFY_SOURCE=2")
  add_compile_options("-fstack-protector-strong")
  set(CMAKE_CXX_FLAGS "-z relro -z now -z noexecstack ${CMAKE_CXX_FLAGS}")
  add_compile_options("-Wall")
endif()
