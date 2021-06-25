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

# Define RelWithDebRT Build Mode
if(MSVC)
  set(CMAKE_DEBUG_POSTFIX "d")
  set(CMAKE_RELWITHDEBRT_POSTFIX "d")
  set(CMAKE_CXX_FLAGS_RELWITHDEBRT
      "${CMAKE_CXX_FLAGS_RELEASE}"
      CACHE STRING "Flags used by the C++ compiler during RelWithDebRT builds."
            FORCE)
  set(CMAKE_C_FLAGS_RELWITHDEBRT
      "${CMAKE_C_FLAGS_RELEASE}"
      CACHE STRING "Flags used by the C compiler during RelWithDebRT builds."
            FORCE)

  foreach(build_flag CMAKE_CXX_FLAGS_RELWITHDEBRT CMAKE_C_FLAGS_RELWITHDEBRT)
    string(REPLACE "/MDd" "" ${build_flag} "${${build_flag}}")
    string(REPLACE "/MTd" "" ${build_flag} "${${build_flag}}")
    string(REPLACE "/MD" "" ${build_flag} "${${build_flag}}")
    string(REPLACE "/MT" "" ${build_flag} "${${build_flag}}")
    set(${build_flag} "${${build_flag}} /MDd /DEBUG:NONE")
  endforeach()

  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBRT
      "${CMAKE_EXE_LINKER_FLAGS_RELEASE}"
      CACHE STRING
            "Flags used for linking binaries during RelWithDebRT builds." FORCE)
  set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBRT
      "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}"
      CACHE
        STRING
        "Flags used by the shared libraries linker during RelWithDebRT builds."
        FORCE)
  set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBRT
      "${CMAKE_MODULE_LINKER_FLAGS_RELEASE}"
      CACHE STRING "Flags used by the linker during RelWithDebRT builds." FORCE)
  mark_as_advanced(
    CMAKE_CXX_FLAGS_RELWITHDEBRT CMAKE_C_FLAGS_RELWITHDEBRT
    CMAKE_EXE_LINKER_FLAGS_RELWITHDEBRT CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBRT)

  # Add RelWithDebRT to avalible config types
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES};RelWithDebRT")

endif()
