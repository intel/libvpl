# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET VPL)

if(WIN32)
  set(SOURCES
      windows/main.cpp
      windows/mfx_critical_section.cpp
      windows/mfx_dispatcher.cpp
      windows/mfx_dispatcher_log.cpp
      windows/mfx_driver_store_loader.cpp
      windows/mfx_dxva2_device.cpp
      windows/mfx_function_table.cpp
      windows/mfx_library_iterator.cpp
      windows/mfx_load_dll.cpp
      windows/mfx_win_reg_key.cpp
      windows/libmfx.def)
endif()

if(UNIX)
  set(SOURCES linux/mfxloader.cpp)

  if(NOT DEFINED MFX_MODULES_DIR)
    set(MFX_MODULES_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
  endif()
  add_definitions(-DMFX_MODULES_DIR="${MFX_MODULES_DIR}")
  message(STATUS "MFX_MODULES_DIR=${MFX_MODULES_DIR}")
endif()

add_definitions(-DMFX_DEPRECATED_OFF)

list(APPEND SOURCES vpl/mfx_dispatcher_vpl.cpp
     vpl/mfx_dispatcher_vpl_loader.cpp vpl/mfx_dispatcher_vpl_config.cpp
     vpl/mfx_dispatcher_vpl_msdk.cpp)

add_library(${TARGET} SHARED "")

if(WIN32)
  # force libxxx style name on Windows
  set_target_properties(${TARGET} PROPERTIES PREFIX "lib")
else()
  # use version script on Linux
  set_target_properties(
    ${TARGET}
    PROPERTIES
      LINK_FLAGS
      "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/linux/libvpl.map")
endif()

set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME "vpl")
set_target_properties(
  ${TARGET} PROPERTIES VERSION ${API_VERSION_MAJOR}.${API_VERSION_MINOR})
set_target_properties(${TARGET} PROPERTIES SOVERSION ${API_VERSION_MAJOR})

target_sources(${TARGET} PRIVATE ${SOURCES})

if(UNIX)
  # require pthreads for loading legacy MSDK runtimes
  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  target_link_libraries(
    ${TARGET}
    PUBLIC vpl-api
    PRIVATE ${CMAKE_DL_LIBS} Threads::Threads)
else()
  target_link_libraries(
    ${TARGET}
    PUBLIC vpl-api
    PRIVATE ${CMAKE_DL_LIBS})
endif()

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                             ${CMAKE_CURRENT_BINARY_DIR})

install(
  TARGETS ${TARGET}
  # EXPORT mfxTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev)

# export(PACKAGE VPL)

configure_file(cmake/VPLConfig.cmake.in cmake/VPLConfig.cmake @ONLY)
configure_file(cmake/VPLConfigVersion.cmake.in cmake/VPLConfigVersion.cmake
               @ONLY)
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/cmake/VPLConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/VPLConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/vpl"
  COMPONENT dev)

configure_file("pkgconfig/vpl.pc.in" "pkgconfig/vpl.pc" @ONLY)
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/vpl.pc"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
  COMPONENT dev)
