@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Run basic tests on project.

@ECHO off

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Run basic tests on project." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

PUSHD %ROOT_DIR%
    ECHO Testing base...
    CALL oneVPL\script\test.bat %FORWARD_OPTS%
    ECHO Testing cpu...
    CALL oneVPL-cpu\script\test.bat %FORWARD_OPTS%
POPD