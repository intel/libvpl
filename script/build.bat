@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Build base.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

@REM Read command line options
CALL %~dp0%\_buildopts.bat ^
    --name "%~n0%" ^
    --desc "Build base." ^
    -- %*
IF DEFINED HELP_OPT ( EXIT /b 0 )

IF DEFINED BOOTSTRAP_OPT (
    ECHO Building dependencies...
    %SCRIPT_DIR%/bootstrap.bat %FORWARD_OPTS%
  )
)

IF "%ARCH_OPT%"=="x86_64" (
  SET ARCH_CM_OPT=-A x64
) ELSE IF "%ARCH_OPT%"=="x86_32" (
  SET ARCH_CM_OPT=-A Win32
)

IF DEFINED VPL_INSTALL_DIR (
  MD %VPL_INSTALL_DIR%
  SET INSTALL_PREFIX_CM_OPT=-DCMAKE_INSTALL_PREFIX=%VPL_INSTALL_DIR%
)

SET COFIG_CM_OPT=-DCMAKE_BUILD_TYPE=%COFIG_OPT%

PUSHD %PROJ_DIR%
  SET BUILD_DIR=_build
  MKDIR %BUILD_DIR%
  PUSHD %BUILD_DIR%
    cmake %ARCH_CM_OPT% %INSTALL_PREFIX_CM_OPT% %COFIG_CM_OPT% .. || EXIT /b 1

    IF DEFINED NUMBER_OF_PROCESSORS (
      SET PARALLEL_OPT=-j %NUMBER_OF_PROCESSORS%
    )
    cmake --build . --config %COFIG_OPT% %PARALLEL_OPT% || EXIT /b 1
    cmake --build . --config %COFIG_OPT% --target package || EXIT /b 1

    @REM Signal to CI system 
    IF DEFINED TEAMCITY_VERSION (
      ECHO ##teamcity[publishArtifacts 'oneVPL/%BUILD_DIR%/*-all.zip=^>']
    )
  POPD
POPD
ENDLOCAL