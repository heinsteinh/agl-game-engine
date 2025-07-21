# Sandbox Demo Project for AGL Game Engine
cmake_minimum_required(VERSION 3.14)
project(AGL_Sandbox VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include the main project's dependency management
include(../Dependency.cmake)

# Include directories from the main project
include_directories(../include)
include_directories(../src)

# Collect all source files from the main project (excluding main.cpp and demo files)
file(GLOB_RECURSE ENGINE_SOURCES "../src/*.cpp")
# Remove main.cpp and existing demo files from engine sources
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "main\\.cpp$")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*_demo\\.cpp$")

# Demo selection option
set(DEMO_NAME "basic" CACHE STRING "Name of the demo to build")
set_property(CACHE DEMO_NAME PROPERTY STRINGS "basic" "renderer" "texture" "input" "game")

message(STATUS "Building demo: ${DEMO_NAME}")

# Demo source files
set(DEMO_SOURCE "demos/${DEMO_NAME}_demo.cpp")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DEMO_SOURCE}")
    message(FATAL_ERROR "Demo file ${DEMO_SOURCE} does not exist!")
endif()

# Create the demo executable
add_executable(${PROJECT_NAME} 
    ${ENGINE_SOURCES}
    ${DEMO_SOURCE}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/../src
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glm::glm
    stb
    imgui
)

# Find and link OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# Platform-specific configurations
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE X11 Xrandr Xinerama Xcursor GL dl pthread)
endif()

# Set the target name based on demo
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "agl_${DEMO_NAME}_demo")

# Display build information
message(STATUS "=== AGL Sandbox Build Configuration ===")
message(STATUS "Demo: ${DEMO_NAME}")
message(STATUS "Output: agl_${DEMO_NAME}_demo")
message(STATUS "=====================================")
