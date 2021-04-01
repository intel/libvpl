@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Run basic tests on base.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Run basic tests on base." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

set /A result_all = 0

PUSHD %PROJ_DIR%
    @REM No tests defined
POPD

ENDLOCAL && EXIT /B %result_all%
