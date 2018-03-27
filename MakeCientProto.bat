@echo off
cd tool
set input_path=..\proto\client\

for /r %input_path% %%i in (*.proto) do (
::echo %%~ni
protoc.exe -I=../proto/client/ --python_out=../../../TestClient/ %%~ni.proto
)

cd ..
echo success
pause