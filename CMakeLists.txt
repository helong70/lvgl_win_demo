cmake_minimum_required(VERSION 3.16)

project(LVGL_Windows_Demo)

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

set(PLATFORM_SOURCES
    platform/windows/win32_platform.c
)

# Force C++ compilation for main.c (since it contains C++ code)
set_source_files_properties(${MAIN_SOURCE} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${UI_SOURCES} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${PLATFORM_SOURCES} PROPERTIES LANGUAGE CXX)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Create executable
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(${PROJECT_NAME} ${MAIN_SOURCE} ${UI_SOURCES} ${PLATFORM_SOURCES})
else()
    add_executable(${PROJECT_NAME} WIN32 ${MAIN_SOURCE} ${UI_SOURCES} ${PLATFORM_SOURCES})
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE lvgl 
    PRIVATE user32
    PRIVATE gdi32
    PRIVATE winmm
    PRIVATE opengl32
    PRIVATE gdiplus
)

# Set executable properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# For debug builds, use console subsystem
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()