@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM User friendly consistant parameter parsing

@REM Set project folder
FOR /D %%i IN ("%~dp0\..") DO (
	SET PROJ_DIR=%%~fi
)

@REM Set script folder
FOR /D %%i IN ("%~dp0") DO (
	SET SCRIPT_DIR=%%~fi
)

@REM Clear options
SET "HELP_OPT="
SET "GPL_OPT="
SET "WARNING_AS_ERROR_OPT="
SET COFIG_OPT=Release
SET ARCH_OPT=x86_64
SET "BOOTSTRAP_OPT="

@REM Read information about origin script before parsing command line
:PREFIX_Loop
  IF [%~1]==[] (
    GOTO PREFIX_Continue
  ) ELSE IF "%~1"=="--" (
    @REM if we see a double dash on its own it marks the end of the prefix
    SHIFT
    GOTO PREFIX_Continue
  ) ELSE IF "%~1"=="--name" (
    SET ORIG_SCRIPT_NAME=%~2
    SHIFT
  ) ELSE IF "%~1"=="--desc" (
    SET ORIG_SCRIPT_DESC=%~2
    SHIFT
  ) ELSE (
    ECHO Unrecognized option "%~1"
    SET HELP_OPT=yes
    GOTO Continue
  )
  SHIFT
  GOTO PREFIX_Loop
:PREFIX_Continue

@REM Read command line options
:Loop
  IF [%~1]==[] (
    GOTO Continue
  ) ELSE IF "%~1"=="--gpl" (
    SET GPL_OPT=yes
  ) ELSE IF "%~1"=="gpl" (
    SET GPL_OPT=yes
  ) ELSE IF "%~1"=="--warning_as_error" (
    SET WARNING_AS_ERROR_OPT=yes
  ) ELSE IF "%~1"=="--config" (
    SET COFIG_OPT=%~2
    SHIFT
  ) ELSE IF "%~1"=="debug" (
    SET COFIG_OPT=Debug
  ) ELSE IF "%~1"=="-A" (
    SET ARCH_OPT=%~2
    SHIFT
  ) ELSE IF "%~1"=="--bootstrap" (
    SET BOOTSTRAP_OPT=yes
  ) ELSE IF "%~1"=="--help" (
    SET HELP_OPT=yes
  ) ELSE IF "%~1"=="-h" (
    SET HELP_OPT=yes
  ) ELSE (
    ECHO Unrecognized option "%~1"
    SET HELP_OPT=yes
    GOTO Continue
  )
  SHIFT
  GOTO Loop
:Continue

@REM Print usage message
IF DEFINED HELP_OPT (
  ECHO Usage: %ORIG_SCRIPT_NAME% [options]
  ECHO   --gpl                Include componentes using GPL licensing
  ECHO   --warning_as_error   Treat compiler warnings as errors
  ECHO   --config CONFIG      Build configuration
  ECHO   -A ARCH              Target architecture
  ECHO   --bootstrap          Include bootstrap steps
  ECHO   --help, -h           Show this help message
  ECHO.
  ECHO Depricated options
  ECHO   debug                same as "--config Debug"
  ECHO   gpl                  same as "--gpl"
  ECHO.
  ECHO CONFIG may be: Release, Debug
  ECHO ARCH may be: x86_64, x86_32
  ECHO.
  ECHO %ORIG_SCRIPT_DESC%
  EXIT /b 0
)

@REM Equivalent parameters to what this was called with for further calls
SET "FORWARD_OPTS="
IF DEFINED GPL_OPT (
  SET FORWARD_OPTS=%FORWARD_OPTS% --gpl
)
IF DEFINED WARNING_AS_ERROR_OPT (
  SET FORWARD_OPTS=%FORWARD_OPTS% --warning_as_error
)
IF DEFINED COFIG_OPT (
  SET FORWARD_OPTS=%FORWARD_OPTS% --config %COFIG_OPT%
)
IF DEFINED ARCH_OPT (
  SET FORWARD_OPTS=%FORWARD_OPTS% -A %ARCH_OPT%
)
IF DEFINED BOOTSTRAP_OPT (
  SET FORWARD_OPTS=%FORWARD_OPTS% --bootstrap
)

exit /b 0
