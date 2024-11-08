@rem ------------------------------------------------------------------------------
@rem Copyright (C) Intel Corporation
@rem
@rem SPDX-License-Identifier: MIT
@rem ------------------------------------------------------------------------------
@rem Run the test suite.
@rem
@rem  Scope can be limited by providing subset of tests as argumene from among:
@rem  lint, unit

@echo off
setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

for %%Q in ("%~dp0\.") DO set "script_dir=%%~fQ"
pushd "%script_dir%\.."
  set "source_dir=%cd%"
popd
set "build_dir=%source_dir%\_build"
set "staging_dir=%source_dir%\_install"


if "%~1"=="" (
 set "do_lint=1" & set "do_unit=1" & goto done
 )
:loop
if "%1"=="lint" (set "do_lint=1") else ^
if "%1"=="unit" (set "do_unit=1") else ^
echo invalid option: '%1' && exit /b 1
shift
if not "%~1"=="" goto loop
:done


if "%do_lint%" == "1" (
  call "%source_dir%\script\lint.bat"
  if !errorlevel! neq 0 exit /b !errorlevel!
)

if "%do_unit%" == "1" (
  ctest --test-dir "%build_dir%" -C Release --output-on-failure -T test
  if !errorlevel! neq 0 exit /b !errorlevel!
)

endlocal
