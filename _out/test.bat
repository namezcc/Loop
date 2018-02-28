@echo off
cd Debug

::game
start Test.exe -p 24001 -t 0 -n 1
::proxy
::start LoopProxy.exe -p 25001 -t 1 -n 1
::sql
::start Test.exe -p 26001 -t 3 -n 1