cmake_minimum_required(VERSION 3.16)

# Project
project(lvgl LANGUAGES C)

# Only build on Windows
if(NOT WIN32)
    message(FATAL_ERROR "This configuration is for Windows only")
endif()

# Collect LVGL source files
file(GLOB_RECURSE LVGL_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Remove platform-specific files we don't need
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*drivers/display.*")
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*drivers/draw.*")
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*drivers/evdev.*")
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*drivers/libinput.*")
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*drivers/nuttx.*")
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*drivers/opengles.*")
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*drivers/qnx.*")
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*drivers/sdl.*")
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*drivers/uefi.*")
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*drivers/wayland.*")
list(FILTER LVGL_SOURCES EXCLUDE REGEX ".*drivers/x11.*")

# Add Windows driver sources
file(GLOB LVGL_WINDOWS_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/windows/*.c"
)

# Create LVGL library
add_library(lvgl STATIC 
    ${LVGL_SOURCES}
    ${LVGL_WINDOWS_SOURCES}
)

# Include directories
target_include_directories(lvgl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compile definitions
target_compile_definitions(lvgl PUBLIC
    LV_CONF_INCLUDE_SIMPLE
)

# Link Windows libraries
target_link_libraries(lvgl PUBLIC 
    user32
    gdi32
    winmm
)