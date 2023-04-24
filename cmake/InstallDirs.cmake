# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

#
# Set installation directories
#

if(WIN32
   AND NOT CMAKE_CROSSCOMPILING
   AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  if(NOT CMAKE_INSTALL_BINDIR)
    set(BINARCH x86)
  endif()
  if(NOT CMAKE_INSTALL_LIBDIR)
    set(LIBARCH x86)
  endif()
endif()

# See https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html for
# variables GNUInstallDirs exposes. This project commonly uses:
# CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_DOCDIR CMAKE_INSTALL_BINDIR
# CMAKE_INSTALL_LIBDIR
include(GNUInstallDirs)

set(CMAKE_INSTALL_DOCDIR ${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME})
gnuinstalldirs_get_absolute_install_dir(CMAKE_INSTALL_FULL_DOCDIR
                                        CMAKE_INSTALL_DOCDIR DOCDIR)

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

if(NOT VPL_INSTALL_PKGCONFIGDIR)
  set(VPL_INSTALL_PKGCONFIGDIR ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
  set(VPL_INSTALL_FULL_PKGCONFIGDIR ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig)
else()
  gnuinstalldirs_get_absolute_install_dir(
    VPL_INSTALL_FULL_PKGCONFIGDIR VPL_INSTALL_PKGCONFIGDIR
    VPL_INSTALL_PKGCONFIGDIR)
endif()

if(NOT VPL_INSTALL_CMAKEDIR)
  set(VPL_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake)
  set(VPL_INSTALL_FULL_CMAKEDIR ${CMAKE_INSTALL_FULL_LIBDIR}/cmake)
else()
  gnuinstalldirs_get_absolute_install_dir(
    VPL_INSTALL_FULL_CMAKEDIR VPL_INSTALL_CMAKEDIR VPL_INSTALL_CMAKEDIR)
endif()

if(NOT VPL_INSTALL_ENVDIR)
  set(VPL_INSTALL_ENVDIR ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME})
  set(VPL_INSTALL_FULL_ENVDIR ${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME})
else()
  gnuinstalldirs_get_absolute_install_dir(VPL_INSTALL_FULL_ENVDIR
                                          VPL_INSTALL_ENVDIR VPL_INSTALL_ENVDIR)
endif()

if(NOT VPL_INSTALL_MODFILEDIR)
  set(VPL_INSTALL_MODFILEDIR ${CMAKE_INSTALL_SYSCONFDIR}/modulefiles)
  set(VPL_INSTALL_FULL_MODFILEDIR ${CMAKE_INSTALL_FULL_SYSCONFDIR}/modulefiles)
else()
  gnuinstalldirs_get_absolute_install_dir(
    VPL_INSTALL_FULL_MODFILEDIR VPL_INSTALL_MODFILEDIR VPL_INSTALL_MODFILEDIR)
endif()

if(NOT VPL_INSTALL_EXAMPLEDIR)
  set(VPL_INSTALL_EXAMPLEDIR
      ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/examples)
  set(VPL_INSTALL_FULL_EXAMPLEDIR
      ${CMAKE_INSTALL_FULL_DATAROOTDIR}/${PROJECT_NAME}/examples)
else()
  gnuinstalldirs_get_absolute_install_dir(
    VPL_INSTALL_FULL_EXAMPLEDIR VPL_INSTALL_EXAMPLEDIR VPL_INSTALL_EXAMPLEDIR)
endif()
if(NOT VPL_INSTALL_LICENSEDIR)
  set(VPL_INSTALL_LICENSEDIR
      ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/licensing)
  set(VPL_INSTALL_FULL_LICENSEDIR
      ${CMAKE_INSTALL_FULL_DATAROOTDIR}/${PROJECT_NAME}/licensing)
else()
  gnuinstalldirs_get_absolute_install_dir(
    VPL_INSTALL_FULL_LICENSEDIR VPL_INSTALL_LICENSEDIR VPL_INSTALL_LICENSEDIR)
endif()

if(WIN32)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
