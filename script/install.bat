@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Install base.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Install base." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

@REM ------------------------------------------------------------------------------
@REM Globals
IF NOT DEFINED VPL_DISP_BUILD_DIR (
    set "VPL_DISP_BUILD_DIR=%PROJ_DIR%\_build"
)
@REM ------------------------------------------------------------------------------

PUSHD %PROJ_DIR%
    SET BUILD_DIR=%VPL_DISP_BUILD_DIR%
    PUSHD %BUILD_DIR%
        cmake --build . --config %COFIG_OPT% --target install
    POPD
POPD

ENDLOCAL