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
endif()

if(MSVC)
  add_link_options("/DYNAMICBASE")
  add_link_options("/HIGHENTROPYVA")
  add_link_options("/LARGEADDRESSAWARE")
  add_link_options("/NXCOMPAT")
  add_compile_options("/GS")
else()
  add_compile_options("-Wformat")
  add_compile_options("-Wformat-security")
  add_compile_options("-Werror=format-security")
  add_definitions("-D_FORTIFY_SOURCE=2")
  add_compile_options("-fstack-protector-strong")
  set(CMAKE_CXX_FLAGS "-z relro -z now -z noexecstack")
  add_compile_options("-Wall")
endif()
