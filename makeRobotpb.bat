@echo off
cd tool\

protoc.exe -I=..\proto\client\ -o..\robot\Robot\proto\allproto.pb define.proto login.proto object.proto room.proto battle.proto

pause 