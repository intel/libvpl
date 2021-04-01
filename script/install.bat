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

PUSHD %PROJ_DIR%
    SET BUILD_DIR=_build
    PUSHD %BUILD_DIR%
        cmake --build . --config %COFIG_OPT% --target install
    POPD
POPD

ENDLOCAL