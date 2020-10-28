::------------------------------------------------------------------------------
:: Copyright (C) Intel Corporation
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
 
cmake --build %CMAKE_BINARY_DIR% --config %BUILD_MODE% --target install
