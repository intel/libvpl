::------------------------------------------------------------------------------
:: Copyright (C) Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
:: Configure environment variables
@echo off
setlocal
set VPL_TARGET_ARCH=intel64
for %%Q in ("%~dp0\.") DO set "script_dir=%%~fQ"

:ParseArgs
if /i "%1"==""         goto EndParseArgs
if /i "%1"=="ia32"     (set VPL_TARGET_ARCH=ia32)    & shift & goto ParseArgs
if /i "%1"=="intel64"  (set VPL_TARGET_ARCH=intel64) & shift & goto ParseArgs
shift & goto ParseArgs
:EndParseArgs

IF "%VPL_TARGET_ARCH%"=="ia32" (
  endlocal
  call "%script_dir%\vars32.bat" %*
  setlocal
  set vpl_vars_errorlevel=%errorlevel%
) ELSE (
  endlocal
  call "%script_dir%\vars64.bat" %*
  setlocal
  set vpl_vars_errorlevel=%errorlevel%
)
exit /B %vpl_vars_errorlevel%
