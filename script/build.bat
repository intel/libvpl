@rem ------------------------------------------------------------------------------
@rem Copyright (C) Intel Corporation
@rem 
@rem SPDX-License-Identifier: MIT
@rem ------------------------------------------------------------------------------
@rem Build in typical fashion.

@echo off
setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION 

for %%Q in ("%~dp0\.") DO set "script_dir=%%~fQ"
pushd %script_dir%\..
  set "source_dir=%cd%"
popd
set "build_dir=%source_dir%\_build"
set "staging_dir=%source_dir%\_install"

cmake -B "%build_dir%" -S "%source_dir%" ^
      -DBUILD_TESTS=ON ^
      -DENABLE_WARNING_AS_ERROR=ON
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build "%build_dir%" --parallel %NUMBER_OF_PROCESSORS% --verbose --config Release
if %errorlevel% neq 0 exit /b %errorlevel%

:: cmake cpack conflicts with choco cpack, at least until next choco release
cmake --build "%build_dir%" --parallel %NUMBER_OF_PROCESSORS% --config Release --target package
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --install "%build_dir%" --prefix "%staging_dir%" --config Release --strip
if %errorlevel% neq 0 exit /b %errorlevel%

endlocal
