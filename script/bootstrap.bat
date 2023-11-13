@rem ------------------------------------------------------------------------------
@rem Copyright (C) Intel Corporation
@rem
@rem SPDX-License-Identifier: MIT
@rem ------------------------------------------------------------------------------
@rem Install dependencies.

@echo off
setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

:: configure winget to accept source agreements
winget list Python.Python.3.10 --accept-source-agreements >NUL

winget list Python.Python.3.10 >NUL || winget install -e --id=Python.Python.3.10
if %errorlevel% neq 0 exit /b %errorlevel%

winget list Kitware.CMake >NUL || winget install -e --id Kitware.CMake
if %errorlevel% neq 0 exit /b %errorlevel%

winget list Microsoft.VisualStudio.2022.Community >NUL || winget install Microsoft.VisualStudio.2022.Community --silent --override "--wait --quiet --add ProductLang En-us --add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended"
if %errorlevel% neq 0 exit /b %errorlevel%

winget list Git.Git >NUL || winget install -e --id=Git.Git
if %errorlevel% neq 0 exit /b %errorlevel%

python -m pip install pre-commit --proxy=%HTTP_PROXY%
python -m pip install virtualenv==v20.24.5 --proxy=%HTTP_PROXY%

endlocal
