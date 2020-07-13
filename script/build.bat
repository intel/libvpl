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
:: start of commands -----------------------------------------------------------
set CMAKE_BINARY_DIR=_build

mkdir %CMAKE_BINARY_DIR%
cd %CMAKE_BINARY_DIR%

cmake -A x64 ..
cmake --build . --config Release
