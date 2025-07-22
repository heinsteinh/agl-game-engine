# AGL Game Engine - Sandbox Demo Project
cmake_minimum_required(VERSION 3.14)
project(AGL_Sandbox VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Demo selection option
set(DEMO_NAME "renderer" CACHE STRING "Name of the demo to build")
set_property(CACHE DEMO_NAME PROPERTY STRINGS "renderer" "texture" "benchmark_deltatime" "example_logger")

message(STATUS "Building demo: ${DEMO_NAME}")

# Find the demo source file
set(DEMO_SOURCE "src/${DEMO_NAME}_demo.cpp")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DEMO_SOURCE}")
    message(FATAL_ERROR "Demo file ${DEMO_SOURCE} does not exist! Available demos: renderer, texture, benchmark_deltatime, example_logger")
endif()

# Create the demo executable
add_executable(${PROJECT_NAME} ${DEMO_SOURCE})

# Set the target name based on demo
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "agl_${DEMO_NAME}_demo"
)

# Check if gamelib target exists (when built as part of main project)
if(TARGET gamelib)
    # Link the gamelib library (this automatically includes all dependencies)
    target_link_libraries(${PROJECT_NAME} PRIVATE gamelib)
    message(STATUS "Using gamelib target from parent project")
else()
    # Try to find pre-built gamelib first
    find_package(agl-gamelib QUIET)
    
    if(agl-gamelib_FOUND)
        # Use pre-built gamelib library
        target_link_libraries(${PROJECT_NAME} PRIVATE AGL::gamelib)
        message(STATUS "Using pre-built gamelib from: ${agl-gamelib_DIR}")
    else()
        # Fallback: Standalone build by adding gamelib as subdirectory
        message(STATUS "Standalone build: adding gamelib as subdirectory")

        # Include parent project dependencies
        include(../Dependency.cmake)

        # Add gamelib
        add_subdirectory(../gamelib gamelib_build)

        # Link the gamelib library
        target_link_libraries(${PROJECT_NAME} PRIVATE gamelib)
    endif()
endif()

# Additional include directories for demo-specific code
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Copy assets to build directory if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    file(GLOB_RECURSE ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")
    if(ASSETS)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
             DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        message(STATUS "Copied assets to build directory")
    endif()
endif()

# Set debug working directory for Visual Studio
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
endif()

# Display build information
message(STATUS "=== AGL Sandbox Build Configuration ===")
message(STATUS "Demo: ${DEMO_NAME}")
message(STATUS "Source: ${DEMO_SOURCE}")
message(STATUS "Output: agl_${DEMO_NAME}_demo")
message(STATUS "Target depends on: gamelib")
message(STATUS "=======================================")
