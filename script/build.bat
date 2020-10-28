::------------------------------------------------------------------------------
:: Copyright (C) 2020 Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
:: start of boilerplate to switch to project root ------------------------------
@echo off
SETLOCAL
FOR /D %%i IN ("%~dp0\..") DO (
	set PROJ_DIR=%%~fi
)
cd %PROJ_DIR%

:: Read options -----------------------------------------------------------
SET BUILD_MODE=Release

:Loop
IF "%~1"=="" GOTO Continue
  IF "%~1"=="debug" (
    SET BUILD_MODE=Debug
  )
SHIFT
GOTO Loop
:Continue

:: start of commands -----------------------------------------------------------
set CMAKE_BINARY_DIR=_build
if defined VPL_INSTALL_DIR (
   set INSTALL_OPTS=-DCMAKE_INSTALL_PREFIX=%VPL_INSTALL_DIR%
)

mkdir %CMAKE_BINARY_DIR%
cd %CMAKE_BINARY_DIR%

cmake -A x64 %INSTALL_OPTS% ..

cmake --build . --config %BUILD_MODE% -j %NUMBER_OF_PROCESSORS%

cmake --build . --config %BUILD_MODE% --target package

if defined TEAMCITY_VERSION (
   echo ##teamcity[publishArtifacts 'oneVPL/%CMAKE_BINARY_DIR%/*-all.zip=^>']
)
