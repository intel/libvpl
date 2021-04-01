@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Build project dependencies.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Build project dependencies." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

ECHO Building project dependencies...

PUSHD %ROOT_DIR%
    ECHO Building base dependencies build folder...
    CALL oneVPL\script\bootstrap.bat %FORWARD_OPTS% || EXIT /b 1
    ECHO Building cpu dependencies build folder...
    CALL oneVPL-cpu\script\bootstrap.bat %FORWARD_OPTS% || EXIT /b 1
POPD

IF DEFINED VPL_BUILD_DEPENDENCIES (
  @REM Export build dependency environment
  ENDLOCAL && SET VPL_BUILD_DEPENDENCIES=%VPL_BUILD_DEPENDENCIES%
) ELSE (
  ENDLOCAL
)
