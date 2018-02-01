@echo off
cd %~dp0
cd Build

if /i "%1"=="help" goto help
if /i "%1"=="-h" goto help
if /i "%1"=="?" goto help
if /i "%1"=="-?" goto help

set config=Debug
set libShared=OFF

:next-arg
if "%1"=="" goto args-done
if /i "%1"=="debug"        set config=Debug&goto arg-ok
if /i "%1"=="release"      set config=Release&goto arg-ok
if /i "%1"=="shared"       set libShared=ON&goto arg-ok
if /i "%1"=="static"       set libShared=OFF&goto arg-ok
:arg-ok
shift
goto next-arg
:args-done

cmake -G "NMake Makefiles" ^
-DCMAKE_BUILD_TYPE=%config% ^
-DBUILD_SHARED_LIBS=%libShared% ^
..
pause
nmake
goto exit
:help
echo make.bat [debug/release] [static/shared]
:exit