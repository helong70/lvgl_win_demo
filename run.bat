@echo off
echo Running LVGL Windows Demo...

if exist "build\bin\LVGL_Windows_Demo.exe" (
    echo Starting application...
    cd build
    .\bin\LVGL_Windows_Demo.exe
) else (
    echo Executable not found! Please build the project first.
    echo Run build_ninja.bat to build the project.
    pause
)