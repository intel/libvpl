::------------------------------------------------------------------------------
:: Copyright (C) 2020 Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
:: start of boilerplate to switch to project root ------------------------------
@echo off
SETLOCAL
FOR /D %%i IN ("%~dp0\..") DO (
	set PROJ_DIR=%%~fi
)
cd %PROJ_DIR%
:: start of commands -----------------------------------------------------------
call "%PROJ_DIR%/test/tools/env/vars.bat"
if defined VPL_BUILD_DEPENDENCIES (
  set ffmpeg_dir=%VPL_BUILD_DEPENDENCIES%\bin
) else (
    echo VPL_BUILD_DEPENDENCIES not defined. Did you run bootstrap?
    exit /b 1
  )
)
set "PATH=%ffmpeg_dir%;%PATH%"

cd _build\Release
set /A result_all = 0

:unit_tests
echo *** Running Unit Tests ***
call vpl-utest.exe --gtest_output=xml:%PROJ_DIR%\_logs\
if %errorlevel%==0 goto unit_tests_passed
echo *** Unit Tests FAILED ***
set /A result_all = 1
goto test_decode

:unit_tests_passed
echo *** Unit Tests PASSED ***

:test_decode
echo *** Running Decode Smoke Test***
call sample_decode.exe h265 -i %PROJ_DIR%\test\content\cars_128x96.h265 ^
     -o out_vpl_h265.i420 -vpl
call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -i %PROJ_DIR%\test\content\cars_128x96.h265 ^
     -f rawvideo -pix_fmt yuv420p out_ref_h265.i420
call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -r 30 -s 128x96 -pix_fmt yuv420p -f rawvideo -i out_vpl_h265.i420 ^
     -r 30 -s 128x96 -pix_fmt yuv420p -f rawvideo -i out_ref_h265.i420 ^
     -filter_complex psnr= -f null nullsink
call python %PYTHONPATH%\check_content\check_smoke_output.py ^
     out_ref_h265.i420 out_vpl_h265.i420 I420 128x96@30

echo.
if %errorlevel%==0 goto test_decode_passed
echo *** Decode Smoke Test FAILED ***
set /A result_all = 1
goto test_encode

:test_decode_passed
echo *** Decode Smoke Test PASSED ***

:test_encode
echo *** Running Encode Smoke Test***
call sample_encode.exe h265 -i out_ref_h265.i420 ^
     -o out_vpl.h265 -w 128 -h 96 -vpl
call sample_decode.exe h265 -i out_vpl.h265 -o out_vpl_dec_h265.i420 -vpl
call python %PYTHONPATH%\check_content\check_smoke_output.py ^
     out_ref_h265.i420 out_vpl_dec_h265.i420 I420 128x96@30

echo.
if %errorlevel%==0 goto test_encode_passed
echo *** Encode Smoke Test FAILED ***
set /A result_all = 1
goto test_end

:test_encode_passed
echo *** Encode Smoke Test PASSED ***

:test_end
exit /B %result_all%
