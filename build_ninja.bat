@echo off
echo Building LVGL Windows Demo with Ninja...

REM Set MinGW64 and Ninja paths
set "MINGW_PATH=D:\Program Files (x86)\mingw64\bin"
set "NINJA_PATH=D:\ninja-win"
set "PATH=%MINGW_PATH%;%NINJA_PATH%;%PATH%"

REM Create build directory
if not exist build mkdir build
cd build

REM Clean previous build
if exist CMakeCache.txt del CMakeCache.txt

echo Using MinGW64 compiler with Ninja...
echo GCC path: %MINGW_PATH%\gcc.exe

REM Configure with CMake using Ninja
cmake .. -G "Ninja" -DCMAKE_C_COMPILER="%MINGW_PATH%\gcc.exe" -DCMAKE_CXX_COMPILER="%MINGW_PATH%\g++.exe" -DCMAKE_BUILD_TYPE=Release

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    goto end
)

REM Build the project with Ninja
echo Building project with Ninja...
ninja

if %errorlevel% neq 0 (
    echo Build failed!
    goto end
)

echo Build completed successfully!
echo.
if exist bin\LVGL_Windows_Demo.exe (
    echo Run bin\LVGL_Windows_Demo.exe to start the application
) else if exist LVGL_Windows_Demo.exe (
    echo Run LVGL_Windows_Demo.exe to start the application
) else (
    echo Executable not found in expected location
    dir /s *.exe
)

:end
echo.
pause