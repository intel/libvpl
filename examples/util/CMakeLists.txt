# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET vpl-example-util)

add_library(${TARGET} INTERFACE)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(PKG_LIBVA libva libva-drm)
  if(PKG_LIBVA_FOUND)
    target_compile_definitions(${TARGET} INTERFACE -DLIBVA_SUPPORT)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET} INTERFACE ${PKG_LIBVA_LIBRARIES}
                                              Threads::Threads)
    target_include_directories(${TARGET} INTERFACE ${PKG_LIBVA_INCLUDE_DIRS})
  else()
    message(WARNING "libva not found: building without libVA support")
  endif()
endif()
