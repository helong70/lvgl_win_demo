@echo off
echo Building LVGL Windows Demo...

REM Create build directory
if not exist build mkdir build
cd build

REM Clean previous build
if exist CMakeCache.txt del CMakeCache.txt

echo Detecting available compilers and build tools...

REM Try to find Visual Studio tools first
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if %errorlevel% equ 0 (
    echo Using Visual Studio 2022 Community
    cmake .. -G "Visual Studio 17 2022" -A x64
    goto build
)

call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if %errorlevel% equ 0 (
    echo Using Visual Studio 2022 Professional
    cmake .. -G "Visual Studio 17 2022" -A x64
    goto build
)

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if %errorlevel% equ 0 (
    echo Using Visual Studio 2019 Community
    cmake .. -G "Visual Studio 16 2019" -A x64
    goto build
)

REM Try NMake with any available MSVC
where cl >nul 2>&1
if %errorlevel% equ 0 (
    echo Using NMake Makefiles with MSVC
    cmake .. -G "NMake Makefiles"
    goto build
)

REM Try MinGW if available
where mingw32-make >nul 2>&1
if %errorlevel% equ 0 (
    echo Using MinGW Makefiles with mingw32-make
    cmake .. -G "MinGW Makefiles"
    goto build
)

REM Try installed MinGW64 with make
if exist "D:\Program Files (x86)\mingw64\bin\gcc.exe" (
    echo Using MinGW64 from D:\Program Files (x86)\mingw64
    set "PATH=D:\Program Files (x86)\mingw64\bin;%PATH%"
    where make >nul 2>&1
    if %errorlevel% equ 0 (
        cmake .. -G "Unix Makefiles"
        goto build
    ) else (
        echo Make not found, trying MinGW Makefiles
        cmake .. -G "MinGW Makefiles"
        goto build
    )
)

REM Try Ninja if available
where ninja >nul 2>&1
if %errorlevel% equ 0 (
    echo Using Ninja
    cmake .. -G "Ninja"
    goto build
)

echo ERROR: No suitable compiler found!
echo Please install one of the following:
echo - Visual Studio 2019 or 2022 with C++ tools
echo - MinGW-w64 with mingw32-make
echo - MSYS2 with development tools
echo - Ninja build system
goto end

:build
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    goto end
)

REM Build the project
echo Building project...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo Build failed!
    goto end
)

echo Build completed successfully!
echo.
if exist bin\Release\LVGL_Windows_Demo.exe (
    echo Run bin\Release\LVGL_Windows_Demo.exe to start the application
) else if exist bin\LVGL_Windows_Demo.exe (
    echo Run bin\LVGL_Windows_Demo.exe to start the application
) else if exist LVGL_Windows_Demo.exe (
    echo Run LVGL_Windows_Demo.exe to start the application
) else (
    echo Executable not found in expected location
)

:end
echo.
pause