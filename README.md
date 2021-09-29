# Loop

跨平台多线程服务器框架
Feature：
简单无锁的线程间通讯
protobuf
http
简单的 ORM 目前仅支持 增删改查 创建表
coroutine
lua

Dependencies:
libuv 1.18.0
mysqlpp 3.2.2
protobuf 3.4
jsoncpp 1.8
boost 1.66



install mysql 5.7

windouw:
编译依赖库生成 dll 到 _out/Debug/
run MakeProto.bat
run GenSolution.bat
build build/solution/Loop.sln
