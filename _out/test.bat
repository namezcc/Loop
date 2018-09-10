@echo off
cd Debug

start Server.exe -t 2 -n 1 -p 22001
start Server.exe -t 3 -n 1 -p 23001
start Server.exe -t 1 -n 1 -p 21001 
start Server.exe -t 7 -n 1 -p 27001 
start Server.exe -t 10 -n 1 -p 30001 
start Server.exe -t 8 -n 1 -p 28001 
start Server.exe -t 9 -n 1 -p 29001
