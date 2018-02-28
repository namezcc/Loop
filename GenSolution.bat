if not exist build md build\solution
cd build\solution\
del /s /q /f *.*
cmake -G "Visual Studio 15 2017 Win64" ^
../..
pause