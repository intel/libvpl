::------------------------------------------------------------------------------
:: Copyright (C) 2020 Intel Corporation
::
:: SPDX-License-Identifier: MIT
::------------------------------------------------------------------------------
::
:: Usage: script\stress.bat [iteration number]
::    ex: script\stress.bat 5
::
:: Steps ::
::   1. In each iterations, it launches listed processes at the same time.
::      (decode h264/h265/av1/jpeg, encode h264/h265/av1/jpeg, vpp crop-csc)
::   2. Check whether all the processes are finished with PROCESS_TIMER_INTERVAL
::      time interval and PROCESS_TIME_OUT_MAX times.
::      (if there's timeout, if there's any process is hung test's failed)
::   3. Compare the output from processes with reference output.
::      (if there's failure, test's failed)
::------------------------------------------------------------------------------

:: start of boilerplate to switch to project root ------------------------------
@echo off
SETLOCAL
FOR /D %%i IN ("%~dp0\..") DO (
	set PROJ_DIR=%%~fi
)

if "%1" == "" (
     set NUM_ITERATION=1
) else (
     set NUM_ITERATION=%1
)

echo [ Total execution number: %NUM_ITERATION% ]
echo.

set /A cur_iter=1

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
set WORK_DIR=%PROJ_DIR%\_build\Release
cd %WORK_DIR%

:: variables for process time out (unit of process_timer_interval is second)
:: script will check processes existance PROCESS_TIME_OUT_MAX times every PROCESS_TIMER_INTERVAL sec
set PROCESS_TIMER_INTERVAL=5
set PROCESS_TIME_OUT_MAX=60

:: vpl-decode in/out file
set FI_DECODE_H264=%PROJ_DIR%\test\content\cars_128x96.h264
set FO_DECODE_H264=out_dec_h264.i420
set FI_DECODE_H265=%PROJ_DIR%\test\content\cars_128x96.h265
set FO_DECODE_H265=out_dec_h265.i420
set FI_DECODE_AV1=%PROJ_DIR%\test\content\cars_128x96.ivf
set FO_DECODE_AV1=out_dec_av1.i420
set FI_DECODE_JPEG=%PROJ_DIR%\test\content\cars_128x96.mjpeg
set FO_DECODE_JPEG=out_dec_mjpeg.i420
:: vpl-encode in/out file
set FI_ENCODE=%PROJ_DIR%\test\content\cars_128x96.i420
set FO_ENCODE_H264=out_enc.h264
set FO_ENCODE_H265=out_enc.h265
set FO_ENCODE_AV1=out_enc.ivf
set FO_ENCODE_JPEG=out_enc.mjpeg
:: vpl-vpp in/out file
set FI_VPP=%FI_ENCODE%
set FO_VPP=out_vpp.bgra

:: define test cases
:::: vpl-decode processing list
set VPL_DECODE_H264="%WORK_DIR%\vpl-decode.exe -if H264 -i %FI_DECODE_H264% -o %FO_DECODE_H264% -int"
set VPL_DECODE_H265="%WORK_DIR%\vpl-decode.exe -if H265 -i %FI_DECODE_H265% -o %FO_DECODE_H265% -int"
set VPL_DECODE_AV1="%WORK_DIR%\vpl-decode.exe -if AV1 -i %FI_DECODE_AV1% -o %FO_DECODE_AV1% -int"
set VPL_DECODE_JPEG="%WORK_DIR%\vpl-decode.exe -if JPEG -i %FI_DECODE_JPEG% -o %FO_DECODE_JPEG% -int"
:::: vpl-encode processing list
set VPL_ENCODE_H264="%WORK_DIR%\vpl-encode.exe -of H264 -if I420 -sw 128 -sh 96 -i %FI_ENCODE% -o %FO_ENCODE_H264% -int"
set VPL_ENCODE_H265="%WORK_DIR%\vpl-encode.exe -of H265 -if I420 -sw 128 -sh 96 -i %FI_ENCODE% -o %FO_ENCODE_H265% -int"
set VPL_ENCODE_AV1="%WORK_DIR%\vpl-encode.exe -of AV1 -if I420 -sw 128 -sh 96 -i %FI_ENCODE% -o %FO_ENCODE_AV1% -gs 30 -fr 30 -bm 2 -br 4000 -tu 7 -int"
set VPL_ENCODE_JPEG="%WORK_DIR%\vpl-encode.exe -of JPEG -if I420 -sw 128 -sh 96 -i %FI_ENCODE% -o %FO_ENCODE_JPEG% -int"
:::: vpl-vpp processing list
set VPL_VPP_SRC=-sw 128 -sh 96 -scrx 10 -scry 10 -scrw 50 -scrh 50 -if I420
set VPL_VPP_DST=-dw 640 -dh 480 -dcrx 10 -dcry 10 -dcrw 300 -dcrh 300 -of BGRA
set VPL_VPP="%WORK_DIR%\vpl-vpp.exe %VPL_VPP_SRC% -i %FI_VPP% %VPL_VPP_DST% -o %FO_VPP% -int"

:: run process
:process_start

echo launch .. %VPL_DECODE_H264%
for /f "tokens=2 delims==; " %%a in (' wmic process call create %VPL_DECODE_H264%^,%WORK_DIR% ^| find "ProcessId" ') do set PID1=%%a
echo launch .. %VPL_DECODE_H265%
for /f "tokens=2 delims==; " %%a in (' wmic process call create %VPL_DECODE_H265%^,%WORK_DIR% ^| find "ProcessId" ') do set PID2=%%a
echo launch .. %VPL_DECODE_AV1%
for /f "tokens=2 delims==; " %%a in (' wmic process call create %VPL_DECODE_AV1%^,%WORK_DIR% ^| find "ProcessId" ') do set PID3=%%a
echo launch .. %VPL_DECODE_JPEG%
for /f "tokens=2 delims==; " %%a in (' wmic process call create %VPL_DECODE_JPEG%^,%WORK_DIR% ^| find "ProcessId" ') do set PID4=%%a
echo launch .. %VPL_ENCODE_H264%
for /f "tokens=2 delims==; " %%a in (' wmic process call create %VPL_ENCODE_H264%^,%WORK_DIR% ^| find "ProcessId" ') do set PID5=%%a
echo launch .. %VPL_ENCODE_H265%
for /f "tokens=2 delims==; " %%a in (' wmic process call create %VPL_ENCODE_H265%^,%WORK_DIR% ^| find "ProcessId" ') do set PID6=%%a
echo launch .. %VPL_ENCODE_AV1%
for /f "tokens=2 delims==; " %%a in (' wmic process call create %VPL_ENCODE_AV1%^,%WORK_DIR% ^| find "ProcessId" ') do set PID7=%%a
echo launch .. %VPL_ENCODE_JPEG%
for /f "tokens=2 delims==; " %%a in (' wmic process call create %VPL_ENCODE_JPEG%^,%WORK_DIR% ^| find "ProcessId" ') do set PID8=%%a
echo launch .. %VPL_VPP%
for /f "tokens=2 delims==; " %%a in (' wmic process call create %VPL_VPP%^,%WORK_DIR% ^| find "ProcessId" ') do set PID9=%%a

set PID_LIST=%PID1% %PID2% %PID3% %PID4% %PID5% %PID6% %PID7% %PID8% %PID9%

:: check process
set /A cnt_check_process=0
:loop
  set /A result=0
  tasklist /fi "pid eq %PID1%" | find "vpl" > nul
  set /A result=result+%errorlevel%
  tasklist /fi "pid eq %PID2%" | find "vpl" > nul
  set /A result=result+%errorlevel%
  tasklist /fi "pid eq %PID3%" | find "vpl" > nul
  set /A result=result+%errorlevel%
  tasklist /fi "pid eq %PID4%" | find "vpl" > nul
  set /A result=result+%errorlevel%
  tasklist /fi "pid eq %PID5%" | find "vpl" > nul
  set /A result=result+%errorlevel%
  tasklist /fi "pid eq %PID6%" | find "vpl" > nul
  set /A result=result+%errorlevel%
  tasklist /fi "pid eq %PID7%" | find "vpl" > nul
  set /A result=result+%errorlevel%
  tasklist /fi "pid eq %PID8%" | find "vpl" > nul
  set /A result=result+%errorlevel%
  tasklist /fi "pid eq %PID9%" | find "vpl" > nul
  set /A result=result+%errorlevel%

  if %result%==9 goto check_output

  timeout /t %PROCESS_TIMER_INTERVAL%
  set /A cnt_check_process=cnt_check_process+1
  if %cnt_check_process%==%PROCESS_TIME_OUT_MAX% goto stress_test_fail
goto loop

:stress_test_fail
echo *** Stress Test FAILED ***
:: kill remained process
for %%a in (%PID_LIST%) do (
     taskkill /f /pid %%a 2>nul
)
exit /b 1

:check_output

set /A chk_out_result_all = 0
set FO_REF=ref.out

:: check decode h264 output
call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -i %FI_DECODE_H264% ^
     -f rawvideo -pixel_format yuv420p %FO_REF%
call py -3 %PYTHONPATH%\check_content\check_smoke_output.py ^
     %FO_DECODE_H264% %FO_REF% I420 128x96@30
echo.
if %errorlevel%==0 goto chk_decode_h264_out_passed
echo *** Decode (H264) Stress Test FAILED ***
set /A chk_out_result_all = 1
goto chk_decode_h265_out

:chk_decode_h264_out_passed
echo *** Decode (H264) Stress Test PASSED ***

:: check decode h265 output
:chk_decode_h265_out

call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -i %FI_DECODE_H265% ^
     -f rawvideo -pixel_format yuv420p %FO_REF%
call py -3 %PYTHONPATH%\check_content\check_smoke_output.py ^
     %FO_DECODE_H265% %FO_REF% I420 128x96@30
echo.
if %errorlevel%==0 goto chk_decode_h265_out_passed
echo *** Decode (H265) Stress Test FAILED ***
set /A chk_out_result_all = 1
goto chk_decode_av1_out

:chk_decode_h265_out_passed
echo *** Decode (H265) Stress Test PASSED ***

:: check decode av1 output
:chk_decode_av1_out

call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -i %FI_DECODE_AV1% ^
     -f rawvideo -pixel_format yuv420p %FO_REF%
call py -3 %PYTHONPATH%\check_content\check_smoke_output.py ^
     %FO_DECODE_AV1% %FO_REF% I420 128x96@30
echo.
if %errorlevel%==0 goto chk_decode_av1_out_passed
echo *** Decode (AV1) Stress Test FAILED ***
set /A chk_out_result_all = 1
goto chk_decode_jpeg_out

:chk_decode_av1_out_passed
echo *** Decode (AV1) Stress Test PASSED ***

:: check decode jpeg output
:chk_decode_jpeg_out

call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -i %FI_DECODE_JPEG% ^
     -f rawvideo -pixel_format yuv420p %FO_REF%
call py -3 %PYTHONPATH%\check_content\check_smoke_output.py ^
     %FO_DECODE_JPEG% %FO_REF% I420 128x96@30
echo.
if %errorlevel%==0 goto chk_decode_jpeg_out_passed
echo *** Decode (JPEG) Stress Test FAILED ***
set /A chk_out_result_all = 1
goto done_decode_out_test

:chk_decode_jpeg_out_passed
echo *** Decode (JPEG) Stress Test PASSED ***

:done_decode_out_test

:: check encode h264 output
call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -f rawvideo -pixel_format yuv420p -video_size 128x96 ^
     -i %FI_ENCODE% ^
     -c:v libx264 ^
     -g 30 -rc 1 -preset ultrafast -b:v 4000*1000 -f h264 ^
     %FO_REF%
call py -3 %PYTHONPATH%\check_content\check_smoke_output.py ^
     %FO_ENCODE_H264% %FO_REF% H264 128x96@30
echo.
if %errorlevel%==0 goto chk_encode_h264_out_passed
echo *** Encode (H264) Stress Test FAILED ***
set /A chk_out_result_all = 1
goto chk_encode_h265_out

:chk_encode_h264_out_passed
echo *** Encode (H264) Stress Test PASSED ***

:: check encode h265 output
:chk_encode_h265_out
call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -f rawvideo -pixel_format yuv420p -video_size 128x96 ^
     -i %FI_ENCODE% ^
     -c:v libsvt_hevc ^
     -g 30 -rc 1 -preset 9 -b:v 4000*1000 -f hevc ^
     %FO_REF%
call py -3 %PYTHONPATH%\check_content\check_smoke_output.py ^
     %FO_ENCODE_H265% %FO_REF% H265 128x96@30
echo.
if %errorlevel%==0 goto chk_encode_h265_out_passed
echo *** Encode (H265) Stress Test FAILED ***
set /A chk_out_result_all = 1
goto chk_encode_av1_out

:chk_encode_h265_out_passed
echo *** Encode (H265) Stress Test PASSED ***

:: check encode av1 output
:chk_encode_av1_out
call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -f rawvideo -pixel_format yuv420p -video_size 128x96 ^
     -i %FI_ENCODE% ^
     -c:v libsvt_av1 ^
     -g 30 -rc 1 -preset 8 -b:v 4000*1000 -f ivf ^
     %FO_REF%
call py -3 %PYTHONPATH%\check_content\check_smoke_output.py ^
     %FO_ENCODE_AV1% %FO_REF% AV1 128x96@30
echo.
if %errorlevel%==0 goto chk_encode_av1_out_passed
echo *** Encode (AV1) Stress Test FAILED ***
set /A chk_out_result_all = 1
goto chk_encode_jpeg_out

:chk_encode_av1_out_passed
echo *** Encode (AV1) Stress Test PASSED ***

:: check encode jpeg output
:chk_encode_jpeg_out
call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -f rawvideo -pixel_format yuv420p -video_size 128x96 ^
     -i %FI_ENCODE% ^
     -c:v mjpeg ^
     -g 30 -b:v 4000*1000 -f mjpeg ^
     %FO_REF%
call py -3 %PYTHONPATH%\check_content\check_smoke_output.py ^
     %FO_ENCODE_JPEG% %FO_REF% MJPEG 128x96@30
echo.
if %errorlevel%==0 goto chk_encode_jpeg_out_passed
echo *** Encode (JPEG) Stress Test FAILED ***
set /A chk_out_result_all = 1
goto done_encode_out_test

:chk_encode_jpeg_out_passed
echo *** Encode (JPEG) Stress Test PASSED ***

:done_encode_out_test

:: check vpp output
set VPP_FILTER=split=2[bg][main];^
               [bg]scale=640:480,drawbox=x=0:y=0:w=640:h=480:t=fill[bg2];^
               [main]crop=50:50:10:10,scale=300:300[ovr];^
               [bg2][ovr]overlay=10:10,format=pix_fmts=bgra

call %VPL_BUILD_DEPENDENCIES%\bin\ffmpeg.exe -y ^
     -f rawvideo -pixel_format yuv420p -video_size 128x96 ^
     -i %FI_VPP% ^
     -filter_complex "%VPP_FILTER%" ^
     -f rawvideo ^
     %FO_REF%
call py -3 %PYTHONPATH%\check_content\check_smoke_output.py ^
     %FO_VPP% %FO_REF% BGRA 640x480@30
echo.
if %errorlevel%==0 goto chk_vpp_out_passed
echo *** VPP (CROP, CSC) Stress Test FAILED ***
set /A chk_out_result_all = 1
goto done_vpp_out_test

:chk_vpp_out_passed
echo *** VPP (CROP, CSC) Stress Test PASSED ***

:done_vpp_out_test

echo.
echo [ Done %cur_iter% / %NUM_ITERATION% time(s) ]
echo.

if %cur_iter%==%NUM_ITERATION% goto end_stress_test
set /A cur_iter=cur_iter+1
goto process_start

:end_stress_test
echo *** Stress Test PASSED ***
exit /b %chk_out_result_all%