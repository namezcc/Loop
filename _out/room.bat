@echo off
cd Debug

::start Server.exe -t 8 -n 1 -p 28001
REM start Server.exe -t 2 -n 1 -p 1

start Server.exe -t 7 -n 1 -p 1
start Server.exe -t 7 -n 2 -p 1
start Server.exe -t 7 -n 3 -p 1