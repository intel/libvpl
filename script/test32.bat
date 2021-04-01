@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Run basic tests on 32bit build of base.

@ECHO off
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

CALL %~dp0%\test.bat -A x86_32
SET result_all=%errorlevel%

ENDLOCAL && EXIT /B %result_all%
