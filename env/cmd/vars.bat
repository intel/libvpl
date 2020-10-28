::------------------------------------------------------------------------------
:: Copyright (C) 2020 Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
:: Configure environment variables
@echo off
FOR /D %%i IN ("%~dp0\..") DO (
  set VPL_ROOT=%%~fi
)
FOR /D %%i IN ("%~dp0\..\..") DO (
  set VPL_PREFIX=%%~fi
)
set "VPL_INCLUDE=%VPL_ROOT%\include"
set "VPL_LIB=%VPL_ROOT%\lib"
set "VPL_BIN=%VPL_ROOT%\bin"

IF DEFINED INCLUDE (
  set "INCLUDE=%VPL_INCLUDE%;%INCLUDE%"
) ELSE (
  set "INCLUDE=%VPL_INCLUDE%"
)

IF DEFINED LIB (
  set "LIB=%VPL_LIB%;%LIB%"
) ELSE (
  set "LIB=%VPL_LIB%"
)

IF DEFINED PATH (
  set "PATH=%VPL_BIN%;%PATH%"
) ELSE (
  set "PATH=%VPL_BIN%"
)

IF DEFINED CMAKE_PREFIX_PATH (
  set "CMAKE_PREFIX_PATH=%VPL_PREFIX%;%CMAKE_PREFIX_PATH%"
) ELSE (
  set "CMAKE_PREFIX_PATH=%VPL_PREFIX%"
)
set VPL_PREFIX=
