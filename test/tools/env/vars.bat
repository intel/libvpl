@echo off

:: ############################################################################
::  # Copyright (C) 2019 Intel Corporation
::  #
::  # SPDX-License-Identifier: MIT
::  ############################################################################

:: Configure enviroment variables

:: set VPL_TOOLS_DIR=%~dp0\..
FOR /D %%i IN ("%~dp0\..") DO (
	set VPL_TOOLS_DIR=%%~fi
)
set "VPL_TOOLS_PYTHONPATH=%VPL_TOOLS_DIR%\python"
IF DEFINED PYTHONPATH ( 
	set "PYTHONPATH=%VPL_TOOLS_PYTHONPATH%;%PYTHONPATH%"
) ELSE (
	set "PYTHONPATH=%VPL_TOOLS_PYTHONPATH%"
)
