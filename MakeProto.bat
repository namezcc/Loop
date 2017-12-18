::@echo off
::setlocal enabledelayedexpansion
cd tool
protoc.exe -I=../proto/ --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:../protoPB/ ../proto/LPBase.proto
protoc.exe -I=../proto/ --cpp_out=dllexport_decl=LIBPROTOC_EXPORT:../protoPB/ ../proto/LPDefine.proto
cd ..
pause