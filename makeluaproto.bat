@echo off
cd tool\

protoc.exe -I=..\proto\client\ -I=..\proto\server\  -o..\_out\Debug\lua\room\proto\allproto.pb define.proto login.proto object.proto room.proto battle.proto common.proto dbdata.proto sroom.proto

pause