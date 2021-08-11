@echo off
cd ..\..\..\tool\

protoc.exe -I=..\proto\client\ -o..\_out\Debug\lua\room\proto\allproto.pb define.proto login.proto object.proto room.proto battle.proto

pause 