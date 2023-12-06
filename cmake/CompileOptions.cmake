# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

#
# Set compilation options
#
if(MSVC)
  add_compile_options("$<$<CONFIG:Debug>:/D_DEBUG>")
else()
  add_compile_options("$<$<CONFIG:Debug>:-D_DEBUG -O0 -g>")
endif()

if(ENABLE_WARNING_AS_ERROR)
  message(STATUS "Warnings as errors enabled")
  set(MFX_DEPRECATED_OFF 1)
endif()

if(DEFINED ENV{MFX_DEPRECATED_OFF})
  set(MFX_DEPRECATED_OFF 1)
endif()

if(MFX_DEPRECATED_OFF)
  message(STATUS "Deprecation warnings disabled")
  add_definitions(-DMFX_DEPRECATED_OFF)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
  add_link_options("/guard:cf")
  add_link_options("/DYNAMICBASE")
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    add_link_options("/HIGHENTROPYVA")
  endif()
  add_link_options("/LARGEADDRESSAWARE")
  add_link_options("/NXCOMPAT")
  if(ENABLE_WARNING_AS_ERROR)
    add_compile_options("/WX")
  endif()
  add_compile_options("/GS")
  add_compile_options("/guard:cf")
else()
  if(UNIX)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")
  endif()
  add_compile_options("-Wformat")
  add_compile_options("-Wformat-security")
  add_compile_options("-Werror=format-security")
  if(NOT MINGW)
    string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
    if(NOT CMAKE_BUILD_TYPE MATCHES debug)
      add_definitions("-D_FORTIFY_SOURCE=2")
    endif()
  endif()
  if(NOT MINGW)
    add_compile_options("-fstack-protector-strong")
  endif()
  if(NOT MINGW)
    add_link_options("-Wl,-z,relro,-z,now,-z,noexecstack")
  endif()
  add_compile_options("-Wall")
  if(ENABLE_WARNING_AS_ERROR)
    add_compile_options("-Werror")
  endif()
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
