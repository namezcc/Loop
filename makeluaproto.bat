@echo off
cd tool\

protoc.exe -I=..\proto\client\ -I=..\proto\server\ -I=..\proto\common\  -o..\_out\Debug\lua\room\proto\allproto.pb define.proto object.proto room.proto battle.proto common.proto dbdata.proto sroom.proto client.proto proto_common.proto

cd ..
makeRobotpb.bat

echo lua proto success
if "%1" == "" pause