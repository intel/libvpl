@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Build base dependencies.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Build base dependencies." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

SET PY_VER=-3-64
IF "%ARCH_OPT%"=="x86_32" (
  SET PY_VER=-3-32
)

PUSHD %PROJ_DIR%
  py %PY_VER% -m venv _build-venv
  call %PROJ_DIR%\_build-venv\Scripts\activate.bat
  python -m pip install -r "%PROJ_DIR%\requirements.txt"

  @REM Add pybind11 to CMake Module Path
  FOR /F "tokens=*" %%g IN ('python -m pybind11 --cmakedir') do (SET pybind11_DIR=%%g)
POPD

ENDLOCAL & set pybind11_DIR=%pybind11_DIR% & call %PROJ_DIR%\_build-venv\Scripts\activate.bat
