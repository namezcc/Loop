@echo off

::echo %1

tasklist /FI "IMAGENAME eq Server.exe" /V /FO CSV | findstr %1-%2

::echo test