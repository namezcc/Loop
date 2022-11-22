@echo off
cd tool\

protoc.exe -I=..\proto\client\ -I=..\proto\common\ -o..\robot\Robot\proto\allproto.pb define.proto client.proto object.proto room.proto battle.proto proto_common.proto

echo robot lua proto success
::if "%1" == "" pause