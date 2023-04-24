@echo off

echo Setting up environment for building Qt 6.5.0
echo MinGW64 12.2 and GnuWin32 make

set PATH=C:\Ninja;C:\CMake\bin;C:\Python311;C:\mingw64\bin;%PATH%
cd /D C:\Qt6\Src
