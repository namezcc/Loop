@echo off
cd %~dp0
if not exist Build md Build
cd Build

if /i "%1"=="help" goto help
if /i "%1"=="-h" goto help
if /i "%1"=="?" goto help
if /i "%1"=="-?" goto help

set config=Debug

:next-arg
if "%1"=="clean" goto args-clean
if "%1"=="" goto args-done
if /i "%1"=="debug"        set config=Debug&goto arg-ok
if /i "%1"=="release"      set config=Release&goto arg-ok
:arg-ok
shift
goto next-arg
:args-clean
del /s /q /f *.*
cmake -G "NMake Makefiles" ^
-DCMAKE_BUILD_TYPE=%config% ^
..
pause
:args-done
nmake
goto exit
:help
echo make.bat [debug/release]
:exit