# Loop
参考NoahGameFrame 写的
测试版未完善 bug多多 windows 编译通过，mac、linux编译还有bug

这是一个跨平台多线程服务器框架
Feature：
简单的线程间通讯
支持mysql、redis
protobuf
http、fastcgi
简单的 ORM 目前仅支持 增删改查 创建表

Dependencies:
libuv 1.18.0
mysqlpp 3.2.2
protobuf 3.4
jsoncpp 1.8
spdlog




install mysql 5.7

windouw:
编译依赖库生成 dll 到 _out/Debug/
run MakeProto.bat
run GenSolution.bat
build build/solution/Loop.sln
