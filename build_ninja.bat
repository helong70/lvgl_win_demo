@echo off
echo ================================================
echo Building LVGL Windows Demo with Ninja
echo ================================================
echo.

REM Parse command line arguments
REM Usage: build_ninja.bat [console|noconsole] [pack]
REM   console   - Build with console window (for debugging)
REM   noconsole - Build without console (pure GUI, default)
REM   pack      - Pack into single executable after build

set "SHOW_CONSOLE_FLAG=OFF"
set "DO_PACK=NO"

REM Check for console/noconsole argument
if /i "%1"=="console" (
    set "SHOW_CONSOLE_FLAG=ON"
    echo [Mode] Building WITH console window (debugging mode)
    echo.
) else if /i "%1"=="noconsole" (
    set "SHOW_CONSOLE_FLAG=OFF"
    echo [Mode] Building WITHOUT console window (pure GUI)
    echo.
) else if "%1"=="" (
    set "SHOW_CONSOLE_FLAG=OFF"
    echo [Mode] Building WITHOUT console window (pure GUI, default)
    echo.
)

REM Check for pack argument (can be first or second parameter)
if /i "%1"=="pack" set "DO_PACK=YES"
if /i "%2"=="pack" set "DO_PACK=YES"

if "%DO_PACK%"=="YES" (
    echo [Option] Will pack into single executable after build
    echo.
)

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
echo.

REM Configure with CMake using Ninja
cmake .. -G "Ninja" ^
    -DCMAKE_C_COMPILER="%MINGW_PATH%\gcc.exe" ^
    -DCMAKE_CXX_COMPILER="%MINGW_PATH%\g++.exe" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DSHOW_CONSOLE=%SHOW_CONSOLE_FLAG%

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

REM Pack into single executable if requested
if "%DO_PACK%"=="YES" (
    echo.
    echo ================================================
    echo Packing into Single Executable
    echo ================================================
    cd ..
    
    REM Check if main exe exists
    if not exist "build\bin\LVGL_Windows_Demo.exe" (
        echo Error: build\bin\LVGL_Windows_Demo.exe not found!
        goto end
    )
    
    REM Step 1: Copy main exe to temp location
    echo [Step 1/4] Copy main exe to temp location...
    copy /Y "build\bin\LVGL_Windows_Demo.exe" "LVGL_Windows_Demo_temp.exe" >nul
    if errorlevel 1 (
        echo Error: Failed to copy exe file.
        goto end
    )
    echo Done.
    
    REM Step 2: Compile resource file
    echo [Step 2/4] Compile resource file...
    windres launcher_packed.rc -O coff -o launcher_packed.res
    if errorlevel 1 (
        echo Error: Failed to compile resource file.
        del /F /Q "LVGL_Windows_Demo_temp.exe" >nul 2>&1
        goto end
    )
    echo Done.
    
    REM Step 3: Compile packed launcher
    echo [Step 3/4] Compile packed launcher...
    gcc -O2 -s -mwindows launcher_packed.c launcher_packed.res -o build\bin\LVGL_Windows_Demo_Noconsole.exe
    if errorlevel 1 (
        echo Error: Failed to compile packed launcher.
        del /F /Q "LVGL_Windows_Demo_temp.exe" >nul 2>&1
        del /F /Q "launcher_packed.res" >nul 2>&1
        goto end
    )
    echo Done.
    
    REM Step 4: Cleanup temp files
    echo [Step 4/4] Cleanup...
    del /F /Q "LVGL_Windows_Demo_temp.exe" >nul 2>&1
    del /F /Q "launcher_packed.res" >nul 2>&1
    echo Done.
    
    echo.
    echo ================================================
    echo Success! Created: build\bin\LVGL_Windows_Demo_Noconsole.exe
    echo ================================================
    echo.
    echo Single-file executable features:
    echo - Contains embedded main program
    echo - No console window on startup
    echo - Auto cleanup after exit
    echo - Size: ~1.3 MB
    echo.
    echo Run build\bin\LVGL_Windows_Demo_Noconsole.exe to start
)

:end
echo.
pause