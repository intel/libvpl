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

@REM ------------------------------------------------------------------------------
@REM Globals
IF NOT DEFINED VPL_DISP_BUILD_DIR (
    set "VPL_DISP_BUILD_DIR=%PROJ_DIR%\_build"
)
@REM ------------------------------------------------------------------------------

IF DEFINED BOOTSTRAP_OPT (
    ECHO Building dependencies...
    call %SCRIPT_DIR%/bootstrap.bat %FORWARD_OPTS%
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

IF DEFINED WARNING_AS_ERROR_OPT (
  SET WARN_CM_OPTS=-DENABLE_WARNING_AS_ERROR=ON
)

SET BUILD_DIR=%VPL_DISP_BUILD_DIR%
MKDIR %BUILD_DIR%
PUSHD %BUILD_DIR%
  cmake %ARCH_CM_OPT% %INSTALL_PREFIX_CM_OPT% %COFIG_CM_OPT% %WARN_CM_OPTS% ^
        -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON %PROJ_DIR% ^
        || EXIT /b 1
  IF DEFINED NUMBER_OF_PROCESSORS (
    SET PARALLEL_OPT=-j %NUMBER_OF_PROCESSORS%
  )
  cmake --build . --config %COFIG_OPT% %PARALLEL_OPT% || EXIT /b 1
  cmake --build . --config %COFIG_OPT% --target package || EXIT /b 1

  @REM Signal to CI system
  IF DEFINED TEAMCITY_VERSION (
    ECHO ##teamcity[publishArtifacts '%BUILD_DIR%/*-all.zip=^>']
  )
POPD
ENDLOCAL
