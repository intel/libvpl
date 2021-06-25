# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

#
# Set installation directories
#

# See https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html for
# variables GNUInstallDirs exposes. This project commonly uses:
# CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_DOCDIR CMAKE_INSTALL_BINDIR
# CMAKE_INSTALL_LIBDIR
include(GNUInstallDirs)

if(NOT ONEAPI_INSTALL_BIN32DIR)
  if(WIN32)
    set(ONEAPI_INSTALL_BIN32DIR "${CMAKE_INSTALL_BINDIR}/x86")
  else()
    set(ONEAPI_INSTALL_BIN32DIR bin32)
  endif()
endif()

if(NOT ONEAPI_INSTALL_LIB32DIR)
  if(WIN32)
    set(ONEAPI_INSTALL_LIB32DIR "${CMAKE_INSTALL_LIBDIR}/x86")
  else()
    set(ONEAPI_INSTALL_LIB32DIR lib32)
  endif()
endif()

if(WIN32 AND "${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
  set(CMAKE_INSTALL_BINDIR ${ONEAPI_INSTALL_BIN32DIR})
  set(CMAKE_INSTALL_LIBDIR ${ONEAPI_INSTALL_LIB32DIR})
endif()

if(NOT ONEAPI_INSTALL_ENVDIR)
  set(ONEAPI_INSTALL_ENVDIR ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/env)
endif()

if(NOT ONEAPI_INSTALL_SYSCHECKDIR)
  set(ONEAPI_INSTALL_SYSCHECKDIR
      ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/sys_check)
endif()
if(NOT ONEAPI_INSTALL_MODFILEDIR)
  set(ONEAPI_INSTALL_MODFILEDIR
      ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/modulefiles)
endif()
if(NOT ONEAPI_INSTALL_EXAMPLEDIR)
  set(ONEAPI_INSTALL_EXAMPLEDIR
      ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/examples)
endif()
if(NOT ONEAPI_INSTALL_LICENSEDIR)
  set(ONEAPI_INSTALL_LICENSEDIR ${CMAKE_INSTALL_DOCDIR})
endif()
if(NOT ONEAPI_INSTALL_PYTHONDIR)
  set(ONEAPI_INSTALL_PYTHONDIR
      ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/python/lib)
endif()

foreach(
  dir
  BIN32DIR
  LIB32DIR
  ENVDIR
  SYSCHECKDIR
  MODFILEDIR
  EXAMPLEDIR
  LICENSEDIR
  PYTHONDIR)
  gnuinstalldirs_get_absolute_install_dir(ONEAPI_INSTALL_FULL_${dir}
                                          ONEAPI_INSTALL_${dir} ${dir})
endforeach()
