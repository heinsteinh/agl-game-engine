# AGL Game Engine - Root CMake Configuration
cmake_minimum_required(VERSION 3.14)  # FetchContent requires 3.14+
project(AGLGameEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Global compiler flags
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()


# Set the output directories for the compiled binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Host/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Host/lib)
set(INCLUDE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Host/include)


# Add dependency management first
include(cmake/Dependency.cmake)

# Add subdirectories
add_subdirectory(gamelib)
add_subdirectory(sandbox)

# Set startup project for Visual Studio
if(WIN32 AND CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT sandbox)
endif()

message(STATUS "=== AGL Game Engine Build Configuration ===")
message(STATUS "Root project: ${PROJECT_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "===========================================")
