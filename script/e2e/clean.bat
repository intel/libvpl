@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Clean project.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Clean project." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

ECHO Cleaning project...

IF EXIST %VPL_INSTALL_DIR% (
    ECHO Cleaning install folder...
    RD /s /q %VPL_INSTALL_DIR%
)

PUSHD %ROOT_DIR%
    ECHO Cleaning base...
    CALL oneVPL\script\clean.bat %FORWARD_OPTS%
    ECHO Cleaning cpu...
    CALL oneVPL-cpu\script\clean.bat %FORWARD_OPTS%
POPD

ENDLOCAL