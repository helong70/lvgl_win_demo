cmake_minimum_required(VERSION 3.16)

project(LVGL_Windows_Demo)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

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

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Create executable
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(${PROJECT_NAME} ${MAIN_SOURCE})
else()
    add_executable(${PROJECT_NAME} WIN32 ${MAIN_SOURCE})
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    lvgl 
    user32
    gdi32
    winmm
    opengl32
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