@echo off
%~d0
cd /d %~dp0

rem or Debug
set build_type=Release

mkdir build
cd build
rem -DCMAKE_BUILD_TYPE=%build_type%
cmake  -G "Visual Studio 11 2012" .. || exit /B 1 
rem 不可以调换 build config 的顺序
cmake --build . --config %build_type% || exit /B 1 

copy ..\test_wumanber.txt /B .\%build_type%\ /B /y || exit /B 1 
.\%build_type%\cpp_string_search.exe || exit /B 1 