@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Build project.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Build project." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

ECHO Building project code...

PUSHD %ROOT_DIR%
    ECHO Building base component...
    CALL oneVPL\script\build.bat %FORWARD_OPTS% || EXIT /b 1
    ECHO Installing base component...
    CALL oneVPL\script\install.bat || EXIT /b 1
    ECHO Building cpu component...
    CALL oneVPL-cpu\script\build.bat %FORWARD_OPTS% || EXIT /b 1
    ECHO Installing cpu component...
    CALL oneVPL-cpu\script\install.bat || EXIT /b 1
POPD

ENDLOCAL