@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Check code for issues.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

@REM Set project folder
FOR /D %%i IN ("%~dp0\..") DO (
	SET PROJ_DIR=%%~fi
)

@REM Set script folder
FOR /D %%i IN ("%~dp0") DO (
	SET SCRIPT_DIR=%%~fi
)

PUSHD %PROJ_DIR%
    echo Check commit-msg
    pre-commit run --hook-stage manual gitlint-ci || EXIT /b 1

    echo Check commit
	pre-commit run --all-files || EXIT /b 1
POPD

ENDLOCAL
