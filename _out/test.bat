@echo off
cd Debug


start Server.exe -t 2 -n 1 -p 1
start Server.exe -t 1 -n 1 -p 1
start Server.exe -t 1 -n 11 -p 1

start Server.exe -t 7 -n 1 -p 1
start Server.exe -t 7 -n 2 -p 1
start Server.exe -t 7 -n 3 -p 1

start Server.exe -t 3 -n 1 -p 1
start Server.exe -t 3 -n 2 -p 1
start Server.exe -t 3 -n 3 -p 1
::--------------------------------------

::start Server.exe -t 1 -n 11 -p 21011
::start Server.exe -t 13 -n 1 -p 33001
::start Server.exe -t 14 -n 1 -p 34001
::start Server.exe -t 11 -n 1 -p 31001
::start Server.exe -t 8 -n 1 -p 28001
::start Server.exe -t 12 -n 1 -p 32001

