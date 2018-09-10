@echo off
::setlocal enabledelayedexpansion
cd tool

set input_path=..\proto\server\
for /r %input_path% %%i in (*.proto) do (
protoc.exe -I=../proto/server/ --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:../protoPB/server/ %%~ni.proto
)

set input_path=..\proto\client\
for /r %input_path% %%i in (*.proto) do (
protoc.exe -I=../proto/client/ --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:../protoPB/client/ %%~ni.proto
)

echo success
echo Enter to make base
pause

set input_path=..\proto\base\
for /r %input_path% %%i in (*.proto) do (
protoc.exe -I=../proto/base/ --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:../protoPB/base/ %%~ni.proto
)

echo base success
pause 