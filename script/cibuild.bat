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
:: capture ci errors that should not block the rest of the chain
set ci_error=0

call "script/lint.bat"
if %ERRORLEVEL% NEQ 0 (
   echo "--- Linting FAILED ---"
   set ci_error=1
)

if not defined VPL_BUILD_DEPENDENCIES (
   if "%~1"=="gpl" (
     call "script/bootstrap.bat" gpl
   ) else (
     call "script/bootstrap.bat"
   )
   if %ERRORLEVEL% NEQ 0 (
      echo "--- Bootstrapping FAILED ---"
      exit /b 1
   )
)
if "%~1"=="gpl" (
  call "script/package.bat" gpl
) else (
  call "script/package.bat"
)
if %ERRORLEVEL% NEQ 0 (
   echo "--- Packaging FAILED ---"
   exit /b 1
)

:: run smoke tests
call "%PROJ_DIR%/script/test.bat"
if %ERRORLEVEL% NEQ 0 (
   echo "--- Smoke Testing FAILED ---"
   set ci_error=1
)

if %ci_error% NEQ 0 (
   echo "--- CI testing FAILED ---"
   exit /b 1
) else (
   echo "---All Tests PASSED ---"
)
