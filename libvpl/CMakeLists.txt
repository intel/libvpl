# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)
set(TARGET VPL)
set(OUTPUT_NAME "vpl")
set(DLL_PREFIX "lib")

if(WIN32)
  set(SOURCES
      src/windows/main.cpp
      src/windows/mfx_critical_section.cpp
      src/windows/mfx_dispatcher.cpp
      src/windows/mfx_dispatcher_log.cpp
      src/windows/mfx_driver_store_loader.cpp
      src/windows/mfx_dxva2_device.cpp
      src/windows/mfx_function_table.cpp
      src/windows/mfx_library_iterator.cpp
      src/windows/mfx_load_dll.cpp
      src/windows/mfx_win_reg_key.cpp
      src/windows/libmfx.def)
  if(BUILD_SHARED_LIBS)
    configure_file(src/windows/version.rc.in src/windows/version.rc @ONLY)
    list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/src/windows/version.rc)
  endif()
endif()
if(UNIX)
  set(SOURCES src/linux/mfxloader.cpp)

  if(NOT DEFINED MFX_MODULES_DIR)
    set(MFX_MODULES_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
  endif()
  add_definitions(-DMFX_MODULES_DIR="${MFX_MODULES_DIR}")
  message(STATUS "MFX_MODULES_DIR=${MFX_MODULES_DIR}")
endif()

if(BUILD_EXPERIMENTAL)
  add_definitions(-DONEVPL_EXPERIMENTAL)
endif()

list(
  APPEND
  SOURCES
  src/mfx_dispatcher_vpl.cpp
  src/mfx_dispatcher_vpl_loader.cpp
  src/mfx_dispatcher_vpl_config.cpp
  src/mfx_dispatcher_vpl_lowlatency.cpp
  src/mfx_dispatcher_vpl_log.cpp
  src/mfx_dispatcher_vpl_msdk.cpp
  src/mfx_config_interface/mfx_config_interface.cpp
  src/mfx_config_interface/mfx_config_interface_string_api.cpp)

add_library(${TARGET} "")

# disable warnings for dispatcher library itself but not subprojects (unit
# tests)
set_target_properties(${TARGET} PROPERTIES COMPILE_DEFINITIONS
                                           "MFX_DEPRECATED_OFF")

# optionally include the install location of dispatcher dll in the runtime
# search process
if(UNIX)
  if(ENABLE_LIBDIR_IN_RUNTIME_SEARCH)
    file(TO_CMAKE_PATH "${CMAKE_INSTALL_FULL_LIBDIR}" _install_libdir_search)
    target_compile_definitions(
      ${TARGET} PRIVATE INSTALL_LIBDIR_SEARCH=\"${_install_libdir_search}\")
    message(STATUS "Enabled install libdir search: ${_install_libdir_search}")
  endif()
endif()

if(WIN32)
  # force libxxx style sharedlib name on Windows
  if(BUILD_SHARED_LIBS)
    set_target_properties(${TARGET} PROPERTIES PREFIX ${DLL_PREFIX})
  endif()
  set(SHLIB_FILE_NAME ${DLL_PREFIX}${OUTPUT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(SHLIB_FILE_NAME_DEBUG
      ${DLL_PREFIX}${OUTPUT_NAME}${CMAKE_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}
  )

else()
  # use version script on Linux
  set_target_properties(
    ${TARGET}
    PROPERTIES
      LINK_FLAGS
      "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/src/linux/libvpl.map")
  set(SHLIB_FILE_NAME
      ${CMAKE_SHARED_LIBRARY_PREFIX}${OUTPUT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}.${API_VERSION_MAJOR}
  )
  set(SHLIB_FILE_NAME_DEBUG ${SHLIB_FILE_NAME})
endif()

set(LIB_FILE_NAME
    ${CMAKE_STATIC_LIBRARY_PREFIX}${OUTPUT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})
set(IMPLIB_FILE_NAME
    ${CMAKE_IMPORT_LIBRARY_PREFIX}${OUTPUT_NAME}${CMAKE_IMPORT_LIBRARY_SUFFIX})
set(LIB_FILE_NAME_DEBUG
    ${CMAKE_STATIC_LIBRARY_PREFIX}${OUTPUT_NAME}${CMAKE_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}
)
set(IMPLIB_FILE_NAME_DEBUG
    ${CMAKE_IMPORT_LIBRARY_PREFIX}${OUTPUT_NAME}${CMAKE_DEBUG_POSTFIX}${CMAKE_IMPORT_LIBRARY_SUFFIX}
)

set_target_properties(
  ${TARGET}
  PROPERTIES OUTPUT_NAME ${OUTPUT_NAME} SOVERSION ${API_VERSION_MAJOR}
             VERSION ${API_VERSION_MAJOR}.${API_VERSION_MINOR})

target_sources(${TARGET} PRIVATE ${SOURCES})

if(UNIX)
  # require pthreads for loading legacy MSDK runtimes
  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  if(BUILD_SHARED_LIBS)
    target_link_libraries(${TARGET} PRIVATE Threads::Threads)
  else()
    target_link_libraries(${TARGET} PUBLIC Threads::Threads)
  endif()
endif()

target_link_libraries(${TARGET} PUBLIC vpl-api)
if(BUILD_SHARED_LIBS)
  target_link_libraries(${TARGET} PRIVATE ${CMAKE_DL_LIBS})
else()
  target_link_libraries(${TARGET} PUBLIC ${CMAKE_DL_LIBS})
endif()

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                             ${CMAKE_CURRENT_BINARY_DIR})

if(INSTALL_LIB)
  install(
    TARGETS ${TARGET}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT ${VPL_COMPONENT_LIB}
            NAMELINK_SKIP
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${VPL_COMPONENT_LIB})
endif()

if(INSTALL_DEV)
  install(
    TARGETS ${TARGET}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT ${VPL_COMPONENT_DEV}
            NAMELINK_ONLY)
  if(WIN32)
    install(TARGETS ${TARGET} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                                      COMPONENT ${VPL_COMPONENT_DEV})
  endif()

  file(RELATIVE_PATH cmake_rel_prefix ${CMAKE_INSTALL_FULL_LIBDIR}/cmake/vpl
       ${CMAKE_INSTALL_PREFIX})
  file(RELATIVE_PATH cmake_rel_libdir ${CMAKE_INSTALL_FULL_LIBDIR}/cmake/vpl
       ${CMAKE_INSTALL_FULL_LIBDIR})
  file(RELATIVE_PATH cmake_rel_incdir ${CMAKE_INSTALL_FULL_LIBDIR}/cmake/vpl
       ${CMAKE_INSTALL_FULL_INCLUDEDIR})
  file(RELATIVE_PATH cmake_rel_bindir ${CMAKE_INSTALL_FULL_LIBDIR}/cmake/vpl
       ${CMAKE_INSTALL_FULL_BINDIR})

  configure_file(cmake/VPLConfig.cmake.in cmake/VPLConfig.cmake @ONLY)
  configure_file(cmake/VPLConfigVersion.cmake.in cmake/VPLConfigVersion.cmake
                 @ONLY)

  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/cmake/VPLConfig.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/cmake/VPLConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/vpl"
    COMPONENT ${VPL_COMPONENT_DEV})

  file(RELATIVE_PATH pc_rel_prefix ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig
       ${CMAKE_INSTALL_PREFIX})
  file(RELATIVE_PATH pc_rel_libdir ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig
       ${CMAKE_INSTALL_FULL_LIBDIR})
  file(RELATIVE_PATH pc_rel_incdir ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig
       ${CMAKE_INSTALL_FULL_INCLUDEDIR})
  set(pc_rel_prefix "$\{pcfiledir}/${pc_rel_prefix}")
  set(pc_rel_libdir "$\{pcfiledir}/${pc_rel_libdir}")
  set(pc_rel_incdir "$\{pcfiledir}/${pc_rel_incdir}")

  if(CMAKE_DL_LIBS)
    set(DL_LIBS "-l${CMAKE_DL_LIBS}")
  endif()
  if(MINGW)
    # being conservative by restricting to MinGW, these are probably ok for
    # WIN32 in general
    set(MINGW_LIBS "-lole32 -lgdi32 -luuid")
  endif()
  set(VPL_PKGCONFIG_DEPENDENT_LIBS
      "${DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${MINGW_LIBS} ${CXX_LIB}")
  configure_file("pkgconfig/vpl.pc.in" "pkgconfig/vpl.pc" @ONLY)
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/vpl.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
    COMPONENT ${VPL_COMPONENT_DEV})
endif()

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
