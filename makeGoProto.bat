@echo off
cd tool
set input_path=..\proto\base\
for /r %input_path% %%i in (*.proto) do (
protoc -I=../proto/base/ --go_out=plugins=grpc:../Servers/Master/goweb/tcp_test/LPMsg %%~ni.proto
)
echo base success
pause 