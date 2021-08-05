@echo off
::setlocal enabledelayedexpansion
cd tool

set input_path=..\proto\server\
for /r %input_path% %%i in (*.proto) do (
protoc.exe -I=../proto/server/ --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:../protoPB/server/ %%~ni.proto
)
::protoc.exe -I=../proto/server/ --cpp_out=:../protoPB/server/ %%~ni.proto

set input_path=..\proto\client\
for /r %input_path% %%i in (*.proto) do (
protoc.exe -I=../proto/client/ --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:../protoPB/client/ %%~ni.proto
)
::protoc.exe -I=../proto/client/ --cpp_out=:../protoPB/client/ %%~ni.proto

echo success
echo Enter to gen base proto
pause

set input_path=..\proto\base\
for /r %input_path% %%i in (*.proto) do (
protoc.exe -I=../proto/base/ --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:../protoPB/base/ %%~ni.proto
)
::protoc.exe -I=../proto/base/ --cpp_out=:../protoPB/base/ %%~ni.proto

echo base success
pause 