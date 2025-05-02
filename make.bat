@echo off
mkdir build
cd build
del /F /Q CMakeCache.txt
rmdir /S /Q CMakeFiles
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
mingw32-make
cd ..
