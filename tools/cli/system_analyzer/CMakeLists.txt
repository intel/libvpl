# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)
project(system_analyzer)

if(UNIX)
  find_package(PkgConfig REQUIRED)
  # note: pkg-config version for libva is *API* version
  pkg_check_modules(PKG_LIBVA libva>=1.2 IMPORTED_TARGET GLOBAL)
  pkg_check_modules(PKG_LIBVA_DRM libva-drm>=1.2 IMPORTED_TARGET GLOBAL)
  # Set defaults for ENABLE_VA
  if(PKG_LIBVA_FOUND AND PKG_LIBVA_DRM_FOUND)
    set(VA_DEFAULT ON)
  else()
    message(STATUS "libva not found.  Skipping system_analyzer.")
    return()
  endif()
  set(ENABLE_VA
      ${VA_DEFAULT}
      CACHE BOOL "Enable VA support in tools.")
else()
  message(STATUS "system_analyzer not supported on Windows")
  return()
endif()

set(TARGET system_analyzer)
set(SOURCES system_analyzer.cpp)

add_executable(${TARGET} ${SOURCES})

if(BUILD_TOOLS_ONEVPL_EXPERIMENTAL)
  target_compile_definitions(${TARGET} PRIVATE -DONEVPL_EXPERIMENTAL)
endif()

find_package(VPL REQUIRED)
target_link_libraries(${TARGET} VPL::dispatcher dl)

if(UNIX)
  if(PKG_LIBVA_FOUND)
    target_compile_definitions(${TARGET} PUBLIC -DLIBVA_SUPPORT)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET} PkgConfig::PKG_LIBVA
                          PkgConfig::PKG_LIBVA_DRM Threads::Threads)
    target_include_directories(${TARGET} PUBLIC ${PKG_LIBVA_INCLUDE_DIRS})
  endif()
endif()

install(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                  COMPONENT dev)
