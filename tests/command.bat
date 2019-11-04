@echo off
%~dp0%
cd /d %~dp0%

rem or Debug/Release
set build_type=Debug

rmdir /Q /S build >nul 2>&1
mkdir build
cd build
rem -DCMAKE_BUILD_TYPE=%build_type% 
rem -G "Visual Studio 11 2012"
cmake   .. || exit /B 1 
rem 不可以调换 build config 的顺序
cmake --build . --config %build_type% || exit /B 1 


cd ..
cpp_string_search.exe
rmdir /Q /S build >nul 2>&1