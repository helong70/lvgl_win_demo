@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

REM LVGL Windows Demo Release Package Script
REM Usage: create_release.bat v1.0.0 [noconsole]
REM   v1.0.0    - Version number
REM   noconsole - Use single-file no-console version

if "%1"=="" (
    echo Error: Please specify version number
    echo Usage: create_release.bat v1.0.0 [noconsole]
    echo.
    echo Parameters:
    echo   v1.0.0    - Version number (required)
    echo   noconsole - Use single-file no-console version (recommended)
    echo.
    echo Example:
    echo   create_release.bat v1.1.0 noconsole
    exit /b 1
)

set VERSION=%1
set PROJECT_NAME=LVGL_Windows_Demo
set RELEASE_NAME=%PROJECT_NAME%_%VERSION%
set RELEASE_DIR=release\%RELEASE_NAME%

REM Check if using no-console version
if /i "%2"=="noconsole" (
    set EXE_PATH=build\bin\LVGL_Windows_Demo_Noconsole.exe
    set USE_NOCONSOLE=1
    set RELEASE_NAME=%PROJECT_NAME%_%VERSION%_Noconsole
    set RELEASE_DIR=release\%RELEASE_NAME%
) else (
    set EXE_PATH=build\bin\LVGL_Windows_Demo.exe
    set USE_NOCONSOLE=0
)

echo ================================
echo Creating Release: %RELEASE_NAME%
echo ================================

REM Display build type
if "%USE_NOCONSOLE%"=="1" (
    echo Build Type: Single-file no-console version (recommended)
) else (
    echo Build Type: Standard version
)
echo.

REM Check executable file
if not exist "%EXE_PATH%" (
    echo ERROR: Executable file not found: %EXE_PATH%
    echo.
    if "%USE_NOCONSOLE%"=="1" (
        echo Please run: build_ninja.bat pack
    ) else (
        echo Please run: build_ninja.bat
    )
    exit /b 1
)

if "%USE_NOCONSOLE%"=="1" (
    echo Found: LVGL_Windows_Demo_Noconsole.exe (single-file version)
) else (
    echo Found: LVGL_Windows_Demo.exe
)

REM Create release directory
echo.
echo Creating release directory...
if exist "%RELEASE_DIR%" rmdir /s /q "%RELEASE_DIR%"
mkdir "%RELEASE_DIR%"

REM Copy files
echo Copying files...
copy "%EXE_PATH%" "%RELEASE_DIR%\" >nul
if "%USE_NOCONSOLE%"=="1" (
    echo    - LVGL_Windows_Demo_Noconsole.exe (single-file, no-console)
) else (
    echo    - LVGL_Windows_Demo.exe
)

if exist "LICENSE" (
    copy "LICENSE" "%RELEASE_DIR%\" >nul
    echo    - LICENSE
)

REM Create README.txt
echo    - README.txt
if "%USE_NOCONSOLE%"=="1" (
    set EXE_NAME=LVGL_Windows_Demo_Noconsole.exe
) else (
    set EXE_NAME=LVGL_Windows_Demo.exe
)

(
echo # LVGL Windows Demo %VERSION%
echo.
echo A LVGL graphical interface demo project running on Windows platform.
echo.
echo ## Quick Start
echo.
echo Double-click %EXE_NAME% to run.
echo.
echo ## System Requirements
echo.
echo - Windows 10 or higher
echo - Graphics card with OpenGL 2.0+ support
echo.
echo ## Features
echo.
echo - OpenGL hardware-accelerated rendering
echo - Modern borderless window with smooth rounded corners
echo - Full keyboard and mouse support
echo - Interactive UI widget demonstrations
if "%USE_NOCONSOLE%"=="1" (
echo - Single-file version with no console window
)
echo.
echo ## Controls
echo.
echo - Drag title bar to move window
echo - Tab key to switch between input fields
echo - Ctrl+C/V keyboard shortcuts
echo - Click settings icon to open settings panel
echo.
echo ## License
echo.
echo This project is licensed under GPL-3.0.
echo.
echo ## Project Repository
echo.
echo https://github.com/helong70/lvgl_win_demo
) > "%RELEASE_DIR%\README.txt"

REM Create ZIP using PowerShell (if available)
echo.
echo Creating ZIP package...
powershell -Command "Compress-Archive -Path '%RELEASE_DIR%' -DestinationPath 'release\%RELEASE_NAME%.zip' -Force" 2>nul

if exist "release\%RELEASE_NAME%.zip" (
    echo    - ZIP created: release\%RELEASE_NAME%.zip
) else (
    echo    - Warning: Cannot create ZIP, please compress %RELEASE_DIR% manually
)

echo.
echo ================================
echo Release package created!
echo ================================
echo Release directory: %RELEASE_DIR%
echo.
echo Next steps:
if "%USE_NOCONSOLE%"=="1" (
echo 1. Test: .\%RELEASE_DIR%\LVGL_Windows_Demo_Noconsole.exe
) else (
echo 1. Test: .\%RELEASE_DIR%\LVGL_Windows_Demo.exe
)
echo 2. Create tag: git tag -a %VERSION% -m "Release %VERSION%"
echo 3. Push tag: git push origin %VERSION%
echo 4. Create GitHub Release and upload ZIP file
echo.
echo Tips:
echo   - Recommended to use noconsole version for release
echo   - See RELEASE.md for detailed instructions
echo.

pause
