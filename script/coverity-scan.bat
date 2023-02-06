::------------------------------------------------------------------------------
:: Copyright (C) Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
:: start of boilerplate to switch to project root ------------------------------
@echo off
SETLOCAL
:: switch cd to repo root
FOR /D %%i IN ("%~dp0") DO (
	set SCRIPT_DIR=%%~fi
)
:: Read options ----------------------------------------------------------------

:: Run scan
py -3 %SCRIPT_DIR%\coverity.py %* || EXIT /b 1
