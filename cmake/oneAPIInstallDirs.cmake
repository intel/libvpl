# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

#
# Set installation directories
#

if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  if(NOT CMAKE_INSTALL_BINDIR)
    set(BINARCH x86)
  endif()
  if(NOT CMAKE_INSTALL_LIBDIR)
    set(LIBARCH x86)
  endif()
endif()

if(USE_ONEAPI_INSTALL_LAYOUT)
  set(CMAKE_INSTALL_DOCDIR "documentation")
  if(NOT WIN32)
    set(CMAKE_INSTALL_LIBDIR "lib")
  endif()
  set(ONEAPI_INSTALL_ENVDIR "env")
  set(ONEAPI_INSTALL_MODFILEDIR "modulefiles")
  set(ONEAPI_INSTALL_EXAMPLEDIR "examples")
  set(ONEAPI_INSTALL_LICENSEDIR "licensing/${PROJECT_NAME}")
  set(ONEAPI_INSTALL_PYTHONDIR "python/lib")
else()
  set(CMAKE_INSTALL_DOCDIR ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/doc)
endif()

# See https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html for
# variables GNUInstallDirs exposes. This project commonly uses:
# CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_DOCDIR CMAKE_INSTALL_BINDIR
# CMAKE_INSTALL_LIBDIR
include(GNUInstallDirs)

if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(CMAKE_INSTALL_BINDIR
      ${CMAKE_INSTALL_BINDIR}/${BINARCH}
      CACHE PATH "user executables" FORCE)
  set(CMAKE_INSTALL_LIBDIR
      ${CMAKE_INSTALL_LIBDIR}/${LIBARCH}
      CACHE PATH "Object code libraries" FORCE)
  foreach(dir LIBDIR BINDIR)
    gnuinstalldirs_get_absolute_install_dir(CMAKE_INSTALL_FULL_${dir}
                                            CMAKE_INSTALL_${dir} ${dir})
  endforeach()
endif()

if(NOT ONEAPI_INSTALL_PKGCONFIGDIR)
  set(ONEAPI_INSTALL_PKGCONFIGDIR ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

if(NOT ONEAPI_INSTALL_CMAKEDIR)
  set(ONEAPI_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake)
endif()

if(NOT ONEAPI_INSTALL_ENVDIR)
  set(ONEAPI_INSTALL_ENVDIR ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME})
endif()

if(NOT ONEAPI_INSTALL_MODFILEDIR)
  set(ONEAPI_INSTALL_MODFILEDIR ${CMAKE_INSTALL_SYSCONFDIR}/modulefiles)
endif()
if(NOT ONEAPI_INSTALL_EXAMPLEDIR)
  set(ONEAPI_INSTALL_EXAMPLEDIR
      ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/examples)
endif()
if(NOT ONEAPI_INSTALL_LICENSEDIR)
  set(ONEAPI_INSTALL_LICENSEDIR
      ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/licensing)
endif()
if(NOT ONEAPI_INSTALL_PYTHONDIR)
  set(ONEAPI_INSTALL_PYTHONDIR
      ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/python/lib)
endif()

foreach(
  dir
  PKGCONFIGDIR
  CMAKEDIR
  ENVDIR
  MODFILEDIR
  EXAMPLEDIR
  LICENSEDIR
  PYTHONDIR)
  gnuinstalldirs_get_absolute_install_dir(ONEAPI_INSTALL_FULL_${dir}
                                          ONEAPI_INSTALL_${dir} ${dir})
endforeach()

if(WIN32)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
