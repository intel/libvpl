::------------------------------------------------------------------------------
:: Copyright (C) 2020 Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
:: start of boilerplate to switch to project root ------------------------------
@echo off
SETLOCAL
:: switch cd to repo root
FOR /D %%i IN ("%~dp0\..") DO (
	set PROJ_DIR=%%~fi
)
cd %PROJ_DIR%
:: start of commands -----------------------------------------------------------
set CMAKE_BINARY_DIR=_build

mkdir %CMAKE_BINARY_DIR%
cd %CMAKE_BINARY_DIR%

if "%~1"=="gpl" (
  cmake -A x64 -DBUILD_GPL_X264=ON ..
) else (
  cmake -A x64 ..
)

cmake --build . --config Release --target package
