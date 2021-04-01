@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Clean base.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Clean base." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

IF DEFINED BOOTSTRAP_OPT (
  IF EXIST %VPL_BUILD_DEPENDENCIES% (
    ECHO Cleaning dependencies cache folder...
    RD /s /q _extbuild
  )
)

PUSHD %PROJ_DIR%
  IF EXIST _extbuild (
    ECHO Cleaning dependencies folder...
    RD /s /q _deps
  )

  IF EXIST _extbuild (
    ECHO Cleaning dependencies build folder...
    RD /s /q _extbuild
  )

  IF EXIST _build (
    ECHO Cleaning build folder...
    RD /s /q _build
  )
POPD

ENDLOCAL