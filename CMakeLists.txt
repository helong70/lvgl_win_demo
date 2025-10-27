cmake_minimum_required(VERSION 3.16)

project(LVGL_Windows_Demo)

# ============================================================
# Console Control: Set to ON to show console, OFF to hide it
# ============================================================
option(SHOW_CONSOLE "Show console window when running the application" OFF)

# Enable C and C++ languages
enable_language(C CXX)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Only build on Windows
if(NOT WIN32)
    message(FATAL_ERROR "This project is designed for Windows only")
endif()

# Add LVGL library
set(LVGL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lvgl)
add_subdirectory(${LVGL_DIR})

# Set the main source file
if(NOT DEFINED MAIN_SOURCE)
    set(MAIN_SOURCE "main.c")
endif()

# Add UI source files
set(UI_SOURCES
    ui/setting.c
    ui/maincontainer.c
    ui/titlebar.c
)

# Add HAL source files
set(HAL_SOURCES
    hal/lv_hal.c
    hal/lv_hal_disp.c
    hal/lv_hal_indev.c
)

set(PLATFORM_SOURCES
    platform/windows/win32_platform.c
)

# Force C++ compilation for main.c (since it contains C++ code)
set_source_files_properties(${MAIN_SOURCE} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${UI_SOURCES} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${HAL_SOURCES} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${PLATFORM_SOURCES} PROPERTIES LANGUAGE CXX)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Create executable
if(SHOW_CONSOLE)
    # Console mode: show terminal window with debug output
    add_executable(${PROJECT_NAME} ${MAIN_SOURCE} ${UI_SOURCES} ${HAL_SOURCES} ${PLATFORM_SOURCES})
    message(STATUS "Building with CONSOLE window (for debugging)")
else()
    # GUI mode: no console window (clean application)
    add_executable(${PROJECT_NAME} WIN32 ${MAIN_SOURCE} ${UI_SOURCES} ${HAL_SOURCES} ${PLATFORM_SOURCES})
    message(STATUS "Building WITHOUT console window (pure GUI)")
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE lvgl 
    PRIVATE user32
    PRIVATE gdi32
    PRIVATE winmm
    PRIVATE opengl32
    PRIVATE gdiplus
    PRIVATE dwmapi
)

# Set linker flags for console control (must be set AFTER target_link_libraries)
if(NOT SHOW_CONSOLE)
    # Force Windows GUI subsystem (no console window)
    # Use -Wl,-e to specify WinMain as entry point and force GUI subsystem
    target_link_options(${PROJECT_NAME} PRIVATE 
        -Wl,--subsystem,windows:6.0
        -Wl,-eWinMainCRTStartup
    )
endif()

# Set executable properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Console subsystem control
if(SHOW_CONSOLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_CONSOLE_OUTPUT)
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    # No ENABLE_CONSOLE_OUTPUT defined = GUI mode with WinMain
endif()