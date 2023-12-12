# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.13.0)

file(RELATIVE_PATH env_rel_prefix "${VPL_INSTALL_FULL_ENVDIR}"
     "${CMAKE_INSTALL_PREFIX}")
# remove trailing slash
string(REGEX REPLACE "\/$" "" env_rel_prefix "${env_rel_prefix}")

# convert to native path
file(TO_NATIVE_PATH "${env_rel_prefix}" env_rel_prefix)
file(TO_NATIVE_PATH "${CMAKE_INSTALL_INCLUDEDIR}" _install_includedir)
file(TO_NATIVE_PATH "${CMAKE_INSTALL_LIBDIR}" _install_libdir)
file(TO_NATIVE_PATH "${CMAKE_INSTALL_BINDIR}" _install_bindir)
file(TO_NATIVE_PATH "${VPL_INSTALL_EXAMPLEDIR}" _install_exampledir)

if(WIN32)
  install(
    PROGRAMS "cmd/vars.bat"
    DESTINATION "${VPL_INSTALL_ENVDIR}"
    COMPONENT ${VPL_COMPONENT_DEV})

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ENV_SCRIPT_NAME "vars64.bat")
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ENV_SCRIPT_NAME "vars32.bat")
  endif()
  configure_file("cmd/varsxx.bat.in" "${ENV_SCRIPT_NAME}" @ONLY)

  install(
    PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${ENV_SCRIPT_NAME}"
    DESTINATION "${VPL_INSTALL_ENVDIR}"
    COMPONENT ${VPL_COMPONENT_DEV})
else()
  file(TO_CMAKE_PATH "${env_rel_prefix}" env_rel_prefix)
  file(TO_CMAKE_PATH "${CMAKE_INSTALL_INCLUDEDIR}" _install_includedir)
  file(TO_CMAKE_PATH "${CMAKE_INSTALL_LIBDIR}" _install_libdir)
  file(TO_CMAKE_PATH "${CMAKE_INSTALL_BINDIR}" _install_bindir)
  file(TO_CMAKE_PATH "${VPL_INSTALL_PKGCONFIGDIR}" _install_pkgconfigdir)
  set(ENV_SCRIPT_NAME "vars.sh")
  configure_file("sh/vars.sh.in" "${ENV_SCRIPT_NAME}" @ONLY)
  install(
    PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${ENV_SCRIPT_NAME}"
    DESTINATION "${VPL_INSTALL_ENVDIR}"
    COMPONENT ${VPL_COMPONENT_DEV})
endif()
