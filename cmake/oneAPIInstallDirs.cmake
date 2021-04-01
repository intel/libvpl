# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

#
# Set installation directories
#

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
  set(CMAKE_INSTALL_BINDIR bin32)
  set(CMAKE_INSTALL_LIBDIR lib32)
else()
  set(CMAKE_INSTALL_BINDIR bin)
  set(CMAKE_INSTALL_LIBDIR lib)
endif()

set(CMAKE_INSTALL_INCLUDEDIR include)
set(CMAKE_INSTALL_DOCDIR documentation)
set(CMAKE_INSTALL_ENVDIR env)
set(CMAKE_INSTALL_MODDIR modulefiles)
set(CMAKE_INSTALL_SYSCHECKDIR sys_check)
set(CMAKE_INSTALL_MODFILEDIR modulefiles)
set(CMAKE_INSTALL_REDISTDIR redist)
set(CMAKE_INSTALL_EXAMPLEDIR examples)
set(CMAKE_INSTALL_LICENSEDIR licensing)
set(CMAKE_INSTALL_TOOLSDIR tools)
set(CMAKE_INSTALL_SYSCONFDIR config)
set(CMAKE_INSTALL_SRCDIR src)
set(CMAKE_INSTALL_PYTHONDIR python/lib)
