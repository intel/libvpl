@REM ------------------------------------------------------------------------------
@REM Copyright (C) Intel Corporation
@REM 
@REM SPDX-License-Identifier: MIT
@REM ------------------------------------------------------------------------------
@REM Build Doxygen documentation
FOR /D %%i IN ("%~dp0") DO (
	set SCRIPT_DIR=%%~fi
)

doxygen %SCRIPT_DIR%\Doxyfile
xcopy assets html\assets\ /s /q /f /y
