@echo off
::setlocal enabledelayedexpansion
cd tool

set input_path=..\proto\server\
for /r %input_path% %%i in (*.proto) do (
::echo %%~ni
protoc.exe -I=../proto/server/ --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:../protoPB/server/ %%~ni.proto
)

set input_path=..\proto\client\
for /r %input_path% %%i in (*.proto) do (
::echo %%~ni
protoc.exe -I=../proto/client/ --cpp_out=../protoPB/client/ %%~ni.proto
)

cd ..
echo success
pause